#include <sensors_config.h>
#include <SensorDriver.h>
#include <ArduinoLog.h>

#define I2C_BUS_CLOCK                           (50000L)
#define SENSOR_RETRY_COUNT_MAX                  (3)
#define SENSOR_RETRY_DELAY_MS                   (50)
#define SENSOR_ERROR_COUNT_MAX                  (10)

#ifndef SENSOR_MANAGE_H
#define SENSOR_MANAGE_H

class sensorManage {

public:
  
  sensorManage();
  ~sensorManage();
  void begin (SensorDriver* sds);
  void newMeasure();
  void run();
  void setTest(bool test);
  SensorDriver* getSensorDriver();
  bool getErrorStatus();
  bool getTest();
  void setEventRead();
  bool getEventRead();
  void setDataReady(bool ready);
  bool getDataReady();

  int32_t values[VALUES_TO_READ_FROM_SENSOR_COUNT];
  char json_values[JSON_BUFFER_LENGTH];
  
private:
  
  typedef enum {
    SENSOR_READING_NONE,
    SENSOR_READING_SETUP_CHECK,
    SENSOR_READING_INIT,
    SENSOR_READING_PREPARE,
    SENSOR_READING_IS_PREPARED,
    SENSOR_READING_GET,
    SENSOR_READING_IS_GETTED,
    SENSOR_READING_READY,
    SENSOR_READING_END,
    SENSOR_READING_WAIT_STATE
  } sensor_reading_state_t;
  
  sensor_reading_state_t sensor_reading_state;
  uint8_t retry;
  sensor_reading_state_t state_after_wait;
  uint32_t delay_ms;
  uint32_t start_time_ms;
  bool is_error;
  bool is_test;
  bool is_reading;
  bool is_data_ready;

  SensorDriver* sensor;
};

#endif
