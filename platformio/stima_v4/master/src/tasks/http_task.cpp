/**@file http_task.cpp */

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

#define TRACE_LEVEL     HTTP_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   HTTP_TASK_ID

#include "tasks/http_task.h"

#if (USE_HTTP)

using namespace cpp_freertos;

HttpTask::HttpTask(const char *taskName, uint16_t stackSize, uint8_t priority, HttpParam_t httpParam) : Thread(taskName, stackSize, priority), param(httpParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(HTTP_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  state = HTTP_STATE_INIT;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void HttpTask::TaskMonitorStack()
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
void HttpTask::TaskWatchDog(uint32_t millis_standby)
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
void HttpTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
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

void HttpTask::Run() {
  uint8_t retry;
  uint8_t retry_get_response;
  bool is_error;
  error_t error;
  IpAddr ipAddr;
  uint_t status;
  const char_t *value;
  char_t serial_number_str[12];
  uint32_t serial_number_l;
  uint32_t serial_number_h;

  char uri[HTTP_URI_LENGTH];
  char header[HTTP_HEADER_SIZE];
  char module_type[STIMA_MODULE_NAME_LENGTH];

  bool is_get_configuration;
  bool is_get_firmware;
  bool bValidFirmwareRequest = false;
  bool bErrorFirmwareDownload = false;
  bool bDownloadedFirmware = false;     // Set true if at least firmware are donloaded
  uint8_t module_download; // Module download firmware from ID Master FF 00..BOARDS_COUNT_MAX (Slave)
  uint8_t module_download_ver, module_download_rev; // firmware version and revision in download
  uint8_t module_download_type; // firmware module type in download
  char module_download_md5[32]; // firmware md5 ckeck
  uint32_t totBytesRead = 0;    // firmware download bytes for file

  connection_request_t connection_request;
  connection_response_t connection_response;

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  while (true)
  {
    switch (state)
    {
    case HTTP_STATE_INIT:
      TRACE_VERBOSE_F(F("HTTP_STATE_INIT -> HTTP_STATE_WAIT_NET_EVENT\r\n"));
      state = HTTP_STATE_WAIT_NET_EVENT;
      break;

    case HTTP_STATE_WAIT_NET_EVENT:
      is_get_configuration = false;
      is_get_firmware = false;
      is_error = false;
      retry = 0;

      // wait connection request
      // Suspend TASK Controller for queue waiting portMAX_DELAY
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);      
      if (param.connectionRequestQueue->Peek(&connection_request, portMAX_DELAY))
      {
        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
        HttpServer = param.configuration->mqtt_server;

        // do http get configuration (prioritary)
        if (connection_request.do_http_get_configuration)
        {
          is_get_configuration = true;
          param.connectionRequestQueue->Dequeue(&connection_request);
          state = HTTP_STATE_SEND_REQUEST;
          TRACE_VERBOSE_F(F("HTTP_STATE_WAIT_NET_EVENT -> HTTP_STATE_SEND_REQUEST (get configuration)\r\n"));
        }
        // do http get firmware
        else if (connection_request.do_http_get_firmware)
        {
          // SD have to GET Ready before Push DATA (Firmware download?! Exit immediatly)
          // EXIT from function if not SD Ready or present into system_status
          if(!param.system_status->flags.sd_card_ready) {
            TRACE_VERBOSE_F(F("HTTP: Reject request upload file (Firmware) SD was not ready [ %s ]\r\n"), ERROR_STRING);
            state = HTTP_STATE_END;
          } else {
            is_get_firmware = true;
            module_download = 0xFF; // Starting from Master
            param.connectionRequestQueue->Dequeue(&connection_request);
            state = HTTP_STATE_SEND_REQUEST;
            TRACE_VERBOSE_F(F("HTTP_STATE_WAIT_NET_EVENT -> HTTP_STATE_SEND_REQUEST (get firmware)\r\n"));
          }
        }
      }
      break;

    // Send Request HTTP
    case HTTP_STATE_SEND_REQUEST:
      httpClientInit(&httpClientContext);

      param.systemStatusLock->Take();
      param.system_status->connection.is_http_configuration_updating = is_get_configuration;
      param.system_status->connection.is_http_firmware_upgrading = is_get_firmware;
      param.systemStatusLock->Give();

      TRACE_INFO_F(F("%s Resolving http server name of %s \r\n"), Thread::GetName().c_str(), HttpServer);

      // Resolve HTTP server name
      TaskState(state, 1, task_flag::suspended); // Or SET Long WDT > 120 sec.
      error = getHostByName(NULL, HttpServer, &ipAddr, 0);
      TaskState(state, 1, task_flag::normal); // Resume
      // Any error to report?
      if (error)
      {
        is_error = true;
        param.systemStatusLock->Take();
        param.system_status->connection.is_dns_failed_resolve = true;
        param.system_status->flags.dns_error = true;
        param.systemStatusLock->Give();
        state = HTTP_STATE_END;
        TRACE_VERBOSE_F(F("HTTP_STATE_SEND_REQUEST -> HTTP_STATE_END\r\n"));
        TRACE_ERROR_F(F("%s Failed to resolve http server name of %s [ %s ]\r\n"), Thread::GetName().c_str(), HttpServer, ERROR_STRING);
        break;
      } else {
        param.systemStatusLock->Take();
        param.system_status->flags.dns_error = false;
        param.systemStatusLock->Give();
      }

      // Shared Pointer
      HttpYarrowContext = param.yarrowContext;
      HttpClientPSKKey = param.configuration->client_psk_key;

      // Set PSK identity
      snprintf(HttpClientPSKIdentity, sizeof(HttpClientPSKIdentity), "%s/%s/%s", param.configuration->mqtt_username, param.configuration->stationslug, param.configuration->board_master.boardslug);
      TRACE_VERBOSE_F(F("HTTP PSK Identity: %s\r\n"), HttpClientPSKIdentity);

      // Register TLS initialization callback
      error = httpClientRegisterTlsInitCallback(&httpClientContext, httpClientTlsInitCallback);
      // Any error to report?
      if (error)
      {
        is_error = true;
        state = HTTP_STATE_END;
        TRACE_VERBOSE_F(F("HTTP_STATE_SEND_REQUEST -> HTTP_STATE_END\r\n"));
        TRACE_ERROR_F(F("%s Failed to init https callback [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
        break;
      }

      // Select HTTP protocol version
      error = httpClientSetVersion(&httpClientContext, HTTP_VERSION_1_1);
      // Any error to report?
      if (error)
      {
        is_error = true;
        state = HTTP_STATE_END;
        TRACE_VERBOSE_F(F("HTTP_STATE_SEND_REQUEST -> HTTP_STATE_END\r\n"));

        TRACE_ERROR_F(F("%s Failed to resolve http client version [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
        break;
      }

      // Set timeout value for blocking operations
      error = httpClientSetTimeout(&httpClientContext, HTTP_CLIENT_TIMEOUT_MS);
      // Any error to report?
      if (error)
      {
        is_error = true;
        state = HTTP_STATE_END;
        TRACE_VERBOSE_F(F("HTTP_STATE_SEND_REQUEST -> HTTP_STATE_END\r\n"));

        TRACE_ERROR_F(F("%s Failed to set http server timeuout [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
        break;
      }

      TaskState(state, 1, task_flag::suspended); // Or SET Long WDT > 120 sec.
      // Connect to the HTTP server
      error = httpClientConnect(&httpClientContext, &ipAddr, HTTP_CLIENT_PORT);
      TaskState(state, 1, task_flag::normal); // Resume
      // Any error to report?
      if (error)
      {
        is_error = true;
        state = HTTP_STATE_END;
        TRACE_VERBOSE_F(F("HTTP_STATE_SEND_REQUEST -> HTTP_STATE_END\r\n"));

        TRACE_ERROR_F(F("%s Failed to connect to http server %s [ %s ]\r\n"), Thread::GetName().c_str(), HttpServer, ERROR_STRING);
        break;
      }

      // Create an HTTP request
      httpClientCreateRequest(&httpClientContext);
      httpClientSetMethod(&httpClientContext, "GET");

      if (is_get_configuration)
      {
        snprintf(uri, sizeof(uri), "/stationconfig/%s/%s", param.configuration->mqtt_username, param.configuration->stationslug);
      }
      else if (is_get_firmware)
      {
        // ? Master or Slave module (module_download_type is type_id module to download)
        if(module_download == 0xFF) {
          module_download_type = param.configuration->module_type;
        } else {
          module_download_type = param.configuration->board_slave[module_download].module_type;
        }
        // snprintf(uri, sizeof(uri), "/firmware/stima/v4/update/%u/", param.configuration->module_type);
        getStimaNameByType(header, module_download_type, STIMA_MODULE_OFFSET_IDENT_V4);
        snprintf(uri, sizeof(uri), "/firmware/stima/v4/update/%s/", header);
      }

      TRACE_INFO_F(F("%s http request to %s%s\r\n"), Thread::GetName().c_str(), HttpServer, uri);

      httpClientSetUri(&httpClientContext, uri);

      // Set query string example
      // httpClientAddQueryParam(&httpClientContext, "param1", "value1");
      // httpClientAddQueryParam(&httpClientContext, "param2", "value2");

      // Add HTTP header fields
      httpClientAddHeaderField(&httpClientContext, "Host", HttpServer);

      // MAC AND Version for Firmware Upload
      if(is_get_firmware) {
        // ? Master or slave (Add Board-SN)
        if(module_download == 0xFF) {
          // Master request
          serial_number_l = param.configuration->board_master.serial_number & 0xFFFFFFFF;
          serial_number_h = (param.configuration->board_master.serial_number >> 32) & 0xFFFFFFFF;
          snprintf(header, sizeof(header), "{\"version\": %d,\"revision\": %d,\"user\":\"%s\",\"slug\":\"%s\",\"bslug\":\"%s\"}", param.configuration->module_main_version, param.configuration->module_minor_version, param.configuration->mqtt_username, param.configuration->stationslug, param.configuration->board_master.boardslug);
        } else {
          // Slave request compose S.N. if not imposted on config
          if(!param.configuration->board_slave[module_download].serial_number) {
            for(uint8_t iSn=1; iSn<8; iSn++) {
              param.configuration->board_slave[module_download].serial_number |= param.configuration->stationslug[iSn] << (iSn*8);
            }
            param.configuration->board_slave[module_download].serial_number |= module_download_type;
          }
          serial_number_l = param.configuration->board_slave[module_download].serial_number & 0xFFFFFFFF;
          serial_number_h = (param.configuration->board_slave[module_download].serial_number >> 32) & 0xFFFFFFFF;
          snprintf(header, sizeof(header), "{\"version\": %d,\"revision\": %d,\"user\":\"%s\",\"slug\":\"%s\",\"bslug\":\"%s\"}", param.system_status->data_slave[module_download].module_version, param.system_status->data_slave[module_download].module_revision, param.configuration->mqtt_username, param.configuration->stationslug, param.configuration->board_slave[module_download].boardslug);
        }
        snprintf(serial_number_str, sizeof(serial_number_str), "%04X%04X", serial_number_h, serial_number_l);
        // Add header request
        httpClientAddHeaderField(&httpClientContext, "X-STIMA4-VERSION", header);
        httpClientAddHeaderField(&httpClientContext, "X-STIMA4-BOARD-MAC", serial_number_str);
      }

      httpClientAddHeaderField(&httpClientContext, "User-Agent", "STIMA4-http-Update");

      // Send HTTP request header
      error = httpClientWriteHeader(&httpClientContext);
      // Any error to report?
      if (error)
      {
        TRACE_ERROR_F(F("%s Failed to write http request header [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);

        is_error = true;
        state = HTTP_STATE_END;
        TRACE_VERBOSE_F(F("HTTP_STATE_SEND_REQUEST -> HTTP_STATE_END\r\n"));
        break;
      }

      // Send HTTP request body example
      // error = httpClientWriteBody(&httpClientContext, "", 0, NULL, 0);
      // // Any error to report?
      // if (error)
      // {
      //   // Debug message
      //   TRACE_ERROR_F(F("%s Failed to write http request body [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);

      //   is_error = true;
      //   state = HTTP_STATE_END;
      //   TRACE_VERBOSE_F(F("HTTP_STATE_SEND_REQUEST -> HTTP_STATE_END\r\n"));
      //   break;
      // }

      state = HTTP_STATE_GET_RESPONSE;
      retry_get_response = 0;
      TRACE_VERBOSE_F(F("HTTP_STATE_SEND_REQUEST -> HTTP_STATE_GET_RESPONSE\r\n"));
      break;

    case HTTP_STATE_GET_RESPONSE:

      TaskState(state, 1, task_flag::suspended); // Or SET Long WDT > 120 sec.
      // Receive HTTP response header
      error = httpClientReadHeader(&httpClientContext);
      TaskState(state, 1, task_flag::normal); // Resume
      // Any error to report?
      if (error)
      {
        if(++retry_get_response<HTTP_TASK_GENERIC_RETRY) {
          is_error = true;
          state = HTTP_STATE_END;
          TRACE_ERROR_F(F("%s Failed to read http response header [ %s ] ABORT!!!\r\n"), Thread::GetName().c_str(), ERROR_STRING);
        } else {
          TaskWatchDog(HTTP_TASK_GENERIC_RETRY_DELAY_MS);
          Delay(Ticks::MsToTicks(HTTP_TASK_GENERIC_RETRY_DELAY_MS));
          TRACE_ERROR_F(F("%s Failed to read http response header [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
        }
        break;
      }

      // Retrieve HTTP status code
      status = httpClientGetStatus(&httpClientContext);
            
      if(is_get_firmware) {
        bValidFirmwareRequest = false;
        switch (status) {
          case 300:
            TRACE_ERROR_F(F("%s http status code %u [Firmware version not correct]\r\n"), Thread::GetName().c_str(), status);
            break;
          case 304:
            TRACE_ERROR_F(F("%s http status code %u [Firmware version already installed]\r\n"), Thread::GetName().c_str(), status);
            break;
          case 403:
            TRACE_ERROR_F(F("%s http status code %u [Firmware version not exist]\r\n"), Thread::GetName().c_str(), status);
            break;
          case 500:
            TRACE_ERROR_F(F("%s http status code %u [Header request not valid]\r\n"), Thread::GetName().c_str(), status);
            break;
          default:
            TRACE_VERBOSE_F(F("%s http status code %u [Firmware request valid]\r\n"), Thread::GetName().c_str(), status);
            bValidFirmwareRequest = true;
        }

        // ************** GET FIRMWARE EXAMPLES REQUEST ****************
        // https://test.rmap.cc/admin/firmware_updater_stima/firmware/
        // Command CURL To Test :
        // curl -v  -H "X-STIMA4-VERSION: {\"version\": 4,\"revision\": 
        // 0,\"user\":\"userv4\",\"slug\":\"stimacan\",\"bslug\":\"stimav4\"}" -H
        // "X-STIMA4-BOARD-MAC: 101" -A "STIMA4-http-Update"  
        // http://test.rmap.cc/firmware/stima/v4/update/11/ --output firmware

        if(bValidFirmwareRequest) {
          TRACE_INFO_F(F("%s http request firmware dowload [ OK ], ready to download\r\n"), Thread::GetName().c_str(), status);
          // Retrieve the value of the Content-Type header field
          value = httpClientGetHeaderField(&httpClientContext, "x-MD5");
          strcpy(module_download_md5, value);
          // Get version and revision for create file and check update is valid
          value = httpClientGetHeaderField(&httpClientContext, "version");
          module_download_ver = atoi(value);
          value = httpClientGetHeaderField(&httpClientContext, "revision");
          module_download_rev = atoi(value);
        }

        // Header field found? With requet OK
        if ((bValidFirmwareRequest) && (value == NULL))
        {
          if(++retry_get_response<HTTP_TASK_GENERIC_RETRY) {
            is_error = true;
            state = HTTP_STATE_END;
            TRACE_ERROR_F(F("%s Content-Type header field not found [ %s ] ABORT!!!\r\n"), Thread::GetName().c_str(), ERROR_STRING);
          } else {
            TaskWatchDog(HTTP_TASK_GENERIC_RETRY_DELAY_MS);
            Delay(Ticks::MsToTicks(HTTP_TASK_GENERIC_RETRY_DELAY_MS));
            TRACE_ERROR_F(F("%s Content-Type header field not found [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
          }
          break;
        }

        // is_firmware (start queue naming file)
        if(bValidFirmwareRequest) {
          bErrorFirmwareDownload = do_firmware_set_name((Module_Type)module_download_type, module_download_ver, module_download_rev);
        } else {
          bErrorFirmwareDownload = true;
        }
      }

      if(is_get_configuration) {
        if(status==200)
          TRACE_VERBOSE_F(F("%s http status code %u [Configuration request valid]\r\n"), Thread::GetName().c_str(), status);
        else
          TRACE_ERROR_F(F("%s http status code %u [Configuration request failed]\r\n"), Thread::GetName().c_str(), status);
      }

      // Firmware bytes download module
      totBytesRead = 0;
      // Receive HTTP response body (and no Error Dowload Firmware (always false is not firmware request))
      while ((!error)&&(!bErrorFirmwareDownload))
      {
        TaskState(state, 1, task_flag::suspended); // Or SET Long WDT > 120 sec.

        if (is_get_configuration)
        {
          is_event_rpc = true;
          param.streamRpc->init();

          error = httpClientReadBody(&httpClientContext, http_buffer, sizeof(http_buffer) - 1, &http_buffer_length, SOCKET_FLAG_BREAK_CRLF);

          #if (ENABLE_STACK_USAGE)
          TaskMonitorStack();
          #endif

          if (!error)
          {
            // Security Remove flag config wait... Start success download 
            if(param.system_status->flags.http_wait_cfg) {
              param.systemStatusLock->Take();
              param.system_status->flags.http_wait_cfg = false;
              param.systemStatusLock->Give();
            }

            http_buffer[http_buffer_length] = '\0';
            TRACE_INFO_F(F("%s"), http_buffer);

          }

          // Put RPC for configuration mode
          if (param.rpcLock->Take(Ticks::MsToTicks(RPC_WAIT_DELAY_MS)))
          {
            while (is_event_rpc)
            {
              #if (ENABLE_STACK_USAGE)
              TaskMonitorStack();
              #endif
              // Security lock task_flag for External Local TASK RPC (Need for risk of WDT Reset)
              param.system_status->tasks[LOCAL_TASK_ID].state = task_flag::suspended;
              param.streamRpc->parseCharpointer(&is_event_rpc, (char *)http_buffer, http_buffer_length, NULL, 0, RPC_TYPE_HTTPS);
              param.system_status->tasks[LOCAL_TASK_ID].state = task_flag::normal;
              param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
            }
            param.rpcLock->Give();
          }
          // Delay for command accept...
          TaskWatchDog(HTTP_TASK_RPC_WAIT_DELAY_MS);
          Delay(Ticks::MsToTicks(HTTP_TASK_RPC_WAIT_DELAY_MS));
          
        }
        else if (is_get_firmware)
        {
          error = httpClientReadBody(&httpClientContext, http_buffer, sizeof(http_buffer), &http_buffer_length, 0);

          if (!error)
          {

            // Security Remove flag firmware wait... Start success download 
            if(param.system_status->flags.http_wait_fw) {
              param.systemStatusLock->Take();
              param.system_status->flags.http_wait_fw = false;
              param.systemStatusLock->Give();
            }

            #if (ENABLE_STACK_USAGE)
            TaskMonitorStack();
            #endif

            totBytesRead += http_buffer_length;

            // Read all entire buffer lenght without filter of CR/LF
            TRACE_INFO_F(F("Recived block of [ %d ] bytes, total downloaded [ %d ] bytes\r\n"), http_buffer_length, totBytesRead);

            // AddBlock Firmware to Queue -> and Put do SD
            bErrorFirmwareDownload |= do_firmware_add_block((uint8_t*)http_buffer, http_buffer_length);

          }
        }

        TaskState(state, 1, task_flag::normal); // Resume
      }

      // Terminate the HTTP response body with a CRLF
      TRACE_INFO_F(F("\r\n"));

      // Any error to report? With FW Request/Response OK
      if (is_get_firmware) {
        if (!bValidFirmwareRequest) {
          // Clean error 
          error = ERROR_END_OF_STREAM;
          TRACE_ERROR_F(F("%s Firmware module not avaiable on server RMAP, next checking...\r\n"), Thread::GetName().c_str());
        }
        else if (bErrorFirmwareDownload)
        {
          // Error if of queue/SD no more action here, exit and signal the problem to server (SD)
          is_error = true;
          state = HTTP_STATE_END;
          TRACE_ERROR_F(F("%s Failed to upload firmware stream to SD [ %s ]\r\n"), Thread::GetName().c_str(), ABORT_STRING);
          break;
        }
      }

      // Any error to report?
      if (error != ERROR_END_OF_STREAM)
      {
        if(++retry_get_response<HTTP_TASK_GENERIC_RETRY) {
          is_error = true;
          state = HTTP_STATE_END;
          TRACE_ERROR_F(F("%s Failed to parse http stream [ %s ] ABORT!!!\r\n"), Thread::GetName().c_str(), ERROR_STRING);
        } else {
          TaskWatchDog(HTTP_TASK_GENERIC_RETRY_DELAY_MS);
          Delay(Ticks::MsToTicks(HTTP_TASK_GENERIC_RETRY_DELAY_MS));
          TRACE_ERROR_F(F("%s Failed to parse http stream [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
        }
        break;
      }

      // Closing Queue and File data (Ready for next firmware...)
      if ((is_get_firmware)&&(bValidFirmwareRequest)) {
        bErrorFirmwareDownload |= do_firmware_end_data();
        // At least one firmware are downloade. Need to resynch version/revision to check avaiables version into SD card...
        bDownloadedFirmware = true;
      }

      // Close HTTP response body
      error = httpClientCloseBody(&httpClientContext);
      // Any error to report?
      if (error)
      {
        if(++retry_get_response<HTTP_TASK_GENERIC_RETRY) {
          is_error = true;
          state = HTTP_STATE_END;
          TRACE_ERROR_F(F("%s Failed to read http response trailer [ %s ] ABORT!!!\r\n"), Thread::GetName().c_str(), ERROR_STRING);
        } else {
          TaskWatchDog(HTTP_TASK_GENERIC_RETRY_DELAY_MS);
          Delay(Ticks::MsToTicks(HTTP_TASK_GENERIC_RETRY_DELAY_MS));
          TRACE_ERROR_F(F("%s Failed to read http response trailer [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
        }
        break;
      }

      // Gracefully disconnect from the HTTP server
      httpClientDisconnect(&httpClientContext);

      // End or next request (only for firmware download)
      if(is_get_firmware)
      {
        state = HTTP_STATE_LOOP_REQUEST_FIRMWARE;
        TRACE_VERBOSE_F(F("HTTP_STATE_GET_RESPONSE -> HTTP_STATE_LOOP_REQUEST_FIRMWARE\r\n"));
      }
      else
      {
        state = HTTP_STATE_END;
        TRACE_VERBOSE_F(F("HTTP_STATE_GET_RESPONSE -> HTTP_STATE_END\r\n"));
      }
      break;

    // Loop for module enabled and configure to check download firmware
    case HTTP_STATE_LOOP_REQUEST_FIRMWARE:
      while(true) {
        // ( From Master to first slave FF -> 00 start to first module )
        // Ending Module? End Of Procedure
        if(++module_download >= BOARDS_COUNT_MAX) break;
        if(param.system_status->data_slave[module_download].module_version) {
          TRACE_INFO_F(F("Http: Starting next firmware request download module: [ %d ] \r\n"), param.system_status->data_slave[module_download].module_type);
          // Here if comunication with slave module are established and module_version is Getted
          // Firmware download request can be start normally
          break;
        }
      }
      if(module_download >= BOARDS_COUNT_MAX) {
        // End of loop downloading firmware
        is_error = NO_ERROR;
        state = HTTP_STATE_END;
        TRACE_VERBOSE_F(F("HTTP_STATE_LOOP_REQUEST_FIRMWARE -> HTTP_STATE_END\r\n"));

        // Resynch firmware SD with avaiables ONLY!!! if new version are downloaded from server
        if (bDownloadedFirmware) {
          // After all download firmware module start queue request reload structure firmware
          // And waiting response. After start update all firmware board status info are resynch
          system_message_t system_message = {0};
          system_message.task_dest = SD_TASK_ID;
          system_message.command.do_reload_fw = true;
          param.systemMessageQueue->Enqueue(&system_message);

          // Waiting a response done before continue (reload status flag firmware OK!!!)
          while(true) {
            // Continuos Switching context non blocking
            // Need Waiting Task for start command on All used TASK
            taskYIELD();
            vTaskDelay(100);
            // Check response done
            if(!param.systemMessageQueue->IsEmpty()) {
              param.systemMessageQueue->Peek(&system_message);
              if(system_message.command.done_reload_fw) {
                // Remove message (Reload Done is OK)
                param.systemMessageQueue->Dequeue(&system_message);
                break;
              }
            }
          }
          // Reset version and revision (force reload difference after download firmware to verify)
          // If new version found, flag new_firmware_ready must be set to true on new connection...          
          param.systemStatusLock->Take();
          for (uint8_t i = 0; i < BOARDS_COUNT_MAX; i++) {
            param.system_status->data_slave[i].module_version = 0;
            param.system_status->data_slave[i].module_revision = 0;
          }
          param.systemStatusLock->Give();
        }
      } 
      else
      {
        // Deinit Current context and restart next
        httpClientDeinit(&httpClientContext);
        is_error = NO_ERROR;
        state = HTTP_STATE_SEND_REQUEST;
        TRACE_VERBOSE_F(F("HTTP_STATE_LOOP_REQUEST_FIRMWARE -> HTTP_STATE_SEND_REQUEST\r\n"));
      }
      break;

    case HTTP_STATE_END:
      // ok
      if (!is_error)
      {
        param.systemStatusLock->Take();
        param.system_status->flags.http_error = false;
        param.system_status->connection.is_http_configuration_updating = false;
        param.system_status->connection.is_http_configuration_updated = is_get_configuration;
        param.system_status->connection.is_http_firmware_upgrading = false;
        param.system_status->connection.is_http_firmware_upgraded = is_get_firmware;
        param.systemStatusLock->Give();

        httpClientDeinit(&httpClientContext);

        memset(&connection_response, 0, sizeof(connection_response_t));
        connection_response.done_http_configuration_getted = is_get_configuration;
        connection_response.error_http_configuration_getted = false;
        connection_response.done_http_firmware_getted = is_get_firmware;
        connection_response.error_http_firmware_getted = false;
        param.connectionResponseQueue->Enqueue(&connection_response);

        state = HTTP_STATE_INIT;
        TRACE_VERBOSE_F(F("HTTP_STATE_END -> HTTP_STATE_INIT\r\n"));
      }
      // retry (DNS Error is connection error than required forced reset connection)
      else if (((++retry) < HTTP_TASK_GENERIC_RETRY) && (!param.system_status->connection.is_dns_failed_resolve))
      {
        TaskWatchDog(HTTP_TASK_GENERIC_RETRY_DELAY_MS);
        Delay(Ticks::MsToTicks(HTTP_TASK_GENERIC_RETRY_DELAY_MS));

        TRACE_VERBOSE_F(F("HTTP_STATE_END -> HTTP_STATE_SEND_REQUEST\r\n"));
        state = HTTP_STATE_SEND_REQUEST;
      }
      // error
      else
      {
        param.systemStatusLock->Take();
        param.system_status->flags.http_error = true;
        param.system_status->connection.is_http_configuration_updating = false;
        param.system_status->connection.is_http_configuration_updated = false;
        param.system_status->connection.is_http_firmware_upgrading = false;
        param.system_status->connection.is_http_firmware_upgraded = false;
        param.systemStatusLock->Give();

        httpClientDeinit(&httpClientContext);

        memset(&connection_response, 0, sizeof(connection_response_t));
        connection_response.done_http_configuration_getted = false;
        connection_response.error_http_configuration_getted = is_get_configuration;
        connection_response.done_http_firmware_getted = false;
        connection_response.error_http_firmware_getted = is_get_firmware;
        param.connectionResponseQueue->Enqueue(&connection_response);

        state = HTTP_STATE_INIT;
        TRACE_VERBOSE_F(F("HTTP_STATE_END -> HTTP_STATE_INIT\r\n"));
      }
      break;

      // One step base non blocking switch
      TaskWatchDog(HTTP_TASK_WAIT_DELAY_MS);
      Delay(Ticks::MsToTicks(HTTP_TASK_WAIT_DELAY_MS));

    }
  }
}

/**
 * @brief TLS initialization callback
 * @param[in] context Pointer to the HTTP client context
 * @param[in] tlsContext Pointer to the TLS context
 * @return Error code
 **/
error_t HttpTask::httpClientTlsInitCallback(HttpClientContext *context, TlsContext *tlsContext)
{
  error_t error;

  // Debug message
  TRACE_INFO_F(F("HTTP Client TLS initialization callback\r\n"));

  // Set the PRNG algorithm to be used
  error = tlsSetPrng(tlsContext, YARROW_PRNG_ALGO, HttpYarrowContext);
  // Any error to report?
  if (error)
    return error;

  // Preferred cipher suite list
  error = tlsSetCipherSuites(tlsContext, HttpCipherSuites, arraysize(HttpCipherSuites));
  // Any error to report?
  if (error)
    return error;

  // Set the fully qualified domain name of the server
  error = tlsSetServerName(tlsContext, HttpServer);
  // Any error to report?
  if (error)
    return error;

  // Set the PSK identity to be used by the client
  error = tlsSetPskIdentity(tlsContext, HttpClientPSKIdentity);
  // Any error to report?
  if (error)
    return error;

  // Set the pre-shared key to be used
  error = tlsSetPsk(tlsContext, HttpClientPSKKey, CLIENT_PSK_KEY_LENGTH);
  // Any error to report?
  if (error)
    return error;

  // Successful processing
  return NO_ERROR;
}

/// @brief Init queue update firmware data block to SD Card (create file name on SD)
/// @param module_type module_type requested and found
/// @param version versione requested and found
/// @param revision revision requested and found
/// @return if error occurs return (true)
bool HttpTask::do_firmware_set_name(Module_Type module_type, uint8_t version, uint8_t revision)
{  
  bool set_file_name_error = false;

  // SD have to GET Ready before Push DATA
  // EXIT from function if not SD Ready or present into system_status
  if(!param.system_status->flags.sd_card_ready) {
    TRACE_VERBOSE_F(F("HTTP: Reject request upload file (Firmware) SD was not ready [ %s ]\r\n"), ERROR_STRING);
    return true;
  }

  // First block NAME OF FILE (Prepare name and Put to queue)
  memset(&firmwareDownloadChunck, 0, sizeof(file_put_request_t));
  firmwareDownloadChunck.block_type = file_block_type::file_name;
  // Chose one method to put name file (only name file without prefix directory)
  setStimaFirmwareName((char*)firmwareDownloadChunck.block, module_type, version, revision);
  // OR FILE NAME FROM TYPE... IF HTTP Responding with Module, Version and Revision...
  // setStimaFirmwareName((char*)firmwareDownloadChunck.block, STIMA_MODULE_TYPE_TH, 4, 3);
  firmwareDownloadChunck.block_lenght = strlen((char*)firmwareDownloadChunck.block);
  TRACE_VERBOSE_F(F("Starting upload file (Firmware) from remote HTTP to local SD [ %s ]\r\n"), firmwareDownloadChunck.block);
  // Push data request to queue SD
  param.dataFilePutRequestQueue->Enqueue(&firmwareDownloadChunck);

  // Waiting response from SD with TimeOUT
  memset(&sdcard_task_response, 0, sizeof(file_put_response_t));
  TaskWatchDog(FILE_IO_DATA_QUEUE_TIMEOUT);
  set_file_name_error = !param.dataFilePutResponseQueue->Dequeue(&sdcard_task_response, FILE_IO_DATA_QUEUE_TIMEOUT);
  set_file_name_error |= !sdcard_task_response.done_operation;

  return(set_file_name_error);
}

/// @brief Adda queue update firmware data block to SD Card
/// @param block_addr adrres buffer to add into SD Block data
/// @param block_len lenght of block to ADD
/// @return if error occurs return (true)
bool HttpTask::do_firmware_add_block(uint8_t *block_addr, uint16_t block_len) {
  bool file_upload_error = false;

  // SD have to GET Ready before Push DATA
  // EXIT from function if not SD Ready or present into system_status
  if(!param.system_status->flags.sd_card_ready) {
    TRACE_VERBOSE_F(F("HTTP: Reject request upload file (Firmware) SD was not ready [ %s ]\r\n"), ERROR_STRING);
    return true;
  }

  // Add Data Chunck...
  // Next block is data_chunk + Lenght to SET (in this all 512 bytes)
  firmwareDownloadChunck.block_type = file_block_type::data_chunck;
  memcpy((char*)firmwareDownloadChunck.block, (char*)block_addr, block_len);
  firmwareDownloadChunck.block_lenght = block_len;

  // Push data request to queue SD
  param.dataFilePutRequestQueue->Enqueue(&firmwareDownloadChunck);
  // Waiting response from SD with TimeOUT
  memset(&sdcard_task_response, 0, sizeof(file_put_response_t));
  TaskWatchDog(FILE_IO_DATA_QUEUE_TIMEOUT);
  file_upload_error = !param.dataFilePutResponseQueue->Dequeue(&sdcard_task_response, FILE_IO_DATA_QUEUE_TIMEOUT);
  file_upload_error |= !sdcard_task_response.done_operation;
  
  return(file_upload_error);
}

/// @brief Close queue update firmware data block to SD Card
/// @param  none
/// @return if error occurs return (true)
bool HttpTask::do_firmware_end_data(void) {
  bool file_upload_error = false;
  memset(&firmwareDownloadChunck, 0, sizeof(file_put_request_t));

  // SD have to GET Ready before Push DATA
  // EXIT from function if not SD Ready or present into system_status
  if(!param.system_status->flags.sd_card_ready) {
    TRACE_VERBOSE_F(F("HTTP: Reject request upload file (Firmware) SD was not ready [ %s ]\r\n"), ERROR_STRING);
    return true;
  }

  // Final Block (EOF, without checksum). If cecksum use file_block_type::end_of_file and put checksum Verify into block...
  firmwareDownloadChunck.block_type = file_block_type::end_of_file;
  // Push data request to queue SD
  param.dataFilePutRequestQueue->Enqueue(&firmwareDownloadChunck);

  // Waiting response from SD with TimeOUT
  memset(&sdcard_task_response, 0, sizeof(file_put_response_t));
  TaskWatchDog(FILE_IO_DATA_QUEUE_TIMEOUT);
  file_upload_error = !param.dataFilePutResponseQueue->Dequeue(&sdcard_task_response, FILE_IO_DATA_QUEUE_TIMEOUT);
  file_upload_error |= !sdcard_task_response.done_operation;

  // FLUSH Security Queue if any Error occurs (Otherwise queue are empty. Pull From TASK SD)
  TRACE_VERBOSE_F(F("HTTP: Uploading file (Firmware) [ %s ]\r\n"), file_upload_error ? ERROR_STRING : OK_STRING);
  return(file_upload_error);

}

#endif
