/*
Copyright (C) 2017  Paolo Paruno <p.patruno@iperbole.bologna.it>
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

// increment on change
#define SOFTWARE_VERSION "2017-11-26T12:00"
#define FIRMWARE_TYPE "stima_wemosd1mini"
//#define FIRMWARE_TYPE "stima_nodemcu"

#define RESET_PIN D0    // pin to connect to ground for reset wifi configuration
#define WIFI_SSED "STIMA-configuration"
#define WIFI_PASSWORD  "bellastima"
#define SAMPLETIME 20

#define SDS_PIN_RX D1
#define SDS_PIN_TX D2

//disable debug at compile time but call function anyway
//#define DISABLE_LOGGING disable

#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "Sds011.h"
#include <SoftwareSerial.h>
#include <TimeAlarms.h>
#include <ArduinoLog.h>

// logging level at compile time
// Available levels are:
// LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
#define LOG_LEVEL   LOG_LEVEL_NOTICE

// watchdog is enabled by default on ESP
// https://techtutorialsx.com/2017/01/21/esp8266-watchdog-functions/

  
const char* update_host = "rmap.cc";
const char* update_url = "/firmware/update/"FIRMWARE_TYPE"/";
const int update_port = 80;

WiFiClient espClient;
PubSubClient mqttclient(espClient);

//flag for saving data
bool shouldSaveConfig = false;

//define your default values here, if there are different values in config.json, they are overwritten.
char rmap_longitude[11] = "";
char rmap_latitude[11] = "";
char rmap_network[31] = "";
char rmap_server[41]= "rmap.cc";
char rmap_user[10]="";
char rmap_password[31]="";
char rmap_slug[31]="stimaesp";
char rmap_mqttrootpath[10] = "sample";
char rmap_mqttmaintpath[10] = "maint";


// for sensoron
#define SENSORS_LEN 5
#define SENSORDRIVER_DRIVER_LEN 5
#define SENSORDRIVER_TYPE_LEN 5
#define SENSORDRIVER_MQTTPATH_LEN 30
#define MAX_VALUES_FOR_SENSOR 3

#include <SensorDriver.h>

SoftwareSerial mySerial(SDS_PIN_RX, SDS_PIN_TX, false, 128);
sds011::Sds011 sensor(mySerial);

// sensor information
struct sensor_t
{
  char driver[SENSORDRIVER_DRIVER_LEN];         // driver name
  int node;                                 // RF24Nework node id
  char type[SENSORDRIVER_TYPE_LEN];         // sensor type name
  int address;                              // i2c address
  char mqttpath[SENSORDRIVER_MQTTPATH_LEN]; // path for mqtt pubblish
  sensor_t() : address(-1) {
       driver[0]='\0';
       node = -1;
       type[0]='\0';
       mqttpath[0]='\0';
  }
} sensors[SENSORS_LEN];;

SensorDriver* sd[SENSORS_LEN];

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

unsigned int coordCharToInt(char* lat){
  String mylat(lat);

  char* sep=".";
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
  for (int i = 0; i < (5-mylat.length()); i++){
    mylat+= String("0");
  }
  return latdegree*100000+mylat.toInt();
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

  LOGN(F("readRmapRemoteConfig url: %s"CR),url.c_str());  
  //http.begin("http://rmap.cc/stations/pat1/luftdaten/json/");
  http.begin(url.c_str());

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) { //Check the returning code
    payload = http.getString();
    LOGN(payload.c_str());
  }else{
    LOGE(F("Error http: %s"CR),String(httpCode).c_str());
    LOGE(F("Error http: %s"CR),http.errorToString(httpCode).c_str());
    payload=String();
  }
  http.end();
  return payload;
}


bool publish_maint() {

  const String data;  
  
  if (!mqttclient.connected()) {
    //String clientId = "ESP8266Client-";
    //clientId += String(random(0xffff), HEX);
    
    LOGN(F("Connet to mqtt broker"CR));

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
    
    LOGN(F("mqttid: %s"CR),mqttid);

    char mainttopic[100]="";
    strcpy (mainttopic,rmap_mqttmaintpath);
    strcat(mainttopic,"/");
    strcat(mainttopic,mqttid);
    strcat (mainttopic,"/-,-,-/-,-,-,-/B01213");
    LOGN(F("MQTT maint topic: %s"CR),mainttopic);
    
    if (mqttclient.connect(mqttid,rmap_user,rmap_password,mainttopic,1,1,"{\"v\":\"error01\"}")){
      LOGN(F("MQTT connected"CR));

      if (!mqttclient.publish(mainttopic,(uint8_t*)"{\"v\":\"conn\"}", 12,1)){
	LOGN(F("MQTT maint published"CR));
      }
      return true;
    }else{
      LOGE(F("Error connecting MQTT"CR));
      return false;
    }
  } else {
    return true;
  }
}

/*
void publish_pm(const char* sensor, const int pm) {
  
  bool havetopublish=false;
  char topic[100]="";
  char payload[100]="{\"v\":";

  strcpy(topic,rmap_mqttrootpath);
  strcat(topic,"/");
  strcat(topic,rmap_user);
  strcat(topic,"/");
  char longitude [10];
  char latitude [10];
  itoa (coordCharToInt(rmap_longitude),longitude,10);
  itoa (coordCharToInt(rmap_latitude),latitude,10);
  
  strcat(topic,longitude);
  strcat(topic,",");
  strcat(topic,latitude);
  strcat(topic,"/fixed/254,0,0/103,2000,-,-/");
  
  LOGN(sensor);
  if (strcmp(sensor,"SDS_PM10")== 0)
    {
      LOGN(F("SDS_PM10 found"CR));
      strcat(topic,"B15195");
      havetopublish=true;
    }
  if (strcmp(sensor,"SDS_PM2")== 0)
    {
      LOGN(F("SDS_PM2 found"CR));
      strcat(topic,"B15198");
      havetopublish=true;
    }

  if (havetopublish)
    {
      // itoa str should be an array long enough to contain any possible value: (sizeof(int)*8+1) for radix=2, i.e. 17 bytes in 16-bits platforms 
      char value[17];
      itoa (pm,value,10);
      strcat(payload,value);
      strcat(payload,"}");
      LOGN(F("mqtt publish: %s %s"CR),topic,payload);
      mqttclient.publish(topic, payload);
    }
}
*/


void publish_data(const char* values) {
  
  bool havetopublish=false;
  char topic[100]="";
  StaticJsonBuffer<200> jsonBuffer;

  char longitude [10];
  char latitude [10];
  itoa (coordCharToInt(rmap_longitude),longitude,10);
  itoa (coordCharToInt(rmap_latitude),latitude,10);

  LOGN(F("have to publish: %s"CR),values);

  JsonObject& json =jsonBuffer.parseObject(values);
  if (json.success()){
    for (JsonPair& pair : json) {

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
      strcat(topic,"254,0,0");
      strcat(topic,"/");
      strcat(topic,"103,2000,-,-");
      strcat(topic,"/");
      strcat(topic,pair.key);

      char payload[100]="{\"v\":";
      strcat(payload,pair.value.as<char*>());
      strcat(payload,"}");
      LOGN(F("mqtt publish: %s %s"CR),topic,payload);
      mqttclient.publish(topic, payload);
    }
  }
  /*
  while(){
  
    // itoa str should be an array long enough to contain any possible value: (sizeof(int)*8+1) for radix=2, i.e. 17 bytes in 16-bits platforms 
    char value[17];
    itoa (pm,value,10);
    strcat(payload,value);
    strcat(payload,"}");
    LOGN(F("mqtt publish: %s %s"CR),topic,payload);
    mqttclient.publish(topic, payload);
  }
  */

}

void firmware_upgrade() {

  StaticJsonBuffer<200> jsonBuffer; 
  JsonObject& root = jsonBuffer.createObject();
  root["ver"] = SOFTWARE_VERSION;
  root["user"] = rmap_user;
  root["slug"] = rmap_slug;
  char buffer[256];
  root.printTo(buffer, sizeof(buffer));
  LOGN(F("version for firmware upgrade %s"CR),buffer);
		
  //		t_httpUpdate_return ret = ESPhttpUpdate.update(update_host, update_port, update_url, String(SOFTWARE_VERSION) + String(" ") + esp_chipid + String(" ") + SDS_version + String(" ") + String(current_lang) + String(" ") + String(INTL_LANG));
  t_httpUpdate_return ret = ESPhttpUpdate.update(update_host, update_port, update_url, String(buffer));
  switch(ret)
    {
    case HTTP_UPDATE_FAILED:
      LOGE(F("[update] Update failed."CR));
      LOGE(F("%s"CR),ESPhttpUpdate.getLastErrorString().c_str());
    break;
    case HTTP_UPDATE_NO_UPDATES:
      LOGN(F("[update] No Update."CR));
      break;
    case HTTP_UPDATE_OK:
      LOGN(F("[update] Update ok."CR)); // may not called we reboot the ESP
      break;
    }
}


String readconfig_rmap() {

  LOGN(F("mounted file system"CR));
  if (SPIFFS.exists("/rmap.json")) {
    //file exists, reading and loading
    LOGN(F("reading config file"CR));
    File configFile = SPIFFS.open("/rmap.json", "r");
    if (configFile) {
      LOGN(F("opened config file"CR));

      //size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      //std::unique_ptr<char[]> buf(new char[size]);
      //configfile.readBytes(buf.get(), size);

      return configFile.readString();
      
    } else {
      LOGN(F("erro reading rmap file"CR));	
    }
  } else {
    LOGN(F("rmap file do not exist"CR));
  }
  //end read
  return String();  
}

void writeconfig_rmap(String payload) {;

  //save the custom parameters to FS
  LOGN(F("saving config"CR));
  
  File configFile = SPIFFS.open("/rmap.json", "w");
  if (!configFile) {
    LOGN(F("failed to open config file for writing"CR));
  }

  //DynamicJsonBuffer jsonBuffer;
  //StaticJsonBuffer<500> jsonBuffer;
  //JsonObject& json = jsonBuffer.parseObject(payload);
  // set json
  //json.printTo(Serial);
  //json.printTo(configFile);

  configFile.print(payload);
  configFile.close();
  LOGN(F("saved config parameter"CR));
  //end save
}

int  rmap_config(String payload){

  int status =0;
  int ii = 0;

  if (! (payload == String())) {
    StaticJsonBuffer<1500> jsonBuffer;
    status = 3;
    JsonArray& array = jsonBuffer.parseArray(payload);
    if (array.success()){
      for (int i = 0; i < array.size(); i++) {
	if  (array[i]["model"] == "stations.stationmetadata"){
	  if (array[i]["fields"]["active"]){
	    LOGN(F("station metadata found!"CR));
	    strncpy (rmap_mqttrootpath, array[i]["fields"]["mqttrootpath"].as< const char*>(),10);
	    rmap_mqttrootpath[9]='\0';
	    LOGN(F("mqttrootpath: %s"CR),rmap_mqttrootpath);
	    strncpy (rmap_mqttmaintpath, array[i]["fields"]["mqttmaintpath"].as< const char*>(),10);
	    rmap_mqttmaintpath[9]='\0';
	    LOGN(F("mqttmaintpath: %s"CR),rmap_mqttmaintpath);
	    strncpy (rmap_longitude, array[i]["fields"]["lon"].as< const char*>(),10);
	    rmap_longitude[10]='\0';
	    LOGN(F("lon: %s"CR),rmap_longitude);
	    strncpy (rmap_latitude , array[i]["fields"]["lat"].as< const char*>(),10);
	    rmap_latitude[10]='\0';
	    LOGN(F("lat: %s"CR),rmap_latitude);
	    strncpy (rmap_network , array[i]["fields"]["network"].as< const char*>(),30);
	    rmap_network[30]='\0';
	    LOGN(F("network: %s"CR),rmap_network);
	    
	    status = 0;
	  }
	}

	/*
[
{
  "model": "stations.stationmetadata",
  "fields": {
    "name": "luftdaten",
    "active": true,
    "slug": "luftdaten",
    "ident": [
      "pat1"
    ],
    "lat": 44.48896,
    "lon": 11.36987,
    "network": "fixed",
    "mqttrootpath": "sample",
    "mqttmaintpath": "sample",
    "category": "test"
  }
},
{
  "model": "stations.board",
  "fields": {
    "name": "luftdaten",
    "active": true,
    "slug": "luftdaten",
    "category": "base",
    "stationmetadata": [
      "luftdaten",
      [
        "pat1"
      ]
    ]
  }
},
{
  "model": "stations.sensor",
  "fields": {
    "active": true,
    "name": "SDS011 particolato",
    "driver": "I2C",
    "type": [
      "SSD"
    ],
    "i2cbus": 1,
    "address": 72,
    "node": 1,
    "timerange": "254,0,0",
    "level": "103,2000,-,-",
    "board": [
      "luftdaten",
      [
        "luftdaten",
        [
          "pat1"
        ]
      ]
    ]
  }
 }
 ]
	*/
	   
	char driver[5];
	char type[4];
	char timerange[30];
	char level[30];
	
	if  (array[i]["model"] == "stations.sensor"){
	  if (array[i]["fields"]["active"]){
	    LOGN(F("station sensor found!"CR));
	    strncpy (driver , array[i]["fields"]["driver"].as< const char*>(),5);
	    driver[4]='\0';
	    LOGN(F("driver: %s"CR),driver);
	    strncpy (type , array[i]["fields"]["type"][0].as< const char*>(),4);
	    type[3]='\0';
	    LOGN(F("type: %s"CR),type);
	    strncpy (timerange, array[i]["fields"]["timerange"].as< const char*>(),30);
	    timerange[29]='\0';
	    LOGN(F("timerange: %s"CR),timerange);
	    strncpy (level, array[i]["fields"]["level"].as< const char*>(),30);
	    level[29]='\0';
	    LOGN(F("level: %s"CR),level);
	    unsigned int address = array[i]["fields"]["address"];	    
	    LOGN(F("address: %d"CR),address);

	    if (ii < SENSORS_LEN) {
	      
	      sd[ii]=SensorDriver::create(driver,type);
	      if (sd[ii] == NULL){
		LOGN(F("%s: driver not created !"CR),driver);
		
	      }else{
		
		if (!(sd[ii]->setup(driver, address, -1, type) == SD_SUCCESS)) {			   
		  LOGE(F("sensor not present or broken"CR));
		  // comment the next line to be less restrictive
		  //return E_INTERNAL_ERROR;
		}

		ii++;
		
	      }
	    }
	    
	    status = 0;
	  }
	}

      }
    } else {
      LOGE(F("error parsing array"CR));
      status = 2;
    }
    
  }else{
    status=1;
  }
  return status;
}



void readconfig() {

  LOGN(F("mounted file system"CR));
  if (SPIFFS.exists("/config.json")) {
    //file exists, reading and loading
    LOGN(F("reading config file"CR));
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile) {
      LOGN(F("opened config file"CR));
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        StaticJsonBuffer<500> jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        if (json.success()) {
          LOGN(F("parsed json"CR));
	  json.printTo(Serial);

	  //if (json.containsKey("rmap_longitude"))strcpy(rmap_longitude, json["rmap_longitude"]);
	  //if (json.containsKey("rmap_latitude")) strcpy(rmap_latitude, json["rmap_latitude"]);
          if (json.containsKey("rmap_server")) strcpy(rmap_server, json["rmap_server"]);
          if (json.containsKey("rmap_user")) strcpy(rmap_user, json["rmap_user"]);
          if (json.containsKey("rmap_password")) strcpy(rmap_password, json["rmap_password"]);
          if (json.containsKey("rmap_slug")) strcpy(rmap_slug, json["rmap_slug"]);
	  if (json.containsKey("rmap_mqttrootpath")) strcpy(rmap_mqttrootpath, json["rmap_mqttrootpath"]);
	  if (json.containsKey("rmap_mqttmaintpath")) strcpy(rmap_mqttmaintpath, json["rmap_mqttmaintpath"]);
	  
	  LOGN(F("loaded config parameter:"CR));
	  //LOGN(F("longitude: %s"CR),rmap_longitude);
	  //LOGN(F("latitude: %s"CR),rmap_latitude);
	  LOGN(F("server: %s"CR),rmap_server);
	  LOGN(F("user: %s"CR),rmap_user);
	  LOGN(F("password: %s"CR),rmap_password);
	  LOGN(F("slug: %s"CR),rmap_slug);
	  LOGN(F("mqttrootpath: %s"CR),rmap_mqttrootpath);
	  LOGN(F("mqttmaintpath: %s"CR),rmap_mqttmaintpath);
	  
        } else {
          LOGN(F("failed to load json config"CR));
        }
      } else {
	LOGN(F("erro reading config file"CR));	
      }
    } else {
      LOGN(F("config file do not exist"CR));
    }
  //end read
}

void writeconfig() {;

  //save the custom parameters to FS
  LOGN(F("saving config"CR));
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
    LOGN(F("failed to open config file for writing"CR));
  }

  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
  LOGN(F("saved config parameter"CR));
}

/*
void repeat() {

#define SDS_SAMPLES 3
  
  int pm2, pm10;
  bool ok;

  sensor.set_sleep(false);
  delay(3000);
  ok = sensor.query_data_auto(&pm2, &pm10, SDS_SAMPLES);
  sensor.set_sleep(true);
  if (!ok) {
    LOGE(F("error getting sds011 data"CR));
  }
  if (publish_maint()) {
    if (ok) {
      publish_pm( "SDS_PM2", pm2);
      publish_pm( "SDS_PM10", pm10);
    }
  }
}
*/

void repeats() {


  long unsigned int waittime,maxwaittime=0;

  char values[MAX_VALUES_FOR_SENSOR*20];
  size_t lenvalues=MAX_VALUES_FOR_SENSOR*20;
  //  long values[MAX_VALUES_FOR_SENSOR];
  //  size_t lenvalues=MAX_VALUES_FOR_SENSOR;
  
  // prepare sensors to measure
  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == NULL){
      LOGN(F("prepare sd %d"CR),i);
      if (sd[i]->prepare(waittime) == SD_SUCCESS){
	maxwaittime=_max(maxwaittime,waittime);
      }else{
	LOGN(F("%s: prepare failed !"CR),sensors[i].driver);
      }
    }
  }

  //wait sensors to go ready
  LOGN(F("wait sensors for ms: %d"CR),maxwaittime);
  delay(maxwaittime);

  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == NULL){

      //      for (int ii = 0; ii < lenvalues; ii++) {
      //	values[ii]=4294967296;
      //      }

      LOGN(F("getJson sd %d"CR),i);
      if (sd[i]->getJson(values,lenvalues) == SD_SUCCESS){
	//for (int ii = 0; ii < lenvalues; ii++) {
	//  if (!(values[ii] == 4294967296))LOGN(F("value: %d: %d"CR),ii,values[ii]);
	//}

	publish_data(values);

	//if (!(values[0] == 4294967296)) publish_pm( "SDS_PM2", values[0]);
	//if (!(values[1] == 4294967296)) publish_pm( "SDS_PM10", values[1]);
	
      }else{
	LOGN(F("Error"CR));
      }
      
      /*
      // get values in json format
      aj=sd[i]->get(&values));
      json=aJson.print(aj,50);
      Serial.print(sensors[i].type);
      Serial.print(" : ");
      Serial.println(json);
      free(json);
      aJson.deleteItem(aj);
      */
    }
  }
}


void reboot() {
  //reset and try again, or maybe put it to deep sleep
  ESP.restart();
  delay(5000);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  // Pass log level, whether to show log level, and print interface.
  // Available levels are:
  // LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
  // Note: if you want to fully remove all logging code, change #define LOG_LEVEL ....
  //       this will significantly reduce your project size

  // set runtime log level to the same of compile time
  Log.begin(LOG_LEVEL, &Serial);

  pinMode(RESET_PIN, INPUT_PULLUP);

  /*
  char esp_chipid[11];
  itoa(ESP.getChipId(),esp_chipid,10);
  LOGN(F("esp_chipid: %s "CR),esp_chipid );
  */

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  if (digitalRead(RESET_PIN) == LOW) {
    LOGN(F("clean FS"));
    SPIFFS.format();
    LOGN(F("Reset wifi configuration"CR));
    wifiManager.resetSettings();
  }
  
  //read configuration from FS json
  LOGN(F("mounting FS..."CR));
  if (SPIFFS.begin()) {
    readconfig();
  } else {
    LOGN(F("failed to mount FS"CR));
  }
  
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_rmap_server("server", "rmap server", rmap_server, 40);
  WiFiManagerParameter custom_rmap_user("user", "rmap user", rmap_user, 10);
  WiFiManagerParameter custom_rmap_password("password", "rmap password", rmap_password, 30, "type = \"password\"");
  WiFiManagerParameter custom_rmap_slug("slug", "rmap station slug", rmap_slug, 30);

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

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(WIFI_SSED,WIFI_PASSWORD)) {
    LOGN(F("failed to connect and hit timeout"CR));
    delay(3000);
    reboot();
  }
  
  //if you get here you have connected to the WiFi
  LOGN(F("connected...yeey :)"CR));


  if (shouldSaveConfig){
    //read updated parameters
    strcpy(rmap_server, custom_rmap_server.getValue());
    strcpy(rmap_user, custom_rmap_user.getValue());
    strcpy(rmap_password, custom_rmap_password.getValue());
    strcpy(rmap_slug, custom_rmap_slug.getValue());

    writeconfig();
  }

  LOGN(F("local ip: %s"CR),WiFi.localIP().toString().c_str());

  firmware_upgrade();

  String remote_config= rmap_get_remote_config();

  if ( remote_config == String() ) {
    LOGN(F("remote configuration failed"CR));
    remote_config=readconfig_rmap();
  }else{
    LOGN(F("write configuration"CR));
    writeconfig_rmap(remote_config);
  }

  //if (strcmp(rmap_longitude,"") == 0 ||strcmp(rmap_latitude,"") == 0) { 
  if (!rmap_config(remote_config) == 0) {
    LOGN(F("station not configurated ! restart"CR));
    //LOGN(F("Reset wifi configuration"CR));
    //wifiManager.resetSettings();
    delay(1000);
    reboot();
  }


  /*
  //SDS011

  strcpy(sensors[0].driver,"SERI");
  strcpy(sensors[0].type,"SSD");
  sensors[0].address=1;

  for (int i = 0; i < SENSORS_LEN; i++) {
    sd[i]=SensorDriver::create(sensors[i].driver,sensors[i].type);
    if (sd[i] == NULL){
      LOGN(F("%s: driver not created !"CR),sensors[i].driver);
    }else{
      LOGN(F("%s: driver created"CR),sensors[i].driver);
      sd[i]->setup(sensors[i].driver,sensors[i].address);
    }
  }
  */
  
  /*
  Serial.print(F("Sds011 firmware version: "));
  LOGN(sensor.firmware_version().c_str());
  LOGN(CR);
    
  sensor.set_sleep(true);
  sensor.set_mode(sds011::QUERY);
  */

  LOGN(F("mqtt server: %s"CR),rmap_server);
  mqttclient.setServer(rmap_server, 1883);

  Alarm.timerRepeat(SAMPLETIME, repeats);             // timer for every tr seconds

  // millis() and other can have overflow problem
  // so we reset everythings one time a week
  Alarm.alarmRepeat(dowMonday,8,0,0,reboot);          // 8:00:00 every Monday

  // upgrade firmware
  Alarm.alarmRepeat(4,0,0,firmware_upgrade);          // 4:00:00 every day  
}


void loop() {
  Alarm.delay(0);
}
