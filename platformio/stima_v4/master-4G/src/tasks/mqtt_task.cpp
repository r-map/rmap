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
  state = MQTT_STATE_INIT;
  version = MQTT_VERSION_3_1_1;
  transportProtocol = MQTT_TRANSPORT_PROTOCOL_TLS;
  qos = MQTT_QOS_LEVEL_1;
  Start();
};

void MqttTask::Run()
{
  bool is_error;
  error_t error;
  system_request_t request;
  system_response_t response;
  IpAddr ipAddr;
  char topic[MQTT_ROOT_TOPIC_LENGTH + MQTT_SENSOR_TOPIC_LENGTH];
  char sensors_topic[MQTT_SENSOR_TOPIC_LENGTH];
  char message[MQTT_MESSAGE_LENGTH];
  
  // TODO: da rimuovere
  strSafeCopy(sensors_topic, "254,0,0/265,0,-,-/B01213", MQTT_SENSOR_TOPIC_LENGTH);

  while (true)
  {
    memset(&request, 0, sizeof(system_request_t));
    memset(&response, 0, sizeof(system_response_t));

    switch (state)
    {
    case MQTT_STATE_INIT:
      error = mqttClientInit(&mqttClientContext);
      if (error)
      {
        TRACE_ERROR_F(F("%s, Failed to initialize MQTT client [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
      }

      state = MQTT_STATE_WAIT_NET_EVENT;
      TRACE_VERBOSE_F(F("MQTT_STATE_INIT -> MQTT_STATE_WAIT_NET_EVENT\r\n"));
      break;

    case MQTT_STATE_WAIT_NET_EVENT:
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
      TRACE_INFO_F(F("%s Resolving mqtt server name of %s\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server);
      // Resolve MQTT server name
      error = getHostByName(NULL, param.configuration->mqtt_server, &ipAddr, 0);
      if (error)
      {
        is_error = true;

        TRACE_ERROR_F(F("%s Failed to resolve mqtt server name of %s\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server);

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

        // username/stationslug/boardslug
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
      if (strlen(clientIdentifier))
      {
        mqttClientSetIdentifier(&mqttClientContext, clientIdentifier);
      }

      // Set user name and password
      if (strlen(param.configuration->mqtt_username) && strlen(param.configuration->mqtt_password))
      {
        mqttClientSetAuthInfo(&mqttClientContext, param.configuration->mqtt_username, param.configuration->mqtt_password);
      }

      // Set Will message
      if (strlen(MQTT_ON_ERROR_MESSAGE))
      {
        snprintf(topic, sizeof(topic), "%d/%s/%s/%s/%07d,%07d/%s", RMAP_PROCOTOL_VERSION, DATA_LEVEL_MAINT, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network);
        mqttClientSetWillMessage(&mqttClientContext, topic, MQTT_ON_ERROR_MESSAGE, strlen(MQTT_ON_ERROR_MESSAGE), qos, isWillMsgRetain);
      }

      // Establish connection with the MQTT server
      error = mqttClientConnect(&mqttClientContext, &ipAddr, param.configuration->mqtt_port, isCleanSession);
      // Any error to report?
      if (error)
      {
        is_error = true;

        TRACE_ERROR_F(F("%s Failed to connect to mqtt server %s\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server);

        TRACE_VERBOSE_F(F("MQTT_STATE_CONNECT -> MQTT_STATE_DISCONNECT\r\n"));
        state = MQTT_STATE_DISCONNECT;
        break;
      }
      else
      {
        TRACE_INFO_F(F("%s Connected to mqtt server %s on port %d\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, param.configuration->mqtt_port);
      }

      // Subscribe to the desired topics
      //  error = mqttClientSubscribe(&mqttClientContext, "board/leds/+", qos, NULL);
      // Any error to report?
      //  if(error)
      //   break;

      // Successful connection?
      state = MQTT_STATE_PUBLISH;
      TRACE_VERBOSE_F(F("MQTT_STATE_CONNECT -> MQTT_STATE_PUBLISH\r\n"));
      break;

    case MQTT_STATE_PUBLISH:
      snprintf(topic, sizeof(topic), "%d/%s/%s/%s/%07d,%07d/%s/%s", RMAP_PROCOTOL_VERSION, param.configuration->data_level, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network, sensors_topic);

      // snprintf(topic, sizeof(topic), "1/report/userv4//1112345,4412345/test/254,0,0/265,0,-,-/B01213");
      snprintf(message, sizeof(message), "msg from ppp");

      error = mqttClientPublish(&mqttClientContext, topic, message, strlen(message), qos, isPublishRetain, NULL);
      TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);
      if (error)
      {
        // Connection to MQTT server lost?
        state = MQTT_STATE_DISCONNECT;
        TRACE_VERBOSE_F(F("MQTT_STATE_PUBLISH -> MQTT_STATE_DISCONNECT\r\n"));
      }
       // pubblica ogni 5 secondi
       Delay(Ticks::MsToTicks(5000));
      break;

    case MQTT_STATE_DISCONNECT:
      mqttClientClose(&mqttClientContext);
      TRACE_INFO_F(F("%s Disconnected from mqtt server %s on port %d\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, param.configuration->mqtt_port);
      
      state = MQTT_STATE_END;
      TRACE_VERBOSE_F(F("MQTT_STATE_DISCONNECT -> MQTT_STATE_END\r\n"));
      break;

    case MQTT_STATE_END:
      mqttClientDeinit(&mqttClientContext);
      state = MQTT_STATE_INIT;
      TRACE_VERBOSE_F(F("MQTT_STATE_END -> MQTT_STATE_INIT\r\n"));
      break;
    }
  }
}

/**
 * @brief Publish callback function
 * @param[in] context Pointer to the MQTT client context
 * @param[in] topic Topic name
 * @param[in] message Message payload
 * @param[in] length Length of the message payload
 * @param[in] dup Duplicate delivery of the PUBLISH packet
 * @param[in] qos QoS level used to publish the message
 * @param[in] retain This flag specifies if the message is to be retained
 * @param[in] packetId Packet identifier
 **/
void mqttPublishCallback(MqttClientContext *context, const char_t *topic, const uint8_t *message, size_t length, bool_t dup, MqttQosLevel qos, bool_t retain, uint16_t packetId) {
  //Debug message
  TRACE_INFO_F(F("PUBLISH packet received...\r\n"));
  TRACE_INFO_F(F("  Dup: %u\r\n"), dup);
  TRACE_INFO_F(F("  QoS: %u\r\n"), qos);
  TRACE_INFO_F(F("  Retain: %u\r\n"), retain);
  TRACE_INFO_F(F("  Packet Identifier: %u\r\n"), packetId);
  TRACE_INFO_F(F("  Topic: %s\r\n"), topic);
  TRACE_INFO_F(F("  Message (%" PRIuSIZE " bytes):\r\n"), length);
  TRACE_INFO_ARRAY("    ", message, length);
}

/**
 * @brief TLS initialization callback
 * @param[in] context Pointer to the MQTT client context
 * @param[in] tlsContext Pointer to the TLS context
 * @return Error code
 **/
error_t mqttTlsInitCallback(MqttClientContext *context, TlsContext *tlsContext) {
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