#include <SensorDriverb.h>

#ifndef MEASURE_THREAD_H_
#define MEASURE_THREAD_H_

struct measure_data_t {
  int id;
  frtosLogging logger;
  Queue mqttqueue;
};

using namespace cpp_freertos;

class measureThread : public Thread {
  
 public:
  measureThread(measure_data_t &measure_data);
  ~measureThread();
  virtual void Cleanup();
    
 protected:  
  virtual void Run();
    
 private:
  measure_data_t data;
};

#endif
