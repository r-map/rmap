#include <debug_config.h>

#define SERIAL_TRACE_LEVEL SERIAL_TRACE_LEVEL_INFO

#include <i2c_config.h>
#include <SensorDriver.h>
#if (USE_JSON)
#include <json_utility.h>
#include <json_config.h>
#endif
#include <sensors_config.h>

#define I2C_BUS_CLOCK                                 (50000L)
#define USE_SENSORS_COUNT                             (10)
#define SENSORS_RETRY_COUNT_MAX                       (3)
#define SENSORS_RETRY_DELAY_MS                        (50)

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
SensorDriver *sensors[USE_SENSORS_COUNT];
bool is_first_run;
bool is_test;
bool is_event_sensors_reading;
bool do_reset_first_run;

int32_t values_readed_from_sensor[USE_SENSORS_COUNT][VALUES_TO_READ_FROM_SENSOR_COUNT];
#if (USE_JSON)
char json_sensors_data[USE_SENSORS_COUNT][JSON_BUFFER_LENGTH];
char topic_buffer[JSONS_TO_READ_FROM_SENSOR_COUNT][MQTT_SENSOR_TOPIC_LENGTH];
char message_buffer[JSONS_TO_READ_FROM_SENSOR_COUNT][MQTT_MESSAGE_LENGTH];
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
  
  SERIAL_INFO(F("Sensors...\r\n"));

  #if (USE_SENSOR_ADT)
  address = 0x48;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_ADT, address, 1, sensors, &sensors_count);
  SERIAL_INFO(F("--> %u: %s-%s [ 0x%x ]: [ %s ]\r\n"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_HIH)
  address = 0x27;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_HIH, address, 1, sensors, &sensors_count);
  SERIAL_INFO(F("--> %u: %s-%s [ 0x%x ]: [ %s ]\r\n"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_HYT)
  address = 0x28;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_HYT, address, 1, sensors, &sensors_count);
  SERIAL_INFO(F("--> %u: %s-%s [ 0x%x ]: [ %s ]\r\n"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_ITH)
  address = 0x23;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_ITH, address, 1, sensors, &sensors_count);
  SERIAL_INFO(F("--> %u: %s-%s [ 0x%x ]: [ %s ]\r\n"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_MTH)
  address = 0x23;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_MTH, address, 1, sensors, &sensors_count);
  SERIAL_INFO(F("--> %u: %s-%s [ 0x%x ]: [ %s ]\r\n"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_NTH)
  address = 0x23;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_NTH, address, 1, sensors, &sensors_count);
  SERIAL_INFO(F("--> %u: %s-%s [ 0x%x ]: [ %s ]\r\n"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_XTH)
  address = 0x23;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_XTH, address, 1, sensors, &sensors_count);
  SERIAL_INFO(F("--> %u: %s-%s [ 0x%x ]: [ %s ]\r\n"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_TBR)
  address = 0x21;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_TBR, address, 1, sensors, &sensors_count);
  SERIAL_INFO(F("--> %u: %s-%s [ 0x%x ]: [ %s ]\r\n"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  // 0x22 for davis wind
  // 0x25 for windsonic
  #if (USE_SENSOR_DW1)
  address = 0x25;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_DW1, address, 1, sensors, &sensors_count);
  SERIAL_INFO(F("--> %u: %s-%s [ 0x%x ]: [ %s ]\r\n"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_DEP)
  address = 0x30;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_DEP, address, 1, sensors, &sensors_count);
  SERIAL_INFO(F("--> %u: %s-%s [ 0x%x ]: [ %s ]\r\n"), sensors_count,  sensors[sensors_count-1]->getDriver(), sensors[sensors_count-1]->getType(), sensors[sensors_count-1]->getAddress(), sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
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
      SERIAL_INFO(F("Sensors reading...\r\n"));
      retry = 0;
      
      if (driver && type && address && node && is_sensor_found) {
	sensors[i]->resetPrepared();
      }
      else {
	for (i=0; i<sensors_count; i++) {
	  sensors[i]->resetPrepared();
	}
	i = 0;
      }
      
        state_after_wait = SENSORS_READING_INIT;
        sensors_reading_state = SENSORS_READING_PREPARE;
        SERIAL_TRACE(F("SENSORS_READING_INIT ---> SENSORS_READING_PREPARE\r\n"));
    }
    else if (do_get) {
      sensors_reading_state = SENSORS_READING_GET;
      SERIAL_TRACE(F("SENSORS_READING_INIT ---> SENSORS_READING_GET\r\n"));
    }
    else {
      sensors_reading_state = SENSORS_READING_END;
      SERIAL_TRACE(F("SENSORS_READING_INIT ---> SENSORS_READING_END\r\n"));
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
      SERIAL_TRACE(F("SENSORS_READING_PREPARE ---> SENSORS_READING_WAIT_STATE\r\n"));
    }
    else {
      sensors_reading_state = SENSORS_READING_IS_PREPARED;
      SERIAL_TRACE(F("SENSORS_READING_PREPARE ---> SENSORS_READING_IS_PREPARED\r\n"));
    }
    break;

  case SENSORS_READING_IS_PREPARED:
    // success
    if (sensors[i]->isPrepared()) {
      retry = 0;
      
      if (do_get) {
	sensors_reading_state = SENSORS_READING_GET;
	SERIAL_TRACE(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_GET\r\n"));
      }
      else {
	sensors_reading_state = SENSORS_READING_END;
	SERIAL_TRACE(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_END\r\n"));
      }
    }
    // retry
    else if ((++retry) < SENSORS_RETRY_COUNT_MAX) {
      SERIAL_INFO(F("prepare %s %u [ ERROR ]\r\n"), sensors[i]->getType(), retry);
      Serial.println("ERRORE ++ 1");	
      i2c_error++;
      delay_ms = SENSORS_RETRY_DELAY_MS;
      start_time_ms = millis();
      state_after_wait = SENSORS_READING_PREPARE;
      sensors_reading_state = SENSORS_READING_WAIT_STATE;
      SERIAL_TRACE(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_WAIT_STATE\r\n"));
    }
    // fail
    else {
      if (do_get) {
	sensors_reading_state = SENSORS_READING_GET;
	SERIAL_TRACE(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_GET\r\n"));
      }
      else {
	sensors_reading_state = SENSORS_READING_END;
	SERIAL_TRACE(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_END\r\n"));
      }
      retry = 0;
    }
    break;
    
  case SENSORS_READING_GET:
    #if (USE_JSON)
    sensors[i]->getJson(&values_readed_from_sensor[i][0], VALUES_TO_READ_FROM_SENSOR_COUNT, &json_sensors_data[i][0]);
    #else
    sensors[i]->get(&values_readed_from_sensor[i][0], VALUES_TO_READ_FROM_SENSOR_COUNT);
    #endif

    delay_ms = sensors[i]->getDelay();
    Serial.print("Devo aspettare: ");
    Serial.println(delay_ms);
    start_time_ms = sensors[i]->getStartTime();

    if (delay_ms) {
      state_after_wait = SENSORS_READING_IS_GETTED;
      sensors_reading_state = SENSORS_READING_WAIT_STATE;
      SERIAL_TRACE(F("SENSORS_READING_GET ---> SENSORS_READING_WAIT_STATE\r\n"));
    }
    else {
      sensors_reading_state = SENSORS_READING_IS_GETTED;
      SERIAL_TRACE(F("SENSORS_READING_GET ---> SENSORS_READING_IS_GETTED\r\n"));
    }
    break;

  case SENSORS_READING_IS_GETTED:
    // end
    if (sensors[i]->isEnd() && !sensors[i]->isReaded()) {
      // success
      if (sensors[i]->isSuccess()) {
	retry = 0;
	sensors_reading_state = SENSORS_READING_READ;
	SERIAL_TRACE(F("SENSORS_READING_IS_GETTED ---> SENSORS_READING_READ\r\n"));
      }
      // retry
      else if ((++retry) < SENSORS_RETRY_COUNT_MAX) {
	Serial.println("ERRORE ++ 2");
	i2c_error++;
	delay_ms = SENSORS_RETRY_DELAY_MS;
	start_time_ms = millis();
	state_after_wait = SENSORS_READING_GET;
	sensors_reading_state = SENSORS_READING_WAIT_STATE;
	SERIAL_TRACE(F("SENSORS_READING_IS_GETTED ---> SENSORS_READING_WAIT_STATE\r\n"));
      }
      // fail
      else {
	retry = 0;
	Serial.println("GRANDE MEGA FALLIMENTO");
	sensors_reading_state = SENSORS_READING_READ;
	SERIAL_TRACE(F("SENSORS_READING_IS_GETTED ---> SENSORS_READING_READ\r\n"));
      }
    }
    // not end
    else {
      Serial.println("I2C lavora");
      sensors_reading_state = SENSORS_READING_GET;
      SERIAL_TRACE(F("SENSORS_READING_IS_GETTED ---> SENSORS_READING_GET\r\n"));
    }
    break;

  case SENSORS_READING_READ:
    if (driver && type && address && node) {
      sensors_reading_state = SENSORS_READING_END;
      SERIAL_TRACE(F("SENSORS_READING_READ ---> SENSORS_READING_END\r\n"));
    }
    else {
      sensors_reading_state = SENSORS_READING_NEXT;
      SERIAL_TRACE(F("SENSORS_READING_READ ---> SENSORS_READING_NEXT\r\n"));
    }
    break;
    
  case SENSORS_READING_NEXT:
    // next sensor
    if ((++i) < sensors_count) {
      retry = 0;
      sensors_reading_state = SENSORS_READING_PREPARE;
      SERIAL_TRACE(F("SENSORS_READING_NEXT ---> SENSORS_READING_PREPARE\r\n"));
    }
    // success: all sensors readed
    else {
      if (!is_first_run || is_test) {
	for (i = 0; i < sensors_count; i++) {
	  SERIAL_INFO(F("%s:\t"), sensors[i]->getType());
	  
	  for (uint8_t v = 0; v < VALUES_TO_READ_FROM_SENSOR_COUNT; v++) {
	    SERIAL_INFO_CLEAN(F("%ld\t"), values_readed_from_sensor[i][v]);
	  }
	  
          #if (USE_JSON)
	  SERIAL_INFO_CLEAN(F("%s"), &json_sensors_data[i][0]);
          #endif
	  
	  SERIAL_INFO_CLEAN(F("\r\n\r\n"));
	}
      }

      sensors_reading_state = SENSORS_READING_END;
      SERIAL_TRACE(F("SENSORS_READING_NEXT ---> SENSORS_READING_END\r\n"));
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
    SERIAL_TRACE(F("SENSORS_READING_END ---> SENSORS_READING_INIT\r\n"));
    break;
    
  case SENSORS_READING_WAIT_STATE:
    Serial.println("Wait");
    delay(10);
    if (millis() - start_time_ms > delay_ms) {
      sensors_reading_state = state_after_wait;
    }
    break;
  }
}

bool check_i2c_bus () {
  if (i2c_error > I2C_MAX_ERROR_COUNT) {
    SERIAL_ERROR(F("Restart I2C BUS\r\n"));
    init_wire();
  }
}

#define DELAY_ACQ_MS          (40000)
#define DELAY_TEST_MS         (2000)

void init_wire() {
  i2c_error = 0;
  Wire.end();
  Wire.begin();
  Wire.setClock(I2C_BUS_CLOCK);
}

void setup() {
  SERIAL_BEGIN(115200);
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

  /*
  if (!is_event_sensors_reading && (millis() - testing_sensors_delay_ms >= DELAY_TEST_MS)) {
    testing_sensors_delay_ms = millis();
    is_test = true;
    is_event_sensors_reading = true;
  }
  */
}
