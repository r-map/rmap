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

#define RESET_PIN D0
#define WIFI_SSED "STIMA-configuration"
#define WIFI_PASSWORD  "bellastima"
// increment on change
#define SOFTWARE_VERSION "2017-11-20T12:00"
#define SDS_PIN_RX D1
#define SDS_PIN_TX D2
#define SAMPLETIME 20


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

SoftwareSerial mySerial(SDS_PIN_RX, SDS_PIN_TX, false, 128);
sds011::Sds011 sensor(mySerial);
  
const char* update_host = "rmap.cc";
const char* update_url = "/firmware/update/luftdaten/";
const int update_port = 80;

bool config_needs_write = false;

WiFiClient espClient;
PubSubClient mqttclient(espClient);

//define your default values here, if there are different values in config.json, they are overwritten.
char rmap_longitude[11] = "";
char rmap_latitude[11] = "";
char rmap_server[40]= "rmap.cc";
char rmap_user[10]="";
char rmap_password[30]="";
char rmap_slug[50]="stimaesp";
char rmap_mqttrootpath[50] = "sample";
char rmap_mqttmaintpath[50] = "maint";

char esp_chipid[11];

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println(F("Should save config"));
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

  Serial.println(F("readRmapRemoteConfig url: "));  
  Serial.println(url);  
  //http.begin("http://rmap.cc/stations/pat1/luftdaten/json/");
  http.begin(url.c_str());

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) { //Check the returning code
    payload = http.getString();
    Serial.println(payload);
  }else{
    Serial.println(F("Error http: "));
    Serial.println(String(httpCode));
    Serial.println(http.errorToString(httpCode));
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
	    strncpy (rmap_mqttrootpath, array[i]["fields"]["mqttrootpath"].as< const char*>(),49);
	    rmap_mqttrootpath[49]='\0';
	    strncpy (rmap_mqttmaintpath, array[i]["fields"]["mqttmaintpath"].as< const char*>(),49);
	    rmap_mqttmaintpath[49]='\0';
	    strncpy (rmap_longitude, array[i]["fields"]["lon"].as< const char*>(),10);
	    rmap_longitude[10]='\0';
	    strncpy (rmap_latitude , array[i]["fields"]["lat"].as< const char*>(),10);
	    rmap_latitude[10]='\0';
	    Serial.println(F("lon: "));
	    Serial.println(rmap_longitude);
	    Serial.println(F("lat: "));
	    Serial.println(rmap_latitude);
	    config_needs_write = true;
	    Serial.println(F("station metadata found!"));
	    status = 0;
	  }
	}
      }
    } else {
      Serial.println(F("error decoding array"));
      status = 2;
    }
    return status;
  }
}


void publish_maint() {

  const String data;  
  
  if (!mqttclient.connected()) {
    //String clientId = "ESP8266Client-";
    //clientId += String(random(0xffff), HEX);
    mqttclient.setServer(rmap_server, 1883);

    char mainttopic[100]="";
    strcpy (mainttopic,rmap_mqttmaintpath);
    strcat(mainttopic,"/");
    strcat(mainttopic,rmap_user);
    strcat(mainttopic,"/");
    char longitude [10];
    char latitude [10];
    itoa (coordCharToInt(rmap_longitude),longitude,10);
    itoa (coordCharToInt(rmap_latitude),latitude,10);

    strcat(mainttopic,longitude);
    strcat(mainttopic,",");
    strcat(mainttopic,latitude);
    strcat (mainttopic,"/fixed/-,-,-/-,-,-,-/B01213");
    Serial.println(F("MQTT maint topic: "));
    Serial.println(mainttopic);
    
    Serial.println(F("Connet to mqtt broker"));
    if (mqttclient.connect(esp_chipid,rmap_user,rmap_password,mainttopic,1,1,"{\"v\":\"error01\"}")){
      Serial.println(F("MQTT connected"));

      if (!mqttclient.publish(mainttopic,(uint8_t*)"{\"v\":\"conn\"}", 12,1)){
	Serial.println(F("MQTT maint published"));
      }
    }else{
      Serial.println(F("Error connecting MQTT"));
    }
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
  
  Serial.println(sensor);
  if (strcmp(sensor,"SDS_PM10")== 0)
    {
      Serial.println(F("SDS_PM10 found"));
      strcat(topic,"B15195");
      havetopublish=true;
    }
  if (strcmp(sensor,"SDS_PM2")== 0)
    {
      Serial.println(F("SDS_PM2 found"));
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
      Serial.println(F("mqtt publish: "));
      Serial.println(topic);
      Serial.println(payload);
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
  Serial.println(F("version for firmware upgrade"));
  Serial.println(buffer);
		
  //		t_httpUpdate_return ret = ESPhttpUpdate.update(update_host, update_port, update_url, String(SOFTWARE_VERSION) + String(" ") + esp_chipid + String(" ") + SDS_version + String(" ") + String(current_lang) + String(" ") + String(INTL_LANG));
  t_httpUpdate_return ret = ESPhttpUpdate.update(update_host, update_port, update_url, String(buffer));
  switch(ret)
    {
    case HTTP_UPDATE_FAILED:
      Serial.println(F("[update] Update failed."));
      Serial.println(ESPhttpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println(F("[update] No Update."));
      break;
    case HTTP_UPDATE_OK:
      Serial.println(F("[update] Update ok.")); // may not called we reboot the ESP
      break;
    }
}

void readconfig() {

  Serial.println(F("mounted file system"));
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println(F("reading config file"));
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println(F("opened config file"));
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        StaticJsonBuffer<500> jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println(F("\nparsed json"));

          strcpy(rmap_longitude, json["rmap_longitude"]);
          strcpy(rmap_latitude, json["rmap_latitude"]);
          strcpy(rmap_server, json["rmap_server"]);
          strcpy(rmap_user, json["rmap_user"]);
          strcpy(rmap_password, json["rmap_password"]);
          strcpy(rmap_slug, json["rmap_slug"]);
	  strcpy(rmap_mqttrootpath, json["rmap_mqttrootpath"]);
	  strcpy(rmap_mqttmaintpath, json["rmap_mqttmaintpath"]);

	  Serial.println(F("loaded config parameter"));
	  Serial.println(rmap_server);
	  Serial.println(rmap_user);
	  Serial.println(rmap_password);
	  Serial.println(rmap_slug);
	  Serial.println(rmap_mqttrootpath);
	  Serial.println(rmap_mqttmaintpath);
	  
        } else {
          Serial.println(F("failed to load json config"));
        }
      } else {
	Serial.println(F("erro reading config file"));	
      }
    } else {
      Serial.println(F("config file do not exist"));
    }
  //end read
}

void writeconfig() {;

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println(F("saving config"));
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
      Serial.println(F("failed to open config file for writing"));
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println(F("saved config parameter"));
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

  if (ok) {
    publish_maint();
    publish_pm( "SDS_PM2", pm2);
    publish_pm( "SDS_PM10", pm10);
  } else {
    Serial.println(F("error getting sds011 data"));
  }

  sensor.set_sleep(true);
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
  pinMode(RESET_PIN, INPUT_PULLUP);

  itoa(ESP.getChipId(),esp_chipid,10);
  
  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println(F("mounting FS..."));
  if (SPIFFS.begin()) {
    readconfig();
  } else {
    Serial.println(F("failed to mount FS"));
  }
  
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_rmap_server("server", "rmap server", rmap_server, 40);
  WiFiManagerParameter custom_rmap_user("user", "rmap user", rmap_user, 10);
  WiFiManagerParameter custom_rmap_password("password", "rmap password", rmap_password, 30, "type = \"password\"");
  WiFiManagerParameter custom_rmap_slug("slug", "rmap station slug", rmap_slug, 50);

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
    Serial.println(F("Reset wifi configuration"));
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
    Serial.println(F("failed to connect and hit timeout"));
    delay(3000);
    reboot();
  }
  
  //if you get here you have connected to the WiFi
  Serial.println(F("connected...yeey :)"));

  //read updated parameters
  strcpy(rmap_server, custom_rmap_server.getValue());
  strcpy(rmap_user, custom_rmap_user.getValue());
  strcpy(rmap_password, custom_rmap_password.getValue());
  strcpy(rmap_slug, custom_rmap_slug.getValue());

  writeconfig();
  
  Serial.println(F("local ip"));
  Serial.println(WiFi.localIP());

  firmware_upgrade();

  if (!(rmap_remote_config() == 0)) {
    Serial.println(F("remote configuration failed"));
  }

  //strcpy(rmap_longitude,"11.36987");
  //strcpy(rmap_latitude,"44.48896");
    
  if (strcmp(rmap_longitude,"") == 0 ||strcmp(rmap_latitude,"") == 0) { 
    Serial.println(F("station not configurated ! restart"));
    //reset settings
    Serial.println(F("Reset wifi configuration"));
    wifiManager.resetSettings();
    delay(1000);
    reboot();
  }
  
  Serial.print(F("Sds011 firmware version: "));
  Serial.println(sensor.firmware_version());

  sensor.set_sleep(true);
  sensor.set_mode(sds011::QUERY);

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
