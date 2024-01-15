#include "measure_thread.h"

void enqueueMqttMessage(char* values, const char* timerange, const char* level, Queue* MqttQueue ) {
  
  mqttMessage_t mqtt_message;
  StaticJsonDocument<500> doc;

  strcpy(values,"{\"B12101\":27315,\"B13003\":88}");
  
  frtosLog.notice(F("have to publish: %s"),values);
  DeserializationError error = deserializeJson(doc,values);
  if (error) {
    frtosLog.error(F("reading json data: %s"),error.c_str());
    return;
  }
  for (JsonPair pair : doc.as<JsonObject>()) {
    if (pair.value().isNull()){
      /*
	analogWriteFreq(2);
	analogWrite(LED_PIN,512);
	delay(1000);
	digitalWrite(LED_PIN,HIGH);      
	analogWriteFreq(1);
	delay(1000);
	digitalWrite(LED_PIN,LOW);      
      */
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

    frtosLog.notice(F("Measure: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);
    
    MqttQueue->Enqueue(&mqtt_message);
    
  }
}

void doMeasure( Queue &MqttQueue ) {

  uint32_t waittime,maxwaittime=0;

  char values[MAX_VALUES_FOR_SENSOR*20];
  size_t lenvalues=MAX_VALUES_FOR_SENSOR*20;
  //  long values[MAX_VALUES_FOR_SENSOR];
  //  size_t lenvalues=MAX_VALUES_FOR_SENSOR;
  displaypos=1;
  u8g2.clearBuffer();

  digitalWrite(LED_PIN,LOW);

  time_t tnow;
  time(&tnow);
  setTime(tnow);              // resync from sntp
  
  frtosLog.notice(F("Time: %s"),ctime(&tnow));
  
  // prepare sensors to measure
  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == 0){
      frtosLog.notice(F("prepare sd %d"),i);
      if (sd[i]->prepare(waittime) == SD_SUCCESS){
	maxwaittime=_max(maxwaittime,waittime);
      }else{
	frtosLog.error(F("%s: prepare failed !"),sensors[i].driver);
      }
    }
  }

  yield();
  
  //wait sensors to go ready
  frtosLog.notice(F("wait sensors for ms: %d"),maxwaittime);
  uint32_t now=millis();


  if (oledpresent) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 20); 
    u8g2.print(F("Measure!"));
    u8g2.sendBuffer();
    displaypos=1;
    u8g2.clearBuffer();
  }

  while ((millis()-now) < maxwaittime) {
    //frtosLog.notice(F("delay"));
    mqttclient.loop();;
    webserver.handleClient();
    yield();
  }

  temperature= NAN;
  humidity=-999;
  pm2=-999;
  pm10=-999;
  co2=-999;
  
  for (int i = 0; i < SENSORS_LEN; i++) {
    yield();
    if (!sd[i] == 0){
      frtosLog.notice(F("getJson sd %d"),i);
      if (sd[i]->getJson(values,lenvalues) == SD_SUCCESS){
	
	enqueueMqttMessage(values,sensors[i].timerange,sensors[i].level, &MqttQueue );
	
        web_values(values);
        if (oledpresent) {
          display_values(values);
        }

      }else{
	frtosLog.error(F("Error getting json from sensor"));
	if (oledpresent) {
	  u8g2.setCursor(0, (displaypos++)*CH); 
	  u8g2.print(F("Sensor error"));
	}
      }
    }
  }

  if (oledpresent) u8g2.sendBuffer();
  digitalWrite(LED_PIN,HIGH);

}


measureThread::measureThread(measure_data_t &measure_data)
  : Thread("measure", 20000, 1),
    data(measure_data)
{
  data.logger.notice("Create Thread %s %d", GetName().c_str(), data.id);
  //Start();
};

measureThread::~measureThread()
{
  data.logger.notice("Delete Thread %s %d", GetName().c_str(), data.id);
}

void measureThread::Cleanup()
{
  delete this;
}

void measureThread::Run() {
  data.logger.notice("Starting Thread %s %d", GetName().c_str(), data.id);
  for(;;){
    WaitForNotification();
    doMeasure(data.mqttqueue);
  }
};
  
