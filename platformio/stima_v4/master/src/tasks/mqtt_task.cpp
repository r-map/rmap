#define TRACE_LEVEL MQTT_TASK_TRACE_LEVEL

#include "tasks/mqtt_task.h"

using namespace cpp_freertos;

YarrowContext *MqttParamYarrowContext;

//Client's PSK identity
#define APP_CLIENT_PSK_IDENTITY "userv4/stimav4/stima4"

//Client's PSK
const uint8_t clientPsk[] = {0x4F, 0x3E, 0x7E, 0x10, 0xD2, 0xD1, 0x6A, 0xE2, 0xC5, 0xAC, 0x60, 0x12, 0x0F, 0x07, 0xEF, 0xAF};

//List of preferred ciphersuites
// https://ciphersuite.info/cs/?security=recommended&singlepage=true&page=2&tls=all&sort=asc
const uint16_t cipherSuites[] = {
  // rmap server psk ciphers
  TLS_PSK_WITH_AES_256_CBC_SHA,            // WEAK BUT WORK
  TLS_PSK_WITH_AES_256_GCM_SHA384,         // WEAK BUT WORK
  TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384,   // WEAK BUT WORK
  TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA,      // WEAK BUT WORK
  // TLS_DHE_PSK_WITH_AES_256_CBC_SHA384,     // WEAK AND NOT WORK
  TLS_PSK_WITH_AES_256_CBC_SHA384,         // WEAK BUT WORK
  // TLS_DHE_PSK_WITH_AES_128_GCM_SHA256,     // RECOMMENDED BUT NOT WORK
  TLS_PSK_WITH_AES_128_GCM_SHA256,         // WEAK BUT WORK
  TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256,   // WEAK BUT WORK
  TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA,      // WEAK BUT WORK
  // TLS_DHE_PSK_WITH_AES_128_CBC_SHA256,     // WEAK AND NOT WORK
  // TLS_DHE_PSK_WITH_AES_128_CBC_SHA,        // WEAK AND NOT WORK
  TLS_PSK_WITH_AES_128_CBC_SHA256,         // WEAK BUT WORK
  TLS_PSK_WITH_AES_128_CBC_SHA             // WEAK BUT WORK

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

MqttTask::MqttTask(const char *taskName, uint16_t stackSize, uint8_t priority, MqttParam_t mqttParam) : Thread(taskName, stackSize, priority), MqttParam(mqttParam) {
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
void mqttPublishCallback(MqttClientContext *context, const char_t *topic, const uint8_t *message, size_t length, bool_t dup, MqttQosLevel qos, bool_t retain, uint16_t packetId) {
  //Debug message
  TRACE_INFO("PUBLISH packet received...\r\n");
  TRACE_INFO("  Dup: %u\r\n", dup);
  TRACE_INFO("  QoS: %u\r\n", qos);
  TRACE_INFO("  Retain: %u\r\n", retain);
  TRACE_INFO("  Packet Identifier: %u\r\n", packetId);
  TRACE_INFO("  Topic: %s\r\n", topic);
  TRACE_INFO("  Message (%" PRIuSIZE " bytes):\r\n", length);
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
  TRACE_INFO("MQTT: TLS initialization callback\r\n");

  //Set the PRNG algorithm to be used
  error = tlsSetPrng(tlsContext, YARROW_PRNG_ALGO, MqttParamYarrowContext);
  //Any error to report?
  if(error)
    return error;

  //Preferred cipher suite list
  error = tlsSetCipherSuites(tlsContext, cipherSuites, arraysize(cipherSuites));
  // Any error to report?
  if(error)
    return error;

  //Set the fully qualified domain name of the server
  error = tlsSetServerName(tlsContext, "test.rmap.cc");
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
        error = initCPRNG(&MqttParam.yarrowContext);
        if (error) {
          TRACE_ERROR("Failed to initialize Cryptographic Pseudo Random Number Generator!\r\n");
        }

        //Initialize MQTT client context
        error = mqttClientInit(&MqttParam.mqttClientContext);
        if (error) {
          TRACE_ERROR("Failed to initialize MQTT client!\r\n");
        }

        state = MQTT_STATE_OPEN_CONNECTION;
        TRACE_VERBOSE("MQTT_STATE_INIT -> MQTT_STATE_OPEN_CONNECTION\r\n");
      break;

      case MQTT_STATE_OPEN_CONNECTION:
        while(!isNetReady(NULL)) {
          Delay(Ticks::MsToTicks(500));
        }

        TRACE_INFO("\r\n\r\nResolving server name...\r\n");
        // Resolve MQTT server name
        error = getHostByName(NULL, MqttParam.server, &serverIpAddr, 0);
        if (error) {
          state = MQTT_STATE_CLOSE_CONNECTION;
          TRACE_VERBOSE("MQTT_STATE_OPEN_CONNECTION -> MQTT_STATE_CLOSE_CONNECTION\r\n");
          break;
        }

        //Set the MQTT version to be used
        mqttClientSetVersion(&MqttParam.mqttClientContext, MqttParam.version);

        if (MqttParam.transportProtocol == MQTT_TRANSPORT_PROTOCOL_TCP) {
          //MQTT over TCP
          mqttClientSetTransportProtocol(&MqttParam.mqttClientContext, MQTT_TRANSPORT_PROTOCOL_TCP);
        }
        else if (MqttParam.transportProtocol == MQTT_TRANSPORT_PROTOCOL_TLS) {
          //Shared Pointer
          MqttParamYarrowContext = &MqttParam.yarrowContext;
          //MQTT over TLS
          mqttClientSetTransportProtocol(&MqttParam.mqttClientContext, MQTT_TRANSPORT_PROTOCOL_TLS);
          //Register TLS initialization callback
          mqttClientRegisterTlsInitCallback(&MqttParam.mqttClientContext, mqttTlsInitCallback);
        }

        //Register publish callback function
        mqttClientRegisterPublishCallback(&MqttParam.mqttClientContext, mqttPublishCallback);

        //Set communication timeout
        mqttClientSetTimeout(&MqttParam.mqttClientContext, MqttParam.timeoutMs);
        //Set keep-alive value
        mqttClientSetKeepAlive(&MqttParam.mqttClientContext, MqttParam.keepAliveS);

        //Set client identifier
        if (strlen(MqttParam.clientIdentifier)) {
          mqttClientSetIdentifier(&MqttParam.mqttClientContext, MqttParam.clientIdentifier);
        }

        //Set user name and password
        if (strlen(MqttParam.username) && strlen(MqttParam.password)) {
          mqttClientSetAuthInfo(&MqttParam.mqttClientContext, "username", "password");
        }

        //Set Will message
        if (strlen(MqttParam.willTopic) && strlen(MqttParam.willMsg)) {
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
        // error = mqttClientSubscribe(&mqttClientContext, "board/leds/+", MqttParam.qos, NULL);
        //Any error to report?
        // if(error)
        //  break;

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
        error = mqttClientPublish(&MqttParam.mqttClientContext, "1/report/userv4//1112345,4412345/test/254,0,0/265,0,-,-/B01213", "online", 6, MqttParam.qos, MqttParam.isPublishRetain, NULL);
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
