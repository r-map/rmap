#include <Countdown.h>
#include <WifiIPStack.h>
#include <MQTTClient.h>


#ifndef PUBLISH_THREAD_H_
#define PUBLISH_THREAD_H_

// thread exchange data struct
struct publish_data_t {
  int id;
  frtosLogging* logger;
  Queue* mqttqueue;
  Queue* dbqueue;
  BinaryQueue* recoveryqueue;
  publishStatus_t* status;
  station_t* station;
  WiFiClient* networkClient;
};

bool publish_maint(MQTT::Client<IPStack, Countdown, MQTT_PACKET_SIZE, 1 >& mqttclient, publish_data_t& data);
bool publish_constantdata(MQTT::Client<IPStack, Countdown, MQTT_PACKET_SIZE, 1 >& mqttclient, publish_data_t& data);

using namespace cpp_freertos;

class publishThread : public Thread {

public:
  publishThread(publish_data_t* publish_data);
  ~publishThread();
  virtual void Cleanup();
  
 protected:  
  virtual void Run();
  bool mqttSubscribeRpc(char* comtopic);
  //static void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

 private:
  bool mqttDisconnect();
  bool mqttConnect(const bool cleanSession=true);
  bool mqttPublish( const mqttMessage_t& mqtt_message, const bool retained);
  bool publish_maint();
  bool publish_constantdata();
  void archive();
  bool doPublish(mqttMessage_t& mqtt_message);
  publish_data_t* data;
  IPStack ipstack;
  MQTT::Client<IPStack, Countdown, MQTT_PACKET_SIZE, 1 > mqttclient;
  uint8_t errorcount;
};

#endif
