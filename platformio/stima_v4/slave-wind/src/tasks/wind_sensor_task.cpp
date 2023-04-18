/**
  ******************************************************************************
  * @file    wind_sensor_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @author  Moreno Gasperini <m.baldinetti@digiteco.it>
  * @brief   wind_sensor_task source file (Module sensor task acquire WindGill)
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

  is_power_on = false;

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
     param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
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

  // Start Serial
  SerialWindSonic.begin(9600);

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
        // Reset check time sync Get and Verify Data
        check_wait = 0;
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
      is_error = false;

      if (isWindOff())
      {
        powerOn();
        TaskWatchDog(WIND_POWER_ON_DELAY_MS);
        Delay(Ticks::MsToTicks(WIND_POWER_ON_DELAY_MS));
        // Flushing all data (After powered ON... Clean message startup)
        serialReset();
      }
      TRACE_VERBOSE_F(F("SENSOR_STATE_INIT --> SENSOR_STATE_WAIT_DATA\r\n"));
      state = SENSOR_STATE_WAIT_DATA;
      break;

    case SENSOR_STATE_WAIT_DATA:
      // Ready data from WindSonic?
      if(SerialWindSonic.available()) {
        Serial.print("check_wait? -> ");
        Serial.println(check_wait);
        Serial.print("Avaiable? -> ");
        Serial.println(SerialWindSonic.available());

        TaskWatchDog(WIND_MESSAGE_DELAY_MS);
        Delay(Ticks::MsToTicks(WIND_MESSAGE_DELAY_MS));

        state = SENSOR_STATE_READING;
        break;
      }
      // Max Delay ms to wait a message from WindSonic... (WIND_check_wait_MAX_DELAY_MS)
      else if (++check_wait <= (WIND_RETRY_MAX_DELAY_MS / WIND_MESSAGE_DELAY_MS))
      {
        // Put task in wait for next check
        TaskWatchDog(WIND_MESSAGE_DELAY_MS);
        Delay(Ticks::MsToTicks(WIND_MESSAGE_DELAY_MS));
      }
      else
      {
        is_error = true;
        error_count++;
        state = SENSOR_STATE_ELABORATE;
        TRACE_VERBOSE_F(F("SENSOR_STATE_WAIT_DATA (ERROR) --> SENSOR_STATE_ELABORATE\r\n"));
      }
      break;

    case SENSOR_STATE_READING:
      // Avaiable > MAX LENGHT_MESSAGE ? Normally the message is as long as expected (Align with STX Character)
      // This can occur after a sensor reset or if the set acquisition time does not comply with the continuous acquisition
      // time set on the sensor. It is still to be considered as an error but the procedure works correctly
      if(SerialWindSonic.available() > UART_RX_BUFFER_LENGTH) {
        while (SerialWindSonic.available()) {
          uint8_t alignSTX = SerialWindSonic.read();
          if(alignSTX == STX_VALUE) {
            uart_rx_buffer[uart_rx_buffer_ptr++] = STX_VALUE;
            break;
          }
        }
      }
      // Get all avaiable data (synch to get Full Buffer)
      while (SerialWindSonic.available())
      {
        // Get Buffer Data ( END TO MAX Buffer or LF occurs )
        // The sensor are syncronized with the functionality of the previous alignment
        uart_rx_buffer[uart_rx_buffer_ptr++] = SerialWindSonic.read();
        if((uart_rx_buffer_ptr >= UART_RX_BUFFER_LENGTH) || (uart_rx_buffer[uart_rx_buffer_ptr - 1] == LF_VALUE)) {
          uart_rx_buffer_ptr--;
          state = SENSOR_STATE_ELABORATE;
          TRACE_VERBOSE_F(F("SENSOR_STATE_READING (OK) --> SENSOR_STATE_ELABORATE\r\n"));
          break;
        }
      }
      // Direct exit (Buffer readed control char OK)
      if(state == SENSOR_STATE_ELABORATE) break;
      // Put task in wait for not completed buffer check
      if (++check_wait <= (WIND_RETRY_MAX_DELAY_MS / WIND_MESSAGE_DELAY_MS)) {
        TaskWatchDog(WIND_MESSAGE_DELAY_MS);
        Delay(Ticks::MsToTicks(WIND_MESSAGE_DELAY_MS));
      }
      else
      {
        is_error = true;
        error_count++;
        state = SENSOR_STATE_ELABORATE;
        TRACE_VERBOSE_F(F("SENSOR_STATE_READING (ERROR) --> SENSOR_STATE_ELABORATE\r\n"));
      }
      break;

    case SENSOR_STATE_ELABORATE:
      // Any Error (Not Readed?)
      if (is_error)
      {
        speed = FLT_MAX;
        direction = FLT_MAX;
      }
      else
      {
        // Get Intepreter data Value from Sensor (Get an error if string not valid)
        if(windsonicInterpreter(&speed, &direction))
          error_count = 0;
        else
          error_count++;
      }
      // Reset In Buffer Serial for next Acquire (Align request immediatly after Interpretate data)
      serialReset();

      // Put data into queue to elaborate istant value
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
        #ifdef WIND_TASK_LOW_POWER_ENABLED
        if (error_count > WIND_TASK_ERROR_FOR_POWER_OFF)
        {
          powerOff();
        }
        #endif

        #if (ENABLE_STACK_USAGE)
        TaskMonitorStack();
        #endif

        // Reset check time synch for next verify get data timings
        check_wait = 0;

        // Local TaskWatchDog update and Sleep Activate before Next Read
        TaskWatchDog(param.configuration->sensor_acquisition_delay_ms - 60);
        TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
        // Freq. To Acquire depends from Sensor Continuos Mode. To be set to Setup Freq.
        // Delay is NotSync RTOS but Delay from Last Acquire Sensor. Freq. ADD DATA Is from Sensor
        // Need to WakeUp Reading before acquire for get Data from Sensor Complete without loss data.
        // We need about 30 mSec for reading correct next record from Sensor. We doubling the time!
        // Time less most possible can set more time for Power_Down. Time WakeUp is already anticipated.
        Delay(Ticks::MsToTicks(param.configuration->sensor_acquisition_delay_ms - 60));
        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

        // If error > WIND_TASK_ERROR_FOR_POWER_OFF ( Perform a Reset Power to Sensor )
        // Next Init Reset Sensor Power and wait time to stabilization data
        if (error_count) {
          state = SENSOR_STATE_INIT;
        } else {
          state = SENSOR_STATE_WAIT_DATA;
        }
        break;
    }
  }
}

void WindSensorTask::serialReset()
{
  while (SerialWindSonic.available()) SerialWindSonic.read();
  memset(uart_rx_buffer, 0, UART_RX_BUFFER_LENGTH);
  uart_rx_buffer_ptr = 0;
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
    digitalWrite(PIN_OUT0, HIGH);    // Enable Sensor alim on P.OUT - 0
    // WDT
    TaskWatchDog(WIND_TASK_POWER_ON_WAIT_DELAY_MS);
    Delay(Ticks::MsToTicks(WIND_TASK_POWER_ON_WAIT_DELAY_MS));
    is_power_on = true;
  }
}

void WindSensorTask::powerOff()
{
  digitalWrite(PIN_OUT0, LOW);    // Disable Sensor alim on P.OUT - 0
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

  if ((uart_rx_buffer[GWS_STX_INDEX] == STX_VALUE) && (uart_rx_buffer[GWS_ETX_INDEX] == ETX_VALUE) && (uart_rx_buffer[uart_rx_buffer_ptr - 1] == CR_VALUE) && (uart_rx_buffer[uart_rx_buffer_ptr] == LF_VALUE))
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
  else if ((uart_rx_buffer[GWS_STX_INDEX] == STX_VALUE) && (uart_rx_buffer[GWS_ETX_INDEX - GWS_WITHOUT_DIRECTION_OFFSET] == ETX_VALUE) && (uart_rx_buffer[uart_rx_buffer_ptr - 1] == CR_VALUE) && (uart_rx_buffer[uart_rx_buffer_ptr] == LF_VALUE))
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