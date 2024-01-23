#include <PubSubClient.h>

#ifndef PUBLISH_THREAD_H_
#define PUBLISH_THREAD_H_

bool publish_maint();
bool publish_constantdata();
void doPublish(const mqttMessage_t* mqtt_message);

struct publish_data_t {
  int id;
  frtosLogging logger;
  Queue mqttqueue;
};

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
