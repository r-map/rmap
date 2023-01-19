/**@file http_task.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>

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

#define TRACE_LEVEL HTTP_TASK_TRACE_LEVEL

#include "tasks/http_task.h"

using namespace cpp_freertos;

#if (USE_HTTP)

HttpTask::HttpTask(const char *taskName, uint16_t stackSize, uint8_t priority, HttpParam_t httpParam) : Thread(taskName, stackSize, priority), param(httpParam)
{
  // Start WDT controller and RunState Flags
  #if (ENABLE_WDT)
  WatchDog(param.system_status, param.systemStatusLock, WDT_TIMEOUT_BASE_US / 1000, false);
  RunState(param.system_status, param.systemStatusLock, RUNNING_START, false);
  #endif

  state = HTTP_STATE_INIT;
  Start();
};

// #define APP_HTTP_SERVER_PORT 80

// #define APP_HTTP_SERVER_NAME "test.rmap.cc"
// #define APP_HTTP_URI "/stations/userv4/stimav4/stima4/json/"

// #define APP_HTTP_SERVER_NAME "www.baldinetti.com"
// #define APP_HTTP_URI "/portfolio"

// error_t httpClientTest()
// {
//   HttpClientContext httpClientContext;
//   error_t error;
//   size_t length;
//   uint_t status;
//   const char_t *value;
//   IpAddr ipAddr;
//   char_t buffer[128];

//   // Initialize HTTP client context
//   httpClientInit(&httpClientContext);

//   // Start of exception handling block
//   do
//   {
//     // Debug message
//     TRACE_INFO("\r\n\r\nResolving server name...\r\n");

//     // Resolve HTTP server name
//     error = getHostByName(NULL, APP_HTTP_SERVER_NAME, &ipAddr, 0);
//     // Any error to report?
//     if (error)
//     {
//       // Debug message
//       TRACE_INFO("Failed to resolve server name!\r\n");
//       break;
//     }

//     // Select HTTP protocol version
//     error = httpClientSetVersion(&httpClientContext, HTTP_VERSION_1_1);
//     // Any error to report?
//     if (error)
//       break;

//     // Set timeout value for blocking operations
//     error = httpClientSetTimeout(&httpClientContext, 20000);
//     // Any error to report?
//     if (error)
//       break;

//     // Debug message
//     TRACE_INFO("Connecting to HTTP server %s...\r\n",
//                ipAddrToString(&ipAddr, NULL));

//     // Connect to the HTTP server
//     error = httpClientConnect(&httpClientContext, &ipAddr,
//                               APP_HTTP_SERVER_PORT);
//     // Any error to report?
//     if (error)
//     {
//       // Debug message
//       TRACE_INFO("Failed to connect to HTTP server!\r\n");
//       break;
//     }

//     // Create an HTTP request
//     httpClientCreateRequest(&httpClientContext);
//     httpClientSetMethod(&httpClientContext, "GET");
//     httpClientSetUri(&httpClientContext, APP_HTTP_URI);

//     // Set query string
//     // httpClientAddQueryParam(&httpClientContext, "", "");
//     // httpClientAddQueryParam(&httpClientContext, "param2", "value2");

//     // Add HTTP header fields
//     httpClientAddHeaderField(&httpClientContext, "Host", APP_HTTP_SERVER_NAME);
//     // httpClientAddHeaderField(&httpClientContext, "Connection", "keep-alive");
//     // httpClientAddHeaderField(&httpClientContext, "Content-Type", "text/plain");
//     // httpClientAddHeaderField(&httpClientContext, "Cache-Control", "max-age=0");
//     // httpClientAddHeaderField(&httpClientContext, "Accept", "*/*");
//     // httpClientAddHeaderField(&httpClientContext, "Accept-Encoding", "text/plain");
//     httpClientAddHeaderField(&httpClientContext, "User-Agent", "master/1.0");
//     // httpClientAddHeaderField(&httpClientContext, "Transfer-Encoding", "chunked");

//     // Send HTTP request header
//     error = httpClientWriteHeader(&httpClientContext);
//     // Any error to report?
//     if (error)
//     {
//       // Debug message
//       TRACE_INFO("Failed to write HTTP request header!\r\n");
//       break;
//     }

//     // Send HTTP request body
//     // error = httpClientWriteBody(&httpClientContext, "", 0, NULL, 0);
//     // // // Any error to report?
//     // if (error)
//     // {
//     //   // Debug message
//     //   TRACE_INFO("Failed to write HTTP request body!\r\n");
//     //   break;
//     // }

//     // Receive HTTP response header
//     error = httpClientReadHeader(&httpClientContext);
//     // Any error to report?
//     if (error)
//     {
//       // Debug message
//       TRACE_INFO("Failed to read HTTP response header!\r\n");
//       break;
//     }

//     // Retrieve HTTP status code
//     status = httpClientGetStatus(&httpClientContext);
//     // Debug message
//     TRACE_INFO("HTTP status code: %u\r\n", status);

//     // Retrieve the value of the Content-Type header field
//     value = httpClientGetHeaderField(&httpClientContext, "Content-Type");

//     // Header field found?
//     if (value != NULL)
//     {
//       // Debug message
//       TRACE_INFO("Content-Type header field value: %s\r\n", value);
//     }
//     else
//     {
//       // Debug message
//       TRACE_INFO("Content-Type header field not found!\r\n");
//     }

//     // Receive HTTP response body
//     while (!error)
//     {
//       // Read data
//       error = httpClientReadBody(&httpClientContext, buffer,
//                                  sizeof(buffer) - 1, &length, 0);

//       // Check status code
//       if (!error)
//       {
//         // Properly terminate the string with a NULL character
//         buffer[length] = '\0';
//         // Dump HTTP response body
//         TRACE_INFO("%s", buffer);
//       }
//     }

//     // Terminate the HTTP response body with a CRLF
//     TRACE_INFO("\r\n");

//     // Any error to report?
//     if (error != ERROR_END_OF_STREAM)
//       break;

//     // Close HTTP response body
//     error = httpClientCloseBody(&httpClientContext);
//     // Any error to report?
//     if (error)
//     {
//       // Debug message
//       TRACE_INFO("Failed to read HTTP response trailer!\r\n");
//       break;
//     }

//     // Gracefully disconnect from the HTTP server
//     httpClientDisconnect(&httpClientContext);

//     // Debug message
//     TRACE_INFO("Connection closed\r\n");

//     // End of exception handling block
//   } while (0);

//   // Release HTTP client context
//   httpClientDeinit(&httpClientContext);

//   // Return status code
//   return error;
// }

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
void HttpTask::monitorStack(system_status_t *status, BinarySemaphore *lock)
{
  u_int16_t stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark( NULL );
  if((stackUsage) && (stackUsage < status->tasks[HTTP_TASK_ID].stack)) {
    if(lock != NULL) lock->Take();
    status->tasks[HTTP_TASK_ID].stack = stackUsage;
    if(lock != NULL) lock->Give();
  }
}
#endif

#if (ENABLE_WDT)
/// @brief local watchDog and Sleep flag Task (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void HttpTask::WatchDog(system_status_t *status, BinarySemaphore *lock, uint16_t millis_standby, bool is_sleep)
{
  // Local WatchDog update
  if(lock != NULL) lock->Take();
  // Signal Task sleep/disabled mode from request
  status->tasks[HTTP_TASK_ID].is_sleep = is_sleep;
  if((millis_standby) < (WDT_TIMEOUT_BASE_US / 1000))
    status->tasks[HTTP_TASK_ID].watch_dog = wdt_flag::set;
  else
    status->tasks[HTTP_TASK_ID].watch_dog = wdt_flag::rest;
  if(lock != NULL) lock->Give();
}

/// @brief local suspend flag and positor running state Task (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param state_position Sw_Poition (STandard 1 RUNNING_START, 2 RUNNING_EXEC, XX User define SW POS fo Logging)
/// @param is_suspend TRUE if task enter suspending mode (Disable from WDT Controller)
void HttpTask::RunState(system_status_t *status, BinarySemaphore *lock, uint8_t state_position, bool is_suspend)
{
  // Local WatchDog update
  if(lock != NULL) lock->Take();
  // Signal Task sleep/disabled mode from request
  status->tasks[HTTP_TASK_ID].is_suspend = is_suspend;
  status->tasks[HTTP_TASK_ID].running_pos = state_position;
  if(lock != NULL) lock->Give();
}
#endif


void HttpTask::Run() {
  uint8_t retry;
  bool is_error;
  error_t error;
  IpAddr ipAddr;
  uint_t status;
  const char_t *value;

  char uri[HTTP_URI_LENGTH];
  char user_agents[HTTP_USER_AGENTS_LENGTH];

  bool is_get_configuration;
  bool is_get_firmware;

  system_request_t request;
  system_response_t response;

  #if (ENABLE_STACK_USAGE)
  monitorStack(param.system_status, param.systemStatusLock);
  #endif

  while (true)
  {
    memset(&request, 0, sizeof(system_request_t));
    memset(&response, 0, sizeof(system_response_t));

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
      if (param.systemRequestQueue->Peek(&request, portMAX_DELAY))
      {
        // do http get configuration
        if (request.connection.do_http_get_configuration)
        {
          is_get_configuration = true;
          param.systemRequestQueue->Dequeue(&request, 0);
          state = HTTP_STATE_SEND_REQUEST;
          TRACE_VERBOSE_F(F("HTTP_STATE_WAIT_NET_EVENT -> HTTP_STATE_SEND_REQUEST\r\n"));
        }
        // do http get firmware
        else if (request.connection.do_http_get_firmware)
        {
          is_get_firmware = true;
          param.systemRequestQueue->Dequeue(&request, 0);
          state = HTTP_STATE_SEND_REQUEST;
          TRACE_VERBOSE_F(F("HTTP_STATE_WAIT_NET_EVENT -> HTTP_STATE_SEND_REQUEST\r\n"));
        }
        else
        {
          Delay(Ticks::MsToTicks(HTTP_TASK_WAIT_DELAY_MS));
        }
      }
      // do something else with non-blocking wait ....
      break;

    case HTTP_STATE_SEND_REQUEST:
      httpClientInit(&httpClientContext);

      param.systemStatusLock->Take();
      param.system_status->connection.is_http_configuration_updating = is_get_configuration;
      param.system_status->connection.is_http_firmware_upgrading = is_get_firmware;
      param.systemStatusLock->Give();

      TRACE_INFO_F(F("%s Resolving http server name of %s \r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server);

      // Resolve HTTP server name
      error = getHostByName(NULL, param.configuration->mqtt_server, &ipAddr, 0);
      // Any error to report?
      if (error)
      {
        is_error = true;
        state = HTTP_STATE_END;
        TRACE_VERBOSE_F(F("HTTP_STATE_SEND_REQUEST -> HTTP_STATE_END\r\n"));

        TRACE_ERROR_F(F("%s Failed to resolve http server name of %s [ %s ]\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, ERROR_STRING);
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

      // Connect to the HTTP server
      error = httpClientConnect(&httpClientContext, &ipAddr, HTTP_PORT);
      // Any error to report?
      if (error)
      {
        is_error = true;
        state = HTTP_STATE_END;
        TRACE_VERBOSE_F(F("HTTP_STATE_SEND_REQUEST -> HTTP_STATE_END\r\n"));

        TRACE_ERROR_F(F("%s Failed to connect to http server %s [ %s ]\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, ERROR_STRING);
        break;
      }

      // Create an HTTP request
      httpClientCreateRequest(&httpClientContext);
      httpClientSetMethod(&httpClientContext, "GET");

      if (is_get_configuration)
      {
        snprintf(uri, sizeof(uri), "/stations/%s/%s/%s/json/", param.configuration->mqtt_username, param.configuration->stationslug, param.configuration->boardslug);
      }
      else if (is_get_firmware)
      {
        snprintf(uri, sizeof(uri), "/stations/%s/%s/%s/?", param.configuration->mqtt_username, param.configuration->stationslug, param.configuration->boardslug);
      }

      TRACE_INFO_F(F("%s http request to %s%s\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, uri);

      httpClientSetUri(&httpClientContext, uri);

      // Set query string
      // httpClientAddQueryParam(&httpClientContext, "param1", "value1");
      // httpClientAddQueryParam(&httpClientContext, "param2", "value2");

      // Add HTTP header fields
      httpClientAddHeaderField(&httpClientContext, "Host", param.configuration->mqtt_server);

      getStimaNameByType(user_agents, param.configuration->module_type);
      snprintf(user_agents, sizeof(user_agents), "%s/%d.%d", user_agents, param.configuration->module_main_version, param.configuration->module_minor_version);
      httpClientAddHeaderField(&httpClientContext, "User-Agent", user_agents);

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

      state = HTTP_STATE_GET_RESPONSE;
      TRACE_VERBOSE_F(F("HTTP_STATE_SEND_REQUEST -> HTTP_STATE_GET_RESPONSE\r\n"));
      break;

    case HTTP_STATE_GET_RESPONSE:
      // Receive HTTP response header
      error = httpClientReadHeader(&httpClientContext);
      // Any error to report?
      if (error)
      {
        TRACE_ERROR_F(F("%s Failed to read http response header [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
        break;
      }

      // Retrieve HTTP status code
      status = httpClientGetStatus(&httpClientContext);
      TRACE_ERROR_F(F("%s http status code %u\r\n"), Thread::GetName().c_str(), status);

      // Retrieve the value of the Content-Type header field
      value = httpClientGetHeaderField(&httpClientContext, "Content-Type");

      // Header field found?
      if (value == NULL)
      {
        TRACE_ERROR_F(F("%s Content-Type header field not found [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
      }

      // Receive HTTP response body
      while (!error)
      {
        if (is_get_configuration)
        {
          // change http_buffer with configuration buffer
        }
        else if (is_get_firmware)
        {
          // change http_buffer with firmware buffer
        }
        
        error = httpClientReadBody(&httpClientContext, http_buffer, sizeof(http_buffer) - 1, &http_buffer_length, 0);
        if (!error)
        {
          // Properly terminate the string with a NULL character
          http_buffer[http_buffer_length] = '\0';
          TRACE_INFO_F(F("%s"), http_buffer);
        }
      }

      // Terminate the HTTP response body with a CRLF
      TRACE_INFO_F(F("\r\n"));

      // Any error to report?
      if (error != ERROR_END_OF_STREAM)
      {
        TRACE_ERROR_F(F("%s Failed to parse http stream [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
        break;
      }

      // Close HTTP response body
      error = httpClientCloseBody(&httpClientContext);
      // Any error to report?
      if (error)
      {
        TRACE_ERROR_F(F("%s Failed to read http response trailer [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
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
        response.connection.done_http_configuration_getted = is_get_configuration;
        response.connection.done_http_firmware_getted = is_get_firmware;
        param.systemResponseQueue->Enqueue(&response, 0);

        param.systemStatusLock->Take();
        param.system_status->connection.is_http_configuration_updating = false;
        param.system_status->connection.is_http_configuration_updated = is_get_configuration;
        param.system_status->connection.is_http_firmware_upgrading = false;
        param.system_status->connection.is_http_firmware_upgraded = is_get_firmware;
        param.systemStatusLock->Give();

        httpClientDeinit(&httpClientContext);
        state = HTTP_STATE_INIT;
        TRACE_VERBOSE_F(F("HTTP_STATE_END -> HTTP_STATE_INIT\r\n"));
      }
      // retry
      else if ((++retry) < HTTP_TASK_GENERIC_RETRY)
      {
        Delay(Ticks::MsToTicks(HTTP_TASK_GENERIC_RETRY_DELAY_MS));

        TRACE_VERBOSE_F(F("HTTP_STATE_END -> HTTP_STATE_SEND_REQUEST\r\n"));
        state = HTTP_STATE_SEND_REQUEST;
      }
      // error
      else
      {
        response.connection.done_http_configuration_getted = is_get_configuration;
        response.connection.done_http_firmware_getted = is_get_firmware;
        param.systemResponseQueue->Enqueue(&response, 0);

        param.systemStatusLock->Take();
        param.system_status->connection.is_http_configuration_updating = false;
        param.system_status->connection.is_http_configuration_updated = false;
        param.system_status->connection.is_http_firmware_upgrading = false;
        param.system_status->connection.is_http_firmware_upgraded = false;
        param.systemStatusLock->Give();

        httpClientDeinit(&httpClientContext);
        state = HTTP_STATE_INIT;
        TRACE_VERBOSE_F(F("HTTP_STATE_END -> HTTP_STATE_INIT\r\n"));
      }
      break;
    }
  }
}

#endif