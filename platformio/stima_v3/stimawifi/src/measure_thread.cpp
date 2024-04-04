#include "common.h"
#include "measure_thread.h"

unsigned short int displaypos;

void reset_summary_data(measure_data_t &data) {
  data.summarydata->temperature=NAN;
  data.summarydata->humidity=-999;
  data.summarydata->pm2=-999;
  data.summarydata->pm10=-999;
  data.summarydata->co2=-999;
}

void get_summary_data(sensorManage sensorm,measure_data_t &data) {
 
  StaticJsonDocument<500> doc;
  DeserializationError error = deserializeJson(doc,sensorm.json_values);
  if (!error) {
    JsonObject obj = doc.as<JsonObject>();
    for (JsonPair pair : obj) {
      if (pair.value().isNull()) {
	//if (strcmp(pair.key().c_str(),"B12101")==0 && (strcmp(sensorm.getSensorDriver()->getType(),"SHT")==0)){
	if (strcmp(pair.key().c_str(),"B12101")==0){
	  data.summarydata->temperature=NAN;
	}
	//if (strcmp(pair.key().c_str(),"B13003")==0 && (strcmp(sensorm.getSensorDriver()->getType(),"SHT")==0)){
	if (strcmp(pair.key().c_str(),"B13003")==0){
	  data.summarydata->humidity=-999;
	}
	if (strcmp(pair.key().c_str(),"B15198")==0){
	  data.summarydata->pm2=-999;
	}
	if (strcmp(pair.key().c_str(),"B15195")==0){
	  data.summarydata->pm10=-999;
	}
	if (strcmp(pair.key().c_str(),"B15242")==0){
	  data.summarydata->co2=-999;
	}
	continue;
      }
      
      float val=pair.value().as<float>();
      
      //if (strcmp(pair.key().c_str(),"B12101")==0 && (strcmp(sensorm.getSensorDriver()->getType(),"SHT")==0)){
      if (strcmp(pair.key().c_str(),"B12101")==0){
	data.summarydata->temperature=round((val-27315)/10.)/10;
      }
      //if (strcmp(pair.key().c_str(),"B13003")==0 && (strcmp(sensorm.getSensorDriver()->getType(),"SHT")==0)){
      if (strcmp(pair.key().c_str(),"B13003")==0){
	data.summarydata->humidity=round(val);
      }
      if (strcmp(pair.key().c_str(),"B15198")==0){
	data.summarydata->pm2=round(val/10.);
      }
      if (strcmp(pair.key().c_str(),"B15195")==0){
	data.summarydata->pm10=round(val/10.);
      }
      if (strcmp(pair.key().c_str(),"B15242")==0){
	data.summarydata->co2=round(val/1.8);
      }
    }
  }else{
    data.logger->error(F("display_values deserialization ERROR"));
  }

  data.logger->notice(F("get temperature: %D"),data.summarydata->temperature);
  data.logger->notice(F("get humidity: %d"),data.summarydata->humidity);
  data.logger->notice(F("get PM2: %d"),data.summarydata->pm2);
  data.logger->notice(F("get PM10: %d"),data.summarydata->pm10);
  data.logger->notice(F("get CO2: %d"),data.summarydata->co2);

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

    if (strcmp(data.station->ident,"") == 0){
      strcat(mqtt_message.topic,"//");
      strcat(mqtt_message.topic,data.station->longitude);
      strcat(mqtt_message.topic,",");
      strcat(mqtt_message.topic,data.station->latitude);
      strcat(mqtt_message.topic,"/");
      data.status->geodef=ok;
    } else if (abs(now()-data.georef->timestamp) < (data.station->sampletime/2)) {
      data.georef->mutex->Lock();
      strcat(mqtt_message.topic,"/");
      strcat(mqtt_message.topic,data.station->ident);
      strcat(mqtt_message.topic,"/");
      strcat(mqtt_message.topic,data.georef->lon);
      strcat(mqtt_message.topic,",");
      strcat(mqtt_message.topic,data.georef->lat);
      strcat(mqtt_message.topic,"/");
      data.georef->mutex->Unlock();      
      data.status->geodef=ok;
    } else {
      data.status->geodef=error;      
      return;
    }
      
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
    if (timeStatus() == timeSet){
      char jsontime[30];
      time_t messagetime=now();
      snprintf(jsontime,30,",\"t\":\"%04u-%02u-%02uT%02u:%02u:%02u\"}",
	       year(messagetime), month(messagetime), day(messagetime),
	       hour(messagetime), minute(messagetime), second(messagetime));
      strcat(mqtt_message.payload,jsontime);
    }else{
      data.logger->error(F("time not set or needs sync"));
      strcat(mqtt_message.payload,"}");
    }
    data.logger->notice(F("Enqueue: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);
    
    data.mqttqueue->Enqueue(&mqtt_message,pdMS_TO_TICKS(100));
    data.dbqueue->Enqueue(&mqtt_message,pdMS_TO_TICKS(100));
    
  }
}

void doMeasure(sensorManage sensorm[], measure_data_t &data ) {

  //LockGuard guard(*data.i2cmutex);

  reset_summary_data(data);
  
  data.status->sensor=unknown;  
  data.status->novalue=unknown;
  data.status->geodef=unknown;

  data.logger->notice(F("doMeasure --> sensors_count: %d"),data.sensors_count);
  for (uint8_t i = 0; i < data.sensors_count; i++) {
    sensorm[i].setTest(false);
    sensorm[i].setEventRead();
  }
  
  while (true){
    for (uint8_t i = 0; i < data.sensors_count; i++) {
      //data.logger->notice(F("doMeasure --> run sensor: %d"),i);
      data.i2cmutex->Lock();
      sensorm[i].run();
      data.i2cmutex->Unlock();
     
      if (sensorm[i].getDataReady()){
	data.logger->notice(F("JSON %s %s %d -> %s"),
			    sensorm[i].getSensorDriver()->getDriver(),
			    sensorm[i].getSensorDriver()->getType(),
			    sensorm[i].getSensorDriver()->getAddress(),
			    sensorm[i].json_values);
	  
	enqueueMqttMessage(sensorm[i].json_values,data.sensors[i].timerange,data.sensors[i].level, data );
	data.i2cmutex->Lock();                           // use the same mutex for i2c for access summary data
	get_summary_data(sensorm[i],data);
	data.i2cmutex->Unlock();
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
	}
	sensorm[i].newMeasure();
      }
      break;
    }
  }

  if(data.status->novalue==unknown) data.status->novalue=ok;
  if(data.status->sensor==unknown) data.status->sensor=ok;

}


measureThread::measureThread(measure_data_t* measure_data)
  : Thread{"measure", 5000, 1},
    data{measure_data}
{
  //data.logger->notice("Create Thread %s %d", GetName().c_str(), data.id);
  data->status->novalue=unknown;
  data->status->sensor=unknown;
    
  //Start();
  
};

measureThread::~measureThread()
{
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
  data->logger->notice("Delete Thread %s %d", GetName().c_str(), data->id);
  // todo disconnect and others
  data->status->novalue=unknown;
  data->status->sensor=unknown;
  delete this;
}

void measureThread::Run() {
  data->logger->notice("Starting Thread %s %d", GetName().c_str(), data->id);
  for(;;){
    WaitForNotification();
    doMeasure(sensorm,*data);
    //data->logger->notice("stack measure: %d",uxTaskGetStackHighWaterMark(NULL)); // free 1800
    if (uxTaskGetStackHighWaterMark(NULL) < 100 ) data->logger->error("stack measure");
  }
};
  
