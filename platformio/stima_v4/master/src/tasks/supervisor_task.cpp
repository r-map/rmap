/**
 ******************************************************************************
 * @file    supervisor_task.cpp
 * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
 * @author  Moreno Gasperini <m.gasperini@digiteco.it>
 * @brief   Supervisor check connection and config cpp file
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (C) 2022  Moreno Gasperini <m.baldinetti@digiteco.it>
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

#define TRACE_LEVEL     SUPERVISOR_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   SUPERVISOR_TASK_ID

#include "tasks/supervisor_task.h"

using namespace cpp_freertos;

SupervisorTask::SupervisorTask(const char *taskName, uint16_t stackSize, uint8_t priority, SupervisorParam_t supervisorParam) : Thread(taskName, stackSize, priority), param(supervisorParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(SUPERVISOR_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  state = SUPERVISOR_STATE_INIT;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void SupervisorTask::TaskMonitorStack()
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
void SupervisorTask::TaskWatchDog(uint32_t millis_standby)
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
void SupervisorTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
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

// TEST FUNCTION DEBUG
#define TEST_CONNECTION     (false)
#define DISABLE_CONNECTION  (false)

void SupervisorTask::Run()
{
  uint8_t retry, hh, nn, ss;
  uint32_t ms;
  bool bGetNextSecondOffsetStart = false;       // VarSet and Offset for Random connection Start
  int16_t bSecondOffsetStart = 0;
  connection_request_t connection_request;
  connection_response_t connection_response;
  SupervisorConnection_t state_check_connection; // Local state (operation) when module connected

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  while (true)
  {
    bool is_saved = false;
    bool is_loaded = false;

    switch (state)
    {
    case SUPERVISOR_STATE_INIT:
      retry = 0;

      TRACE_VERBOSE_F(F("SUPERVISOR_STATE_INIT -> SUPERVISOR_STATE_LOAD_CONFIGURATION\r\n"));
      state = SUPERVISOR_STATE_LOAD_CONFIGURATION;
      #if (INIT_PARAMETER)
      saveConfiguration(CONFIGURATION_DEFAULT);
      #endif
      break;

    case SUPERVISOR_STATE_LOAD_CONFIGURATION:
      is_loaded = loadConfiguration();

      if (is_loaded)
      {
        retry = 0;

        param.systemStatusLock->Take();
        param.system_status->configuration.is_loaded = true;
        // Init acquire base datetime (for get next)
        param.system_status->datetime.ptr_time_for_sensors_get_istant = 0; // Force get istant at startup display...
        param.system_status->datetime.ptr_time_for_sensors_get_value = rtc.getEpoch() / param.configuration->report_s;
        // Init default security value
        param.system_status->connection.is_disconnected = true;
        param.system_status->connection.is_mqtt_disconnected = true;
        // Start INIT for Clear RPC Queue at first connection MQTT (Only at startup)
        param.system_status->flags.clean_session = true;
        // Check configuration remote is valid
        param.system_status->flags.config_empty = true;
        for(uint8_t iIdx=0; iIdx < BOARDS_COUNT_MAX; iIdx ++) {
          if(param.configuration->board_slave[iIdx].module_type != Module_Type::undefined) {
            param.system_status->flags.config_empty = false;
          }
        }
        param.systemStatusLock->Give();

        #if (FIXED_CONFIGURATION)
        strSafeCopy(param.configuration->gsm_apn, GSM_APN_WIND, GSM_APN_LENGTH);
        strSafeCopy(param.configuration->gsm_number, GSM_NUMBER_WIND, GSM_NUMBER_LENGTH);
        strSafeCopy(param.configuration->mqtt_root_topic, CONFIGURATION_DEFAULT_MQTT_ROOT_TOPIC, MQTT_ROOT_TOPIC_LENGTH);
        // Acquisition from module
        param.configuration->report_s = 900;
        param.configuration->observation_s = 60;
        param.system_status->flags.config_empty = false;
        #endif

        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_LOAD_CONFIGURATION -> SUPERVISOR_STATE_WAITING_EVENT\r\n"));
        state = SUPERVISOR_STATE_WAITING_EVENT;
      }
      else if ((++retry <= SUPERVISOR_TASK_GENERIC_RETRY) && !is_loaded)
      {
        TaskWatchDog(SUPERVISOR_TASK_GENERIC_RETRY_DELAY_MS);
        Delay(Ticks::MsToTicks(SUPERVISOR_TASK_GENERIC_RETRY_DELAY_MS));
      }
      else
      {
        param.systemStatusLock->Take();
        param.system_status->configuration.is_loaded = false;
        param.systemStatusLock->Give();

        // gestire condizione di errore di lettura della configurazione
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_LOAD_CONFIGURATION -> ??? Condizione non gestita!!!\r\n"));
        // Post Suspend TaskWatchDog Module for TASK
        TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
        Suspend();
      }
      break;

    case SUPERVISOR_STATE_WAITING_EVENT:

      // Inibition Sleep on Waiting Event (always false, but TRUE on Starting Event Connection)
      param.systemStatusLock->Take();
      param.system_status->flags.run_connection = false;
      param.systemStatusLock->Give();

      // Retry connection
      retry = 0;

      // Start only modulePower Full OK (no energy rest) Exit on Deep Power Save or Critical mode...
      if(param.system_status->flags.power_state >= Power_Mode::pwr_deep_save) {
        // Sleep continuos TASK if notingh to do
        TaskWatchDog(SUPERVISOR_TASK_SLEEP_DELAY_MS);
        TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
        Delay(Ticks::MsToTicks(SUPERVISOR_TASK_DEEP_POWER_DELAY_MS));
        break;
      } else {
        // Standard Waiting Sleeping mode
        TaskWatchDog(SUPERVISOR_TASK_SLEEP_DELAY_MS);
        TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
        Delay(Ticks::MsToTicks(SUPERVISOR_TASK_SLEEP_DELAY_MS));
        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
      }

      #if (!DISABLE_CONNECTION)

      // Get RND(XX = 60) Second Offset Start connection for avoid overcharging RMAP Server
      if(!bGetNextSecondOffsetStart) {
        bGetNextSecondOffsetStart = true;
        bSecondOffsetStart = random(60);
      }
      // Waiting coutdown on Timing
      if((param.system_status->flags.new_start_connect)&&(bSecondOffsetStart > 0)) {
        bSecondOffsetStart -= (SUPERVISOR_TASK_SLEEP_DELAY_MS / 1000);
        break;
      }

      // Check date RTime for syncro operation (if required)
      rtc.getTime(&hh, &nn, &ss, &ms);

      // With configuration active Time 12.00 - 12.14.59 ( Update NTP 1 x Days ) With connection request start
      if((param.system_status->flags.new_start_connect) &&
        ((hh == 12) && (nn < 15))) {
        param.systemStatusLock->Take();
        param.system_status->command.do_ntp_synchronization = true;
        param.systemStatusLock->Give();
      }

      #if (!TEST_CONNECTION)
      // Checking starting Connection inibition next Start... If something Wrong... (Default 10 min)
      // RPC Request Command as (configure or download firmware) will start immediatly
      if(((rtc.getEpoch() - param.system_status->data_master.connect_run_epoch) > MIN_INIBITH_CONNECT_RETRY_S)||
          (param.system_status->command.do_http_configuration_update)||
          (param.system_status->command.do_http_firmware_download)) {
        // ? External or internal request command strart connection
        // New data to send are syncronized with add new data (report_s time at data acquire)...
        // If config_empty ( Start connection every hour... )
        if((param.system_status->flags.new_start_connect) ||
            (param.system_status->command.do_ntp_synchronization) ||
            (param.system_status->command.do_http_configuration_update) ||
            (param.system_status->command.do_http_firmware_download) ||
            (param.system_status->command.do_mqtt_connect)) {
          // ? Request external command OR Time to Run Connection?
          // First connection? Update Time from NTP
          if(param.system_status->modem.connection_attempted == 0) {
            param.systemStatusLock->Take();
            param.system_status->command.do_ntp_synchronization = true;
            param.systemStatusLock->Give();
          }

          // Remove request connect. Now Started.
          param.system_status->flags.new_start_connect = false;
          // Renew GET Random value for Start Next Connection
          bGetNextSecondOffsetStart = false;

          // START REQUEST function LIST...
          param.systemStatusLock->Take();
          param.system_status->connection.is_ntp_synchronized = !param.system_status->command.do_ntp_synchronization;
          param.system_status->connection.is_http_configuration_updated = !param.system_status->command.do_http_configuration_update;
          param.system_status->connection.is_http_firmware_upgraded = !param.system_status->command.do_http_firmware_download;
          // is_mqtt_connected, Always enabled MQTT Connection
          param.system_status->connection.is_mqtt_connected = false;
          // Saving starting Connection to inibith next Connection... If something Wrong...
          param.system_status->data_master.connect_run_epoch = rtc.getEpoch();
          // Resetting command request (Not event setted but external request)
          param.system_status->command.do_http_configuration_update = false;
          param.system_status->command.do_http_firmware_download = false;
          param.system_status->command.do_mqtt_connect = false;
          param.system_status->command.do_ntp_synchronization = false;

          if(param.system_status->modem.connection_attempted) {
            param.system_status->modem.perc_modem_connection_valid = (uint8_t)
              (((float)(param.system_status->modem.connection_completed / (float)param.system_status->modem.connection_attempted)) * 100.0);
          } else {
            param.system_status->modem.perc_modem_connection_valid = 100;
          }
          // Save next attempt of connection
          param.system_status->modem.connection_attempted++;
          param.systemStatusLock->Give();

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_WAITING_EVENT -> SUPERVISOR_STATE_CONNECTION_OPERATION\r\n"));
          TRACE_VERBOSE_F(F("Attempted: [ %d ] , Completed: [ %d ]\r\n"),
            param.system_status->modem.connection_attempted, param.system_status->modem.connection_completed);

          // Inibition Sleep on Waiting Event (always false, but TRUE on Starting Event Connection)
          param.system_status->flags.run_connection = true;

          // Start state check connection
          state_check_connection = CONNECTION_INIT;
          state = SUPERVISOR_STATE_CONNECTION_OPERATION;

        }
      }

      #else

      // ***** ONLY TEST CONNECTION *****

      // START REQUEST LIST...
      param.systemStatusLock->Take();
      param.system_status->connection.is_ntp_synchronized = false;
      param.system_status->connection.is_http_configuration_updated = false;
      param.system_status->connection.is_http_firmware_upgraded = false;
      param.system_status->connection.is_mqtt_connected = false;
      param.systemStatusLock->Give();

      // Start state check connection
      state_check_connection = CONNECTION_INIT;
      state = SUPERVISOR_STATE_CONNECTION_OPERATION;

      // Update percentage good connection vs attemtped
      param.systemStatusLock->Take();
      if(param.system_status->modem.connection_attempted) {
        param.system_status->modem.perc_modem_connection_valid = (uint8_t)
          (((float)(param.system_status->modem.connection_completed / (float)param.system_status->modem.connection_attempted)) * 100.0);
      } else {
        param.system_status->modem.perc_modem_connection_valid = 100;
      }
      // Save next attempt of connection
      param.system_status->modem.connection_attempted++;
      TRACE_VERBOSE_F(F("SUPERVISOR_STATE_WAITING_EVENT -> SUPERVISOR_STATE_CONNECTION_OPERATION\r\n"));
      TRACE_VERBOSE_F(F("Attempted: [ %d ] , Completed: [ %d ]\r\n"),
        param.system_status->modem.connection_attempted, param.system_status->modem.connection_completed);
      // Inibition Sleep on Waiting Event (always false, but TRUE on Starting Event Connection)
      param.system_status->flags.run_connection = true;
      param.systemStatusLock->Give();

      #endif

      #endif

      break;

    case SUPERVISOR_STATE_CONNECTION_OPERATION:

      // Here config was already loaded

      // ERROR DNS Resolving condition. Need to restart connection. Connection apn not estabilished correctly and modem state must to reset.
      if(param.system_status->connection.is_dns_failed_resolve) {
          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_REQUEST_DISCONNECTION\r\n"));
          // Init retry disconnection
          retry = 0;
          // Exit immediatly from the switch (no more action)
          state = SUPERVISOR_STATE_REQUEST_DISCONNECTION;
          // Remove error flag DNS
          param.systemStatusLock->Take();
          param.system_status->connection.is_dns_failed_resolve = false;
          param.systemStatusLock->Give();
          break;
      }

      // SUB Case of sequence of check (connection / operation) state
      switch(state_check_connection) {

        case CONNECTION_INIT: // STARTING CONNECTION
          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_REQUEST_CONNECTION\r\n"));
          state = SUPERVISOR_STATE_REQUEST_CONNECTION;
          state_check_connection = CONNECTION_CHECK;
          break;

        case CONNECTION_CHECK: // CONNECTION VERIFY
          if (!param.system_status->connection.is_connected) // Ready Connected ?
          {
            TRACE_VERBOSE_F(F("SUPERVISOR: Connection not ready\r\n"));
            TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_END\r\n"));
            // Exit from the switch (no more action)
            state = SUPERVISOR_STATE_END;
            break;
          }
          // Prepare next state controller
          state_check_connection = CONNECTION_CHECK_NTP;
          break;

        case CONNECTION_CHECK_NTP: // NTP_SYNCRO (NTP)
          if (!param.system_status->connection.is_ntp_synchronized) // Already Syncronized?
          {
            // Request ntp sync
            memset(&connection_request, 0, sizeof(connection_request_t));
            connection_request.do_ntp_sync = true;
            param.connectionRequestQueue->Enqueue(&connection_request);

            TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_DO_NTP\r\n"));
            state = SUPERVISOR_STATE_DO_NTP;
          }
          // Prepare next state controller
          state_check_connection = CONNECTION_CHECK_HTTP;
          break;

        case CONNECTION_CHECK_HTTP: // READ_CONFIG, FIRMWARE (HTTP)
          if (!param.system_status->connection.is_http_configuration_updated) // Already Configured?
          {
            TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_REQUEST_CONNECTION\r\n"));

            // Request configuration update by http request
            memset(&connection_request, 0, sizeof(connection_request_t));
            connection_request.do_http_get_configuration = true;
            param.connectionRequestQueue->Enqueue(&connection_request);

            TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_DO_HTTP\r\n"));
            state = SUPERVISOR_STATE_DO_HTTP;
          }
          else if (!param.system_status->connection.is_http_firmware_upgraded)
          {
            TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_REQUEST_CONNECTION\r\n"));

            // Request firmware download by http request
            memset(&connection_request, 0, sizeof(connection_request_t));
            connection_request.do_http_get_firmware = true;
            param.connectionRequestQueue->Enqueue(&connection_request);

            TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_DO_HTTP\r\n"));
            state = SUPERVISOR_STATE_DO_HTTP;
          }
          // Prepare next state controller
          if (param.system_status->connection.is_mqtt_connected) // Mqtt have executed connect for this session? (False = require connect)
          {
            // No more action. Mqtt is not required (Also connected or dropped)
            param.system_status->connection.is_mqtt_disconnected = true;
            state_check_connection = CONNECTION_END;
            break;
          }
          state_check_connection = CONNECTION_CHECK_MQTT;
          break;

        case CONNECTION_CHECK_MQTT: // Rmap Publish data (MQTT)
          if (!param.system_status->connection.is_mqtt_connected) // Mqtt connected?
          {
            // Request mqtt connection
            memset(&connection_request, 0, sizeof(connection_request_t));
            connection_request.do_mqtt_connect = true;
            param.connectionRequestQueue->Enqueue(&connection_request);

            TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_DO_MQTT\r\n"));
            state = SUPERVISOR_STATE_DO_MQTT;
          }
          // Prepare next state controller
          state_check_connection = CONNECTION_END;
          break;

        case CONNECTION_END: // Publish completed? Or End Connection Mqtt -> END
          if(param.system_status->connection.is_mqtt_disconnected)
          {
            if(param.system_status->connection.is_mqtt_publishing_end) {
              TRACE_VERBOSE_F(F("SUPERVISOR: Publish data [ %s ] from MQTT\r\n"), OK_STRING);
            } else {
              TRACE_VERBOSE_F(F("SUPERVISOR: Publish data [ %s ] from MQTT\r\n"), ERROR_STRING);
            }
            // Remove Flag for Start Next Publish and Connect for Next attempt
            param.systemStatusLock->Take();
            param.system_status->connection.is_mqtt_publishing_end = false;
            param.systemStatusLock->Give();
            TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_REQUEST_DISCONNECTION\r\n"));
            // Init retry disconnection
            retry = 0;
            // Exit from the switch (no more action)
            state = SUPERVISOR_STATE_REQUEST_DISCONNECTION;
            // Saving connection sequence completed
            param.systemStatusLock->Take();
            // Clear inibith time. Next Send can start immediatly
            param.system_status->data_master.connect_run_epoch = 0;
            param.system_status->modem.connection_completed++;
            param.systemStatusLock->Give();
          }
          // No action (waiting END, error or disconnet)
          break;
      }
      break;

    case SUPERVISOR_STATE_SAVE_CONFIGURATION:
      is_saved = saveConfiguration(CONFIGURATION_CURRENT);

      if (is_saved)
      {
        retry = 0;

        param.systemStatusLock->Take();
        param.system_status->configuration.is_saved = true;
        param.system_status->configuration.is_loaded = true;
        param.systemStatusLock->Give();

        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_SAVE_CONFIGURATION -> SUPERVISOR_STATE_CONNECTION_OPERATION\r\n"));
        state = SUPERVISOR_STATE_CONNECTION_OPERATION;
      }
      else if ((++retry <= SUPERVISOR_TASK_GENERIC_RETRY) && !is_saved)
      {
        TaskWatchDog(SUPERVISOR_TASK_GENERIC_RETRY_DELAY_MS);
        Delay(Ticks::MsToTicks(SUPERVISOR_TASK_GENERIC_RETRY_DELAY_MS));
      }
      else
      {
        param.systemStatusLock->Take();
        param.system_status->configuration.is_saved = false;
        param.system_status->configuration.is_loaded = false;
        param.systemStatusLock->Give();

        // gestire condizione di errore di scrittura della configurazione
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_SAVE_CONFIGURATION -> ??? Condizione non gestita!!!\r\n"));
        // Post Suspend TaskWatchDog Module for TASK
        TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
        Suspend();
      }
      break;

    case SUPERVISOR_STATE_REQUEST_CONNECTION:    
      // already connected
      if (param.system_status->connection.is_connected)
      {
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_REQUEST_CONNECTION -> SUPERVISOR_STATE_CONNECTION_OPERATION\r\n"));
        state = SUPERVISOR_STATE_CONNECTION_OPERATION;
      }
      // not connected -> request connection.
      else
      {
        param.systemStatusLock->Take();
        param.system_status->connection.is_connecting = true;
        param.systemStatusLock->Give();

        memset(&connection_request, 0, sizeof(connection_request_t));
        connection_request.do_connect = true;                
        param.connectionRequestQueue->Enqueue(&connection_request);

        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_REQUEST_CONNECTION -> SUPERVISOR_STATE_CHECK_CONNECTION\r\n"));
        state = SUPERVISOR_STATE_CHECK_CONNECTION;
      }
      break;
    
    case SUPERVISOR_STATE_CHECK_CONNECTION:
      // wait connection
      // Suspend TASK Controller for queue waiting portMAX_DELAY
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
      if (param.connectionResponseQueue->Peek(&connection_response, portMAX_DELAY))
      {
        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);      
        // ok connected
        if (connection_response.done_connected)
        {
          param.connectionResponseQueue->Dequeue(&connection_response);
          param.systemStatusLock->Take();
          param.system_status->connection.is_connected = true;
          param.system_status->connection.is_connecting = false;
          param.system_status->connection.is_disconnecting = false;
          param.system_status->connection.is_disconnected = false;
          param.systemStatusLock->Give();
          TRACE_INFO_F(F("%s Connection [ %s ]\r\n"), Thread::GetName().c_str(), OK_STRING);

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_CONNECTION -> SUPERVISOR_STATE_CONNECTION_OPERATION\r\n"));
          state = SUPERVISOR_STATE_CONNECTION_OPERATION;
        }
        // Error connection?
        else if (connection_response.error_connected) {
          retry++; // Add error retry
          param.connectionResponseQueue->Dequeue(&connection_response);
          param.systemStatusLock->Take();
          param.system_status->connection.is_connected = false;
          param.system_status->connection.is_connecting = false;
          param.system_status->connection.is_disconnecting = false;
          param.system_status->connection.is_disconnected = true;
          param.systemStatusLock->Give();
          TRACE_ERROR_F(F("%s Connection [ %s ], retry remaining [ %d ]\r\n"), Thread::GetName().c_str(), ERROR_STRING, SUPERVISOR_TASK_GENERIC_RETRY - retry);
          if (retry < SUPERVISOR_TASK_GENERIC_RETRY) // Check Retry
          {
            TaskWatchDog(SUPERVISOR_TASK_GENERIC_RETRY_DELAY_MS);
            Delay(Ticks::MsToTicks(SUPERVISOR_TASK_GENERIC_RETRY_DELAY_MS));
            // Try to powerOff Modem other times
            TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_CONNECTION -> SUPERVISOR_STATE_REQUEST_CONNECTION\r\n"));
            state = SUPERVISOR_STATE_REQUEST_CONNECTION;
          } else {
            // Return state to CONNECTION_OPERATION (next switch control operation)
            // Retry, abort connection on check_connection estabilished (->not connected !!!)
            TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_CONNECTION -> SUPERVISOR_STATE_CONNECTION_OPERATION\r\n"));
            state = SUPERVISOR_STATE_CONNECTION_OPERATION;
          }
        }
      }
      break;

    case SUPERVISOR_STATE_DO_NTP:
      // wait ntp to be sync
      // Suspend TASK Controller for queue waiting portMAX_DELAY
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);      
      if (param.connectionResponseQueue->Peek(&connection_response, portMAX_DELAY))
      {
        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);      
        // ok ntp synchronized
        if (connection_response.done_ntp_synchronized)
        {
          param.connectionResponseQueue->Dequeue(&connection_response);
          TRACE_INFO_F(F("%s NTP synchronization [ %s ]\r\n"), Thread::GetName().c_str(), OK_STRING);

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_DO_NTP -> SUPERVISOR_STATE_CONNECTION_OPERATION\r\n"));
          state = SUPERVISOR_STATE_CONNECTION_OPERATION;
        }
        // error: ntp syncronized
        else if (connection_response.error_ntp_synchronized)
        {
          param.connectionResponseQueue->Dequeue(&connection_response);
          TRACE_ERROR_F(F("%s NTP synchronization [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_DO_NTP -> SUPERVISOR_STATE_CONNECTION_OPERATION\r\n"));
          state = SUPERVISOR_STATE_CONNECTION_OPERATION;
        }
      }
      break;

    case SUPERVISOR_STATE_DO_HTTP:
      // wait http to be connected
      // Suspend TASK Controller for queue waiting portMAX_DELAY
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);      
      if (param.connectionResponseQueue->Peek(&connection_response, portMAX_DELAY))
      {
        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);      
        // ok http gettet configuration or firmware
        if (connection_response.done_http_configuration_getted || connection_response.done_http_firmware_getted)
        {
          param.connectionResponseQueue->Dequeue(&connection_response);
          if(connection_response.done_http_configuration_getted) {
            TRACE_INFO_F(F("%s HTTP connected [ get config ] [ %s ]\r\n"), Thread::GetName().c_str(), OK_STRING);
          }
          if(connection_response.done_http_firmware_getted) {
            TRACE_INFO_F(F("%s HTTP connected [ get firmware ] [ %s ]\r\n"), Thread::GetName().c_str(), OK_STRING);
          }

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_DO_HTTP -> SUPERVISOR_STATE_CONNECTION_OPERATION\r\n"));
          state = SUPERVISOR_STATE_CONNECTION_OPERATION;
        }
        // error: http not gettet configuration or firmware
        else if (connection_response.error_http_configuration_getted || connection_response.error_http_firmware_getted)
        {
          param.connectionResponseQueue->Dequeue(&connection_response);
          if(connection_response.error_http_configuration_getted) {
            TRACE_INFO_F(F("%s HTTP request [ get config ] [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
          }
          if(connection_response.error_http_firmware_getted) {
            TRACE_INFO_F(F("%s HTTP request [ get firmware ] [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
          }

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_DO_HTTP -> SUPERVISOR_STATE_CONNECTION_OPERATION\r\n"));
          state = SUPERVISOR_STATE_CONNECTION_OPERATION;
        }
      }
      break;

    case SUPERVISOR_STATE_DO_MQTT:
      // wait mqtt to be connected
      // Suspend TASK Controller for queue waiting portMAX_DELAY
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);      
      if (param.connectionResponseQueue->Peek(&connection_response, portMAX_DELAY))
      {
        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);      
        // ok mqtt connected
        if (connection_response.done_mqtt_connected)
        {
          param.connectionResponseQueue->Dequeue(&connection_response);
          TRACE_INFO_F(F("%s MQTT connected [ %s ]\r\n"), Thread::GetName().c_str(), OK_STRING);

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_DO_MQTT -> SUPERVISOR_STATE_CONNECTION_OPERATION\r\n"));
          state = SUPERVISOR_STATE_CONNECTION_OPERATION;
        }
        // error: not connected
        else if (connection_response.error_mqtt_connected)
        {
          param.connectionResponseQueue->Dequeue(&connection_response);
          TRACE_ERROR_F(F("%s MQTT connection [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_DO_MQTT -> SUPERVISOR_STATE_CONNECTION_OPERATION\r\n"));
          state = SUPERVISOR_STATE_CONNECTION_OPERATION;
        }
      }
      break;

    case SUPERVISOR_STATE_REQUEST_DISCONNECTION:
      // already disconnected
      if (param.system_status->connection.is_disconnected)
      {
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_REQUEST_DISCONNECTION -> SUPERVISOR_STATE_WAITING_EVENT\r\n"));
        state = SUPERVISOR_STATE_WAITING_EVENT;
      }
      // connected -> request disconnection.
      else
      {
        param.systemStatusLock->Take();
        param.system_status->connection.is_disconnecting = true;
        param.systemStatusLock->Give();

        memset(&connection_request, 0, sizeof(connection_request_t));
        connection_request.do_disconnect = true;
        param.connectionRequestQueue->Enqueue(&connection_request);

        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_REQUEST_DISCONNECTION -> SUPERVISOR_STATE_CHECK_DISCONNECTION\r\n"));
        state = SUPERVISOR_STATE_CHECK_DISCONNECTION;
      }
      break;

    case SUPERVISOR_STATE_CHECK_DISCONNECTION:
      // wait connection
      // Suspend TASK Controller for queue waiting portMAX_DELAY
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);      
      if (param.connectionResponseQueue->Peek(&connection_response, portMAX_DELAY)) {
        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);      
        // ok disconnected
        if (connection_response.done_disconnected)
        {
          param.connectionResponseQueue->Dequeue(&connection_response);
          param.systemStatusLock->Take();
          param.system_status->connection.is_connected = false;
          param.system_status->connection.is_connecting = false;
          param.system_status->connection.is_disconnected = true;
          param.system_status->connection.is_disconnecting = false;
          param.systemStatusLock->Give();
          TRACE_INFO_F(F("%s Disconnection [ %s ]\r\n"), Thread::GetName().c_str(), OK_STRING);

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_DISCONNECTION -> SUPERVISOR_STATE_END\r\n"));
          state = SUPERVISOR_STATE_END;
          break;
        }
        // else if (++retry <= SUPERVISOR_TASK_GENERIC_RETRY) // Check Retry
        // {
        //   TaskWatchDog(param.system_status, param.systemStatusLock, SUPERVISOR_TASK_GENERIC_RETRY_DELAY_MS, false);
        //   Delay(Ticks::MsToTicks(SUPERVISOR_TASK_GENERIC_RETRY_DELAY_MS));
        //   // Try to powerOff Modem other times
        //   TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_DISCONNECTION -> SUPERVISOR_STATE_REQUEST_DISCONNECTION\r\n"));
        //   state = SUPERVISOR_STATE_REQUEST_DISCONNECTION;
        // } else {
        //   // Disconnection impossible, perform a Reboot
        //   // Signal to E2Prom the problem
        //   NVIC_SystemReset();
        // }
      }
      break;

    case SUPERVISOR_STATE_END:
      TRACE_VERBOSE_F(F("SUPERVISOR_STATE_END -> SUPERVISOR_STATE_WAITING_EVENT\r\n"));
      state = SUPERVISOR_STATE_WAITING_EVENT;
      break;
    }

    #if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
    #endif

    // One step base non blocking switch
    TaskWatchDog(SUPERVISOR_TASK_WAIT_DELAY_MS);
    Delay(Ticks::MsToTicks(SUPERVISOR_TASK_WAIT_DELAY_MS));

  }
}

/// @brief Load configuration base from E2
/// @return true if loading OK
bool SupervisorTask::loadConfiguration()
{
  // Private param and Semaphore: param.configuration, param.configurationLock
  bool status = true;

  //! read configuration from eeprom
  if (param.configurationLock->Take())
  {
    status = param.eeprom->Read(CONFIGURATION_EEPROM_ADDRESS, (uint8_t *)(param.configuration), sizeof(configuration_t));
    param.configurationLock->Give();
  }

  // Always FIX configuration eeprom saved paramtere with FIRMWARE fixed parameter
  param.configuration->module_main_version = MODULE_MAIN_VERSION;
  param.configuration->module_minor_version = MODULE_MINOR_VERSION;
  param.configuration->module_type = (Module_Type)MODULE_TYPE;

  #if (INIT_PARAMETER)
  status = saveConfiguration(CONFIGURATION_DEFAULT);
  #else
  if (param.configuration->module_type != MODULE_TYPE || param.configuration->module_main_version != MODULE_MAIN_VERSION)
  {
    status = saveConfiguration(CONFIGURATION_DEFAULT);
  }
  #endif

  TRACE_INFO_F(F("Load configuration... [ %s ]\r\n"), status ? OK_STRING : ERROR_STRING);
  printConfiguration();

  return status;
}

/// @brief Trace print current configuration from param
void SupervisorTask::printConfiguration()
{
  // Private param and Semaphore: param.configuration, param.configurationLock
  if (param.configurationLock->Take()) {
    char stima_name[STIMA_MODULE_NAME_LENGTH];
    char topic[MQTT_ROOT_TOPIC_LENGTH];

    getStimaNameByType(stima_name, param.configuration->module_type);
    TRACE_INFO_F(F("-> type: %s\r\n"), stima_name);
    TRACE_INFO_F(F("-> main version: %u\r\n"), param.configuration->module_main_version);
    TRACE_INFO_F(F("-> minor version: %u\r\n"), param.configuration->module_minor_version);
    TRACE_INFO_F(F("-> constant data: %d\r\n"), param.configuration->constantdata_count);
    
    for (uint8_t i = 0; i < param.configuration->constantdata_count; i++)
    {
      TRACE_INFO_F(F("--> cd %d:/t%s : %s"), i, param.configuration->constantdata[i].btable, param.configuration->constantdata[i].value);
    }

    // TRACE_INFO_F(F("-> %u configured sensors:\r\n"), configuration->sensors_count);

    // for (uint8_t i = 0; i < configuration->sensors_count; i++)
    // {
    //   TRACE_INFO_F(F("--> %u: %s-%s\r\n"), i + 1, configuration->sensors[i].driver, configuration->sensors[i].type);
    // }

    TRACE_INFO_F(F("-> data report every %d seconds\r\n"), param.configuration->report_s);
    TRACE_INFO_F(F("-> data observation every %d seconds\r\n"), param.configuration->observation_s);
    // TRACE_INFO_F(F("-> data level: %s\r\n"), param.configuration->data_level);
    TRACE_INFO_F(F("-> ident: %s\r\n"), param.configuration->ident);
    TRACE_INFO_F(F("-> longitude %07d and latitude %07d\r\n"), param.configuration->longitude, param.configuration->latitude);
    TRACE_INFO_F(F("-> network: %s\r\n"), param.configuration->network);

    #if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_ETH)
    TRACE_INFO_F(F("-> dhcp: %s\r\n"), configuration->is_dhcp_enable ? "on" : "off");
    TRACE_INFO_F(F("-> ethernet mac: %02X:%02X:%02X:%02X:%02X:%02X\r\n"), configuration->ethernet_mac[0], configuration->ethernet_mac[1], configuration->ethernet_mac[2], configuration->ethernet_mac[3], configuration->ethernet_mac[4], configuration->ethernet_mac[5]);
    #endif

    #if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
    TRACE_INFO_F(F("-> gsm apn: %s\r\n"), param.configuration->gsm_apn);
    TRACE_INFO_F(F("-> gsm number: %s\r\n"), param.configuration->gsm_number);
    TRACE_INFO_F(F("-> gsm username: %s\r\n"), param.configuration->gsm_username);
    TRACE_INFO_F(F("-> gsm password: %s\r\n"), param.configuration->gsm_password);
    #endif

    #if (USE_NTP)
    TRACE_INFO_F(F("-> ntp server: %s\r\n"), param.configuration->ntp_server);
    #endif

    #if (USE_MQTT)
    TRACE_INFO_F(F("-> mqtt server: %s\r\n"), param.configuration->mqtt_server);
    TRACE_INFO_F(F("-> mqtt port: %d\r\n"), param.configuration->mqtt_port);    
    TRACE_INFO_F(F("-> mqtt username: %s\r\n"), param.configuration->mqtt_username);
    TRACE_INFO_F(F("-> mqtt password: %s\r\n"), param.configuration->mqtt_password);
    TRACE_INFO_F(F("-> station slug: %s\r\n"), param.configuration->stationslug);
    TRACE_INFO_F(F("-> board slug: %s\r\n"), param.configuration->board_master.boardslug);
    TRACE_INFO_F(F("-> client psk key "));
    TRACE_INFO_ARRAY("", param.configuration->client_psk_key, CLIENT_PSK_KEY_LENGTH);    
    TRACE_INFO_F(F("-> mqtt root topic: %s/%s/%s/%07d,%07d/%s/\r\n"), 
      param.configuration->mqtt_root_topic, param.configuration->mqtt_username, param.configuration->ident,
      param.configuration->longitude, param.configuration->latitude, param.configuration->network);
    TRACE_INFO_F(F("-> mqtt maint topic: %s/%s/%s/%07d,%07d/%s/\r\n"),
      param.configuration->mqtt_maint_topic, param.configuration->mqtt_username, param.configuration->ident,
      param.configuration->longitude, param.configuration->latitude, param.configuration->network);
    TRACE_INFO_F(F("-> mqtt rpc topic: %s/%s/%s/%07d,%07d/%s/\r\n"),
      param.configuration->mqtt_rpc_topic, param.configuration->mqtt_username, param.configuration->ident,
      param.configuration->longitude, param.configuration->latitude, param.configuration->network);
#endif

    param.configurationLock->Give();
  }
}

/// @brief Save configuration to E2
/// @param is_default require to write the Default configuration
/// @return true is saving is done
bool SupervisorTask::saveConfiguration(bool is_default)
{
  // Private param and Semaphore: param.configuration, param.configurationLock
  bool status = true;

  if (param.configurationLock->Take())
  {
    if (is_default)
    {
      osMemset(param.configuration, 0, sizeof(configuration_t));

      param.configuration->module_main_version = MODULE_MAIN_VERSION;
      param.configuration->module_minor_version = MODULE_MINOR_VERSION;
      param.configuration->module_type = (Module_Type)MODULE_TYPE;

      param.configuration->observation_s = CONFIGURATION_DEFAULT_OBSERVATION_S;
      param.configuration->report_s = CONFIGURATION_DEFAULT_REPORT_S;

      strSafeCopy(param.configuration->ident, CONFIGURATION_DEFAULT_IDENT, IDENT_LENGTH);
      strSafeCopy(param.configuration->network, CONFIGURATION_DEFAULT_DATA_LEVEL, DATA_LEVEL_LENGTH);
      strSafeCopy(param.configuration->network, CONFIGURATION_DEFAULT_NETWORK, NETWORK_LENGTH);

      param.configuration->latitude = CONFIGURATION_DEFAULT_LATITUDE;
      param.configuration->longitude = CONFIGURATION_DEFAULT_LONGITUDE;

      #if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_ETH)
      char temp_string[20];
      param.configuration->is_dhcp_enable = CONFIGURATION_DEFAULT_ETHERNET_DHCP_ENABLE;
      strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_MAC);
      macStringToArray(param.configuration->ethernet_mac, temp_string);
      strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_IP);
      ipStringToArray(param.configuration->ip, temp_string);
      strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_NETMASK);
      ipStringToArray(param.configuration->netmask, temp_string);
      strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_GATEWAY);
      ipStringToArray(param.configuration->gateway, temp_string);
      strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_PRIMARY_DNS);
      ipStringToArray(param.configuration->primary_dns, temp_string);
      #endif

      #if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
      strSafeCopy(param.configuration->gsm_apn, CONFIGURATION_DEFAULT_GSM_APN, GSM_APN_LENGTH);
      strSafeCopy(param.configuration->gsm_number, CONFIGURATION_DEFAULT_GSM_NUMBER, GSM_NUMBER_LENGTH);
      strSafeCopy(param.configuration->gsm_username, CONFIGURATION_DEFAULT_GSM_USERNAME, GSM_USERNAME_LENGTH);
      strSafeCopy(param.configuration->gsm_password, CONFIGURATION_DEFAULT_GSM_PASSWORD, GSM_PASSWORD_LENGTH);
      #endif

      #if (USE_NTP)
      strSafeCopy(param.configuration->ntp_server, CONFIGURATION_DEFAULT_NTP_SERVER, NTP_SERVER_LENGTH);
      #endif

      #if (USE_MQTT)
      param.configuration->mqtt_port = CONFIGURATION_DEFAULT_MQTT_PORT;
      strSafeCopy(param.configuration->mqtt_server, CONFIGURATION_DEFAULT_MQTT_SERVER, MQTT_SERVER_LENGTH);
      strSafeCopy(param.configuration->mqtt_username, CONFIGURATION_DEFAULT_MQTT_USERNAME, MQTT_USERNAME_LENGTH);
      strSafeCopy(param.configuration->mqtt_password, CONFIGURATION_DEFAULT_MQTT_PASSWORD, MQTT_PASSWORD_LENGTH);
      strSafeCopy(param.configuration->mqtt_root_topic, CONFIGURATION_DEFAULT_MQTT_ROOT_TOPIC, MQTT_ROOT_TOPIC_LENGTH);
      strSafeCopy(param.configuration->mqtt_maint_topic, CONFIGURATION_DEFAULT_MQTT_MAINT_TOPIC, MQTT_MAINT_TOPIC_LENGTH);
      strSafeCopy(param.configuration->mqtt_rpc_topic, CONFIGURATION_DEFAULT_MQTT_RPC_TOPIC, MQTT_RPC_TOPIC_LENGTH);
      strSafeCopy(param.configuration->stationslug, CONFIGURATION_DEFAULT_STATIONSLUG, STATIONSLUG_LENGTH);
      strSafeCopy(param.configuration->board_master.boardslug, CONFIGURATION_DEFAULT_BOARDSLUG, BOARDSLUG_LENGTH);
      #endif

      param.configuration->board_master.serial_number = StimaV4GetSerialNumber();

      #if ((USE_MQTT)&&(FIXED_CONFIGURATION))
      uint8_t temp_psk_key[] = {0x1A, 0xF1, 0x9D, 0xC0, 0x05, 0xFF, 0xCE, 0x92, 0x77, 0xB4, 0xCF, 0xC6, 0x96, 0x41, 0x04, 0x25};
      osMemcpy(param.configuration->client_psk_key, temp_psk_key, CLIENT_PSK_KEY_LENGTH);

      strSafeCopy(param.configuration->mqtt_username, "userv4", MQTT_USERNAME_LENGTH);
      strSafeCopy(param.configuration->stationslug, "verifica", STATIONSLUG_LENGTH);
      strSafeCopy(param.configuration->boardslug, "stimav4", BOARDSLUG_LENGTH);

      param.configuration->latitude = 4512345;
      param.configuration->longitude = 1212345;

      strSafeCopy(param.configuration->board_master.boardslug, "stimav4", BOARDSLUG_LENGTH);
      strSafeCopy(param.configuration->board_slave[0].boardslug, "stimacan1", BOARDSLUG_LENGTH);
      strSafeCopy(param.configuration->board_slave[1].boardslug, "stimacan2", BOARDSLUG_LENGTH);
      strSafeCopy(param.configuration->board_slave[2].boardslug, "stimacan3", BOARDSLUG_LENGTH);
      strSafeCopy(param.configuration->board_slave[3].boardslug, "stimacan4", BOARDSLUG_LENGTH);
      strSafeCopy(param.configuration->board_slave[4].boardslug, "stimacan5", BOARDSLUG_LENGTH);
      strSafeCopy(param.configuration->board_slave[5].boardslug, "stimacan6", BOARDSLUG_LENGTH);
      strSafeCopy(param.configuration->board_slave[6].boardslug, "stimacan7", BOARDSLUG_LENGTH);
      strSafeCopy(param.configuration->board_slave[7].boardslug, "stimacan8", BOARDSLUG_LENGTH);

      strSafeCopy(param.configuration->gsm_apn, GSM_APN_FASTWEB, GSM_APN_LENGTH);
      strSafeCopy(param.configuration->gsm_number, GSM_NUMBER_FASTWEB, GSM_NUMBER_LENGTH);

      #endif
    }

    //! write configuration to eeprom
    status = param.eeprom->Write(CONFIGURATION_EEPROM_ADDRESS, (uint8_t *)(param.configuration), sizeof(configuration_t));

    if (is_default)
    {
      TRACE_INFO_F(F("Save default configuration... [ %s ]\r\n"), status ? OK_STRING : ERROR_STRING);
    }
    else
    {
      TRACE_INFO_F(F("Save configuration... [ %s ]\r\n"), status ? OK_STRING : ERROR_STRING);
    }

    param.configurationLock->Give();
  }

  return status;
}