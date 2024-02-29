/**
  ******************************************************************************
  * @file    wind_sensor_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @author  Moreno Gasperini <m.baldinetti@digiteco.it>
  * @brief   wind_sensor_task source file (Module sensor task acquire WindGill)
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
  * <h2><center>All rights reserved.</center></h2>
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
  uint16_t stackUsage = (uint16_t)uxTaskGetStackHighWaterMark( NULL );
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
  // TimeOut mode from millis local timer FullOn on PolledMode or TaskWaiting standard Delay
  #if (WINDSONIC_POLLED_MODE)
  uint32_t millis_timeout;
  #else
  uint16_t check_wait;
  #endif
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
        #if (!WINDSONIC_POLLED_MODE)
        // Reset check time sync Get and Verify Data
        check_wait = 0;
        #endif
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
      error_count = 0;

      if (isWindOff())
      {
        powerOn();
        TaskWatchDog(WIND_POWER_ON_DELAY_MS);
        Delay(Ticks::MsToTicks(WIND_POWER_ON_DELAY_MS));
        // Flushing all data (After powered ON... Clean message startup)
        serialReset();
      }
      #if (WINDSONIC_POLLED_MODE)
      TRACE_VERBOSE_F(F("SENSOR_STATE_INIT --> SENSOR_STATE_SETUP\r\n"));
      state = SENSOR_STATE_SETUP;
      #else
      TRACE_VERBOSE_F(F("SENSOR_STATE_INIT --> SENSOR_STATE_WAIT_DATA\r\n"));
      state = SENSOR_STATE_WAIT_DATA;
      #endif
      break;

    #if (WINDSONIC_POLLED_MODE)
    case SENSOR_STATE_SETUP:
      TRACE_VERBOSE_F(F("SENSOR_STATE_SETUP --> SENSOR_STATE_REQUEST\r\n"));
      state = SENSOR_STATE_REQUEST;
      break;

    case SENSOR_STATE_REQUEST:
    
      /*
        ***************** POLLED MODE, Preopare Request to Wind Sonic ******************
        When in the Polled mode, an output is only generated when the host system sends a Poll 
        signal to the WindSonic consisting of the WindSonic Unit Identifier that is, the relevant 
        letter A - Z.
        The commands available in this mode are:
        Description                       Command            WindSonic response
        WindSonic Unit Identifier         A ..... Z          Wind speed output generated
        Enable Polled mode                ?                  (None)
        Disable Polled mode               !                  (None)
        Request WindSonic Unit Identifier ?&                 A ..... Z (as configured)
        Enter Configuration mode          *<N>               CONFIGURATION MODE
        
        Where <N> is the unit identifier, if used in a multidrop system then it is recommended that 
        ID's A to F and KMNP are not used as these characters can be present in the data string.
        
        It is suggested that in polled mode the following sequence is used for every poll for 
        information.
        ? Ensures that the Sensor is enabled to cover the event that a power down has occurred.
        A-Z Appropriate unit designator sent to retrieve a line of data.
        ! Sent to disable poll mode and reduce possibility of erroneous poll generation.
        
        When in polled mode the system will respond to the data command within 130mS with the 
        last valid data sample as calculated by the Output rate (P Mode Setting).
      */
      // Disable Sleep Mode while request response is done
      LowPower.idleHookDisable();
      // Starting request polling
      SerialWindSonic.print("?Q!\n");
      // Flush and Rest Buffer IN before Reading Messag response
      SerialWindSonic.flush();
      serialReset();
      TRACE_VERBOSE_F(F("SENSOR_STATE_REQUEST --> SENSOR_STATE_WAIT_DATA\r\n"));
      // Start timeout checking from now
      millis_timeout = millis();
      state = SENSOR_STATE_WAIT_DATA;
      break;
    #endif

    // Waiting Data String from Sensor
    case SENSOR_STATE_WAIT_DATA:

      #if (WINDSONIC_POLLED_MODE)

      // Data ready on buffer?
      if(SerialWindSonic.available()) {
        // Got to Read message immediatly (start timeout end message)
        millis_timeout = millis();
        state = SENSOR_STATE_READING;
        break;
      } else {
        // Checking Time OUT End of Buffer IN Data otwerwise exit task without delay
        if(millis() > (millis_timeout + WIND_WAITING_RESPONSE_TIMEOUT_MS)) {
          is_error = true;
          error_count++;
          state = SENSOR_STATE_ELABORATE;
          TRACE_VERBOSE_F(F("SENSOR_STATE_WAIT_DATA (ERROR) --> SENSOR_STATE_ELABORATE\r\n"));
        }        
        else Yield();
      }
      
      #else

      // Ready data from WindSonic?
      if(SerialWindSonic.available()) {
        TRACE_DEBUG_F(F("check_wait ms [ %u ], Avaiable char [ %u ]\r\n"), check_wait * WIND_MESSAGE_DELAY_MS, SerialWindSonic.available());

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

      #endif
      break;

    // Reading data from Serial Buffer to local buffer (in polled or continuos mode)
    case SENSOR_STATE_READING:

      #if (WINDSONIC_POLLED_MODE)

      // Reading data buffer
      if (SerialWindSonic.available()) {
        // Read all buffer existing and waiting next...
        while(SerialWindSonic.available())
          uart_rx_buffer[uart_rx_buffer_ptr++] = SerialWindSonic.read();
        // Update last char reading time out and exit
        millis_timeout = millis();
      } else {
        // Got to Read message
        if(millis() > (millis_timeout + WIND_WAITING_READCHAR_TIMEOUT_MS)) {
          // Set correct pointer last buffer index
          uart_rx_buffer_ptr--;
          state = SENSOR_STATE_ELABORATE;
          TRACE_VERBOSE_F(F("SENSOR_STATE_READING (MESSAGE COMPLETE) --> SENSOR_STATE_ELABORATE\r\n"));
          // Relax flag inibth_sleep. Mesage readed, Now sleep can be activable          
          // Enable Sleep Mode while request response is done
          LowPower.idleHookEnable();
          break;
        }
      }
      // In any case return control immediatly at other task... On Wait
      Yield();

      #else

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

      #endif

      break;

    case SENSOR_STATE_ELABORATE:
      // Return code for signal error to master
      uint8_t windCodeError;
      
      // Any Error (Not Readed?)
      if (is_error)
      {
        windCodeError = -10;  // Add interpreter error No message measure...
      }
      else
      {
        // Get Intepreter data Value from Sensor (Get an error if string not valid)        
        windCodeError = windsonicInterpreter(&speed, &direction);
      }

      // Limit control check
      if(windCodeError) {
        speed = RMAPDATA_MAX;
        direction = RMAPDATA_MAX;
      } else {
        if((speed < MIN_VALID_WIND_SPEED) || (speed > MAX_VALID_WIND_SPEED)) speed = RMAPDATA_MAX;
        if((direction < MIN_VALID_WIND_DIRECTION) || (direction > MAX_VALID_WIND_DIRECTION)) direction = RMAPDATA_MAX;
      }

      #if (!WINDSONIC_POLLED_MODE)
      // Reset In Buffer Serial for next Acquire (Align request immediatly after Interpretate data)
      serialReset();
      #endif

      /// -1 Invalid message, -2 Axis Measurement Error, -3 Status hardware error,
      /// -4 status message error, -5 Unit measure not valid, -6 CRC Error
      param.systemStatusLock->Take();
      param.system_status->events.measure_count++;
      if(windCodeError) {
        param.system_status->events.error_count++;
        if(windCodeError == -2) {
          param.system_status->events.is_windsonic_axis_error = true;
        }
        else if(windCodeError == -3) {
          // Message ROM Error
          param.system_status->events.is_windsonic_hardware_error = true;
        }
        else if(windCodeError == -6) {
          // Message CRC Error
          param.system_status->events.is_windsonic_crc_error = true;
        }
        else if(windCodeError == -5) {
          // Generic Unnown message Error or Format Data Not Valid
          param.system_status->events.is_windsonic_unit_error = true;
        } else {
          // Fake Error code (Init value, Reset to 0 if Measurement OK...) Or Unknown Error or Message Format Error
          param.system_status->events.is_windsonic_responding_error = true;
        }
      } else {
        param.system_status->events.is_windsonic_axis_error = false;
        param.system_status->events.is_windsonic_hardware_error = false;
        param.system_status->events.is_windsonic_crc_error = false;
        param.system_status->events.is_windsonic_unit_error = false;
        param.system_status->events.is_windsonic_responding_error = false;
      }
      param.system_status->events.perc_rs232_error = (uint8_t)
        ((float)(param.system_status->events.error_count / (float)param.system_status->events.measure_count)) * 100.0;
      param.systemStatusLock->Give();

      // Put speed data into queue to elaborate istant value
      if(speed < RMAPDATA_MAX) {
        edata.value = (rmapdata_t)(speed * WIND_CASTING_SPEED_MULT_ACQUIRE);
      } else {
        edata.value = speed;
      }
      edata.index = WIND_SPEED_INDEX;
      param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_PUSHDATA_MS));

      // Put direction data into queue to elaborate istant value
      edata.value = (rmapdata_t)(direction);
      edata.index = WIND_DIRECTION_INDEX;
      param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_PUSHDATA_MS));

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

        #if (WINDSONIC_POLLED_MODE)
        // Synch request DelayUntil (RTOS Timer absolute) to delay_ms programmed on register UAVCAN
        // Using DelayUntil to perform timeOut from Last Event (Synch Time is exacltly internal from Task)
        // TaskWait Until is configured register uavcan param.configuration->sensor_acquisition_delay_ms
        TaskWatchDog(param.configuration->sensor_acquisition_delay_ms);
        TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
        DelayUntil(Ticks::MsToTicks(param.configuration->sensor_acquisition_delay_ms));
        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
        #else
        // Reset check time synch for next verify get data timings
        check_wait = 0;
        // Local TaskWatchDog update and Sleep Activate before Next Read
        // Using Delay to perform timeOut from Now (Synch Time is from external sensor publish message on RS232)
        TaskWatchDog(param.configuration->sensor_acquisition_delay_ms - 70);
        TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
        // Freq. To Acquire depends from Sensor Continuos Mode. To be set to Setup Freq.
        // Delay is NotSync RTOS but Delay from Last Acquire Sensor. Freq. ADD DATA Is from Sensor
        // Need to WakeUp Reading before acquire for get Data from Sensor Complete without loss data.
        // We need about 30 mSec for reading correct next record from Sensor. We use secure time!
        // Time less most possible can set more time for Power_Down. Time WakeUp is already anticipated.
        Delay(Ticks::MsToTicks(param.configuration->sensor_acquisition_delay_ms - 70));
        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
        #endif

        // If error > WIND_TASK_ERROR_FOR_POWER_OFF ( Perform a Reset Power to Sensor )
        // Next Init Reset Sensor Power and wait time to stabilization data
        if (error_count) {
          state = SENSOR_STATE_INIT;
        } else {
          // Goto Read or Request state depending Polled Mode
          #if (WINDSONIC_POLLED_MODE)
          state = SENSOR_STATE_REQUEST;
          #else
          state = SENSOR_STATE_WAIT_DATA;
          #endif
        }
        break;
    }
  }
}

/// @brief Reset buffer serial
void WindSensorTask::serialReset()
{
  while (SerialWindSonic.available()) SerialWindSonic.read();
  memset(uart_rx_buffer, 0, UART_RX_BUFFER_LENGTH);
  uart_rx_buffer_ptr = 0;
}

/// @brief Get power state
/// @return true if power are ON
bool WindSensorTask::isWindOn()
{
  return is_power_on;
}

/// @brief Get power state
/// @return true if power are OFF
bool WindSensorTask::isWindOff()
{
  return (!is_power_on);
}

/// @brief Turn ON Power sensor
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

/// @brief Turn OFF power sensor
void WindSensorTask::powerOff()
{
  digitalWrite(PIN_OUT0, LOW);    // Disable Sensor alim on P.OUT - 0
  is_power_on = false;
}

/// @brief Wind sonic interpreter serial message as -> <0x02>Q,000,000.04,M,00,<0x03>1A␍␊
/// @param speed pointer to returned speed wind value
/// @param direction pointer to returned direction wind value
/// @return 0 is measure is valid (value = check control status, CRC and message composition)
///         -1 Invalid message, -2 Axis Measurement Error, -3 Status hardware error,
///         -4 status message error, -5 Unit measure not valid, -6 CRC Error
uint8_t WindSensorTask::windsonicInterpreter(float *speed, float *direction)
{
  char tempstr[GWS_SPEED_LENGTH + 1];
  char *tempstrptr;
  uint8_t myCrc = 0, chrOffset;
  int crc = 0;
  bool is_crc_ok = false;
  bool bVoidDataMessage = false;
  *speed = RMAPDATA_MAX;
  *direction = RMAPDATA_MAX;

  if ((uart_rx_buffer[GWS_STX_INDEX] == STX_VALUE) && (uart_rx_buffer[GWS_ETX_INDEX] == ETX_VALUE) && (uart_rx_buffer[uart_rx_buffer_ptr - 1] == CR_VALUE) && (uart_rx_buffer[uart_rx_buffer_ptr] == LF_VALUE))
  {
    // Standard complete measure Dir + Wind (no offset required)
    chrOffset = 0;
    memset(tempstr, 0, sizeof(tempstr));
    strncpy(tempstr, (const char *)(uart_rx_buffer + GWS_DIRECTION_INDEX), GWS_DIRECTION_LENGTH);
    *direction = (float)atof(tempstr);
  }
  else if ((uart_rx_buffer[GWS_STX_INDEX] == STX_VALUE) && (uart_rx_buffer[GWS_ETX_INDEX - GWS_WITHOUT_DIRECTION_OFFSET] == ETX_VALUE) && (uart_rx_buffer[uart_rx_buffer_ptr - 1] == CR_VALUE) && (uart_rx_buffer[uart_rx_buffer_ptr] == LF_VALUE))
  {
    // Only wind measure without direction (adding offset)    
    chrOffset = GWS_WITHOUT_DIRECTION_OFFSET;
    *direction = WIND_DIRECTION_MIN;
  }
  else if ((uart_rx_buffer[GWS_STX_INDEX] == STX_VALUE) && (uart_rx_buffer[GWS_ETX_INDEX - GWS_WITHOUT_MEASUREMENT_OFFSET] == ETX_VALUE) && (uart_rx_buffer[uart_rx_buffer_ptr - 1] == CR_VALUE) && (uart_rx_buffer[uart_rx_buffer_ptr] == LF_VALUE))
  {
    // Only message error without wind and direction (adding offset)    
    chrOffset = GWS_WITHOUT_MEASUREMENT_OFFSET;
    bVoidDataMessage = true;
  }
  else
  {
    // Return an error
    TRACE_ERROR_F(F("Windsonic: data read invalid block\r\n"));
    return -1;
  }

  if (!bVoidDataMessage) {
    memset(tempstr, 0, sizeof(tempstr));
    strncpy(tempstr, (const char *)(uart_rx_buffer + GWS_SPEED_INDEX - chrOffset), GWS_SPEED_LENGTH);
    *speed = (float)atof(tempstr);

    if (*speed < CALM_WIND_MAX_MS)
    {
        *speed = WIND_SPEED_MIN;
    }
    else if (*speed > WIND_SPEED_MAX)
    {
        *speed = RMAPDATA_MAX;
    }
  }

  // check status flag
  memset(tempstr, 0, sizeof(tempstr));
  strncpy(tempstr, (const char *)(uart_rx_buffer + GWS_STATUS_INDEX - chrOffset), GWS_STATUS_LENGTH);

  if (strncmp(tempstr, "00", 2)!=0) {
    // Trace known error
    if (strncmp(tempstr, "01", 2)==0){
      TRACE_ERROR_F(F("Windsonic: Axis 1 failed: Insufficient samples in average period on U axis\r\n"));
      return -2;
    }
    else if (strncmp(tempstr, "02", 2)==0){
      TRACE_ERROR_F(F("Windsonic: Axis 2 failed: Insufficient samples in average period on V axis\r\n"));
      return -2;
    }
    else if (strncmp(tempstr, "04", 2)==0){
      TRACE_ERROR_F(F("Windsonic: Axis 1 and 2 failed: Insufficient samples in average period on both axes\r\n"));
      return -2;
    }
    else if (strncmp(tempstr, "08", 2)==0){
      TRACE_ERROR_F(F("Windsonic: NVM error checksum failed\r\n"));
      return -3;
    }
    else if (strncmp(tempstr, "09", 2)==0){
      TRACE_ERROR_F(F("Windsonic: ROM checksum failed\r\n"));
      return -3;
    }
    else
      TRACE_ERROR_F(F("Windsonic: Unknown error, status error in windsonic message\r\n"));
    return -4;
  }

  // Check units (Only "M" m/s is valid)
  memset(tempstr, 0, sizeof(tempstr));
  strncpy(tempstr, (const char *)(uart_rx_buffer + GWS_SPEED_INDEX + GWS_SPEED_LENGTH + 1 - chrOffset), 1);
  if (strncmp(tempstr, "M", 1)!= 0) {
    TRACE_ERROR_F(F("Windsonic: units error in windsonic message\r\n"));
    return -5;
  }

  memset(tempstr, 0, sizeof(tempstr));
  strncpy(tempstr, (const char *)(uart_rx_buffer + GWS_CRC_INDEX - chrOffset), GWS_CRC_LENGTH);
  crc = (uint8_t)strtol(tempstr, &tempstrptr, 16);

  for (uint8_t i = GWS_STX_INDEX + 1; i < (GWS_ETX_INDEX - chrOffset); i++)
  {
      myCrc ^= uart_rx_buffer[i];
  }

  if (*direction < WIND_DIRECTION_MIN)
  {
      *direction = WIND_DIRECTION_MIN;
  }
  else if (*direction > WIND_DIRECTION_MAX)
  {
      *direction = RMAPDATA_MAX;
  }

  is_crc_ok = (crc == myCrc);
  if(!is_crc_ok) {
    TRACE_ERROR_F(F("Windsonic: CRC Error\r\n"));    
  }

  if (!ISVALID_RMAPDATA(*speed) || !ISVALID_RMAPDATA(*direction))
  {
      is_crc_ok = false;
  }

  // Check status and CRC
  if(is_crc_ok) return 0;
  else return -6;
}
#endif
