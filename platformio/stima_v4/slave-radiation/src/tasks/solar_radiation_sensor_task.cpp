/**@file solar_radiation_sensor_task.cpp */

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

#define TRACE_LEVEL     SOLAR_RADIATION_SENSOR_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   SENSOR_TASK_ID

#include "tasks/solar_radiation_sensor_task.h"

#include "stm32yyxx_ll_adc.h"

/* Values available in datasheet */
#define CALX_TEMP   25
#define VTEMP       760
#define AVG_SLOPE   2500
#define VREFINT     1210

/* Analog read resolution */
#define LL_ADC_RESOLUTION LL_ADC_RESOLUTION_12B
#define ADC_RANGE 4096

static int32_t readTempSensor(int32_t VRef)
{
  return (__LL_ADC_CALC_TEMPERATURE(VRef, analogRead(ATEMP), LL_ADC_RESOLUTION));
}

static int32_t readVref()
{
  return (__LL_ADC_CALC_VREFANALOG_VOLTAGE(analogRead(AVREF), LL_ADC_RESOLUTION));
}

static int32_t readVoltage(int32_t VRef, uint32_t pin)
{
  return (__LL_ADC_CALC_DATA_TO_VOLTAGE(VRef, analogRead(pin), LL_ADC_RESOLUTION));
}

// The loop routine runs over and over again forever:
void loop2() {
  // Print out the value read
  int32_t VRef = readVref();
  Serial.printf("VRef(mv)= %i", VRef);
#ifdef ATEMP
  Serial.printf("\tTemp(°C)= %i", readTempSensor(VRef));
#endif
#ifdef AVBAT
  Serial.printf("\tVbat(mv)= %i", readVoltage(VRef, AVBAT));
#endif
  Serial.printf("\tA0(mv)= %i\n", readVoltage(VRef, A0));
  delay(200);
}

#if (MODULE_TYPE == STIMA_MODULE_TYPE_SOLAR_RADIATION)

using namespace cpp_freertos;

SolarRadiationSensorTask::SolarRadiationSensorTask(const char *taskName, uint16_t stackSize, uint8_t priority, SolarRadiationSensorParam_t solarRadiationSensorParam) : Thread(taskName, stackSize, priority), param(solarRadiationSensorParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(SENSOR_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

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
     param.system_status->tasks->watch_dog = wdt_flag::set;
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
  uint8_t adc_index;
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
        solar_radiation_acquisition_count = 0;
        TRACE_VERBOSE_F(F("WAIT -> INIT\r\n"));
        state = SENSOR_STATE_INIT;
      }
      // other
      else
      {
        // Local WatchDog update;
        TaskWatchDog(SOLAR_RADIATION_TASK_WAIT_DELAY_MS);
        Delay(Ticks::MsToTicks(SOLAR_RADIATION_TASK_WAIT_DELAY_MS));
      }
      // do something else with non-blocking wait ....
      break;

    case SENSOR_STATE_INIT:
      TRACE_INFO_F(F("Initializing sensors...\r\n"));
      powerOn();
      adc_index = 0;
      adc_channel = 0;
      retry = 0;
      is_error = false;
      solar_radiation_acquisition_count++;
      value = FLT_MAX;
      state = SENSOR_STATE_SET;
      SERIAL_TRACE(F("SENSOR_STATE_INIT --> SENSOR_STATE_SET\r\n"));
      break;

    case SENSOR_STATE_SET:
      adc_result = adc1.readSingleChannel(adc_channel, &adc_value);

      if (adc_result == ADC_OK)
      {
        value = (float)(adc_value);
        state = SENSOR_STATE_EVALUATE;
        SERIAL_TRACE(F("SENSOR_STATE_SET --> SENSOR_STATE_EVALUATE\r\n"));
      }
      else if (adc_result == ADC_ERROR)
      {
        i2c_error++;
        value = FLT_MAX;
        is_error = true;
        state = SENSOR_STATE_EVALUATE;
        SERIAL_TRACE(F("SENSOR_STATE_SET --> SENSOR_STATE_EVALUATE\r\n"));
      }
      break;

    case SENSOR_STATE_EVALUATE:
#if (IS_CALIBRATION)
      SERIAL_INFO(F("ADC %u\tAIN%u ==> (%.6f + %.6f) * %.6f = "), adc_index, adc_channel, value, configuration.adc_calibration_offset[adc_index][adc_channel], configuration.adc_calibration_gain[adc_index][adc_channel]);
#endif

      if (!is_error)
      {
        value = getAdcCalibratedValue(value, configuration.adc_calibration_offset[adc_index][adc_channel], configuration.adc_calibration_gain[adc_index][adc_channel]);
        value = getAdcAnalogValue(value, configuration.adc_analog_min[adc_index][adc_channel], configuration.adc_analog_max[adc_index][adc_channel]);
      }

#if (IS_CALIBRATION)
      TRACE_INFO_F(F("%.6f [ %s ]\r\n"), value, is_error ? ERROR_STRING : OK_STRING);
#endif

      if (!is_error)
      {
        value = getSolarRadiation(value, configuration.adc_analog_min[adc_index][adc_channel], configuration.adc_analog_max[adc_index][adc_channel]);
      }

      state = SENSOR_STATE_READ;
      SERIAL_TRACE(F("SENSOR_STATE_EVALUATE --> SENSOR_STATE_READ\r\n"));
      break;

    case SENSOR_STATE_READ:
      edata.value = value;
      edata.index = SOLAR_RADIATION_INDEX;
      param.elaborataDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));
      SERIAL_TRACE(F("SENSOR_STATE_READ --> SENSOR_STATE_END\r\n"));
      state = SENSOR_STATE_END;
      break;

    case SENSOR_STATE_END:
      // spegnimento programmato ongi ACQUISITION_COUNT_FOR_POWER_RESET letture
      if ((solar_radiation_acquisition_count >= ACQUISITION_COUNT_FOR_POWER_RESET) || is_error)
      {
        solar_radiation_acquisition_count = 0;
        powerOff();
      }

#ifdef TH_TASK_LOW_POWER_ENABLED
      powerOff();
#else
      if (error_count > SOLAR_RADIATION_TASK_ERROR_FOR_POWER_OFF)
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

void SolarRadiationSensorTask::powerOn()
{
  if (!is_power_on)
  {
    digitalWrite(PIN_EN_5VS, HIGH);  // Enable + 5VS / +3V3S External Connector Power Sens
    digitalWrite(PIN_EN_SPLY, HIGH); // Enable Supply + 3V3_I2C / + 5V_I2C
    digitalWrite(PIN_I2C2_EN, HIGH); // I2C External Enable PIN (LevelShitf PCA9517D)
    // WDT
    TaskWatchDog(SOLAR_RADIATION_TASK_POWER_ON_WAIT_DELAY_MS);
    Delay(Ticks::MsToTicks(SOLAR_RADIATION_TASK_POWER_ON_WAIT_DELAY_MS));
    is_power_on = true;
  }
}

void SolarRadiationSensorTask::powerOff()
{
  digitalWrite(PIN_EN_5VS, LOW);  // Enable + 5VS / +3V3S External Connector Power Sens
  digitalWrite(PIN_EN_SPLY, LOW); // Enable Supply + 3V3_I2C / + 5V_I2C
  digitalWrite(PIN_I2C2_EN, LOW); // I2C External Enable PIN (LevelShitf PCA9517D)
  is_power_on = false;
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

float SolarRadiationSensorTask::getAdcAnalogValue(float adc_value, float min, float max)
{
  float value = (float)UINT16_MAX;

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