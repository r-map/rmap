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

// watchdog is enabled by default on ESP
// https://techtutorialsx.com/2017/01/21/esp8266-watchdog-functions/

SoftwareSerial mySerial(SDS_PIN_RX, SDS_PIN_TX, false, 128);
sds011::Sds011 sensor(mySerial);
  
const char* update_host = "rmap.cc";
const char* update_url = "/firmware/update/"FIRMWARE_TYPE"/";
const int update_port = 80;

bool config_needs_write = false;

WiFiClient espClient;
PubSubClient mqttclient(espClient);

//define your default values here, if there are different values in config.json, they are overwritten.
char rmap_longitude[11] = "";
char rmap_latitude[11] = "";
char rmap_server[41]= "rmap.cc";
char rmap_user[10]="";
char rmap_password[31]="";
char rmap_slug[31]="stimaesp";
char rmap_mqttrootpath[10] = "sample";
char rmap_mqttmaintpath[10] = "maint";


//flag for saving data
bool shouldSaveConfig = false;


/*

// for sensoron
#define SENSORS_LEN 5
#define SENSORDRIVER_DRIVER_LEN 5
#define SENSORDRIVER_TYPE_LEN 5
#define SENSORDRIVER_MQTTPATH_LEN 30
#define MAX_VALUES_FOR_SENSOR 5

#include <SensorDriver.h>

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
};

struct driver_t   // use this to instantiate a driver
{
  SensorDriver* manager;
  driver_t() : manager(NULL) {}

  int setup(const char* driver, int node, const char* type, int address
    #if defined (AES)
		       , uint8_t* key, uint8_t* iv
    #endif
	      )
  {
    if (manager != NULL)
      delete manager;
    manager = SensorDriver::create(driver,type);
    if (manager == NULL)
      return -2;

    if (manager->setup(driver, address, node, type
      #if defined (RADIORF24)
		       , mainbuf,sizeof(mainbuf), &network
        #if defined (AES)
		       , key, iv
        #endif
      #endif
		       ) != 0) return -1;
    return 0;
  }
} drivers[SENSORS_LEN];
*/


//callback notifying us of the need to save config
void saveConfigCallback () {
  Log.notice(F("Should save config"CR));
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


int  rmap_remote_config(){
  
  String payload;
  int status =0;
  
  HTTPClient http;
  // Make a HTTP request:

  String url="http://";
  url += rmap_server;
  url+="/stations/";
  url+=rmap_user;
  url+="/";
  url+=rmap_slug;
  url+="/json/";

  Log.notice(F("readRmapRemoteConfig url: "CR));  
  Log.notice(url.c_str());  
  //http.begin("http://rmap.cc/stations/pat1/luftdaten/json/");
  http.begin(url.c_str());

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) { //Check the returning code
    payload = http.getString();
    Log.notice(payload.c_str());
  }else{
    Log.error(F("Error http: %s"CR),String(httpCode).c_str());
    Log.error(F("Error http: %s"CR),http.errorToString(httpCode).c_str());
    status= 1;
  }
  http.end();					\

  if (status == 0) {
    StaticJsonBuffer<1500> jsonBuffer;
    status = 3;
    JsonArray& array = jsonBuffer.parseArray(payload);
    if (array.success()){
      for (int i = 0; i < array.size(); i++) {
	if  (array[i]["model"] == "stations.stationmetadata"){
	  if (array[i]["fields"]["active"]){
	    strncpy (rmap_mqttrootpath, array[i]["fields"]["mqttrootpath"].as< const char*>(),10);
	    rmap_mqttrootpath[9]='\0';
	    strncpy (rmap_mqttmaintpath, array[i]["fields"]["mqttmaintpath"].as< const char*>(),10);
	    rmap_mqttmaintpath[9]='\0';
	    strncpy (rmap_longitude, array[i]["fields"]["lon"].as< const char*>(),10);
	    rmap_longitude[10]='\0';
	    strncpy (rmap_latitude , array[i]["fields"]["lat"].as< const char*>(),10);
	    rmap_latitude[10]='\0';
	    Log.notice(F("lon: "));
	    Log.notice(rmap_longitude);
	    Log.notice(F("lat: "));
	    Log.notice(rmap_latitude);
	    Log.notice(CR);
	    config_needs_write = true;
	    Log.notice(F("station metadata found!"CR));
	    status = 0;
	  }
	}
      }
    } else {
      Log.error(F("error decoding array"CR));
      status = 2;
    }
    return status;
  }
}


bool publish_maint() {

  const String data;  
  
  if (!mqttclient.connected()) {
    //String clientId = "ESP8266Client-";
    //clientId += String(random(0xffff), HEX);
    
    Log.notice(F("Connet to mqtt broker"CR));

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
    strcat(mqttid,"/fixed");
    
    Log.notice(F("mqttid: %s"CR),mqttid);

    char mainttopic[100]="";
    strcpy (mainttopic,rmap_mqttmaintpath);
    strcat(mainttopic,"/");
    strcat(mainttopic,mqttid);
    strcat (mainttopic,"/-,-,-/-,-,-,-/B01213");
    Log.notice(F("MQTT maint topic: %s"CR),mainttopic);
    
    if (mqttclient.connect(mqttid,rmap_user,rmap_password,mainttopic,1,1,"{\"v\":\"error01\"}")){
      Log.notice(F("MQTT connected"CR));

      if (!mqttclient.publish(mainttopic,(uint8_t*)"{\"v\":\"conn\"}", 12,1)){
	Log.notice(F("MQTT maint published"CR));
      }
      return true;
    }else{
      Log.error(F("Error connecting MQTT"CR));
      return false;
    }
  } else {
    return true;
  }
}

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
  
  Log.notice(sensor);
  if (strcmp(sensor,"SDS_PM10")== 0)
    {
      Log.notice(F("SDS_PM10 found"CR));
      strcat(topic,"B15195");
      havetopublish=true;
    }
  if (strcmp(sensor,"SDS_PM2")== 0)
    {
      Log.notice(F("SDS_PM2 found"CR));
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
      Log.notice(F("mqtt publish: "));
      Log.notice(topic);
      Log.notice(payload);
      Log.notice(CR);
      mqttclient.publish(topic, payload);
    }
}


void firmware_upgrade() {

  StaticJsonBuffer<200> jsonBuffer; 
  JsonObject& root = jsonBuffer.createObject();
  root["ver"] = SOFTWARE_VERSION;
  root["user"] = rmap_user;
  root["slug"] = rmap_slug;
  char buffer[256];
  root.printTo(buffer, sizeof(buffer));
  Log.notice(F("version for firmware upgrade"CR));
  Log.notice(buffer);
  Log.notice(CR);
		
  //		t_httpUpdate_return ret = ESPhttpUpdate.update(update_host, update_port, update_url, String(SOFTWARE_VERSION) + String(" ") + esp_chipid + String(" ") + SDS_version + String(" ") + String(current_lang) + String(" ") + String(INTL_LANG));
  t_httpUpdate_return ret = ESPhttpUpdate.update(update_host, update_port, update_url, String(buffer));
  switch(ret)
    {
    case HTTP_UPDATE_FAILED:
      Log.error(F("[update] Update failed."CR));
      Log.error(ESPhttpUpdate.getLastErrorString().c_str());
      Log.notice(CR);
    break;
    case HTTP_UPDATE_NO_UPDATES:
      Log.notice(F("[update] No Update."CR));
      break;
    case HTTP_UPDATE_OK:
      Log.notice(F("[update] Update ok."CR)); // may not called we reboot the ESP
      break;
    }
}

void readconfig() {

  Log.notice(F("mounted file system"CR));
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Log.notice(F("reading config file"CR));
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Log.notice(F("opened config file"CR));
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        StaticJsonBuffer<500> jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Log.notice(F("\nparsed json"CR));

          strcpy(rmap_longitude, json["rmap_longitude"]);
          strcpy(rmap_latitude, json["rmap_latitude"]);
          strcpy(rmap_server, json["rmap_server"]);
          strcpy(rmap_user, json["rmap_user"]);
          strcpy(rmap_password, json["rmap_password"]);
          strcpy(rmap_slug, json["rmap_slug"]);
	  strcpy(rmap_mqttrootpath, json["rmap_mqttrootpath"]);
	  strcpy(rmap_mqttmaintpath, json["rmap_mqttmaintpath"]);

	  Log.notice(F("loaded config parameter"CR));
	  Log.notice(F("%s"CR),rmap_server);
	  Log.notice(F("%s"CR),rmap_user);
	  Log.notice(F("%s"CR),rmap_password);
	  Log.notice(F("%s"CR),rmap_slug);
	  Log.notice(F("%s"CR),rmap_mqttrootpath);
	  Log.notice(F("%s"CR),rmap_mqttmaintpath);
	  
        } else {
          Log.notice(F("failed to load json config"CR));
        }
      } else {
	Log.notice(F("erro reading config file"CR));	
      }
    } else {
      Log.notice(F("config file do not exist"CR));
    }
  //end read
}

void writeconfig() {;

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Log.notice(F("saving config"CR));
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    
    json["rmap_longitude"] = rmap_longitude;
    json["rmap_latitude"] = rmap_latitude;
    json["rmap_server"] = rmap_server;
    json["rmap_user"] = rmap_user;
    json["rmap_password"] = rmap_password;
    json["rmap_slug"] = rmap_slug;
    json["rmap_mqttrootpath"] = rmap_mqttrootpath;
    json["rmap_mqttmaintpath"] = rmap_mqttmaintpath;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Log.notice(F("failed to open config file for writing"CR));
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Log.notice(F("saved config parameter"CR));
    //end save
  }
}

void repeats() {

#define SDS_SAMPLES 3
  
  int pm2, pm10;
  bool ok;

  //firmware_upgrade();

  sensor.set_sleep(false);
  delay(3000);
  ok = sensor.query_data_auto(&pm2, &pm10, SDS_SAMPLES);
  sensor.set_sleep(true);
  if (!ok) {
    Log.error(F("error getting sds011 data"CR));
  }
  if (publish_maint()) {
    if (ok) {
      publish_pm( "SDS_PM2", pm2);
      publish_pm( "SDS_PM10", pm10);
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
  // Note: if you want to fully remove all logging code, uncomment #define DISABLE_LOGGING in Logging.h
  //       this will significantly reduce your project size
  
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  pinMode(RESET_PIN, INPUT_PULLUP);

  /*
  char esp_chipid[11];
  itoa(ESP.getChipId(),esp_chipid,10);
  Log.notice(F("esp_chipid: %s "CR),esp_chipid );
  */
  
  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Log.notice(F("mounting FS..."CR));
  if (SPIFFS.begin()) {
    readconfig();
  } else {
    Log.notice(F("failed to mount FS"CR));
  }
  
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_rmap_server("server", "rmap server", rmap_server, 40);
  WiFiManagerParameter custom_rmap_user("user", "rmap user", rmap_user, 10);
  WiFiManagerParameter custom_rmap_password("password", "rmap password", rmap_password, 30, "type = \"password\"");
  WiFiManagerParameter custom_rmap_slug("slug", "rmap station slug", rmap_slug, 30);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  
  //add all your parameters here
  wifiManager.addParameter(&custom_rmap_server);
  wifiManager.addParameter(&custom_rmap_user);
  wifiManager.addParameter(&custom_rmap_password);
  wifiManager.addParameter(&custom_rmap_slug);

  if (digitalRead(RESET_PIN) == LOW) {
    //reset settings
    Log.notice(F("Reset wifi configuration"CR));
    wifiManager.resetSettings();
  }
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
    Log.notice(F("failed to connect and hit timeout"CR));
    delay(3000);
    reboot();
  }
  
  //if you get here you have connected to the WiFi
  Log.notice(F("connected...yeey :)"CR));

  //read updated parameters
  strcpy(rmap_server, custom_rmap_server.getValue());
  strcpy(rmap_user, custom_rmap_user.getValue());
  strcpy(rmap_password, custom_rmap_password.getValue());
  strcpy(rmap_slug, custom_rmap_slug.getValue());

  writeconfig();
  
  Log.notice(F("local ip"));
  Log.notice(WiFi.localIP().toString().c_str());
  Log.notice(CR);

  firmware_upgrade();

  if (!(rmap_remote_config() == 0)) {
    Log.notice(F("remote configuration failed"CR));
  }

  if (strcmp(rmap_longitude,"") == 0 ||strcmp(rmap_latitude,"") == 0) { 
    Log.notice(F("station not configurated ! restart"CR));
    //reset settings
    Log.notice(F("Reset wifi configuration"CR));
    wifiManager.resetSettings();
    delay(1000);
    reboot();
  }
  
  Serial.print(F("Sds011 firmware version: "));
  Log.notice(sensor.firmware_version().c_str());
  Log.notice(CR);

  sensor.set_sleep(true);
  sensor.set_mode(sds011::QUERY);

  Log.notice(F("mqtt server: %s"CR),rmap_server);
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
