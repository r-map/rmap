/**********************************************************************
Copyright (C) 2019  Paolo Paruno <p.patruno@iperbole.bologna.it>
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
**********************************************************************/

/*********************************************************************
 *
 * This program implements measuremets of temperature and humidity
 * - calibration if required
 * - compute obsevation from samples
 * - show it on display
 * - serve a webserver over wifi and AP
 * - use an encoder for user interaction
 * 
**********************************************************************/

// define sensors in use
#define SENSOR_TEMPLATE_HIHADT        1
#define SENSOR_TEMPLATE_SHT           2
#define SENSOR_TEMPLATE_SHT_SPS_SCD   3

#define SENSOR_TEMPLATE SENSOR_TEMPLATE_SHT_SPS_SCD

// sensor definition
#if SENSOR_TEMPLATE == SENSOR_TEMPLATE_HIHADT
#define SENSORS_LEN 2
#elif SENSOR_TEMPLATE == SENSOR_TEMPLATE_SHT
#define SENSORS_LEN 1
#elif SENSOR_TEMPLATE == SENSOR_TEMPLATE_SHT_SPS_SCD
#define SENSORS_LEN 3
#endif

#define LENVALUES 4

//display definition
#define OLEDI2CADDRESS 0X3C

// Precipitations
#define PRECPIN   D3  //IO, 10k Pull-up

// H bridge
#define MR_PWM   D3
#define ML_PWM   D0
#define MR_EN    D4
#define ML_EN    D8
#define MR_IS    0XFF
#define ML_IS    0XFF

// I2C BUS
#define SCL D1
#define SDA D2

// rotary encoder pins
#define encBtn  D5
#define encA    D6
#define encB    D7

// define this if serial menu is required
//#define USESERIAL

// define this if hbridge is required
//#define HBRIDGE

// logging level at compile time
// Available levels are:
// LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
#define LOG_LEVEL   LOG_LEVEL_NOTICE

// disable debug at compile time but call function anyway
// this may significantly reduce your sketch/library size.
#define DISABLE_LOGGING disable

// file for saved configurations
#define FILESAVEDDATA "/saveddata.json"

// set the I2C frequency
#define I2C_CLOCK 10000
// #define I2CPULLUP define this if you want software pullup on I2C

#include <limits>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ArduinoLog.h>
#include <Wire.h>
#include <SensorDriverb.h>

#ifdef HBRIDGE
#include <ibt_2.h>
//#include <i2cibt_2.h>g
#else
#include <JC_Button.h> // https://github.com/JChristensen/JC_Button
#endif
#include <U8g2lib.h>
#include <menu.h>
#include <menuIO/u8g2Out.h>
#include <menuIO/RotaryIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <math.h>
#include "Calibration.h"
#include "FloatBuffer.h"
#include <Time.h>
#include <TimeAlarms.h>

#ifdef USESERIAL
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>
#endif

// display definitions
#define fontNameS u8g2_font_tom_thumb_4x6_tf
#define fontNameB u8g2_font_t0_11_tf
#define fontX 5
#define fontY 8
#define offsetX 1
#define offsetY 1
#define U8_Width 64
#define U8_Height 48
#define fontMarginX 1
#define fontMarginY 1

// calibration definitions
#define calibrationPoints 3

// sensor sample definitions
#define SAMPLERATE    5
#define SAMPLEPERIOD 60
#define NSAMPLE SAMPLEPERIOD/SAMPLERATE


// global variables for sensors measure
FloatBuffer t;
FloatBuffer u;
float tmean=NAN;
float umean=NAN;
long pm2=-999,pm10=-999,co2=-999;

struct sensor_t
{
  char driver[5];         // driver name
  char type[5];           // driver name
  int address;            // i2c address
} sensors[SENSORS_LEN];
static SensorDriver* sd[SENSORS_LEN];

//char* json;

// global variables for calibration
float trawmeasures[]={-50.,0.,50.};
float tmeasures[]={-50.,0.,50.};

float hrawmeasures[]={0.,50.,100.};
float hmeasures[]={0.,50.,100.};

calibration::Calibration tcal,hcal;


#ifdef HBRIDGE
// global variables H bridge
ibt_2 hbridge(IBT_2_2HALF,MR_PWM,ML_PWM,MR_EN ,ML_EN ,MR_IS ,ML_IS);
//i2cgpio gpio(I2C_GPIO_DEFAULTADDRESS);
//i2cibt_2 hbridge(IBT_2_2HALF,gpio);
#else
unsigned long debounce=25;
Button precBtn(PRECPIN,debounce); // define the button
volatile uint8_t prec=0;
#endif

// global variable for network (apn, dns server and web server)
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
const byte DNS_PORT = 53;
ESP8266WebServer webserver(80);

// global variables for display

bool displaydata=false;

U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0);

// define menu colors
// each color is in the format:
//  {{disabled normal,disabled selected},{enabled normal,enabled selected, enabled editing}}
// this is a monochromatic color table
const colorDef<uint8_t> colors[] MEMMODE={
  {{0,0},{0,1,1}},//bgColor
  {{1,1},{1,0,0}},//fgColor
  {{1,1},{1,0,0}},//valColor
  {{1,1},{1,0,0}},//unitColor
  {{0,1},{0,0,1}},//cursorColor
  {{1,1},{1,0,0}},//titleColor
};


// global variables for sensors state machine
static unsigned long s_start_wait;
enum s_states {
	       UNKNOWN
	       ,SETUP
	       ,PREPARE
	       ,WAIT
	       ,GET
	       ,IDLE
} s_state= UNKNOWN;

enum s_events {
	       START_MEASURE
	       ,START_WAIT
	       ,NONE
}s_event=NONE;

#ifdef HBRIDGE
// global variables for manual control of ventilation
int ventCtrl=HIGH;
float vent=100.;

// global definition for menu
result ventOn();
result ventOff();

TOGGLE(ventCtrl,setVent,"Vent: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("On",HIGH,ventOn,noEvent)
  ,VALUE("Off",LOW,ventOff,noEvent)
);
#else
void ICACHE_RAM_ATTR button1changed()
{
  precBtn.read();
  if (precBtn.wasReleased()) prec++;
}

#endif
result save();
result resetprec();
result changeDebounce();

#define MAX_DEPTH 2

MENU(tempMenu,"Temperature",doNothing,noEvent,noStyle
     ,FIELD(trawmeasures[0],"uncal1","%",-50.,50.,1.,.1,doNothing,noEvent,wrapStyle)
     ,FIELD(tmeasures[0],   "  cal1","%",-50.,50.,1.,.1,doNothing,noEvent,wrapStyle)
     ,FIELD(trawmeasures[1],"uncal2","%",-50.,50.,1.,.1,doNothing,noEvent,wrapStyle)
     ,FIELD(tmeasures[1],   "  cal2","%",-50.,50.,1.,.1,doNothing,noEvent,wrapStyle)
     ,FIELD(trawmeasures[2],"uncal3","%",-50.,50.,1.,.1,doNothing,noEvent,wrapStyle)
     ,FIELD(tmeasures[2],   "  cal3","%",-50.,50.,1.,.1,doNothing,noEvent,wrapStyle)
     ,EXIT("<Back")
     );

MENU(humidMenu,"Humidity",doNothing,noEvent,noStyle
     ,FIELD(hrawmeasures[0],"uncal1","%",0.,100.,10,1,doNothing,noEvent,wrapStyle)
     ,FIELD(hmeasures[0],   "  cal1","%",0.,100.,10,1,doNothing,noEvent,wrapStyle)
     ,FIELD(hrawmeasures[1],"uncal2","%",0.,100.,10,1,doNothing,noEvent,wrapStyle)
     ,FIELD(hmeasures[1],   "  cal2","%",0.,100.,10,1,doNothing,noEvent,wrapStyle)
     ,FIELD(hrawmeasures[2],"uncal3","%",0.,100.,10,1,doNothing,noEvent,wrapStyle)
     ,FIELD(hmeasures[2],   "  cal3","%",0.,100.,10,1,doNothing,noEvent,wrapStyle)
     ,EXIT("<Back")
     );

MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
     ,OP("Save!",save,enterEvent)
     ,OP("reset prec",resetprec,enterEvent)
     ,FIELD(debounce,"DB t","ms",10,200,10,1,changeDebounce,enterEvent,wrapStyle)
     ,SUBMENU(tempMenu)
     ,SUBMENU(humidMenu)
#ifdef HBRIDGE
     ,SUBMENU(setVent)
     ,FIELD(vent,"Vent","%",0.0,100,10,1,doNothing,noEvent,wrapStyle)
#endif
     ,EXIT("<Exit")
     );

encoderIn<encA,encB> encoder;//simple quad encoder driver
encoderInStream<encA,encB> encStream(encoder);// simple encoder Stream


//a keyboard with only one key as the encoder button
keyMap encBtn_map[]={{-encBtn,defaultNavCodes[enterCmd].ch}};//negative pin numbers use internal pull-up, this is on when low
keyIn<1> encButton(encBtn_map);//1 is the number of keys

#ifdef USESERIAL
serialIn serial(Serial);
MENU_INPUTS(in,&encStream,&encButton,&serial);
#else
MENU_INPUTS(in,&encStream,&encButton);
#endif

idx_t gfx_tops[MAX_DEPTH];

PANELS(gfxPanels,{0,0,U8_Width/fontX,U8_Height/fontY});
u8g2Out oledOut(u8g2,colors,gfx_tops,gfxPanels,fontX,fontY,offsetX,offsetY,fontMarginX,fontMarginY);

#ifdef USESERIAL
idx_t serialTops[MAX_DEPTH]={0};
serialOut outSerial(*(Print*)&Serial,serialTops);
//define outputs controller
menuOut* outputs[]{&oledOut,&outSerial};//list of output devices
#else
menuOut* outputs[]{&oledOut};//list of output devices
#endif

outputsList out(outputs,sizeof(outputs)/sizeof(menuOut*));//outputs list controller

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

#ifdef HBRIDGE
// power on the ventilation
result ventOn(){
  LOGN(F("vent ON" CR));
  hbridge.start(IBT_2_L_HALF);
  return proceed;
}

// power off the ventilation
result ventOff(){
  LOGN(F("vent OFF" CR));
  hbridge.stop(IBT_2_L_HALF);
  return proceed;
}
#endif

// reset precipitation count
result resetprec() {
  prec=0;
}

// chenge debounce runtime
result changeDebounce() {
  LOGN(F("changedebounce %d" CR),debounce);
  precBtn.setDbTime(debounce);
}

// save configuration
result save() {

  LOGN(F("Save config" CR));

  tcal.setCalibrationPoints(trawmeasures,tmeasures, calibrationPoints, 1);
  hcal.setCalibrationPoints(hrawmeasures,hmeasures, calibrationPoints, 1);

  for (int i = 0; i < calibrationPoints; i++) {
    LOGN(F("trawmeasures %F : %d\n"),trawmeasures[i],i);
    LOGN(F("tmeasures    %F : %d\n"),tmeasures[i],i);
    LOGN(F("hrawmeasures %F : %d\n"),hrawmeasures[i],i);
    LOGN(F("hmeasures    %F : %d\n"),hmeasures[i],i);
  }

  LOGN(F("debounce   : %d\n"),debounce);

  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  
#ifdef HBRIDGE
  LOGN(F("vent     %D" CR),vent);
  LOGN(F("ventctrl %D" CR),ventCtrl);
  
  json["vent"] = vent;
  json["ventctrl"] = ventCtrl;
#endif

  json["debounce"] = debounce;
  
  JsonArray& jsontrawmeasures =json.createNestedArray("trawmeasures");
  for (int i = 0; i < calibrationPoints; i++) {
    jsontrawmeasures.add(trawmeasures[i]);
  }
  JsonArray& jsontmeasures =json.createNestedArray("tmeasures");
  for (int i = 0; i < calibrationPoints; i++) {
    jsontmeasures.add(tmeasures[i]);
  }

  JsonArray& jsonhrawmeasures =json.createNestedArray("hrawmeasures");
  for (int i = 0; i < calibrationPoints; i++) {
    jsonhrawmeasures.add(hrawmeasures[i]);
  }
  JsonArray& jsonhmeasures =json.createNestedArray("hmeasures");
  for (int i = 0; i < calibrationPoints; i++) {
    jsonhmeasures.add(hmeasures[i]);
  }
  
  File configFile = SPIFFS.open(FILESAVEDDATA, "w");
  if (!configFile) {
    LOGE(F("failed to open config file for writing" CR));
  }else{
    //json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    LOGN(F("saved parameter" CR));
  }
}

String read_savedparams() {

  LOGN(F("mounted file system" CR));
  if (SPIFFS.exists(FILESAVEDDATA)) {
    //file exists, reading and loading
    LOGN(F("reading config file" CR));
    File configFile = SPIFFS.open(FILESAVEDDATA, "r");
    if (configFile) {
      LOGN(F("opened config file" CR));

      //size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      //std::unique_ptr<char[]> buf(new char[size]);
      //configfile.readBytes(buf.get(), size);

      return configFile.readString();
      
    } else {
      LOGE(F("error reading params file" CR));	
    }
  } else {
    LOGN(F("params file do not exist" CR));
  }
  //end read
  return String();  
}


//when menu is suspended
result idle(menuOut& o,idleEvent e) {
  o.clear();
  switch(e) {
  case idleStart:
    o.println("suspending menu!");
    break;
  case idling:
    o.println("suspended...");
    displaydata=true;
    break;
  case idleEnd:
    o.println("resuming menu.");
    displaydata=false;
    break;
  }
  return proceed;
}

// ISR for encoder management
void ICACHE_RAM_ATTR encoderprocess (){
  encoder.process();
}


// set start measure event on sensor machine
void start_measure(){
  LOGN(F("event START_MEASURE" CR));
  s_event=START_MEASURE;
}


// sensor machine
void sensor_machine(){
  unsigned long waittime;
  static unsigned long maxwaittime=0;

  switch(s_state) {
  case UNKNOWN:

    for (int i = 0; i < SENSORS_LEN; i++) {

      LOGN(F("driver: %s" CR),sensors[i].driver);
      LOGN(F("type: %s" CR),sensors[i].type);      
      
      sd[i]=SensorDriver::create(sensors[i].driver,sensors[i].type);
      if (sd[i] == 0){
	LOGN(F("%s: driver not created !" CR),sensors[i].driver);
      }else{

	if (!(sd[i]->setup(sensors[i].driver, sensors[i].address, -1, sensors[i].type) == SD_SUCCESS)) {
	  LOGE(F("sensor not present or broken" CR));
	}		
      }
    }

    s_state = IDLE;
    break;
    
  case IDLE:
    switch(s_event) {
    case START_MEASURE:
      s_event = NONE;
      s_state = PREPARE;
      break;
    default:
      return;
      break;
    }
    break;

  case PREPARE:
    // prepare sensors to measure
    for (int i = 0; i < SENSORS_LEN; i++) {
      if (!sd[i] == 0){
	if (sd[i]->prepare(waittime) == SD_SUCCESS){
	  //Serial.print(sensors[i].driver);
	  //Serial.print(" : ");
	  //Serial.print(sensors[i].type);
	  //Serial.println(" : Prepare OK");
	  maxwaittime=max(maxwaittime,waittime);
	}else{
	  LOGN(F("%s : %s : Prepare failed!" CR),sensors[i].driver, sensors[i].type);
	}
      }
    }

    LOGN(F("max wait time: %d" CR), maxwaittime);

    s_state = WAIT;
    s_event = START_WAIT;
    break;
 
  case WAIT:
    switch(s_event) {
    case START_WAIT:
      s_event = NONE;
      s_start_wait=millis();
      break;
    default:

      //wait sensors to go ready
      //Serial.print("# wait sensors for ms:");  Serial.println(maxwaittime);
      //delay(maxwaittime);  // 500 for tmp and 250 for adt and 2500 for davis
      
      if ((millis()-s_start_wait) >= maxwaittime) {
	s_state = GET;
      }
      return;
      break;
    }
    break;

  case GET:

    pm2=-999;
    pm10=-999;
    co2=-999;

    if (displaydata){
      u8g2.setFont(fontNameB);
      //u8g2.setFontMode(0); // enable transparent mode, which is faster
      u8g2.clearBuffer();
    }
  
    for (int i = 0; i < SENSORS_LEN; i++) {
      if (!sd[i] == 0){
	// get integers values 
	long values[LENVALUES];
	size_t lenvalues=LENVALUES;
	
	if (sd[i]->get(values,lenvalues) == SD_SUCCESS){
	  for (size_t ii = 0; ii < lenvalues; ii++) {
	    LOGN(F("sensor:%d element:%d value:%d " CR),i,ii,values[ii]);
	  }
	  
	  if (i == 0){
	    float U_Input;
	    
	    //U_Input=float(values[0]);

#if SENSOR_TEMPLATE == SENSOR_TEMPLATE_HIHADT
	    hcal.getConcentration(float(values[0]),&U_Input);
#elif SENSOR_TEMPLATE == SENSOR_TEMPLATE_SHT || SENSOR_TEMPLATE == SENSOR_TEMPLATE_SHT_SPS_SCD
	    hcal.getConcentration(float(values[1]),&U_Input);
#endif
	    
	    LOGN(F("calibrated U: %F" CR),U_Input);
	    
	    if (displaydata){
	      u8g2.setCursor(0, 12); 
	      u8g2.print("U:");
	      u8g2.setCursor(30, 12); 
	    }
	    u.autoput(U_Input);
	    LOGN(F("U size %d" CR),u.getSize());
	    
	    if (u.getSize() == u.getCapacity()){
	      umean=0;
	      for ( uint8_t i=0 ; i < u.getCapacity() ; i++)  {
		LOGN(F("U ele %d %F" CR),i,u.peek(i));
		umean += (u.peek(i) - umean) / (i+1);
	      }
	      if (displaydata) u8g2.print(round(umean),0);
	    }else{
	    if (displaydata) u8g2.print("wait");
	    }
	    
	    if (displaydata){
	      //u8g2.setCursor(0, 36); 
	      //u8g2.print("t:");
	      //u8g2.setCursor(30, 36); 
	      //u8g2.print(round(T_Input*10.)/10.,1);
	    }
#if SENSOR_TEMPLATE == SENSOR_TEMPLATE__HIHADT
	  }
	  if (i == 1){
#endif
	    float T_Input;
	    tcal.getConcentration(float(values[0])/100.-273.15,&T_Input);
	    LOGN(F("calibrated T: %F" CR),T_Input);
	    
	    if (displaydata) {
	      u8g2.setCursor(0, 24); 
	      u8g2.print("T:");
	      u8g2.setCursor(30, 24); 
	    }
	    t.autoput(T_Input);
	    if (t.getSize() == t.getCapacity()){
	      tmean=0;
	      for ( uint8_t i=0 ; i < t.getCapacity() ; i++)  {
		tmean += (t.peek(i) - tmean) / (i+1);
	      }
	      if (displaydata) u8g2.print(round(tmean*10.)/10.,1);	    	    
	    } else{
	      if (displaydata) u8g2.print("wait");	    	    
	    }
	  }

#if SENSOR_TEMPLATE == SENSOR_TEMPLATE_SHT || SENSOR_TEMPLATE == SENSOR_TEMPLATE_SHT_SPS_SCD

	  if (i == 1){

	    pm2=round(values[1]/10.);
	    pm10=round(values[3]/10.);
	    if (displaydata) {
	      u8g2.setCursor(0, 36); 
	      u8g2.print("PM2:");
	      u8g2.setCursor(30, 36); 
	      u8g2.print(round(values[1]/10.),0);	    	    
	      /*
	      u8g2.setCursor(0, 48); 
	      u8g2.print("PM10:");
	      u8g2.setCursor(30, 48); 
	      u8g2.print(round(values[3]/10.),0);	    	    
	      */
	    }
	  }

	  if (i == 2){
	    co2=round(values[0]/1.8);
	    if (displaydata) {
	      u8g2.setCursor(0, 48); 
	      u8g2.print("CO2:");
	      u8g2.setCursor(30, 48); 
	      u8g2.print(round(values[0]/1.8),0);	    	    
	    }
	  }
	  
#endif
	}else{
	  
	  if (displaydata){
	    LOGE(F("Error on sensor: disable" CR));
	    //u8g2.clearBuffer();
	    u8g2.setCursor(0, 10); 
	    u8g2.print("Error Sensor");
	    //u8g2.setCursor(0, 20); 
	    //u8g2.print("Disable");
	  }	  

	}  
      }
    }

    if (displaydata){
      u8g2.sendBuffer();
      u8g2.setFont(fontNameS);
    }
    s_state = IDLE;
    break;
    
  default:
    LOGN(F("Something go wrong in sensor_machine"));
    break;
    
  }
  return;
}


// web server response function
void handle_OnConnect() {
  webserver.send(200, "text/html", SendHTML()); 
}

void handle_NotFound(){
  webserver.send(404, "text/plain", "Not found");
}


// function to prepare HTML response
//https://lastminuteengineers.com/esp8266-dht11-dht22-web-server-tutorial/
String SendHTML(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Data Report</title>\n";
  ptr +="<style>html { display: block; margin: 0px auto; text-align: center;color: #333333;}\n";
  ptr +="body{margin-top: 50px;}\n";
  ptr +="h1 {margin: 50px auto 30px;}\n";
  ptr +=".side-by-side{display: inline-block;vertical-align: middle;position: relative;}\n";
  ptr +=".humidity-icon{background-color: #3498db;width: 30px;height: 30px;border-radius: 50%;line-height: 36px;}\n";
  ptr +=".humidity-text{font-weight: 600;padding-left: 15px;font-size: 19px;width: 160px;text-align: left;}\n";
  ptr +=".humidity{font-weight: 300;font-size: 60px;color: #3498db;}\n";
  ptr +=".temperature-icon{background-color: #f39c12;width: 30px;height: 30px;border-radius: 50%;line-height: 40px;}\n";
  ptr +=".temperature-text{font-weight: 600;padding-left: 15px;font-size: 19px;width: 160px;text-align: left;}\n";
  ptr +=".temperature{font-weight: 300;font-size: 60px;color: #f39c12;}\n";
  ptr +=".superscript{font-size: 17px;font-weight: 600;position: relative;right: -20px;top: -10px;}\n";
  ptr +=".data{padding: 10px;}\n";
  ptr +="</style>\n";
  ptr +="<script>\n";
  ptr +="setInterval(loadDoc,200);\n";
  ptr +="function loadDoc() {\n";
  ptr +="var xhttp = new XMLHttpRequest();\n";
  ptr +="xhttp.onreadystatechange = function() {\n";
  ptr +="if (this.readyState == 4 && this.status == 200)\n";
  ptr +="{document.getElementById(\"webpage\").innerHTML =this.responseText}\n";
  ptr +="if (this.readyState == 4 && this.status != 200)\n";
  ptr +="{document.getElementById(\"webpage\").innerHTML =\"not connected\"}\n";
  ptr +="};\n";
  ptr +="xhttp.open(\"GET\", \"/\", true);\n";
  ptr +="xhttp.send();\n";
  ptr +="}\n";
  ptr +="</script>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  
  ptr +="<div id=\"webpage\">\n";
  
  ptr +="<h1>Data Report</h1>\n";
  ptr +="<div class=\"data\">\n";
  ptr +="<div class=\"side-by-side temperature-icon\">\n";
  ptr +="<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n";
  ptr +="width=\"9.915px\" height=\"22px\" viewBox=\"0 0 9.915 22\" enable-background=\"new 0 0 9.915 22\" xml:space=\"preserve\">\n";
  ptr +="<path fill=\"#FFFFFF\" d=\"M3.498,0.53c0.377-0.331,0.877-0.501,1.374-0.527C5.697-0.04,6.522,0.421,6.924,1.142\n";
  ptr +="c0.237,0.399,0.315,0.871,0.311,1.33C7.229,5.856,7.245,9.24,7.227,12.625c1.019,0.539,1.855,1.424,2.301,2.491\n";
  ptr +="c0.491,1.163,0.518,2.514,0.062,3.693c-0.414,1.102-1.24,2.038-2.276,2.594c-1.056,0.583-2.331,0.743-3.501,0.463\n";
  ptr +="c-1.417-0.323-2.659-1.314-3.3-2.617C0.014,18.26-0.115,17.104,0.1,16.022c0.296-1.443,1.274-2.717,2.58-3.394\n";
  ptr +="c0.013-3.44,0-6.881,0.007-10.322C2.674,1.634,2.974,0.955,3.498,0.53z\"/>\n";
  ptr +="</svg>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"side-by-side temperature-text\">Temperature</div>\n";
  ptr +="<div class=\"side-by-side temperature\">";
  ptr +=round(tmean*10.)/10.;
  ptr +="<span class=\"superscript\">°C</span></div>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"data\">\n";
  ptr +="<div class=\"side-by-side humidity-icon\">\n";
  ptr +="<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n\"; width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 13 17.955\" xml:space=\"preserve\">\n";
  ptr +="<path fill=\"#FFFFFF\" d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n";
  ptr +="c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C-0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n";
  ptr +="</svg>\n";
  ptr +="</div>\n";
  ptr +="<div class=\"side-by-side humidity-text\">Humidity</div>\n";
  ptr +="<div class=\"side-by-side humidity\">";
  ptr +=round(umean);
  ptr +="<span class=\"superscript\">%</span></div>\n";
  ptr +="</div>\n";

  ptr +="<div class=\"data\">\n";
  ptr +="<div class=\"side-by-side humidity-text\">Precipitation</div>\n";
  ptr +="<div class=\"side-by-side humidity\">";
  ptr +=prec;
  ptr +="<span class=\"superscript\">N</span></div>\n";
  ptr +="</div>\n";

  ptr +="<div class=\"data\">\n";
  ptr +="<div class=\"side-by-side temperature-text\">PM2.5</div>\n";
  ptr +="<div class=\"side-by-side temperature\">";
  ptr +=pm2;
  ptr +="<span class=\"superscript\">ug/m3</span></div>\n";
  ptr +="</div>\n";

  ptr +="<div class=\"data\">\n";
  ptr +="<div class=\"side-by-side temperature-text\">PM10</div>\n";
  ptr +="<div class=\"side-by-side temperature\">";
  ptr +=pm10;
  ptr +="<span class=\"superscript\">ug/m3</span></div>\n";
  ptr +="</div>\n";

  ptr +="<div class=\"data\">\n";
  ptr +="<div class=\"side-by-side temperature-text\">CO2</div>\n";
  ptr +="<div class=\"side-by-side temperature\">";
  ptr +=co2;
  ptr +="<span class=\"superscript\">ppm</span></div>\n";
  ptr +="</div>\n";

  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

#ifdef HBRIDGE
// update the display about ventilation
void do_display(){
  if (displaydata){
    u8g2.setFont(fontNameS);
    u8g2.setCursor(0, 49); 
    u8g2.print("Vent:");
    u8g2.setCursor(25, 49); 
    u8g2.print(vent);
    u8g2.setCursor(55, 49); 
    u8g2.print(ventCtrl);
    u8g2.sendBuffer();
  }
}
#else
void do_display_prec(){
  if (displaydata){
    u8g2.setFont(fontNameB);
    u8g2.setCursor(0, 40); 
    u8g2.print("P:");
    u8g2.setCursor(30, 40); 
    u8g2.print(prec);	    	    
    u8g2.setFont(fontNameS);
    u8g2.sendBuffer();
  }
}
#endif

// management of H bridge and update display
void do_etc(){

#ifdef HBRIDGE
  hbridge.setpwm(int(vent*255.0/100.0),IBT_2_L_HALF);
#endif

  nav.doInput();
  if (nav.changed(0)) {//only draw if menu changed for gfx device
    u8g2.firstPage();
    do nav.doOutput(); while(u8g2.nextPage());
  }  
}

void setup()
{

  // start up the serial interface
  Serial.begin(115200);
  // Pass log level, whether to show log level, and print interface.
  // Available levels are:
  // LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
  // Note: if you want to fully remove all logging code, change #define LOG_LEVEL ....
  //       this will significantly reduce your project size

  // set runtime log level to the same of compile time
  Log.begin(LOG_LEVEL, &Serial);
  LOGN(F("Started" CR));

#ifdef I2CPULLUP
  //if you want to set the internal pullup
  digitalWrite( SDA, HIGH);
  digitalWrite( SCL, HIGH);
#else
  // here we enforce we do not want pullup
  digitalWrite( SDA, LOW);
  digitalWrite( SCL, LOW);
#endif
  
  // start up the i2c interface
  Wire.begin(SDA,SCL);
  Wire.setClock(I2C_CLOCK);

  delay(1000);

  // start up display
  u8g2.setI2CAddress(OLEDI2CADDRESS*2);
  u8g2.begin();
  u8g2.setFont(fontNameS);
  u8g2.setFontMode(0); // enable transparent mode, which is faster
  u8g2.clearBuffer();
  u8g2.setCursor(0, 10); 
  u8g2.print(F("Starting up!"));
  u8g2.sendBuffer();

  delay(1000);
  
  //read configuration from FS in json format
  LOGN(F("mounting FS..." CR));
  if (!SPIFFS.begin()) {
    LOGE(F("failed to mount FS" CR));
    LOGN(F("Reformat SPIFFS" CR));
    SPIFFS.format();
    if (!SPIFFS.begin()) {
      LOGN(F("failed to mount FS" CR));
    }
    // messages on display
    u8g2.clearBuffer();
    u8g2.setCursor(0, 10); 
    u8g2.print(F("Mount FS"));
    u8g2.setCursor(0, 20); 
    u8g2.print(F("Failed"));
    u8g2.setCursor(0, 30); 
    u8g2.print(F("RESET"));
    u8g2.setCursor(0, 40); 
    u8g2.print(F("parameters"));
    u8g2.sendBuffer();
    delay(3000);
  }

  String savedparams=read_savedparams();
  if ( savedparams== String()) {
    LOGN(F("station configuration not found!" CR));
  }else{
    //Serial.println(savedparams);
    StaticJsonBuffer<500> jsonBuffer;
    JsonObject& json =jsonBuffer.parseObject(savedparams);
    if (!json.success()) {
      LOGE(F("reading json data" CR));
    }else{
#ifdef HBRIDGE      
      if (json.containsKey("vent"))     vent=json["vent"];
      if (json.containsKey("ventctrl")) ventCtrl=json["ventctrl"];      
      LOGN(F("vent     %D" CR),vent);
      LOGN(F("ventctrl %D" CR),ventCtrl);
#endif

      if (json.containsKey("debounce")) debounce=json["debounce"];      
      LOGN(F("debounce     %D" CR),debounce);
      
      if (json.containsKey("trawmeasures") && json.containsKey("tmeasures")) {
	for (int i = 0; i < calibrationPoints; i++) {
	  trawmeasures[i]=json["trawmeasures"][i];
	  tmeasures[i]=json["tmeasures"][i];
	}
      }

      if (json.containsKey("hrawmeasures") && json.containsKey("hmeasures")) {
	for (int i = 0; i < calibrationPoints; i++) {
	  hrawmeasures[i]=json["hrawmeasures"][i];
	  hmeasures[i]=json["hmeasures"][i];
	}
      }
    }
  }

#ifdef HBRIDGE
  // start up H bridge
  hbridge.start(IBT_2_R_HALF);

  if (ventCtrl) {
    LOGN(F("vent ON" CR));
    hbridge.start(IBT_2_L_HALF);
  }else{
    LOGN(F("vent OFF" CR));
    hbridge.stop(IBT_2_L_HALF);
  }
#else
  // Configuramos los pines de interrupciones para que
  // detecten un cambio
  precBtn.begin(); // initialize the button object
  attachInterrupt(digitalPinToInterrupt(PRECPIN),button1changed , CHANGE);
#endif

  // define which sensors are connected

#if SENSOR_TEMPLATE == SENSOR_TEMPLATE_HIHADT

  // HIH humidity and temperature sensor
  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"HIH");
  sensors[0].address=39;

  // ADT humidity and temperature sensor
  strcpy(sensors[1].driver,"I2C");
  strcpy(sensors[1].type,"ADT");
  sensors[1].address=73;

#elif SENSOR_TEMPLATE == SENSOR_TEMPLATE_SHT

  // SHT humidity and temperature sensor
  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"SHT");
  sensors[0].address=68;

#elif SENSOR_TEMPLATE == SENSOR_TEMPLATE_SHT || SENSOR_TEMPLATE == SENSOR_TEMPLATE_SHT_SPS_SCD

  // SHT humidity and temperature sensor
  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"SHT");
  sensors[0].address=68;
  
  strcpy(sensors[1].driver,"I2C");
  strcpy(sensors[1].type,"SPS");
  sensors[1].address=105;

  strcpy(sensors[2].driver,"I2C");
  strcpy(sensors[2].type,"SCD");
  sensors[2].address=97;
  
#endif

  // SPS PM sensor
  // strcpy(sensors[1].driver,"I2C");
  // strcpy(sensors[1].type,"SPS");
  // sensors[1].address=0;
  
  // start up encoder
  encoder.begin();

  // start up button
  encButton.begin();
  
  // encoder with interrupt on the A & B pins
  attachInterrupt(digitalPinToInterrupt(encA), encoderprocess, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB), encoderprocess, CHANGE);

  // setup menu
  nav.idleTask=idle;//point a function to be used when menu is suspended
  nav.timeOut=10;
  nav.exit();

  // setup PWM
  analogWriteRange(255);
  //analogWriteFreq(100);

  // setup calibration
  tcal.setCalibrationPoints(trawmeasures,tmeasures, calibrationPoints, 1);
  hcal.setCalibrationPoints(hrawmeasures,hmeasures, calibrationPoints, 1);

  for (int i = 0; i < calibrationPoints; i++) {
    LOGN(F("trawmeasures %F : %d\n"),trawmeasures[i],i);
    LOGN(F("tmeasures    %F : %d\n"),tmeasures[i],i);
    LOGN(F("hrawmeasures %F : %d\n"),hrawmeasures[i],i);
    LOGN(F("hmeasures    %F : %d\n"),hmeasures[i],i);
  }

  t.init(NSAMPLE);
  u.init(NSAMPLE);

  // setup AP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("stima-WiFi");

  // setup DNS
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  
  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  // setup web server
  webserver.on("/", handle_OnConnect);
  webserver.onNotFound(handle_NotFound);
  
  webserver.begin();
  LOGN(F("HTTP server started" CR));

  // start sensor machine
  sensor_machine();

  // setup alarms
  Alarm.timerRepeat(SAMPLERATE, start_measure);            // timer for every second    
  
  LOGN(F("setup done." CR));

  Serial.println("Menu 4.x");
#ifdef USESERIAL
  Serial.println("Use keys + - * /");
  Serial.println("to control the menu navigation");
#endif
  
}

void loop()
{
  Alarm.delay(0);
  sensor_machine();
  webserver.handleClient();
  dnsServer.processNextRequest();
#ifdef HBRIDGE
  do_display();
#else
#if SENSOR_TEMPLATE == SENSOR_TEMPLATE_HIHADT || SENSOR_TEMPLATE == SENSOR_TEMPLATE_SHT 
  do_display_prec();
#endif
#endif
  do_etc();
}
