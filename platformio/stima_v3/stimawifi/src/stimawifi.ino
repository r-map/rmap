/*
Copyright (C) 2025  Paolo Patruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
TODO
* manage remote procedure call when it will be required
*/

/*
documentation at
https://doc.rmap.cc/stima_wifi/stimawifi_v3/stima_wifi_howto.html#software
*/

#include "stimawifi.h"

/*
*  Print last reset reason of ESP32
*  =================================
*
*  Use either of the methods print_reset_reason
*  or verbose_print_reset_reason to display the
*  cause for the last reset of this device.
*
*  Public Domain License.
*
*  Author:
*  Evandro Luis Copercini - 2017
*/

void print_reset_reason(int reason) {
  switch (reason) {
    case 1:  Serial.println("POWERON_RESET"); break;          /**<1,  Vbat power on reset*/
    case 3:  Serial.println("SW_RESET"); break;               /**<3,  Software reset digital core*/
    case 4:  Serial.println("OWDT_RESET"); break;             /**<4,  Legacy watch dog reset digital core*/
    case 5:  Serial.println("DEEPSLEEP_RESET"); break;        /**<5,  Deep Sleep reset digital core*/
    case 6:  Serial.println("SDIO_RESET"); break;             /**<6,  Reset by SLC module, reset digital core*/
    case 7:  Serial.println("TG0WDT_SYS_RESET"); break;       /**<7,  Timer Group0 Watch dog reset digital core*/
    case 8:  Serial.println("TG1WDT_SYS_RESET"); break;       /**<8,  Timer Group1 Watch dog reset digital core*/
    case 9:  Serial.println("RTCWDT_SYS_RESET"); break;       /**<9,  RTC Watch dog Reset digital core*/
    case 10: Serial.println("INTRUSION_RESET"); break;        /**<10, Instrusion tested to reset CPU*/
    case 11: Serial.println("TGWDT_CPU_RESET"); break;        /**<11, Time Group reset CPU*/
    case 12: Serial.println("SW_CPU_RESET"); break;           /**<12, Software reset CPU*/
    case 13: Serial.println("RTCWDT_CPU_RESET"); break;       /**<13, RTC Watch dog Reset CPU*/
    case 14: Serial.println("EXT_CPU_RESET"); break;          /**<14, for APP CPU, reset by PRO CPU*/
    case 15: Serial.println("RTCWDT_BROWN_OUT_RESET"); break; /**<15, Reset when the vdd voltage is not stable*/
    case 16: Serial.println("RTCWDT_RTC_RESET"); break;       /**<16, RTC Watch dog reset digital core and rtc module*/
    default: Serial.println("NO_MEAN");
  }
}

void   verbose_print_reset_reason(int reason) {
  switch (reason) {
    case 1:  Serial.println("Vbat power on reset"); break;
    case 3:  Serial.println("Software reset digital core"); break;
    case 4:  Serial.println("Legacy watch dog reset digital core"); break;
    case 5:  Serial.println("Deep Sleep reset digital core"); break;
    case 6:  Serial.println("Reset by SLC module, reset digital core"); break;
    case 7:  Serial.println("Timer Group0 Watch dog reset digital core"); break;
    case 8:  Serial.println("Timer Group1 Watch dog reset digital core"); break;
    case 9:  Serial.println("RTC Watch dog Reset digital core"); break;
    case 10: Serial.println("Instrusion tested to reset CPU"); break;
    case 11: Serial.println("Time Group reset CPU"); break;
    case 12: Serial.println("Software reset CPU"); break;
    case 13: Serial.println("RTC Watch dog Reset CPU"); break;
    case 14: Serial.println("for APP CPU, reset by PRO CPU"); break;
    case 15: Serial.println("Reset when the vdd voltage is not stable"); break;
    case 16: Serial.println("RTC Watch dog reset digital core and rtc module"); break;
    default: Serial.println("NO_MEAN");
  }  
}

void set_status_summary(int reason) {

  stimawifiStatus.summary.err_power_on=false;
  stimawifiStatus.summary.err_reboot=false;
  stimawifiStatus.summary.err_georef=false;
  stimawifiStatus.summary.err_db=false;
  stimawifiStatus.summary.err_mqtt_publish=false;
  stimawifiStatus.summary.err_mqtt_connect=false;
  stimawifiStatus.summary.err_geodef=false;
  stimawifiStatus.summary.err_sensor=false;
  stimawifiStatus.summary.err_novalue=false;

  switch (reason) {
    case 1:  stimawifiStatus.summary.err_power_on=true; break;
    default: stimawifiStatus.summary.err_reboot=true;
  }
}

// show some measure (fixed selection) on display in a fixed position
// display a sintetic status too
void display_summary_data(char* status) {
  
  DynamicJsonDocument doc(500);

  frtosLog.notice(F("display_values"));

  uint8_t displaypos=1;

  LockGuard guard(i2cmutex);
  u8g2->clearBuffer();

  //u8g2->setCursor(0, 1*CH); 
  //u8g2->setDrawColor(0);
  //u8g2->drawBox( 0, 0*CH,64, CH);
  //u8g2->setDrawColor(1);
  
  u8g2->setCursor(0, (displaypos++)*CH); 
  u8g2->print(status);

  frtosLog.notice(F("display Temperature: %D"),summarydata.temperature);
  u8g2->setCursor(0, (displaypos++)*CH); 
  u8g2->print(F("T   : "));
  u8g2->print(summarydata.temperature,1);
  u8g2->print(F(" C"));

  frtosLog.notice(F("display Humidity: %d"),summarydata.humidity);
  u8g2->setCursor(0, (displaypos++)*CH); 
  u8g2->print(F("U   : "));
  u8g2->print(summarydata.humidity,0);
  u8g2->print(F(" %"));

  frtosLog.notice(F("display PM2: %d"),summarydata.pm2);
  u8g2->setCursor(0, (displaypos++)*CH); 
  u8g2->print(F("PM2 : "));
  u8g2->print(summarydata.pm2,0);
  u8g2->print(F(" ug/m3"));

  frtosLog.notice(F("display PM10: %d"),summarydata.pm10);
  u8g2->setCursor(0, (displaypos++)*CH); 
  u8g2->print(F("PM10: "));
  u8g2->print(summarydata.pm10,0);
  u8g2->print(F(" ug/m3"));

  frtosLog.notice(F("display CO2: %d"),summarydata.co2);
  u8g2->setCursor(0, (displaypos++)*CH); 
  u8g2->print(F("CO2 : "));
  u8g2->print(summarydata.co2,0);
  u8g2->print(F(" ppm"));

  /*
  u8g2->clearBuffer();
  u8g2->setCursor(0, 20); 
  u8g2->print(F("WIFI KO"));
  u8g2->sendBuffer();
  */
  u8g2->sendBuffer(); 
}

// Print date and time to loggin system
void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    frtosLog.error("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%y %m %d  %H:%M:%S");
}

// intermediate function to avoid static member in class 
time_t rtc_set_time(){
  return frtosRTC.get();
}

// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t)
{
  frtosLog.notice("Got time from NTP!");
  time_t tnow;
  time(&tnow);
  setTime(tnow);              // resync from sntp
  printLocalTime();

  if (oledpresent){
    LockGuard guard(i2cmutex);
    u8g2->setCursor(0, 3*CH);
    u8g2->print(F("           "));
    u8g2->print(F("Time OK"));
    u8g2->sendBuffer();
  }  
  if (!frtosRTC.set(now())){
    frtosLog.error("Setting RTC time from NTP!");
    stimawifiStatus.rtc=error;
  }else{
    stimawifiStatus.rtc=ok;
  }
}

// HTTP response with json data
// data is a fixed selection of total dataset read from sensors and sintetic status
// key are:
// TEMP
// HUMID
// PM2
// PM10
// CO2
// STAT

String Json(){

  String str ="{"
    "\"TEMP\":\"";
  str +=summarydata.temperature;
  str +="\","
    "\"HUMID\":\"";
  str +=summarydata.humidity;
  str +="\","
    "\"PM2\":\"";
  str +=summarydata.pm2;
  str +="\","
    "\"PM10\":\"";
  str +=summarydata.pm10;
  str +="\","
    "\"CO2\":\"";
  str +=summarydata.co2;
  str +="\","
    "\"STAT\":\"";
  str +=status;
  str +="\"}";

  return str;
}

// HTTP response for browser in smartphone
// The browser get a page from server, query the phone for geolocation,
// send coordinate with an ajax request to ESP
// everything do not work for a mixed protocol not admitted by browser (http/https)
// getting coordinate from phone is enabled by https only protocol
// ESP support http only protocol (not full true...  but not easy)
String Geo(){
  if (webserver.method() == HTTP_GET){
    frtosLog.notice(F("geo get N arguments: %d"),webserver.args());
    georef.mutex->Lock();
    for (uint8_t i = 0; i < webserver.args(); i++) {
      frtosLog.notice(F("geo get argument %s: %s"),webserver.argName(i),webserver.arg(i));
      if (strcmp(webserver.argName(i).c_str(),"lat") == 0) strcpy(georef.lat,webserver.arg(i).c_str());
      if (strcmp(webserver.argName(i).c_str(),"lon") == 0) strcpy(georef.lon,webserver.arg(i).c_str());
    }
    georef.timestamp=now();
    georef.mutex->Unlock();

    return "OK";
  }
  return "KO";
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
  str +=summarydata.temperature;
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
  str +=summarydata.humidity;
  str +="<span class=\"superscript\">%</span></div>\n"
    "</div>\n"
  
    "<div class=\"data\">\n"
    "<div class=\"side-by-side temperature-text\">PM2.5</div>\n"
    "<div class=\"side-by-side temperature\">";
  str +=summarydata.pm2;
  str +="<span class=\"superscript\">ug/m3</span></div>\n"
    "</div>\n"

    "<div class=\"data\">\n"
    "<div class=\"side-by-side temperature-text\">PM10</div>\n"
    "<div class=\"side-by-side temperature\">";
  str +=summarydata.pm10;
  str +="<span class=\"superscript\">ug/m3</span></div>\n"
    "</div>\n"
    
    "<div class=\"data\">\n"
    "<div class=\"side-by-side temperature-text\">CO2</div>\n"
    "<div class=\"side-by-side temperature\">";
  str +=summarydata.co2;
  str +="<span class=\"superscript\">ppm</span></div>\n"
    "</div>\n";

  return str;
}

// function to prepare HTML response main page
// https://lastminuteengineers.com/esp8266-dht11-dht22-web-server-tutorial/
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


// web server response callback function
void handle_FullPage() {
  webserver.send(200, "text/html", FullPage()); 
}

// web server response callback function
void handle_Data() {
  webserver.send(200, "text/html", Data()); 
}

// web server response callback function
void handle_Json() {
  webserver.sendHeader("Access-Control-Allow-Origin", "*", true);
  webserver.sendHeader("Access-Control-Allow-Methods", "*", true);
  webserver.send(200, "application/json", Json()); 
}

// web server response callback function
void handle_Geo() {
  webserver.sendHeader("Access-Control-Allow-Origin", "*", true);
  webserver.sendHeader("Access-Control-Allow-Methods", "*", true);
  webserver.send(200, "text/plain", Geo()); 
}

// web server response callback function
void handle_Archive() {
  // open archive on sd card
  File archiveFile = SD.open(SDCARD_ARCHIVE_FILE_NAME, FILE_READ);
  if (!archiveFile){
    frtosLog.error(F("db failed to open archive file for reading"));
    webserver.send(500, "text/plain", "Internal server error");
    return;
  }
  webserver.sendHeader("Access-Control-Allow-Origin", "*", true);
  webserver.sendHeader("Access-Control-Allow-Methods", "*", true);
  webserver.streamFile(archiveFile, "application/octet-stream");
  archiveFile.close();
}

// web server response callback function
void handle_Info() {
  // open archive on sd card
  File infoFile = SD.open(SDCARD_INFO_FILE_NAME, FILE_READ);
  if (!infoFile){
    frtosLog.error(F("db failed to open info file for reading"));
    webserver.send(500, "text/plain", "Internal server error");
    return;
  }
  webserver.sendHeader("Access-Control-Allow-Origin", "*", true);
  webserver.sendHeader("Access-Control-Allow-Methods", "*", true);
  webserver.streamFile(infoFile, "application/octet-stream");
  infoFile.close();
}

// web server response callback function
void handle_NotFound(){
  webserver.send(404, "text/plain", "Not found");
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  frtosLog.notice("Should save config");
  shouldSaveConfig = true;
}

// callback from Time library
time_t ntp_set_time(){               // resync from sntp
  time_t tnow;
  time(&tnow);
  return  tnow;
}

// get station configuration from server
String  rmap_get_remote_config(){
  
  String payload;
  
  HTTPClient httpClient;
  WiFiClient client;
  client.setTimeout(16000); // esp32 issue https://github.com/espressif/arduino-esp32/issues/3732
  // Make a HTTP request:

  String url="http://";
  url += station.server;
  url+="/stations/";
  url+=station.user;
  url+="/";
  url+=station.stationslug;
  url+="/";
  url+=station.boardslug;
  url+="/json/";     // get one station, default boards

  frtosLog.notice(F("readRmapRemoteConfig url: %s"),url.c_str());  
  httpClient.begin(client,url.c_str());

  int httpCode = httpClient.GET();
  if (httpCode == HTTP_CODE_OK) { //Check the returning code
    payload = httpClient.getString();
    frtosLog.notice(payload.c_str());
  }else{
    frtosLog.error(F("Error http: %s"),String(httpCode).c_str());
    frtosLog.error(F("Error http: %s"),httpClient.errorToString(httpCode).c_str());
    payload=String();
  }
  httpClient.end();
  return payload;
}

// check and execute firmware update from server
void firmware_update() {

  DynamicJsonDocument doc(200); 
  doc["ver"] = SOFTWARE_VERSION;
  doc["user"] = station.user;
  doc["slug"] = station.stationslug;
  char buffer[256];
  serializeJson(doc, buffer, sizeof(buffer));

  frtosLog.notice(F("user for firmware update: %s"),station.user);
  frtosLog.notice(F("station slug for firmware update: %s"),station.stationslug);
  frtosLog.notice(F("server for firmware update: %s"),station.server);
  frtosLog.notice(F("port for firmware update: %d"),update_port);
  frtosLog.notice(F("url for firmware update: %s"),update_url);
  frtosLog.notice(F("version for firmware update: %s"),buffer);

  pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  pixels.show();
  delay(3000);

  HTTPUpdate httpUpdate(16000); //double default timeout
  WiFiClient client;

  t_httpUpdate_return ret = httpUpdate.update(client,String(station.server), update_port, String(update_url), String(buffer));
  
  switch(ret)
    {
    case HTTP_UPDATE_FAILED:
      frtosLog.error(F("[update] Update failed with message:"));
      frtosLog.error(F("%s"),httpUpdate.getLastErrorString().c_str());
      if (oledpresent) {
	LockGuard guard(i2cmutex);
	u8g2->setCursor(0, 2*CH); 
	u8g2->print(F("FW Update"));
	u8g2->setCursor(0, 3*CH); 
	u8g2->print(F("Failed"));
	u8g2->sendBuffer();
      }

      pixels.setPixelColor(0, pixels.Color(255, 0, 0));
      pixels.show();
      delay(3000);
      
    break;
    case HTTP_UPDATE_NO_UPDATES:
      frtosLog.notice(F("[update] No Update."));
      if (oledpresent) {
	LockGuard guard(i2cmutex);
	u8g2->setCursor(0, 2*CH); 
	u8g2->print(F("NO Firmware"));
	u8g2->setCursor(0, 3*CH); 
	u8g2->print(F("Update"));
	u8g2->sendBuffer();
      }
      pixels.setPixelColor(0, pixels.Color(0, 0, 255));
      pixels.show();
      delay(3000);
      
      break;
    case HTTP_UPDATE_OK:
      frtosLog.notice(F("[update] Update ok.")); // may not called we reboot the ESP
      
      if (oledpresent) {
	LockGuard guard(i2cmutex);
	u8g2->setCursor(0, 2*CH); 
	u8g2->print(F("FW Updated!"));
	u8g2->sendBuffer();
      }

      pixels.setPixelColor(0, pixels.Color(0, 255, 255));
      pixels.show();
      delay(3000);
      break;
    }

  //#endif

  pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  pixels.show();
  delay(3000);
}

// read configuration from EEPROM
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
      frtosLog.error(F("reading rmap file"));	
    }
  } else {
    frtosLog.warning(F("rmap file do not exist"));
  }
  //end read
  return String();  
}

// write configuration to EEPROM
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

// convert json configuratio to station structure
int  rmap_config(const String payload){

  bool status_station = false;
  bool status_board_mqtt = false;
  bool status_board_tcpip = false;
  bool status_sensors = false;
  int status = 0;
  measure_data.sensors_count=0;

  if (! (payload == String())) {
    DynamicJsonDocument doc(4000);
    status = 3;
    DeserializationError error = deserializeJson(doc,payload);
    if (!error){
      JsonArrayConst array = doc.as<JsonArray>();
      frtosLog.notice(F("deserialize json array[ %d ]"),array.size());
      //for (uint8_t i = 0; i < array.size(); i++) {
      for(JsonObjectConst element: array){
	
	if  (element["model"] == "stations.stationmetadata"){
	  if (element["fields"]["active"]){
	    frtosLog.notice(F("station metadata found!"));

	    //strncpy (station.longitude, element["fields"]["lon"].as<const char*>(),10);
	    //station.longitude[10]='\0';
	    itoa(int(element["fields"]["lon"].as<float>()*100000),station.longitude,10);
	    frtosLog.notice(F("lon: %s"),station.longitude);

	    //strncpy (station.latitude , element["fields"]["lat"].as<const char*>(),10);
	    //station.latitude[10]='\0';
	    itoa(int(element["fields"]["lat"].as<float>()*100000),station.latitude,10);
	    frtosLog.notice(F("lat: %s"),station.latitude);

	    strncpy (station.ident , element["fields"]["ident"].as< const char*>(),9);
	    station.ident[9]='\0';
	    frtosLog.notice(F("ident: %s"),station.ident);

	    strncpy (station.network , element["fields"]["network"].as< const char*>(),30);
	    station.network[30]='\0';
	    frtosLog.notice(F("network: %s"),station.network);

	    strncpy (station.mqttrootpath, element["fields"]["mqttrootpath"].as< const char*>(),9);
	    station.mqttrootpath[9]='\0';
	    frtosLog.notice(F("mqttrootpath: %s"),station.mqttrootpath);

	    strncpy (station.mqttmaintpath, element["fields"]["mqttmaintpath"].as< const char*>(),9);
	    station.mqttmaintpath[9]='\0';
	    frtosLog.notice(F("mqttmaintpath: %s"),station.mqttmaintpath);

	    status_station = true;
	  }
	}

	if  (element["model"] == "stations.transportmqtt"){
	  if (element["fields"]["board"][0] == station.boardslug){
	    if (element["fields"]["active"]){
	      frtosLog.notice(F("board transportmqtt found!"));
	      station.sampletime=element["fields"]["mqttsampletime"];
	      frtosLog.notice(F("station.sampletime: %d"),station.sampletime);

	      if (!element["fields"]["mqttserver"].isNull()){
		strncpy (station.mqtt_server, element["fields"]["mqttserver"].as< const char*>(),40);
		station.mqtt_server[40]='\0';
		frtosLog.notice(F("station.mqtt_server: %s"),station.mqtt_server);
	      }
	      
	      if (!element["fields"]["mqttuser"].isNull()){
		strncpy (station.user, element["fields"]["mqttuser"].as< const char*>(),9);
		station.user[9]='\0';
		frtosLog.notice(F("station.user: %s"),station.user);
	      }

	      status_board_mqtt = true;
	    }
	  }
	}

	if  (element["model"] == "stations.transporttcpip"){
	  if (element["fields"]["board"][0] == station.boardslug){
	    if (element["fields"]["active"]){
	      frtosLog.notice(F("board transporttcpip found!"));

	      if (!element["fields"]["ntpserver"].isNull()){
		strncpy (station.ntp_server, element["fields"]["ntpserver"].as< const char*>(),40);
		station.ntp_server[40]='\0';
		frtosLog.notice(F("ntp_server: %s"),station.ntp_server);
	      }
	      status_board_tcpip = true;	      
	    }
	  }
	}

	
	if  (element["model"] == "stations.sensor"){
	  if (element["fields"]["active"]){
	    if (measure_data.sensors_count < SENSORS_MAX) {
	      frtosLog.notice(F("station sensor found!"));
	      strncpy (measure_data.sensors[measure_data.sensors_count].driver , element["fields"]["driver"].as< const char*>(),SENSORDRIVER_DRIVER_LEN);
	      frtosLog.notice(F("driver: %s"),measure_data.sensors[measure_data.sensors_count].driver);
	      strncpy (measure_data.sensors[measure_data.sensors_count].type , element["fields"]["type"][0].as< const char*>(),SENSORDRIVER_TYPE_LEN);
	      frtosLog.notice(F("type: %s"),measure_data.sensors[measure_data.sensors_count].type);
	      strncpy (measure_data.sensors[measure_data.sensors_count].timerange, element["fields"]["timerange"].as< const char*>(),SENSORDRIVER_META_LEN);
	      frtosLog.notice(F("timerange: %s"),measure_data.sensors[measure_data.sensors_count].timerange);
	      strncpy (measure_data.sensors[measure_data.sensors_count].level, element["fields"]["level"].as< const char*>(),SENSORDRIVER_META_LEN);
	      frtosLog.notice(F("level: %s"),measure_data.sensors[measure_data.sensors_count].level);
	      measure_data.sensors[measure_data.sensors_count].address = element["fields"]["address"];	    
	      frtosLog.notice(F("address: %d"),measure_data.sensors[measure_data.sensors_count].address);

	      if (strcmp(measure_data.sensors[measure_data.sensors_count].type,"PMS")==0) pmspresent=true;
	      
	      measure_data.sensors_count++;
	    }
	    status_sensors = true;
	  }
	}

	if  (element["model"] == "stations.stationconstantdata"){
	  if (element["fields"]["active"]){
	    frtosLog.notice(F("station constant data found!"));
	    char btable[7];
	    strncpy (btable, element["fields"]["btable"].as< const char*>(),6);
	    btable[6]='\0';
	    frtosLog.notice(F("btable: %s"),btable);
	    char value[31];
	    strncpy (value, element["fields"]["value"].as< const char*>(),30);
	    value[30]='\0';
	    frtosLog.notice(F("value: %s"),value);
	
	    strcpy(station.constantdata[station.constantdata_count].btable,btable);
	    strcpy(station.constantdata[station.constantdata_count].value,value);
	    station.constantdata_count++;
	  }
	}
      }
      status = (int)!(status_station && status_board_mqtt && status_board_tcpip && status_sensors); //Variable 'status' is reassigned a value before the old one has been used.
    } else {
      frtosLog.error(F("error parsing array: %s"),error.c_str());
      //analogWrite(LED_PIN,973);
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));      
      pixels.show();
      delay(5000);
      status = 2;
    }

  }else{
    status=1;
  }
  return status;
}

// read configuration from EEPROM and put it in station structure
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
        DynamicJsonDocument doc(500);
        DeserializationError error = deserializeJson(doc,buf.get());
	if (!error) {
	  //json.printTo(Serial);

	  //if (doc.containsKey("rmap_longitude"))strcpy(station.longitude, doc["rmap_longitude"]);
	  //if (doc.containsKey("rmap_latitude")) strcpy(station.latitude, doc["rmap_latitude"]);
          if (doc.containsKey("rmap_server")) strcpy(station.server, doc["rmap_server"]);
          //if (doc.containsKey("ntp_server")) strcpy(station.ntp_server, doc["ntp_server"]);
          //if (doc.containsKey("rmap_mqtt_server")) strcpy(station.mqtt_server, doc["rmap_mqtt_server"]);
          if (doc.containsKey("rmap_user")) strcpy(station.user, doc["rmap_user"]);
          if (doc.containsKey("rmap_password")) strcpy(station.password, doc["rmap_password"]);
          if (doc.containsKey("rmap_stationslug")) strcpy(station.stationslug, doc["rmap_stationslug"]);
	  //if (doc.containsKey("rmap_mqttrootpath")) strcpy(station.mqttrootpath, doc["rmap_mqttrootpath"]);
	  //if (doc.containsKey("rmap_mqttmaintpath")) strcpy(station.mqttmaintpath, doc["rmap_mqttmaintpath"]);
	  
	  frtosLog.notice(F("loaded config parameter:"));
	  //frtosLog.notice(F("longitude: %s"),station.longitude);
	  //frtosLog.notice(F("latitude: %s"),station.latitude);
	  frtosLog.notice(F("server: %s"),station.server);
	  //frtosLog.notice(F("ntp server: %s"),station.ntp_server);
	  //frtosLog.notice(F("mqtt server: %s"),station.mqtt_server);
	  frtosLog.notice(F("user: %s"),station.user);
	  //frtosLog.notice(F("password: %s"),station.password);
	  frtosLog.notice(F("stationslug: %s"),station.stationslug);
	  //frtosLog.notice(F("mqttrootpath: %s"),station.mqttrootpath);
	  //frtosLog.notice(F("mqttmaintpath: %s"),station.mqttmaintpath);
	  
        } else {
          frtosLog.error(F("failed to deserialize json config %s"),error.c_str());
        }
      } else {
	frtosLog.error(F("error reading config file"));	
      }
    } else {
      frtosLog.warning(F("config file do not exist"));
    }
  //end read
}

// write configuration to EEPROM from station structure
void writeconfig() {;

  //save the custom parameters to FS
  frtosLog.notice(F("saving config"));
  //DynamicJsonDocument jsonBuffer;
  DynamicJsonDocument json(500);
    
  //json["rmap_longitude"] = station.longitude;
  //json["rmap_latitude"] = station.latitude;
  json["rmap_server"] = station.server;
  //json["ntp_server"] = station.ntp_server;
  //json["rmap_mqtt_server"] = station.mqtt_server;
  json["rmap_user"] = station.user;
  json["rmap_password"] = station.password;
  json["rmap_stationslug"] = station.stationslug;
  //json["rmap_mqttrootpath"] = station.mqttrootpath;
  //json["rmap_mqttmaintpath"] = station.mqttmaintpath;
  
  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    frtosLog.error(F("failed to open config file for writing"));
  }

  //json.printTo(Serial);
  serializeJson(json,configFile);
  configFile.close();
  frtosLog.notice(F("saved config parameter"));
}

// print status and summary it for display and manage LED
void displayStatus()
{

  bool light=true;
  
  frtosLog.notice(F("status measure: novalue %d, sensor  %d, geodef %d"),stimawifiStatus.measure.novalue,stimawifiStatus.measure.sensor,stimawifiStatus.measure.geodef);
  frtosLog.notice(F("status publish: connect %d, publish %d"),stimawifiStatus.publish.connect,stimawifiStatus.publish.publish);
  frtosLog.notice(F("status db     : sdcard %d, database %d, archive %d"),stimawifiStatus.db.sdcard,stimawifiStatus.db.database,stimawifiStatus.db.archive);
  frtosLog.notice(F("status stimawifi: rtc    : %d"),stimawifiStatus.rtc);
  //here print memory status?
  if (strcmp(station.ident,"") != 0){
    frtosLog.notice(F("status gps    : receive %d" ),stimawifiStatus.gps.receive);
    frtosLog.notice(F("status udp    : receive %d" ),stimawifiStatus.udp.receive);
  }

  // collect error in summary  
  //  data.status.summary.err_power_on= false;	
  //  data.status.summary.err_reboot=	false;
  stimawifiStatus.summary.err_georef |= 	   ((strcmp(station.ident,"") != 0) && (stimawifiStatus.gps.receive == error || stimawifiStatus.udp.receive == error));

  stimawifiStatus.summary.err_sdcard |=  	   stimawifiStatus.db.sdcard == error;  
  stimawifiStatus.summary.err_db |=  	           stimawifiStatus.db.database == error;
  stimawifiStatus.summary.err_archive |=  	   stimawifiStatus.db.archive == error;  
  stimawifiStatus.summary.err_mqtt_publish |=      stimawifiStatus.publish.publish == error;
  stimawifiStatus.summary.err_mqtt_connect |=      stimawifiStatus.publish.connect == error;
  stimawifiStatus.summary.err_geodef |=	           stimawifiStatus.measure.geodef  == error;
  stimawifiStatus.summary.err_sensor |=	           stimawifiStatus.measure.sensor  == error;
  stimawifiStatus.summary.err_novalue |=           stimawifiStatus.measure.novalue == error;
  stimawifiStatus.summary.err_rtc |=  	           stimawifiStatus.rtc == error;  
  stimawifiStatus.summary.err_memory |=  	   stimawifiStatus.db.memory_collision == error || stimawifiStatus.db.no_heap_memory == error ||
                                                   stimawifiStatus.publish.memory_collision == error || stimawifiStatus.publish.no_heap_memory == error ||
                                                   stimawifiStatus.measure.memory_collision == error || stimawifiStatus.measure.no_heap_memory == error ||
                                                   stimawifiStatus.udp.memory_collision == error || stimawifiStatus.udp.no_heap_memory == error ||
                                                   stimawifiStatus.gps.memory_collision == error || stimawifiStatus.gps.no_heap_memory == error ;

  // start with unknown BLACK
  strcpy(status,"Stat: unknown");
  uint32_t color = pixels.Color(0,0,0);

  // if one not unknown then BLUE
  if (    stimawifiStatus.measure.novalue != unknown || stimawifiStatus.measure.sensor  != unknown || stimawifiStatus.measure.geodef  != unknown
	  || stimawifiStatus.publish.connect != unknown || stimawifiStatus.publish.publish != unknown
	  || stimawifiStatus.db.sdcard != unknown || stimawifiStatus.db.database != unknown || stimawifiStatus.db.archive != unknown
	  || ((strcmp(station.ident,"") != 0) && (stimawifiStatus.gps.receive != unknown || stimawifiStatus.udp.receive != unknown))) {
    strcpy(status,"Stat: working");
    color = pixels.Color(0,0,255);
    light=true;
  }
  // if all OK then GREEN
  if (    stimawifiStatus.measure.novalue == ok && stimawifiStatus.measure.sensor  == ok && stimawifiStatus.measure.geodef  == ok
	  &&  stimawifiStatus.publish.connect == ok && stimawifiStatus.publish.publish == ok
	  &&  stimawifiStatus.db.sdcard == ok && stimawifiStatus.db.database == ok && stimawifiStatus.db.archive == ok
	  &&  stimawifiStatus.db.database == ok && stimawifiStatus.db.archive == ok
	  &&  stimawifiStatus.rtc == ok
	  //&&  stimawifiStatus.db.memory_collision == ok && stimawifiStatus.db.no_heap_memory == ok
          //&&  stimawifiStatus.publish.memory_collision == ok && stimawifiStatus.publish.no_heap_memory == ok
          //&&  stimawifiStatus.measure.memory_collision == ok && stimawifiStatus.measure.no_heap_memory == ok
          //&&  stimawifiStatus.udp.memory_collision == ok && stimawifiStatus.udp.no_heap_memory == ok
	  //&&  stimawifiStatus.gps.memory_collision == ok && stimawifiStatus.gps.no_heap_memory == ok
	  &&  ((strcmp(station.ident,"") == 0) || (stimawifiStatus.gps.receive == ok && stimawifiStatus.udp.receive == ok))) {
    strcpy(status,"Stat: ok");
    color = pixels.Color(0,255,0);
    light=true;
  }
  // if one is error then RED
  if (      stimawifiStatus.measure.novalue == error
	 || stimawifiStatus.measure.sensor  == error
	 || stimawifiStatus.measure.geodef  == error
	 || stimawifiStatus.publish.connect == error
	 || stimawifiStatus.publish.publish == error
	 || stimawifiStatus.db.sdcard == error
	 || stimawifiStatus.db.database == error
	 || stimawifiStatus.db.archive == error
	 || stimawifiStatus.rtc == error
	 || stimawifiStatus.db.memory_collision == error || stimawifiStatus.db.no_heap_memory == error
         || stimawifiStatus.publish.memory_collision == error || stimawifiStatus.publish.no_heap_memory == error
         || stimawifiStatus.measure.memory_collision == error || stimawifiStatus.measure.no_heap_memory == error
         || stimawifiStatus.udp.memory_collision == error || stimawifiStatus.udp.no_heap_memory == error
	 || stimawifiStatus.gps.memory_collision == error || stimawifiStatus.gps.no_heap_memory == error
	 || ((strcmp(station.ident,"") != 0) && (stimawifiStatus.gps.receive == error || stimawifiStatus.udp.receive == error))){
    strcpy(status,"Stat: error");
    color = pixels.Color(255,0,0);
    light = true;
  }

  if (oledpresent) { // message on display
    display_summary_data(status);
  }

  if (light){   // set neopixel
    pixels.setPixelColor(0, color);
  }else{
    pixels.setPixelColor(0, pixels.Color(0,0,0));    
  }
  pixels.show();
}

// send signal to DB thread for recovery unsent data from DB on SD card
void dataRecovery() {
  recoverySemaphore.Give();
}

// send signal to measure thread to start measuremets
void measureAndPublish() {
  threadMeasure.Notify();
}

// reboot ESP
void reboot() {
  //reset and try again, or maybe put it to deep sleep
  ESP.restart();
  delay(5000);
}

// prefix for logging system
void logPrefix(Print* _logOutput) {
  char dt[DATE_TIME_STRING_LENGTH];
  snprintf(dt, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u", year(), month(), day(), hour(), minute(), second());
  _logOutput->print("#");
  _logOutput->print(dt);
  _logOutput->print(" ");
}

// suffix for logging system
void logSuffix(Print* _logOutput) {
  _logOutput->print('\n');
  _logOutput->flush();  // we use this to flush every log message
}

// arduino setup routine
void setup() {
  // put your setup code here, to run once in Arduin task:

  /*
  #include "soc/soc.h"
  #include "soc/rtc_cntl_reg.h"
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable Brownout detector
  */
  
  // Initialize GPIO, library and sensors
  pinMode(RESET_PIN, INPUT_PULLUP);
  
  pixels.begin();            //INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();            // Turn OFF all pixels ASAP
  pixels.setBrightness(125); // Set BRIGHTNESS (max = 255)
  
  /*
  pinMode(PMS_RESET, OUTPUT);
  //reset pin for sensor
  digitalWrite(PMS_RESET,LOW); // reset low
  delay(500);
  digitalWrite(PMS_RESET,HIGH);
  */
  
  //Serial.setTxTimeoutMs(0);  // https://github.com/espressif/arduino-esp32/issues/6983
  delay(5000);
  Serial.begin(115200);
  //Serial.setDebugOutput(true);

  // set summary status from one CPU only
  set_status_summary(rtc_get_reset_reason(0));
    
  Serial.println("CPU0 reset reason:");
  print_reset_reason(rtc_get_reset_reason(0));
  verbose_print_reset_reason(rtc_get_reset_reason(0));
  
  Serial.println("CPU1 reset reason:");
  print_reset_reason(rtc_get_reset_reason(1));
  verbose_print_reset_reason(rtc_get_reset_reason(1));
  
  // manage reset button in hardware (RESET_PIN) or in software (I2C)
  bool reset=digitalRead(RESET_PIN) == LOW;
  if (button.get() == 0)
  {
    if (button.BUTTON_A)
    {
      //String keyString[] = {"None", "Press", "Long Press", "Double Press", "Hold"};
      //Serial.print(F("BUTTON A: "));
      //Serial.println(keyString[button.BUTTON_A]);
      if (button.BUTTON_A == KEY_VALUE_HOLD) reset=true;
    }
  }

  if (reset) {

    Serial.println(F("Try to reformat SD card."));
    SPI.begin(C3SCK, C3MISO, C3MOSI, C3SS); //SCK, MISO, MOSI, SS

    // SdCardFactory constructs and initializes the appropriate card.
    SdCardFactory cardFactory;
    ExFatFormatter exFatFormatter;
    FatFormatter fatFormatter;
    uint8_t  sectorBuffer[512];
    SdCard* m_card = cardFactory.newCard(SdSpiConfig(C3SS, DEDICATED_SPI));
    if (!m_card) {
      Serial.println(F("Invalid SD_CONFIG"));
    } else if (m_card->errorCode()) {
      if (m_card->errorCode() == SD_CARD_ERROR_CMD0) {
	Serial.println(F("No card, wrong chip select pin, or wiring error?"));
      }
      Serial.print(F("SD errorCode: "));
      printSdErrorSymbol(&Serial, m_card->errorCode());
      Serial.print(F(" = "));
      Serial.println(m_card->errorCode());
      Serial.print(F("SD errorData = "));
      Serial.println(int(m_card->errorData()));
    }else{
      uint32_t cardSectorCount = m_card->sectorCount();
      if (!cardSectorCount){
	Serial.println(F("Error: get sector count failed."));
      }else{
	// Format exFAT if larger than 32GB.
	bool rtn = cardSectorCount > 67108864
	  ? exFatFormatter.format(m_card, sectorBuffer, &Serial)
	  : fatFormatter.format(m_card, sectorBuffer, &Serial);
	
	if (rtn){
	  Serial.println(F("OK SD card formatted."));
	}
      }
    }

    m_card->end();
    
  }
    

  // if loggin on SDcard is enabled initialize SDcard
  // initialize logging
#if (ENABLE_SDCARD_LOGGING)

  Serial.println("\nInitializing SD card..." );

  SPI.begin(C3SCK, C3MISO, C3MOSI, C3SS); //SCK, MISO, MOSI, SS
  //SPI.setDataMode(SPI_MODE3);
  //bool begin(uint8_t ssPin=SS, SPIClass &spi=SPI, uint32_t frequency=SPICLOCK, const char * mountpoint="/sd", uint8_t max_files=5, bool format_if_empty=false)
  if (!SD.begin(C3SS,SPI,SPICLOCK, "/sd",SDMAXFILE, false)){
    Serial.println   (F("initialization failed. Things to check:"));
    Serial.println   (F("* is a card inserted?"));
    Serial.println   (F("* is your wiring correct?"));
    Serial.println   (F("* did you change the chipSelect pin to match your shield or module?"));
  } else {
    Serial.println   (F("Wiring is correct and a card is present."));

    logFile = SD.open(SDCARD_LOGGING_FILE_NAME, FILE_APPEND);
    if (logFile) {
      //logFile.seekEnd(0);
      frtosLog.begin(LOG_LEVEL, &loggingStream,loggingmutex);
      //Log.begin(LOG_LEVEL_VERBOSE, &loggingStream);
  } else {
      frtosLog.begin(LOG_LEVEL, &Serial,loggingmutex);
      //Log.begin(LOG_LEVEL_VERBOSE, &Serial);
      frtosLog.error(F("logging begin with sdcard\n"));
    }
  }
#else
  // Pass log level, whether to show log level, and print interface.
  // Available levels are:
  // LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
  // Note: if you want to fully remove all logging code, change #define LOG_LEVEL ....
  //       this will significantly reduce your project size

  // set runtime log level to the same of compile time
  frtosLog.begin(LOG_LEVEL, &Serial,loggingmutex);

  /*
  // enable to get logging from library (no mutex protected)
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.setPrefix(logPrefix); // Uncomment to get timestamps as prefix
  Log.setSuffix(logSuffix); // Uncomment to get newline as suffix
  */
  
#endif

  frtosLog.setPrefix(logPrefix); // Uncomment to get timestamps as prefix
  frtosLog.setSuffix(logSuffix); // Uncomment to get newline as suffix
  frtosLog.notice(F("Started"));
  frtosLog.notice(F("Version: " SOFTWARE_VERSION));

  // two different display with different dimension are managed with two different I2C address
  // check return value of
  // the Write.endTransmisstion to see if
  // a device did acknowledge to the address.  
  {
    LockGuard guard(i2cmutex);

    Wire.begin();
    //Wire.begin(SDA_PIN,SCL_PIN);
    if (!Wire.setClock(I2C_BUS_CLOCK)) frtosLog.error(F("setting I2C clock"));
    Wire.setTimeOut(I2C_BUS_TIMEOUT);
    delay(50);

    for (uint8_t retry=0; retry < 3 ; retry++) {
      Wire.beginTransmission(OLEDI2CADDRESS_128X64);
      if (Wire.endTransmission() == 0) {
	u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0);
	u8g2->setI2CAddress(OLEDI2CADDRESS_128X64*2);
	u8g2->begin();
	u8g2->setFont(u8g2_font_6x10_tf);
	CH=10;
	oledpresent=true;
	break;
      }
      
      Wire.beginTransmission(OLEDI2CADDRESS_64X48);
      if (Wire.endTransmission() == 0) {
	u8g2 = new U8G2_SSD1306_64X48_ER_F_HW_I2C(U8G2_R0);
	u8g2->setI2CAddress(OLEDI2CADDRESS_64X48*2);
	u8g2->begin();
	u8g2->setFont(u8g2_font_5x7_tf);      
	CH=7;
	oledpresent=true;
	break;
      }
    }
    
    if (oledpresent) {
      frtosLog.notice(F("OLED Found"));
      u8g2->setFontMode(0); // enable transparent mode, which is faster
      u8g2->clearBuffer();
      u8g2->setCursor(0, 1*CH); 
      u8g2->print(F("Starting up!"));
      u8g2->setCursor(0, 2*CH); 
      u8g2->print(F("Version:"));
      u8g2->setCursor(0, 3*CH); 
      u8g2->print(F(SOFTWARE_VERSION));
      u8g2->sendBuffer();
    }else{
      frtosLog.notice(F("OLED NOT Found"));
    }
  }
  pixels.setPixelColor(0, pixels.Color(255, 0, 255));
  pixels.show();
  delay(3000);
  
  /*
  char esp_chipid[11];
  itoa(ESP.getChipId(),esp_chipid,10);
  frtosLog.notice(F("esp_chipid: %s "),esp_chipid );
  */
  
  if (reset) {
    frtosLog.notice(F("clean FS"));
    if (oledpresent) {
      LockGuard guard(i2cmutex);
      u8g2->clearBuffer();
      u8g2->setCursor(0, 1*CH); 
      u8g2->print(F("Clean FS"));
      u8g2->setCursor(0, 2*CH); 
      u8g2->print(F("Reset wifi"));
      u8g2->setCursor(0, 3*CH); 
      u8g2->print(F("Reset conf"));
      u8g2->sendBuffer();
      delay(3000);
    }
    // reset configuration on  on EEPROM
    LittleFS.begin(false,"/littlefs",LFMAXFILE);    
    LittleFS.format();
    frtosLog.notice(F("user request Reset wifi configuration"));
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
    frtosLog.warning(F("littlefs error Reset wifi configuration"));
    wifiManager.resetSettings();
    
    if (oledpresent) {
      LockGuard guard(i2cmutex);
      u8g2->clearBuffer();
      u8g2->setCursor(0, 1*CH); 
      u8g2->print(F("Mount FS"));
      u8g2->setCursor(0, 2*CH); 
      u8g2->print(F("Failed"));
      u8g2->setCursor(0, 3*CH); 
      u8g2->print(F("RESET"));
      u8g2->setCursor(0, 4*CH); 
      u8g2->print(F("CONFIGURATION"));
      u8g2->sendBuffer();
      delay(3000);
    }
  }

  if (readconfig_rmap() == String()) {
    frtosLog.notice(F("station configuration not found!"));
    frtosLog.notice(F("no station conf; Reset wifi configuration"));
    wifiManager.resetSettings();
  }

  // initialize RTC with mutex
  frtosRTC.begin(RTC,i2cmutex);
  
  // configure NTP
  //sntp_init();
  //sntp_setoperatingmode(SNTP_OPMODE_POLL);
  //sntp_setservername(0, station.ntp_server);
  // set notification call-back function
  sntp_set_time_sync_notification_cb( timeavailable );
  sntp_servermode_dhcp(1);
  configTime(0, 0, station.ntp_server);
  sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_rmap_server("server", "rmap server", station.server, 41);
  WiFiManagerParameter custom_rmap_user("user", "rmap user", station.user, 10);
  WiFiManagerParameter custom_rmap_password("password", "station password", station.password, 31, "type = \"password\"");
  WiFiManagerParameter custom_rmap_stationslug("slug", "station slug", station.stationslug, 31);

  //add all your parameters here
  wifiManager.addParameter(&custom_rmap_server);
  wifiManager.addParameter(&custom_rmap_user);
  wifiManager.addParameter(&custom_rmap_password);
  wifiManager.addParameter(&custom_rmap_stationslug);

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
  wifiManager.setConfigPortalTimeout(180);

  // USE THIS OPTIONS WITH WIFIMANAGER VERSION 2
  //if false, timeout captive portal even if a STA client connected to softAP (false), suggest disabling if captiveportal is open
  wifiManager.setAPClientCheck(false);
  //if true, reset timeout when webclient connects (true), suggest disabling if captiveportal is open    
  wifiManager.setWebPortalClientCheck(false);
    
  if (oledpresent) {
      LockGuard guard(i2cmutex);
      u8g2->clearBuffer();
      u8g2->setCursor(0, 1*CH); 
      u8g2->print(F("ssed:"));
      u8g2->setCursor(0, 2*CH); 
      u8g2->print(F(WIFI_SSED));
      u8g2->setCursor(0, 3*CH); 
      u8g2->print(F("password:"));
      u8g2->setCursor(0, 4*CH); 
      u8g2->print(F(WIFI_PASSWORD));
      u8g2->sendBuffer();
  }

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.setDebugOutput(true);
  wifiManager.setWiFiAutoReconnect(true);
  if (!wifiManager.autoConnect(WIFI_SSED,WIFI_PASSWORD)) {
    frtosLog.error(F("failed to connect and hit timeout"));
    if (oledpresent) {
      LockGuard guard(i2cmutex);
      u8g2->clearBuffer();
      u8g2->setCursor(0, 1*CH); 
      u8g2->print(F("WIFI KO"));
      u8g2->sendBuffer();
    }
    delay(3000);
    //WiFi.disconnect(true);
    //esp_wifi_stop();
    //reboot();
  }else{
    //if you get here you have connected to the WiFi
    //WiFi.setAutoReconnect(true);
    wifiManager.setDisableConfigPortal(true);
    frtosLog.notice(F("connected... good!"));
    frtosLog.notice(F("local ip: %s"),WiFi.localIP().toString().c_str());
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    pixels.show();
    delay(3000);
    
    if (oledpresent) {
      LockGuard guard(i2cmutex);
      u8g2->clearBuffer();
      u8g2->setCursor(0, 1*CH); 
      u8g2->print(F("WIFI OK"));
      u8g2->sendBuffer();
      u8g2->setCursor(0, 4*CH); 
      u8g2->print(F("IP:"));
      //u8g2->setFont(u8g2_font_u8glib_4_tf);
      u8g2->print(WiFi.localIP().toString().c_str());
      //u8g2->setFont(u8g2_font_5x7_tf);
      u8g2->sendBuffer();
    }    
  }
  
  /*
  WiFi.begin("ssid", "password");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  */

  if (shouldSaveConfig){
    //read updated parameters
    strcpy(station.server, custom_rmap_server.getValue());
    strcpy(station.user, custom_rmap_user.getValue());
    strcpy(station.password, custom_rmap_password.getValue());
    strcpy(station.stationslug, custom_rmap_stationslug.getValue());

    writeconfig();
    if (oledpresent) {
      LockGuard guard(i2cmutex);
      u8g2->clearBuffer();
      u8g2->setCursor(0, 1*CH); 
      u8g2->print(F("NEW configuration"));
      u8g2->setCursor(0, 2*CH); 
      u8g2->print(F("saved"));
      u8g2->sendBuffer();
    }
  }

  // perform remote configuration
  String remote_config= rmap_get_remote_config();

  if ( remote_config == String() ) {
    frtosLog.error(F("remote configuration failed"));
    remote_config=readconfig_rmap();
  }else{
    writeconfig_rmap(remote_config);
  }

  // check for a firmware update from server
  firmware_update();

  // if here we do not have configuration reboot
  if (!rmap_config(remote_config) == 0) {
    frtosLog.error(F("station not configurated"));
    //frtosLog.notice(F("Reset wifi configuration"));
    //wifiManager.resetSettings();

    if (oledpresent){
      LockGuard guard(i2cmutex);
      u8g2->clearBuffer();
      u8g2->setCursor(0, 1*CH); 
      u8g2->print(F("Station not"));
      u8g2->setCursor(0, 2*CH); 
      u8g2->print(F("configurated!"));
      u8g2->setCursor(0, 3*CH);
      u8g2->print(F("RESTART"));
      u8g2->sendBuffer();
    }
    
    frtosLog.error(F("reboot!"));
    delay(5000);
    reboot();
  }

  if (reset) {
    if (oledpresent){
      LockGuard guard(i2cmutex);
      u8g2->clearBuffer();
      u8g2->setCursor(0, 1*CH); 
      u8g2->print(F("Station initialized"));
      u8g2->setCursor(0, 2*CH); 
      u8g2->print(F("and configurated!"));
      u8g2->setCursor(0, 4*CH);
      u8g2->print(F("remove clear bridge"));
      u8g2->setCursor(0, 5*CH);
      u8g2->print(F("and POWER OFF"));
      u8g2->sendBuffer();
    }
    while(true){
      frtosLog.notice(F("remove clear bridge"));
      frtosLog.notice(F("and POWER OFF!"));
      delay(1000);
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));
      pixels.show();
      delay(1000);
      pixels.setPixelColor(0, pixels.Color(255, 255, 255));
      pixels.show();
    }
  }
  
  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "stimawifi.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  while (!MDNS.begin(station.stationslug)) {
    frtosLog.error(F("Error setting up MDNS responder!"));
    delay(1000);
  }
  frtosLog.notice(F("mDNS responder started"));

  // setup web server
  webserver.enableDelay(false);
  webserver.on("/", handle_FullPage);
  webserver.on("/data", handle_Data);
  webserver.on("/data.json", handle_Json);
  webserver.on("/geo", handle_Geo);
  webserver.on("/archive.dat", handle_Archive);
  webserver.on("/info.dat", handle_Info);
  webserver.onNotFound(handle_NotFound);
  
  webserver.begin();
  frtosLog.notice(F("HTTP server started"));

  if (oledpresent){
    LockGuard guard(i2cmutex);
    u8g2->setDrawColor(0);
    u8g2->drawBox( 0, 1*CH,64, CH);
    u8g2->setDrawColor(1);
    u8g2->setCursor(0, 2*CH);
    u8g2->print(F("Setting time"));
    u8g2->setDrawColor(0);
    u8g2->drawBox( 0, 2*CH,64, CH);
    u8g2->setDrawColor(1);
    u8g2->sendBuffer();
  }

  // wait for  date and time setup from NTP
  if (WiFi.status() == WL_CONNECTED) {
    //if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(60000)) == ESP_OK) {  // alternative starting from idf 5.1 with esp_netif_sntp.h
    int retry = 0;
    const int retry_count = 60;
    time_t datetime = 0;
    struct tm timeinfo = { 0 };
    while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
      frtosLog.notice(F("Waiting for system time to be set... (%d/%d)"), retry, retry_count);
      delay(1000);
      datetime=now();
      localtime_r(&datetime, &timeinfo);
    }
  }
  

  if (timeStatus() == timeSet) {
    frtosLog.notice(F("Getted time from NTP"));
     setSyncProvider(ntp_set_time);
  } else{
    esp_sntp_stop();
    if (frtosRTC.isRunning() && (year(frtosRTC.get()) > 2020)){
      frtosLog.notice(F("Getted time from RTC"));
      setSyncProvider(rtc_set_time);   // the function to get the time from the RTC
    }else{
      stimawifiStatus.rtc=error;
    }
  }

  if (timeStatus() != timeSet) {
    frtosLog.warning(F("Time not setted"));  
    if (oledpresent){
      LockGuard guard(i2cmutex);
      u8g2->clearBuffer();
      u8g2->setDrawColor(0);
      u8g2->drawBox( 0, 1*CH,64, CH);
      u8g2->setDrawColor(1);
      u8g2->setCursor(0, 2*CH); 
      u8g2->print(F("NTP Time Error"));
      //u8g2->setDrawColor(0);
      //u8g2->drawBox( 0, 2*CH,64, CH);
      //u8g2->setDrawColor(1);
      //u8g2->setCursor(0, 3*CH);
      //u8g2->print(F("RESTART"));
      u8g2->sendBuffer();

      // delay(5000);
      // reboot(); //timeout - reset board
    }
  } else {
    time_t time=now();
    frtosLog.notice(F("Time: %s"),ctime(&time));  
    if (oledpresent){
      LockGuard guard(i2cmutex);
      u8g2->setCursor(0, 3*CH);
      u8g2->print(F("Time OK"));
      u8g2->sendBuffer();
    }
  }

  frtosLog.notice(F("mqtt server: %s"),station.mqtt_server);
  
  Alarm.timerRepeat(10, dataRecovery);                         // timer for data recovery from DB
  Alarm.timerRepeat(station.sampletime, measureAndPublish);    // timer for measure every SAMPLETIME seconds
  Alarm.timerRepeat(3,displayStatus);                          // display status every 3 seconds

  time_t reboottime;                                    
  if (pmspresent){
    reboottime=3600*24;                                        // pms stall sometime, we reboot more
  }else{
    reboottime=3600*24*7;                                      // we reset everythings one time a week
  }
  frtosLog.notice(F("reboot every: %l"),reboottime);
  Alarm.timerRepeat(reboottime,reboot);                        // timer for reboot

  // update firmware
  //Alarm.alarmRepeat(4,0,0,firmware_update);                  // 4:00:00 every day  
  Alarm.timerRepeat(3600*24,firmware_update);                  // check for firmware update every day  
  
  // Add http service to MDNS-SD
  MDNS.addService("http", "tcp", STIMAHTTP_PORT);

  // if mobile station start geolocation thread
  if (strcmp(station.ident,"") != 0 || (timeStatus() != timeSet)){
    threadUdp.Start();
    threadGps.Start();
  }

  // start other thread
  threadDb.Start();
  threadPublish.Start();
  threadMeasure.Begin();
  threadMeasure.Start();

  //esp_task_wdt_init(60, true);
  //enableLoopWDT();
  //disableLoopWDT();
  //rtc_wdt_protect_off();
  //rtc_wdt_disable();
  //wdt_hal_disable();
}

// arduino loop routine
void loop() {
  if (loopinit){
    // set the priority of this thread
    vTaskPrioritySet(NULL, TASK_LOOP_PRIORITY);
    //disableLoopWDT();
    loopinit=false;
  }
  webserver.handleClient();
  //MDNS.update();
  Alarm.delay(0);       // check for alarms
  delay(100);
  
  //frtosLog.notice("stack loop: %d",uxTaskGetStackHighWaterMark(NULL));
  if(uxTaskGetStackHighWaterMark(NULL)< 100) frtosLog.error("stack loop");   // check memory collision
}
