#include "stimawifi.h"

/*
void mqttRxCallback(MQTT::MessageData &md) {
  MQTT::Message &rx_message = md.message;

  data.logger->notice(F("MQTT RX: %s"), (char*)rx_message.payload);
  data.logger->notice(F("--> len %d"), rx_message.payloadlen);
  data.logger->notice(F("--> qos %d"), rx_message.qos);
  data.logger->notice(F("--> retained %d"), rx_message.retained);
  data.logger->notice(F("--> dup %d"), rx_message.dup);
  data.logger->notice(F("--> id %d"), rx_message.id);

  is_event_rpc=true;
  while (is_event_rpc) {                                  // here we block because pahoMQTT is blocking
    streamRpc.parseCharpointer(&is_event_rpc, (char*)rx_message.payload, rx_message.payloadlen, rpcpayload, MQTT_RPC_RESPONSE_LENGTH );
  }
}

bool mqttSubscribeRpc(publish_data_t& data, MQTT::connackData& data) {
  // remove previous handler               
  // MessageHandler in MQTTClient is not cleared betwen sessions  
  mqtt_client.setMessageHandler(comtopic, NULL);
  bool returnstatus = mqtt_client.subscribe(comtopic, MQTT::QOS1, mqttRxCallback) == 0;    // subscribe and set new handler
  data.logger->notice(F("MQTT Subscription %s [ %s ]"), comtopic, returnstatus ? OK_STRING : FAIL_STRING);

  return returnstatus;
}

bool mqttDisconnect(publish_data_t& data, MQTT::connackData& data) {

  mqtt_client.disconnect();
  mqtt_client.setMessageHandler(comtopic, NULL); // remove handler setted
  ipstack.disconnect();
  data.logger->notice(F("MQTT Disconnect... [ %s ]"), OK_STRING);
  return true;
}

*/

bool mqttConnect(IPStack& ipstack, MQTT::Client<IPStack, Countdown, MQTT_PACKET_SIZE, 1 >& mqttclient, publish_data_t& data, const bool cleanSession=true) {

  bool returnstatus = ipstack.connect(data.station->mqtt_server, MQTT_SERVER_PORT);
  if (returnstatus){
    data.logger->notice(F("IPstack connected"));
  } else {
    data.logger->error(F("IPstack connect failed"));
    return returnstatus;
  }
  
  char username[100];
  strcpy(username, data.station->user);
  strcat(username, "/");
  strcat(username, data.station->stationslug);
  strcat(username, "/");
  strcat(username, data.station->boardslug);

  mqttMessage_t mqtt_message;
  strcpy(mqtt_message.payload,"ERROR 01");
  strcpy(mqtt_message.topic,"1/");
  strcat(mqtt_message.topic,data.station->mqttmaintpath);
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,data.station->user);
  strcat(mqtt_message.topic,"//");
  strcat(mqtt_message.topic,data.station->longitude);
  strcat(mqtt_message.topic,",");
  strcat(mqtt_message.topic,data.station->latitude);
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,data.station->network);
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,"254,0,0");
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,"265,0,-,-");
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,"B01000");

  MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
  options.MQTTVersion = 4;   // Version of MQTT to be used.  3 = 3.1 4 = 3.1.1
  options.will.topicName.cstring = mqtt_message.topic;
  options.will.message.cstring=mqtt_message.payload;
  options.will.retained = true;
  options.will.qos = MQTT::QOS1;
  options.willFlag = true;
  options.clientID.cstring = username;
  options.username.cstring = username;
  options.password.cstring = data.station->password;
  options.cleansession = cleanSession;
  options.keepAliveInterval = 60;
    
  data.logger->notice(F("MQTT clientID: %s"), options.clientID.cstring);
  data.logger->notice(F("MQTT will topic: %s"), options.will.topicName.cstring);
  data.logger->notice(F("MQTT will payload: %s"), options.will.message.cstring);
  data.logger->notice(F("MQTT cleansession: %s"), options.cleansession ? "true" : "false");
  data.logger->notice(F("MQTT server: %s"), data.station->mqtt_server);
  data.logger->notice(F("MQTT keepAliveInterval: %d"), options.keepAliveInterval);
  data.logger->notice(F("MQTT user: %s"), options.username.cstring);
  data.logger->notice(F("MQTT password: %s"), options.password.cstring);
    
  MQTT::connackData connack;
  returnstatus = mqttclient.connect(options,connack);
  if (returnstatus == 0){
    data.logger->notice(F("mqttclient connected"));
  } else {
    data.logger->notice(F("mqttclient connect failed"));
    return returnstatus == 0;
  }
  
  data.logger->notice(F("MQTT sessionPresent: %T"), connack.sessionPresent);
  
  returnstatus=false;
  switch (connack.rc)
    {
    case 0:
      data.logger->notice(F("Connection accepted"));
      returnstatus=true;
      break;
    case 1:
      data.logger->error(F("Connection Refused"));
      data.logger->notice(F("The Server does not support the level of the MQTT protocol requested by the Client"));
      break;
      
    case 2:
      data.logger->error(F("Connection Refused"));
      data.logger->notice(F("The Client identifier is correct UTF-8 but not allowed by the Server"));
      break;
       
    case 3:
      data.logger->error(F("Connection Refused"));
      data.logger->notice(F("The Network Connection has been made but the MQTT service is unavailable"));
      break;
      
    case 4:
      data.logger->error(F("Connection Refused"));
      data.logger->notice(F("bad user name or password"));
      break;
      
    case 5:
      data.logger->error(F("Connection Refused"));
      data.logger->notice(F("not authorized"));
      break;
      
    default:
      data.logger->error(F("unknow connect respose"));
      data.logger->notice(F("RC Reserved for future use"));
      break;
    }

  if (connack.sessionPresent && (connack.rc != 0)){
    data.logger->error(F("inconsistent connect respose and session present "));
  }
  return returnstatus;
}


bool mqttPublish(MQTT::Client<IPStack, Countdown, MQTT_PACKET_SIZE, 1 >& mqttclient, publish_data_t& data, const mqttMessage_t& mqtt_message, const bool retained) {

    MQTT::Message tx_message;
    tx_message.qos = MQTT::QOS1;
    tx_message.retained = retained;
    tx_message.dup = false;
    tx_message.payload = (void*) mqtt_message.payload;
    tx_message.payloadlen = strlen(mqtt_message.payload);

    data.logger->notice(F("Publish: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);
    
    MQTT::returnCode rc = (MQTT::returnCode) mqttclient.publish(mqtt_message.topic, tx_message);
    switch (rc){
    case MQTT::BUFFER_OVERFLOW:
      data.logger->error(F("publish BUFFER_OVERFLOW"));
      break;
    case MQTT::FAILURE:
      data.logger->error(F("publish FAILURE"));
      break;
    case MQTT::SUCCESS:
      data.logger->notice(F("publish SUCCESS"));
      break;
    }
    return (rc == MQTT::SUCCESS);
}

bool publish_maint(MQTT::Client<IPStack, Countdown, MQTT_PACKET_SIZE, 1 >& mqttclient, publish_data_t& data) {

  mqttMessage_t mqtt_message;
  strcpy(mqtt_message.payload,"ERROR 01");
  strcpy(mqtt_message.topic,"1/");
  strcat(mqtt_message.topic,data.station->mqttmaintpath);
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,data.station->user);
  strcat(mqtt_message.topic,"//");
  strcat(mqtt_message.topic,data.station->longitude);
  strcat(mqtt_message.topic,",");
  strcat(mqtt_message.topic,data.station->latitude);
  strcat(mqtt_message.topic,"/");
  strcat(mqtt_message.topic,data.station->network);
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
  
  return mqttPublish(mqttclient, data, mqtt_message, false); 
}


bool publish_constantdata(MQTT::Client<IPStack, Countdown, MQTT_PACKET_SIZE, 1 >& mqttclient, publish_data_t& data) {

  mqttMessage_t mqtt_message;

  for (uint8_t i=0; i< data.station->constantdata_count; i++){
  
      strcpy(mqtt_message.payload,"{\"v\":\"");
      strcat(mqtt_message.payload,data.station->constantdata[i].value);
      strcat(mqtt_message.payload,"\"}");
      
      strcpy(mqtt_message.topic,"1/");
      strcat(mqtt_message.topic,data.station->mqttrootpath);
      strcat(mqtt_message.topic,"/");
      strcat(mqtt_message.topic,data.station->user);
      strcat(mqtt_message.topic,"//");  
      strcat(mqtt_message.topic,data.station->longitude);
      strcat(mqtt_message.topic,",");
      strcat(mqtt_message.topic,data.station->latitude);
      strcat(mqtt_message.topic,"/");
      strcat(mqtt_message.topic,data.station->network);
      strcat(mqtt_message.topic,"/-,-,-/-,-,-,-/");
      strcat(mqtt_message.topic,data.station->constantdata[i].btable);

      bool returnstatus=mqttPublish(mqttclient, data, mqtt_message, false);
      if (!returnstatus) return false;
    }

  return true;
}


void doPublish(IPStack& ipstack, MQTT::Client<IPStack, Countdown, MQTT_PACKET_SIZE, 1 >& mqttclient, publish_data_t& data, const mqttMessage_t& mqtt_message) {

  // manage mqtt reconnect as RMAP standard
  if (!mqttclient.isConnected()){
    if (mqttConnect(ipstack,mqttclient, data, true)) {
      data.status->connect=ok;
      if (!publish_maint(mqttclient,data)) {
	data.logger->error(F("Error in publish maint"));
	data.status->publish=error;
      }else{
	data.logger->notice(F("Published maint"));
	data.status->publish=ok;      
      }
      if (!publish_constantdata(mqttclient,data)) {
	data.logger->error(F("Error in publish constant data"));
	data.status->publish=error;
      }else{
	data.logger->notice(F("Published constant data"));
	data.status->publish=ok;
      }
    } else {
      data.status->connect=error;
      data.logger->error(F("MQTT connect failed"));
      data.status->publish=error;
    }
  }

  if (mqttclient.isConnected()){
    if(mqttPublish( mqttclient, data, mqtt_message,false)){
      data.logger->notice(F("Data published"));    
      data.status->publish=ok;
    }else{
      //mqttclient.disconnect(); ////////////////////////////////////// do to ?
      data.logger->error(F("Error in publish data"));
      data.status->publish=error;
    }
  }
}

publishThread::publishThread(publish_data_t &publish_data)
  : Thread{"publish", 50000, 1},
    data{publish_data},
    ipstack{*data.mqttClient},
    mqttclient{ipstack, IP_STACK_TIMEOUT_MS}
{
  //data.logger->notice("Create Thread %s %d", GetName().c_str(), data.id);
  data.status->connect=unknown;
  data.status->publish=unknown;
  //Start();
};

publishThread::~publishThread()
{
}
  
void publishThread::Cleanup()
{
  data.logger->notice("Delete Thread %s %d", GetName().c_str(), data.id);
  data.status->connect=unknown;
  data.status->publish=unknown;
  delete this;
}
  
void publishThread::Run() {
  data.logger->notice("Starting Thread %s %d", GetName().c_str(), data.id);
  for(;;){
    mqttMessage_t mqttMessage;
    while (data.mqttqueue->Dequeue(&mqttMessage, pdMS_TO_TICKS( 1000 ))){
      doPublish(ipstack,mqttclient, data, mqttMessage);
    }
    data.logger->notice(F("mqtt yield"));
    mqttclient.yield(0);
  }
};
