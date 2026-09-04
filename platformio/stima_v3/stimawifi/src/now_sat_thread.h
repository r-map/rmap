#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <frtosLog.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <esp_rom_crc.h>
#include "TimeLib.h"

#ifndef NOW_SAT_THREAD_H_
#define NOW_SAT_THREAD_H_

#define SAT_TRANSACTION_TIMEOUT 1000
#define S_TO_uS_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10         /* Time ESP32 will go to sleep (in seconds) */

struct now_sat_data_t {    // thread communication data
  int id;
  frtosLogging* logger;
  Queue* mqttqueue;
  Queue* dbqueue;
  Queue* recoveryqueue;
};

typedef enum {
  STATE_SAT_NONE,
  STATE_SAT_PAIR_RECEIVED,
    STATE_SAT_PAIR_ACK_SENDED,
    STATE_SAT_PAIR_DONE,
    STATE_SAT_DATA_SENDED,
    STATE_SAT_DATA_ACK_RECEIVED,
    STATE_SAT_DATA_DONE
} state_sat_t;

extern "C" {
  extern uint8_t _rtc_data_start;
  extern uint8_t _rtc_data_end;
}

static bool read_local_config();
static bool write_local_config();
static void OnDataSent(const  uint8_t *des_addr, esp_now_send_status_t status);
static void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len);
static void add_broadcast_peer();

using namespace cpp_freertos;

class nowSatThread : public Thread {
  
public:
  nowSatThread(now_sat_data_t* now_sat_data);
  ~nowSatThread();
  virtual void Cleanup();
  void Begin();

  static now_sat_data_t* global_data;
  
    
protected:  
  virtual void Run();
    
private:
  now_sat_data_t* data;
  void store();
  bool doRelay(mqttMessage_t mqtt_message, const bool recovery=false);
  bool nowPublish(mqttMessage_t mqtt_message);

};

#endif
