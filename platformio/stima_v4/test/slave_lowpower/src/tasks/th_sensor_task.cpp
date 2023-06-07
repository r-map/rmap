/**@file th_sensor_task.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
<http://www.gnu.org/licenses/>.
**********************************************************************/

#define TRACE_LEVEL     TH_SENSOR_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   SENSOR_TASK_ID

#include "tasks/th_sensor_task.h"

#if ((MODULE_TYPE == STIMA_MODULE_TYPE_THR) || (MODULE_TYPE == STIMA_MODULE_TYPE_TH))

using namespace cpp_freertos;

TemperatureHumidtySensorTask::TemperatureHumidtySensorTask(const char *taskName, uint16_t stackSize, uint8_t priority, TemperatureHumidtySensorParam_t temperatureHumidtySensorParam) : Thread(taskName, stackSize, priority), param(temperatureHumidtySensorParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(SENSOR_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  state = SENSOR_STATE_WAIT_CFG;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void TemperatureHumidtySensorTask::TaskMonitorStack()
{
  u_int16_t stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark( NULL );
  if((stackUsage) && (stackUsage < param.system_status->tasks[LOCAL_TASK_ID].stack)) {
    param.systemStatusLock->Take();
    param.system_status->tasks[LOCAL_TASK_ID].stack = stackUsage;
    param.systemStatusLock->Give();
  }
}
#endif

/// @brief local watchDog and Sleep flag Task (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void TemperatureHumidtySensorTask::TaskWatchDog(uint32_t millis_standby)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Update WDT Signal (Direct or Long function Timered)
  if(millis_standby)  
  {
    // Check 1/2 Freq. controller ready to WDT only SET flag
    if((millis_standby) < WDT_CONTROLLER_MS / 2) {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
    } else {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::timer;
      // Add security milimal Freq to check
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog_ms = millis_standby + WDT_CONTROLLER_MS;
    }
  }
  else
    param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
  param.systemStatusLock->Give();
}

/// @brief local suspend flag and positor running state Task (optional)
/// @param state_position Sw_Position (Local STATE)
/// @param state_subposition Sw_SubPosition (Optional Local SUB_STATE Position Monitor)
/// @param state_operation operative mode flag status for this task
void TemperatureHumidtySensorTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Signal Task sleep/disabled mode from request (Auto SET WDT on Resume)
  if((param.system_status->tasks[LOCAL_TASK_ID].state == task_flag::suspended)&&
     (state_operation==task_flag::normal))
     param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
  param.system_status->tasks[LOCAL_TASK_ID].state = state_operation;
  param.system_status->tasks[LOCAL_TASK_ID].running_pos = state_position;
  param.system_status->tasks[LOCAL_TASK_ID].running_sub = state_subposition;
  param.systemStatusLock->Give();
}

void TemperatureHumidtySensorTask::Run() {
  rmapdata_t values_readed_from_sensor[VALUES_TO_READ_FROM_SENSOR_COUNT];
  elaborate_data_t edata;
  uint32_t delay_ms;
  static bool is_test;
  bool is_temperature_redundant;
  bool is_humidity_redundant;
  // Request response for system queue Task controlled...
  system_message_t system_message;
  
  uint8_t error_count;

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  powerOff();

  while (true)
  {

    switch (state)
    {
    case SENSOR_STATE_WAIT_CFG:
      // check if configuration is done loaded
      if (param.system_status->flags.is_cfg_loaded)
      {
        TRACE_VERBOSE_F(F("WAIT -> INIT\r\n"));
        state = SENSOR_STATE_INIT;
      }
      // other
      else
      {
        // Local WatchDog update;
        TaskWatchDog(TH_TASK_WAIT_DELAY_MS);
        Delay(Ticks::MsToTicks(TH_TASK_WAIT_DELAY_MS));
      }
      // do something else with non-blocking wait ....
      break;

    case SENSOR_STATE_INIT:
      TRACE_INFO_F(F("Initializing sensors...\r\n"));
      for (uint8_t i = 0; i < param.configuration->sensors_count; i++)
      {
        if (strlen(param.configuration->sensors[i].type) == 3)
        {
          SensorDriver::createSensor(SENSOR_DRIVER_I2C, param.configuration->sensors[i].type, param.configuration->sensors[i].i2c_address, 1, sensors, param.wire);
        }
      }
      state = SENSOR_STATE_SETUP;
      break;

    case SENSOR_STATE_SETUP:
      error_count = 0;

      powerOn();

      is_test = false;
      memset((void *)values_readed_from_sensor, RMAPDATA_MAX, (size_t)(VALUES_TO_READ_FROM_SENSOR_COUNT * sizeof(rmapdata_t)));

      for (uint8_t i = 0; i < SensorDriver::getSensorsCount(); i++)
      {
        if (!sensors[i]->isSetted())
        {
          param.wireLock->Take();
          sensors[i]->setup();
          param.wireLock->Give();
          TRACE_INFO_F(F("--> %u: %s-%s 0x%02X [ %s ]\t [ %s ]\r\n"), i + 1, SENSOR_DRIVER_I2C, sensors[i]->getType(), sensors[i]->getAddress(), param.configuration->sensors[i].is_redundant ? REDUNDANT_STRING : MAIN_STRING, sensors[i]->isSetted() ? OK_STRING : FAIL_STRING);
        }
      }
      state = SENSOR_STATE_PREPARE;
      break;

    case SENSOR_STATE_PREPARE:
      delay_ms = 0;
      for (uint8_t i = 0; i < SensorDriver::getSensorsCount(); i++)
      {
        sensors[i]->resetPrepared();
        param.wireLock->Take();
        sensors[i]->prepare(is_test);
        param.wireLock->Give();

        // wait the most slowest
        if (sensors[i]->getDelay() > delay_ms)
        {
          delay_ms = sensors[i]->getDelay();
        }

        // end with error
        if (!sensors[i]->isSuccess())
        {
          error_count++;
        }
      }

      // Local WatchDog update;
      TaskWatchDog(delay_ms);
      Delay(Ticks::MsToTicks(delay_ms));

      state = SENSOR_STATE_READ;

      break;

      case SENSOR_STATE_READ:
        is_temperature_redundant = false;
        is_humidity_redundant = false;
        
        for (uint8_t i=0; i<SensorDriver::getSensorsCount(); i++) {
          do {
            param.wireLock->Take();
            sensors[i]->get(&values_readed_from_sensor[0], VALUES_TO_READ_FROM_SENSOR_COUNT, is_test);
            param.wireLock->Give();
            // Secure WDT
            TaskWatchDog(sensors[i]->getDelay());
            Delay(Ticks::MsToTicks(sensors[i]->getDelay()));
          } while (!sensors[i]->isEnd() && !sensors[i]->isReaded());

          // end with error
          if (!sensors[i]->isSuccess())
          {
            error_count++;
          }

          if (false) {}

          #if (USE_SENSOR_ADT)
          else if (strcmp(sensors[i]->getType(), SENSOR_TYPE_ADT) == 0) {
            edata.value = values_readed_from_sensor[0];
            edata.index = param.configuration->sensors[i].is_redundant ? TEMPERATURE_REDUNDANT_INDEX : TEMPERATURE_MAIN_INDEX;
            param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));
            is_temperature_redundant = param.configuration->sensors[i].is_redundant;
          }
          #endif

          #if (USE_SENSOR_HIH)
          else if (strcmp(sensors[i]->getType(), SENSOR_TYPE_HIH) == 0) {
            edata.value = values_readed_from_sensor[0];
            edata.index = param.configuration->sensors[i].is_redundant ? HUMIDITY_REDUNDANT_INDEX : HUMIDITY_MAIN_INDEX;
            param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));
            is_humidity_redundant = param.configuration->sensors[i].is_redundant;
          }
          #endif

          #if (USE_SENSOR_HYT)
          else if (strcmp(sensors[i]->getType(), SENSOR_TYPE_HYT) == 0) {
            edata.value = values_readed_from_sensor[1];
            edata.index = param.configuration->sensors[i].is_redundant ? TEMPERATURE_REDUNDANT_INDEX : TEMPERATURE_MAIN_INDEX;
            param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));
            is_temperature_redundant = param.configuration->sensors[i].is_redundant;

            edata.value = values_readed_from_sensor[0];
            edata.index = param.configuration->sensors[i].is_redundant ? HUMIDITY_REDUNDANT_INDEX : HUMIDITY_MAIN_INDEX;
            param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));
            is_humidity_redundant = param.configuration->sensors[i].is_redundant;
          }
          #endif

          #if (USE_SENSOR_SHT)
          else if (strcmp(sensors[i]->getType(), SENSOR_TYPE_SHT) == 0) {
            edata.value = values_readed_from_sensor[1];
            edata.index = param.configuration->sensors[i].is_redundant ? TEMPERATURE_REDUNDANT_INDEX : TEMPERATURE_MAIN_INDEX;
            param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));
            is_temperature_redundant = param.configuration->sensors[i].is_redundant;

            edata.value = values_readed_from_sensor[0];
            edata.index = param.configuration->sensors[i].is_redundant ? HUMIDITY_REDUNDANT_INDEX : HUMIDITY_MAIN_INDEX;
            param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));
            is_humidity_redundant = param.configuration->sensors[i].is_redundant;
          }
          #endif

          #if (TRACE_LEVEL >= TRACE_LEVEL_VERBOSE)
          for (uint8_t k=0; k<VALUES_TO_READ_FROM_SENSOR_COUNT; k++) {
            TRACE_VERBOSE_F(F("%d\t"), values_readed_from_sensor[k]);
          }
          TRACE_VERBOSE_F(F("\r\n"));
          #endif
        }

        // If module fail fill void error data
        if (!is_temperature_redundant) {
          edata.value = RMAPDATA_MAX;
          edata.index = TEMPERATURE_REDUNDANT_INDEX;
          param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));
        }

        // If module fail fill void error data
        if (!is_humidity_redundant) {
          edata.value = RMAPDATA_MAX;
          edata.index = HUMIDITY_REDUNDANT_INDEX;
          param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));
        }

        state = SENSOR_STATE_END;
        break;

      case SENSOR_STATE_END:
        #ifdef TH_TASK_LOW_POWER_ENABLED
        powerOff();
        #else
        if (error_count > TH_TASK_ERROR_FOR_POWER_OFF)
        {
          powerOff();
        }
        #endif

        #if (ENABLE_STACK_USAGE)
        TaskMonitorStack();
        #endif

        // Local TaskWatchDog update and Sleep Activate before Next Read
        TaskWatchDog(param.configuration->sensor_acquisition_delay_ms);
        TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
        DelayUntil(Ticks::MsToTicks(param.configuration->sensor_acquisition_delay_ms));
        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

        state = SENSOR_STATE_SETUP;
        break;
    }
  }
}

void TemperatureHumidtySensorTask::powerOn()
{
  if (!is_power_on)
  {
    digitalWrite(PIN_EN_5VS, HIGH);  // Enable + 5VS / +3V3S External Connector Power Sens
    digitalWrite(PIN_EN_SPLY, HIGH); // Enable Supply + 3V3_I2C / + 5V_I2C
    digitalWrite(PIN_I2C2_EN, HIGH); // I2C External Enable PIN (LevelShitf PCA9517D)
    // WDT
    TaskWatchDog(TH_TASK_POWER_ON_WAIT_DELAY_MS);
    Delay(Ticks::MsToTicks(TH_TASK_POWER_ON_WAIT_DELAY_MS));
    is_power_on = true;
  }
}

void TemperatureHumidtySensorTask::powerOff()
{
  digitalWrite(PIN_EN_5VS, LOW);  // Enable + 5VS / +3V3S External Connector Power Sens
  digitalWrite(PIN_EN_SPLY, LOW); // Enable Supply + 3V3_I2C / + 5V_I2C
  digitalWrite(PIN_I2C2_EN, LOW); // I2C External Enable PIN (LevelShitf PCA9517D)
  is_power_on = false;
}

#endif