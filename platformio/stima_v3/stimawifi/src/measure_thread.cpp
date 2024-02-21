#include "stimawifi.h"
#include "measure_thread.h"

unsigned short int displaypos;

void web_values(const char* values) {
  
  StaticJsonDocument<500> doc;

  DeserializationError error =deserializeJson(doc,values);
  if (!error) {
    JsonObject obj = doc.as<JsonObject>();
    for (JsonPair pair : obj) {

      if (pair.value().isNull()) continue;
      float val=pair.value().as<float>();

      if (strcmp(pair.key().c_str(),"B12101")==0){
	temperature=round((val-27315)/10.)/10;
      }
      if (strcmp(pair.key().c_str(),"B13003")==0){
	humidity=round(val);
      }
      if (strcmp(pair.key().c_str(),"B15198")==0){
	pm2=round(val/10.);
      }
      if (strcmp(pair.key().c_str(),"B15195")==0){
	pm10=round(val/10.);
      }
      if (strcmp(pair.key().c_str(),"B15242")==0){
	co2=round(val/1.8);
      }
    }
  }
}

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
    strcat(mqtt_message.topic,data.station->mqttrootpath);
    strcat(mqtt_message.topic,"/");
    strcat(mqtt_message.topic,data.station->user);
    strcat(mqtt_message.topic,"//");
    strcat(mqtt_message.topic,data.station->longitude);
    strcat(mqtt_message.topic,",");
    strcat(mqtt_message.topic,data.station->latitude);
    strcat(mqtt_message.topic,"/");
    strcat(mqtt_message.topic,data.station->network);
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

void doMeasure(sensorManage sensorm[], measure_data_t &data ) {

  data.status->sensor=unknown;  
  data.status->novalue=unknown;

  data.logger->notice(F("doMeasure --> sensors_count: %d"),data.sensors_count);
  for (uint8_t i = 0; i < data.sensors_count; i++) {
    sensorm[i].setTest(false);
    sensorm[i].setEventRead();
  }
  
  while (true){
    for (uint8_t i = 0; i < data.sensors_count; i++) {
      //data.logger->notice(F("doMeasure --> run sensor: %d"),i);
      sensorm[i].run();
      if (sensorm[i].getDataReady()){
	data.logger->notice(F("JSON %s %s %d -> %s"),
			    sensorm[i].getSensorDriver()->getDriver(),
			    sensorm[i].getSensorDriver()->getType(),
			    sensorm[i].getSensorDriver()->getAddress(),
			    sensorm[i].json_values);
	  
	enqueueMqttMessage(sensorm[i].json_values,data.sensors[i].timerange,data.sensors[i].level, data );
        web_values(sensorm[i].json_values);
	if (oledpresent) {
	  display_values(sensorm[i].json_values,data);
        }
	sensorm[i].setDataReady(false);      
      }    
    }
    
    bool reading = false;
    for (uint8_t i = 0; i < data.sensors_count; i++) {
      reading |= sensorm[i].getEventRead();
    }
    
    if (!reading){
      for (uint8_t i = 0; i < data.sensors_count; i++) {
	if(sensorm[i].getErrorStatus()){
	  data.logger->error(F("sensor ERROR: %s-%s:"), sensorm[i].getSensorDriver()->getDriver(),sensorm[i].getSensorDriver()->getType());	
	  data.status->sensor=error;
	  if (oledpresent) {
	    u8g2.setCursor(0, (displaypos++)*CH); 
	    u8g2.print(F("Sensor error"));
	    u8g2.sendBuffer();
	  }
	}
	sensorm[i].newMeasure();
      }
      break;
    }
  }

  if(data.status->novalue==unknown) data.status->novalue=ok;
  if(data.status->sensor==unknown) data.status->sensor=ok;

  if (oledpresent) u8g2.sendBuffer();
}


measureThread::measureThread(measure_data_t* measure_data)
  : Thread{"measure", 50000, 1},
    data{measure_data}
{
  //data.logger->notice("Create Thread %s %d", GetName().c_str(), data.id);
  data->status->novalue=unknown;
  data->status->sensor=unknown;
    
  //Start();
  
};

measureThread::~measureThread()
{
  data->logger->notice("Delete Thread %s %d", GetName().c_str(), data->id);
  // todo disconnect and others
  data->status->novalue=unknown;
  data->status->sensor=unknown;
}


void measureThread::Begin()
{
  uint8_t tmp_count=0;
  for (uint8_t i = 0; i < data->sensors_count; i++) {
    //data.logger->notice(F("create --> %d: %s-%s [ 0x%x ]"), i,sensors[i].driver,sensors[i].type, sensors[i].address);
    SensorDriver::createAndSetup(data->sensors[i].driver,data->sensors[i].type, data->sensors[i].address, 1, sd, tmp_count);
    if (sd[i]){
      data->logger->notice(F("created --> %d: %s-%s [ 0x%x ]: [ %s ]"), i,
			   sd[i]->getDriver(),
			   sd[i]->getType(),
			   sd[i]->getAddress(),
			   sd[i]->isSetted() ? OK_STRING : FAIL_STRING);
    }else{
      data->logger->error(F("sensor driver not created"));
    }
    sensorm[i].begin(sd[i]);
  }
}

void measureThread::Cleanup()
{
  delete this;
}

void measureThread::Run() {
  data->logger->notice("Starting Thread %s %d", GetName().c_str(), data->id);
  for(;;){
    WaitForNotification();
    doMeasure(sensorm,*data);
  }
};
  
