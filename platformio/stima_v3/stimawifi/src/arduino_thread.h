#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <HTTPClient.h>
//#include <ESP32httpUpdate.h>
#include <LittleFS.h>
#include "esp_netif_sntp.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include <TimeAlarms.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <LOLIN_I2C_BUTTON.h>

#ifndef ARDUINO_THREAD_H_
#define ARDUINO_THREAD_H_


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
void display_values(const char* values);
void measureAndPublish();
void reboot();
void logPrefix(Print* _logOutput);
void logSuffix(Print* _logOutput);
void setup();
void loop();

#endif
