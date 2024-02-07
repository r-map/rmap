#include <SensorManager.h>

#define DELAY_ACQ_MS          (60000)
#define DELAY_TEST_MS         (10000)

sensorManage sensorm[SENSORS_MAX];
uint32_t acquiring_sensor_delay_ms;
uint32_t testing_sensor_delay_ms;  
uint8_t sensors_count;
SensorDriver* sensor[SENSORS_MAX];

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
  delay(5000);
  Log.begin(LOG_LEVEL, &Serial);
  Log.setPrefix(logPrefix);
  Log.setSuffix(logSuffix);
  Wire.begin();
  Wire.setClock(I2C_BUS_CLOCK);

  LOGN(F("Sensor:"));
  uint8_t address;
  sensors_count=0;

  #if (USE_SENSOR_SHT)
  address = 0x44;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_SHT, address, 1, sensor, sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensor[sensors_count-1]->getDriver(), sensor[sensors_count-1]->getType(), sensor[sensors_count-1]->getAddress(), sensor[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif
  
  #if (USE_SENSOR_SPS)
  address = SPS30_ADDRESS;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_SPS, address, 1, sensor, sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensor[sensors_count-1]->getDriver(), sensor[sensors_count-1]->getType(), sensor[sensors_count-1]->getAddress(), sensor[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif

  #if (USE_SENSOR_SCD)
  address = 97;
  SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_SCD, address, 1, sensor, sensors_count);
  LOGN(F("--> %d: %s-%s [ 0x%x ]: [ %s ]"), sensors_count,  sensor[sensors_count-1]->getDriver(), sensor[sensors_count-1]->getType(), sensor[sensors_count-1]->getAddress(), sensor[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
  #endif
  
  for (uint8_t i = 0; i < sensors_count; i++) {
    sensorm[i].begin(sensor[i]);
  }
  acquiring_sensor_delay_ms = 0;
  testing_sensor_delay_ms = 0;
}

void loop() {

  for (uint8_t i = 0; i < sensors_count; i++) {
    sensorm[i].run();
    if (sensorm[i].getDataReady()){
      LOGN(F("sensor mode:%s %s-%s:"), sensorm[i].getTest() ? "Test" : "Report",sensor[i]->getDriver(),sensor[i]->getType());	
      for (uint8_t v = 0; v < VALUES_TO_READ_FROM_SENSOR_COUNT; v++) {
	LOGN(F("value %d: %l"), v,sensorm[i].values[v]);
      }    
      #if (USE_JSON)
      LOGN(F("JSON -> %s"), sensorm[i].json_values);
      #endif
      LOGN(F("end sensor"));
      sensorm[i].setDataReady(false);      
    }    
  }

  if ((millis() - testing_sensor_delay_ms) >= DELAY_TEST_MS) {
    testing_sensor_delay_ms = millis();
    for (uint8_t i = 0; i < sensors_count; i++) {
      sensorm[i].setTest(true);
      sensorm[i].setEventRead();
    }
  }

  if ((millis() - acquiring_sensor_delay_ms) >= DELAY_ACQ_MS) {
    acquiring_sensor_delay_ms = millis();
    for (uint8_t i = 0; i < sensors_count; i++) {
      sensorm[i].setTest(false);
      sensorm[i].setEventRead();
    }
  }
}
