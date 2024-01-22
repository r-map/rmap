/*
Copyright (C) 2024  Paolo Patruno <p.patruno@iperbole.bologna.it>
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
TODO PORTING TO ESP32 WeMos-D1-Mini-ESP32
https://www.dropbox.com/s/4phxfx75hje5eu4/Board-de-desarrollo-WeMos-D1-Mini-ESP32-WiFiBluetooth-BLE-Pines.jpg
https://cdn.shopify.com/s/files/1/1509/1638/files/D1_Mini_ESP32_-_pinout.pdf

* implementing OTA firmware updater for ESP32 (https): now I use a old simple porting of ESP8266httpUpdate
* change second serial port from software to hardware (but it seems implemented in not wemos connector)
* check if LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED) default to do not autoformat littlefs
* wait for new LittleFS release for esp8266 API compatibility  https://github.com/espressif/arduino-esp32/pull/5396
*/

#include "stimawifi.h"
#include "esp_task_wdt.h"

sensor_t  sensors[SENSORS_LEN];
SensorDriver* sd[SENSORS_LEN];

const char* update_url = "/firmware/update/" FIRMWARE_TYPE "/";
const uint16_t update_port = 80;

WiFiManager wifiManager;
WebServer webserver(STIMAHTTP_PORT);

WiFiClient espClient;
PubSubClient mqttclient(espClient);
//WebSocketsServer webSocket(WS_PORT);
//EspHtmlTemplateProcessor templateProcessor(&server);
WiFiUDP UDP;
OZGPS gps;
MGPS mgps;
MutexStandard loggingmutex;

//flag for saving data
bool shouldSaveConfig = false;
bool pmspresent =  false;

//define your default values here, if there are different values in config.json, they are overwritten.
char rmap_longitude[11] = "";
char rmap_latitude[11] = "";
char rmap_network[31] = "";
char rmap_server[41] = "rmap.cc";
char ntp_server[41] = "europe.pool.ntp.org";
char rmap_mqtt_server[41] = "rmap.cc";
int  rmap_sampletime = DEFAULT_SAMPLETIME;
char rmap_user[10] = "";
char rmap_password[31] = "";
char rmap_slug[31] = "stimawifi";
char rmap_mqttrootpath[10] = "sample";
char rmap_mqttmaintpath[10] = "maint";

U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0);
bool oledpresent=false;
unsigned short int displaypos;

// i2c button for wemos OLED version 2.1.0
I2C_BUTTON button; //I2C address 0x31
// I2C_BUTTON button(DEFAULT_I2C_BUTTON_ADDRESS); //I2C address 0x31
// I2C_BUTTON button(your_address); //using customize I2C address

float temperature=NAN;
int humidity=-999,pm2=-999,pm10=-999,co2=-999;

udp_data_t udp_data={1,frtosLog};
udpThread threadUdp(udp_data);

Queue mqttQueue(10,sizeof(mqttMessage_t));

measure_data_t measure_data={1,frtosLog,mqttQueue};
measureThread threadMeasure(measure_data);

publish_data_t publish_data={1,frtosLog,mqttQueue};
publishThread threadPublish(publish_data);


void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    frtosLog.error("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%y %m %d  %H:%M:%S");
}

// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t)
{
  frtosLog.notice("Got time adjustment from NTP!");
  time_t tnow;
  time(&tnow);
  setTime(tnow);              // resync from sntp
  esp_task_wdt_reset();
  printLocalTime();
}

void analogWriteFreq(const double frequency){
  //analogWriteFrequency(frequency);
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
  frtosLog.notice("Should save config");
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

  frtosLog.notice(F("readRmapRemoteConfig url: %s"),url.c_str());  
  http.begin(espClient,url.c_str());

  esp_task_wdt_reset();

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) { //Check the returning code
    payload = http.getString();
    frtosLog.notice(payload.c_str());
  }else{
    frtosLog.error(F("Error http: %s"),String(httpCode).c_str());
    frtosLog.error(F("Error http: %s"),http.errorToString(httpCode).c_str());
    payload=String();
  }
  esp_task_wdt_reset();
  http.end();
  esp_task_wdt_reset();
  return payload;
}


void firmware_upgrade() {

  StaticJsonDocument<200> doc; 
  doc["ver"] = SOFTWARE_VERSION;
  doc["user"] = rmap_user;
  doc["slug"] = rmap_slug;
  char buffer[256];
  serializeJson(doc, buffer, sizeof(buffer));
  frtosLog.notice(F("url for firmware update: %s"),update_url);
  frtosLog.notice(F("version for firmware update: %s"),buffer);

  analogWriteFreq(4);
  analogWrite(LED_PIN,512);  

  //		t_httpUpdate_return ret = ESPhttpUpdate.update(update_host, update_port, update_url, String(SOFTWARE_VERSION) + String(" ") + esp_chipid + String(" ") + SDS_version + String(" ") + String(current_lang) + String(" ") + String(INTL_LANG));
  t_httpUpdate_return ret = ESPhttpUpdate.update(String(rmap_server), update_port, String(update_url), String(buffer));

  switch(ret)
    {
    case HTTP_UPDATE_FAILED:
      frtosLog.error(F("[update] Update failed with message:"));
      frtosLog.error(F("%s"),ESPhttpUpdate.getLastErrorString().c_str());
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
      frtosLog.notice(F("[update] No Update."));
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
      frtosLog.notice(F("[update] Update ok.")); // may not called we reboot the ESP
      
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

      break;
    }

  //#endif

  analogWriteFreq(1);
  digitalWrite(LED_PIN,HIGH);
}

String readconfig_rmap() {

    if (LittleFS.exists("/rmap.json")) {
      //file exists, reading and loading
    frtosLog.notice(F("reading rmap config file"));
    File configFile = LittleFS.open("/rmap.json", "r");
    if (configFile) {
      frtosLog.notice(F("opened rmap config file"));

      //size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      //std::unique_ptr<char[]> buf(new char[size]);
      //configfile.readBytes(buf.get(), size);

      return configFile.readString();
      
    } else {
      frtosLog.notice(F("erro reading rmap file"));	
    }
  } else {
    frtosLog.notice(F("rmap file do not exist"));
  }
  //end read
  return String();  
}

void writeconfig_rmap(const String payload) {;

  //save the custom parameters to FS
  frtosLog.notice(F("saving rmap config"));
  
  File configFile = LittleFS.open("/rmap.json", "w");
  if (!configFile) {
    frtosLog.error(F("failed to open rmap config file for writing"));
  }

  configFile.print(payload);
  configFile.close();
  frtosLog.notice(F("saved rmap config parameter"));
  //end save
}

int  rmap_config(const String payload){

  bool status_station = false;
  bool status_board_mqtt = false;
  bool status_board_tcpip = false;
  bool status_sensors = false;
  int status = 0;
  int ii = 0;

  if (! (payload == String())) {
    DynamicJsonDocument doc(4000);
    status = 3;
    DeserializationError error = deserializeJson(doc,payload);
    if (!error){
      JsonArrayConst array = doc.as<JsonArray>();
      frtosLog.notice(F("array: %d"),array.size());
      //for (uint8_t i = 0; i < array.size(); i++) {
      for(JsonObjectConst element: array){
	
	if  (element["model"] == "stations.stationmetadata"){
	  if (element["fields"]["active"]){
	    frtosLog.notice(F("station metadata found!"));
	    strncpy (rmap_mqttrootpath, element["fields"]["mqttrootpath"].as< const char*>(),9);
	    rmap_mqttrootpath[9]='\0';
	    frtosLog.notice(F("mqttrootpath: %s"),rmap_mqttrootpath);
	    strncpy (rmap_mqttmaintpath, element["fields"]["mqttmaintpath"].as< const char*>(),9);
	    rmap_mqttmaintpath[9]='\0';
	    frtosLog.notice(F("mqttmaintpath: %s"),rmap_mqttmaintpath);

	    //strncpy (rmap_longitude, element["fields"]["lon"].as<const char*>(),10);
	    //rmap_longitude[10]='\0';
	    itoa(int(element["fields"]["lon"].as<float>()*100000),rmap_longitude,10);
	    frtosLog.notice(F("lon: %s"),rmap_longitude);

	    //strncpy (rmap_latitude , element["fields"]["lat"].as<const char*>(),10);
	    //rmap_latitude[10]='\0';
	    itoa(int(element["fields"]["lat"].as<float>()*100000),rmap_latitude,10);
	    frtosLog.notice(F("lat: %s"),rmap_latitude);
	    
	    strncpy (rmap_network , element["fields"]["network"].as< const char*>(),30);
	    rmap_network[30]='\0';
	    frtosLog.notice(F("network: %s"),rmap_network);

	    strncpy (rmap_mqttrootpath , element["fields"]["mqttrootpath"].as< const char*>(),9);
	    rmap_mqttrootpath[9]='\0';
	    frtosLog.notice(F("rmap_mqttrootpath: %s"),rmap_mqttrootpath);

	    strncpy (rmap_mqttmaintpath , element["fields"]["mqttmaintpath"].as< const char*>(),9);
	    rmap_mqttmaintpath[9]='\0';
	    frtosLog.notice(F("rmap_mqttmaintpath: %s"),rmap_mqttmaintpath);

	    status_station = true;
	  }
	}

	if  (element["model"] == "stations.transportmqtt"){
	  if (element["fields"]["board"][0] == "default"){
	    if (element["fields"]["active"]){
	      frtosLog.notice(F("board transportmqtt found!"));
	      rmap_sampletime=element["fields"]["mqttsampletime"];
	      frtosLog.notice(F("rmap_sampletime: %d"),rmap_sampletime);

	      if (!element["fields"]["mqttserver"].isNull()){
		strncpy (rmap_mqtt_server, element["fields"]["mqttserver"].as< const char*>(),40);
		rmap_mqtt_server[40]='\0';
		frtosLog.notice(F("rmap_mqtt_server: %s"),rmap_mqtt_server);
	      }
	      
	      if (!element["fields"]["mqttuser"].isNull()){
		strncpy (rmap_user, element["fields"]["mqttuser"].as< const char*>(),9);
		rmap_user[9]='\0';
		frtosLog.notice(F("rmap_user: %s"),rmap_user);
	      }

	      status_board_mqtt = true;
	    }
	  }
	}

	if  (element["model"] == "stations.transporttcpip"){
	  if (element["fields"]["board"][0] == "default"){
	    if (element["fields"]["active"]){
	      frtosLog.notice(F("board transporttcpip found!"));

	      if (!element["fields"]["ntpserver"].isNull()){
		strncpy (ntp_server, element["fields"]["ntpserver"].as< const char*>(),40);
		ntp_server[40]='\0';
		frtosLog.notice(F("ntp_server: %s"),ntp_server);
	      }
	      status_board_tcpip = true;	      
	    }
	  }
	}

	
	if  (element["model"] == "stations.sensor"){
	  if (element["fields"]["active"]){
	    if (ii < SENSORS_LEN) {
	      frtosLog.notice(F("station sensor found!"));
	      strncpy (sensors[ii].driver , element["fields"]["driver"].as< const char*>(),SENSORDRIVER_DRIVER_LEN);
	      frtosLog.notice(F("driver: %s"),sensors[ii].driver);
	      strncpy (sensors[ii].type , element["fields"]["type"][0].as< const char*>(),SENSORDRIVER_TYPE_LEN);
	      frtosLog.notice(F("type: %s"),sensors[ii].type);
	      strncpy (sensors[ii].timerange, element["fields"]["timerange"].as< const char*>(),SENSORDRIVER_META_LEN);
	      frtosLog.notice(F("timerange: %s"),sensors[ii].timerange);
	      strncpy (sensors[ii].level, element["fields"]["level"].as< const char*>(),SENSORDRIVER_META_LEN);
	      frtosLog.notice(F("level: %s"),sensors[ii].level);
	      sensors[ii].address = element["fields"]["address"];	    
	      frtosLog.notice(F("address: %d"),sensors[ii].address);

	      if (strcmp(sensors[ii].type,"PMS")==0) pmspresent=true;
	      
	      sd[ii]=SensorDriver::create(sensors[ii].driver,sensors[ii].type);
	      if (sd[ii] == 0){
		frtosLog.error(F("%s:%s driver not created !"),sensors[ii].driver,sensors[ii].type);
	      }else{		
		if (!(sd[ii]->setup(sensors[ii].driver, sensors[ii].address, -1, sensors[ii].type) == SD_SUCCESS)) {
		  frtosLog.error(F("sensor not present or broken"));
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
      frtosLog.error(F("error parsing array: %s"),error.c_str());
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

  if (LittleFS.exists("/config.json")) {
    //file exists, reading and loading
    frtosLog.notice(F("reading config file"));
    File configFile = LittleFS.open("/config.json", "r");
    if (configFile) {
      frtosLog.notice(F("opened config file"));
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
	  
	  frtosLog.notice(F("loaded config parameter:"));
	  frtosLog.notice(F("longitude: %s"),rmap_longitude);
	  frtosLog.notice(F("latitude: %s"),rmap_latitude);
	  frtosLog.notice(F("server: %s"),rmap_server);
	  frtosLog.notice(F("ntp server: %s"),ntp_server);
	  frtosLog.notice(F("mqtt server: %s"),rmap_mqtt_server);
	  frtosLog.notice(F("user: %s"),rmap_user);
	  //frtosLog.notice(F("password: %s"),rmap_password);
	  frtosLog.notice(F("slug: %s"),rmap_slug);
	  frtosLog.notice(F("mqttrootpath: %s"),rmap_mqttrootpath);
	  frtosLog.notice(F("mqttmaintpath: %s"),rmap_mqttmaintpath);
	  
        } else {
          frtosLog.error(F("failed to deserialize json config %s"),error.c_str());
        }
      } else {
	frtosLog.error(F("erro reading config file"));	
      }
    } else {
      frtosLog.warning(F("config file do not exist"));
    }
  //end read
}

void writeconfig() {;

  //save the custom parameters to FS
  frtosLog.notice(F("saving config"));
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
    frtosLog.error(F("failed to open config file for writing"));
  }

  //json.printTo(Serial);
  serializeJson(json,configFile);
  configFile.close();
  frtosLog.notice(F("saved config parameter"));
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

void measureAndPublish() {
  threadMeasure.Notify();
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

  /*
  #include "soc/soc.h"
  #include "soc/rtc_cntl_reg.h"
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable Brownout detector
  */
  
  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  analogWriteFreq(1);
  digitalWrite(LED_PIN,HIGH);
  pinMode(PMS_RESET, OUTPUT);
  //reset pin for sensor
  digitalWrite(PMS_RESET,LOW); // reset low
  delay(500);
  digitalWrite(PMS_RESET,HIGH);

  //Serial.setTxTimeoutMs(0);  // https://github.com/espressif/arduino-esp32/issues/6983
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
 
  // Pass log level, whether to show log level, and print interface.
  // Available levels are:
  // LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
  // Note: if you want to fully remove all logging code, change #define LOG_LEVEL ....
  //       this will significantly reduce your project size

  // set runtime log level to the same of compile time
  frtosLog.begin(LOG_LEVEL, &Serial,loggingmutex);
  frtosLog.setPrefix(logPrefix); // Uncomment to get timestamps as prefix
  frtosLog.setSuffix(logSuffix); // Uncomment to get newline as suffix
  frtosLog.notice(F("Started"));
  frtosLog.notice(F("Version: " SOFTWARE_VERSION));

  espClient.setTimeout(5000); // esp32 issue https://github.com/espressif/arduino-esp32/issues/3732
  
  Wire.begin();
  //Wire.begin(SDA_PIN,SCL_PIN);
  Wire.setClock(I2C_CLOCK);

  // check return value of
  // the Write.endTransmisstion to see if
  // a device did acknowledge to the address.
  Wire.beginTransmission(OLEDI2CADDRESS);
  if (Wire.endTransmission() == 0) {
    frtosLog.notice(F("OLED Found"));
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
        frtosLog.notice(F("OLED NOT Found"));
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
  frtosLog.notice(F("esp_chipid: %s "),esp_chipid );
  */

  // manage reset button in hardware (RESET_PIN) or in software (I2C)
  bool reset=digitalRead(RESET_PIN) == LOW;
  if (button.get() == 0)
  {
    if (button.BUTTON_A)
    {
      //String keyString[] = {"None", "Press", "Long Press", "Double Press", "Hold"};
      //frtosLog.notice(F("BUTTON A: %s"),keyString[button.BUTTON_A].c_str());
      if (button.BUTTON_A == KEY_VALUE_HOLD) reset=true;
    }
  }
  
  if (reset) {
    frtosLog.notice(F("clean FS"));
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
    frtosLog.notice(F("Reset wifi configuration"));
    wifiManager.resetSettings();
  }
  
  //read configuration from FS json
  frtosLog.notice(F("mounting FS..."));
  if (LittleFS.begin()) {
    frtosLog.notice(F("mounted LittleFS file system"));
    readconfig();    
  } else {
    frtosLog.error(F("failed to mount FS"));
    frtosLog.warning(F("Reformat LittleFS"));
    LittleFS.begin();    
    LittleFS.format();
    frtosLog.warning(F("Reset wifi configuration"));
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
    frtosLog.notice(F("station configuration not found!"));
    frtosLog.notice(F("Reset wifi configuration"));
    wifiManager.resetSettings();
  }

  //sntp_init();
  //sntp_setoperatingmode(SNTP_OPMODE_POLL);
  //sntp_setservername(0, ntp_server);
  // set notification call-back function
  sntp_set_time_sync_notification_cb( timeavailable );
  sntp_servermode_dhcp(1);
  configTime(0, 0, ntp_server);

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_rmap_server("server", "rmap server", rmap_server, 41);
  WiFiManagerParameter custom_rmap_user("user", "rmap user", rmap_user, 10);
  WiFiManagerParameter custom_rmap_password("password", "station password", rmap_password, 31, "type = \"password\"");
  WiFiManagerParameter custom_rmap_slug("slug", "rmap station slug", rmap_slug, 31);

  //add all your parameters here
  wifiManager.addParameter(&custom_rmap_server);
  wifiManager.addParameter(&custom_rmap_user);
  wifiManager.addParameter(&custom_rmap_password);
  wifiManager.addParameter(&custom_rmap_slug);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  
  //set minimum quality of signal so it ignores AP's under that quality
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

  //analogWrite(LED_PIN,512);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.setDebugOutput(true);
  if (!wifiManager.autoConnect(WIFI_SSED,WIFI_PASSWORD)) {
    frtosLog.error(F("failed to connect and hit timeout"));
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
    frtosLog.notice(F("connected... good!"));
    frtosLog.notice(F("local ip: %s"),WiFi.localIP().toString().c_str());
    digitalWrite(LED_PIN,HIGH);

    
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
    
  }
  
  /*
  WiFi.begin("pat1", "comodinacomodino");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  */
  esp_task_wdt_reset();
  
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
      esp_task_wdt_reset();
    }
  }
  
  esp_task_wdt_reset();
  String remote_config= rmap_get_remote_config();
  esp_task_wdt_reset();

  if ( remote_config == String() ) {
    frtosLog.error(F("remote configuration failed"));
    //analogWrite(LED_PIN,50);
    //delay(5000);
    //digitalWrite(LED_PIN,HIGH);    
    remote_config=readconfig_rmap();
  }else{
    writeconfig_rmap(remote_config);
  }

  esp_task_wdt_reset();
  firmware_upgrade();
  esp_task_wdt_reset();
  
  if (!rmap_config(remote_config) == 0) {
    frtosLog.notice(F("station not configurated ! restart"));
    //frtosLog.notice(F("Reset wifi configuration"));
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
    frtosLog.error(F("Error setting up MDNS responder!"));
    delay(1000);
  }
  frtosLog.notice(F("mDNS responder started"));

  esp_task_wdt_reset();

  // setup web server
  webserver.on("/", handle_FullPage);
  webserver.on("/data", handle_Data);
  webserver.on("/data.json", handle_Json);
  webserver.onNotFound(handle_NotFound);
  
  webserver.begin();
  frtosLog.notice(F("HTTP server started"));

  if (oledpresent){
    u8g2.setCursor(0, 30);
    u8g2.print(F("Setting time"));
    u8g2.sendBuffer();
  }
  
  //if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000)) != ESP_OK) {
  //  frtosLog.error("Failed to update system time within 30s timeout");
  //}
  
  printLocalTime();
  
  //sntp_init();
  // wait for time to be set
  /*
  time_t now = 0;
  struct tm timeinfo = { 0 };
  int retry = 0;
  const int retry_count = 10;
  while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
    frtosLog.notice(F("Waiting for system time to be set... (%d/%d)"), retry, retry_count);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    time(&now);
    localtime_r(&now, &timeinfo);
  }
  
  if(retry >= retry_count) {
    if (oledpresent){
      u8g2.clearBuffer();
      u8g2.setCursor(0, 10); 
      u8g2.print(F("Time not"));
      u8g2.setCursor(0, 20); 
      u8g2.print(F("configurated!"));
      u8g2.setCursor(0, 30);
      u8g2.print(F("RESTART"));
      u8g2.sendBuffer();
      delay(5000);
    }
    frtosLog.error(F("NTP time out: Time not configurated, REBOOT"));
    delay(1000);
    reboot(); //300 seconds timeout - reset board
  }
  */

  time_t datetime = now();
  frtosLog.notice(F("Time: %s"),ctime(&datetime));

  frtosLog.notice(F("mqtt server: %s"),rmap_mqtt_server);

  mqttclient.setServer(rmap_mqtt_server, 1883);
  
  Alarm.timerRepeat(rmap_sampletime, measureAndPublish);             // timer for every SAMPLETIME seconds

  // millis() and other can have overflow problem
  // so we reset everythings one time a week
  //Alarm.alarmRepeat(dowMonday,8,0,0,reboot);          // 8:00:00 every Monday
  time_t reboottime;
  if (pmspresent){
    reboottime=3600*24;            // pms stall sometime
  }else{
    reboottime=3600*24*7;          // every week
  }
  frtosLog.notice(F("reboot every: %d"),reboottime);
  Alarm.timerRepeat(reboottime,reboot);                 // reboot

  // upgrade firmware
  //Alarm.alarmRepeat(4,0,0,firmware_upgrade);          // 4:00:00 every day  
  Alarm.timerRepeat(3600*24,firmware_upgrade);          // every day  

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", STIMAHTTP_PORT);

  // Begin listening to UDP port
  UDP.begin(UDP_PORT);
  frtosLog.notice(F("Listening on UDP port %d"),UDP_PORT);
  //UDP.stop();
  //frtosLog.notice(F("Stop listening on UDP port %d"),UDP_PORT);
  
  gps.init(&mgps);
  gps.set_filter(0xE); // "RMC","GGA","GLL"

  threadUdp.Start();
  threadMeasure.Start();
  threadPublish.Start();

}
