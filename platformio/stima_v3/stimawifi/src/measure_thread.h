#include "SensorManager.h"

#ifndef MEASURE_THREAD_H_
#define MEASURE_THREAD_H_

struct measure_data_t {
  int id;
  frtosLogging* logger;
  Queue* mqttqueue;
  measureStatus_t* status;
  station_t* station;
  summarydata_t* summarydata;
  MutexStandard* i2cmutex;
  georef_t* georef;
  sensor_t  sensors[SENSORS_LEN];
  uint8_t sensors_count;
};

void enqueueMqttMessage(const char* values, const char* timerange, const char* level, measure_data_t& data );
void doMeasure( measure_data_t& data );
void web_values(const char* values);

using namespace cpp_freertos;

class measureThread : public Thread {
  
public:
  measureThread(measure_data_t* measure_data);
  ~measureThread();
  virtual void Cleanup();
  void Begin();
    
protected:  
  virtual void Run();
    
private:
  measure_data_t* data;
  SensorDriver* sd[SENSORS_LEN];
  sensorManage sensorm[SENSORS_MAX];
  uint8_t sensors_count;
};

#endif
