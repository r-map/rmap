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


static bool enqueueMqttMessage(const mqttMessage_t mqtt_message);
static bool read_local_config();
static bool write_local_config();
static void OnDataSent(const  uint8_t *des_addr, esp_now_send_status_t status);
static void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len);

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
