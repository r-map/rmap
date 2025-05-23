
#include "Arduino.h"
#include "pins_stima.h"
#include "stimawifi_config.h"
#include "typedef.h"
#include <frtosLog.h>
#include <frtosRtc.h>
#include "thread.hpp"
//#include "critical.hpp"
#include "ticks.hpp"
#include "queue.hpp"
#include <DNSServer.h>
//#include <WebSocketsServer.h>
//#include "EspHtmlTemplateProcessor.h"
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <Wire.h>
#include <U8g2lib.h>
#include "time.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <LittleFS.h>
//#include "esp_netif_sntp.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include <TimeAlarms.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <LOLIN_I2C_BUTTON.h>
#include <HTTPUpdate.h>
#include <Adafruit_NeoPixel.h>
#include <sqlite3.h>

#include "udp_thread.h"
#include "gps_thread.h"
#include "measure_thread.h"
#include "publish_thread.h"
#include "db_thread.h"
#include "critical.hpp"
#include "semaphore.hpp"
#include "SD.h"
#include <SdFat.h>
//#include <ArduinoLog.h>
#include <StreamUtils.h>
#include <esp_task_wdt.h>

