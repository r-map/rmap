/*
 * ESP8266 + BOOTSTRAP + SPIFFS
 * Copyright (C) 2018 Paolo Paruno <p.patruno@iperbole.bologna.it>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
//#include <PubSubClient.h>
//#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <SoftwareSerial.h>
#include <TimeAlarms.h>
#include <ArduinoLog.h>
#include <Wire.h>
#include <U8g2lib.h>
//#include <IRremote.h>             // Include IR Remote Library by Ken Shirriff
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <i2cibt_2.h>

#include "config.h"

// increment on change
#define SOFTWARE_VERSION "2018-09-21T00:00"
#define FIRMWARE_TYPE "DOMOTICA"

// Define IR Receiver and Results Objects
IRrecv irrecv(RECV_PIN);
decode_results results;

// watchdog is enabled by default on ESP
// https://techtutorialsx.com/2017/01/21/esp8266-watchdog-functions/
  
//const char* update_host = "rmap.cc";
const char* update_url = "/firmware/update/" FIRMWARE_TYPE "/";
const int update_port = 80;

WiFiClient espClient;
//PubSubClient mqttclient(espClient);

U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0);
bool oledpresent=false;
uint8_t displaypos;

// create Objects
ESP8266WebServer server ( 80 );
DNSServer dnsServer;

AlarmID_t alarmpowersave=0;
short int lastkey=-1;
int8_t lastvalue=0;
uint8_t value7=10;
uint8_t value8=10;
uint8_t value9=10;

bool status7=false;
bool status8=false;
bool status9=false;

i2cgpio gpio1;
i2cgpio gpio2(I2C_PWM_DEFAULTADDRESS+1);
i2cibt_2 mybridgef(IBT_2_FULL,gpio1);
i2cibt_2 mybridge2h(IBT_2_2HALF,gpio2);


void firmware_upgrade() {

  StaticJsonBuffer<200> jsonBuffer; 
  JsonObject& root = jsonBuffer.createObject();
  root["ver"] = SOFTWARE_VERSION;
  char buffer[256];
  root.printTo(buffer, sizeof(buffer));
  LOGN(F("url for firmware update: %s" CR),update_url);
  LOGN(F("version for firmware update: %s" CR),buffer);

  analogWriteFreq(4);
  analogWrite(LED_PIN,512);  

  //		t_httpUpdate_return ret = ESPhttpUpdate.update(update_host, update_port, update_url, String(SOFTWARE_VERSION) + String(" ") + esp_chipid + String(" ") + SDS_version + String(" ") + String(current_lang) + String(" ") + String(INTL_LANG));
  t_httpUpdate_return ret = ESPhttpUpdate.update(ota_server, update_port, update_url, String(buffer));
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

void reboot() {
  //reset and try again, or maybe put it to deep sleep
  ESP.restart();
  delay(5000);
}

void updateGpio(){
  String gpio = server.arg("id");
  String etat = server.arg("etat");
  String success = "1";
  int pin = GPIOPIN[0];

  Serial.println(gpio);  
  const char* cind = gpio.c_str(); 
  int ind = atoi(cind);
 
  if ( ind >= 0 && ind <= 4 ) {
    pin = GPIOPIN[ind];
  }
  LOGN(F("pin: %d" CR),pin);

  if ( etat == "1" ) {
    digitalWrite(pin, HIGH);
  } else if ( etat == "0" ) {
    digitalWrite(pin, LOW);
  } else {
    success = "0";
    //LOGE(F("Err pin value: %d" CR),etat);    
  }

  /*
  if (oledpresent) {
    //u8g2.clearBuffer();
    u8g2.setCursor(0, (ind+1)*10); 
    u8g2.print(F("key "));
    u8g2.print(ind+1);
    //u8g2.setCursor(0, 20); 
    u8g2.print(F(" : "));
    u8g2.print(digitalRead(GPIOPIN[ind]));
    u8g2.sendBuffer();
  }
  */

  
  String json = "{\"gpio\":\"" + String(gpio) + "\",";
  json += "\"etat\":\"" + String(etat) + "\",";
  json += "\"success\":\"" + String(success) + "\"}";
    
  server.send(200, "application/json", json);
  LOGN(F("GPIO updated" CR));
}


void tryupgrade(){
  LOGN(F("Wait for wifi configuration" CR));
  if (oledpresent) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 10); 
    u8g2.print(F("Wait conf"));
    u8g2.sendBuffer();
    delay(3000);
  }
  
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //wifiManager.resetSettings();
  
  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  
  //set minimum quality of signal so it ignores AP's under that quality
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
    reboot();
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
    LOGN(F("local ip: %s" CR),WiFi.localIP().toString().c_str());
    
    
    if (oledpresent) {
      u8g2.setCursor(0, 40); 
      u8g2.print(F("IP:"));
      u8g2.setFont(u8g2_font_u8glib_4_tf);
      u8g2.print(WiFi.localIP().toString().c_str());
      u8g2.setFont(u8g2_font_5x7_tf);
      u8g2.sendBuffer();
    }
    
    firmware_upgrade();
  }
}


void powersave(){
  u8g2.setPowerSave(1);
}


void display_status(){

  if (oledpresent) {
    u8g2.clearBuffer();
    
    u8g2.setCursor(0,10); 
    u8g2.print(F("7: "));
    if (status7){    
      u8g2.print(F("On  "));
    }else{
      u8g2.print(F("Off "));
    }
    u8g2.print(F(" "));
    u8g2.print(value7);
    
    u8g2.setCursor(0,20); 
    u8g2.print(F("8: "));
    if (status8){    
      u8g2.print(F("On  "));
    }else{
      u8g2.print(F("Off "));
    }
    u8g2.print(F(" "));
    u8g2.print(value8);
    
    u8g2.setCursor(0,30); 
    u8g2.print(F("9: "));
    if (status9){    
      u8g2.print(F("On  "));
    }else{
      u8g2.print(F("Off "));
    }
    u8g2.print(F(" "));
    u8g2.print(value9);
    
    u8g2.setCursor(0, 45);
    for (uint8_t i=0; i<4; i++){
      if (digitalRead(GPIOPIN[i])){    
	u8g2.print(F(" On"));
      }else{
	u8g2.print(F(" Of"));
      }
    }

    u8g2.sendBuffer();
    u8g2.setPowerSave(0);
    Alarm.free(alarmpowersave);
    alarmpowersave=Alarm.timerOnce(TIMEON, powersave);
  }
}

void setup() {
     
  Serial.begin ( 115200 );

  // Pass log level, whether to show log level, and print interface.
  // Available levels are:
  // LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
  // Note: if you want to fully remove all logging code, change #define LOG_LEVEL ....
  //       this will significantly reduce your project size

  // set runtime log level to the same of compile time
  Log.begin(LOG_LEVEL, &Serial);
  LOGN(F("Started" CR));
  LOGN(F("Version: " SOFTWARE_VERSION CR));
  
    
  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  analogWriteFreq(1);
  digitalWrite(LED_PIN,HIGH);
  
  for ( int x = 0 ; x < 4 ; x++ ) {
    pinMode(GPIOPIN[x], OUTPUT);
  }

  /*
#ifdef I2CPULLUP
  //if you want to set the internal pullup
  digitalWrite( SDA, HIGH);
  digitalWrite( SCL, HIGH);
#else
  // here we enforce we do not want pullup
  digitalWrite( SDA, LOW);
  digitalWrite( SCL, LOW);
#endif
  */
  
  Wire.begin(SDA,SCL);
  Wire.setClock(I2C_CLOCK);
  
  delay(100);
  
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

  if (oledpresent) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 10); 
    u8g2.print(F("Starting"));
    u8g2.sendBuffer();
  }

  delay(1000);
  digitalWrite(LED_PIN,HIGH);

  mybridgef.stop();
  mybridgef.setrotation();
  
  mybridge2h.stop();
  mybridge2h.setpwm(0,IBT_2_R_HALF);
  mybridge2h.setpwm(0,IBT_2_L_HALF);
  
  LOGN(F("mounting FS..." CR));
  if (oledpresent) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 10); 
    u8g2.print(F("mounting FS..."));
    u8g2.sendBuffer();
    delay(3000);
  }
  if (SPIFFS.begin()) {
    Serial.println();  
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
      Serial.println();
    }
  } else {
    LOGN(F("failed to mount FS" CR));
    if (oledpresent) {
      u8g2.clearBuffer();
      u8g2.setCursor(0, 10); 
      u8g2.print(F("Failed Mount FS"));
      u8g2.sendBuffer();
      delay(3000);
    }
  }

  irrecv.enableIRIn();  // Start the receiver

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
    delay(1000);
  }

#ifdef  WIFI_APMODE
  
  IPAddress apIP(192, 168, 1, 1);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(WIFI_SSED,WIFI_PASSWORD  );

  // modify TTL associated  with the domain name (in seconds)
  // default is 60 seconds
  dnsServer.setTTL(300);
  // set which return code will be used for all other domains (e.g. sending
  // ServerFailure instead of NonExistentDomain will reduce number of queries
  // sent by clients)
  // default is DNSReplyCode::NonExistentDomain
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);


  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(53, "*", apIP);

#else
  
  configTime(0, 0, "0.europe.pool.ntp.org");
  void tryupgrade(){

#endif  
  
  server.on("/gpio", updateGpio);

  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/css", SPIFFS, "/css");
  server.serveStatic("/img", SPIFFS, "/img");
  server.serveStatic("/", SPIFFS, "/");

  server.begin();
  LOGN(F("HTTP server started" CR));

  if (oledpresent) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 10); 
    u8g2.print(F("Started:"));
    delay(1000);
  }

  display_status();
  
  LOGN(F("END setup" CR));
}
  
void loop() {
  // put your main code here, to run repeatedly:
  dnsServer.processNextRequest();
  server.handleClient();
  
  if (irrecv.decode(&results)){
    // Print Code in HEX
    //serialPrintUint64(results.value, HEX);
    //Serial.print(" : ");
    //Serial.print(results.decode_type);
    //Serial.print(" : ");
    //Serial.println(NEC);

    if (results.decode_type == DECODETYPE) {
      int8_t ind=-1;
      int8_t key=-1;
      int8_t value=0;

      switch(results.value){
      case KEYPAD_1: // 1 Keypad Button
	ind=0;
	lastkey=-1;
	break;

      case KEYPAD_2: // 2 Keypad Button
	ind=1;
	lastkey=-1;
	break;

      case KEYPAD_3: // 3 Keypad Button
	ind=2;
	lastkey=-1;
	break;

      case KEYPAD_4: // 4 Keypad Button
	ind=3;
	lastkey=-1;
	break;

      case KEYPAD_7:
	key=7;
	lastkey=key;
	break;

      case KEYPAD_8:
	key=8;
	lastkey=key;
	break;

      case KEYPAD_9:
	key=9;
	lastkey=key;
	break;
	
      case KEYPAD_MINUS:
	value=-10;
	lastvalue=value;
	break;

      case KEYPAD_PLUS:
	value=10;
	lastvalue=value;
	break;
	
      case REPEAT: // REPEAT code
	LOGN(F("received repeat value" CR));
	value=lastvalue;
	//key=lastkey;
	break;

      case KEYPAD_POWERDOWN:

	if (oledpresent) {
	  u8g2.clearBuffer();
	  u8g2.setCursor(0,10); 
	  u8g2.print(F("Power Down!"));
	  u8g2.sendBuffer();
	  u8g2.setPowerSave(0);
	}
	
	mybridgef.stop();
	mybridge2h.stop();
	for ( int x = 0 ; x < 4 ; x++ ) {
	  digitalWrite(GPIOPIN[x], LOW);
	}

	delay(5000);
	if (oledpresent) u8g2.setPowerSave(1);
	delay(100);
	

	while (true){
	  ESP.deepSleep(ESP.deepSleepMax());
	  delay(100);
	}
	break;
	
      default:
	LOGN(F("unknown key" CR));
	lastkey=-1;
	break;
      }

      if (results.repeat) LOGN(F("received repeat code" CR));
      LOGN(F("key: %d  lastkey: %d value: %d lastvalue: %d" CR),key,lastkey,value,lastvalue);

      
      if (key > 0){
	lastvalue=0;

	switch(key){
	case 7: // 7 Keypad Button
	  status7=!status7;
	  if (status7){
	    mybridgef.start();
	  }else{
	    mybridgef.stop();
	  }
      	  
	  break;

	case 8: // 8 Keypad Button
	  status8=!status8;
	  if (status8){
	    mybridge2h.start(IBT_2_L_HALF);
	  }else{
	    mybridge2h.stop(IBT_2_L_HALF);
	  }	  

	  break;

	case 9: // 9 Keypad Button
	  status9=!status9;
	  if (status9){
	    mybridge2h.start(IBT_2_R_HALF);
	  }else{
	    mybridge2h.stop(IBT_2_R_HALF);
	  }
      
	  break;
	  
	}
      }
      
      if (value != 0){
	switch(lastkey){
	case 7: // 7 Keypad Button
	  if (value > 0 && value7 < 250) value7+=value;
	  if (value < 0 && value7 >= 10) value7+=value;
	  if (value7 >= 245) value7=254;
	  if (value7 <= 9) value7=10;	  
	  mybridgef.setrotation(value7);

	  break;

	case 8: // 8 Keypad Button
	  if (value > 0 && value8 < 250) value8+=value;
	  if (value < 0 && value8 >= 10) value8+=value;
	  if (value8 >= 245) value8=254;
	  if (value8 <= 9) value8=10;
	  mybridge2h.setpwm(value8,IBT_2_L_HALF);

	  break;
	  
	case 9: // 9 Keypad Button
	  if (value > 0 && value9 < 250) value9+=value;
	  if (value < 0 && value9 >= 10) value9+=value;
	  if (value9 >= 245) value9=254;
	  if (value9 <= 9) value9=10;
	  mybridge2h.setpwm(value9,IBT_2_R_HALF);	  

	  break;
	  
	}
      }
      

      if (ind >= 0){
	// Toggle PIN On or Off
	digitalWrite( GPIOPIN[ind] , !digitalRead(GPIOPIN[ind]));
	LOGN(F("received key %d status %d" CR),ind+1,digitalRead(GPIOPIN[ind]));
      }

      display_status();
      
    }
    
    irrecv.resume();
  }

  if (digitalRead(RESET_PIN) == LOW) tryupgrade();  
  Alarm.delay(0);

}
