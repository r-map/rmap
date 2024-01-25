#include <SensorDriverb.h>

#ifndef MEASURE_THREAD_H_
#define MEASURE_THREAD_H_

struct measure_data_t {
  int id;
  frtosLogging logger;
  Queue mqttqueue;
  measureStatus_t status;
};

void display_values(const char* values);
void enqueueMqttMessage(const char* values, const char* timerange, const char* level, measure_data_t* data );
void doMeasure( measure_data_t &data );

using namespace cpp_freertos;

class measureThread : public Thread {
  
public:
  measureThread(measure_data_t &measure_data);
  ~measureThread();
  virtual void Cleanup();
    
protected:  
  virtual void Run();
    
private:
  measure_data_t* data;
};

#endif
