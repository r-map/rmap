/**@file supervisor_task.cpp */

/*********************************************************************
<h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
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

void SupervisorTask::Run()
{
  uint8_t retry;
  connection_request_t connection_request;
  connection_response_t connection_response;
  SupervisorConnection_t state_check_connection; // Local state (operation) when module connected

  // TODO: remove
  bool test_put_firmware = false;
  bool test_get_data = false;

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

      // INIT CONFIGURATION FOR TEST CONNECTION ANT NET FUNCTION
      saveConfiguration(true);

      TRACE_VERBOSE_F(F("SUPERVISOR_STATE_INIT -> SUPERVISOR_STATE_LOAD_CONFIGURATION\r\n"));
      state = SUPERVISOR_STATE_LOAD_CONFIGURATION;
      #if(INIT_PARAMETER)
      saveConfiguration(CONFIGURATION_DEFAULT);
      #endif
      break;

    case SUPERVISOR_STATE_LOAD_CONFIGURATION:
      is_loaded = loadConfiguration();

      if (is_loaded)
      {
        // ***** TEST CONNECTION LOAD CONFIGURATION OK
        Serial.println("// ***** TEST CONNECTION LOAD CONFIGURATION OK");

        retry = 0;

        param.systemStatusLock->Take();
        param.system_status->configuration.is_loaded = true;
        // Init acquire base datetime (for get next)
        param.system_status->datetime.ptr_time_for_sensors_get_istant = 0; // Force get istant at startup display...
        param.system_status->datetime.ptr_time_for_sensors_get_value = rtc.getEpoch() / param.configuration->report_s;
        // Init default security value
        param.system_status->connection.is_disconnected = true;
        param.system_status->connection.is_mqtt_disconnected = true;
        param.systemStatusLock->Give();

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
        // ***** TEST CONNECTION LOAD CONFIGURATION FAIL
        Serial.println("// ***** TEST CONNECTION LOAD CONFIGURATION FAIL");

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
      // TODO: Wait StimaV4 Start Event controlled time RUN Send Data

      // Retry connection
      retry = 0;

      Serial.println("// ***** TEST CONNECTION START");
      // ***** TEST CONNECTION START

      strSafeCopy(param.configuration->gsm_apn, GSM_APN_WIND, GSM_APN_LENGTH);
      strSafeCopy(param.configuration->gsm_number, GSM_NUMBER_WIND, GSM_NUMBER_LENGTH);

      // ToDo: ReNew Sequence... or NOT (START REQUEST LIST...)
      param.systemStatusLock->Take();
      param.system_status->connection.is_ntp_synchronized = false;
      param.system_status->connection.is_http_configuration_updated = false;
      param.system_status->connection.is_http_firmware_upgraded = false;
      param.system_status->connection.is_mqtt_connected = false;
      param.systemStatusLock->Give();

      // Start state check connection
      state_check_connection = CONNECTION_INIT;
      state = SUPERVISOR_STATE_CONNECTION_OPERATION;
      // Save next attempt of connection
      param.systemStatusLock->Take();
      param.system_status->modem.connection_attempted++;
      TRACE_VERBOSE_F(F("SUPERVISOR_STATE_WAITING_EVENT -> SUPERVISOR_STATE_CONNECTION_OPERATION\r\n"));
      TRACE_VERBOSE_F(F("Attempted: [ %d ] , Completed: [ %d ]\r\n"),
        param.system_status->modem.connection_attempted, param.system_status->modem.connection_completed);
      param.systemStatusLock->Give();

      break;

    case SUPERVISOR_STATE_CONNECTION_OPERATION:

      // Here config was already loaded

      // SUB Case of sequence of check (connection / operation) state
      switch(state_check_connection) {

        case CONNECTION_INIT: // STARTING CONNECTION
          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_REQUEST_CONNECTION\r\n"));
          state = SUPERVISOR_STATE_REQUEST_CONNECTION;
          state_check_connection = CONNECTION_CHECK;
          break;

        case CONNECTION_CHECK: // CONNECTION VERIFY
          if (param.system_status->connection.is_connected) // Ready Connected ?
          {
            // ***** TEST CONNECTION QUEUE RESPONSE OK
            Serial.println("// ***** TEST CONNECTION QUEUE RESPONSE OK");

            // Prepare request connection Close
            // Init retry disconnection
            retry = 0;
            // Exit from the switch (no more action)
            state = SUPERVISOR_STATE_REQUEST_DISCONNECTION;
            // Saving connection sequence completed
            param.systemStatusLock->Take();
            param.system_status->modem.connection_completed++;
            param.systemStatusLock->Give();
          } else {
            // ***** TEST CONNECTION QUEUE RESPONSE OK BUT FAILED CONNECTION
            Serial.println("// ***** TEST CONNECTION QUEUE RESPONSE OK BUT FAILED CONNECTION");

            TRACE_VERBOSE_F(F("SUPERVISOR: Connection not ready\r\n"));
            TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_END\r\n"));
            // Exit from the switch (no more action)
            state = SUPERVISOR_STATE_END;
          }
          break;
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

        // ***** TEST Start Request Connection from queue OK
        Serial.println("// ***** TEST Start Request Connection from queue OK");

        memset(&connection_request, 0, sizeof(connection_request_t));
        connection_request.do_connect = true;                
        param.connectionRequestQueue->Enqueue(&connection_request, 0);

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
          param.connectionResponseQueue->Dequeue(&connection_response, 0);
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
          param.connectionResponseQueue->Dequeue(&connection_response, 0);
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
        param.connectionRequestQueue->Enqueue(&connection_request, 0);

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
          // ***** TEST Disconnection OK
          // ***** TEST END
          Serial.println("// ***** TEST Disconnection form queue OK");
          Serial.println("// ***** TEST END");
          param.connectionResponseQueue->Dequeue(&connection_response, 0);
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

  #if(INIT_PARAMETER)
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

    TRACE_INFO_F(F("-> data report every %d seconds\r\n"), param.configuration->report_s);
    TRACE_INFO_F(F("-> data observation every %d seconds\r\n"), param.configuration->observation_s);
    // TRACE_INFO_F(F("-> data level: %s\r\n"), param.configuration->data_level);
    TRACE_INFO_F(F("-> ident: %s\r\n"), param.configuration->ident);
    TRACE_INFO_F(F("-> longitude %07d and latitude %07d\r\n"), param.configuration->longitude, param.configuration->latitude);
    TRACE_INFO_F(F("-> network: %s\r\n"), param.configuration->network);

    #if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
    TRACE_INFO_F(F("-> gsm apn: %s\r\n"), param.configuration->gsm_apn);
    TRACE_INFO_F(F("-> gsm number: %s\r\n"), param.configuration->gsm_number);
    TRACE_INFO_F(F("-> gsm username: %s\r\n"), param.configuration->gsm_username);
    TRACE_INFO_F(F("-> gsm password: %s\r\n"), param.configuration->gsm_password);
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
      // strSafeCopy(param.configuration->data_level, CONFIGURATION_DEFAULT_DATA_LEVEL, DATA_LEVEL_LENGTH);
      strSafeCopy(param.configuration->network, CONFIGURATION_DEFAULT_NETWORK, NETWORK_LENGTH);

      param.configuration->latitude = CONFIGURATION_DEFAULT_LATITUDE;
      param.configuration->longitude = CONFIGURATION_DEFAULT_LONGITUDE;

      #if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
      strSafeCopy(param.configuration->gsm_apn, CONFIGURATION_DEFAULT_GSM_APN, GSM_APN_LENGTH);
      strSafeCopy(param.configuration->gsm_number, CONFIGURATION_DEFAULT_GSM_NUMBER, GSM_NUMBER_LENGTH);
      strSafeCopy(param.configuration->gsm_username, CONFIGURATION_DEFAULT_GSM_USERNAME, GSM_USERNAME_LENGTH);
      strSafeCopy(param.configuration->gsm_password, CONFIGURATION_DEFAULT_GSM_PASSWORD, GSM_PASSWORD_LENGTH);
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