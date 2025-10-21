#include "common.h"

//***********************************************************************************************
//                         global definition to use in RPC callback
// initialize an instance of the JsonRPC library for registering 
// exactly 1 local method
//radio mode is false; do not use compact protocoll
JsonRPC publishThread::global_jsonrpc = JsonRPC(false);
// pointers setted by class istance
publish_data_t* publishThread::global_data=NULL;
MQTT::Client<IPStack, Countdown, MQTT_PACKET_SIZE, 2 >* publishThread::global_mqttclient=NULL;
//***********************************************************************************************

publishThread::publishThread(publish_data_t* publish_data)
  : Thread{"publish", TASK_PUBLISH_STACK_SIZE, TASK_PUBLISH_PRIORITY},
    data{publish_data},
    ipstack{networkClient},
    mqttclient{ipstack, IP_STACK_TIMEOUT_MS},
    bootConnect{true}
{
  //data->logger->notice("Create Thread %s %d", GetName().c_str(), data->id);
  data->status->publish.connect=unknown;
  data->status->publish.publish=unknown;
  connect_errorcount=0;
  status_published=false;
  status_connected=false;

  global_data=data;
  global_mqttclient=&mqttclient;
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
  data->status->publish.connect=unknown;
  data->status->publish.publish=unknown;
  status_connected=false;
  status_published=false;
  delete this;
}
  
void publishThread::Run() {
  data->logger->notice("Starting Thread %s %d", GetName().c_str(), data->id);

  //WiFi.onEvent(publishThread::WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  last_status_sended = millis()/1000;

  // register RPC
  global_jsonrpc.registerMethod("pinout", &pinOutRpc);
  global_jsonrpc.registerMethod("recovery", &recoveryDataRpc);
  global_jsonrpc.registerMethod("reboot", &rebootRpc);
  
  for(;;){
    
    // if there are no enough space left on the mqtt queue send it to the DB
    while (data->mqttqueue->NumSpacesLeft() < MQTT_QUEUE_SPACELEFT_PUBLISH){
      store();
      data->status->publish.publish=error;
    }

    // https://github.com/eclipse/paho.mqtt.embedded-c/issues/110
    // mqttclient.yield(5000);
    // data->logger->notice(F("publish MQTT connect status %T %T %T"),mqttclient.yield(0)!=0, !mqttclient.isConnected(), WiFi.status() != WL_CONNECTED);
    // if ( (mqttclient.yield(1000)!=0) || (!mqttclient.isConnected()) || (WiFi.status() != WL_CONNECTED)){

    //if ((!mqttclient.isConnected()) || (WiFi.status() != WL_CONNECTED)){

    mqttclient.yield(0);
    
    if ( !status_published ){
      if (status_connected) mqttDisconnect();
      if (mqttConnect(false)) {   // manage mqtt reconnect as RMAP standard
	data->logger->notice(F("publish MQTT connected"));
	connect_errorcount = 0;
	data->status->publish.connect=ok;
      } else {
	data->logger->error(F("publish MQTT connect failed"));
	connect_errorcount++;
	if (connect_errorcount > 15) {
	  data->logger->error(F("publish too much error: drop WiFi"));
	  data->status->publish.connect=error;
	  WiFi.disconnect();
	  connect_errorcount = 0;
	}
	delay(3000);
      }
    }

    if (((millis()/1000)-last_status_sended) > STATUS_SEND_S) {
      if (publish_status_summary()) {
	data->logger->notice(F("publish Published maint status"));
	last_status_sended = millis()/1000;
	reset_status_summary();
      } else {
	data->logger->error(F("publish Publishing maint status"));
      }
    }

    if (status_connected){
      mqttMessage_t mqttMessage;
      // wait for message and peek it from the queue
      while (data->mqttqueue->Peek(&mqttMessage, pdMS_TO_TICKS( 1000 ))){
	mqttclient.yield(0);
	// publish message
	if (!doPublish()) break;
      }
    }
    
    data->logger->notice(F("publish mqtt queue space left %d"),data->mqttqueue->NumSpacesLeft());

    // check heap and stack
    //data->logger->notice(F("HEAP: %l"),esp_get_minimum_free_heap_size());
    if( esp_get_minimum_free_heap_size() < HEAP_MIN_WARNING){
      data->logger->error(F("HEAP: %l"),esp_get_minimum_free_heap_size());
      data->status->publish.no_heap_memory=error;
    }
    //data->logger->notice("stack publish: %d",uxTaskGetStackHighWaterMark(NULL));
    if( uxTaskGetStackHighWaterMark(NULL) < STACK_MIN_WARNING ){
      data->logger->error(F("publish stack"));  // check for memory collision
      data->status->publish.memory_collision=error;
    }
  }
};

// disconnect from MQTT broker
bool publishThread::mqttDisconnect() {

  /*
    we do not manage disconnect admin message
    disconnect in whole station is used for reconnect or for reboot
    so no long time is required for reconnect if all go right
    if not, no alert will be sent by alert system on server
    so I think is better do not disconnect with admin message
    
  mqttMessage_t mqtt_message;
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
  
  strcpy(mqtt_message.payload,"{\"v\":\"disconn\"}");
  return mqttPublish(mqtt_message, false); 
  */

  mqttclient.disconnect();
  status_connected=false;

  char comtopic[MQTT_SUBSCRIBE_TOPIC_LENGTH];

  strcpy(comtopic,"1/");
  strcat(comtopic,data->station->mqttrpcpath);
  strcat(comtopic,"/");
  strcat(comtopic,data->station->user);
  strcat(comtopic,"//");
  strcat(comtopic,data->station->longitude);
  strcat(comtopic,",");
  strcat(comtopic,data->station->latitude);
  strcat(comtopic,"/");
  strcat(comtopic,data->station->network);
  strcat(comtopic,"/com");

  mqttclient.setMessageHandler(comtopic, NULL); // remove handler setted
  
  ipstack.disconnect();
  data->logger->notice(F("publish MQTT Disconnectted"));
  return true;
}


// connect to MQTT broker setting will message
//  cleanSession=true;   //clean messages queued by MQTT broker
//  cleanSession=false;  // if I lost connectio when reconnect I get all messages queued by broker

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
  strcpy(mqtt_message.payload,"{\"v\":\"ERROR 01\"}");
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


  // the first connect after boot we skip all ald RPC queued by broker
  if (!bootConnect){
    char comtopic[MQTT_SUBSCRIBE_TOPIC_LENGTH];

    strcpy(comtopic,"1/");
    strcat(comtopic,data->station->mqttrpcpath);
    strcat(comtopic,"/");
    strcat(comtopic,data->station->user);
    strcat(comtopic,"//");
    strcat(comtopic,data->station->longitude);
    strcat(comtopic,",");
    strcat(comtopic,data->station->latitude);
    strcat(comtopic,"/");
    strcat(comtopic,data->station->network);
    strcat(comtopic,"/com");
    
    mqttclient.setDefaultMessageHandler(mqttRxCallback);
    mqttclient.setMessageHandler(comtopic, mqttRxCallback);   // messages queued for persistent client are sended at connect time and we have to "remember" the subscription
  } else {
    bootConnect=false;
  }
  
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
      data->status->publish.publish=ok;      
    }else{
      data->logger->error(F("publish Error in publish maint"));
      return false;
    }
    if (publish_constantdata()) {
      data->logger->notice(F("publish Published constant data"));
      data->status->publish.publish=ok;
    }else{
      data->logger->error(F("publish Error in publish constant data"));
      return false;
    }
  }

  if (mqttSubscribeRpc()) {
    data->logger->notice(F("publish Subscribed to rpc path"));
    data->status->publish.publish=ok;
  }else{
    data->logger->error(F("publish Subscribe to rpc path"));
    return false;
  }

  status_connected=true;
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

    uint8_t retry=0;
    MQTT::returnCode rc = MQTT::FAILURE; 
    while (rc == MQTT::FAILURE and retry++ <= 3) {
      data->logger->notice(F("publish Publish: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);
      rc = (MQTT::returnCode) mqttclient.publish(mqtt_message.topic, tx_message);
      switch (rc){
      case MQTT::BUFFER_OVERFLOW:
	data->logger->error(F("publish BUFFER_OVERFLOW"));
	break;
      case MQTT::FAILURE:
	data->logger->error(F("publish FAILURE"));
	status_published=false;
	break;
      case MQTT::SUCCESS:
	data->logger->notice(F("publish SUCCESS"));
	status_published=true;
	break;
      }
    }
    return (rc == MQTT::SUCCESS);
}

// publish maint messages (support messages)
// connection message with station version
bool publishThread::publish_maint() {

  mqttMessage_t mqtt_message;
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



void publishThread::reset_status_summary() {
  data->status->summary.err_power_on=false;
  data->status->summary.err_reboot=false;  
  data->status->summary.err_georef=false;	   
  // do not reset; task is dead, cannot recover or set it
  //data->status->summary.err_sdcard=false;
  data->status->summary.err_db=false;
  data->status->summary.err_archive=false;
  data->status->summary.err_mqtt_publish=false;
  data->status->summary.err_mqtt_connect=false;
  data->status->summary.err_geodef=false;
  data->status->summary.err_sensor=false;
  data->status->summary.err_novalue=false;
  data->status->summary.err_rtc=false;
  data->status->summary.err_memory=false;
}

// publish maint messages (support messages)
// connection message with station version
bool publishThread::publish_status_summary() {
  
  mqttMessage_t mqtt_message;
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
  
  // payload full example
  //Topic: 1/maint/simcv4//1198190,4404111/agrmet/254,0,0/265,0,-,-/B01213 Payload: {"t":"2025-05-04T14:45:00", "bs":"masterv4", "b":"0b0000000000000001", "c":[0,0,0,0]}

  // timestamp will be added by rmap server
  //
  //  if (timeStatus() == timeSet){
  //    char jsontime[30];
  //    time_t messagetime=now();
  //    snprintf(jsontime,28,"\"t\":\"%04u-%02u-%02uT%02u:%02u:%02u\"",
  //         year(messagetime), month(messagetime), day(messagetime),
  //	     hour(messagetime), minute(messagetime), second(messagetime));
  //    strcat(mqtt_message.payload,jsontime);
  //  }
  
  // "c" array is omitted by now
    
  // take in account error status only
  snprintf(mqtt_message.payload,MQTT_MESSAGE_LENGTH,"{\"bs\":\"%s\",\"b\":\"0b%d%d%d%d%d%d%d%d%d%d%d%d%d\"}"
	   //, jsontime
	   , data->station->boardslug

	   , data->status->summary.err_power_on	   
	   , data->status->summary.err_reboot	   
	   , data->status->summary.err_georef	   
	   , data->status->summary.err_sdcard	   
	   , data->status->summary.err_db	   
	   , data->status->summary.err_archive
	   , data->status->summary.err_mqtt_publish 
	   , data->status->summary.err_mqtt_connect 
	   , data->status->summary.err_geodef	   
	   , data->status->summary.err_sensor	   
	   , data->status->summary.err_novalue
	   , data->status->summary.err_rtc
	   , data->status->summary.err_memory
	   );
  // return true if published
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

// get one message from publish queue and send it to the queue for DB
// but if it is a resend message (sent == true) skip it; It's not a good thing but it's the lesser evil
void publishThread::store() {

  mqttMessage_t mqtt_message;

  if (data->mqttqueue->Dequeue(&mqtt_message, pdMS_TO_TICKS( 0 ))){;  // dequeue
    if (mqtt_message.sent){
      data->logger->error(F("publish skip and do no publish or store message sended before: %s ; %s"), mqtt_message.topic, mqtt_message.payload);
    }else{
      if(data->dbqueue->Enqueue(&mqtt_message,pdMS_TO_TICKS(0))){
	data->logger->notice(F("publish skip and enqueue message for db: %s ; %s"), mqtt_message.topic, mqtt_message.payload);
      }else{
	data->logger->error(F("publish lost message for db: %s ; %s"), mqtt_message.topic, mqtt_message.payload);
      }
    }
  }else{
    data->logger->error(F("publish getting message from mqtt queue"));
  }
}

// try to send message to the broker
// send the same message to the queue for DB with flag to describe if publish is completed with success
bool publishThread::doPublish() {

  bool rc=false;
  
  mqttMessage_t mqtt_message;  
  data->mqttqueue->Dequeue(&mqtt_message, pdMS_TO_TICKS( 0 ));

  bool resend = mqtt_message.sent;
  
  if(mqttPublish(mqtt_message,false)){
    mqtt_message.sent=1;  // all done: flag as sent
    rc=true;
  }

  // if it was already sendend skip it and do do not store
  if (!resend) {
    if(!data->dbqueue->Enqueue(&mqtt_message,pdMS_TO_TICKS(0))){
      data->logger->error(F("publish lost message for db: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);
    }
  }
  
  if ( rc ) data->status->publish.publish=ok;
  return rc;
}

//https://stackoverflow.com/questions/400257/how-can-i-pass-a-class-member-function-as-a-callback
//void mqttRxCallback(MQTT::MessageData &md, publish_data_t& data) {
//auto callback = std::bind(&mqttRxCallback,std::placeholders::_1,data); 
//bool returnstatus = mqttclient.subscribe(comtopic, MQTT::QOS1, callback) == 0;    // subscribe and set new handler

bool publishThread::mqttSubscribeRpc() {

  char comtopic[MQTT_SUBSCRIBE_TOPIC_LENGTH];

  strcpy(comtopic,"1/");
  strcat(comtopic,data->station->mqttrpcpath);
  strcat(comtopic,"/");
  strcat(comtopic,data->station->user);
  strcat(comtopic,"//");
  strcat(comtopic,data->station->longitude);
  strcat(comtopic,",");
  strcat(comtopic,data->station->latitude);
  strcat(comtopic,"/");
  strcat(comtopic,data->station->network);
  strcat(comtopic,"/com");

  // remove previous handler               
  // MessageHandler in MQTTClient is not cleared betwen sessions  
  //mqttclient.setMessageHandler(comtopic, NULL);

  // TODO subscribe seems do not set handle
  mqttclient.setDefaultMessageHandler(mqttRxCallback);
  mqttclient.setMessageHandler(comtopic, mqttRxCallback);   // messages queued for persistent client are sended at connect time and we have t
  bool returnstatus = mqttclient.subscribe(comtopic, MQTT::QOS1, mqttRxCallback) == 0;    // subscribe and set new handler

  if (returnstatus){
    data->logger->notice(F("publish MQTT subscribe %s"), comtopic);
  }else{
    data->logger->error(F("publish MQTT subscription %s"), comtopic);
  }    

  return returnstatus;
}

/*
  RPC reboot

  Richiede il riavvio della stazione
  parametri NON utilizzati:
  bool fupdate: true=update firmware available on SDcard
esempio:
{"jsonrpc": "2.0", "method": "reboot","params": {"fupdate":true}, "id": 0}
*/

int rebootRpc(JsonObject params, JsonObject result) {
  // reboot ESP
  publishThread::global_data->logger->notice(F("publish reboot rpc"));
  //publishThread::mqttDisconnect(); // cannot be called here
  delay(5000);
  ESP.restart();
  delay(5000);
  result[F("state")] = F("done");  
  return 0;  
}

/*
    RPC recovery

    Richiede il re-invio dei dati non trasmessi al server da una data iniziale a una data finale
    int[6] dts: start date and time; anno, mese, giorno, ora, minuti, secondi [esempio: 2014,2,10,18,45,18]
    int[6] dte (OPZIONALE): end date and time; anno, mese, giorno, ora, minuti, secondi [esempio: 2015,3,25,12,0,0]
    se omesso dte corrisponde all'istante attuale
esempi:
{"jsonrpc": "2.0", "method": "recovery", "params": {"dts":[2014,2,10,18,45,18],"dte":[2015,3,25,12,0,0] }, "id": 0}
{"jsonrpc": "2.0", "method": "recovery", "params": {"dts":[2014,2,10,18,45,18] }, "id": 1}
*/

int recoveryDataRpc(JsonObject params, JsonObject result) {

  rpcRecovery_t rpcrecovery;
  
  if (params.containsKey("dts")){
    JsonArray dts=params["dts"];
    sprintf(rpcrecovery.dtstart, "%04d-%02d-%02dT%02d:%02d:%02d",
	    dts[0].as<int>(),dts[1].as<int>(),dts[2].as<int>(),
	    dts[3].as<int>(),dts[4].as<int>(),dts[5].as<int>()
	    );
  }else{
    time_t dts=0;
    sprintf(rpcrecovery.dtend,"%04u-%02u-%02uT%02u:%02u:%02u",
	    year(dts), month(dts), day(dts),
	    hour(dts), minute(dts), second(dts));
  }
  
  if (params.containsKey("dte")){
    JsonArray dte=params["dte"];  
    sprintf(rpcrecovery.dtend, "%04u-%02u-%02uT%02u:%02u:%02u",
	    dte[0].as<int>(),dte[1].as<int>(),dte[2].as<int>(),
	    dte[3].as<int>(),dte[4].as<int>(),dte[5].as<int>()
	    );
  }else{
      time_t dte=now();
      sprintf(rpcrecovery.dtend,"%04u-%02u-%02uT%02u:%02u:%02u",
	      year(dte), month(dte), day(dte),
	      hour(dte), minute(dte), second(dte));
  }
 
  //strcpy(rpcrecovery.dtstart,"2024-05-09T00:00:00") ;
  //strcpy(rpcrecovery.dtend, "2024-05-09T01:00:00") ;
  
  if(publishThread::global_data->recoveryqueue->Enqueue(&rpcrecovery)){
    publishThread::global_data->logger->notice(F("enqueue rpc recovery : %s ; %s"), rpcrecovery.dtstart, rpcrecovery.dtend);
    result[F("state")] = F("done");  
    return 0;  
  }else{
    publishThread::global_data->logger->error(F("enqueue rpc recovery : %s ; %s"), rpcrecovery.dtstart, rpcrecovery.dtend);
    result[F("state")] = F("error");
    return 1;
  }
}

/*
  RPC pinout

  Attuatore che accende/spegne uno o più pin.
  parametri:
  array di oggetti con la seguente struttura:
        integer n: pin number
        bool s: true=on; false=off
esempio:
{"jsonrpc": "2.0", "method": "pinout", "params": [{"n":4,"s":true},{"n":5,"s":false}], "id": 0}
*/

int pinOutRpc(JsonObject params, JsonObject result) {

  JsonVariant params_variant =params;
  JsonArray array = params_variant;
  const uint8_t pins [] = {OUTPUTPINS};
  
  for(JsonVariant pinout : array) {
    if (pinout.containsKey("n")) {
      bool found=false;
      if (pinout.containsKey("n")) {
	uint8_t pin = pinout["n"];
	for(int i = 0; i < sizeof(pins) / sizeof(pins[0]); i++){
	  if(pins[i] == pin){
	    found=true;
	    break;
	  }
	}
	if (found){
	  bool status = pinout["value"];
	  pinMode(pin, OUTPUT);
	  digitalWrite(pin, status);
	}else{
	  result[F("state")] = F("error");
	  return 1;
	}
      } else {
	result[F("state")] = F("error");
	return 1;
      }
    } else {
      result[F("state")] = F("error");
      return 1;
    }
  }

  result[F("state")] = F("done");  
  return 0;  
}

// callback di ricezione di una RPC
void mqttRxCallback(MQTT::MessageData &md) {
  MQTT::Message &rx_message = md.message;

  publishThread::global_data->logger->notice(F("publish rpc MQTT RX: %s"), (char*)rx_message.payload);
  publishThread::global_data->logger->notice(F("publish rpc --> len %d"), rx_message.payloadlen);
  publishThread::global_data->logger->notice(F("publish rpc --> qos %d"), rx_message.qos);
  publishThread::global_data->logger->notice(F("publish rpc --> retained %d"), rx_message.retained);
  publishThread::global_data->logger->notice(F("publish rpc --> dup %d"), rx_message.dup);
  publishThread::global_data->logger->notice(F("publish rpc --> id %d"), rx_message.id);
 
  bool is_event_rpc=true;
  char rpcresponse[MQTT_RPC_RESPONSE_LENGTH];

  while (is_event_rpc) {                                  // here we block because pahoMQTT is blocking
    publishThread::global_jsonrpc.parseCharpointer(&is_event_rpc, (char*)rx_message.payload, rx_message.payloadlen, rpcresponse, MQTT_RPC_RESPONSE_LENGTH );
  }

  MQTT::Message tx_message;
  tx_message.qos = MQTT::QOS1;
  tx_message.retained = false;
  tx_message.dup = false;
  tx_message.payload = (void*) rpcresponse;
  tx_message.payloadlen = strlen(rpcresponse);

  char restopic[MQTT_SUBSCRIBE_TOPIC_LENGTH];

  strcpy(restopic,"1/");
  strcat(restopic,publishThread::global_data->station->mqttrpcpath);
  strcat(restopic,"/");
  strcat(restopic,publishThread::global_data->station->user);
  strcat(restopic,"//");
  strcat(restopic,publishThread::global_data->station->longitude);
  strcat(restopic,",");
  strcat(restopic,publishThread::global_data->station->latitude);
  strcat(restopic,"/");
  strcat(restopic,publishThread::global_data->station->network);
  strcat(restopic,"/res");

  // TODO: the publish of response do not SUCCESS when calback is called at connect time with not clean session
  // no reason found but paho mqtt bug
  // on the server this ends with a infinite running RPC
 
  uint8_t retry=0;
  MQTT::returnCode rc=MQTT::FAILURE;
  
  do {  
    publishThread::global_data->logger->notice(F("publish Publish: %s ; %s"),  restopic, rpcresponse);
    if (retry > 0) publishThread::global_mqttclient->yield(1000);

    rc = (MQTT::returnCode) publishThread::global_mqttclient->publish(restopic, tx_message);
    switch (rc){
    case MQTT::BUFFER_OVERFLOW:
      publishThread::global_data->logger->warning(F("publish rpc response BUFFER_OVERFLOW"));
      break;
    case MQTT::FAILURE:
      publishThread::global_data->logger->warning(F("publish rpc response FAILURE"));
      break;
    case MQTT::SUCCESS:
      publishThread::global_data->logger->notice(F("publish rpc response SUCCESS"));
      break;
    }    
  } while (rc != MQTT::SUCCESS and retry++ < 3);

  if (rc != MQTT::SUCCESS) publishThread::global_data->logger->error(F("publish rpc response"));
}
