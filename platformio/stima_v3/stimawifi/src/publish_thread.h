#include <PubSubClient.h>

#ifndef PUBLISH_THREAD_H_
#define PUBLISH_THREAD_H_

struct publish_data_t {
  int id;
  frtosLogging* logger;
  Queue* mqttqueue;
  publishStatus_t* status;
  station_t* station;
};

bool publish_maint(publish_data_t& data);
bool publish_constantdata(publish_data_t& data);
void doPublish(publish_data_t& data, const mqttMessage_t* mqtt_message);

using namespace cpp_freertos;

class publishThread : public Thread {
  
public:
  publishThread(publish_data_t &publish_data);
  ~publishThread();
  virtual void Cleanup();
  
 protected:  
  virtual void Run();
    
 private:
  publish_data_t data;
};

#endif
