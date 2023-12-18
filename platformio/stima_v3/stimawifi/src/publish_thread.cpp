#include "publish_thread.h"

bool publish_maint() {

  const String data;  
  
  //String clientId = "ESP8266Client-";
  //clientId += String(random(0xffff), HEX);
    
  frtosLog.notice(F("Connet to mqtt broker"));

  char mqttid[100]="";
  strcat(mqttid,rmap_user);
  strcat(mqttid,"/");
  strcat(mqttid,rmap_slug);
  strcat(mqttid,"/default");
  
  frtosLog.notice(F("mqttid: %s"),mqttid);
  
  char mainttopic[100]="1/";
  strcat(mainttopic,rmap_mqttmaintpath);
  strcat(mainttopic,"/");
  strcat(mainttopic,rmap_user);
  strcat(mainttopic,"//");  
  strcat(mainttopic,rmap_longitude);
  strcat(mainttopic,",");
  strcat(mainttopic,rmap_latitude);
  strcat(mainttopic,"/");
  strcat(mainttopic,rmap_network);
  strcat(mainttopic,"/254,0,0/265,0,-,-/B01213");
  frtosLog.notice(F("MQTT maint topic: %s"),mainttopic);
    
  if (!mqttclient.connect(mqttid,mqttid,rmap_password,mainttopic,1,1,"{\"v\":\"error01\"}")){
    frtosLog.error(F("Error connecting MQTT"));
    frtosLog.error(F("Error status %d"),mqttclient.state());
    return false;
  }
  frtosLog.notice(F("MQTT connected"));
  yield();
  if (!mqttclient.publish(mainttopic,(uint8_t*)"{\"v\":\"conn\",\"s\":" MAJOR_VERSION ",\"m\":" MINOR_VERSION "}   ", 34,1)){ //padded 3 blank char for time
    //if (!mqttclient.publish(mainttopic,(uint8_t*)"{\"v\":\"conn\"}", 12,1)){
    frtosLog.error(F("MQTT maint not published"));
    mqttclient.disconnect();
    return false;
  }
  frtosLog.notice(F("MQTT maint published"));
  return true;
}


bool publish_constantdata() {

  char topic[100];

  /////////////////////////////////////// remove !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  String payload=readconfig_rmap();
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  if (! (payload == String())) {
    //StaticJsonDocument<2900> doc;
    DynamicJsonDocument doc(4000);
    DeserializationError error = deserializeJson(doc,payload);
    if (!error) {
      JsonArrayConst array = doc.as<JsonArray>();
      //for (uint8_t i = 0; i < array.size(); i++) {
      for(JsonObjectConst element: array){ 
	if  (element["model"] == "stations.stationconstantdata"){
	  if (element["fields"]["active"]){
	    frtosLog.notice(F("station constant data found!"));
	    char btable[7];
	    strncpy (btable, element["fields"]["btable"].as< const char*>(),6);
	    btable[6]='\0';
	    frtosLog.notice(F("btable: %s"),btable);
	    char value[31];
	    strncpy (value, element["fields"]["value"].as< const char*>(),30);
	    value[30]='\0';
	    frtosLog.notice(F("value: %s"),value);

	    char payload[100]="{\"v\":\"";
	    strcat(payload,value);
	    strcat(payload,"\"}");
      
	    strcpy(topic,"1/");
	    strcat(topic,rmap_mqttrootpath);
	    strcat(topic,"/");
	    strcat(topic,rmap_user);
	    strcat(topic,"//");  
	    strcat(topic,rmap_longitude);
	    strcat(topic,",");
	    strcat(topic,rmap_latitude);
	    strcat(topic,"/");
	    strcat(topic,rmap_network);
	    strcat(topic,"/-,-,-/-,-,-,-/");
	    strcat(topic,btable);

	    frtosLog.notice(F("mqtt publish: %s %s"),topic,payload);
	    if (!mqttclient.publish(topic, payload)){
	      frtosLog.error(F("MQTT data not published"));
	      mqttclient.disconnect();
	      return false;
	    }
	    frtosLog.notice(F("MQTT data published"));
	  }
	}
      }
    } else {
      frtosLog.error(F("error parsing array: %s"),error.c_str());
      analogWrite(LED_PIN,973);
      delay(5000);
      return false;
    }
    
  }else{
    return false;
  }
  return true;
}

void doPublish(const mqttMessage_t* mqtt_message) {

  // manage mqtt reconnect as RMAP standard
  if (!mqttclient.connected()){
    if (!publish_maint()) {
      frtosLog.error(F("Error in publish maint"));
      if (oledpresent) {
	u8g2.clearBuffer();
	u8g2.setCursor(0, 20); 
	u8g2.print(F("MQTT Error maint"));
	u8g2.sendBuffer();
	u8g2.clearBuffer();
	delay(3000);
      }else{
	// if we do not have display terminate (we do not display values)
	analogWrite(LED_PIN,512);
	delay(5000);
	digitalWrite(LED_PIN,HIGH);
	//return;
      }
    }

    if (!publish_constantdata()) {
      frtosLog.error(F("Error in publish constant data"));
      if (oledpresent) {
	u8g2.clearBuffer();
	u8g2.setCursor(0, 20); 
	u8g2.print(F("MQTT Error constant"));
	u8g2.sendBuffer();
	u8g2.clearBuffer();
	delay(3000);
      }else{
	// if we do not have display terminate (we do not display values)
	analogWrite(LED_PIN,512);
	delay(5000);
	digitalWrite(LED_PIN,HIGH);
	//return;
      }
    }    
  }

  frtosLog.notice(F("Publish: %s ; %s"),  mqtt_message->topic, mqtt_message->payload);
  if(mqttclient.publish(mqtt_message->topic, mqtt_message->payload)){
    //if(publish_data(values,sensors[i].timerange,sensors[i].level)){
    frtosLog.notice(F("Data published"));    
  }else{
    mqttclient.disconnect(); ////////////////////////////////////// do to ?
    frtosLog.error(F("Error in publish data"));
    if (oledpresent) {
      u8g2.setCursor(0, (displaypos++)*CH); 
      u8g2.print(F("MQTT error publish"));
    }else{
      analogWrite(LED_PIN,973);
      delay(5000);
    }
  }
}



publishThread::publishThread(publish_data_t &publish_data)
  : Thread("publish", 10000, 1),
    data(publish_data)
{
  data.logger.notice("Create Thread %s %d", GetName().c_str(), data.id);
  //Start();
};

publishThread::~publishThread()
{
  data.logger.notice("Delete Thread %s %d", GetName().c_str(), data.id);
}
  
void publishThread::Cleanup()
{
  delete this;
}
  
void publishThread::Run() {
  data.logger.notice("Starting Thread %s %d", GetName().c_str(), data.id);
  for(;;){
    mqttMessage_t mqttMessage;
    data.mqttqueue.Dequeue(&mqttMessage);     
    doPublish(&mqttMessage);
  }
};
