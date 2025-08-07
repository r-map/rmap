#include <Countdown.h>
#include <WifiIPStack.h>
#include <MQTTClient.h>


#ifndef PUBLISH_THREAD_H_
#define PUBLISH_THREAD_H_

void mqttRxCallback(MQTT::MessageData &md);

int rebootRpc(JsonObject params, JsonObject result);
int recoveryDataRpc(JsonObject params, JsonObject result);
int pinOutRpc(JsonObject params, JsonObject result);
int pinOutRpc(JsonObject params, JsonObject result);



// thread exchange data struct
struct publish_data_t {
  int id;
  frtosLogging* logger;
  Queue* mqttqueue;
  Queue* dbqueue;
  BinaryQueue* recoveryqueue;
  stimawifiStatus_t* status;
  station_t* station;
};

using namespace cpp_freertos;

class publishThread : public Thread {

public:
  publishThread(publish_data_t* publish_data);
  ~publishThread();
  virtual void Cleanup();

  static JsonRPC global_jsonrpc;
  static publish_data_t* global_data;
  static MQTT::Client<IPStack, Countdown, MQTT_PACKET_SIZE, 2 >* global_mqttclient;
  
 protected:  
  virtual void Run();
  bool mqttSubscribeRpc();
  //static void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

 private:
  bool mqttDisconnect();
  bool mqttConnect(const bool cleanSession=true);
  bool mqttPublish( const mqttMessage_t& mqtt_message, const bool retained);
  bool publish_maint();
  bool publish_status_summary();
  void reset_status_summary();
  bool publish_constantdata();
  void archive();
  bool doPublish(mqttMessage_t& mqtt_message);
  publish_data_t* data;
  IPStack ipstack;
  MQTT::Client<IPStack, Countdown, MQTT_PACKET_SIZE, 2 > mqttclient;
  uint8_t errorcount;
  time_t last_status_sended;
  WiFiClient networkClient;
  bool bootConnect;
};

#endif
