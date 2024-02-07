#include "SensorManager.h"

sensorManage::sensorManage(){}

sensorManage::~sensorManage(){}

void sensorManage::setTest (bool test) {
  if (!sensorManage::getEventRead()) is_test=test;
}

bool sensorManage::getTest () {
  return is_test;
}

void sensorManage::setEventRead(){
  if (!is_reading) sensor_reading_state = SENSOR_READING_INIT;
  is_reading=true;
}

bool sensorManage::getEventRead(){
  return is_reading;
}

void sensorManage::setDataReady(bool ready){
  is_data_ready = ready;
}

bool sensorManage::getDataReady(){
  return is_data_ready;
}

void sensorManage::begin (SensorDriver* sds) {
  sensor=sds; 
  is_reading = false;
  is_test = false;
  is_data_ready = false;
  sensor_reading_state = SENSOR_READING_NONE; 
}

void sensorManage::run () {

  switch (sensor_reading_state) {

  case SENSOR_READING_NONE:
    break;
      
  case SENSOR_READING_INIT:
    LOGN(F("Sensor reading..."));
    retry = 0;
    is_data_ready = false;
    sensor->resetPrepared(is_test);

    sensor_reading_state = SENSOR_READING_SETUP_CHECK;
    LOGV(F("SENSOR_READING_INIT ---> SENSOR_READING_SETUP_CHECK"));

    break;

   case SENSOR_READING_SETUP_CHECK:

        LOGN(F("Sensor error count: %d"),sensor->getErrorCount());
     
	if (sensor->getErrorCount() > SENSOR_ERROR_COUNT_MAX){
	  LOGE(F("Sensor i2c error > SENSOR_ERROR_COUNT_MAX"));
	  sensor->resetSetted();
	}
	
	if (!sensor->isSetted()) {
	  LOGE(F("ESEGUO SETUP !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
	  sensor->setup();
	  LOGN(F("Sensor error count: %d"),sensor->getErrorCount());
	}

        if (sensor->isSetted()) {
	  sensor_reading_state = SENSOR_READING_PREPARE;
	}else{
	  LOGE(F("Skip failed Sensor"));
	  sensor_reading_state = SENSOR_READING_END;
	} 

	break;
    
  case SENSOR_READING_PREPARE:
    sensor->prepare(is_test);
    delay_ms = sensor->getDelay();
    start_time_ms = sensor->getStartTime();
    
    if (delay_ms) {
      state_after_wait = SENSOR_READING_IS_PREPARED;
      sensor_reading_state = SENSOR_READING_WAIT_STATE;
      LOGV(F("SENSOR_READING_PREPARE ---> SENSOR_READING_WAIT_STATE"));
    }
    else {
      sensor_reading_state = SENSOR_READING_IS_PREPARED;
      LOGV(F("SENSOR_READING_PREPARE ---> SENSOR_READING_IS_PREPARED"));
    }
    break;

  case SENSOR_READING_IS_PREPARED:
    // success
    if (sensor->isPrepared()) {
      retry = 0;
      sensor_reading_state = SENSOR_READING_GET;
      LOGV(F("SENSOR_READING_IS_PREPARED ---> SENSOR_READING_GET"));

    }
    // retry
    else if ((++retry) < SENSOR_RETRY_COUNT_MAX) {
      LOGE(F("prepare %s %l [ ERROR ]"), sensor->getType(), retry);
      delay_ms = SENSOR_RETRY_DELAY_MS;
      start_time_ms = millis();
      state_after_wait = SENSOR_READING_PREPARE;
      sensor_reading_state = SENSOR_READING_WAIT_STATE;
      LOGV(F("SENSOR_READING_IS_PREPARED ---> SENSOR_READING_WAIT_STATE"));
    }
    // fail
    else {
      LOGE("is prepared ERROR");	
      sensor_reading_state = SENSOR_READING_END;
      LOGV(F("SENSOR_READING_IS_PREPARED ---> SENSOR_READING_END"));
      retry = 0;
    }
    break;
    
  case SENSOR_READING_GET:

    #if (USE_JSON)
    sensor->getJson(values, VALUES_TO_READ_FROM_SENSOR_COUNT, json_values,JSON_BUFFER_LENGTH,is_test);
    #else
    sensor->get(values, VALUES_TO_READ_FROM_SENSOR_COUNT,is_test);
    #endif
    
    delay_ms = sensor->getDelay();
    LOGT(F("Devo aspettare: %l"),delay_ms);
    start_time_ms = sensor->getStartTime();

    if (delay_ms) {
      state_after_wait = SENSOR_READING_IS_GETTED;
      sensor_reading_state = SENSOR_READING_WAIT_STATE;
      LOGV(F("SENSOR_READING_GET ---> SENSOR_READING_WAIT_STATE"));
    }
    else {
      sensor_reading_state = SENSOR_READING_IS_GETTED;
      LOGV(F("SENSOR_READING_GET ---> SENSOR_READING_IS_GETTED"));
    }
    break;

  case SENSOR_READING_IS_GETTED:
    // end
    if (sensor->isEnd() && !sensor->isReaded()) {
      // success
      if (sensor->isSuccess()) {
	retry = 0;
	sensor_reading_state = SENSOR_READING_READY;
	LOGV(F("SENSOR_READING_IS_GETTED ---> SENSOR_READING_READY"));
      }
      // retry
      else if ((++retry) < SENSOR_RETRY_COUNT_MAX) {
	LOGE("is getted retry");	
	delay_ms = SENSOR_RETRY_DELAY_MS;
	start_time_ms = millis();
	state_after_wait = SENSOR_READING_GET;
	sensor_reading_state = SENSOR_READING_WAIT_STATE;
	LOGV(F("SENSOR_READING_IS_GETTED ---> SENSOR_READING_WAIT_STATE"));
      }
      // fail
      else {
	retry = 0;
	LOGE("is getted ERROR");	
	sensor_reading_state = SENSOR_READING_END;
	LOGV(F("SENSOR_READING_IS_GETTED ---> SENSOR_READING_END"));
      }
    }
    // not end
    else {
      LOGT("I2C is working");
      sensor_reading_state = SENSOR_READING_GET;
      LOGV(F("SENSOR_READING_IS_GETTED ---> SENSOR_READING_GET"));
    }
    break;

  case SENSOR_READING_READY:

    is_data_ready = true;
    sensor_reading_state = SENSOR_READING_END;
    LOGV(F("SENSOR_READING_READY ---> SENSOR_READING_END"));
    
    break;
    
  case SENSOR_READING_END:
    is_reading = false;    
    sensor_reading_state = SENSOR_READING_NONE;
    LOGV(F("SENSOR_READING_END ---> SENSOR_READING_NONE"));
    break;
    
  case SENSOR_READING_WAIT_STATE:
    LOGV("Wait sensor");
    delay(10);
    if (millis() - start_time_ms > delay_ms) {
      sensor_reading_state = state_after_wait;
    }
    break;
  }
}

