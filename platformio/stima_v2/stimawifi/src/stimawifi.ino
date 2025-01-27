/*
Copyright (C) 2023  Paolo Patruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Freeg Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
Standard compilation options:
scheda: lolin (wemos) D1 R2 & mini
flash size: 4M (1M LittleFS)
debug port: disabled
debug level nessuno
lwIP variant: V2 lower memory
Vtables: "flash"
exceptions: disabled
SSL support: Basic SSL"

TODO PORTING TO ESP32 WeMos-D1-Mini-ESP32
https://www.dropbox.com/s/4phxfx75hje5eu4/Board-de-desarrollo-WeMos-D1-Mini-ESP32-WiFiBluetooth-BLE-Pines.jpg
https://cdn.shopify.com/s/files/1/1509/1638/files/D1_Mini_ESP32_-_pinout.pdf

* implementing OTA firmware updater for ESP32 (https): now I use a old simple porting of ESP8266httpUpdate
* change second serial port from software to hardware (but it seems implemented in not wemos connector)
* check if LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED) default to do not autoformat littlefs
* wait for new LittleFS release for esp8266 API compatibility  https://github.com/espressif/arduino-esp32/pull/5396
*/


// increment on change
#define SOFTWARE_VERSION "2025-01-27T00:00"    // date and time
#define MAJOR_VERSION    "20250127"            // date  YYYYMMDD
#define MINOR_VERSION    "0"                   // time  HHMM without leading 0
//
// firmware type for nodemcu is "ESP8266_NODEMCU"
// firmware type for Wemos D1 mini "ESP8266_WEMOS_D1MINI"


#define WIFI_SSED "STIMA-config"
#define WIFI_PASSWORD  "bellastima"
#define DEFAULT_SAMPLETIME 30

#define OLEDI2CADDRESS 0X3C

// logging level at compile time
// Available levels are:
// LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
#define LOG_LEVEL   LOG_LEVEL_NOTICE
// Length of datetime string %04u-%02u-%02uT%02u:%02u:%02u
#define DATE_TIME_STRING_LENGTH                       (25)

#define STIMAHTTP_PORT 80
#define WS_PORT 81

// set the frequency
// 30418,25 Hz  : minimum freq with prescaler set to 1 and CPU clock to 16MHz 
#define I2C_CLOCK 10000
// #define I2CPULLUP define this if you want software pullup on I2C

const int EPOCH_1_1_2019 = 1546300800; //1546300800 =  01/01/2019 @ 12:00am (UTC)

//disable debug at compile time but call function anyway
//#define DISABLE_LOGGING disable

#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#if defined(ARDUINO_ESP8266_NODEMCU) 
// NODEMCU FOR LUFDATEN HOWTO
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <LittleFS.h>
#define FIRMWARE_TYPE "ESP8266_NODEMCU"
#define PMS_RESET D0
#define SDA D5
#define SCL D6
#define RESET_PIN D7
#define LED_PIN D4
// those are defined in SensorDriverb_config.h
//#define SDS_PIN_RX D1
//#define SDS_PIN_TX D2
ESP8266WebServer webserver(STIMAHTTP_PORT);

#elif defined(ARDUINO_ESP8266_WEMOS_D1MINI)
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <LittleFS.h>
//#include <sntp.h> // nonos-sdk

#define FIRMWARE_TYPE "ESP8266_WEMOS_D1MINI"
#define PMS_RESET D0
#define SCL D1
#define SDA D2
#define RESET_PIN D7    // pin to connect to ground for reset wifi configuration
#define LED_PIN D4
// those are defined in SensorDriverb_config.h
//#define SDS_PIN_RX D5
//#define SDS_PIN_TX D6
ESP8266WebServer webserver(STIMAHTTP_PORT);

#elif defined(ARDUINO_ESP8266_WEMOS_D1MINIPRO)
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <LittleFS.h>
#define FIRMWARE_TYPE "ESP8266_WEMOS_D1MINIPRO"
#define PMS_RESET D0
#define SCL D1
#define SDA D2
#define RESET_PIN D7    // pin to connect to ground for reset wifi configuration
#define LED_PIN D4
// those are defined in SensorDriverb_config.h
//#define SDS_PIN_RX D5
//#define SDS_PIN_TX D6
ESP8266WebServer webserver(STIMAHTTP_PORT);

#elif defined(ARDUINO_D1_MINI32)
//#include <analogWrite.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ESP32httpUpdate.h>
//#include <ESP32LittleFS.h>
#include <LittleFS.h>
#define FIRMWARE_TYPE "WEMOS_D1_MINI32"
#define PMS_RESET D0
#define SCL D1
#define SDA D2
#define RESET_PIN D7    // pin to connect to ground for reset wifi configuration
#define LED_PIN 2

WebServer webserver(STIMAHTTP_PORT);

void analogWriteFreq(double frequency){
  analogWriteFrequency(frequency);
}


#else
#error "unknown platform"
#endif

//needed for library
#include <DNSServer.h>
//#include <WebSocketsServer.h>
//#include "EspHtmlTemplateProcessor.h"
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <TimeAlarms.h>
#include <ArduinoLog.h>
#include <Wire.h>
#include <SensorDriverb.h>
#include <U8g2lib.h>
#include "time.h"
#include <LOLIN_I2C_BUTTON.h>

// watchdog is enabled by default on ESP
// https://techtutorialsx.com/2017/01/21/esp8266-watchdog-functions/

  
//const char* update_host = "rmap.cc";
const char* update_url = "/firmware/update/" FIRMWARE_TYPE "/";
const uint16_t update_port = 80;

WiFiClient espClient;
PubSubClient mqttclient(espClient);
//WebSocketsServer webSocket(WS_PORT);
//EspHtmlTemplateProcessor templateProcessor(&server);

//flag for saving data
bool shouldSaveConfig = false;
bool pmspresent =  false;

//define your default values here, if there are different values in config.json, they are overwritten.
char rmap_longitude[11] = "";
char rmap_latitude[11] = "";
char rmap_network[31] = "";
char rmap_server[41] = "rmap.cc";
char ntp_server[41] = "0.europe.pool.ntp.org";
char rmap_mqtt_server[41] = "rmap.cc";
int  rmap_sampletime = DEFAULT_SAMPLETIME;
char rmap_user[10] = "";
char rmap_password[31] = "";
char rmap_slug[31] = "stimawifi";
char rmap_mqttrootpath[10] = "sample";
char rmap_mqttmaintpath[10] = "maint";

// for sensor_t
#define SENSORS_LEN 5
#define SENSORDRIVER_DRIVER_LEN 5
#define SENSORDRIVER_TYPE_LEN 5
#define SENSORDRIVER_META_LEN 30
#define MAX_VALUES_FOR_SENSOR 9

// sensor information
struct sensor_t
{
  char driver[SENSORDRIVER_DRIVER_LEN];     // driver name
  int node;                                 // RF24Nework node id
  char type[SENSORDRIVER_TYPE_LEN];         // sensor type name
  int address;                              // i2c address
  char timerange[SENSORDRIVER_META_LEN];    // timerange for mqtt pubblish
  char level [SENSORDRIVER_META_LEN];       // level for mqtt pubblish

  sensor_t() : address(-1) {
       driver[0]='\0';
       node = -1;
       type[0]='\0';
       timerange[0]='\0';
       level[0]='\0';
  }
} sensors[SENSORS_LEN];;

SensorDriver* sd[SENSORS_LEN];

U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0);
bool oledpresent=false;
unsigned short int displaypos;
#define CH 9            // character height px

// i2c button for wemos OLED version 2.1.0
I2C_BUTTON button; //I2C address 0x31
// I2C_BUTTON button(DEFAULT_I2C_BUTTON_ADDRESS); //I2C address 0x31
// I2C_BUTTON button(your_address); //using customize I2C address


static float temperature=NAN;
static int humidity=-999,pm2=-999,pm10=-999,co2=-999;

/*
const char* reportKeyProcessor(const String& key)
{
  if (key == "TEMPERATURE") return "28.5";
  else if (key == "HUMIDITY") return "55.6";

  return "ERROR: Key not found";
}

void handleReport()
{
  templateProcessor.processAndSend("/report.html", reportKeyProcessor);
}
*/

String Json(){

  String str ="{"
    "\"TEMP\":\"";
  str +=temperature;
  str +="\","
    "\"HUMID\":\"";
  str +=humidity;
  str +="\","
    "\"PM2\":\"";
  str +=pm2;
  str +="\","
    "\"PM10\":\"";
  str +=pm10;
  str +="\","
    "\"CO2\":\"";
  str +=co2;
  str +="\","
    "\"STAT\":\"";
  if (mqttclient.connected()){
    str +="Connected";
  }else{
    str +="Not connected";
  }
  str +="\"}";
  
  return str;
}

// function to prepare HTML response
//https://lastminuteengineers.com/esp8266-dht11-dht22-web-server-tutorial/

String Data(){
  String str ="<h1>StimaWifi Report</h1>\n"
    "<div class=\"data\">\n"
    "<div class=\"side-by-side temperature-icon\">\n"
    "<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n"
    "width=\"9.915px\" height=\"22px\" viewBox=\"0 0 9.915 22\" enable-background=\"new 0 0 9.915 22\" xml:space=\"preserve\">\n"
    "<path fill=\"#FFFFFF\" d=\"M3.498,0.53c0.377-0.331,0.877-0.501,1.374-0.527C5.697-0.04,6.522,0.421,6.924,1.142\n"
    "c0.237,0.399,0.315,0.871,0.311,1.33C7.229,5.856,7.245,9.24,7.227,12.625c1.019,0.539,1.855,1.424,2.301,2.491\n"
    "c0.491,1.163,0.518,2.514,0.062,3.693c-0.414,1.102-1.24,2.038-2.276,2.594c-1.056,0.583-2.331,0.743-3.501,0.463\n"
    "c-1.417-0.323-2.659-1.314-3.3-2.617C0.014,18.26-0.115,17.104,0.1,16.022c0.296-1.443,1.274-2.717,2.58-3.394\n"
    "c0.013-3.44,0-6.881,0.007-10.322C2.674,1.634,2.974,0.955,3.498,0.53z\"></path>\n"
    "</svg>\n"
    "</div>\n"
    "<div class=\"side-by-side temperature-text\">Temperature</div>\n"
    "<div class=\"side-by-side temperature\">";
  str +=temperature;
  str +="<span class=\"superscript\">°C</span></div>\n"
    "</div>\n"
    "<div class=\"data\">\n"
    "<div class=\"side-by-side humidity-icon\">\n"
    "<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n"
    "width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 13 17.955\" xml:space=\"preserve\">\n"
    "<path fill=\"#FFFFFF\" d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n"
    "c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C-0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n"
    "</svg>\n"
    "</div>\n"
    "<div class=\"side-by-side humidity-text\">Humidity</div>\n"
    "<div class=\"side-by-side humidity\">";
  str +=humidity;
  str +="<span class=\"superscript\">%</span></div>\n"
    "</div>\n"
  
    "<div class=\"data\">\n"
    "<div class=\"side-by-side temperature-text\">PM2.5</div>\n"
    "<div class=\"side-by-side temperature\">";
  str +=pm2;
  str +="<span class=\"superscript\">ug/m3</span></div>\n"
    "</div>\n"

    "<div class=\"data\">\n"
    "<div class=\"side-by-side temperature-text\">PM10</div>\n"
    "<div class=\"side-by-side temperature\">";
  str +=pm10;
  str +="<span class=\"superscript\">ug/m3</span></div>\n"
    "</div>\n"
    
    "<div class=\"data\">\n"
    "<div class=\"side-by-side temperature-text\">CO2</div>\n"
    "<div class=\"side-by-side temperature\">";
  str +=co2;
  str +="<span class=\"superscript\">ppm</span></div>\n"
    "</div>\n";

  return str;
}

String FullPage(){
  String ptr = "<!DOCTYPE html> <html>\n"
    "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n"
    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>\n"
    "<title>StimaWiFI Report</title>\n"
    "<style>html { display: block; margin: 0px auto; text-align: center;color: #333333;}\n"
    "body{margin-top: 50px;}\n"
    "h1 {margin: 50px auto 30px;}\n"
    ".side-by-side{display: inline-block;vertical-align: middle;position: relative;}\n"
    ".humidity-icon{background-color: #3498db;width: 30px;height: 30px;border-radius: 50%;line-height: 36px;}\n"
    ".humidity-text{font-weight: 600;padding-left: 15px;font-size: 19px;width: 160px;text-align: left;}\n"
    ".humidity{font-weight: 300;font-size: 60px;color: #3498db;}\n"
    ".temperature-icon{background-color: #f39c12;width: 30px;height: 30px;border-radius: 50%;line-height: 40px;}\n"
    ".temperature-text{font-weight: 600;padding-left: 15px;font-size: 19px;width: 160px;text-align: left;}\n"
    ".temperature{font-weight: 300;font-size: 60px;color: #f39c12;}\n"
    ".superscript{font-size: 17px;font-weight: 600;position: relative;right: -20px;top: -10px;}\n"
    ".data{padding: 10px;}\n"
    "</style>\n"
    "<script>\n"
    "setInterval(loadDoc,5000);\n"
    "function loadDoc() {\n"
    "var xhttp = new XMLHttpRequest();\n"
    "xhttp.onreadystatechange = function() {\n"
    "if (this.readyState == 4 && this.status == 200)\n"
    "{document.getElementById(\"data\").innerHTML =this.responseText}\n"
    "if (this.readyState == 4 && this.status != 200)\n"
    "{document.getElementById(\"data\").innerHTML =\"not connected\"}\n"
    "};\n"
    "xhttp.open(\"GET\", \"/data\", true);\n"
    "xhttp.send();\n"
    "}\n"
    "</script>\n"
    "</head>\n"
    "<body>\n"
    
    "<div id=\"data\">\n";

  ptr +=Data();  
  
  ptr +="</div>\n"
    "</body>\n"
    "</html>\n";
  return ptr;
}

void writeconfig();

// web server response function
void handle_FullPage() {
  webserver.send(200, "text/html", FullPage()); 
}

void handle_Data() {
  webserver.send(200, "text/html", Data()); 
}

void handle_Json() {
  webserver.sendHeader("Access-Control-Allow-Origin", "*", true);
  webserver.sendHeader("Access-Control-Allow-Methods", "*", true);
  webserver.send(200, "application/json", Json()); 
}

void handle_NotFound(){
  webserver.send(404, "text/plain", "Not found");
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  LOGN("Should save config");
  shouldSaveConfig = true;
}

String  rmap_get_remote_config(){
  
  String payload;
  
  HTTPClient http;
  // Make a HTTP request:

  String url="http://";
  url += rmap_server;
  url+="/stations/";
  url+=rmap_user;
  url+="/";
  url+=rmap_slug;
  url+="/default/json/";     // get one station, default boards

  LOGN(F("readRmapRemoteConfig url: %s"),url.c_str());  
  //http.begin("http://rmap.cc/stations/pat1/luftdaten/json/");
  http.begin(espClient,url.c_str());

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) { //Check the returning code
    payload = http.getString();
    LOGN(payload.c_str());
  }else{
    LOGE(F("Error http: %s"),String(httpCode).c_str());
    LOGE(F("Error http: %s"),http.errorToString(httpCode).c_str());
    payload=String();
  }
  http.end();
  return payload;
}


bool publish_maint() {

  const String data;  
  
  //String clientId = "ESP8266Client-";
  //clientId += String(random(0xffff), HEX);
    
  LOGN(F("Connet to mqtt broker"));

  char mqttid[100]="";
  strcat(mqttid,rmap_user);
  strcat(mqttid,"/");
  strcat(mqttid,rmap_slug);
  strcat(mqttid,"/default");
  
  LOGN(F("mqttid: %s"),mqttid);
  
  char mainttopic[100]="1/";
  strcat(mainttopic,rmap_mqttmaintpath);
  strcat(mainttopic,"/");
  strcat(mainttopic,rmap_user);
  strcat(mainttopic,"//");  
  strcat(mainttopic,rmap_longitude);
  strcat(mainttopic,",");
  strcat(mainttopic,rmap_latitude);
  strcat(mainttopic,"/");
  strcat(mainttopic,rmap_network);
  strcat(mainttopic,"/254,0,0/265,0,-,-/B01213");
  LOGN(F("MQTT maint topic: %s"),mainttopic);
    
  if (!mqttclient.connect(mqttid,mqttid,rmap_password,mainttopic,1,1,"{\"v\":\"error01\"}")){
    LOGE(F("Error connecting MQTT"));
    LOGE(F("Error status %d"),mqttclient.state());
    return false;
  }
  LOGN(F("MQTT connected"));
  yield();
  if (!mqttclient.publish(mainttopic,(uint8_t*)"{\"v\":\"conn\",\"s\":" MAJOR_VERSION ",\"m\":" MINOR_VERSION "}   ", 34,1)){ //padded 3 blank char for time
    //if (!mqttclient.publish(mainttopic,(uint8_t*)"{\"v\":\"conn\"}", 12,1)){
    LOGE(F("MQTT maint not published"));
    mqttclient.disconnect();
    return false;
  }
  LOGN(F("MQTT maint published"));
  return true;
}


bool publish_data(const char* values, const char* timerange, const char* level) {
  
  char topic[100];
  StaticJsonDocument<500> doc;

  LOGN(F("have to publish: %s"),values);
  DeserializationError error = deserializeJson(doc,values);
  if (error) {
    LOGE(F("reading json data: %s"),error.c_str());
    return false;
  }
  for (JsonPair pair : doc.as<JsonObject>()) {
    if (pair.value().isNull()){
      /*
      analogWriteFreq(2);
      analogWrite(LED_PIN,512);
      delay(1000);
      digitalWrite(LED_PIN,HIGH);      
      analogWriteFreq(1);
      delay(1000);
      digitalWrite(LED_PIN,LOW);      
      */
      continue;
    }

    char payload[100]="{\"v\":";
    char value[33];
    itoa(pair.value().as<uint32_t>(),value,10);
    strcat(payload,value);
    strcat(payload,"}");
    
    strcpy(topic,"1/");
    strcat(topic,rmap_mqttrootpath);
    strcat(topic,"/");
    strcat(topic,rmap_user);
    strcat(topic,"//");  
    strcat(topic,rmap_longitude);
    strcat(topic,",");
    strcat(topic,rmap_latitude);
    strcat(topic,"/");
    strcat(topic,rmap_network);
    strcat(topic,"/");
    strcat(topic,timerange);
    strcat(topic,"/");
    strcat(topic,level);
    strcat(topic,"/");
    strcat(topic,pair.key().c_str());

    LOGN(F("mqtt publish: %s %s"),topic,payload);
    if (!mqttclient.publish(topic, payload)){
      LOGE(F("MQTT data not published"));
      mqttclient.disconnect();
      return false;
    }
    LOGN(F("MQTT data published"));
  }
  return true;
}

void firmware_upgrade() {

  StaticJsonDocument<200> doc; 
  doc["ver"] = SOFTWARE_VERSION;
  doc["user"] = rmap_user;
  doc["slug"] = rmap_slug;
  char buffer[256];
  serializeJson(doc, buffer, sizeof(buffer));
  LOGN(F("url for firmware update: %s"),update_url);
  LOGN(F("version for firmware update: %s"),buffer);

  analogWriteFreq(4);
  analogWrite(LED_PIN,512);  



  //		t_httpUpdate_return ret = ESPhttpUpdate.update(update_host, update_port, update_url, String(SOFTWARE_VERSION) + String(" ") + esp_chipid + String(" ") + SDS_version + String(" ") + String(current_lang) + String(" ") + String(INTL_LANG));
#if defined(ARDUINO_D1_MINI32)
  t_httpUpdate_return ret = ESPhttpUpdate.update(String(rmap_server), update_port, String(update_url), String(buffer));
#else
  t_httpUpdate_return ret = ESPhttpUpdate.update(espClient,String(rmap_server), update_port, String(update_url), String(buffer));
#endif
  switch(ret)
    {
    case HTTP_UPDATE_FAILED:
      LOGE(F("[update] Update failed with message:"));
      LOGE(F("%s"),ESPhttpUpdate.getLastErrorString().c_str());
      if (oledpresent) {
	u8g2.setCursor(0, 20); 
	u8g2.print(F("FW Update"));
	u8g2.setCursor(0, 30); 
	u8g2.print(F("Failed"));
	u8g2.sendBuffer();
      }
      digitalWrite(LED_PIN,LOW);      
      delay(1000);
      digitalWrite(LED_PIN,HIGH);      
      delay(1000);
      digitalWrite(LED_PIN,LOW);      
      delay(1000);
    break;
    case HTTP_UPDATE_NO_UPDATES:
      LOGN(F("[update] No Update."));
      if (oledpresent) {
	u8g2.setCursor(0, 20); 
	u8g2.print(F("NO Firmware"));
	u8g2.setCursor(0, 30); 
	u8g2.print(F("Update"));
	u8g2.sendBuffer();
      }
      digitalWrite(LED_PIN,LOW);      
      delay(1000);
      break;
    case HTTP_UPDATE_OK:
      LOGN(F("[update] Update ok.")); // may not called we reboot the ESP
      /*
      if (oledpresent) {
	u8g2.setCursor(0, 20); 
	u8g2.print(F("FW Updated!"));
	u8g2.sendBuffer();
      }
      digitalWrite(LED_PIN,LOW);      
      delay(1000);
      digitalWrite(LED_PIN,HIGH);      
      delay(1000);
      digitalWrite(LED_PIN,LOW);      
      delay(1000);
      digitalWrite(LED_PIN,HIGH);      
      delay(1000);
      digitalWrite(LED_PIN,LOW);      
      delay(1000);
      */
      break;
    }

  //#endif

  analogWriteFreq(1);
  digitalWrite(LED_PIN,HIGH);
}


String readconfig_rmap() {

    if (LittleFS.exists("/rmap.json")) {
      //file exists, reading and loading
    LOGN(F("reading rmap config file"));
    File configFile = LittleFS.open("/rmap.json", "r");
    if (configFile) {
      LOGN(F("opened rmap config file"));

      //size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      //std::unique_ptr<char[]> buf(new char[size]);
      //configfile.readBytes(buf.get(), size);

      return configFile.readString();
      
    } else {
      LOGN(F("erro reading rmap file"));	
    }
  } else {
    LOGN(F("rmap file do not exist"));
  }
  //end read
  return String();  
}

void writeconfig_rmap(String payload) {;

  //save the custom parameters to FS
  LOGN(F("saving rmap config"));
  
  File configFile = LittleFS.open("/rmap.json", "w");
  if (!configFile) {
    LOGE(F("failed to open rmap config file for writing"));
  }

  configFile.print(payload);
  configFile.close();
  LOGN(F("saved rmap config parameter"));
  //end save
}

int  rmap_config(String payload){

  bool status_station = false;
  bool status_board_mqtt = false;
  bool status_board_tcpip = false;
  bool status_sensors = false;
  int status = 0;
  int ii = 0;

  if (! (payload == String())) {
    //StaticJsonDocument<2900> jsonBuffer;
    DynamicJsonDocument doc(4000);
    status = 3;
    DeserializationError error = deserializeJson(doc,payload);
    if (!error){
      JsonArrayConst array = doc.as<JsonArray>();
      LOGN(F("array: %d"),array.size());
      //for (uint8_t i = 0; i < array.size(); i++) {
      for(JsonObjectConst element: array){
	
	if  (element["model"] == "stations.stationmetadata"){
	  if (element["fields"]["active"]){
	    LOGN(F("station metadata found!"));
	    strncpy (rmap_mqttrootpath, element["fields"]["mqttrootpath"].as< const char*>(),9);
	    rmap_mqttrootpath[9]='\0';
	    LOGN(F("mqttrootpath: %s"),rmap_mqttrootpath);
	    strncpy (rmap_mqttmaintpath, element["fields"]["mqttmaintpath"].as< const char*>(),9);
	    rmap_mqttmaintpath[9]='\0';
	    LOGN(F("mqttmaintpath: %s"),rmap_mqttmaintpath);

	    //strncpy (rmap_longitude, element["fields"]["lon"].as<const char*>(),10);
	    //rmap_longitude[10]='\0';
	    itoa(int(element["fields"]["lon"].as<float>()*100000),rmap_longitude,10);
	    LOGN(F("lon: %s"),rmap_longitude);

	    //strncpy (rmap_latitude , element["fields"]["lat"].as<const char*>(),10);
	    //rmap_latitude[10]='\0';
	    itoa(int(element["fields"]["lat"].as<float>()*100000),rmap_latitude,10);
	    LOGN(F("lat: %s"),rmap_latitude);
	    
	    strncpy (rmap_network , element["fields"]["network"].as< const char*>(),30);
	    rmap_network[30]='\0';
	    LOGN(F("network: %s"),rmap_network);

	    strncpy (rmap_mqttrootpath , element["fields"]["mqttrootpath"].as< const char*>(),9);
	    rmap_mqttrootpath[9]='\0';
	    LOGN(F("rmap_mqttrootpath: %s"),rmap_mqttrootpath);

	    strncpy (rmap_mqttmaintpath , element["fields"]["mqttmaintpath"].as< const char*>(),9);
	    rmap_mqttmaintpath[9]='\0';
	    LOGN(F("rmap_mqttmaintpath: %s"),rmap_mqttmaintpath);

	    status_station = true;
	  }
	}

	if  (element["model"] == "stations.transportmqtt"){
	  if (element["fields"]["board"][0] == "default"){
	    if (element["fields"]["active"]){
	      LOGN(F("board transportmqtt found!"));
	      rmap_sampletime=element["fields"]["mqttsampletime"];
	      LOGN(F("rmap_sampletime: %d"),rmap_sampletime);

	      if (!element["fields"]["mqttserver"].isNull()){
		strncpy (rmap_mqtt_server, element["fields"]["mqttserver"].as< const char*>(),40);
		rmap_mqtt_server[40]='\0';
		LOGN(F("rmap_mqtt_server: %s"),rmap_mqtt_server);
	      }
	      
	      if (!element["fields"]["mqttuser"].isNull()){
		strncpy (rmap_user, element["fields"]["mqttuser"].as< const char*>(),9);
		rmap_user[9]='\0';
		LOGN(F("rmap_user: %s"),rmap_user);
	      }

	      ///////////////////////////////////////////////////////////////////////////////
	      // use this to migrate from user authentication to user/station_slug/board_slug

	      if (!element["fields"]["mqttpassword"].isNull()){
		strncpy (rmap_password, element["fields"]["mqttpassword"].as< const char*>(),30);
		rmap_password[30]='\0';
		LOGN(F("rmap_password: %s"),rmap_password);
		// save new user and password (station auth)
		writeconfig();
	      }
	      ///////////////////////////////////////////////////////////////////////////////

	      status_board_mqtt = true;
	    }
	  }
	}

	if  (element["model"] == "stations.transporttcpip"){
	  if (element["fields"]["board"][0] == "default"){
	    if (element["fields"]["active"]){
	      LOGN(F("board transporttcpip found!"));

	      if (!element["fields"]["ntpserver"].isNull()){
		strncpy (ntp_server, element["fields"]["ntpserver"].as< const char*>(),40);
		ntp_server[40]='\0';
		LOGN(F("ntp_server: %s"),ntp_server);
	      }
	      status_board_tcpip = true;	      
	    }
	  }
	}

	
	if  (element["model"] == "stations.sensor"){
	  if (element["fields"]["active"]){
	    if (ii < SENSORS_LEN) {
	      LOGN(F("station sensor found!"));
	      strncpy (sensors[ii].driver , element["fields"]["driver"].as< const char*>(),SENSORDRIVER_DRIVER_LEN);
	      LOGN(F("driver: %s"),sensors[ii].driver);
	      strncpy (sensors[ii].type , element["fields"]["type"][0].as< const char*>(),SENSORDRIVER_TYPE_LEN);
	      LOGN(F("type: %s"),sensors[ii].type);
	      strncpy (sensors[ii].timerange, element["fields"]["timerange"].as< const char*>(),SENSORDRIVER_META_LEN);
	      LOGN(F("timerange: %s"),sensors[ii].timerange);
	      strncpy (sensors[ii].level, element["fields"]["level"].as< const char*>(),SENSORDRIVER_META_LEN);
	      LOGN(F("level: %s"),sensors[ii].level);
	      sensors[ii].address = element["fields"]["address"];	    
	      LOGN(F("address: %d"),sensors[ii].address);

	      if (strcmp(sensors[ii].type,"PMS")==0) pmspresent=true;
	      
	      sd[ii]=SensorDriver::create(sensors[ii].driver,sensors[ii].type);
	      if (sd[ii] == 0){
		LOGE(F("%s:%s driver not created !"),sensors[ii].driver,sensors[ii].type);
	      }else{		
		if (!(sd[ii]->setup(sensors[ii].driver, sensors[ii].address, -1, sensors[ii].type) == SD_SUCCESS)) {
		  LOGE(F("sensor not present or broken"));
		  analogWrite(LED_PIN,750);
		  delay(5000);
		}		
	      }
	      ii++;
	    }
	    status_sensors = true;
	  }
	}
      }
      status = (int)!(status_station && status_board_mqtt && status_board_tcpip && status_sensors);
    } else {
      LOGE(F("error parsing array: %s"),error.c_str());
      analogWrite(LED_PIN,973);
      delay(5000);
      status = 2;
    }
    
  }else{
    status=1;
  }
  return status;
}

#if not defined(ARDUINO_D1_MINI32)

void readconfig_SPIFFS() {

  if (SPIFFS.exists("/config.json")) {
    //file exists, reading and loading
    LOGN(F("reading config file"));
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile) {
      LOGN(F("opened config file"));
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        StaticJsonDocument<500> doc;
        DeserializationError error = deserializeJson(doc,buf.get());
	if (!error) {
	  //json.printTo(Serial);
	  if (doc.containsKey("rmap_longitude"))strcpy(rmap_longitude, doc["rmap_longitude"]);
	  if (doc.containsKey("rmap_latitude")) strcpy(rmap_latitude, doc["rmap_latitude"]);
          if (doc.containsKey("rmap_server")) strcpy(rmap_server, doc["rmap_server"]);
          if (doc.containsKey("ntp_server")) strcpy(ntp_server, doc["ntp_server"]);
          if (doc.containsKey("rmap_mqtt_server")) strcpy(rmap_mqtt_server, doc["rmap_mqtt_server"]);
          if (doc.containsKey("rmap_user")) strcpy(rmap_user, doc["rmap_user"]);
          if (doc.containsKey("rmap_password")) strcpy(rmap_password, doc["rmap_password"]);
          if (doc.containsKey("rmap_slug")) strcpy(rmap_slug, doc["rmap_slug"]);
	  if (doc.containsKey("rmap_mqttrootpath")) strcpy(rmap_mqttrootpath, doc["rmap_mqttrootpath"]);
	  if (doc.containsKey("rmap_mqttmaintpath")) strcpy(rmap_mqttmaintpath, doc["rmap_mqttmaintpath"]);
	  
	  LOGN(F("loaded config parameter:"));
	  LOGN(F("longitude: %s"),rmap_longitude);
	  LOGN(F("latitude: %s"),rmap_latitude);
	  LOGN(F("server: %s"),rmap_server);
	  LOGN(F("ntp server: %s"),ntp_server);	  
	  LOGN(F("mqtt server: %s"),rmap_mqtt_server);
	  LOGN(F("user: %s"),rmap_user);
	  //LOGN(F("password: %s"),rmap_password);
	  LOGN(F("slug: %s"),rmap_slug);
	  LOGN(F("mqttrootpath: %s"),rmap_mqttrootpath);
	  LOGN(F("mqttmaintpath: %s"),rmap_mqttmaintpath);
	  
        } else {
          LOGE(F("failed to deserialize json config %s"),error.c_str());
        }
      } else {
	LOGE(F("erro reading config file"));	
      }
    } else {
      LOGW(F("config file do not exist"));
    }
  //end read
}


String readconfig_rmap_SPIFFS() {

  if (SPIFFS.exists("/rmap.json")) {
    //file exists, reading and loading
    LOGN(F("reading rmap config file"));
    File configFile = SPIFFS.open("/rmap.json", "r");
    if (configFile) {
      LOGN(F("opened rmap config file"));

      //size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      //std::unique_ptr<char[]> buf(new char[size]);
      //configfile.readBytes(buf.get(), size);

      return configFile.readString();
      
    } else {
      LOGN(F("erro reading rmap file"));	
    }
  } else {
    LOGN(F("rmap file do not exist"));
  }
  //end read
  return String();  
}

#endif

void readconfig() {

  if (LittleFS.exists("/config.json")) {
    //file exists, reading and loading
    LOGN(F("reading config file"));
    File configFile = LittleFS.open("/config.json", "r");
    if (configFile) {
      LOGN(F("opened config file"));
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        StaticJsonDocument<500> doc;
        DeserializationError error = deserializeJson(doc,buf.get());
	if (!error) {
	  //json.printTo(Serial);

	  if (doc.containsKey("rmap_longitude"))strcpy(rmap_longitude, doc["rmap_longitude"]);
	  if (doc.containsKey("rmap_latitude")) strcpy(rmap_latitude, doc["rmap_latitude"]);
          if (doc.containsKey("rmap_server")) strcpy(rmap_server, doc["rmap_server"]);
          if (doc.containsKey("ntp_server")) strcpy(ntp_server, doc["ntp_server"]);
          if (doc.containsKey("rmap_mqtt_server")) strcpy(rmap_mqtt_server, doc["rmap_mqtt_server"]);
          if (doc.containsKey("rmap_user")) strcpy(rmap_user, doc["rmap_user"]);
          if (doc.containsKey("rmap_password")) strcpy(rmap_password, doc["rmap_password"]);
          if (doc.containsKey("rmap_slug")) strcpy(rmap_slug, doc["rmap_slug"]);
	  if (doc.containsKey("rmap_mqttrootpath")) strcpy(rmap_mqttrootpath, doc["rmap_mqttrootpath"]);
	  if (doc.containsKey("rmap_mqttmaintpath")) strcpy(rmap_mqttmaintpath, doc["rmap_mqttmaintpath"]);
	  
	  LOGN(F("loaded config parameter:"));
	  LOGN(F("longitude: %s"),rmap_longitude);
	  LOGN(F("latitude: %s"),rmap_latitude);
	  LOGN(F("server: %s"),rmap_server);
	  LOGN(F("ntp server: %s"),ntp_server);
	  LOGN(F("mqtt server: %s"),rmap_mqtt_server);
	  LOGN(F("user: %s"),rmap_user);
	  //LOGN(F("password: %s"),rmap_password);
	  LOGN(F("slug: %s"),rmap_slug);
	  LOGN(F("mqttrootpath: %s"),rmap_mqttrootpath);
	  LOGN(F("mqttmaintpath: %s"),rmap_mqttmaintpath);
	  
        } else {
          LOGE(F("failed to deserialize json config %s"),error.c_str());
        }
      } else {
	LOGE(F("erro reading config file"));	
      }
    } else {
      LOGW(F("config file do not exist"));
    }
  //end read
}

void writeconfig() {;

  //save the custom parameters to FS
  LOGN(F("saving config"));
  //DynamicJsonDocument jsonBuffer;
  StaticJsonDocument<500> json;
    
  json["rmap_longitude"] = rmap_longitude;
  json["rmap_latitude"] = rmap_latitude;
  json["rmap_server"] = rmap_server;
  json["ntp_server"] = ntp_server;
  json["rmap_mqtt_server"] = rmap_mqtt_server;
  json["rmap_user"] = rmap_user;
  json["rmap_password"] = rmap_password;
  json["rmap_slug"] = rmap_slug;
  json["rmap_mqttrootpath"] = rmap_mqttrootpath;
  json["rmap_mqttmaintpath"] = rmap_mqttmaintpath;
  
  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    LOGE(F("failed to open config file for writing"));
  }

  //json.printTo(Serial);
  serializeJson(json,configFile);
  configFile.close();
  LOGN(F("saved config parameter"));
}


void web_values(const char* values) {
  
  StaticJsonDocument<500> doc;

  DeserializationError error =deserializeJson(doc,values);
  if (!error) {
    JsonObject obj = doc.as<JsonObject>();
    for (JsonPair pair : obj) {

      if (pair.value().isNull()) continue;
      float val=pair.value().as<float>();

      if (strcmp(pair.key().c_str(),"B12101")==0){
	temperature=round((val-27315)/10.)/10;
      }
      if (strcmp(pair.key().c_str(),"B13003")==0){
	humidity=round(val);
      }
      if (strcmp(pair.key().c_str(),"B15198")==0){
	pm2=round(val/10.);
      }
      if (strcmp(pair.key().c_str(),"B15195")==0){
	pm10=round(val/10.);
      }
      if (strcmp(pair.key().c_str(),"B15242")==0){
	co2=round(val/1.8);
      }
    }
  }
}


void display_values(const char* values) {
  
  StaticJsonDocument<500> doc;

  DeserializationError error = deserializeJson(doc,values);
  if (!error) {
    JsonObject obj = doc.as<JsonObject>();
    for (JsonPair pair : obj) {

      if (pair.value().isNull()) continue;
      float val=pair.value().as<float>();

      u8g2.setCursor(0, (displaypos)*CH); 
      
      if (strcmp(pair.key().c_str(),"B12101")==0){
	u8g2.print(F("T   : "));
	u8g2.print(round((val-27315)/10.)/10,1);
	u8g2.print(F(" C"));
	displaypos++;	
      }
      if (strcmp(pair.key().c_str(),"B13003")==0){
	u8g2.print(F("U   : "));
	u8g2.print(round(val),0);
	u8g2.print(F(" %"));
	displaypos++;	
      }
      if (strcmp(pair.key().c_str(),"B15198")==0){
	u8g2.print(F("PM2 : "));
	u8g2.print(round(val/10.),0);
	u8g2.print(F(" ug/m3"));
	displaypos++;	
      }
      if (strcmp(pair.key().c_str(),"B15195")==0){
	u8g2.print(F("PM10: "));
	u8g2.print(round(val/10.),0);
	u8g2.print(F(" ug/m3"));
	displaypos++;	
      }
      if (strcmp(pair.key().c_str(),"B15242")==0){
	u8g2.print(F("CO2 : "));
	u8g2.print(round(val/1.8),0);
	u8g2.print(F(" ppm"));
	displaypos++;	
      }
    }
  }
}

bool publish_constantdata() {

  char topic[100];
  String payload=readconfig_rmap();

  if (! (payload == String())) {
    //StaticJsonDocument<2900> doc;
    DynamicJsonDocument doc(4000);
    DeserializationError error = deserializeJson(doc,payload);
    if (!error) {
      JsonArrayConst array = doc.as<JsonArray>();
      //for (uint8_t i = 0; i < array.size(); i++) {
      for(JsonObjectConst element: array){ 
	if  (element["model"] == "stations.stationconstantdata"){
	  if (element["fields"]["active"]){
	    LOGN(F("station constant data found!"));
	    char btable[7];
	    strncpy (btable, element["fields"]["btable"].as< const char*>(),6);
	    btable[6]='\0';
	    LOGN(F("btable: %s"),btable);
	    char value[31];
	    strncpy (value, element["fields"]["value"].as< const char*>(),30);
	    value[30]='\0';
	    LOGN(F("value: %s"),value);

	    char payload[100]="{\"v\":\"";
	    strcat(payload,value);
	    strcat(payload,"\"}");
      
	    strcpy(topic,"1/");
	    strcat(topic,rmap_mqttrootpath);
	    strcat(topic,"/");
	    strcat(topic,rmap_user);
	    strcat(topic,"//");  
	    strcat(topic,rmap_longitude);
	    strcat(topic,",");
	    strcat(topic,rmap_latitude);
	    strcat(topic,"/");
	    strcat(topic,rmap_network);
	    strcat(topic,"/-,-,-/-,-,-,-/");
	    strcat(topic,btable);

	    LOGN(F("mqtt publish: %s %s"),topic,payload);
	    if (!mqttclient.publish(topic, payload)){
	      LOGE(F("MQTT data not published"));
	      mqttclient.disconnect();
	      return false;
	    }
	    LOGN(F("MQTT data published"));
	  }
	}
      }
    } else {
      LOGE(F("error parsing array: %s"),error.c_str());
      analogWrite(LED_PIN,973);
      delay(5000);
      return false;
    }
    
  }else{
    return false;
  }
  return true;
}

void repeats() {
  
  uint32_t waittime,maxwaittime=0;

  char values[MAX_VALUES_FOR_SENSOR*20];
  size_t lenvalues=MAX_VALUES_FOR_SENSOR*20;
  //  long values[MAX_VALUES_FOR_SENSOR];
  //  size_t lenvalues=MAX_VALUES_FOR_SENSOR;
  displaypos=1;
  u8g2.clearBuffer();

  digitalWrite(LED_PIN,LOW);

  time_t tnow = now();
  setTime(tnow);              // resync from sntp
  
  LOGN(F("Time: %s"),ctime(&tnow));
  
  // prepare sensors to measure
  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == 0){
      LOGN(F("prepare sd %d"),i);
      if (sd[i]->prepare(waittime) == SD_SUCCESS){
	maxwaittime=_max(maxwaittime,waittime);
      }else{
	LOGE(F("%s: prepare failed !"),sensors[i].driver);
      }
    }
  }

  yield();
  
  //wait sensors to go ready
  LOGN(F("wait sensors for ms: %d"),maxwaittime);
  uint32_t now=millis();

  // manage mqtt reconnect as RMAP standard
  if (!mqttclient.connected()){
    if (!publish_maint()) {
      LOGE(F("Error in publish maint"));
      if (oledpresent) {
	u8g2.clearBuffer();
	u8g2.setCursor(0, 20); 
	u8g2.print(F("MQTT Error maint"));
	u8g2.sendBuffer();
	u8g2.clearBuffer();
	delay(3000);
      }else{
	// if we do not have display terminate (we do not display values)
	analogWrite(LED_PIN,512);
	delay(5000);
	digitalWrite(LED_PIN,HIGH);
	//return;
      }
    }

    if (!publish_constantdata()) {
      LOGE(F("Error in publish constant data"));
      if (oledpresent) {
	u8g2.clearBuffer();
	u8g2.setCursor(0, 20); 
	u8g2.print(F("MQTT Error constant"));
	u8g2.sendBuffer();
	u8g2.clearBuffer();
	delay(3000);
      }else{
	// if we do not have display terminate (we do not display values)
	analogWrite(LED_PIN,512);
	delay(5000);
	digitalWrite(LED_PIN,HIGH);
	//return;
      }
    }    
  }

  if (oledpresent) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 20); 
    u8g2.print(F("Measure!"));
    u8g2.sendBuffer();
    displaypos=1;
    u8g2.clearBuffer();
  }

  while ((millis()-now) < maxwaittime) {
    //LOGN(F("delay"));
    mqttclient.loop();;
    webserver.handleClient();
#if not defined(ARDUINO_D1_MINI32)
    MDNS.update();
#endif
    yield();
  }

  temperature= NAN;
  humidity=-999;
  pm2=-999;
  pm10=-999;
  co2=-999;
  
  for (int i = 0; i < SENSORS_LEN; i++) {
    yield();
    if (!sd[i] == 0){
      LOGN(F("getJson sd %d"),i);
      if (sd[i]->getJson(values,lenvalues) == SD_SUCCESS){
	if(publish_data(values,sensors[i].timerange,sensors[i].level)){
	  web_values(values);
	  if (oledpresent) {
	    display_values(values);
	  }
	}else{
	  LOGE(F("Error in publish data"));
	  if (oledpresent) {
	    u8g2.setCursor(0, (displaypos++)*CH); 
	    u8g2.print(F("MQTT error publish"));
	  }else{
	    analogWrite(LED_PIN,973);
	    delay(5000);
	  }
	}
      }else{
	LOGE(F("Error getting json from sensor"));
	if (oledpresent) {
	  u8g2.setCursor(0, (displaypos++)*CH); 
	  u8g2.print(F("Sensor error"));
	}
      }
    }
  }

  if (oledpresent) u8g2.sendBuffer();
  digitalWrite(LED_PIN,HIGH);
}


void reboot() {
  //reset and try again, or maybe put it to deep sleep
  ESP.restart();
  delay(5000);
}

void logPrefix(Print* _logOutput) {
  char dt[DATE_TIME_STRING_LENGTH];
  snprintf(dt, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u", year(), month(), day(), hour(), minute(), second());
  _logOutput->print("#");
  _logOutput->print(dt);
  _logOutput->print(" ");
}

void logSuffix(Print* _logOutput) {
  _logOutput->print('\n');
  _logOutput->flush();  // we use this to flush every log message
}

void setup() {
  // put your setup code here, to run once:
  
  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  analogWriteFreq(1);
  digitalWrite(LED_PIN,HIGH);

  pinMode(PMS_RESET, OUTPUT);
  //reset pin for sensor
  digitalWrite(PMS_RESET,LOW); // reset low
  delay(500);
  digitalWrite(PMS_RESET,HIGH);

  
  Serial.begin(115200);
  Serial.println();

  // Pass log level, whether to show log level, and print interface.
  // Available levels are:
  // LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
  // Note: if you want to fully remove all logging code, change #define LOG_LEVEL ....
  //       this will significantly reduce your project size

  // set runtime log level to the same of compile time
  Log.begin(LOG_LEVEL, &Serial);
  Log.setPrefix(logPrefix); // Uncomment to get timestamps as prefix
  Log.setSuffix(logSuffix); // Uncomment to get newline as suffix
  LOGN(F("Started"));
  LOGN(F("Version: " SOFTWARE_VERSION));

#ifdef I2CPULLUP
  //if you want to set the internal pullup
  digitalWrite( SDA, HIGH);
  digitalWrite( SCL, HIGH);
#else
  // here we enforce we do not want pullup
  digitalWrite( SDA, LOW);
  digitalWrite( SCL, LOW);
#endif

  espClient.setTimeout(5000); // esp32 issue https://github.com/espressif/arduino-esp32/issues/3732
  
  
  Wire.begin(SDA,SCL);
  Wire.setClock(I2C_CLOCK);

  // check return value of
  // the Write.endTransmisstion to see if
  // a device did acknowledge to the address.
  Wire.beginTransmission(OLEDI2CADDRESS);
  if (Wire.endTransmission() == 0) {
    LOGN(F("OLED Found"));
    oledpresent=true;
    u8g2.setI2CAddress(OLEDI2CADDRESS*2);
    u8g2.begin();
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.setFontMode(0); // enable transparent mode, which is faster
    u8g2.clearBuffer();
    u8g2.setCursor(0, 10); 
    u8g2.print(F("Starting up!"));
    u8g2.setCursor(0, 20); 
    u8g2.print(F("Version:"));
    u8g2.setCursor(0, 30); 
    u8g2.print(F(SOFTWARE_VERSION));
    u8g2.sendBuffer();
  }else{
        LOGN(F("OLED NOT Found"));
  }
  
  digitalWrite(LED_PIN,LOW);
  delay(1000);
  digitalWrite(LED_PIN,HIGH);
  delay(1000);
  digitalWrite(LED_PIN,LOW);
  delay(1000);
  digitalWrite(LED_PIN,HIGH);
  delay(1000);
  digitalWrite(LED_PIN,LOW);
  delay(1000);
  digitalWrite(LED_PIN,HIGH);

  /*
  char esp_chipid[11];
  itoa(ESP.getChipId(),esp_chipid,10);
  LOGN(F("esp_chipid: %s "),esp_chipid );
  */

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;


  // manage reset button in hardware (RESET_PIN) or in software (I2C)
  bool reset=digitalRead(RESET_PIN) == LOW;
  if (button.get() == 0)
  {
    if (button.BUTTON_A)
    {
      //String keyString[] = {"None", "Press", "Long Press", "Double Press", "Hold"};
      //LOGN(F("BUTTON A: %s"),keyString[button.BUTTON_A].c_str());
      if (button.BUTTON_A == KEY_VALUE_HOLD) reset=true;
    }
  }
  
  if (reset) {
    LOGN(F("clean FS"));
    if (oledpresent) {
      u8g2.clearBuffer();
      u8g2.setCursor(0, 10); 
      u8g2.print(F("Clean FS"));
      u8g2.setCursor(0, 20); 
      u8g2.print(F("Reset wifi"));
      u8g2.setCursor(0, 30); 
      u8g2.print(F("configuration"));
      u8g2.sendBuffer();
      delay(3000);
    }
    LittleFS.begin();    
    LittleFS.format();
    LOGN(F("Reset wifi configuration"));
    wifiManager.resetSettings();
  }
  
  //read configuration from FS json
  LOGN(F("mounting FS..."));


#if not defined(ARDUINO_D1_MINI32)

  LittleFSConfig cfg;
  cfg.setAutoFormat(false);
  LittleFS.setConfig(cfg);
  
  SPIFFSConfig spiffscfg;
  spiffscfg.setAutoFormat(false);
  SPIFFS.setConfig(spiffscfg);
  if (SPIFFS.begin()) {
    // migrate configuration from old SPIFFS to new LittleFS
    LOGN(F("mounted old SPIFFS file system"));
    readconfig_SPIFFS();
    String remote_config=readconfig_rmap_SPIFFS();
    LOGW(F("Old configuration read"));
    SPIFFS.end();
    LOGW(F("Reformat LittleFS"));
    LittleFS.begin();
    LittleFS.format();
    LOGW(F("writeconfig"));
    writeconfig();
    LOGW(F("writeconfig rmap"));
    writeconfig_rmap(remote_config);
    LOGW(F("filesystem conversion done"));
  } else
#endif
  if (LittleFS.begin()) {
    LOGN(F("mounted LittleFS file system"));
    readconfig();    
  } else {
    LOGE(F("failed to mount FS"));
    LOGW(F("Reformat LittleFS"));
    LittleFS.begin();    
    LittleFS.format();
    LOGW(F("Reset wifi configuration"));
    wifiManager.resetSettings();

    if (oledpresent) {
      u8g2.clearBuffer();
      u8g2.setCursor(0, 10); 
      u8g2.print(F("Mount FS"));
      u8g2.setCursor(0, 20); 
      u8g2.print(F("Failed"));
      u8g2.setCursor(0, 30); 
      u8g2.print(F("RESET"));
      u8g2.setCursor(0, 40); 
      u8g2.print(F("CONFIGURATION"));
      u8g2.sendBuffer();
      delay(3000);
    }
  }

  if (readconfig_rmap() == String()) {
    LOGN(F("station configuration not found!"));
    LOGN(F("Reset wifi configuration"));
    wifiManager.resetSettings();
  }
  
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_rmap_server("server", "rmap server", rmap_server, 41);
  WiFiManagerParameter custom_rmap_user("user", "rmap user", rmap_user, 10);
  WiFiManagerParameter custom_rmap_password("password", "station password", rmap_password, 31, "type = \"password\"");
  WiFiManagerParameter custom_rmap_slug("slug", "rmap station slug", rmap_slug, 31);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  
  //add all your parameters here
  wifiManager.addParameter(&custom_rmap_server);
  wifiManager.addParameter(&custom_rmap_user);
  wifiManager.addParameter(&custom_rmap_password);
  wifiManager.addParameter(&custom_rmap_slug);

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setConfigPortalTimeout(300);

  // USE THIS OPTIONS WITH WIFIMANAGER VERSION 2
  //if false, timeout captive portal even if a STA client connected to softAP (false), suggest disabling if captiveportal is open
  wifiManager.setAPClientCheck(false);
  //if true, reset timeout when webclient connects (true), suggest disabling if captiveportal is open    
  wifiManager.setWebPortalClientCheck(false);
    
  if (oledpresent) {
      u8g2.clearBuffer();
      u8g2.setCursor(0, 10); 
      u8g2.print(F("ssed:"));
      u8g2.setCursor(0, 20); 
      u8g2.print(F(WIFI_SSED));
      u8g2.setCursor(0, 35); 
      u8g2.print(F("password:"));
      u8g2.setCursor(0, 45); 
      u8g2.print(F(WIFI_PASSWORD));
      u8g2.sendBuffer();
    }

  analogWrite(LED_PIN,512);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  //wifiManager.setDebugOutput(false);
  if (!wifiManager.autoConnect(WIFI_SSED,WIFI_PASSWORD)) {
    LOGE(F("failed to connect and hit timeout"));
    if (oledpresent) {
      u8g2.clearBuffer();
      u8g2.setCursor(0, 10); 
      u8g2.print(F("WIFI KO"));
      u8g2.sendBuffer();
    }
    delay(3000);
    //reboot();
  }else{
    //if you get here you have connected to the WiFi
    LOGN(F("connected... good!"));
    LOGN(F("local ip: %s"),WiFi.localIP().toString().c_str());
    digitalWrite(LED_PIN,HIGH);

    yield();
    
    if (oledpresent) {
      u8g2.clearBuffer();
      u8g2.setCursor(0, 10); 
      u8g2.print(F("WIFI OK"));
      u8g2.sendBuffer();
      u8g2.setCursor(0, 40); 
      u8g2.print(F("IP:"));
      u8g2.setFont(u8g2_font_u8glib_4_tf);
      u8g2.print(WiFi.localIP().toString().c_str());
      u8g2.setFont(u8g2_font_5x7_tf);
      u8g2.sendBuffer();
    }
    yield();
    
  }

  if (shouldSaveConfig){
    //read updated parameters
    strcpy(rmap_server, custom_rmap_server.getValue());
    strcpy(rmap_user, custom_rmap_user.getValue());
    strcpy(rmap_password, custom_rmap_password.getValue());
    strcpy(rmap_slug, custom_rmap_slug.getValue());

    writeconfig();
    if (oledpresent) {
      u8g2.clearBuffer();
      u8g2.setCursor(0, 10); 
      u8g2.print(F("NEW configuration"));
      u8g2.setCursor(0, 20); 
      u8g2.print(F("saved"));
      u8g2.sendBuffer();
      yield();
    }
    
  }
  
  String remote_config= rmap_get_remote_config();

  if ( remote_config == String() ) {
    LOGE(F("remote configuration failed"));
    analogWrite(LED_PIN,50);
    delay(5000);
    digitalWrite(LED_PIN,HIGH);    
    remote_config=readconfig_rmap();
  }else{
    writeconfig_rmap(remote_config);
  }


  firmware_upgrade();
  
  if (!rmap_config(remote_config) == 0) {
    LOGN(F("station not configurated ! restart"));
    //LOGN(F("Reset wifi configuration"));
    //wifiManager.resetSettings();

    if (oledpresent){
      u8g2.clearBuffer();
      u8g2.setCursor(0, 10); 
      u8g2.print(F("Station not"));
      u8g2.setCursor(0, 20); 
      u8g2.print(F("configurated!"));
      u8g2.setCursor(0, 30);
      u8g2.print(F("RESTART"));
      u8g2.sendBuffer();
    }
    
    delay(5000);
    reboot();
  }

 // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "stimawifi.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  while (!MDNS.begin(rmap_slug)) {
    LOGE(F("Error setting up MDNS responder!"));
    delay(1000);
  }
  LOGN(F("mDNS responder started"));


  // setup web server
  webserver.on("/", handle_FullPage);
  webserver.on("/data", handle_Data);
  webserver.on("/data.json", handle_Json);
  webserver.onNotFound(handle_NotFound);
  
  webserver.begin();
  LOGN(F("HTTP server started"));

  configTime(0,0, ntp_server); // this seems not taken in account
  // ESP time and arduino time are different thinks !
  //time_t tnow = now();
  time_t tnow = time(nullptr);

  uint16_t counter=0;
  while (tnow < EPOCH_1_1_2019)
  {
    tnow = now();
    LOGN(F("Time: %s"),ctime(&tnow));
    LOGN(F("Wait for NTP"));

    if (oledpresent){
      u8g2.setCursor(0, 30);
      u8g2.print(F("Setting time"));
      u8g2.sendBuffer();
    }
    if(counter++>=2) {
      if (oledpresent){
	u8g2.clearBuffer();
	u8g2.setCursor(0, 10); 
	u8g2.print(F("Time not"));
	u8g2.setCursor(0, 20); 
	u8g2.print(F("configurated!"));
	//u8g2.setCursor(0, 30);
	//u8g2.print(F("RESTART"));
	u8g2.sendBuffer();
	delay(5000);
      }
      //LOGE(F("NTP time out: Time not configurated, REBOOT"));
      //delay(1000);
      //reboot(); //300 seconds timeout - reset board
      break;
    }
    yield();
    delay(1000);
  }

  setTime(tnow);
  
  LOGN(F("Time: %s"),ctime(&tnow));
  
  LOGN(F("mqtt server: %s"),rmap_mqtt_server);

  mqttclient.setServer(rmap_mqtt_server, 1883);
  
  Alarm.timerRepeat(rmap_sampletime, repeats);             // timer for every SAMPLETIME seconds

  // millis() and other can have overflow problem
  // so we reset everythings one time a week
  //Alarm.alarmRepeat(dowMonday,8,0,0,reboot);          // 8:00:00 every Monday
  unsigned long reboottime;
  if (pmspresent){
    reboottime=3600*24;            // pms stall sometime
  }else{
    reboottime=3600*24*7;          // every week
  }
  LOGN(F("reboot every: %l sec"),reboottime);
  Alarm.timerRepeat(reboottime,reboot);                 // reboot

  // upgrade firmware
  //Alarm.alarmRepeat(4,0,0,firmware_upgrade);          // 4:00:00 every day  
  Alarm.timerRepeat(3600*24,firmware_upgrade);          // every day  

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", STIMAHTTP_PORT);

}


void loop() {
  mqttclient.loop();
  webserver.handleClient();
#if not defined(ARDUINO_D1_MINI32)
  MDNS.update();
#else
  // sometimes ESP32 do not reconnect and we need a restart
  uint16_t counter=0;
  while (WiFi.status() != WL_CONNECTED) { //lost connection
    LOGE(F("WIFI disconnected!"));
    if (oledpresent){
      u8g2.clearBuffer();
      u8g2.setCursor(0, 20); 
      u8g2.print(F("WIFI KO"));
      u8g2.sendBuffer();
    }
    if(counter++>=300) reboot(); //300 seconds timeout - reset board
    delay(1000);
  }
#endif

  Alarm.delay(0);
}
