#include <i2c_config.h>
#include <debug_config.h>
#include <sensors_config.h>
#include <SensorDriver.h>
#if (USE_JSON)
#include <json_utility.h>
#include <json_config.h>
#endif
#include <ArduinoLog.h>

#define I2C_BUS_CLOCK                                 (50000L)
#define SENSORS_RETRY_COUNT_MAX                       (3)
#define SENSORS_RETRY_DELAY_MS                        (50)


#define DELAY_ACQ_MS          (900000)
#define DELAY_TEST_MS         (10000)

typedef enum {
  SENSORS_READING_INIT,
  SENSORS_READING_PREPARE,
  SENSORS_READING_IS_PREPARED,
  SENSORS_READING_GET,
  SENSORS_READING_IS_GETTED,
  SENSORS_READING_READ,
  SENSORS_READING_NEXT,
  SENSORS_READING_END,
  SENSORS_READING_WAIT_STATE
} sensors_reading_state_t;

uint8_t sensors_count;
SensorDriver *sensors[SENSORS_MAX];
bool is_first_run;
bool is_test;
bool is_event_sensors_reading;
bool do_reset_first_run;

int32_t values_readed_from_sensor[SENSORS_MAX][VALUES_TO_READ_FROM_SENSOR_COUNT];
#if (USE_JSON)
char json_sensors_data[SENSORS_MAX][JSON_BUFFER_LENGTH];
#endif

sensors_reading_state_t sensors_reading_state;
uint32_t acquiring_sensors_delay_ms;
uint32_t testing_sensors_delay_ms;

uint8_t i2c_error;

void init_sensors () {
  uint8_t address;
  
  do_reset_first_run = false;
  is_first_run = true;
  is_test = false;
  sensors_count = 0;
  
  LOGN(F("Sensors:"));

  #if (USE_SENSOR_ADT)
  address = 0x48;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_ADT, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_HIH)
  address = 0x27;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_HIH, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_HYT)
  address = 0x28;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_HYT, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_STH)
  address = 0x44;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_STH, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif
  
  #if (USE_SENSOR_ITH)
  address = I2C_TH_DEFAULT_ADDRESS;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_ITH, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_MTH)
  address = I2C_TH_DEFAULT_ADDRESS;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_MTH, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_NTH)
  address = I2C_TH_DEFAULT_ADDRESS;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_NTH, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_XTH)
  address = I2C_TH_DEFAULT_ADDRESS;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_XTH, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_TBR)
  address = 0x21;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_TBR, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  // 0x22 for davis wind
  // 0x25 for windsonic
  #if (USE_SENSOR_DW1)
  address = 0x25;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_DW1, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_DEP)
  address = 0x30;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_DEP, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_DSA)
  #include <registers-radiation.h>
  address = I2C_SOLAR_RADIATION_DEFAULT_ADDRESS;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_DSA, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_DWA)
  #include <registers-wind.h>
  address = I2C_WIND_DEFAULT_ADDRESS;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_DWA, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif  

  #if (USE_SENSOR_DWB)
  #include <registers-wind.h>
  address = I2C_WIND_DEFAULT_ADDRESS;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_DWB, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif  

  #if (USE_SENSOR_DWC)
  #include <registers-wind.h>
  address = I2C_WIND_DEFAULT_ADDRESS;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_DWC, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif  

  #if (USE_SENSOR_DWD)
  #include <registers-wind.h>
  address = I2C_WIND_DEFAULT_ADDRESS;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_DWD, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif  

  #if (USE_SENSOR_DWE)
  #include <registers-wind.h>
  address = I2C_WIND_DEFAULT_ADDRESS;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_DWE, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif  

  #if (USE_SENSOR_DWF)
  #include <registers-wind.h>
  address = I2C_WIND_DEFAULT_ADDRESS;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_DWF, address, 1, sensors, &sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif  

  
}

void sensors_reading_task (bool do_prepare = true, bool do_get = true, char *driver = NULL, char *type = NULL, uint8_t address = 0, uint8_t node = 0, uint8_t *sensor_index = 0, uint32_t *wait_time = NULL) {
  static uint8_t i;
  static uint8_t retry;
  static sensors_reading_state_t state_after_wait;
  static uint32_t delay_ms;
  static uint32_t start_time_ms;
  static bool is_sensor_found;

  switch (sensors_reading_state) {
  case SENSORS_READING_INIT:
    i = 0;
    is_sensor_found = false;
    
    if (driver && type && address && node) {
      while (!is_sensor_found && (i < sensors_count)) {
	is_sensor_found = strcmp(sensors[i]->getDriver(), driver) == 0 && strcmp(sensors[i]->getType(), type) == 0 && sensors[i]->getAddress() == address && sensors[i]->getNode() == node;
	if (!is_sensor_found) {
	  i++;
	}
      }
      
      if (is_sensor_found) {
	*sensor_index = i;
        }
    }
    
    if (do_prepare) {
      LOGN(F("Sensors reading..."));
      retry = 0;

      if (driver && type && address && node && is_sensor_found) {
	sensors[i]->resetPrepared(is_test);
      }
      else {
	for (i=0; i<sensors_count; i++) {
	  sensors[i]->resetPrepared(is_test);
	}
	i = 0;
      }

        sensors_reading_state = SENSORS_READING_PREPARE;
        LOGV(F("SENSORS_READING_INIT ---> SENSORS_READING_PREPARE"));
    }
    else if (do_get) {
      sensors_reading_state = SENSORS_READING_GET;
      LOGV(F("SENSORS_READING_INIT ---> SENSORS_READING_GET"));
    }
    else {
      sensors_reading_state = SENSORS_READING_END;
      LOGV(F("SENSORS_READING_INIT ---> SENSORS_READING_END"));
    }
    break;

  case SENSORS_READING_PREPARE:
    sensors[i]->prepare(is_test);
    delay_ms = sensors[i]->getDelay();
    start_time_ms = sensors[i]->getStartTime();
    
    if (driver && type && address && node) {
      *wait_time = delay_ms;
    }

    if (delay_ms) {
      state_after_wait = SENSORS_READING_IS_PREPARED;
      sensors_reading_state = SENSORS_READING_WAIT_STATE;
      LOGV(F("SENSORS_READING_PREPARE ---> SENSORS_READING_WAIT_STATE"));
    }
    else {
      sensors_reading_state = SENSORS_READING_IS_PREPARED;
      LOGV(F("SENSORS_READING_PREPARE ---> SENSORS_READING_IS_PREPARED"));
    }
    break;

  case SENSORS_READING_IS_PREPARED:
    // success
    if (sensors[i]->isPrepared()) {
      retry = 0;
      
      if (do_get) {
	sensors_reading_state = SENSORS_READING_GET;
	LOGV(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_GET"));
      }
      else {
	sensors_reading_state = SENSORS_READING_END;
	LOGV(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_END"));
      }
    }
    // retry
    else if ((++retry) < SENSORS_RETRY_COUNT_MAX) {
      LOGT(F("prepare %s %l [ ERROR ]"), sensors[i]->getType(), retry);
      i2c_error++;
      LOGT("is prepared i2c_error: %d",i2c_error);	
      delay_ms = SENSORS_RETRY_DELAY_MS;
      start_time_ms = millis();
      state_after_wait = SENSORS_READING_PREPARE;
      sensors_reading_state = SENSORS_READING_WAIT_STATE;
      LOGV(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_WAIT_STATE"));
    }
    // fail
    else {
      LOGE("is prepared ERROR i2c_error: %d",i2c_error);	
      if (do_get) {
	sensors_reading_state = SENSORS_READING_GET;
	LOGV(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_GET"));
      }
      else {
	sensors_reading_state = SENSORS_READING_END;
	LOGV(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_END"));
      }
      retry = 0;
    }
    break;
    
  case SENSORS_READING_GET:
    #if (USE_JSON)
    sensors[i]->getJson(&values_readed_from_sensor[i][0], VALUES_TO_READ_FROM_SENSOR_COUNT, &json_sensors_data[i][0],JSON_BUFFER_LENGTH,is_test);
    #else
    sensors[i]->get(&values_readed_from_sensor[i][0], VALUES_TO_READ_FROM_SENSOR_COUNT,is_test);
    #endif

    delay_ms = sensors[i]->getDelay();
    LOGT(F("Devo aspettare: %l"),delay_ms);
    start_time_ms = sensors[i]->getStartTime();

    if (delay_ms) {
      state_after_wait = SENSORS_READING_IS_GETTED;
      sensors_reading_state = SENSORS_READING_WAIT_STATE;
      LOGV(F("SENSORS_READING_GET ---> SENSORS_READING_WAIT_STATE"));
    }
    else {
      sensors_reading_state = SENSORS_READING_IS_GETTED;
      LOGV(F("SENSORS_READING_GET ---> SENSORS_READING_IS_GETTED"));
    }
    break;

  case SENSORS_READING_IS_GETTED:
    // end
    if (sensors[i]->isEnd() && !sensors[i]->isReaded()) {
      // success
      if (sensors[i]->isSuccess()) {
	retry = 0;
	sensors_reading_state = SENSORS_READING_READ;
	LOGV(F("SENSORS_READING_IS_GETTED ---> SENSORS_READING_READ"));
      }
      // retry
      else if ((++retry) < SENSORS_RETRY_COUNT_MAX) {
	i2c_error++;
	LOGT("is getted i2c_error: %d",i2c_error);	
	delay_ms = SENSORS_RETRY_DELAY_MS;
	start_time_ms = millis();
	state_after_wait = SENSORS_READING_GET;
	sensors_reading_state = SENSORS_READING_WAIT_STATE;
	LOGV(F("SENSORS_READING_IS_GETTED ---> SENSORS_READING_WAIT_STATE"));
      }
      // fail
      else {
	retry = 0;
	LOGE("is getted ERROR i2c_error: %d",i2c_error);	
	sensors_reading_state = SENSORS_READING_READ;
	LOGV(F("SENSORS_READING_IS_GETTED ---> SENSORS_READING_READ"));
      }
    }
    // not end
    else {
      LOGT("I2C is working");
      sensors_reading_state = SENSORS_READING_GET;
      LOGV(F("SENSORS_READING_IS_GETTED ---> SENSORS_READING_GET"));
    }
    break;

  case SENSORS_READING_READ:
    if (driver && type && address && node) {
      sensors_reading_state = SENSORS_READING_END;
      LOGV(F("SENSORS_READING_READ ---> SENSORS_READING_END"));
    }
    else {
      sensors_reading_state = SENSORS_READING_NEXT;
      LOGV(F("SENSORS_READING_READ ---> SENSORS_READING_NEXT"));
    }
    break;
    
  case SENSORS_READING_NEXT:
    // next sensor
    if ((++i) < sensors_count) {
      retry = 0;
      sensors_reading_state = SENSORS_READING_PREPARE;
      LOGV(F("SENSORS_READING_NEXT ---> SENSORS_READING_PREPARE"));
    }
    // success: all sensors readed
    else {
      //if (!is_first_run || is_test) {
      for (i = 0; i < sensors_count; i++) {
	LOGN(F("sensor mode:%s %s-%s:"), is_test ? "Test" : "Report",sensors[i]->getDriver(),sensors[i]->getType());
	
	  for (uint8_t v = 0; v < VALUES_TO_READ_FROM_SENSOR_COUNT; v++) {
	    LOGN(F("value %d,%d: %l"), i,v,values_readed_from_sensor[i][v]);
	  }
	
          #if (USE_JSON)
	  LOGN(F("JSON -> %s"), &json_sensors_data[i][0]);
          #endif
	  
	  LOGN(F("end sensor"));
      }
	//}

      sensors_reading_state = SENSORS_READING_END;
      LOGV(F("SENSORS_READING_NEXT ---> SENSORS_READING_END"));
    }
    break;

  case SENSORS_READING_END:
    if (do_reset_first_run) {
      is_first_run = false;
    }
    
    noInterrupts();
    if (is_event_sensors_reading) {
      is_event_sensors_reading = false;
    }
    interrupts();
    
    sensors_reading_state = SENSORS_READING_INIT;
    LOGV(F("SENSORS_READING_END ---> SENSORS_READING_INIT"));
    break;
    
  case SENSORS_READING_WAIT_STATE:
    LOGV("Wait sensor");
    delay(10);
    if (millis() - start_time_ms > delay_ms) {
      sensors_reading_state = state_after_wait;
    }
    break;
  }
}

void reset_wire() {
  uint8_t i2c_bus_state = I2C_ClearBus(); // clear the I2C bus first before calling Wire.begin()
  
   switch (i2c_bus_state) {
   case 1:
     LOGE(F("SCL clock line held low"));
     break;
    
   case 2:
     LOGE(F("SCL clock line held low by slave clock stretch"));
     break;
    
   case 3:
     LOGE(F("SDA data line held low"));
     break;
   }

   /*
   if (i2c_bus_state) {
     LOGE(F("I2C bus error: Could not clear!!!"));
     //while(1);
    have_to_reboot = true;
   }
   */
    
#ifdef ARDUINO_ARCH_AVR
   Wire.end();
#endif
   init_wire();
}


void init_wire() {
  i2c_error = 0;
  Wire.begin();
  Wire.setClock(I2C_BUS_CLOCK);
  digitalWrite(SDA, HIGH);
  digitalWrite(SCL, HIGH);
}

void check_i2c_bus () {
  if (i2c_error > I2C_MAX_ERROR_COUNT) {
    LOGE(F("Restart I2C BUS"));
    // 
    reset_wire();
    init_wire();
  }
}

void logPrefix(Print* _logOutput) {
  char m[12];
  sprintf(m, "%10lu ", millis());
  _logOutput->print("#");
  _logOutput->print(m);
  _logOutput->print(": ");
}

void logSuffix(Print* _logOutput) {
  _logOutput->print('\n');
  //_logOutput->flush();  // we use this to flush every log message
}

void setup() {
  Serial.begin(115200);
  Log.begin(LOG_LEVEL, &Serial);
  Log.setPrefix(logPrefix);
  Log.setSuffix(logSuffix);
  init_wire();
  init_sensors();
  acquiring_sensors_delay_ms = -DELAY_ACQ_MS;
  testing_sensors_delay_ms = -DELAY_TEST_MS;
  is_event_sensors_reading = false;
  sensors_reading_state = SENSORS_READING_INIT;

  i2c_error = 0;
}

void loop() {

  check_i2c_bus();
  
  if (is_event_sensors_reading) {
    sensors_reading_task();
  }

  if (!is_event_sensors_reading && (millis() - acquiring_sensors_delay_ms >= DELAY_ACQ_MS)) {
    acquiring_sensors_delay_ms = millis();
    is_test = false;
    is_event_sensors_reading = true;
  }

  if (!is_event_sensors_reading && (millis() - testing_sensors_delay_ms >= DELAY_TEST_MS)) {
    testing_sensors_delay_ms = millis();
    is_test = true;
    is_event_sensors_reading = true;
  }
}
