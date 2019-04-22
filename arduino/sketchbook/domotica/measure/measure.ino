/**********************************************************************
Copyright (C) 2019  Paolo Paruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Paruno <p.patruno@iperbole.bologna.it>

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


#define SENSORS_LEN 2
#define LENVALUES 2
#define OLEDI2CADDRESS 0X3C

#define MR_PWM   D3
#define ML_PWM   D0
#define MR_EN    D4
#define ML_EN    D8
#define MR_IS    0XFF
#define ML_IS    0XFF
#define SCL D1
#define SDA D2
// rotary encoder pins
#define encBtn  D5
#define encA    D6
#define encB    D7

//#define USESERIAL

// logging level at compile time
// Available levels are:
// LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
#define LOG_LEVEL   LOG_LEVEL_VERBOSE

//disable debug at compile time but call function anyway
//#define DISABLE_LOGGING disable

#define FILESAVEDDATA "/saveddata.json"

#include <limits>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ArduinoLog.h>
#include <Wire.h>
#include <SensorDriverb.h>
#include <ibt_2.h>
//#include <i2cibt_2.h>g
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

#define calibrationPoints 3

#define SAMPLERATE    1
#define SAMPLEPERIOD 60
#define NSAMPLE SAMPLEPERIOD/SAMPLERATE

FloatBuffer t;
FloatBuffer u;
float tmean=NAN;
float umean=NAN;
	 
struct sensor_t
{
  char driver[5];         // driver name
  char type[5];           // driver name
  int address;            // i2c address
} sensors[SENSORS_LEN];
SensorDriver* sd[SENSORS_LEN];

char* json;

float trawmeasures[]={-50.,0.,50.};
float tmeasures[]={-50.,0.,50.};

float hrawmeasures[]={0.,50.,100.};
float hmeasures[]={0.,50.,100.};

// Sensor calibration
calibration::Calibration tcal,hcal;

ibt_2 hbridge(IBT_2_2HALF,MR_PWM,ML_PWM,MR_EN ,ML_EN ,MR_IS ,ML_IS);
//i2cgpio gpio(I2C_GPIO_DEFAULTADDRESS);
//i2cibt_2 hbridge(IBT_2_2HALF,gpio);

IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
const byte DNS_PORT = 53;
ESP8266WebServer server(80);

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

// sensors state machine
unsigned long s_start_wait;
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

int ventCtrl=HIGH;

result ventOn(){
  LOGN(F("vent ON" CR));
  hbridge.start(IBT_2_L_HALF);
  return proceed;
}
result ventOff(){
  LOGN(F("vent OFF" CR));
  hbridge.stop(IBT_2_L_HALF);
  return proceed;
}


TOGGLE(ventCtrl,setVent,"Vent: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("On",HIGH,ventOn,noEvent)
  ,VALUE("Off",LOW,ventOff,noEvent)
);

float vent=100.;

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
  
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  
  LOGN(F("vent     %D" CR),vent);
  LOGN(F("ventctrl %D" CR),ventCtrl);
    
  json["vent"] = vent;
  json["ventctrl"] = ventCtrl;
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
     //,FIELD(umid,"Umid    ","%",10.0,100,10,1,doNothing,noEvent,wrapStyle)
     ,SUBMENU(tempMenu)
     ,SUBMENU(humidMenu)
     ,SUBMENU(setVent)
     ,FIELD(vent,"Vent","%",0.0,100,10,1,doNothing,noEvent,wrapStyle)
     ,EXIT("<Exit")
     );

#define MAX_DEPTH 2

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

bool displaydata=false;

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
void encoderprocess (){
  encoder.process();
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


void start_measure(){
  s_event=START_MEASURE;
}

void sensor_machine(){
  long unsigned int waittime,maxwaittime=0;

  switch(s_state) {
  case UNKNOWN:

    for (int i = 0; i < SENSORS_LEN; i++) {
      
      sd[i]=SensorDriver::create(sensors[i].driver,sensors[i].type);
      if (sd[i] == 0){
	LOGN(F("%s: driver not created !" CR),sensors[i].driver);
      }else{
	sd[i]->setup(sensors[i].driver,sensors[i].address);
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
	    float T_Input;
	    
	    //U_Input=float(values[0]);	   
	    hcal.getConcentration(float(values[0]),&U_Input);
	    T_Input=float(values[1])/100.-273.15;
	    
	    LOGN(F("calibrated U: %F" CR),U_Input);
	    
	    u8g2.setCursor(0, 12); 
	    u8g2.print("U:");
	    u8g2.setCursor(25, 12); 
	    
	    u.autoput(U_Input);
	    LOGN(F("U size %d" CR),u.getSize());
	    
	    if (u.getSize() == u.getCapacity()){
	      umean=0;
	      for ( uint8_t i=0 ; i < u.getCapacity() ; i++)  {
		LOGN(F("U ele %d %F" CR),i,u.peek(i));
		umean += (u.peek(i) - umean) / (i+1);
	      }
	      u8g2.print(round(umean),0);
	    }else{
	    u8g2.print("wait");
	    }
	    
	    
	    u8g2.setCursor(0, 36); 
	    u8g2.print("t:");
	    u8g2.setCursor(25, 36); 
	    u8g2.print(round(T_Input*10.)/10.,1);	    	    
	  }
	  
	  if (i == 1){
	    float T_Input;
	    tcal.getConcentration(float(values[0])/100.-273.15,&T_Input);
	    LOGN(F("calibrated T: %F" CR),T_Input);
	    
	    u8g2.setCursor(0, 24); 
	    u8g2.print("T:");
	    u8g2.setCursor(25, 24); 
	    
	    t.autoput(T_Input);
	    if (t.getSize() == t.getCapacity()){
	      tmean=0;
	      for ( uint8_t i=0 ; i < t.getCapacity() ; i++)  {
		tmean += (t.peek(i) - tmean) / (i+1);
	      }
	      u8g2.print(round(tmean*10.)/10.,1);	    	    
	    } else{
	      u8g2.print("wait");	    	    
	    }
	  }
	  
	}else{
	  
	  if (displaydata){
	    LOGE(F("Error on sensor: disable" CR));
	    u8g2.clearBuffer();
	    u8g2.setCursor(0, 10); 
	    u8g2.print("Error Sensor");
	    u8g2.setCursor(0, 20); 
	    u8g2.print("Disable");
	  }
	  
	  return;
	}  
      }
    }

    s_state = IDLE;
    break;
    
  default:
    LOGN(F("Something go wrong in sensor_machine"));
    break;
    
  }
  return;
}

void handle_OnConnect() {

  server.send(200, "text/html", SendHTML(tmean,umean)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}


//https://lastminuteengineers.com/esp8266-dht11-dht22-web-server-tutorial/
// todo: manage missed ajax update 
String SendHTML(float Temperaturestat,float Humiditystat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Senamhi EUAV Weather Report</title>\n";
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
  ptr +=".superscript{font-size: 17px;font-weight: 600;position: absolute;right: -20px;top: 15px;}\n";
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
  
  ptr +="<h1>Senamhi EUAV Weather Report</h1>\n";
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
  ptr +=round(Temperaturestat*10.)/10.;
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
  ptr +=round(Humiditystat);
  ptr +="<span class=\"superscript\">%</span></div>\n";
  ptr +="</div>\n";
  
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

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

void do_etc(){
  hbridge.setpwm(int(vent*255.0/100.0),IBT_2_L_HALF);

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

 // start up the i2c interface
  Wire.begin(SDA,SCL);

  delay(1000);
  u8g2.setI2CAddress(OLEDI2CADDRESS*2);
  u8g2.begin();
  u8g2.setFont(fontNameS);
  u8g2.setFontMode(0); // enable transparent mode, which is faster
  u8g2.clearBuffer();
  u8g2.setCursor(0, 10); 
  u8g2.print(F("Starting up!"));
  u8g2.sendBuffer();

  delay(1000);
  
  //read configuration from FS json
  LOGN(F("mounting FS..." CR));
  if (!SPIFFS.begin()) {
    LOGE(F("failed to mount FS" CR));
    LOGN(F("Reformat SPIFFS" CR));
    SPIFFS.format();
    if (!SPIFFS.begin()) {
      LOGN(F("failed to mount FS" CR));
    }
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
      if (json.containsKey("vent"))     vent=json["vent"];
      if (json.containsKey("ventctrl")) ventCtrl=json["ventctrl"];      

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
      
      LOGN(F("vent     %D" CR),vent);
      LOGN(F("ventctrl %D" CR),ventCtrl);
    }
  }
  
  hbridge.start(IBT_2_R_HALF);

  if (ventCtrl) {
    LOGN(F("vent ON" CR));
    hbridge.start(IBT_2_L_HALF);
  }else{
    LOGN(F("vent OFF" CR));
    hbridge.stop(IBT_2_L_HALF);
  }
  
  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"HIH");
  sensors[0].address=39;

  strcpy(sensors[1].driver,"I2C");
  strcpy(sensors[1].type,"ADT");
  sensors[1].address=73;
  
  encoder.begin();
  encButton.begin();
  

  // encoder with interrupt on the A & B pins
  attachInterrupt(digitalPinToInterrupt(encA), encoderprocess, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB), encoderprocess, CHANGE);
  
  nav.idleTask=idle;//point a function to be used when menu is suspended
  nav.timeOut=10;
  nav.exit();
  
  analogWriteRange(255);
  //analogWriteFreq(100);

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

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("Senamhi-EUAV");

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  
  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  LOGN(F("HTTP server started" CR));

  sensor_machine();
  
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
  server.handleClient();
  dnsServer.processNextRequest();
  do_display();
  do_etc();
}
