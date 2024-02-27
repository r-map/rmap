
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
#include <ESP32httpUpdate.h>
#include <Adafruit_NeoPixel.h>

#include "udp_thread.h"
#include "measure_thread.h"
#include "publish_thread.h"
#include "arduino_thread.h"

#ifndef STIMAWIFI_H_
#define STIMAWIFI_H_

//////////////// REMOVE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
extern String readconfig_rmap();
///////////////////////////////////////////////////////////

extern WebServer webserver;

extern U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2;
extern bool oledpresent;

extern float temperature;
extern int humidity,pm2,pm10,co2;

#endif
