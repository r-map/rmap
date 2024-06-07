#include "common.h"

// disconnect from MQTT broker
bool publishThread::mqttDisconnect() {

  mqttclient.disconnect();
  //mqttclient.setMessageHandler(comtopic, NULL); // remove handler setted
  ipstack.disconnect();
  data->logger->notice(F("publish MQTT Disconnectted"));
  return true;
}


// connect to MQTT broker setting will message
bool publishThread::mqttConnect(const bool cleanSession) {

  if (WiFi.status() != WL_CONNECTED) {    
    data->logger->error(F("publish WIFI not connected!  Try reconnect"));
    WiFi.disconnect();
    WiFi.setAutoReconnect(true);
    WiFi.reconnect();
    unsigned long start=millis();
    while ((WiFi.status() != WL_CONNECTED) && (millis() < (start+30000))){
      data->logger->notice(F("publish WIFI not connetted"));
      delay(1000);
    }
  }
	   
  if (WiFi.status() != WL_CONNECTED) {
    data->logger->error(F("publish WIFI reconnect failed"));
    return false;
  }
  
  bool returnstatus = ipstack.connect(data->station->mqtt_server, MQTT_SERVER_PORT);
  if (returnstatus){
    data->logger->notice(F("publish IPstack connected"));
  } else {
    data->logger->error(F("publish IPstack connect failed"));
    return returnstatus;
  }
  
  char unique_id[MQTT_CLIENT_ID_LENGTH];
  strcpy(unique_id, data->station->user);
  strcat(unique_id, "/");
  strcat(unique_id, data->station->stationslug);
  strcat(unique_id, "/");
  strcat(unique_id, data->station->boardslug);

  mqttMessage_t mqtt_message;
  strcpy(mqtt_message.payload,"ERROR 01");
  strcpy(mqtt_message.topic,"1/");
  strcat(mqtt_message.topic,data->station->mqttmaintpath);
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,data->station->user);
  strcat(mqtt_message.topic,"//");
  strcat(mqtt_message.topic,data->station->longitude);
  strcat(mqtt_message.topic,",");
  strcat(mqtt_message.topic,data->station->latitude);
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,data->station->network);
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,"254,0,0");
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,"265,0,-,-");
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,"B01000");

  MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
  options.MQTTVersion = 4;   // Version of MQTT to be used.  3 = 3.1 4 = 3.1.1
  if (strcmp(data->station->ident,"") == 0){
    options.will.topicName.cstring = mqtt_message.topic;
    options.will.message.cstring=mqtt_message.payload;
    options.will.retained = true;
    options.will.qos = MQTT::QOS1;
    options.willFlag = true;
  }
  options.clientID.cstring = unique_id;
  options.username.cstring = unique_id;
  options.password.cstring = data->station->password;
  options.cleansession = cleanSession;
  options.keepAliveInterval = 60;
    
  data->logger->notice(F("publish MQTT clientID: %s"), options.clientID.cstring);
  data->logger->notice(F("publish MQTT will topic: %s"), options.will.topicName.cstring);
  data->logger->notice(F("publish MQTT will payload: %s"), options.will.message.cstring);
  data->logger->notice(F("publish MQTT cleansession: %s"), options.cleansession ? "true" : "false");
  data->logger->notice(F("publish MQTT server: %s"), data->station->mqtt_server);
  data->logger->notice(F("publish MQTT keepAliveInterval: %d"), options.keepAliveInterval);
  data->logger->notice(F("publish MQTT user: %s"), options.username.cstring);
  data->logger->notice(F("publish MQTT password: %s"), options.password.cstring);
    
  MQTT::connackData connack;
  uint8_t rc = mqttclient.connect(options,connack);
  if (rc == 0){
    data->logger->notice(F("publish mqttclient connected"));
  } else {
    data->logger->notice(F("publish mqttclient connect failed"));
  }
    
  returnstatus=false;
  switch (connack.rc)
    {
    case 0:
      data->logger->notice(F("publish Connection accepted"));
      returnstatus=true;
      break;
    case 1:
      data->logger->error(F("publish Connection Refused"));
      data->logger->notice(F("publish The Server does not support the level of the MQTT protocol requested by the Client"));
      break;
      
    case 2:
      data->logger->error(F("publish Connection Refused"));
      data->logger->notice(F("publish The Client identifier is correct UTF-8 but not allowed by the Server"));
      break;
       
    case 3:
      data->logger->error(F("publish Connection Refused"));
      data->logger->notice(F("publish The Network Connection has been made but the MQTT service is unavailable"));
      break;
      
    case 4:
      data->logger->error(F("publish Connection Refused"));
      data->logger->notice(F("publish bad user name or password"));
      break;
      
    case 5:
      data->logger->error(F("publish Connection Refused"));
      data->logger->notice(F("publish not authorized"));
      break;
      
    default:
      data->logger->error(F("publish unknow connect respose"));
      data->logger->notice(F("publish RC Reserved for future use"));
      break;
    }

  data->logger->notice(F("publish MQTT sessionPresent: %T"), connack.sessionPresent);
  if (connack.sessionPresent && (connack.rc != 0)){
    data->logger->error(F("publish inconsistent connect respose and session present "));
  }

  if (!returnstatus) return returnstatus;
  
  if (strcmp(data->station->ident,"") == 0){
    if (publish_maint()) {
      data->logger->notice(F("publish Published maint"));
      data->status->publish=ok;      
    }else{
      data->logger->error(F("publish Error in publish maint"));
      data->status->publish=error;
      return false;
    }
    if (publish_constantdata()) {
      data->logger->notice(F("publish Published constant data"));
      data->status->publish=ok;
    }else{
      data->logger->error(F("publish Error in publish constant data"));
      data->status->publish=error;
      return false;
    }
  }
  return true;
}

// publish one message to the MQTT broker (retained or not retained)
bool publishThread::mqttPublish( const mqttMessage_t& mqtt_message, const bool retained) {

    mqttclient.yield(0);
    MQTT::Message tx_message;
    tx_message.qos = MQTT::QOS1;
    tx_message.retained = retained;
    tx_message.dup = false;
    tx_message.payload = (void*) mqtt_message.payload;
    tx_message.payloadlen = strlen(mqtt_message.payload);

    data->logger->notice(F("publish Publish: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);
    
    MQTT::returnCode rc = (MQTT::returnCode) mqttclient.publish(mqtt_message.topic, tx_message);
    switch (rc){
    case MQTT::BUFFER_OVERFLOW:
      data->logger->error(F("publish BUFFER_OVERFLOW"));
      break;
    case MQTT::FAILURE:
      data->logger->error(F("publish FAILURE"));
      break;
    case MQTT::SUCCESS:
      data->logger->notice(F("publish SUCCESS"));
      break;
    }
    return (rc == MQTT::SUCCESS);
}

// publish maint messages (support messages)
// connection message with station version
bool publishThread::publish_maint() {

  mqttMessage_t mqtt_message;
  strcpy(mqtt_message.payload,"ERROR 01");
  strcpy(mqtt_message.topic,"1/");
  strcat(mqtt_message.topic,data->station->mqttmaintpath);
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,data->station->user);
  strcat(mqtt_message.topic,"//");
  strcat(mqtt_message.topic,data->station->longitude);
  strcat(mqtt_message.topic,",");
  strcat(mqtt_message.topic,data->station->latitude);
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,data->station->network);
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,"254,0,0");
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,"265,0,-,-");
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,"B01213");
  
  strcpy(mqtt_message.payload,"{\"v\":\"conn\",\"s\":");
  strcat(mqtt_message.payload,MAJOR_VERSION);
  strcat(mqtt_message.payload,",\"m\":");
  strcat(mqtt_message.payload,MINOR_VERSION);
  strcat(mqtt_message.payload,"}   ");
  
  return mqttPublish(mqtt_message, false); 
}

// publish constant data messages like station name and height
bool publishThread::publish_constantdata() {

  mqttMessage_t mqtt_message;

  for (uint8_t i=0; i< data->station->constantdata_count; i++){
  
      strcpy(mqtt_message.payload,"{\"v\":\"");
      strcat(mqtt_message.payload,data->station->constantdata[i].value);
      strcat(mqtt_message.payload,"\"}");
      
      strcpy(mqtt_message.topic,"1/");
      strcat(mqtt_message.topic,data->station->mqttrootpath);
      strcat(mqtt_message.topic,"/");
      strcat(mqtt_message.topic,data->station->user);
      strcat(mqtt_message.topic,"//");  
      strcat(mqtt_message.topic,data->station->longitude);
      strcat(mqtt_message.topic,",");
      strcat(mqtt_message.topic,data->station->latitude);
      strcat(mqtt_message.topic,"/");
      strcat(mqtt_message.topic,data->station->network);
      strcat(mqtt_message.topic,"/-,-,-/-,-,-,-/");
      strcat(mqtt_message.topic,data->station->constantdata[i].btable);

      if (!mqttPublish(mqtt_message, false)) return false;
    }

  return true;
}

// get one message from publish queue and send it to the queue for SD card archive
void publishThread::archive() {
  mqttMessage_t mqtt_message;
  if (data->mqttqueue->Dequeue(&mqtt_message, pdMS_TO_TICKS( 0 ))){;  // dequeue the message and archive
    mqtt_message.sent=0;
    if(data->dbqueue->Enqueue(&mqtt_message,pdMS_TO_TICKS(0))){
      data->logger->notice(F("publish enqueue for db"));
    }else{
      data->logger->error(F("publish lost message for db: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);
    }
  }else{
    data->logger->error(F("publish dequeue mqtt message"));
  }
 }

// if required connect to the broker, publish maint message, publish constant data messages
// try to send message to the broker
// send the same message to the queue for archive with flag to describe if publish is completed with success
bool publishThread::doPublish(mqttMessage_t& mqtt_message) {

  bool rc=false;
  mqtt_message.sent=0;
  
  mqttMessage_t tmp_mqtt_message;  
  data->mqttqueue->Dequeue(&tmp_mqtt_message, pdMS_TO_TICKS( 0 ));

  if(mqttPublish(mqtt_message,false)){
    mqtt_message.sent=1;  // all done: archive
    rc=true;
  }

  if(!data->dbqueue->Enqueue(&mqtt_message,pdMS_TO_TICKS(0))){
    data->logger->error(F("publish lost message for db: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);
  }
  
  if ( rc ){
    data->status->publish=ok;
  } else {
    data->status->publish=error;
  }
  
  return rc;
}

publishThread::publishThread(publish_data_t* publish_data)
  : Thread{"publish", 3500, 1},
    data{publish_data},
    ipstack{*data->mqttClient},
    mqttclient{ipstack, IP_STACK_TIMEOUT_MS}
{
  //data->logger->notice("Create Thread %s %d", GetName().c_str(), data->id);
  data->status->connect=unknown;
  data->status->publish=unknown;
  //Start();
};

publishThread::~publishThread()
{
}


/*
static void publishThread::WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  data->logger->error(F("Disconnected from WiFi access point"));
  data->logger->notice(F("WiFi lost connection. Reason: %s"),info.wifi_sta_disconnected.reason);
  data->logger->notice(F("Trying to Reconnect"));
  mqttDisconnect(ipstack,mqttclient, data);
  WiFi.disconnect();
  WiFi.reconnect();
}
*/
void publishThread::Cleanup()
{
  data->logger->notice("Delete Thread %s %d", GetName().c_str(), data->id);
  data->status->connect=unknown;
  data->status->publish=unknown;
  delete this;
}
  
void publishThread::Run() {
  data->logger->notice("Starting Thread %s %d", GetName().c_str(), data->id);

  //WiFi.onEvent(publishThread::WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  /*
  rpcRecovery_t rpcrecovery;  
  strcpy(rpcrecovery.dtstart,"2024-05-09T00:00:00") ;
  strcpy(rpcrecovery.dtend, "2024-05-09T01:00:00") ;
  
  if(data->recoveryqueue->Enqueue(&rpcrecovery)){
    data->logger->notice(F("enqueue rpc recovery : %s ; %s"), rpcrecovery.dtstart, rpcrecovery.dtend);
  }else{
    data->logger->error(F("enqueue rpc recovery : %s ; %s"), rpcrecovery.dtstart, rpcrecovery.dtend);
  }
  */
  
  for(;;){
    
    // if there are no enough space left on the mqtt queue send it to the DB archive
    while (data->mqttqueue->NumSpacesLeft() < MQTT_QUEUE_SPACELEFT_PUBLISH){
      archive();
    }

    // https://github.com/eclipse/paho.mqtt.embedded-c/issues/110
    // mqttclient.yield(5000);
    // data->logger->notice(F("publish MQTT connect status %T %T %T"),mqttclient.yield(0)!=0, !mqttclient.isConnected(), WiFi.status() != WL_CONNECTED);
    // if ( (mqttclient.yield(1000)!=0) || (!mqttclient.isConnected()) || (WiFi.status() != WL_CONNECTED)){

    //if ((!mqttclient.isConnected()) || (WiFi.status() != WL_CONNECTED)){

    mqttclient.yield(0);
    
    if (data->status->publish != ok){
      mqttDisconnect();
      if (mqttConnect(true)) {   // manage mqtt reconnect as RMAP standard
	data->logger->notice(F("publish MQTT connected"));
	data->status->connect=ok;
      } else {
	data->status->connect=error;
	data->logger->error(F("publish MQTT connect failed"));
	data->status->publish=error;
	delay(3000);
      }
    }
    
    if (data->status->connect==ok){
      mqttMessage_t mqttMessage;
      // wait for message and peek it from the queue
      while (data->mqttqueue->Peek(&mqttMessage, pdMS_TO_TICKS( 1000 ))){
	// publish message
	if (!doPublish(mqttMessage)) break;
      }
    }
    
    data->logger->notice(F("publish mqtt queue space left %d"),data->mqttqueue->NumSpacesLeft());

    // check heap and stack
    //data->logger->notice(F("HEAP: %l"),esp_get_minimum_free_heap_size());
    if( esp_get_minimum_free_heap_size() < HEAP_MIN_WARNING)data->logger->error(F("HEAP: %l"),esp_get_minimum_free_heap_size());
    //data->logger->notice("stack publish: %d",uxTaskGetStackHighWaterMark(NULL));
    if( uxTaskGetStackHighWaterMark(NULL) < STACK_MIN_WARNING )data->logger->error(F("publish stack"));
  }
};


//https://stackoverflow.com/questions/400257/how-can-i-pass-a-class-member-function-as-a-callback

/*
void mqttRxCallback(MQTT::MessageData &md, publish_data_t& data) {
  MQTT::Message &rx_message = md.message;

  data->logger->notice(F("MQTT RX: %s"), (char*)rx_message.payload);
  data->logger->notice(F("--> len %d"), rx_message.payloadlen);
  data->logger->notice(F("--> qos %d"), rx_message.qos);
  data->logger->notice(F("--> retained %d"), rx_message.retained);
  data->logger->notice(F("--> dup %d"), rx_message.dup);
  data->logger->notice(F("--> id %d"), rx_message.id);

  bool is_event_rpc=true;
  while (is_event_rpc) {                                  // here we block because pahoMQTT is blocking
    //streamRpc.parseCharpointer(&is_event_rpc, (char*)rx_message.payload, rx_message.payloadlen, rpcpayload, MQTT_RPC_RESPONSE_LENGTH );
  }
}

bool publishThread::mqttSubscribeRpc(char* comtopic) {

  // remove previous handler               
  // MessageHandler in MQTTClient is not cleared betwen sessions  
  mqttclient.setMessageHandler(comtopic, NULL);

  auto callback = std::bind(&mqttRxCallback,std::placeholders::_1,data); 
  bool returnstatus = mqttclient.subscribe(comtopic, MQTT::QOS1, callback) == 0;    // subscribe and set new handler

  data->logger->notice(F("MQTT Subscription %s [ %s ]"), comtopic, returnstatus ? "OK" : "FAIL");

  return returnstatus;
}
*/
