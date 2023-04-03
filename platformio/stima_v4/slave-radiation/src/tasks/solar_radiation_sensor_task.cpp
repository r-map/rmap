/**
  ******************************************************************************
  * @file    solar_radiation_task.cpp
  * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Module sensor source file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (C) 2022  Moreno Gasperini <m.gasperini@digiteco.it>
  * All rights reserved.</center></h2>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  * 
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
  * <http://www.gnu.org/licenses/>.
  * 
  ******************************************************************************
*/

#define TRACE_LEVEL     SOLAR_RADIATION_SENSOR_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   SENSOR_TASK_ID

#include "tasks/solar_radiation_sensor_task.h"

#if (MODULE_TYPE == STIMA_MODULE_TYPE_SOLAR_RADIATION)

using namespace cpp_freertos;

SolarRadiationSensorTask::SolarRadiationSensorTask(const char *taskName, uint16_t stackSize, uint8_t priority, SolarRadiationSensorParam_t solarRadiationSensorParam) : Thread(taskName, stackSize, priority), param(solarRadiationSensorParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(SENSOR_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  // Set analog resolution
  analogReadResolution(ADC_RESOLUTION);

  state = SENSOR_STATE_WAIT_CFG;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void SolarRadiationSensorTask::TaskMonitorStack()
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
void SolarRadiationSensorTask::TaskWatchDog(uint32_t millis_standby)
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
void SolarRadiationSensorTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
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

void SolarRadiationSensorTask::Run() {
  rmapdata_t values_readed_from_sensor[VALUES_TO_READ_FROM_SENSOR_COUNT];
  elaborate_data_t edata;
  // Request response for system queue Task controlled...
  system_message_t system_message;
  
  uint8_t error_count;
  uint8_t retry;
  uint8_t adc_channel;
  int16_t adc_value;
  uint8_t solar_radiation_acquisition_count;
  float value;
  bool is_error;

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
        // Select read chanel first from config
        for(uint8_t idx=0; idx<MAX_ADC_CHANELS; idx++) {
          if(param.configuration->sensors[idx].is_active) {
            // Select first active chanel selected (only one is used on this module)
            adc_channel = idx;
            break;
          }
        }
        solar_radiation_acquisition_count = 0;
        TRACE_VERBOSE_F(F("WAIT -> INIT\r\n"));
        state = SENSOR_STATE_INIT;
      }
      // do something else with non-blocking wait ....
      TaskWatchDog(SOLAR_RADIATION_TASK_WAIT_DELAY_MS);
      Delay(Ticks::MsToTicks(SOLAR_RADIATION_TASK_WAIT_DELAY_MS));
      break;

    case SENSOR_STATE_INIT:
      TRACE_INFO_F(F("Initializing sensors...\r\n"));
      powerOn(adc_channel);
      value = 0;
      solar_radiation_acquisition_count = SAMPLES_REPETED_ADC;
      state = SENSOR_STATE_SET;
      TRACE_INFO_F(F("SENSOR_STATE_INIT --> SENSOR_STATE_SET\r\n"));
      // TODO: NB. STABILIZED TIME FOR SENSOR HERE IF > OF TIME_SLEEP SENSOR POWER FULL ON FOR EVER
      // ON OFF ONLY CIRCUIT COMPARATOR ADC IN

      break;

    case SENSOR_STATE_SET:

      solar_radiation_acquisition_count++;
      value+=(float)analogRead(PIN_ANALOG_01);

      if(solar_radiation_acquisition_count>=SAMPLES_REPETED_ADC)
      {
        state = SENSOR_STATE_EVALUATE;
        break;
      }

      DelayUntil(Ticks::MsToTicks(SENSORS_ACQUISITION_DELAY_MS));
      TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

      TRACE_INFO_F(F("SENSOR_STATE_SET --> SENSOR_STATE_EVALUATE\r\n"));
      break;

    case SENSOR_STATE_EVALUATE:

      // Report to a single acquisition
      value /= SAMPLES_REPETED_ADC;

      value = getAdcCalibratedValue(value, param.configuration->sensors[adc_channel].adc_offset, param.configuration->sensors[adc_channel].adc_gain);
      value = getAdcAnalogValue(value, param.configuration->sensors[adc_channel].adc_type);
      value = getSolarRadiation(value, param.configuration->sensors[adc_channel].analog_min, param.configuration->sensors[adc_channel].analog_max);

      state = SENSOR_STATE_READ;
      TRACE_INFO_F(F("SENSOR_STATE_EVALUATE --> SENSOR_STATE_READ\r\n"));
      break;

    case SENSOR_STATE_READ:
      edata.value = value;
      edata.index = SOLAR_RADIATION_INDEX;
      param.elaborataDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));
      TRACE_INFO_F(F("SENSOR_STATE_READ --> SENSOR_STATE_END\r\n"));
      state = SENSOR_STATE_END;
      break;

    case SENSOR_STATE_END:
      powerOff();

#if (ENABLE_STACK_USAGE)
      TaskMonitorStack();
#endif

      // Local TaskWatchDog update and Sleep Activate before Next Read
      TaskWatchDog(param.configuration->sensor_acquisition_delay_ms);
      TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
      DelayUntil(Ticks::MsToTicks(param.configuration->sensor_acquisition_delay_ms));
      TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

      state = SENSOR_STATE_INIT;
      break;
    } // End Of Switch

  }

}

void SolarRadiationSensorTask::powerOn(uint8_t chanel_out)
{
  if (!is_power_on)
  {
    digitalWrite(PIN_EN_5VA, HIGH);  // Enable Analog Comparator (xIAN)
    if(chanel_out == 0) digitalWrite(PIN_OUT0, HIGH);  // Enable relative Output Chanel alim
    if(chanel_out == 1) digitalWrite(PIN_OUT1, HIGH);  // Enable relative Output Chanel alim
    if(chanel_out == 2) digitalWrite(PIN_OUT2, HIGH);  // Enable relative Output Chanel alim
    if(chanel_out == 3) digitalWrite(PIN_OUT3, HIGH);  // Enable relative Output Chanel alim
    // WDT
    TaskWatchDog(SOLAR_RADIATION_TASK_POWER_ON_WAIT_DELAY_MS);
    Delay(Ticks::MsToTicks(SOLAR_RADIATION_TASK_POWER_ON_WAIT_DELAY_MS));
    is_power_on = true;
  }
}

void SolarRadiationSensorTask::powerOff()
{
  digitalWrite(PIN_EN_5VA, LOW);  // Disable Analog Comparator (xIAN)
  digitalWrite(PIN_OUT0, HIGH);   // Disable Output Chanel alim
  digitalWrite(PIN_OUT1, HIGH);   // Disable Output Chanel alim
  digitalWrite(PIN_OUT2, HIGH);   // Disable Output Chanel alim
  digitalWrite(PIN_OUT3, HIGH);   // Disable Output Chanel alim
  is_power_on = false;
}

int32_t SolarRadiationSensorTask::getVrefTemp(void)
{
  int32_t vRefVoltage = (__LL_ADC_CALC_VREFANALOG_VOLTAGE(analogRead(AVREF), LL_ADC_RESOLUTION));
  return (__LL_ADC_CALC_TEMPERATURE(vRefVoltage, analogRead(ATEMP), LL_ADC_RESOLUTION));
}

float SolarRadiationSensorTask::getAdcCalibratedValue(float adc_value, float offset, float gain)
{
  float value = (float)UINT16_MAX;

  if (!isnan(adc_value) && (adc_value >= ADC_MIN) && (adc_value <= ADC_MAX))
  {
    value = adc_value;
    value += offset;
    value *= gain;
  }

  return value;
}

float SolarRadiationSensorTask::getAdcAnalogValue(float adc_value, Adc_Mode adc_type)
{
  float min, max;
  float value = (float)UINT16_MAX;

  switch (adc_type)
  {
  case Adc_Mode::mVolt:
    min = 0.0;
    max = 3.3;
    /* code */
    break;

  case Adc_Mode::Volt:
    min = 0.0;
    max = 15.0;
    /* code */
    break;

  case Adc_Mode::mA:
    min = 4.0;
    max = 20.0;
    /* code */
    break;

  default:
    break;
  }

  if (!isnan(adc_value))
  {
    value = adc_value;
    value *= (((max - min) / (float)(ADC_MAX)));
    value += min;
  }

  return value;
}

float SolarRadiationSensorTask::getSolarRadiation(float adc_value, float adc_voltage_min, float adc_voltage_max)
{
  float value = adc_value;

  if ((value < (adc_voltage_min + SOLAR_RADIATION_ERROR_VOLTAGE_MIN)) || (value > (adc_voltage_max + SOLAR_RADIATION_ERROR_VOLTAGE_MAX)))
  {
    value = UINT16_MAX;
  }
  else
  {
    value = ((value - adc_voltage_min) / (adc_voltage_max - adc_voltage_min) * SOLAR_RADIATION_MAX);

    if (value <= SOLAR_RADIATION_ERROR_MIN)
    {
        value = SOLAR_RADIATION_MIN;
    }

    if (value >= SOLAR_RADIATION_ERROR_MAX)
    {
        value = SOLAR_RADIATION_MAX;
    }
  }

  return round(value);
}

#endif