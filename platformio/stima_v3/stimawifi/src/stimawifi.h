
#include "Arduino.h"
#include "stimawifi-config.h"
#include "typedef.h"
#include <frtosLog.h>
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

#include "udp_thread.h"
#include "measure_thread.h"
#include "publish_thread.h"

#ifndef STIMAWIFI_H_
#define STIMAWIFI_H_


void analogWriteFreq(const double frequency);
String Json();
String Data();
String FullPage();
void writeconfig();

// web server response function
void handle_FullPage();
void handle_Data();
void handle_Json();
void handle_NotFound();
//callback notifying us of the need to save config
void saveConfigCallback ();
String  rmap_get_remote_config();
void firmware_upgrade();
String readconfig_rmap();
void writeconfig_rmap(const String payload);
int  rmap_config(const String payload);
void readconfig();
void writeconfig();
void web_values(const char* values);
void measureAndPublish();
void reboot();
void logPrefix(Print* _logOutput);
void logSuffix(Print* _logOutput);
void setup();
void loop();

#endif
