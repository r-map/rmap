
#ifndef STIMAWIFI_H_
#define STIMAWIFI_H_

#include "Arduino.h"
#include "stimawifi-config.h"
#include "typedef.h"
#include <frtosLog.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ESP32httpUpdate.h>
#include <LittleFS.h>
#include "thread.hpp"
//#include "critical.hpp"
#include "ticks.hpp"
#include "queue.hpp"
//needed for library
#include <DNSServer.h>
//#include <WebSocketsServer.h>
//#include "EspHtmlTemplateProcessor.h"
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <TimeAlarms.h>
#include <Wire.h>
#include <SensorDriverb.h>
#include <U8g2lib.h>
#include "time.h"
#include <LOLIN_I2C_BUTTON.h>
#include <WiFiUdp.h>
#include "ozgps.h"
#include "esp_sntp.h"
//#include "esp_netif_sntp.h"
#include "udp_thread.h"
#include "measure_thread.h"
#include "publish_thread.h"
#include "arduino_thread.h"

//////////////// REMOVE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
extern String readconfig_rmap();
extern void web_values(const char* values);
///////////////////////////////////////////////////////////

extern sensor_t  sensors[SENSORS_LEN];
extern SensorDriver* sd[SENSORS_LEN];

extern const char* update_url;
extern const uint16_t update_port;

extern WebServer webserver;

extern WiFiClient espClient;
extern PubSubClient mqttclient;
//extern WebSocketsServer webSocket;
//extern EspHtmlTemplateProcessor templateProcessor;
extern WiFiUDP UDP;
extern OZGPS gps;
extern MGPS mgps;
extern MutexStandard loggingmutex;

//flag for saving data
extern bool shouldSaveConfig;
extern bool pmspresent;

//define your default values here, if there are different values in config.json, they are overwritten.
extern char rmap_longitude[11];
extern char rmap_latitude[11];
extern char rmap_network[31];
extern char rmap_server[41];
extern char ntp_server[41];
extern char rmap_mqtt_server[41];
extern int  rmap_sampletime;
extern char rmap_user[10];
extern char rmap_password[31];
extern char rmap_slug[31];
extern char rmap_mqttrootpath[10];
extern char rmap_mqttmaintpath[10];

extern U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2;
extern bool oledpresent;
extern unsigned short int displaypos;

// i2c button for wemos OLED version 2.1.0
extern I2C_BUTTON button; //I2C address 0x31
// extern I2C_BUTTON button; //I2C address 0x31
// extern I2C_BUTTON button; //using customize I2C address

extern float temperature;
extern int humidity,pm2,pm10,co2;


extern udp_data_t udp_data;
extern udpThread threadUdp;
extern Queue mqttQueue;
extern measure_data_t measure_data;
extern measureThread threadMeasure;
extern publish_data_t publish_data;
extern publishThread threadPublish;


#endif
