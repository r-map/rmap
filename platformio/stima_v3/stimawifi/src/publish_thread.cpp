#include "stimawifi.h"

bool publish_maint(publish_data_t& data) {

  data.logger->notice(F("Connet to mqtt broker"));

  char mqttid[100]="";
  strcat(mqttid,data.station->user);
  strcat(mqttid,"/");
  strcat(mqttid,data.station->slug);
  strcat(mqttid,"/default");
  
  data.logger->notice(F("mqttid: %s"),mqttid);
  
  char mainttopic[100]="1/";
  strcat(mainttopic,data.station->mqttmaintpath);
  strcat(mainttopic,"/");
  strcat(mainttopic,data.station->user);
  strcat(mainttopic,"//");  
  strcat(mainttopic,data.station->longitude);
  strcat(mainttopic,",");
  strcat(mainttopic,data.station->latitude);
  strcat(mainttopic,"/");
  strcat(mainttopic,data.station->network);
  strcat(mainttopic,"/254,0,0/265,0,-,-/B01213");
  data.logger->notice(F("MQTT maint topic: %s"),mainttopic);
    
  if (!mqttclient.connect(mqttid,mqttid,data.station->password,mainttopic,1,1,"{\"v\":\"error01\"}")){
    data.logger->error(F("Error connecting MQTT"));
    data.logger->error(F("Error status %d"),mqttclient.state());
    data.status->connect=error;
    return false;
  }
  data.logger->notice(F("MQTT connected"));
  data.status->connect=ok;
  if (!mqttclient.publish(mainttopic,(uint8_t*)"{\"v\":\"conn\",\"s\":" MAJOR_VERSION ",\"m\":" MINOR_VERSION "}   ", 34,1)){ //padded 3 blank char for time
    //if (!mqttclient.publish(mainttopic,(uint8_t*)"{\"v\":\"conn\"}", 12,1)){
    data.logger->error(F("MQTT maint not published"));
    mqttclient.disconnect();
    return false;
  }
  data.logger->notice(F("MQTT maint published"));
  return true;
}


bool publish_constantdata(publish_data_t& data) {

  char topic[100];

  /////////////////////////////////////// remove !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  String payload=readconfig_rmap();
  //////////////////////////////////////////////////////////////////////////////////////////////

  if (! (payload == String())) {
    //StaticJsonDocument<2900> doc;
    DynamicJsonDocument doc(4000);
    DeserializationError deerror = deserializeJson(doc,payload);
    if (!deerror) {
      JsonArrayConst array = doc.as<JsonArray>();
      //for (uint8_t i = 0; i < array.size(); i++) {
      for(JsonObjectConst element: array){ 
	if  (element["model"] == "stations.stationconstantdata"){
	  if (element["fields"]["active"]){
	    data.logger->notice(F("station constant data found!"));
	    char btable[7];
	    strncpy (btable, element["fields"]["btable"].as< const char*>(),6);
	    btable[6]='\0';
	    data.logger->notice(F("btable: %s"),btable);
	    char value[31];
	    strncpy (value, element["fields"]["value"].as< const char*>(),30);
	    value[30]='\0';
	    data.logger->notice(F("value: %s"),value);

	    char payload[100]="{\"v\":\"";
	    strcat(payload,value);
	    strcat(payload,"\"}");
      
	    strcpy(topic,"1/");
	    strcat(topic,data.station->mqttrootpath);
	    strcat(topic,"/");
	    strcat(topic,data.station->user);
	    strcat(topic,"//");  
	    strcat(topic,data.station->longitude);
	    strcat(topic,",");
	    strcat(topic,data.station->latitude);
	    strcat(topic,"/");
	    strcat(topic,data.station->network);
	    strcat(topic,"/-,-,-/-,-,-,-/");
	    strcat(topic,btable);

	    data.logger->notice(F("mqtt publish: %s %s"),topic,payload);
	    if (!mqttclient.publish(topic, payload)){
	      data.logger->error(F("MQTT data not published"));
	      mqttclient.disconnect();
	      return false;
	    }
	    data.logger->notice(F("MQTT data published"));
	  }
	}
      }
    } else {
      data.logger->error(F("error parsing array: %s"),deerror.c_str());
      return false;
    }
    
  }else{
    return false;
  }
  return true;
}

void doPublish(publish_data_t& data, const mqttMessage_t* mqtt_message) {

  // manage mqtt reconnect as RMAP standard
  if (!mqttclient.connected()){
    if (!publish_maint(data)) {
      data.logger->error(F("Error in publish maint"));
      data.status->publish=error;
    }else{
      data.logger->error(F("Published maint"));
      data.status->publish=ok;      
    }

    if (!publish_constantdata(data)) {
      data.logger->error(F("Error in publish constant data"));
      data.status->publish=error;
    }else{
      data.logger->error(F("Published constant data"));
      data.status->publish=ok;
    }    
  }

  data.logger->notice(F("Publish: %s ; %s"),  mqtt_message->topic, mqtt_message->payload);
  if(mqttclient.publish(mqtt_message->topic, mqtt_message->payload)){
    //if(publish_data(values,sensors[i].timerange,sensors[i].level)){
    data.logger->notice(F("Data published"));    
    data.status->publish=ok;
  }else{
    mqttclient.disconnect(); ////////////////////////////////////// do to ?
    data.logger->error(F("Error in publish data"));
    data.status->publish=error;
  }
}

publishThread::publishThread(publish_data_t &publish_data)
  : Thread{"publish", 50000, 1},
    data{publish_data}
{
  //data.logger->notice("Create Thread %s %d", GetName().c_str(), data.id);
  data.status->connect=unknown;
  data.status->publish=unknown;
  //Start();
};

publishThread::~publishThread()
{
  data.logger->notice("Delete Thread %s %d", GetName().c_str(), data.id);
  data.status->connect=unknown;
  data.status->publish=unknown;
}
  
void publishThread::Cleanup()
{
  delete this;
}
  
void publishThread::Run() {
  data.logger->notice("Starting Thread %s %d", GetName().c_str(), data.id);
  for(;;){
    mqttMessage_t mqttMessage;
    data.mqttqueue->Dequeue(&mqttMessage);     
    doPublish(data,&mqttMessage);
  }
};
