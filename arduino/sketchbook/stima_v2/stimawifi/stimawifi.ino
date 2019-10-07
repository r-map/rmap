/*
Copyright (C) 2019  Paolo Paruno <p.patruno@iperbole.bologna.it>
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
flash size: 4M (1M SPIFFS)
debug port: disabled
debug level nessuno
lwIP variant: V2 lower memory
Vtables: "flash"
exceptions: disabled
SSL support: Basic SSL"
*/


// increment on change
#define SOFTWARE_VERSION "2019-10-07T00:00"
#define FIRMWARE_TYPE ARDUINO_BOARD
// firmware type for nodemcu is "ESP8266_NODEMCU"
// firmware type for Wemos D1 mini "ESP8266_WEMOS_D1MINI"


#define WIFI_SSED "STIMA-config"
#define WIFI_PASSWORD  "bellastima"
#define SAMPLETIME 10

#define OLEDI2CADDRESS 0X3C

// logging level at compile time
// Available levels are:
// LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
#define LOG_LEVEL   LOG_LEVEL_NOTICE


#if defined(ARDUINO_ESP8266_NODEMCU) 
// NODEMCU FOR LUFDATEN HOWTO
#define SDA D5
#define SCL D6
#define RESET_PIN D7
#define LED_PIN D4
// those are defined in SensorDriverb_config.h
//#define SDS_PIN_RX D1
//#define SDS_PIN_TX D2

#elif defined(ARDUINO_ESP8266_WEMOS_D1MINI)
#define SCL D1
#define SDA D2
#define RESET_PIN D7    // pin to connect to ground for reset wifi configuration
#define LED_PIN D4
// those are defined in SensorDriverb_config.h
//#define SDS_PIN_RX D5
//#define SDS_PIN_TX D6
#elif defined(ARDUINO_ESP8266_WEMOS_D1MINIPRO)
#define SCL D1
#define SDA D2
#define RESET_PIN D7    // pin to connect to ground for reset wifi configuration
#define LED_PIN D4
// those are defined in SensorDriverb_config.h
//#define SDS_PIN_RX D5
//#define SDS_PIN_TX D6
#else
#error "unknown platform"
#endif

#define HTTP_PORT 80
#define WS_PORT 81

// set the frequency
// 30418,25 Hz  : minimum freq with prescaler set to 1 and CPU clock to 16MHz 
#define I2C_CLOCK 10000
// #define I2CPULLUP define this if you want software pullup on I2C

//disable debug at compile time but call function anyway
//#define DISABLE_LOGGING disable

#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
//#include <WebSocketsServer.h>
//#include "EspHtmlTemplateProcessor.h"
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
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
const int update_port = 80;

WiFiClient espClient;
PubSubClient mqttclient(espClient);
ESP8266WebServer webserver(HTTP_PORT);
//WebSocketsServer webSocket(WS_PORT);
//EspHtmlTemplateProcessor templateProcessor(&server);

//flag for saving data
bool shouldSaveConfig = false;

//define your default values here, if there are different values in config.json, they are overwritten.
char rmap_longitude[11] = "";
char rmap_latitude[11] = "";
char rmap_network[31] = "";
char rmap_server[41]= "rmap.cc";
char rmap_user[10]="";
char rmap_password[31]="";
char rmap_slug[31]="stimawifi";
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
  webserver.send(200, "text/html", Json()); 
}

void handle_NotFound(){
  webserver.send(404, "text/plain", "Not found");
}


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


//callback notifying us of the need to save config
void saveConfigCallback () {
  LOGN("Should save config" CR);
  shouldSaveConfig = true;
}

unsigned int coordCharToInt(char* lat){
  String mylat(lat);

  char sep[]=".";
  // without "." or ","
  if (mylat.indexOf(".") < 0 ) {
    if (mylat.indexOf(".") < 0 ) return 0;
    strcpy(sep,",");
  }
  // separator sep found
  
  mylat=mylat.substring(0,mylat.indexOf(sep));
  mylat.trim();
  int latdegree =mylat.toInt();

  mylat=lat;
  mylat=mylat.substring(mylat.indexOf(sep)+1);
  mylat.trim();
  mylat=mylat.substring(0,5);
  uint8_t missed0=5-mylat.length();
  for (uint8_t i = 0; i < (missed0); i++){
    mylat+= String("0");
  }
  if ( latdegree > 0)
    {
      return latdegree*100000+mylat.toInt();
    }
  else
    {
      return latdegree*100000-mylat.toInt();
    }
}

const char* coordIntToChar(int lat){
  if ( lat == 0) return "";
  
  String mylat=String(lat/100000);
  mylat+=".";
  mylat+=String(lat-((lat/100000)*100000));
  return mylat.c_str();
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
  url+="/json/";

  LOGN(F("readRmapRemoteConfig url: %s" CR),url.c_str());  
  //http.begin("http://rmap.cc/stations/pat1/luftdaten/json/");
  http.begin(url.c_str());

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) { //Check the returning code
    payload = http.getString();
    LOGN(payload.c_str());
  }else{
    LOGE(F("Error http: %s" CR),String(httpCode).c_str());
    LOGE(F("Error http: %s" CR),http.errorToString(httpCode).c_str());
    payload=String();
  }
  http.end();
  return payload;
}


bool publish_maint() {

  const String data;  
  
  //String clientId = "ESP8266Client-";
  //clientId += String(random(0xffff), HEX);
    
  LOGN(F("Connet to mqtt broker" CR));

  char longitude [10];
  char latitude [10];
  itoa (coordCharToInt(rmap_longitude),longitude,10);
  itoa (coordCharToInt(rmap_latitude),latitude,10);
  
  char mqttid[100]="";
  strcat(mqttid,rmap_user);
  strcat(mqttid,"/");
  strcat(mqttid,longitude);
  strcat(mqttid,",");
  strcat(mqttid,latitude);
  strcat(mqttid,"/");
  strcat(mqttid,rmap_network);
  
  LOGN(F("mqttid: %s" CR),mqttid);
  
  char mainttopic[100]="";
  strcpy (mainttopic,rmap_mqttmaintpath);
  strcat(mainttopic,"/");
  strcat(mainttopic,mqttid);
  strcat (mainttopic,"/-,-,-/-,-,-,-/B01213");
  LOGN(F("MQTT maint topic: %s" CR),mainttopic);
    
  if (!mqttclient.connect(mqttid,rmap_user,rmap_password,mainttopic,1,1,"{\"v\":\"error01\"}")){
    LOGE(F("Error connecting MQTT" CR));
    LOGE(F("Error status %d" CR),mqttclient.state());
    return false;
  }
  LOGN(F("MQTT connected" CR));
  yield();
  
  if (!mqttclient.publish(mainttopic,(uint8_t*)"{\"v\":\"conn\"}", 12,1)){
    LOGE(F("MQTT maint not published" CR));
    mqttclient.disconnect();
    return false;
  }
  LOGN(F("MQTT maint published" CR));
  return true;
}


bool publish_data(const char* values, const char* timerange, const char* level) {
  
  char topic[100]="";
  StaticJsonBuffer<500> jsonBuffer;

  char longitude [10];
  char latitude [10];
  itoa (coordCharToInt(rmap_longitude),longitude,10);
  itoa (coordCharToInt(rmap_latitude),latitude,10);

  LOGN(F("have to publish: %s" CR),values);

  JsonObject& json =jsonBuffer.parseObject(values);
  if (!json.success()) {
    LOGE(F("reading json data" CR));
    return false;
  }
  for (JsonPair& pair : json) {

    if (pair.value.as<char*>() == NULL){
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
    strcat(payload,pair.value.as<char*>());
    strcat(payload,"}");
      
    strcpy(topic,rmap_mqttrootpath);
    strcat(topic,"/");
    strcat(topic,rmap_user);
    strcat(topic,"/");  
    strcat(topic,longitude);
    strcat(topic,",");
    strcat(topic,latitude);
    strcat(topic,"/");
    strcat(topic,rmap_network);
    strcat(topic,"/");
    strcat(topic,timerange);
    strcat(topic,"/");
    strcat(topic,level);
    strcat(topic,"/");
    strcat(topic,pair.key);

    LOGN(F("mqtt publish: %s %s" CR),topic,payload);
    if (!mqttclient.publish(topic, payload)){
      LOGE(F("MQTT data not published" CR));
      mqttclient.disconnect();
      return false;
    }
    LOGN(F("MQTT data published" CR));
  }
  return true;
}

void firmware_upgrade() {

  StaticJsonBuffer<200> jsonBuffer; 
  JsonObject& root = jsonBuffer.createObject();
  root["ver"] = SOFTWARE_VERSION;
  root["user"] = rmap_user;
  root["slug"] = rmap_slug;
  char buffer[256];
  root.printTo(buffer, sizeof(buffer));
  LOGN(F("url for firmware update: %s" CR),update_url);
  LOGN(F("version for firmware update: %s" CR),buffer);

  analogWriteFreq(4);
  analogWrite(LED_PIN,512);  

  //		t_httpUpdate_return ret = ESPhttpUpdate.update(update_host, update_port, update_url, String(SOFTWARE_VERSION) + String(" ") + esp_chipid + String(" ") + SDS_version + String(" ") + String(current_lang) + String(" ") + String(INTL_LANG));
  t_httpUpdate_return ret = ESPhttpUpdate.update(rmap_server, update_port, update_url, String(buffer));
  switch(ret)
    {
    case HTTP_UPDATE_FAILED:
      LOGE(F("[update] Update failed." CR));
      LOGE(F("%s" CR),ESPhttpUpdate.getLastErrorString().c_str());
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
      LOGN(F("[update] No Update." CR));
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
      LOGN(F("[update] Update ok." CR)); // may not called we reboot the ESP
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

  analogWriteFreq(1);
  digitalWrite(LED_PIN,HIGH);

}


String readconfig_rmap() {

  LOGN(F("mounted file system" CR));
  if (SPIFFS.exists("/rmap.json")) {
    //file exists, reading and loading
    LOGN(F("reading config file" CR));
    File configFile = SPIFFS.open("/rmap.json", "r");
    if (configFile) {
      LOGN(F("opened config file" CR));

      //size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      //std::unique_ptr<char[]> buf(new char[size]);
      //configfile.readBytes(buf.get(), size);

      return configFile.readString();
      
    } else {
      LOGN(F("erro reading rmap file" CR));	
    }
  } else {
    LOGN(F("rmap file do not exist" CR));
  }
  //end read
  return String();  
}

void writeconfig_rmap(String payload) {;

  //save the custom parameters to FS
  LOGN(F("saving config" CR));
  
  File configFile = SPIFFS.open("/rmap.json", "w");
  if (!configFile) {
    LOGN(F("failed to open config file for writing" CR));
  }

  configFile.print(payload);
  configFile.close();
  LOGN(F("saved config parameter" CR));
  //end save
}

int  rmap_config(String payload){

  int status =0;
  int ii = 0;

  if (! (payload == String())) {
    //StaticJsonBuffer<2900> jsonBuffer;
    DynamicJsonBuffer jsonBuffer(4000);
    status = 3;
    JsonArray& array = jsonBuffer.parseArray(payload);
    if (array.success()){
      for (uint8_t i = 0; i < array.size(); i++) {
	if  (array[i]["model"] == "stations.stationmetadata"){
	  if (array[i]["fields"]["active"]){
	    LOGN(F("station metadata found!" CR));
	    strncpy (rmap_mqttrootpath, array[i]["fields"]["mqttrootpath"].as< const char*>(),10);
	    rmap_mqttrootpath[9]='\0';
	    LOGN(F("mqttrootpath: %s" CR),rmap_mqttrootpath);
	    strncpy (rmap_mqttmaintpath, array[i]["fields"]["mqttmaintpath"].as< const char*>(),10);
	    rmap_mqttmaintpath[9]='\0';
	    LOGN(F("mqttmaintpath: %s" CR),rmap_mqttmaintpath);
	    strncpy (rmap_longitude, array[i]["fields"]["lon"].as< const char*>(),10);
	    rmap_longitude[10]='\0';
	    LOGN(F("lon: %s" CR),rmap_longitude);
	    strncpy (rmap_latitude , array[i]["fields"]["lat"].as< const char*>(),10);
	    rmap_latitude[10]='\0';
	    LOGN(F("lat: %s" CR),rmap_latitude);
	    strncpy (rmap_network , array[i]["fields"]["network"].as< const char*>(),30);
	    rmap_network[30]='\0';
	    LOGN(F("network: %s" CR),rmap_network);
	    
	    status = 0;
	  }
	}

	if  (array[i]["model"] == "stations.sensor"){
	  if (array[i]["fields"]["active"]){
	    if (ii < SENSORS_LEN) {
	      LOGN(F("station sensor found!" CR));
	      strncpy (sensors[ii].driver , array[i]["fields"]["driver"].as< const char*>(),SENSORDRIVER_DRIVER_LEN-1);
	      LOGN(F("driver: %s" CR),sensors[ii].driver);
	      strncpy (sensors[ii].type , array[i]["fields"]["type"][0].as< const char*>(),SENSORDRIVER_TYPE_LEN-1);
	      LOGN(F("type: %s" CR),sensors[ii].type);
	      strncpy (sensors[ii].timerange, array[i]["fields"]["timerange"].as< const char*>(),SENSORDRIVER_META_LEN-1);
	      LOGN(F("timerange: %s" CR),sensors[ii].timerange);
	      strncpy (sensors[ii].level, array[i]["fields"]["level"].as< const char*>(),SENSORDRIVER_META_LEN-1);
	      LOGN(F("level: %s" CR),sensors[ii].level);
	      sensors[ii].address = array[i]["fields"]["address"];	    
	      LOGN(F("address: %d" CR),sensors[ii].address);
	      
	      sd[ii]=SensorDriver::create(sensors[ii].driver,sensors[ii].type);
	      if (sd[ii] == 0){
		LOGN(F("%s:%s driver not created !" CR),sensors[ii].driver,sensors[ii].type);
	      }else{		
		if (!(sd[ii]->setup(sensors[ii].driver, sensors[ii].address, -1, sensors[ii].type) == SD_SUCCESS)) {
		  LOGE(F("sensor not present or broken" CR));
		  analogWrite(LED_PIN,750);
		  delay(5000);
		}		
	      }
	      ii++;
	    }
	    
	    status = 0;
	  }
	}

      }
    } else {
      LOGE(F("error parsing array" CR));
      analogWrite(LED_PIN,973);
      delay(5000);
      status = 2;
    }
    
  }else{
    status=1;
  }
  return status;
}



void readconfig() {

  LOGN(F("mounted file system" CR));
  if (SPIFFS.exists("/config.json")) {
    //file exists, reading and loading
    LOGN(F("reading config file" CR));
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile) {
      LOGN(F("opened config file" CR));
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        StaticJsonBuffer<500> jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        if (json.success()) {
          LOGN(F("parsed json" CR));
	  //json.printTo(Serial);

	  //if (json.containsKey("rmap_longitude"))strcpy(rmap_longitude, json["rmap_longitude"]);
	  //if (json.containsKey("rmap_latitude")) strcpy(rmap_latitude, json["rmap_latitude"]);
          if (json.containsKey("rmap_server")) strcpy(rmap_server, json["rmap_server"]);
          if (json.containsKey("rmap_user")) strcpy(rmap_user, json["rmap_user"]);
          if (json.containsKey("rmap_password")) strcpy(rmap_password, json["rmap_password"]);
          if (json.containsKey("rmap_slug")) strcpy(rmap_slug, json["rmap_slug"]);
	  if (json.containsKey("rmap_mqttrootpath")) strcpy(rmap_mqttrootpath, json["rmap_mqttrootpath"]);
	  if (json.containsKey("rmap_mqttmaintpath")) strcpy(rmap_mqttmaintpath, json["rmap_mqttmaintpath"]);
	  
	  LOGN(F("loaded config parameter:" CR));
	  //LOGN(F("longitude: %s" CR),rmap_longitude);
	  //LOGN(F("latitude: %s" CR),rmap_latitude);
	  LOGN(F("server: %s" CR),rmap_server);
	  LOGN(F("user: %s" CR),rmap_user);
	  //LOGN(F("password: %s" CR),rmap_password);
	  LOGN(F("slug: %s" CR),rmap_slug);
	  LOGN(F("mqttrootpath: %s" CR),rmap_mqttrootpath);
	  LOGN(F("mqttmaintpath: %s" CR),rmap_mqttmaintpath);
	  
        } else {
          LOGN(F("failed to load json config" CR));
        }
      } else {
	LOGN(F("erro reading config file" CR));	
      }
    } else {
      LOGN(F("config file do not exist" CR));
    }
  //end read
}

void writeconfig() {;

  //save the custom parameters to FS
  LOGN(F("saving config" CR));
  //DynamicJsonBuffer jsonBuffer;
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
    
  //json["rmap_longitude"] = rmap_longitude;
  //json["rmap_latitude"] = rmap_latitude;
  json["rmap_server"] = rmap_server;
  json["rmap_user"] = rmap_user;
  json["rmap_password"] = rmap_password;
  json["rmap_slug"] = rmap_slug;
  json["rmap_mqttrootpath"] = rmap_mqttrootpath;
  json["rmap_mqttmaintpath"] = rmap_mqttmaintpath;
  
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    LOGN(F("failed to open config file for writing" CR));
  }

  //json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
  LOGN(F("saved config parameter" CR));
}


void web_values(const char* values) {
  
  StaticJsonBuffer<500> jsonBuffer;

  JsonObject& json =jsonBuffer.parseObject(values);
  if (json.success()){
    for (JsonPair& pair : json) {

      if (pair.value.as<char*>() == NULL) continue;
      float val=pair.value.as<float>();

      if (strcmp(pair.key,"B12101")==0){
	temperature=round((val-27315)/10.)/10;
      }
      if (strcmp(pair.key,"B13003")==0){
	humidity=round(val);
      }
      if (strcmp(pair.key,"B15198")==0){
	pm2=round(val/10.);
      }
      if (strcmp(pair.key,"B15195")==0){
	pm10=round(val/10.);
      }
      if (strcmp(pair.key,"B15242")==0){
	co2=round(val/1.8);
      }
    }
  }
}


void display_values(const char* values) {
  
  StaticJsonBuffer<500> jsonBuffer;

  JsonObject& json =jsonBuffer.parseObject(values);
  if (json.success()){
    for (JsonPair& pair : json) {

      if (pair.value.as<char*>() == NULL) continue;
      float val=pair.value.as<float>();

      u8g2.setCursor(0, (displaypos)*CH); 
      
      if (strcmp(pair.key,"B12101")==0){
	u8g2.print(F("T   : "));
	u8g2.print(round((val-27315)/10.)/10,1);
	u8g2.print(F(" C"));
	displaypos++;	
      }
      if (strcmp(pair.key,"B13003")==0){
	u8g2.print(F("U   : "));
	u8g2.print(round(val),0);
	u8g2.print(F(" %"));
	displaypos++;	
      }
      if (strcmp(pair.key,"B15198")==0){
	u8g2.print(F("PM2 : "));
	u8g2.print(round(val/10.),0);
	u8g2.print(F(" ug/m3"));
	displaypos++;	
      }
      if (strcmp(pair.key,"B15195")==0){
	u8g2.print(F("PM10: "));
	u8g2.print(round(val/10.),0);
	u8g2.print(F(" ug/m3"));
	displaypos++;	
      }
      if (strcmp(pair.key,"B15242")==0){
	u8g2.print(F("CO2 : "));
	u8g2.print(round(val/1.8),0);
	u8g2.print(F(" ppm"));
	displaypos++;	
      }
    }
  }
}


void repeats() {

  long unsigned int waittime,maxwaittime=0;

  char values[MAX_VALUES_FOR_SENSOR*20];
  size_t lenvalues=MAX_VALUES_FOR_SENSOR*20;
  //  long values[MAX_VALUES_FOR_SENSOR];
  //  size_t lenvalues=MAX_VALUES_FOR_SENSOR;
  displaypos=1;
  u8g2.clearBuffer();

  digitalWrite(LED_PIN,LOW);
  time_t tnow = time(nullptr);
  LOGN(F("Time: %s" CR),ctime(&tnow));
  
  // prepare sensors to measure
  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == 0){
      LOGN(F("prepare sd %d" CR),i);
      if (sd[i]->prepare(waittime) == SD_SUCCESS){
	maxwaittime=_max(maxwaittime,waittime);
      }else{
	LOGN(F("%s: prepare failed !" CR),sensors[i].driver);
      }
    }
  }

  yield();
  
  //wait sensors to go ready
  LOGN(F("wait sensors for ms: %d" CR),maxwaittime);
  unsigned long int now=millis();

  // manage mqtt reconnect as RMAP standard
  if (!mqttclient.connected()){
    if (!publish_maint()) {
      LOGE(F("Error in publish maint" CR));
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
  }

  if (oledpresent) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 20); 
    u8g2.print(F("Measure!"));
    u8g2.sendBuffer();
    displaypos=1;
    u8g2.clearBuffer();
  }

  while ((float(maxwaittime)-float(millis()-now)) >0.) {
    //LOGN(F("delay" CR));
    mqttclient.loop();;
    webserver.handleClient();
    MDNS.update();
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
      LOGN(F("getJson sd %d" CR),i);
      if (sd[i]->getJson(values,lenvalues) == SD_SUCCESS){
	if(publish_data(values,sensors[i].timerange,sensors[i].level)){
	  web_values(values);
	  if (oledpresent) {
	    display_values(values);
	  }
	}else{
	  LOGE(F("Error in publish data" CR));
	  if (oledpresent) {
	    u8g2.setCursor(0, (displaypos++)*CH); 
	    u8g2.print(F("MQTT error publish"));
	  }else{
	    analogWrite(LED_PIN,973);
	    delay(5000);
	  }
	}
      }else{
	LOGE(F("Error getting json from sensor" CR));
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

void setup() {
  // put your setup code here, to run once:

  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  analogWriteFreq(1);
  digitalWrite(LED_PIN,HIGH);

  Serial.begin(115200);
  Serial.println();

  // Pass log level, whether to show log level, and print interface.
  // Available levels are:
  // LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
  // Note: if you want to fully remove all logging code, change #define LOG_LEVEL ....
  //       this will significantly reduce your project size

  // set runtime log level to the same of compile time
  Log.begin(LOG_LEVEL, &Serial);
  LOGN(F("Started" CR));
  LOGN(F("Version: " SOFTWARE_VERSION CR));

#ifdef I2CPULLUP
  //if you want to set the internal pullup
  digitalWrite( SDA, HIGH);
  digitalWrite( SCL, HIGH);
#else
  // here we enforce we do not want pullup
  digitalWrite( SDA, LOW);
  digitalWrite( SCL, LOW);
#endif

  Wire.begin(SDA,SCL);
  Wire.setClock(I2C_CLOCK);


  // check return value of
  // the Write.endTransmisstion to see if
  // a device did acknowledge to the address.
  Wire.beginTransmission(OLEDI2CADDRESS);
  if (Wire.endTransmission() == 0) {
    LOGN(F("OLED Found" CR));
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
        LOGN(F("OLED NOT Found" CR));
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
  LOGN(F("esp_chipid: %s " CR),esp_chipid );
  */

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  configTime(0, 0, "0.europe.pool.ntp.org");

  // manage reset button in hardware (RESET_PIN) or in software (I2C)
  bool reset=digitalRead(RESET_PIN) == LOW;
  if (button.get() == 0)
  {
    if (button.BUTTON_A)
    {
      //String keyString[] = {"None", "Press", "Long Press", "Double Press", "Hold"};
      //LOGN(F("BUTTON A: %s" CR),keyString[button.BUTTON_A].c_str());
      if (button.BUTTON_A == KEY_VALUE_HOLD) reset=true;
    }
  }
  
  if (reset) {
    LOGN(F("clean FS" CR));
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
    SPIFFS.format();
    LOGN(F("Reset wifi configuration" CR));
    wifiManager.resetSettings();
  }
  
  //read configuration from FS json
  LOGN(F("mounting FS..." CR));
  if (SPIFFS.begin()) {
    readconfig();
  } else {
    LOGN(F("failed to mount FS" CR));
    LOGN(F("Reformat SPIFFS" CR));
    SPIFFS.format();
    LOGN(F("Reset wifi configuration" CR));
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
    LOGN(F("station configuration not found!" CR));
    LOGN(F("Reset wifi configuration" CR));
    wifiManager.resetSettings();
  }
  
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_rmap_server("server", "rmap server", rmap_server, 41);
  WiFiManagerParameter custom_rmap_user("user", "rmap user", rmap_user, 10);
  WiFiManagerParameter custom_rmap_password("password", "rmap password", rmap_password, 31, "type = \"password\"");
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
  wifiManager.setTimeout(180);


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
    LOGN(F("failed to connect and hit timeout" CR));
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
    LOGN(F("connected... good!" CR));
    if (oledpresent) {
      u8g2.clearBuffer();
      u8g2.setCursor(0, 10); 
      u8g2.print(F("WIFI OK"));
      u8g2.sendBuffer();
    }
    digitalWrite(LED_PIN,HIGH);
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
    }
    
  }

  LOGN(F("local ip: %s" CR),WiFi.localIP().toString().c_str());

  firmware_upgrade();

  if (oledpresent) {
    u8g2.setCursor(0, 40); 
    u8g2.print(F("IP:"));
    u8g2.setFont(u8g2_font_u8glib_4_tf);
    u8g2.print(WiFi.localIP().toString().c_str());
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.sendBuffer();
  }

  
  String remote_config= rmap_get_remote_config();

  if ( remote_config == String() ) {
    LOGN(F("remote configuration failed" CR));
    analogWrite(LED_PIN,50);
    delay(5000);
    digitalWrite(LED_PIN,HIGH);    
    remote_config=readconfig_rmap();
  }else{
    LOGN(F("write configuration" CR));
    writeconfig_rmap(remote_config);
  }

  //if (strcmp(rmap_longitude,"") == 0 ||strcmp(rmap_latitude,"") == 0) { 
  if (!rmap_config(remote_config) == 0) {
    LOGN(F("station not configurated ! restart" CR));
    //LOGN(F("Reset wifi configuration" CR));
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
    LOGN(F("Error setting up MDNS responder!"));
    delay(1000);
  }
  LOGN(F("mDNS responder started" CR));


  // setup web server
  webserver.on("/", handle_FullPage);
  webserver.on("/data", handle_Data);
  webserver.on("/data.json", handle_Json);
  webserver.onNotFound(handle_NotFound);
  
  webserver.begin();
  LOGN(F("HTTP server started" CR));

  time_t tnow = time(nullptr);
  LOGN(F("Time: %s" CR),ctime(&tnow));
  
  LOGN(F("mqtt server: %s" CR),rmap_server);
  mqttclient.setServer(rmap_server, 1883);

  Alarm.timerRepeat(SAMPLETIME, repeats);             // timer for every SAMPLETIME seconds

  // millis() and other can have overflow problem
  // so we reset everythings one time a week
  //Alarm.alarmRepeat(dowMonday,8,0,0,reboot);          // 8:00:00 every Monday
  Alarm.timerRepeat(3600*24*7,reboot);          // every week

  // upgrade firmware
  //Alarm.alarmRepeat(4,0,0,firmware_upgrade);          // 4:00:00 every day  
  Alarm.timerRepeat(3600*24,firmware_upgrade);          // every day  

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", HTTP_PORT);
}


void loop() {
  mqttclient.loop();
  webserver.handleClient();
  MDNS.update();
  Alarm.delay(0);
}
