#include <Countdown.h>
#include <WifiIPStack.h>
#include <MQTTClient.h>


#ifndef PUBLISH_THREAD_H_
#define PUBLISH_THREAD_H_


struct publish_data_t {
  int id;
  frtosLogging* logger;
  Queue* mqttqueue;
  Queue* dbqueue;
  publishStatus_t* status;
  station_t* station;
  WiFiClient* mqttClient;
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
  IPStack ipstack;
  MQTT::Client<IPStack, Countdown, MQTT_PACKET_SIZE, 1 > mqttclient;
};

#endif
