/**@file mqtt_task.cpp */

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

#define TRACE_LEVEL     MQTT_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   MQTT_TASK_ID

#include "tasks/mqtt_task.h"

#if (USE_MQTT)

using namespace cpp_freertos;

/* 
  TEST VALUE FIXED PSK KEY
  
  //Client's PSK identity
  #define APP_CLIENT_PSK_IDENTITY "userv4/stimav4/stima4"

  //Client's PSK
  const uint8_t MqttClientPSKKey[] = {0x4F, 0x3E, 0x7E, 0x10, 0xD2, 0xD1, 0x6A, 0xE2, 0xC5, 0xAC, 0x60, 0x12, 0x0F, 0x07, 0xEF, 0xAF};

  List of preferred ciphersuites
  https://ciphersuite.info/cs/?security=recommended&singlepage=true&page=2&tls=all&sort=asc
  defined in tasks/mqtt_task.h -> const uint16_t cipherSuites[] = TYPE_VALUE
*/

MqttTask::MqttTask(const char *taskName, uint16_t stackSize, uint8_t priority, MqttParam_t mqttParam) : Thread(taskName, stackSize, priority), param(mqttParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(MQTT_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  // local to static member private access
  localRpcLock = param.rpcLock;
  localStreamRpc = param.streamRpc;

  state = MQTT_STATE_INIT;
  version = MQTT_VERSION_3_1_1;
  transportProtocol = MQTT_TRANSPORT_PROTOCOL_TLS;
  qos = MQTT_QOS_LEVEL_1;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void MqttTask::TaskMonitorStack()
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
void MqttTask::TaskWatchDog(uint32_t millis_standby)
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
void MqttTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
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

void MqttTask::Run()
{
  uint8_t retry;
  bool is_error;
  bool is_subscribed;
  bool mqtt_connection_estabilished;
  error_t error;
  connection_request_t connection_request;
  connection_response_t connection_response;
  IpAddr ipAddr;
  bool is_data_publish_end = false;
  // RMAP Data queue and status local VAR
  bool start_get_data = true;
  rmap_get_request_t rmap_get_request;
  rmap_get_response_t rmap_get_response;
  bool rmap_eof;
  bool rmap_data_error;
  uint32_t countData;
  // RMAP Casting value to Uavcan Structure
  rmap_module_TH_1_0 *rmapDataTH;
  rmap_module_Rain_1_0 *rmapDataRain;
  rmap_module_Radiation_1_0 *rmapDataRadiation;
  rmap_module_Wind_1_0 *rmapDataWind;
  rmap_module_VWC_1_0 *rmapDataVWC;
  rmap_module_Power_1_0 *rmapDataPower;

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  while (true)
  {
    switch (state)
    {
    case MQTT_STATE_INIT:
      state = MQTT_STATE_WAIT_NET_EVENT;
      param.systemStatusLock->Take();
      param.system_status->connection.is_mqtt_connected = false;
      param.system_status->connection.is_mqtt_connecting = false;
      param.system_status->connection.is_mqtt_disconnected = true;
      param.system_status->connection.is_mqtt_disconnecting = false;
      param.system_status->connection.is_mqtt_publishing = false;
      param.system_status->connection.is_mqtt_publishing_end = false;
      param.systemStatusLock->Give();
      TRACE_VERBOSE_F(F("MQTT_STATE_INIT -> MQTT_STATE_WAIT_NET_EVENT\r\n"));
      break;

    case MQTT_STATE_WAIT_NET_EVENT:
      retry = 0;
      is_error = false;
      is_data_publish_end = false;

      // wait connection request
      // Suspend TASK Controller for queue waiting portMAX_DELAY
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
      if (param.connectionRequestQueue->Peek(&connection_request, portMAX_DELAY))
      {
        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);      
        // do mqtt connection
        if (connection_request.do_mqtt_connect)
        {

          // ***** TEST REQUEST QUEUE DO MQTT CONNECTION INCOMING START
          Serial.println("// ***** TEST REQUEST QUEUE DO MQTT CONNECTION INCOMING START");

          MqttServer = param.configuration->mqtt_server;
          // strSafeCopy(MqttServer, param.configuration->mqtt_server, MQTT_SERVER_LENGTH);
          
          // Start new connection sequence
          mqtt_connection_estabilished = false;

          param.connectionRequestQueue->Dequeue(&connection_request, 0);
          TRACE_VERBOSE_F(F("MQTT_STATE_WAIT_NET_EVENT -> MQTT_STATE_CONNECT\r\n"));
          state = MQTT_STATE_CONNECT;
        }
      }
      break;

    case MQTT_STATE_CONNECT:
      error = mqttClientInit(&mqttClientContext);
      if (error)
      {
        // ***** TEST INIT MQTT client CYCLONE FAILED
        Serial.println("// ***** TEST INIT MQTT client CYCLONE FAILED");
      } else {
        // ***** TEST INIT MQTT client CYCLONE OK
        Serial.println("// ***** TEST INIT MQTT client CYCLONE OK");
      }
      
      param.systemStatusLock->Take();
      param.system_status->connection.is_mqtt_connecting = true;
      param.systemStatusLock->Give();

      TRACE_INFO_F(F("%s Resolving mqtt server name of %s\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server);
      // Resolve MQTT server name
      TaskState(state, 1, task_flag::suspended); // Or SET Long WDT > 120 sec.
      error = getHostByName(NULL, param.configuration->mqtt_server, &ipAddr, 0);
      TaskState(state, 1, task_flag::normal); // Resume
      if (error)
      {
        is_error = true;

        // ***** TEST REQUEST RESOLVE NAME SERVER MQTT FAILED
        Serial.println("// ***** TEST REQUEST RESOLVE NAME SERVER MQTT FAILED");

        TRACE_ERROR_F(F("%s Failed to resolve mqtt server name of %s [ %s ]\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, ERROR_STRING);

        state = MQTT_STATE_DISCONNECT;
        TRACE_VERBOSE_F(F("MQTT_STATE_CONNECT -> MQTT_STATE_DISCONNECT\r\n"));
        break;
      } else {
        // ***** TEST REQUEST RESOLVE NAME SERVER MQTT OK
        Serial.println("// ***** TEST REQUEST RESOLVE NAME SERVER MQTT OK");
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
        snprintf(topic, sizeof(topic), "%s/%s/%s/%07d,%07d/%s", param.configuration->mqtt_maint_topic, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network);
        mqttClientSetWillMessage(&mqttClientContext, topic, MQTT_ON_ERROR_MESSAGE, strlen(MQTT_ON_ERROR_MESSAGE), qos, true);
      }

      // Establish connection with the MQTT server
      error = mqttClientConnect(&mqttClientContext, &ipAddr, param.configuration->mqtt_port, false);
      // Any error to report?
      if (error)
      {
        is_error = true;

        // ***** TEST CONNECT MQTT SERVER FAILED
        Serial.println("// ***** TEST CONNECT MQTT SERVER FAILED");

        TRACE_ERROR_F(F("%s Failed to connect to mqtt server %s [ %s ]\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, ERROR_STRING);

        TRACE_VERBOSE_F(F("MQTT_STATE_CONNECT -> MQTT_STATE_DISCONNECT\r\n"));
        state = MQTT_STATE_DISCONNECT;
        break;
      }
      else
      {

        // ***** TEST CONNECT MQTT SERVER OK
        Serial.println("// ***** TEST CONNECT MQTT SERVER OK");

        // publish connection message
        snprintf(topic, sizeof(topic), "%s/%s/%s/%07d,%07d/%s/%s", param.configuration->mqtt_maint_topic, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network, MQTT_STATUS_TOPIC);
        error = mqttClientPublish(&mqttClientContext, topic, MQTT_ON_CONNECT_MESSAGE, strlen(MQTT_ON_CONNECT_MESSAGE), qos, true, NULL);
        TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, MQTT_ON_CONNECT_MESSAGE, error ? ERROR_STRING : OK_STRING);

        TRACE_INFO_F(F("%s Connected to mqtt server %s on port %d\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, param.configuration->mqtt_port);
      }

      // Subscribe to the desired topics (Subscribe error not blocking connection)
      snprintf(topic, sizeof(topic), "%s/%s/%s/%07d,%07d/%s/%s", param.configuration->mqtt_rpc_topic, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network, MQTT_RPC_COM_TOPIC);
      is_subscribed = !mqttClientSubscribe(&mqttClientContext, topic, qos, NULL);
      TRACE_INFO_F(F("%s Subscribe to mqtt server %s on %s [ %s ]\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, topic, error ? ERROR_STRING : OK_STRING);

      if(is_subscribed) {
        // ***** TEST CONNECT MQTT SERVER SUBCRIBED OK
        Serial.println("// ***** TEST CONNECT MQTT SERVER SUBCRIBED OK");
      } else {
        // ***** TEST CONNECT MQTT SERVER SUBCRIBED FAILED
        Serial.println("// ***** TEST CONNECT MQTT SERVER SUBCRIBED FAILED");
      }

      param.systemStatusLock->Take();
      param.system_status->connection.is_mqtt_subscribed = is_subscribed;      
      param.system_status->connection.is_mqtt_connected = true;
      param.system_status->connection.is_mqtt_disconnected = false;
      param.system_status->connection.is_mqtt_connecting = false;
      // Checked for end of transmission
      param.system_status->connection.is_mqtt_publishing_end = false;
      param.system_status->connection.mqtt_data_published = 0;
      param.systemStatusLock->Give();

      // Session connection complete (send response to request command)
      // No other response from TASK with this session connection
      mqtt_connection_estabilished = true;
      // Response direct when connection complete
      memset(&connection_response, 0, sizeof(connection_response_t));
      connection_response.done_mqtt_connected = true;
      param.connectionResponseQueue->Enqueue(&connection_response, 0);

      // Successful connection?
      state = MQTT_STATE_PUBLISH;
      TRACE_VERBOSE_F(F("MQTT_STATE_CONNECT -> MQTT_STATE_PUBLISH\r\n"));
      break;

    case MQTT_STATE_PUBLISH:

      // **************************************
      //   GET RMAP Data, And Append to MQTT
      // **************************************
      start_get_data = true;

      if (param.system_status->connection.mqtt_data_published == 0)
      {
        strSafeCopy(sensors_topic, "254,0,0/103,2000,-,-/B12101", MQTT_SENSOR_TOPIC_LENGTH);
        snprintf(message, sizeof(message), "{\"v\" : 100, \"t\" : \"2023-03-16T07:45:00\", \"a\" : { \"B33199\" : 100 } }");

        // Set topic
        snprintf(topic, sizeof(topic), "%s/%s/%s/%07d,%07d/%s/%s", param.configuration->mqtt_root_topic, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network, sensors_topic);

        error = mqttClientPublish(&mqttClientContext, topic, message, strlen(message), qos, false, NULL);
        TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);
        if (error)
        {
          // ***** TEST MQTT DATA PUBLISH FAILED
          Serial.println("// ***** TEST MQTT DATA PUBLISH FAILED");
          // Connection to MQTT server lost?
          state = MQTT_STATE_DISCONNECT;
          TRACE_VERBOSE_F(F("MQTT_STATE_PUBLISH -> MQTT_STATE_DISCONNECT\r\n"));
        }
        else
        {
          // ***** TEST MQTT DATA PUBLISH OK
          Serial.println("// ***** TEST MQTT DATA PUBLISH OK");
          // Starting publishing
          param.systemStatusLock->Take();
          param.system_status->connection.is_mqtt_publishing = true;
          param.system_status->connection.mqtt_data_published++;
          param.systemStatusLock->Give();
        }
      }

      // TODO. Set Time corretto
      // pubblica ogni 5 secondi
      TaskWatchDog(5000);
      Delay(Ticks::MsToTicks(5000));
      //TODO:REMOVE End connecction on END Data... >=3 Remove...
      //TODO: is_data_publish_end = true when data END !!!
      if(param.system_status->connection.mqtt_data_published >=1) {
        param.systemStatusLock->Take();
        is_data_publish_end = true;
        param.systemStatusLock->Give();
        state = MQTT_STATE_DISCONNECT;
      }
      break;

    case MQTT_STATE_DISCONNECT:

      if(is_error) {
        // ***** TEST MQTT DATA CONNECTION SEQUENCE FAILED
        Serial.println("// ***** TEST MQTT DATA CONNECTION SEQUENCE FAILED");
      } else {
        // ***** TEST MQTT DATA CONNECTION SEQUENCE OK
        Serial.println("// ***** TEST MQTT DATA CONNECTION SEQUENCE OK");
      }

      param.systemStatusLock->Take();
      param.system_status->connection.is_mqtt_disconnecting = true;
      param.systemStatusLock->Give();

      // publish disconnection message
      snprintf(topic, sizeof(topic), "%s/%s/%s/%07d,%07d/%s/%s", param.configuration->mqtt_maint_topic, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network, MQTT_STATUS_TOPIC);
      error = mqttClientPublish(&mqttClientContext, topic, MQTT_ON_DISCONNECT_MESSAGE, strlen(MQTT_ON_DISCONNECT_MESSAGE), qos, true, NULL);
      if (!error)
      {
        TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, MQTT_ON_DISCONNECT_MESSAGE, error ? ERROR_STRING : OK_STRING);
        // ***** TEST MQTT DATA PUBLISH DISCONNECTION MESSAGE FAIL
        Serial.println("// ***** TEST MQTT DATA PUBLISH DISCONNECTION MESSAGE OK");
      } else {
        // ***** TEST MQTT DATA PUBLISH DISCONNECTION MESSAGE OK
        Serial.println("// ***** TEST MQTT DATA PUBLISH DISCONNECTION MESSAGE FAIL");
      }

      mqttClientClose(&mqttClientContext);
      TRACE_INFO_F(F("%s Disconnected from mqtt server %s on port %d\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, param.configuration->mqtt_port);

      // ***** TEST MQTT END
      Serial.println("// ***** TEST MQTT END");

      state = MQTT_STATE_END;
      TRACE_VERBOSE_F(F("MQTT_STATE_DISCONNECT -> MQTT_STATE_END\r\n"));
      break;

    case MQTT_STATE_END:

      mqttClientDeinit(&mqttClientContext);

      // Response direct (error) when connection not complete
      // Only when sequence connection is not completed (otherwise response already sended)
      if(!mqtt_connection_estabilished) {
        // Check retry before error end connection MQTT
        if ((++retry) < MQTT_TASK_GENERIC_RETRY)
        {
          TaskWatchDog(MQTT_TASK_GENERIC_RETRY_DELAY_MS);
          Delay(Ticks::MsToTicks(MQTT_TASK_GENERIC_RETRY_DELAY_MS));
          TRACE_VERBOSE_F(F("MQTT_STATE_END -> MQTT_STATE_CONNECT\r\n"));
          state = MQTT_STATE_CONNECT;
          break;
        } else {
          // Send response to request
          memset(&connection_response, 0, sizeof(connection_response_t));
          connection_response.error_mqtt_connected = true;
          param.connectionResponseQueue->Enqueue(&connection_response, 0);
        }
      }

      // Exit end complete
      param.systemStatusLock->Take();
      param.system_status->connection.is_mqtt_connected = false;
      param.system_status->connection.is_mqtt_connecting = false;
      param.system_status->connection.is_mqtt_disconnected = true;
      param.system_status->connection.is_mqtt_disconnecting = false;
      param.system_status->connection.is_mqtt_subscribed = false;
      param.system_status->connection.is_mqtt_publishing = false;
      // (true if ok data send, do no clean. Only on Next Connect)
      param.system_status->connection.is_mqtt_publishing_end = is_data_publish_end;
      param.systemStatusLock->Give();

      state = MQTT_STATE_WAIT_NET_EVENT;
      TRACE_VERBOSE_F(F("MQTT_STATE_END -> MQTT_STATE_WAIT_NET_EVENT\r\n"));

      break;
    }

    #if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
    #endif

    // One step base non blocking switch
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_WAIT_DELAY_MS));

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
void MqttTask::mqttPublishCallback(MqttClientContext *context, const char_t *topic, const uint8_t *message, size_t length, bool_t dup, MqttQosLevel qos, bool_t retain, uint16_t packetId)
{
  TRACE_INFO_F(F("MQTT packet received...\r\n"));
  TRACE_INFO_F(F("Dup: %u\r\n"), dup);
  TRACE_INFO_F(F("QoS: %u\r\n"), qos);
  TRACE_INFO_F(F("Retain: %u\r\n"), retain);
  TRACE_INFO_F(F("Packet Identifier: %u\r\n"), packetId);
  TRACE_INFO_F(F("Message (%" PRIuSIZE " bytes):\r\n"), length);
  TRACE_INFO_F(F("%s %s\r\n"), topic, message);

  bool is_event_rpc = true;
  localStreamRpc->init();

  if (localRpcLock->Take(Ticks::MsToTicks(RPC_WAIT_DELAY_MS)))
  {
    while (is_event_rpc)
    {
      localStreamRpc->parseCharpointer(&is_event_rpc, (char *)message, length, NULL, 0, RPC_TYPE_SERIAL);
    }
    localRpcLock->Give();
  }
}

/**
 * @brief TLS initialization callback
 * @param[in] context Pointer to the MQTT client context
 * @param[in] tlsContext Pointer to the TLS context
 * @return Error code
 **/
error_t MqttTask::mqttTlsInitCallback(MqttClientContext *context, TlsContext *tlsContext)
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
  error = tlsSetCipherSuites(tlsContext, MqttCipherSuites, sizeof(MqttCipherSuites));
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