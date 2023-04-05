/**@file supervisor_task.cpp */

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

#define TRACE_LEVEL     SUPERVISOR_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   SUPERVISOR_TASK_ID

#include "tasks/supervisor_task.h"

// TODO: Move to MQTT
#include "date_time.h"
// Namespace RMAP
#include <rmap/_module/TH_1_0.h>
#include <rmap/service/_module/TH_1_0.h>
#include <rmap/_module/RAIN_1_0.h>
#include <rmap/service/_module/RAIN_1_0.h>

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
     param.system_status->tasks->watch_dog = wdt_flag::set;
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

      // TODO: Remove Test Wait new start as time test 2,5 sec...
      // TODO: Create Date/Time Event start from configuration
      // TODO: Start only modulePower Full OK (no energy rest)
      // TODO: Start only if data enable to send
      // TODO: Remove NTP Syncro (1 x day?)
      // TODO: Get RPC Remote? When connected... + Queue Command RPC (system_message_t)      

      // **************************************
      //    TEST PUT Firmware Queue Usage
      // **************************************
      if(!test_put_firmware) {

        test_put_firmware = true;

        file_put_request_t firmwareDownloadChunck;
        file_put_response_t sdcard_task_response;
        bool file_upload_error = false;

        // MMC have to GET Ready before Push DATA
        // EXIT from function if not MMC Ready or present into system_status
        if(!param.system_status->flags.sd_card_ready) {
          TRACE_VERBOSE_F(F("SUPERVISOR: Reject request upload file (Firmware) MMC was not ready [ %s ]\r\n"), ERROR_STRING);
          break;
        }

        // First block NAME OF FILE (Prepare name and Put to queue)
        // TODO: Get From HTTP
        memset(&firmwareDownloadChunck, 0, sizeof(file_put_request_t));
        firmwareDownloadChunck.block_type = file_block_type::file_name;
        // Chose one method to put name file (only name file without prefix directory)
        strcpy((char*)firmwareDownloadChunck.block, "stima4.module_th-4.3.app.hex");
        // OR FILE NAME FROM TYPE... IF HTTP Responding with Module, Version and Revision...
        // setStimaFirmwareName((char*)firmwareDownloadChunck.block, STIMA_MODULE_TYPE_TH, 4, 3);
        firmwareDownloadChunck.block_lenght = strlen((char*)firmwareDownloadChunck.block);
        TRACE_VERBOSE_F(F("Starting upload file (Firmware) from remote HTTP to local MMC [ %s ]\r\n"), firmwareDownloadChunck.block);
        // Push data request to queue MMC
        param.dataFilePutRequestQueue->Enqueue(&firmwareDownloadChunck, 0);

        // Non blocking task
        TaskWatchDog(SUPERVISOR_TASK_WAIT_DELAY_MS);
        Delay(Ticks::MsToTicks(SUPERVISOR_TASK_WAIT_DELAY_MS));

        // Waiting response from MMC with TimeOUT
        memset(&sdcard_task_response, 0, sizeof(file_put_response_t));
        TaskWatchDog(FILE_IO_DATA_QUEUE_TIMEOUT);
        file_upload_error = !param.dataFilePutResponseQueue->Dequeue(&sdcard_task_response, FILE_IO_DATA_QUEUE_TIMEOUT);
        file_upload_error |= !sdcard_task_response.done_operation;
        // Add Data Chunck... TODO: Get From HTTP...
        if(!file_upload_error) {
          // Next block is data_chunk + Lenght to SET (in this all 512 bytes)
          firmwareDownloadChunck.block_type = file_block_type::data_chunck;
          for(u_int16_t j=0; j<512; j++) {
            // ASCII Char... Fill example
            // TODO: Correct bytes read from buffer http...
            firmwareDownloadChunck.block[j] = 48 + (j % 10);
          }
          firmwareDownloadChunck.block_lenght = 512;
          // Try 100 Block Data chunk... Queue to MMC (x 512 Bytes -> 51200 Bytes to Write)
          for(uint8_t i=0; i<100; i++) {
            // Push data request to queue MMC
            param.dataFilePutRequestQueue->Enqueue(&firmwareDownloadChunck, 0);
            // Waiting response from MMC with TimeOUT
            memset(&sdcard_task_response, 0, sizeof(file_put_response_t));
            TaskWatchDog(FILE_IO_DATA_QUEUE_TIMEOUT);
            file_upload_error = !param.dataFilePutResponseQueue->Dequeue(&sdcard_task_response, FILE_IO_DATA_QUEUE_TIMEOUT);
            file_upload_error |= !sdcard_task_response.done_operation;
            // Non blocking task
            TaskWatchDog(SUPERVISOR_TASK_WAIT_DELAY_MS);
            Delay(Ticks::MsToTicks(SUPERVISOR_TASK_WAIT_DELAY_MS));
            // Any error? Exit Uploading
            if (file_upload_error) {
              TRACE_VERBOSE_F(F("Uploading file error!!!\r\n"));
              break;
            }
          }
          // Final Block (EOF, without checksum). If cecksum use file_block_type::end_of_file and put checksum Verify into block...
          firmwareDownloadChunck.block_type = file_block_type::end_of_file;
          // Push data request to queue MMC
          param.dataFilePutRequestQueue->Enqueue(&firmwareDownloadChunck, 0);

          // Waiting response from MMC with TimeOUT
          memset(&sdcard_task_response, 0, sizeof(file_put_response_t));
          TaskWatchDog(FILE_IO_DATA_QUEUE_TIMEOUT);
          file_upload_error = !param.dataFilePutResponseQueue->Dequeue(&sdcard_task_response, FILE_IO_DATA_QUEUE_TIMEOUT);
          file_upload_error |= !sdcard_task_response.done_operation;
        }

        // FLUSH Security Queue if any Error occurs (Otherwise queue are empty. Pull From TASK MMC)
        TRACE_VERBOSE_F(F("Uploading file (Firmware) [ %s ]\r\n"), file_upload_error ? ERROR_STRING : OK_STRING);
      }
      // **************************************
      //   END TEST PUT Firmware Queue Usage
      // **************************************

      // **************************************
      //   TEST GET RMAP Data, To Append MQTT
      // **************************************
      if(!test_get_data) {

        test_get_data = true;

        rmap_get_request_t rmap_get_request;
        rmap_get_response_t rmap_get_response;
        bool rmap_data_error = false;

        // MMC have to GET Ready before Push DATA
        // EXIT from function if not MMC Ready or present into system_status
        if(!param.system_status->flags.sd_card_ready) {
          TRACE_VERBOSE_F(F("SUPERVISOR: Reject request get rmap data, MMC was not ready [ %s ]\r\n"), ERROR_STRING);
          break;
        }

        // // **** SET POINTER DATA REQUEST *****
        // // Example SET Timer from datTime ->
        uint32_t countData = 0;
        DateTime date_request;
        date_request.day = 16;
        date_request.month = 2;
        date_request.year = 2023;
        date_request.hours = 4;
        date_request.minutes = 22;
        date_request.seconds = 23;
        uint32_t rmap_date_time_ptr = convertDateToUnixTime(&date_request);

        memset(&rmap_get_request, 0, sizeof(rmap_get_request_t));
        rmap_get_request.param = rmap_date_time_ptr;
        rmap_get_request.command.do_synch_ptr = true;
        // Optional Save Pointer in File (Probabiliy always in SetPtr)
        rmap_get_request.command.do_save_ptr = true;
        TRACE_VERBOSE_F(F("Starting request SET Data RMAP PTR to local MMC\r\n"));
        // Push data request to queue MMC
        param.dataRmapGetRequestQueue->Enqueue(&rmap_get_request, 0);

        // Non blocking task
        TaskWatchDog(SUPERVISOR_TASK_WAIT_DELAY_MS);
        Delay(Ticks::MsToTicks(SUPERVISOR_TASK_WAIT_DELAY_MS));

        // Waiting response from MMC with TimeOUT
        memset(&rmap_get_response, 0, sizeof(rmap_get_response));
        // Seek Operation can Be Long Time Procedure. Queue can be post in waiting state without Time End
        // Task WDT Are suspended
        TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
        rmap_data_error = !param.dataRmapGetResponseQueue->Dequeue(&rmap_get_response);
        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
        rmap_data_error |= rmap_get_response.result.event_error;
        // Rmap Pointer setted? Get All Data from RMAP Archive
        bool rmap_eof = false;
        // Exit on End of data or Error from queue
        while((!rmap_data_error)&&(!rmap_eof)) {
          memset(&rmap_get_request, 0, sizeof(rmap_get_request));
          // Get Next data... Stop at EOF
          rmap_get_request.command.do_get_data = true;
          // Save Pointer? Optional
          // rmap_get_request.command.do_save_ptr = true;
          // Push data request to queue MMC
          param.dataRmapGetRequestQueue->Enqueue(&rmap_get_request, 0);
          // Waiting response from MMC with TimeOUT
          memset(&rmap_get_response, 0, sizeof(rmap_get_response));
          TaskWatchDog(FILE_IO_DATA_QUEUE_TIMEOUT);
          rmap_data_error = !param.dataRmapGetResponseQueue->Dequeue(&rmap_get_response, FILE_IO_DATA_QUEUE_TIMEOUT);
          rmap_data_error |= rmap_get_response.result.event_error;
          if(!rmap_data_error) {
            // EOF Data? (Save and Exit, after last data process)
            rmap_eof = rmap_get_response.result.end_of_data;
            // ******************************************************************
            // Exampe of Current Session Upload CountData and DateTime Block Print
            countData++;
            DateTime rmap_date_time_val;
            convertUnixTimeToDate(rmap_get_response.rmap_data.date_time, &rmap_date_time_val);
            TRACE_VERBOSE_F(F("Data RMAP current date/time [ %d ] %s\r\n"), (uint32_t)countData, formatDate(&rmap_date_time_val, NULL));
            // ******************************************************************
            // Process Data with casting RMAP Module Type
            switch (rmap_get_response.rmap_data.module_type) {
              case Module_Type::th:
                rmap_module_TH_1_0 *rmapDataTH;
                rmapDataTH = (rmap_module_TH_1_0 *)&rmap_get_response.rmap_data;
                #if (ENABLE_STACK_USAGE)
                TaskMonitorStack();
                #endif
                // Prepare MQTT String -> rmapDataTH->ITH.humidity.val.value ecc...
                // PUT String MQTT in Buffer Memory...
                // SEND To MQTT Server
                break;
              case Module_Type::rain:
                rmap_module_Rain_1_0 *rmapDataRain;
                rmapDataRain = (rmap_module_Rain_1_0 *)&rmap_get_response.rmap_data;
                #if (ENABLE_STACK_USAGE)
                TaskMonitorStack();
                #endif
                // Prepare MQTT String -> rmapDataRain->TBR.metadata.level.L1.value ecc...
                // PUT String MQTT in Buffer Memory...
                // SEND To MQTT Server
                break;
            }
          } else {
            TRACE_VERBOSE_F(F("RMAP Reading Data queue error!!!\r\n"));
          }
          // Non blocking task
          TaskWatchDog(TASK_WAIT_REALTIME_DELAY_MS);
          Delay(Ticks::MsToTicks(TASK_WAIT_REALTIME_DELAY_MS));
        }
        // Trace END Data response
        TRACE_VERBOSE_F(F("Uploading data RMAP Archive [ %s ]. Updated %d record\r\n"), rmap_eof ? OK_STRING : ERROR_STRING, countData);
      }
      // **************************************
      // END TEST GET RMAP Data, To Append MQTT
      // **************************************
      TaskWatchDog(3500);
      Delay(Ticks::MsToTicks(3500));

      // TEST CONNECTION
      if(0) {

        // TODO: REMOVE
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

      }

      break;

    case SUPERVISOR_STATE_CONNECTION_OPERATION:

      // Here config was already loaded

      // Sequence connection (on start set request param list operation)
      // es. ->
      // param.system_status->connection.is_ntp_synchronized = true; (require new NTP synch)
      // param.system_status->connection.is_http_configuration_updated = true; // (no operation)
      // param.system_status->connection.is_http_firmware_upgraded = true;
      // param.system_status->connection.is_mqtt_connected = true;

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
            state = SUPERVISOR_STATE_REQUEST_CONNECTION;
          } else {
            TRACE_VERBOSE_F(F("SUPERVISOR: Connection not ready\r\n"));
            TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_END\r\n"));
            // Exit from the switch (no more action)
            state = SUPERVISOR_STATE_END;
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
            param.connectionRequestQueue->Enqueue(&connection_request, 0);

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
            param.connectionRequestQueue->Enqueue(&connection_request, 0);

            TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_DO_HTTP\r\n"));
            state = SUPERVISOR_STATE_DO_HTTP;
          }
          else if (!param.system_status->connection.is_http_firmware_upgraded)
          {
            TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_REQUEST_CONNECTION\r\n"));

            // Request firmware download by http request
            memset(&connection_request, 0, sizeof(connection_request_t));
            connection_request.do_http_get_firmware = true;
            param.connectionRequestQueue->Enqueue(&connection_request, 0);

            TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_DO_HTTP\r\n"));
            state = SUPERVISOR_STATE_DO_HTTP;
          }
          // Prepare next state controller
          state_check_connection = CONNECTION_CHECK_MQTT;
          break;

        case CONNECTION_CHECK_MQTT: // Rmap Publish data (MQTT)
          if (!param.system_status->connection.is_mqtt_connected) // Mqtt connected?
          {
            // Request mqtt connection
            memset(&connection_request, 0, sizeof(connection_request_t));
            connection_request.do_mqtt_connect = true;
            param.connectionRequestQueue->Enqueue(&connection_request, 0);

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
          param.connectionResponseQueue->Dequeue(&connection_response, 0);
          TRACE_INFO_F(F("%s NTP synchronization [ %s ]\r\n"), Thread::GetName().c_str(), OK_STRING);

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_DO_NTP -> SUPERVISOR_STATE_CONNECTION_OPERATION\r\n"));
          state = SUPERVISOR_STATE_CONNECTION_OPERATION;
        }
        // error: ntp syncronized
        else if (connection_response.error_ntp_synchronized)
        {
          param.connectionResponseQueue->Dequeue(&connection_response, 0);
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
          param.connectionResponseQueue->Dequeue(&connection_response, 0);
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
          param.connectionResponseQueue->Dequeue(&connection_response, 0);
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
          param.connectionResponseQueue->Dequeue(&connection_response, 0);
          TRACE_INFO_F(F("%s MQTT connected [ %s ]\r\n"), Thread::GetName().c_str(), OK_STRING);

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_DO_MQTT -> SUPERVISOR_STATE_CONNECTION_OPERATION\r\n"));
          state = SUPERVISOR_STATE_CONNECTION_OPERATION;
        }
        // error: not connected
        else if (connection_response.error_mqtt_connected)
        {
          param.connectionResponseQueue->Dequeue(&connection_response, 0);
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
    TRACE_INFO_F(F("-> board slug: %s\r\n"), param.configuration->boardslug);
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
      // strSafeCopy(param.configuration->data_level, CONFIGURATION_DEFAULT_DATA_LEVEL, DATA_LEVEL_LENGTH);
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
      strSafeCopy(param.configuration->boardslug, CONFIGURATION_DEFAULT_BOARDSLUG, BOARDSLUG_LENGTH);
      #endif

      // TODO: da rimuovere
      #if (USE_MQTT)
      // uint8_t temp_psk_key[] = {0x4F, 0x3E, 0x7E, 0x10, 0xD2, 0xD1, 0x6A, 0xE2, 0xC5, 0xAC, 0x60, 0x12, 0x0F, 0x07, 0xEF, 0xAF};
      // uint8_t temp_psk_key[] = {0x30, 0xA4, 0x45, 0xD2, 0xE6, 0x1A, 0x88, 0xD7, 0xDB, 0x7D, 0xC4, 0xF7, 0xC9, 0x6B, 0xC5, 0x27};
      uint8_t temp_psk_key[] = {0x1A, 0xF1, 0x9D, 0xC0, 0x05, 0xFF, 0xCE, 0x92, 0x77, 0xB4, 0xCF, 0xC6, 0x96, 0x41, 0x04, 0x25};
      osMemcpy(param.configuration->client_psk_key, temp_psk_key, CLIENT_PSK_KEY_LENGTH);

      strSafeCopy(param.configuration->mqtt_username, "userv4", MQTT_USERNAME_LENGTH);
      strSafeCopy(param.configuration->stationslug, "stimacan", STATIONSLUG_LENGTH);
      strSafeCopy(param.configuration->boardslug, "stimav4", BOARDSLUG_LENGTH);

      param.configuration->latitude = 4512345;
      param.configuration->longitude = 1212345;

      strSafeCopy(param.configuration->gsm_apn, GSM_APN_FASTWEB, GSM_APN_LENGTH);
      strSafeCopy(param.configuration->gsm_number, GSM_NUMBER_FASTWEB, GSM_NUMBER_LENGTH);

      param.configuration->board_master.serial_number = StimaV4GetSerialNumber();
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