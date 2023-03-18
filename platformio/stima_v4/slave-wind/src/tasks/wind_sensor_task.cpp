/**@file swind_sensor_task.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@digiteco.it>
authors:
Marco Baldinetti <marco.baldinetti@digiteco.it>

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

#define TRACE_LEVEL     WIND_SENSOR_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   SENSOR_TASK_ID

#include "tasks/wind_sensor_task.h"

#if (MODULE_TYPE == STIMA_MODULE_TYPE_WIND)

using namespace cpp_freertos;

WindSensorTask::WindSensorTask(const char *taskName, uint16_t stackSize, uint8_t priority, WindSensorParam_t windSensorParam) : Thread(taskName, stackSize, priority), param(windSensorParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(SENSOR_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  state = SENSOR_STATE_WAIT_CFG;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void WindSensorTask::TaskMonitorStack()
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
void WindSensorTask::TaskWatchDog(uint32_t millis_standby)
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
void WindSensorTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Signal Task sleep/disabled mode from request (Auto SET WDT on Resume)
  if((param.system_status->tasks[LOCAL_TASK_ID].state == task_flag::suspended)&&
     (state_operation==task_flag::normal))
     param.system_status->tasks->watch_dog = wdt_flag::set;
  param.system_status->tasks[LOCAL_TASK_ID].state = state_operation;
  param.system_status->tasks[LOCAL_TASK_ID].running_pos = state_position;
  param.system_status->tasks[LOCAL_TASK_ID].running_sub = state_subposition;
  param.systemStatusLock->Give();
}

void WindSensorTask::Run() {
  rmapdata_t values_readed_from_sensor[VALUES_TO_READ_FROM_SENSOR_COUNT];
  elaborate_data_t edata;
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
        TaskWatchDog(WIND_TASK_WAIT_DELAY_MS);
        Delay(Ticks::MsToTicks(WIND_TASK_WAIT_DELAY_MS));
      }
      // do something else with non-blocking wait ....
      break;

    case SENSOR_STATE_INIT:
      TRACE_INFO_F(F("Initializing sensors...\r\n"));
      retry = 0;
      is_error = false;
      serialReset();
      wind_acquisition_count++;

      if (isWindOff())
      {
        windPowerOn();
        TaskWatchDog(WIND_POWER_ON_DELAY_MS);
        Delay(Ticks::MsToTicks(WIND_POWER_ON_DELAY_MS));
        TRACE_VERBOSE_F(F("SENSOR_STATE_INIT --> SENSOR_STATE_READING\r\n"));
        state = SENSOR_STATE_READING;
      }
      else
      {
        TRACE_VERBOSE_F(F("SENSOR_STATE_INIT --> SENSOR_STATE_READING\r\n"));
        state = SENSOR_STATE_READING;
      }
      break;

    case SENSOR_STATE_READING:
      if (Serial2.available())
      {
        uart_rx_buffer_length = Serial2.readBytes(uart_rx_buffer, UART_RX_BUFFER_LENGTH);
        state = SENSOR_STATE_ELABORATE;
        TRACE_VERBOSE_F(F("SENSOR_STATE_READING --> SENSOR_STATE_ELABORATE\r\n"));
      }
      else if (++retry <= WIND_RETRY_MAX)
      {
        TaskWatchDog(WIND_RETRY_DELAY_MS);
        Delay(Ticks::MsToTicks(WIND_RETRY_DELAY_MS));
      }
      else
      {
        is_error = true;
        error_count++;
        state = SENSOR_STATE_ELABORATE;
        TRACE_VERBOSE_F(F("SENSOR_STATE_READING --> SENSOR_STATE_ELABORATE\r\n"));
      }
      break;

    case SENSOR_STATE_ELABORATE:
      if (is_error)
      {
        speed = FLT_MAX;
        direction = FLT_MAX;
      }
      else
      {
        windsonicInterpreter(&speed, &direction);
      }

      edata.value = (rmapdata_t)(speed * WIND_CASTING_SPEED_MULT);
      edata.index = WIND_SPEED_INDEX;
      param.elaborataDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));

      edata.value = (rmapdata_t)(direction * WIND_CASTING_DIRECTION_MULT);
      edata.index = WIND_DIRECTION_INDEX;
      param.elaborataDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));

      TRACE_VERBOSE_F(F("SENSOR_STATE_ELABORATE --> SENSOR_STATE_END\r\n"));
      state = SENSOR_STATE_END;
      break;

      case SENSOR_STATE_END:
        #ifdef TH_TASK_LOW_POWER_ENABLED
        powerOff();
        #else
        if (error_count > WIND_TASK_ERROR_FOR_POWER_OFF)
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

        state = SENSOR_STATE_READING;
        break;
    }
  }
}

void WindSensorTask::serialReset()
{
  while (Serial2.available())
  {
    Serial2.read();
  }
  memset(uart_rx_buffer, 0, uart_rx_buffer_length);
  uart_rx_buffer_length = 0;
}

bool WindSensorTask::isWindOn()
{
  return is_power_on;
}

bool WindSensorTask::isWindOff()
{
  return (!is_power_on);
}

void WindSensorTask::powerOn()
{
  if (!is_power_on)
  {
    digitalWrite(PIN_EN_5VS, HIGH);  // Enable + 5VS / +3V3S External Connector Power Sens
    digitalWrite(PIN_EN_SPLY, HIGH); // Enable Supply + 3V3_I2C / + 5V_I2C
    digitalWrite(PIN_I2C2_EN, HIGH); // I2C External Enable PIN (LevelShitf PCA9517D)
    // WDT
    TaskWatchDog(WIND_TASK_POWER_ON_WAIT_DELAY_MS);
    Delay(Ticks::MsToTicks(WIND_TASK_POWER_ON_WAIT_DELAY_MS));
    is_power_on = true;
  }
}

void WindSensorTask::powerOff()
{
  digitalWrite(PIN_EN_5VS, LOW);  // Enable + 5VS / +3V3S External Connector Power Sens
  digitalWrite(PIN_EN_SPLY, LOW); // Enable Supply + 3V3_I2C / + 5V_I2C
  digitalWrite(PIN_I2C2_EN, LOW); // I2C External Enable PIN (LevelShitf PCA9517D)
  is_power_on = false;
}

// <0x02>Q,000,000.04,M,00,<0x03>1A␍␊
bool WindSensorTask::windsonicInterpreter(float *speed, float *direction)
{
  char tempstr[GWS_SPEED_LENGTH + 1];
  char *tempstrptr;
  uint8_t myCrc = 0;
  int crc = 0;
  bool is_crc_ok = false;
  *speed = UINT16_MAX;
  *direction = UINT16_MAX;
  memset(tempstr, 0, GWS_SPEED_LENGTH + 1);

  if ((uart_rx_buffer[GWS_STX_INDEX] == STX_VALUE) && (uart_rx_buffer[GWS_ETX_INDEX] == ETX_VALUE) && (uart_rx_buffer[uart_rx_buffer_length - 2] == CR_VALUE) && (uart_rx_buffer[uart_rx_buffer_length - 1] == LF_VALUE))
  {
    strncpy(tempstr, (const char *)(uart_rx_buffer + GWS_DIRECTION_INDEX), GWS_DIRECTION_LENGTH);
    *direction = (float)atof(tempstr);
    memset(tempstr, 0, GWS_SPEED_LENGTH + 1);

    strncpy(tempstr, (const char *)(uart_rx_buffer + GWS_SPEED_INDEX), GWS_SPEED_LENGTH);
    *speed = (float)atof(tempstr);
    memset(tempstr, 0, GWS_SPEED_LENGTH + 1);

    if (*speed < CALM_WIND_MAX_MS)
    {
        *speed = WIND_SPEED_MIN;
    }
    else if (*speed > WIND_SPEED_MAX)
    {
        *speed = UINT16_MAX;
    }

    strncpy(tempstr, (const char *)(uart_rx_buffer + GWS_CRC_INDEX), GWS_CRC_LENGTH);
    crc = (uint8_t)strtol(tempstr, &tempstrptr, 16);
    memset(tempstr, 0, GWS_SPEED_LENGTH + 1);

    for (uint8_t i = GWS_STX_INDEX + 1; i < GWS_ETX_INDEX; i++)
    {
        myCrc ^= uart_rx_buffer[i];
    }

    if (*direction < WIND_DIRECTION_MIN)
    {
        *direction = WIND_DIRECTION_MIN;
    }
    else if (*direction > WIND_DIRECTION_MAX)
    {
        *direction = UINT16_MAX;
    }

    is_crc_ok = (crc == myCrc);

    if (!ISVALID_FLOAT(*speed) || !ISVALID_FLOAT(*direction))
    {
        is_crc_ok = false;
    }
  }
  else if ((uart_rx_buffer[GWS_STX_INDEX] == STX_VALUE) && (uart_rx_buffer[GWS_ETX_INDEX - GWS_WITHOUT_DIRECTION_OFFSET] == ETX_VALUE) && (uart_rx_buffer[uart_rx_buffer_length - 2] == CR_VALUE) && (uart_rx_buffer[uart_rx_buffer_length - 1] == LF_VALUE))
  {
    *direction = WIND_DIRECTION_MIN;

    strncpy(tempstr, (const char *)(uart_rx_buffer + GWS_SPEED_INDEX - GWS_WITHOUT_DIRECTION_OFFSET), GWS_SPEED_LENGTH);
    *speed = (float)atof(tempstr);
    memset(tempstr, 0, GWS_SPEED_LENGTH + 1);

    if (*speed < CALM_WIND_MAX_MS)
    {
        *speed = WIND_SPEED_MIN;
    }
    else if (*speed > WIND_SPEED_MAX)
    {
        *speed = UINT16_MAX;
    }

    strncpy(tempstr, (const char *)(uart_rx_buffer + GWS_CRC_INDEX - GWS_WITHOUT_DIRECTION_OFFSET), GWS_CRC_LENGTH);
    crc = (uint8_t)strtol(tempstr, &tempstrptr, 16);
    memset(tempstr, 0, GWS_SPEED_LENGTH + 1);

    for (uint8_t i = GWS_STX_INDEX + 1; i < (GWS_ETX_INDEX - GWS_WITHOUT_DIRECTION_OFFSET); i++)
    {
        myCrc ^= uart_rx_buffer[i];
    }

    is_crc_ok = (crc == myCrc);

    if (!ISVALID_FLOAT(*speed))
    {
        is_crc_ok = false;
    }
  }

  return is_crc_ok;
}

#endif