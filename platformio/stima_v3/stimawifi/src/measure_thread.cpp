#include "stimawifi.h"
#include "measure_thread.h"

unsigned short int displaypos;

void display_values(const char* values,measure_data_t &data) {
  
  StaticJsonDocument<500> doc;

  data.logger->notice(F("display_values: %s"),values);
  
  DeserializationError error = deserializeJson(doc,values);
  if (!error) {
    JsonObject obj = doc.as<JsonObject>();
    for (JsonPair pair : obj) {

      if (pair.value().isNull()) continue;
      float val=pair.value().as<float>();
      
      if (strcmp(pair.key().c_str(),"B12101")==0){
	data.logger->notice(F("Temperature: %D"),val);
	u8g2.setCursor(0, (displaypos++)*CH); 
	u8g2.print(F("T   : "));
	u8g2.print(round((val-27315)/10.)/10,1);
	u8g2.print(F(" C"));
      }
      if (strcmp(pair.key().c_str(),"B13003")==0){
	data.logger->notice(F("Humidity: %D"),val);
	u8g2.setCursor(0, (displaypos++)*CH); 
	u8g2.print(F("U   : "));
	u8g2.print(round(val),0);
	u8g2.print(F(" %"));
      }
      if (strcmp(pair.key().c_str(),"B15198")==0){
	data.logger->notice(F("PM2: %D"),val);
	u8g2.setCursor(0, (displaypos++)*CH); 
	u8g2.print(F("PM2 : "));
	u8g2.print(round(val/10.),0);
	u8g2.print(F(" ug/m3"));
      }
      if (strcmp(pair.key().c_str(),"B15195")==0){
	data.logger->notice(F("PM10: %D"),val);
	u8g2.setCursor(0, (displaypos++)*CH); 
	u8g2.print(F("PM10: "));
	u8g2.print(round(val/10.),0);
	u8g2.print(F(" ug/m3"));
      }
      if (strcmp(pair.key().c_str(),"B15242")==0){
	data.logger->notice(F("CO2: %D"),val);
	u8g2.setCursor(0, (displaypos++)*CH); 
	u8g2.print(F("CO2 : "));
	u8g2.print(round(val/1.8),0);
	u8g2.print(F(" ppm"));
      }
    }
  }else{
    data.logger->error(F("display_values deserialization ERROR"));
  }  
}

void enqueueMqttMessage(const char* values, const char* timerange, const char* level, measure_data_t& data ) {
  
  mqttMessage_t mqtt_message;
  StaticJsonDocument<500> doc;

  data.logger->notice(F("have to publish: %s"),values);
  DeserializationError deerror = deserializeJson(doc,values);
  if (deerror) {
    data.logger->error(F("reading json data: %s"),deerror.c_str());
    return;
  }
  for (JsonPair pair : doc.as<JsonObject>()) {
    if (pair.value().isNull()){
      data.logger->error(F("novalue error"));
      data.status->novalue=error;
      continue;
    }
        
    strcpy(mqtt_message.topic,"1/");
    strcat(mqtt_message.topic,rmap_mqttrootpath);
    strcat(mqtt_message.topic,"/");
    strcat(mqtt_message.topic,rmap_user);
    strcat(mqtt_message.topic,"//");  
    strcat(mqtt_message.topic,rmap_longitude);
    strcat(mqtt_message.topic,",");
    strcat(mqtt_message.topic,rmap_latitude);
    strcat(mqtt_message.topic,"/");
    strcat(mqtt_message.topic,rmap_network);
    strcat(mqtt_message.topic,"/");
    strcat(mqtt_message.topic,timerange);
    strcat(mqtt_message.topic,"/");
    strcat(mqtt_message.topic,level);
    strcat(mqtt_message.topic,"/");
    strcat(mqtt_message.topic,pair.key().c_str());

    strcpy(mqtt_message.payload,"{\"v\":");
    char value[33];
    itoa(pair.value().as<uint32_t>(),value,10);
    strcat(mqtt_message.payload,value);
    strcat(mqtt_message.payload,"}");

    data.logger->notice(F("Enqueue: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);
    
    data.mqttqueue->Enqueue(&mqtt_message);
    
  }
}

void doMeasure( measure_data_t &data ) {

  uint32_t waittime,maxwaittime=0;

  char values[MAX_VALUES_FOR_SENSOR*20];
  size_t lenvalues=MAX_VALUES_FOR_SENSOR*20;
  //  long values[MAX_VALUES_FOR_SENSOR];
  //  size_t lenvalues=MAX_VALUES_FOR_SENSOR; 
  
  data.status->sensor=unknown;
  
  // prepare sensors to measure
  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == 0){
      data.logger->notice(F("prepare sd %d"),i);
      if (sd[i]->prepare(waittime) == SD_SUCCESS){
	maxwaittime=_max(maxwaittime,waittime);
      }else{
	data.logger->error(F("%s: prepare failed !"),sensors[i].driver);
	data.status->sensor=error;
      }
    }
  }

  //wait sensors to go ready
  data.logger->notice(F("wait sensors for ms: %d"),maxwaittime);
  uint32_t now=millis();
  int32_t delayt;


  if (oledpresent) {
    displaypos=1;
    u8g2.clearBuffer();
    u8g2.setCursor(0, 1*CH); 
    u8g2.print(F("Measure!"));
    u8g2.sendBuffer();
  }
  delayt= maxwaittime -(millis()-now);
  if(delayt > 0) {
    data.logger->notice(F("delay"));
    delay(delayt);
  }

  if (oledpresent) {
    displaypos=2;
    u8g2.clearBuffer();
    u8g2.sendBuffer();
  }

  data.status->novalue=unknown;

  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == 0){
      data.logger->notice(F("getJson sd %d"),i);
      if (sd[i]->getJson(values,lenvalues) == SD_SUCCESS){
	//strcpy(values,"{\"B12101\":27315,\"B13003\":88}");
	enqueueMqttMessage(values,sensors[i].timerange,sensors[i].level, data );
        web_values(values);
	if (oledpresent) {
	  display_values(values,data);
        }

      }else{
	data.logger->error(F("Error getting json from sensor"));
	if (oledpresent) {
	  u8g2.setCursor(0, (displaypos++)*CH); 
	  u8g2.print(F("Sensor error"));
	  u8g2.sendBuffer();
	}
	data.status->sensor=error;
      }
    }
  }

  if(data.status->novalue==unknown) data.status->novalue=ok;
  if(data.status->sensor==unknown) data.status->sensor=ok;

  if (oledpresent) u8g2.sendBuffer();
}


measureThread::measureThread(measure_data_t &measure_data)
  : Thread{"measure", 50000, 1},
    data{measure_data}
{
  //data.logger->notice("Create Thread %s %d", GetName().c_str(), data.id);
  data.status->novalue=unknown;
  data.status->sensor=unknown;
  //Start();
};

measureThread::~measureThread()
{
  data.logger->notice("Delete Thread %s %d", GetName().c_str(), data.id);
  // todo disconnect and others
  data.status->novalue=unknown;
  data.status->sensor=unknown;
}

void measureThread::Cleanup()
{
  delete this;
}

void measureThread::Run() {
  data.logger->notice("Starting Thread %s %d", GetName().c_str(), data.id);
  for(;;){
    WaitForNotification();
    doMeasure(data);
  }
};
  
