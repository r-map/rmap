#include "common.h"
#include "measure_thread.h"

unsigned short int displaypos;

// reset data used as summary
void measureThread::reset_summary_data_in_progress() {
  summarydata_in_progress.temperature=NAN;
  summarydata_in_progress.humidity=-999;
  summarydata_in_progress.pm2=-999;
  summarydata_in_progress.pm10=-999;
  summarydata_in_progress.co2=-999;
}

// update data used as summary from working buffer
void measureThread::update_summary_data() {
  data->summarydata->temperature=summarydata_in_progress.temperature;
  data->summarydata->humidity=   summarydata_in_progress.humidity;
  data->summarydata->pm2=        summarydata_in_progress.pm2;
  data->summarydata->pm10=       summarydata_in_progress.pm10;
  data->summarydata->co2=        summarydata_in_progress.co2;
}

// set summary data from measure
void measureThread::get_summary_data_in_progress(uint8_t i) {
  
  StaticJsonDocument<500> doc;
  DeserializationError error = deserializeJson(doc,(const char*)sensorm[i].json_values);
  if (error) {
    data->logger->error(F("measure get summary data deserialization ERROR: %s"),sensorm[i].json_values);
  }else{
    JsonObject obj = doc.as<JsonObject>();
    for (JsonPair pair : obj) {
      if (pair.value().isNull()) {
	//if (strcmp(pair.key().c_str(),"B12101")==0 && (strcmp(sensorm[i].getSensorDriver()->getType(),"SHT")==0)){
	if (strcmp(pair.key().c_str(),"B12101")==0){
	  summarydata_in_progress.temperature=NAN;
	}
	//if (strcmp(pair.key().c_str(),"B13003")==0 && (strcmp(sensorm[i].getSensorDriver()->getType(),"SHT")==0)){
	if (strcmp(pair.key().c_str(),"B13003")==0){
	  summarydata_in_progress.humidity=-999;
	}
	if (strcmp(pair.key().c_str(),"B15198")==0){
	  summarydata_in_progress.pm2=-999;
	}
	if (strcmp(pair.key().c_str(),"B15195")==0){
	  summarydata_in_progress.pm10=-999;
	}
	if (strcmp(pair.key().c_str(),"B15242")==0){
	  summarydata_in_progress.co2=-999;
	}

      } else {
      
	float val=pair.value().as<float>();
	
	//if (strcmp(pair.key().c_str(),"B12101")==0 && (strcmp(sensorm[i].getSensorDriver()->getType(),"SHT")==0)){
	if (strcmp(pair.key().c_str(),"B12101")==0){
	  summarydata_in_progress.temperature=round((val-27315)/10.)/10;
	}
	//if (strcmp(pair.key().c_str(),"B13003")==0 && (strcmp(sensorm[i].getSensorDriver()->getType(),"SHT")==0)){
	if (strcmp(pair.key().c_str(),"B13003")==0){
	  summarydata_in_progress.humidity=round(val);
	}
	if (strcmp(pair.key().c_str(),"B15198")==0){
	  summarydata_in_progress.pm2=round(val/10.);
	}
	if (strcmp(pair.key().c_str(),"B15195")==0){
	  summarydata_in_progress.pm10=round(val/10.);
	}
	if (strcmp(pair.key().c_str(),"B15242")==0){
	  summarydata_in_progress.co2=round(val/1.8);
	}
      }
    }
  }

  data->logger->notice(F("measure get temperature: %D"),summarydata_in_progress.temperature);
  data->logger->notice(F("measure get humidity: %d"),summarydata_in_progress.humidity);
  data->logger->notice(F("measure get PM2: %d"),summarydata_in_progress.pm2);
  data->logger->notice(F("measure get PM10: %d"),summarydata_in_progress.pm10);
  data->logger->notice(F("measure get CO2: %d"),summarydata_in_progress.co2);

}

// encode and enqueue in a proper queue one message
void measureThread::enqueueMqttMessage(uint8_t i ) {
  
  mqttMessage_t mqtt_message;
  StaticJsonDocument<500> doc;

  mqtt_message.sent=0;
  
  data->logger->notice(F("measure have to publish: %s"),sensorm[i].json_values);
  DeserializationError deerror = deserializeJson(doc,(const char*)sensorm[i].json_values);
  if (deerror) {
    data->logger->error(F("measure reading json data: %s"),deerror.c_str());
    return;
  }
  for (JsonPair pair : doc.as<JsonObject>()) {
    if (pair.value().isNull()){
      data->logger->error(F("measure novalue error"));
      data->status->novalue=error;
      continue;
    }
        
    strcpy(mqtt_message.topic,"1/");
    strcat(mqtt_message.topic,data->station->mqttrootpath);
    strcat(mqtt_message.topic,"/");
    strcat(mqtt_message.topic,data->station->user);

    if (strcmp(data->station->ident,"") == 0){
      strcat(mqtt_message.topic,"//");
      strcat(mqtt_message.topic,data->station->longitude);
      strcat(mqtt_message.topic,",");
      strcat(mqtt_message.topic,data->station->latitude);
      strcat(mqtt_message.topic,"/");
      data->status->geodef=ok;
    } else if (abs(now()-data->georef->timestamp) < (data->station->sampletime/2)) {
      data->georef->mutex->Lock();
      strcat(mqtt_message.topic,"/");
      strcat(mqtt_message.topic,data->station->ident);
      strcat(mqtt_message.topic,"/");
      strcat(mqtt_message.topic,data->georef->lon);
      strcat(mqtt_message.topic,",");
      strcat(mqtt_message.topic,data->georef->lat);
      strcat(mqtt_message.topic,"/");
      data->georef->mutex->Unlock();      
      data->status->geodef=ok;
    } else {
      data->logger->error(F("measure georef undefined"));
      data->status->geodef=error;      
      return;
    }
      
    strcat(mqtt_message.topic,data->station->network);
    strcat(mqtt_message.topic,"/");
    strcat(mqtt_message.topic,data->sensors[i].timerange);
    strcat(mqtt_message.topic,"/");
    strcat(mqtt_message.topic,data->sensors[i].level);
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
      data->logger->error(F("measure time not set or needs sync"));
      strcat(mqtt_message.payload,"}");
      return;
    }


    // if there are enough space left on the publish queue send it
    if (data->mqttqueue->NumSpacesLeft() > MQTT_QUEUE_SPACELEFT_MEASURE){
      data->logger->notice(F("measure enqueue for mqtt: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);    
      if(!data->mqttqueue->Enqueue(&mqtt_message,pdMS_TO_TICKS(0))){
	data->logger->error(F("measure enqueue for mqtt: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);
	if (data->dbqueue->Enqueue(&mqtt_message,pdMS_TO_TICKS(0))){      // on error send il to DB
	  data->logger->notice(F("measure enqueue for db"));
	}else{
	  data->logger->error(F("measure lost message for db: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);
	}
      }
    } else {    // if there are no enough space left on the publish queue send it to the archive
      if(data->dbqueue->Enqueue(&mqtt_message,pdMS_TO_TICKS(0))){
	data->logger->notice(F("measure enqueue for db"));
      }else{
	data->logger->error(F("measure lost message for db: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);
      }
    }
  }
}

// execute all required measure 
void measureThread::doMeasure() {

  //LockGuard guard(data->i2cmutex);

  reset_summary_data_in_progress();
  
  data->status->sensor=unknown;  
  data->status->novalue=unknown;
  data->status->geodef=unknown;
  
  // sensorm (sensor Manager) is a finite state machine
  // here we can execute measure in parallel starting one state machine (sensorm) for each sensor
  // each sensor can do one or more measure

  // start to initialize sensorm
  data->logger->notice(F("measure --> sensors_count: %d"),data->sensors_count);
  for (uint8_t i = 0; i < data->sensors_count; i++) {
    sensorm[i].setTest(false);  // this is real measure, no test
    sensorm[i].setEventRead();  // start machine for read with this event
  }

  // exit with break from here
  while (true){
    // run measurements
    for (uint8_t i = 0; i < data->sensors_count; i++) {
      //data->logger->notice(F("measure --> run sensor: %d"),i);
      data->i2cmutex->Lock();
      sensorm[i].run();
      data->i2cmutex->Unlock();
      
      // this machine is ready for data get
      if (sensorm[i].getDataReady()){
	data->logger->notice(F("measure JSON %s %s %d -> %s"),
			    sensorm[i].getSensorDriver()->getDriver(),
			    sensorm[i].getSensorDriver()->getType(),
			    sensorm[i].getSensorDriver()->getAddress(),
			    sensorm[i].json_values);

	// send to publish queue
	//enqueueMqttMessage(sensorm[i].json_values,sensorm[i].json_values,data->sensors[i].timerange,data->sensors[i].level, data );
	enqueueMqttMessage(i);
	get_summary_data_in_progress(i);
	sensorm[i].setDataReady(false);      
      }    
    }

    // set reading status when all sensor get read
    bool reading = false;
    for (uint8_t i = 0; i < data->sensors_count; i++) {
      reading |= sensorm[i].getEventRead();
    }

    if (!reading){
      // end of work
      for (uint8_t i = 0; i < data->sensors_count; i++) {
	//check for error
	if(sensorm[i].getErrorStatus()){
	  data->logger->error(F("measure sensor ERROR: %s-%s:"), sensorm[i].getSensorDriver()->getDriver(),sensorm[i].getSensorDriver()->getType());	
	  data->status->sensor=error;
	}
	// prepare sensor manager machine for a new measure
	sensorm[i].newMeasure();
      }
      // all is done
      break;
    }
  }

  data->i2cmutex->Lock();                           // use the same mutex for i2c for access summary data
  update_summary_data();
  data->i2cmutex->Unlock();

  if(data->status->novalue==unknown) data->status->novalue=ok;
  if(data->status->sensor==unknown) data->status->sensor=ok;

}


measureThread::measureThread(measure_data_t* measure_data)
  : Thread{"measure", TASK_MEASURE_STACK_SIZE, TASK_MEASURE_PRIORITY},
    data{measure_data}
{
  //data->logger->notice("Create Thread %s %d", GetName().c_str(), data->id);
  data->status->novalue=unknown;
  data->status->sensor=unknown;
  data->status->geodef=unknown;
  data->status->memory_collision=unknown;
  data->status->no_heap_memory=unknown;
    
  //Start();
  
};

measureThread::~measureThread()
{
}


void measureThread::Begin()
{
  // create one driver for each sensor
  uint8_t tmp_count=0;
  for (uint8_t i = 0; i < data->sensors_count; i++) {
    data->logger->notice(F("measure create --> %d: %s-%s [ 0x%x ]"), i,data->sensors[i].driver,data->sensors[i].type, data->sensors[i].address);
    SensorDriver::createAndSetup(data->sensors[i].driver,data->sensors[i].type, data->sensors[i].address, 1, sd, tmp_count);
    if (sd[i]){
      data->logger->notice(F("measure created --> %d: %s-%s [ 0x%x ]: [ %s ]"), i,
			   sd[i]->getDriver(),
			   sd[i]->getType(),
			   sd[i]->getAddress(),
			   sd[i]->isSetted() ? OK_STRING : FAIL_STRING);
    }else{
      data->logger->fatal(F("measure sensor driver not created"));
    }
    // begin sensor manager state machines with one driver each
    sensorm[i].begin(sd[i]);
  }
}

void measureThread::Cleanup()
{
  data->logger->notice("Delete Thread %s %d", GetName().c_str(), data->id);
  // todo disconnect and others
  data->status->novalue=unknown;
  data->status->sensor=unknown;
  data->status->geodef=unknown;
  data->status->memory_collision=unknown;
  data->status->no_heap_memory=unknown;
  delete this;
}

void measureThread::Run() {
  data->logger->notice("Starting Thread %s %d", GetName().c_str(), data->id);
  for(;;){
    // wait for notification from the main task; start when we have to do measurements
    WaitForNotification();
    if (timeStatus() == timeSet) doMeasure();  // measure il we can use a timestamp

    // check heap and stack
    //data->logger->notice(F("HEAP: %l"),esp_get_minimum_free_heap_size());
    if( esp_get_minimum_free_heap_size() < HEAP_MIN_WARNING){
      data->logger->error(F("HEAP: %l"),esp_get_minimum_free_heap_size());
      data->status->no_heap_memory=error;
    }
    //data->logger->notice("measure stack: %d",uxTaskGetStackHighWaterMark(NULL));
    if (uxTaskGetStackHighWaterMark(NULL) < STACK_MIN_WARNING ){
      data->logger->error("measure stack"); //check memory collision
      data->status->memory_collision=error;
    }
  }
};
  
