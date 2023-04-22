/**@file http_task.cpp */

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
  // std::string serial_number_str;
  char_t serial_number_str[12];
  uint32_t serial_number_l;
  uint32_t serial_number_h;

  char uri[HTTP_URI_LENGTH];
  char header[HTTP_HEADER_SIZE];
  char module_type[STIMA_MODULE_NAME_LENGTH];

  bool is_get_configuration;
  bool is_get_firmware;

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

        // do http get configuration
        if (connection_request.do_http_get_configuration)
        {
          is_get_configuration = true;
          param.connectionRequestQueue->Dequeue(&connection_request, 0);
          state = HTTP_STATE_SEND_REQUEST;
          TRACE_VERBOSE_F(F("HTTP_STATE_WAIT_NET_EVENT -> HTTP_STATE_SEND_REQUEST\r\n"));
        }
        // do http get firmware
        else if (connection_request.do_http_get_firmware)
        {
          is_get_firmware = true;
          param.connectionRequestQueue->Dequeue(&connection_request, 0);
          state = HTTP_STATE_SEND_REQUEST;
          TRACE_VERBOSE_F(F("HTTP_STATE_WAIT_NET_EVENT -> HTTP_STATE_SEND_REQUEST\r\n"));
        }
      }
      break;

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
        state = HTTP_STATE_END;
        TRACE_VERBOSE_F(F("HTTP_STATE_SEND_REQUEST -> HTTP_STATE_END\r\n"));

        TRACE_ERROR_F(F("%s Failed to resolve http server name of %s [ %s ]\r\n"), Thread::GetName().c_str(), HttpServer, ERROR_STRING);
        break;
      }

      // Shared Pointer
      HttpYarrowContext = param.yarrowContext;
      HttpClientPSKKey = param.configuration->client_psk_key;

      #if (ENABLE_STACK_USAGE)
      TaskMonitorStack();
      #endif

      // Set PSK identity
      snprintf(HttpClientPSKIdentity, sizeof(HttpClientPSKIdentity), "%s/%s/%s", param.configuration->mqtt_username, param.configuration->stationslug, param.configuration->boardslug);
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

      #if (ENABLE_STACK_USAGE)
      TaskMonitorStack();
      #endif

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

      #if (ENABLE_STACK_USAGE)
      TaskMonitorStack();
      #endif

      if (is_get_configuration)
      {
        snprintf(uri, sizeof(uri), "/stationconfig/%s/%s", param.configuration->mqtt_username, param.configuration->stationslug);
      }
      else if (is_get_firmware)
      {
        snprintf(uri, sizeof(uri), "/firmware/stima/v4/update/%u/", param.configuration->module_type);
      }

      TRACE_INFO_F(F("%s http request to %s%s\r\n"), Thread::GetName().c_str(), HttpServer, uri);

      httpClientSetUri(&httpClientContext, uri);

      // Set query string
      // httpClientAddQueryParam(&httpClientContext, "param1", "value1");
      // httpClientAddQueryParam(&httpClientContext, "param2", "value2");

      // Add HTTP header fields
      httpClientAddHeaderField(&httpClientContext, "Host", HttpServer);

      #if (ENABLE_STACK_USAGE)
      TaskMonitorStack();
      #endif

      // from uint64_t to string
      // TODO: SERIALNUMBER: OGGI
      serial_number_l = param.configuration->board_master.serial_number & 0xFFFFFFFF;
      serial_number_h = (param.configuration->board_master.serial_number >> 32) & 0xFFFFFFFF;

      snprintf(serial_number_str, sizeof(serial_number_str), "%04X%04X", serial_number_h, serial_number_l);

      if (is_get_firmware)
      {
        snprintf(header, sizeof(header), "{\"version\": %d,\"revision\": %d,\"user\":\"%s\",\"slug\":\"%s\",\"bslug\":\"%s\"}", param.configuration->module_main_version, param.configuration->module_minor_version, param.configuration->mqtt_username, param.configuration->stationslug, param.configuration->boardslug);
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

      // // Send HTTP request body
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
      #if (ENABLE_STACK_USAGE)
      TaskMonitorStack();
      #endif

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
      // TODO: OGGI 304 NO FIRMWARE NUOVO
      // 300 VERSIONE NON CORRETA
      // 500 FIRMWARE NON ESISTE
      // 403 HEADER NON CORRETTO
      // 200 O ALTRI OK....

      TRACE_ERROR_F(F("%s http status code %u\r\n"), Thread::GetName().c_str(), status);

      // Retrieve the value of the Content-Type header field
      value = httpClientGetHeaderField(&httpClientContext, "x-MD5");
      // TODO: CHECK HEADER
      // https://test.rmap.cc/admin/firmware_updater_stima/firmware/

      // curl -v  -H "X-STIMA4-VERSION: {\"version\": 4,\"revision\": 
      // 0,\"user\":\"userv4\",\"slug\":\"stimacan\",\"bslug\":\"stimav4\"}" -H
      // "X-STIMA4-BOARD-MAC: 101" -A "STIMA4-http-Update"  
      // http://test.rmap.cc/firmware/stima/v4/update/11/ --output firmware

      // 


      #if (ENABLE_STACK_USAGE)
      TaskMonitorStack();
      #endif

      // Header field found?
      if (value == NULL)
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

      // Receive HTTP response body
      while (!error)
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
            http_buffer[http_buffer_length] = '\0';
            TRACE_INFO_F(F("%s"), http_buffer);
          }

          if (param.rpcLock->Take(Ticks::MsToTicks(RPC_WAIT_DELAY_MS)))
          {
            while (is_event_rpc)
            {
              #if (ENABLE_STACK_USAGE)
              TaskMonitorStack();
              #endif
              param.streamRpc->parseCharpointer(&is_event_rpc, (char *)http_buffer, http_buffer_length, NULL, 0, RPC_TYPE_HTTPS);
            }
            param.rpcLock->Give();
          }
        }
        else if (is_get_firmware)
        {
          error = httpClientReadBody(&httpClientContext, http_buffer, sizeof(http_buffer) - 1, &http_buffer_length, 0);

          if (!error)
          {

            #if (ENABLE_STACK_USAGE)
            TaskMonitorStack();
            #endif

            http_buffer[http_buffer_length] = '\0';
            TRACE_INFO_F(F("%s"), http_buffer);

            //TODO: PUT INTO QUEUE
            //OGGI

          }
        }

        TaskState(state, 1, task_flag::normal); // Resume
      }

      // Terminate the HTTP response body with a CRLF
      TRACE_INFO_F(F("\r\n"));

// TODO: OGGI ENDO OF STREAM CHIUDO CODA
// MD5 FILE????

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

      state = HTTP_STATE_END;
      TRACE_VERBOSE_F(F("HTTP_STATE_GET_RESPONSE -> HTTP_STATE_END\r\n"));
      break;

    case HTTP_STATE_END:
      // ok
      if (!is_error)
      {
        param.systemStatusLock->Take();
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
        param.connectionResponseQueue->Enqueue(&connection_response, 0);

        state = HTTP_STATE_INIT;
        TRACE_VERBOSE_F(F("HTTP_STATE_END -> HTTP_STATE_INIT\r\n"));
      }
      // retry
      else if ((++retry) < HTTP_TASK_GENERIC_RETRY)
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
        param.connectionResponseQueue->Enqueue(&connection_response, 0);

        state = HTTP_STATE_INIT;
        TRACE_VERBOSE_F(F("HTTP_STATE_END -> HTTP_STATE_INIT\r\n"));
      }
      break;

      #if (ENABLE_STACK_USAGE)
      TaskMonitorStack();
      #endif

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

  // // MMC have to GET Ready before Push DATA
  // // EXIT from function if not MMC Ready or present into system_status
  // if(!param.system_status->flags.sd_card_ready) {
  //   TRACE_VERBOSE_F(F("SUPERVISOR: Reject request upload file (Firmware) MMC was not ready [ %s ]\r\n"), ERROR_STRING);
  //   break;
  // }

void HttpTask::do_firmware(void) {
  file_put_request_t firmwareDownloadChunck;
  file_put_response_t sdcard_task_response;
  bool file_upload_error = false;

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
  TaskWatchDog(HTTP_TASK_WAIT_DELAY_MS);
  Delay(Ticks::MsToTicks(HTTP_TASK_WAIT_DELAY_MS));

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
      TaskWatchDog(HTTP_TASK_WAIT_DELAY_MS);
      Delay(Ticks::MsToTicks(HTTP_TASK_WAIT_DELAY_MS));
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

#endif