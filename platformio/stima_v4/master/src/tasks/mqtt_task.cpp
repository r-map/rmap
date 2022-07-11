#define TRACE_LEVEL MQTT_TASK_TRACE_LEVEL

#include "tasks/mqtt_task.h"

using namespace cpp_freertos;

YarrowContext *mqttYarrowContext;
char_t *mqttTrustedCaList;
uint8_t *clientPsk;
uint16_t *cipherSuites;

MqttTask::MqttTask(const char *taskName, uint16_t stackSize, uint8_t priority, MqttParam_t mqttParam) : Thread(taskName, stackSize, priority), MqttParam(mqttParam) {
  mqttYarrowContext = MqttParam.yarrowContext;
  mqttTrustedCaList = MqttParam.trustedCaList;
  clientPsk = MqttParam.clientPsk;
  cipherSuites = MqttParam.cipherSuites;
  state = MQTT_STATE_INIT;
  Start();
};

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

void mqttPublishCallback(MqttClientContext *context,
   const char_t *topic, const uint8_t *message, size_t length,
   bool_t dup, MqttQosLevel qos, bool_t retain, uint16_t packetId)
{
   //Debug message
   TRACE_INFO("PUBLISH packet received...\r\n");
   TRACE_INFO("  Dup: %u\r\n", dup);
   TRACE_INFO("  QoS: %u\r\n", qos);
   TRACE_INFO("  Retain: %u\r\n", retain);
   TRACE_INFO("  Packet Identifier: %u\r\n", packetId);
   TRACE_INFO("  Topic: %s\r\n", topic);
   TRACE_INFO("  Message (%" PRIuSIZE " bytes):\r\n", length);
   TRACE_INFO_ARRAY("    ", message, length);

   //Check topic name
   // if(!strcmp(topic, "board/leds/1"))
   // {
   //    if(length == 6 && !strncasecmp((char_t *) message, "toggle", 6))
   //       BSP_LED_Toggle(LED2);
   //    else if(length == 2 && !strncasecmp((char_t *) message, "on", 2))
   //       BSP_LED_On(LED2);
   //    else
   //       BSP_LED_Off(LED2);
   // }
   // else if(!strcmp(topic, "board/leds/2"))
   // {
   //    if(length == 6 && !strncasecmp((char_t *) message, "toggle", 6))
   //       BSP_LED_Toggle(LED3);
   //    else if(length == 2 && !strncasecmp((char_t *) message, "on", 2))
   //       BSP_LED_On(LED3);
   //    else
   //       BSP_LED_Off(LED3);
   // }
}

/**
 * @brief TLS initialization callback
 * @param[in] context Pointer to the MQTT client context
 * @param[in] tlsContext Pointer to the TLS context
 * @return Error code
 **/

 error_t mqttTestTlsInitCallback(MqttClientContext *context, TlsContext *tlsContext) {
   error_t error;

   //Debug message
   TRACE_INFO("MQTT: TLS initialization callback\r\n");

   //Set the PRNG algorithm to be used
   error = tlsSetPrng(tlsContext, YARROW_PRNG_ALGO, mqttYarrowContext);
   //Any error to report?
   if(error)
   return error;

   //Preferred cipher suite list
   error = tlsSetCipherSuites(tlsContext, cipherSuites, arraysize(cipherSuites));
   //Any error to report?
   if(error)
   return error;

   //Set the fully qualified domain name of the server
   error = tlsSetServerName(tlsContext, APP_SERVER_NAME);
   //Any error to report?
   if(error)
   return error;

   #if (APP_SERVER_PORT == 8885)
   //Import client's certificate
  //  error = tlsAddCertificate(tlsContext, clientCert, strlen(clientCert), clientKey, strlen(clientKey));
   //Any error to report?
   if(error)
   return error;

   //Set the PSK identity to be used by the client
   error = tlsSetPskIdentity(tlsContext, APP_CLIENT_PSK_IDENTITY);
   //Any error to report?
   if(error)
   return error;

   //Set the pre-shared key to be used
   error = tlsSetPsk(tlsContext, clientPsk, sizeof(clientPsk));
   //Any error to report?
   if(error)
   return error;
   #endif

   //Import trusted CA certificates
   error = tlsSetTrustedCaList(tlsContext, mqttTrustedCaList, strlen(mqttTrustedCaList));
   //Any error to report?
   if(error)
   return error;

   //Successful processing
   return NO_ERROR;
 }

void MqttTask::Run() {
  //Initialize variables
  error = NO_ERROR;
  isConnected = false;

  while (true) {
    switch (state) {
      case MQTT_STATE_INIT:
        //Initialize MQTT client context
        error = mqttClientInit(&MqttParam.mqttClientContext);
        if (error) {
          TRACE_ERROR("Failed to initialize MQTT client!\r\n");
        }

        state = MQTT_STATE_OPEN_CONNECTION;
        TRACE_VERBOSE("MQTT_STATE_INIT -> MQTT_STATE_OPEN_CONNECTION\r\n");
      break;

      case MQTT_STATE_OPEN_CONNECTION:
        while(!isNetReady(MqttParam.interface)) {
          Delay(Ticks::MsToTicks(500));
        }

        // Resolve MQTT server name
        error = getHostByName(MqttParam.interface, MqttParam.server, &serverIpAddr, 0);
        if (error) {
          state = MQTT_STATE_CLOSE_CONNECTION;
          TRACE_VERBOSE("MQTT_STATE_OPEN_CONNECTION -> MQTT_STATE_CLOSE_CONNECTION\r\n");
          break;
        }

        #if (APP_SERVER_PORT == 8080 || APP_SERVER_PORT == 8081)
        //Register RNG callback
        webSocketRegisterRandCallback(webSocketRngCallback);
        #endif

        //Set the MQTT version to be used
        mqttClientSetVersion(&MqttParam.mqttClientContext, MQTT_VERSION_3_1_1);

        if (MqttParam.transportProtocol == MQTT_TRANSPORT_PROTOCOL_TCP) {
          //MQTT over TCP
          mqttClientSetTransportProtocol(&MqttParam.mqttClientContext, MQTT_TRANSPORT_PROTOCOL_TCP);
        }
        else if (MqttParam.transportProtocol == MQTT_TRANSPORT_PROTOCOL_TLS) {
          #if (MQTT_CLIENT_TLS_SUPPORT)
          //MQTT over TLS
          mqttClientSetTransportProtocol(&MqttParam.mqttClientContext, MQTT_TRANSPORT_PROTOCOL_TLS);
          //Register TLS initialization callback
          mqttClientRegisterTlsInitCallback(&MqttParam.mqttClientContext, mqttTestTlsInitCallback);
          #endif
        }
        else if (MqttParam.transportProtocol == MQTT_TRANSPORT_PROTOCOL_WS) {
          #if (MQTT_CLIENT_WS_SUPPORT)
          //MQTT over WebSocket
          mqttClientSetTransportProtocol(&MqttParam.mqttClientContext, MQTT_TRANSPORT_PROTOCOL_WS);
          #endif
        }
        else if (MqttParam.transportProtocol == MQTT_TRANSPORT_PROTOCOL_WSS) {
          #if (MQTT_CLIENT_TLS_SUPPORT && MQTT_CLIENT_WS_SUPPORT)
          //MQTT over secure WebSocket
          mqttClientSetTransportProtocol(&MqttParam.mqttClientContext, MQTT_TRANSPORT_PROTOCOL_WSS);
          //Register TLS initialization callback
          mqttClientRegisterTlsInitCallback(&MqttParam.mqttClientContext, mqttTestTlsInitCallback);
          #endif
        }

        //Register publish callback function
        mqttClientRegisterPublishCallback(&MqttParam.mqttClientContext, mqttPublishCallback);

        //Set communication timeout
        mqttClientSetTimeout(&MqttParam.mqttClientContext, MqttParam.timeoutMs);
        //Set keep-alive value
        mqttClientSetKeepAlive(&MqttParam.mqttClientContext, MqttParam.keepAliveS);

        // #if (APP_SERVER_PORT == 8080 || APP_SERVER_PORT == 8081)
        // //Set the hostname of the resource being requested
        // mqttClientSetHost(&MqttParam.mqttClientContext, APP_SERVER_NAME);
        // //Set the name of the resource being requested
        // mqttClientSetUri(&MqttParam.mqttClientContext, APP_SERVER_URI);
        // #endif

        //Set client identifier
        mqttClientSetIdentifier(&MqttParam.mqttClientContext, "client12345678");

        //Set user name and password
        if (strlen(MqttParam.username) && strlen(MqttParam.password)) {
          mqttClientSetAuthInfo(&MqttParam.mqttClientContext, "username", "password");
        }

        //Set Will message
        if (strlen(MqttParam.willMsg) && strlen(MqttParam.willTopic)) {
          mqttClientSetWillMessage(&MqttParam.mqttClientContext, MqttParam.willTopic, MqttParam.willMsg, strlen(MqttParam.willMsg), MqttParam.qos, MqttParam.isWillMsgRetain);
        }

        //Debug message
        TRACE_INFO("Connecting to MQTT server %s...\r\n", ipAddrToString(&serverIpAddr, NULL));

        //Establish connection with the MQTT server
        error = mqttClientConnect(&MqttParam.mqttClientContext, &serverIpAddr, MqttParam.port, MqttParam.isCleanSession);
        //Any error to report?
        if (error) {
          state = MQTT_STATE_CLOSE_CONNECTION;
          break;
        }

        //Subscribe to the desired topics
        // error = mqttClientSubscribe(&MqttParam.mqttClientContext, "board/leds/+", MQTT_QOS_LEVEL_1, NULL);
        //Any error to report?
        // if(error)
        // break;

        //Successful connection?
        //The MQTT client is connected to the server
        isConnected = TRUE;
        state = MQTT_STATE_PUBLISH;
        TRACE_VERBOSE("MQTT_STATE_OPEN_CONNECTION -> MQTT_STATE_PUBLISH\r\n");

        //Delay between subsequent connection attempts
        Delay(Ticks::MsToTicks(MqttParam.attemptDelayMs));
        TRACE_VERBOSE("MQTT_STATE_OPEN_CONNECTION -> Retry\r\n");
      break;

      case MQTT_STATE_PUBLISH:
        //Send PUBLISH packet
        error = mqttClientPublish(&MqttParam.mqttClientContext, "report/myuser/prova", "test", 6, MqttParam.qos, MqttParam.isPublishRetain, NULL);
        if (error) {
          //Connection to MQTT server lost?
          state = MQTT_STATE_CLOSE_CONNECTION;
          TRACE_VERBOSE("MQTT_STATE_PUBLISH -> MQTT_STATE_CLOSE_CONNECTION\r\n");
        }
        // pubblica ogni 5 secondi
        Delay(Ticks::MsToTicks(5000));
      break;

      case MQTT_STATE_CLOSE_CONNECTION:
        //Close connection
        mqttClientClose(&MqttParam.mqttClientContext);
        //Update connection state
        isConnected = FALSE;
        state = MQTT_STATE_END;
        //Recovery delay
        // Delay(Ticks::MsToTicks(MqttParam.attemptDelayMs));
        // state = MQTT_STATE_OPEN_CONNECTION;
      break;

      case MQTT_STATE_END:
        state = MQTT_STATE_INIT;
        Thread::Suspend();
      break;
    }
  }
}
