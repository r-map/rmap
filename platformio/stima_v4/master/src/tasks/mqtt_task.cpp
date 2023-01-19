/**@file mqtt_task.cpp */

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

#define TRACE_LEVEL MQTT_TASK_TRACE_LEVEL

#include "tasks/mqtt_task.h"

using namespace cpp_freertos;

#if (USE_MQTT)

YarrowContext *MqttYarrowContext;

// Client's PSK key
uint8_t *MqttClientPSKKey;

// Client's PSK identity
char_t MqttClientPSKIdentity[CLIENT_PSK_IDENTITY_LENGTH];

char_t *MqttServer;

// //Client's PSK identity
// #define APP_CLIENT_PSK_IDENTITY "userv4/stimav4/stima4"

// //Client's PSK
// const uint8_t MqttClientPSKKey[] = {0x4F, 0x3E, 0x7E, 0x10, 0xD2, 0xD1, 0x6A, 0xE2, 0xC5, 0xAC, 0x60, 0x12, 0x0F, 0x07, 0xEF, 0xAF};

//List of preferred ciphersuites
//https://ciphersuite.info/cs/?security=recommended&singlepage=true&page=2&tls=all&sort=asc
const uint16_t cipherSuites[] =
{
  // rmap server psk ciphers
  TLS_PSK_WITH_AES_256_CCM                      // WEAK BUT WORK
  // TLS_DHE_PSK_WITH_AES_128_GCM_SHA256            // RECOMMENDED BUT NOT WORK
  // TLS_DHE_PSK_WITH_AES_256_GCM_SHA384            // RECOMMENDED BUT NOT WORK
  // TLS_ECDHE_PSK_WITH_CHACHA20_POLY1305_SHA256    // RECOMMENDED BUT NOT WORK
  // TLS_DHE_PSK_WITH_CHACHA20_POLY1305_SHA256      // RECOMMENDED BUT NOT WORK

  // TLS_PSK_WITH_AES_256_CBC_SHA,            // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_PSK_WITH_AES_256_GCM_SHA384,         // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384,   // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA,      // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_DHE_PSK_WITH_AES_256_CBC_SHA384,     // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_PSK_WITH_AES_256_CBC_SHA384,         // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_DHE_PSK_WITH_AES_128_GCM_SHA256,     // RECOMMENDED BUT NOT WORK
  // TLS_PSK_WITH_AES_128_GCM_SHA256,         // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256,   // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA,      // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_DHE_PSK_WITH_AES_128_CBC_SHA256,     // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_DHE_PSK_WITH_AES_128_CBC_SHA,        // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_PSK_WITH_AES_128_CBC_SHA256,         // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_PSK_WITH_AES_128_CBC_SHA             // WEAK BUT NOT WORK (PREVIOUSLY WORK)

  // Recommended psk ciphers
  // TLS_DHE_PSK_WITH_AES_128_GCM_SHA256,
  // TLS_DHE_PSK_WITH_AES_256_GCM_SHA384,
  // TLS_DHE_PSK_WITH_CAMELLIA_128_GCM_SHA256,
  // TLS_DHE_PSK_WITH_CAMELLIA_256_GCM_SHA384,
  // TLS_DHE_PSK_WITH_ARIA_128_GCM_SHA256,
  // TLS_DHE_PSK_WITH_ARIA_256_GCM_SHA384,
  // TLS_DHE_PSK_WITH_CHACHA20_POLY1305_SHA256,
  // TLS_ECDHE_PSK_WITH_AES_128_GCM_SHA256,
  // TLS_ECDHE_PSK_WITH_AES_256_GCM_SHA384,
  // TLS_ECDHE_PSK_WITH_CHACHA20_POLY1305_SHA256
};

MqttTask::MqttTask(const char *taskName, uint16_t stackSize, uint8_t priority, MqttParam_t mqttParam) : Thread(taskName, stackSize, priority), param(mqttParam)
{
  // Start WDT controller and RunState Flags
  #if (ENABLE_WDT)
  WatchDog(param.system_status, param.systemStatusLock, WDT_TIMEOUT_BASE_US / 1000, false);
  RunState(param.system_status, param.systemStatusLock, RUNNING_START, false);
  #endif

  state = MQTT_STATE_INIT;
  version = MQTT_VERSION_3_1_1;
  transportProtocol = MQTT_TRANSPORT_PROTOCOL_TLS;
  qos = MQTT_QOS_LEVEL_1;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
void MqttTask::monitorStack(system_status_t *status, BinarySemaphore *lock)
{
  u_int16_t stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark( NULL );
  if((stackUsage) && (stackUsage < status->tasks[MQTT_TASK_ID].stack)) {
    if(lock != NULL) lock->Take();
    status->tasks[MQTT_TASK_ID].stack = stackUsage;
    if(lock != NULL) lock->Give();
  }
}
#endif

#if (ENABLE_WDT)
/// @brief local watchDog and Sleep flag Task (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void MqttTask::WatchDog(system_status_t *status, BinarySemaphore *lock, uint16_t millis_standby, bool is_sleep)
{
  // Local WatchDog update
  if(lock != NULL) lock->Take();
  // Signal Task sleep/disabled mode from request
  status->tasks[MQTT_TASK_ID].is_sleep = is_sleep;
  if((millis_standby) < (WDT_TIMEOUT_BASE_US / 1000))
    status->tasks[MQTT_TASK_ID].watch_dog = wdt_flag::set;
  else
    status->tasks[MQTT_TASK_ID].watch_dog = wdt_flag::rest;
  if(lock != NULL) lock->Give();
}

/// @brief local suspend flag and positor running state Task (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param state_position Sw_Poition (STandard 1 RUNNING_START, 2 RUNNING_EXEC, XX User define SW POS fo Logging)
/// @param is_suspend TRUE if task enter suspending mode (Disable from WDT Controller)
void MqttTask::RunState(system_status_t *status, BinarySemaphore *lock, uint8_t state_position, bool is_suspend)
{
  // Local WatchDog update
  if(lock != NULL) lock->Take();
  // Signal Task sleep/disabled mode from request
  status->tasks[MQTT_TASK_ID].is_suspend = is_suspend;
  status->tasks[MQTT_TASK_ID].running_pos = state_position;
  if(lock != NULL) lock->Give();
}
#endif


void MqttTask::Run()
{
  uint8_t retry;
  bool is_error;
  error_t error;
  system_request_t request;
  system_response_t response;
  IpAddr ipAddr;

  // Starting Task and first WDT (if required and enabled. Time < than WDT_TIMEOUT_BASE_US)
  #if (ENABLE_WDT)
  RunState(param.system_status, param.systemStatusLock, RUNNING_EXEC, false);
  #endif
  #if (ENABLE_STACK_USAGE)
  monitorStack(param.system_status, param.systemStatusLock);
  #endif

  while (true)
  {
    memset(&request, 0, sizeof(system_request_t));
    memset(&response, 0, sizeof(system_response_t));

    switch (state)
    {
    case MQTT_STATE_INIT:
      state = MQTT_STATE_WAIT_NET_EVENT;
      TRACE_VERBOSE_F(F("MQTT_STATE_INIT -> MQTT_STATE_WAIT_NET_EVENT\r\n"));
      break;

    case MQTT_STATE_WAIT_NET_EVENT:
      retry = 0;
      is_error = false;

      // wait connection request
      if (param.systemRequestQueue->Peek(&request, portMAX_DELAY))
      {
        // do mqtt connection
        if (request.connection.do_mqtt_connect)
        {
          MqttServer = param.configuration->mqtt_server;
          // strSafeCopy(MqttServer, param.configuration->mqtt_server, MQTT_SERVER_LENGTH);
          
          param.systemRequestQueue->Dequeue(&request, 0);
          TRACE_VERBOSE_F(F("MQTT_STATE_WAIT_NET_EVENT -> MQTT_STATE_CONNECT\r\n"));
          state = MQTT_STATE_CONNECT;
        }
        // do disconnect
        else if (request.connection.do_mqtt_disconnect)
        {
          param.systemRequestQueue->Dequeue(&request, 0);
          TRACE_VERBOSE_F(F("MQTT_STATE_WAIT_NET_EVENT -> MQTT_STATE_DISCONNECT\r\n"));
          state = MQTT_STATE_DISCONNECT;
          Suspend();
        }
        // other
        else
        {
          Delay(Ticks::MsToTicks(MQTT_TASK_WAIT_DELAY_MS));
        }
      }
      // do something else with non-blocking wait ....
      break;

    case MQTT_STATE_CONNECT:
      error = mqttClientInit(&mqttClientContext);
      if (error)
      {
        TRACE_ERROR_F(F("%s, Failed to initialize MQTT client [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
      }
      
      param.systemStatusLock->Take();
      param.system_status->connection.is_mqtt_connecting = true;
      param.systemStatusLock->Give();

      TRACE_INFO_F(F("%s Resolving mqtt server name of %s\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server);
      // Resolve MQTT server name
      error = getHostByName(NULL, param.configuration->mqtt_server, &ipAddr, 0);
      if (error)
      {
        is_error = true;

        TRACE_ERROR_F(F("%s Failed to resolve mqtt server name of %s [ %s ]\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, ERROR_STRING);

        state = MQTT_STATE_DISCONNECT;
        TRACE_VERBOSE_F(F("MQTT_STATE_CONNECT -> MQTT_STATE_DISCONNECT\r\n"));
        break;
      }

      // Set the MQTT version to be used
      mqttClientSetVersion(&mqttClientContext, version);

      if (transportProtocol == MQTT_TRANSPORT_PROTOCOL_TLS)
      {
        // Shared Pointer
        MqttYarrowContext = param.yarrowContext;
        MqttClientPSKKey = param.configuration->client_psk_key;

        // Set PSK identity
        snprintf(MqttClientPSKIdentity, sizeof(MqttClientPSKIdentity), "%s/%s/%s", param.configuration->mqtt_username, param.configuration->stationslug, param.configuration->boardslug);

        // MQTT over TLS
        mqttClientSetTransportProtocol(&mqttClientContext, MQTT_TRANSPORT_PROTOCOL_TLS);
        // Register TLS initialization callback
        mqttClientRegisterTlsInitCallback(&mqttClientContext, mqttTlsInitCallback);
      }

      // Register publish callback function
      mqttClientRegisterPublishCallback(&mqttClientContext, mqttPublishCallback);

      // Set communication timeout
      mqttClientSetTimeout(&mqttClientContext, MQTT_TIMEOUT_MS);
      // Set keep-alive value
      mqttClientSetKeepAlive(&mqttClientContext, MQTT_KEEP_ALIVE_S);

      // Set client identifier
      snprintf(clientIdentifier, sizeof(clientIdentifier), "%s/%s/%s", param.configuration->mqtt_username, param.configuration->stationslug, param.configuration->boardslug);
      mqttClientSetIdentifier(&mqttClientContext, clientIdentifier);

      // Set username and password
      if (strlen(param.configuration->mqtt_username) && strlen(param.configuration->mqtt_password))
      {
        mqttClientSetAuthInfo(&mqttClientContext, param.configuration->mqtt_username, param.configuration->mqtt_password);
      }

      // Set Will message
      if (strlen(MQTT_ON_ERROR_MESSAGE))
      {
        snprintf(topic, sizeof(topic), "%d/%s/%s/%s/%07d,%07d/%s", RMAP_PROCOTOL_VERSION, DATA_LEVEL_MAINT, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network);
        mqttClientSetWillMessage(&mqttClientContext, topic, MQTT_ON_ERROR_MESSAGE, strlen(MQTT_ON_ERROR_MESSAGE), qos, true);
      }

      // Establish connection with the MQTT server
      error = mqttClientConnect(&mqttClientContext, &ipAddr, param.configuration->mqtt_port, false);
      // Any error to report?
      if (error)
      {
        is_error = true;

        TRACE_ERROR_F(F("%s Failed to connect to mqtt server %s [ %s ]\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, ERROR_STRING);

        TRACE_VERBOSE_F(F("MQTT_STATE_CONNECT -> MQTT_STATE_DISCONNECT\r\n"));
        state = MQTT_STATE_DISCONNECT;
        break;
      }
      else
      {
        // publish connection message
        snprintf(topic, sizeof(topic), "%d/%s/%s/%s/%07d,%07d/%s/%s", RMAP_PROCOTOL_VERSION, param.configuration->mqtt_maint_topic, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network, MQTT_STATUS_TOPIC);
        error = mqttClientPublish(&mqttClientContext, topic, MQTT_ON_CONNECT_MESSAGE, strlen(MQTT_ON_CONNECT_MESSAGE), qos, true, NULL);
        TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, MQTT_ON_CONNECT_MESSAGE, error ? ERROR_STRING : OK_STRING);


        TRACE_INFO_F(F("%s Connected to mqtt server %s on port %d\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, param.configuration->mqtt_port);
      }

      // Subscribe to the desired topics
      snprintf(topic, sizeof(topic), "%d/%s/%s/%s/%07d,%07d/%s/%s", RMAP_PROCOTOL_VERSION, param.configuration->mqtt_rpc_topic, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network, MQTT_RPC_COM_TOPIC);
      error = mqttClientSubscribe(&mqttClientContext, topic, qos, NULL);

      TRACE_INFO_F(F("%s Subscribe to mqtt server %s on %s [ %s ]\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, topic, error ? ERROR_STRING : OK_STRING);

      param.systemStatusLock->Take();
      param.system_status->connection.is_mqtt_connected = true;
      param.system_status->connection.is_mqtt_connecting = false;
      param.system_status->connection.is_mqtt_publishing = true;
      param.system_status->mqtt_data_published = 0;
      param.systemStatusLock->Give();

      response.connection.done_mqtt_connected = true;
      param.systemResponseQueue->Enqueue(&response, 0);

      // Successful connection?
      state = MQTT_STATE_PUBLISH;
      TRACE_VERBOSE_F(F("MQTT_STATE_CONNECT -> MQTT_STATE_PUBLISH\r\n"));
      break;

    case MQTT_STATE_PUBLISH:
      // TODO: da recuperare da SD Card
      strSafeCopy(sensors_topic, "254,0,0/265,0,-,-/B01213", MQTT_SENSOR_TOPIC_LENGTH);
      snprintf(message, sizeof(message), "msg from ppp");

      // Set topic
      snprintf(topic, sizeof(topic), "%d/%s/%s/%s/%07d,%07d/%s/%s", RMAP_PROCOTOL_VERSION, param.configuration->data_level, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network, sensors_topic);

      error = mqttClientPublish(&mqttClientContext, topic, message, strlen(message), qos, false, NULL);
      TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);
      if (error)
      {
        // Connection to MQTT server lost?
        state = MQTT_STATE_DISCONNECT;
        TRACE_VERBOSE_F(F("MQTT_STATE_PUBLISH -> MQTT_STATE_DISCONNECT\r\n"));
      }
      else
      {
        param.systemStatusLock->Take();
        param.system_status->mqtt_data_published++;
        param.systemStatusLock->Give();
      }

      // pubblica ogni 5 secondi
      Delay(Ticks::MsToTicks(5000));
      break;

    case MQTT_STATE_DISCONNECT:
      param.systemStatusLock->Take();
      param.system_status->connection.is_mqtt_connected = false;
      param.system_status->connection.is_mqtt_disconnecting = true;
      param.system_status->connection.is_mqtt_publishing = false;
      param.systemStatusLock->Give();

      // publish disconnection message
      snprintf(topic, sizeof(topic), "%d/%s/%s/%s/%07d,%07d/%s/%s", RMAP_PROCOTOL_VERSION, param.configuration->mqtt_maint_topic, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network, MQTT_STATUS_TOPIC);
      error = mqttClientPublish(&mqttClientContext, topic, MQTT_ON_DISCONNECT_MESSAGE, strlen(MQTT_ON_DISCONNECT_MESSAGE), qos, true, NULL);
      if (!error)
      {
        TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, MQTT_ON_DISCONNECT_MESSAGE, error ? ERROR_STRING : OK_STRING);
      }

      mqttClientClose(&mqttClientContext);
      TRACE_INFO_F(F("%s Disconnected from mqtt server %s on port %d\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, param.configuration->mqtt_port);
      
      state = MQTT_STATE_END;
      TRACE_VERBOSE_F(F("MQTT_STATE_DISCONNECT -> MQTT_STATE_END\r\n"));
      break;

    case MQTT_STATE_END:
      // ok
      if (!is_error)
      {
        response.connection.done_mqtt_disconnected = true;
        param.systemResponseQueue->Enqueue(&response, 0);

        param.systemStatusLock->Take();
        param.system_status->connection.is_mqtt_disconnecting = false;
        param.system_status->connection.is_mqtt_disconnected = true;
        param.systemStatusLock->Give();

        mqttClientDeinit(&mqttClientContext);
        state = MQTT_STATE_INIT;
        TRACE_VERBOSE_F(F("MQTT_STATE_END -> MQTT_STATE_INIT\r\n"));
      }
      // retry
      else if ((++retry) < MQTT_TASK_GENERIC_RETRY)
      {
        Delay(Ticks::MsToTicks(MQTT_TASK_GENERIC_RETRY_DELAY_MS));

        TRACE_VERBOSE_F(F("MQTT_STATE_END -> MQTT_STATE_CONNECT\r\n"));
        state = MQTT_STATE_CONNECT;
      }
      // error
      else
      {
        response.connection.done_mqtt_disconnected = true;
        param.systemResponseQueue->Enqueue(&response, 0);

        param.systemStatusLock->Take();
        param.system_status->connection.is_mqtt_disconnecting = false;
        param.system_status->connection.is_mqtt_disconnected = true;
        param.systemStatusLock->Give();

        mqttClientDeinit(&mqttClientContext);
        state = MQTT_STATE_INIT;
        TRACE_VERBOSE_F(F("MQTT_STATE_END -> MQTT_STATE_INIT\r\n"));
      }
      break;
    }
  }
}

/**
 * @brief Subscriber callback function
 * @param[in] context Pointer to the MQTT client context
 * @param[in] topic Topic name
 * @param[in] message Message payload
 * @param[in] length Length of the message payload
 * @param[in] dup Duplicate delivery of the PUBLISH packet
 * @param[in] qos QoS level used to publish the message
 * @param[in] retain This flag specifies if the message is to be retained
 * @param[in] packetId Packet identifier
 **/
void mqttPublishCallback(MqttClientContext *context, const char_t *topic, const uint8_t *message, size_t length, bool_t dup, MqttQosLevel qos, bool_t retain, uint16_t packetId)
{
  TRACE_INFO_F(F("MQTT packet received...\r\n"));
  TRACE_INFO_F(F("Dup: %u\r\n"), dup);
  TRACE_INFO_F(F("QoS: %u\r\n"), qos);
  TRACE_INFO_F(F("Retain: %u\r\n"), retain);
  TRACE_INFO_F(F("Packet Identifier: %u\r\n"), packetId);
  TRACE_INFO_F(F("Topic: %s\r\n"), topic);
  TRACE_INFO_F(F("Message (%" PRIuSIZE " bytes):\r\n"), length);
  TRACE_INFO_ARRAY("    ", message, length);
}

/**
 * @brief TLS initialization callback
 * @param[in] context Pointer to the MQTT client context
 * @param[in] tlsContext Pointer to the TLS context
 * @return Error code
 **/
error_t mqttTlsInitCallback(MqttClientContext *context, TlsContext *tlsContext)
{
  error_t error;

  //Debug message
  TRACE_INFO_F(F("MQTT TLS initialization callback\r\n"));

  //Set the PRNG algorithm to be used
  error = tlsSetPrng(tlsContext, YARROW_PRNG_ALGO, MqttYarrowContext);
  //Any error to report?
  if(error)
    return error;

  //Preferred cipher suite list
  error = tlsSetCipherSuites(tlsContext, cipherSuites, sizeof(cipherSuites));
  // Any error to report?
  if(error)
    return error;

  //Set the fully qualified domain name of the server
  error = tlsSetServerName(tlsContext, MqttServer);
  //Any error to report?
  if(error)
    return error;

  //Set the PSK identity to be used by the client
  error = tlsSetPskIdentity(tlsContext, MqttClientPSKIdentity);
  //Any error to report?
  if(error)
    return error;

  //Set the pre-shared key to be used
  error = tlsSetPsk(tlsContext, MqttClientPSKKey, CLIENT_PSK_KEY_LENGTH);
  //Any error to report?
  if(error)
    return error;

  //Successful processing
  return NO_ERROR;
}

#endif