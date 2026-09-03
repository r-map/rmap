#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <frtosLog.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <esp_rom_crc.h>
#include "TimeLib.h"


#ifndef NOW_THREAD_H_
#define NOW_THREAD_H_

#define RESET_PAIR false
#define TRANSACTION_TIMEOUT 1000


struct now_data_t {    // thread communication data
  int id;
  frtosLogging* logger;
  Queue* mqttqueue;
  Queue* dbqueue;
};

typedef enum {
    STATE_NONE,
    STATE_PAIR_SENDED,
    STATE_PAIR_ACK_RECEIVED,
    STATE_PAIR_DONE,
    STATE_DATA_RECEIVED,
    STATE_DATA_ACK_SENDED,
    STATE_DATA_DONE
} state_t;

struct struct_config {
  bool accoppiato = false;
  uint8_t satelliteMac[6];
  uint8_t channel = 3;
};

//Structure to send pair messages
//Must match the receiver structure
struct message_pair {
  uint16_t type;
  uint16_t seq;
  time_t datetime;
};

struct message_pair_crc {
  message_pair message;
  uint8_t crc;
};

//Structure to send data
//Must match the receiver structure
// 5 + MQTT_ROOT_TOPIC_LENGTH + MQTT_SENSOR_TOPIC_LENGTH + MQTT_MESSAGE_LENGTH] < 250 (max size of esp-now packet)

struct message_data {
  uint16_t type;
  uint16_t seq;
  mqttMessage_t mqttmessage;
};

struct message_data_crc {
  message_data message;
  uint8_t crc;
};

bool enqueueMqttMessage(const mqttMessage_t mqtt_message);

using namespace cpp_freertos;

class nowThread : public Thread {
  
public:
  nowThread(now_data_t* now_data);
  ~nowThread();
  virtual void Cleanup();
  void Begin();

  static now_data_t* global_data;
  
    
protected:  
  virtual void Run();
    
private:
  void add_broadcast_peer();
  now_data_t* data;
};

#endif
