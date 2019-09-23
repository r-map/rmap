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
#define SENSOR_TEMPLATE_PREC          1
#define SENSOR_TEMPLATE_HIHADT        2
#define SENSOR_TEMPLATE_SHT           3
#define SENSOR_TEMPLATE_SHT_SPS_SCD   4

#define SENSOR_TEMPLATE SENSOR_TEMPLATE_SHT_SPS_SCD

// sensor definition
#if SENSOR_TEMPLATE == SENSOR_TEMPLATE_PREC
#define SENSORS_LEN 0
#elif SENSOR_TEMPLATE == SENSOR_TEMPLATE_HIHADT
#define SENSORS_LEN 2
#elif SENSOR_TEMPLATE == SENSOR_TEMPLATE_SHT
#define SENSORS_LEN 1
#elif SENSOR_TEMPLATE == SENSOR_TEMPLATE_SHT_SPS_SCD
#define SENSORS_LEN 3
#endif

#define LENVALUES 4

#define APName"stima-WiFi"

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

#define HTTP_PORT 80
#define WS_PORT 81

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
#include <WebSocketsServer.h>
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
#include "EspHtmlTemplateProcessor.h"
#include <menu.h>
#include <menuIO/esp8266Out.h>
#include <menuIO/u8g2Out.h>
#include <menuIO/RotaryIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/xmlFmt.h>//to write a menu has html page
#include <menuIO/jsonFmt.h>//to write a menu has xml page
#include <streamFlow.h>//https://github.com/neu-rah/streamFlow
#include <menuIO/jsFmt.h>//to send javascript thru web socket (live update)
#include <math.h>
#include "Calibration.h"
#include "FloatBuffer.h"
#include "LongIntBuffer.h"
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
#define SAMPLERATEPREC 1      // time intervall fron two ticks in prec
#define NSAMPLE SAMPLEPERIOD/SAMPLERATE
#define NSAMPLEPREC SAMPLEPERIOD/SAMPLERATEPREC

// MENU
#define CUR_VERSION "1.0"  // this version numbers MUST be the same as SPIFFS data/1.0
#define MAX_DEPTH 2

// global variables for sensors measure
FloatBuffer t;
FloatBuffer u;
LongIntBuffer p;
float tmean=NAN;
float umean=NAN;
float rrate=NAN;
time_t sprec=NAN;
float symmetry=NAN;
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
float resolution=0.1;
Button precBtn(PRECPIN,debounce); // define the button
volatile uint8_t prec=0;
#endif

// global variable for network (apn, dns server and web server)
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
const byte DNS_PORT = 53;
ESP8266WebServer webserver(HTTP_PORT);
WebSocketsServer webSocket(WS_PORT);
EspHtmlTemplateProcessor templateProcessor(&webserver);

// WEB menu
idx_t web_tops[MAX_DEPTH];
PANELS(webPanels,{0,0,80,100});
xmlFmt<esp8266_WebServerStreamOut> serverOut(webserver,web_tops,webPanels);
jsonFmt<esp8266_WebServerStreamOut> jsonOut(webserver,web_tops,webPanels);
jsonFmt<esp8266BufferedOut> wsOut(web_tops,webPanels);

// global variables for display
bool displaydata=false;
bool displayredraw=false;
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
ICACHE_RAM_ATTR void button1changed()
{
  precBtn.read();
  if (precBtn.wasReleased()){
    p.autoput(millis());
    prec++;
  }
}

#endif
result actionsave(eventMask event, navNode& nav, prompt &item);
result actionresetprec(eventMask event, navNode& nav, prompt &item);
result changeDebounce();
result changeResolution();



MENU(precMenu,"Precipitation",doNothing,noEvent,noStyle
     ,FIELD(resolution,"res","Kg/m2",0.05,2,0.1,0.01,changeResolution,enterEvent,wrapStyle)
     ,FIELD(debounce,"DB t","ms",10,200,10,1,changeDebounce,enterEvent,wrapStyle)
     ,EXIT("<Back")
     );

MENU(tempMenu,"Temperature",doNothing,noEvent,noStyle
     ,FIELD(trawmeasures[0],"uncal1","C",-50.,50.,1.,.1,doNothing,noEvent,wrapStyle)
     ,FIELD(tmeasures[0],   "  cal1","C",-50.,50.,1.,.1,doNothing,noEvent,wrapStyle)
     ,FIELD(trawmeasures[1],"uncal2","C",-50.,50.,1.,.1,doNothing,noEvent,wrapStyle)
     ,FIELD(tmeasures[1],   "  cal2","C",-50.,50.,1.,.1,doNothing,noEvent,wrapStyle)
     ,FIELD(trawmeasures[2],"uncal3","C",-50.,50.,1.,.1,doNothing,noEvent,wrapStyle)
     ,FIELD(tmeasures[2],   "  cal3","C",-50.,50.,1.,.1,doNothing,noEvent,wrapStyle)
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
     ,OP("Save!",actionsave,enterEvent)
     ,OP("reset prec",actionresetprec,enterEvent)
     ,SUBMENU(precMenu)
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


//xml+http navigation control
noInput none;//web uses its own API
menuOut* web_outputs[]={&serverOut};
outputsList web_out(web_outputs,sizeof(web_outputs)/sizeof(decltype(web_outputs[0])));
navNode web_cursors[MAX_DEPTH];
navRoot webNav(mainMenu, web_cursors, MAX_DEPTH, none, web_out);

//json+http navigation control
menuOut* json_outputs[]={&jsonOut};
outputsList json_out(json_outputs,sizeof(json_outputs)/sizeof(decltype(json_outputs[0])));
navNode json_cursors[MAX_DEPTH];
navRoot jsonNav(mainMenu, json_cursors, MAX_DEPTH, none, json_out);

//websockets navigation control
menuOut* ws_outputs[]={&wsOut};
		    outputsList ws_out(ws_outputs,sizeof(ws_outputs)/sizeof(decltype(ws_outputs[0])));
navNode ws_cursors[MAX_DEPTH];
navRoot wsNav(mainMenu, ws_cursors, MAX_DEPTH, none, ws_out);

void circularbuffer_clear(){
  t.clear();
  u.clear();
  p.clear();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      //Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        //Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        webSocket.sendTXT(num, "console.log('ArduinoMenu Connected')");
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[%u] get Text: %s\n", num, payload);
        // nav.async((const char*)payload);//this is slow!!!!!!!!
        __trace(Serial.printf("[%u] get Text: %s\n", num, payload));
        char*s=(char*)payload;
        _trace(Serial<<"serve websocket menu"<<endl);
        wsOut.response.remove(0);
        wsOut<<"{\"output\":\"";
        wsNav.async((const char*)payload);
        wsOut<<"\",\n\"menu\":";
        wsNav.doOutput();
        wsOut<<"\n}";
        webSocket.sendTXT(num,wsOut.response);
        // wsOut.response.remove(0);
        // jsonEnd();

	displayredraw=true;
	
    } break;
    case WStype_BIN: {
        Serial<<"[WSc] get binary length:"<<length<<"[";
        for(int c=0;c<length;c++) {
          Serial.print(*(char*)(payload+c),HEX);
          Serial.write(',');
        }
        Serial<<"]"<<endl;
        uint16_t id=*(uint16_t*) payload++;
        idx_t len=*((idx_t*)++payload);
        idx_t* pathBin=(idx_t*)++payload;
        const char* inp=(const char*)(payload+len);
        //Serial<<"id:"<<id<<endl;
        if (id==nav.active().hash()) {
          //Serial<<"id ok."<<endl;Serial.flush();
          //Serial<<"input:"<<inp<<endl;
          //StringStream inStr(inp);
          //while(inStr.available())

	  // refresh other menu
	  nav.doInput(inp);

	  // reset circularbuffer on menu change
	  circularbuffer_clear();

          webSocket.sendTXT(num, "binBusy=false;");//send javascript to unlock the state
        } //else Serial<<"id not ok!"<<endl;
        //Serial<<endl;
      }
      break;
    default:break;
  }
}

void pageStart() {
  _trace(Serial<<"pasgeStart!"<<endl);
  serverOut<<"HTTP/1.1 200 OK\r\n"
    <<"Content-Type: text/xml\r\n"
    <<"Connection: close\r\n"
    <<"Expires: 0\r\n"
    <<"\r\n";
  serverOut<<"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\r\n"
    "<?xml-stylesheet type=\"text/xsl\" href=\"";
  serverOut<<CUR_VERSION"/device.xslt";
  serverOut<<"\"?>\r\n<menuLib"
    #ifdef WEB_DEBUG
      <<" debug=\"yes\""
    #endif
    <<" host=\"";
    serverOut.print(APName);
    serverOut<<"\">\r\n<sourceURL ver=\"" CUR_VERSION "/\">";
  if (webserver.hasHeader("host"))
    serverOut.print(webserver.header("host"));
  else
    serverOut.print(APName);
  serverOut<<"</sourceURL>";
}

void pageEnd() {
  serverOut<<"</menuLib>";
  webserver.client().stop();
}

void jsonStart() {
  _trace(Serial<<"jsonStart!"<<endl);
  serverOut<<"HTTP/1.1 200 OK\r\n"
    <<"Content-Type: application/json; charset=utf-8\r\n"
    <<"Connection: close\r\n"
    <<"Expires: 0\r\n"
    <<"\r\n";
}

void jsonEnd() {
  webserver.client().stop();
}

bool handleMenu(navRoot& nav){
  _trace(
    uint32_t free = system_get_free_heap_size();
    Serial.print(F("free memory:"));
    Serial.print(free);
    Serial.print(F(" handleMenu "));
    Serial.println(webserver.arg("at").c_str());
  );
  String at=webserver.arg("at");
  bool r;
  r=nav.async(webserver.hasArg("at")?at.c_str():"/");
  return r;
}

//redirect to version folder,
//this allows agressive caching with no need to cache reset on version change
auto mainPage= []() {
  _trace(Serial<<"serving main page from root!"<<endl);
  webserver.sendHeader("Location", CUR_VERSION "/index.html", true);
  webserver.send ( 302, "text/plain", "");
  if (webserver.hasArg("at"))
    nav.async(webserver.arg("at").c_str());
};


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

//menu action functions
result actionresetprec(eventMask event, navNode& nav, prompt &item) {
  Serial.println(" ");
  Serial.println("action resetprec called!");
  webserver.sendContent("Reset precipitation DONE");

  resetprec();

  return proceed;
}

// reset precipitation count
result resetprec() {
  prec=0;
  p.clear();
}

// change debounce runtime
result changeDebounce() {
  LOGN(F("changedebounce %d" CR),debounce);
  precBtn.setDbTime(debounce);
}

// change resolution
result changeResolution() {
  LOGN(F("changeresolution %.2f" CR),resolution);
}

//menu action functions
result actionsave(eventMask event, navNode& nav, prompt &item) {
  Serial.println(" ");
  Serial.println("action save called!");
  webserver.sendContent("Save configuration DONE");

  save();

  return proceed;
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
  LOGN(F("resolution : %.2f\n"),resolution);

  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  
#ifdef HBRIDGE
  LOGN(F("vent     %D" CR),vent);
  LOGN(F("ventctrl %D" CR),ventCtrl);
  
  json["vent"] = vent;
  json["ventctrl"] = ventCtrl;
#endif

  json["debounce"] = debounce;
  json["resolution"] = resolution;
  
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
    circularbuffer_clear();
    break;
  case idleEnd:
    o.println("resuming menu.");
    displaydata=false;
    break;
  }
  return proceed;
}

// ISR for encoder management
ICACHE_RAM_ATTR void encoderprocess (){
  encoder.process();
}


// set start measure event on sensor machine
void start_measure(){
  LOGN(F("event START_MEASURE" CR));
  if (displaydata) s_event=START_MEASURE;
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
	    
	    u.autoput(U_Input);
	    LOGN(F("U size %d" CR),u.getSize());
	    
	    if (u.getSize() == u.getCapacity()){
	      umean=0;
	      for ( uint8_t i=0 ; i < u.getCapacity() ; i++)  {
		LOGN(F("U ele %d %F" CR),i,u.peek(i));
		umean += (u.peek(i) - umean) / (i+1);
	      }
	    }else{
	      umean=NAN;
	    }
	    
#if SENSOR_TEMPLATE == SENSOR_TEMPLATE__HIHADT
	  }
	  if (i == 1){
#endif
	    float T_Input;
	    tcal.getConcentration(float(values[0])/100.-273.15,&T_Input);
	    LOGN(F("calibrated T: %F" CR),T_Input);
	    
	    t.autoput(T_Input);
	    if (t.getSize() == t.getCapacity()){
	      tmean=0;
	      for ( uint8_t i=0 ; i < t.getCapacity() ; i++)  {
		tmean += (t.peek(i) - tmean) / (i+1);
	      }
	    } else{
	      tmean=NAN;
	    }
	  }

#if SENSOR_TEMPLATE == SENSOR_TEMPLATE_SHT || SENSOR_TEMPLATE == SENSOR_TEMPLATE_SHT_SPS_SCD

	  if (i == 1){

	    pm2=round(values[1]/10.);
	    pm10=round(values[3]/10.);
	  }

	  if (i == 2){
	    co2=round(values[0]/1.8);
	  }
	  
#endif
	}else{
	  
	  if (displaydata){
	    LOGE(F("Error on sensor: disable" CR));
	    u8g2.clearBuffer();
	    u8g2.setCursor(0, 10); 
	    u8g2.print("Error Sensor");
	    u8g2.sendBuffer();
	    delay(3000);
	  }	  

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


void handle_NotFound(){
  webserver.send(404, "text/plain", "Not found");
}


const char* reportKeyProcessor(const char* key)
{
  //LOGN(F("KEY:>%s<" CR),key);
  static char cvalue[21];

  // attention floating value print have round here?


  if (strcmp(key,"TEMP")==0) snprintf(cvalue,20,"%.1f",tmean);
  else if (strcmp(key, "HUMID")==0) snprintf(cvalue,20,"%.0f",umean);
  else if (strcmp(key, "PM2")==0) snprintf(cvalue,20,"%d",pm2);
  else if (strcmp(key, "PM10")==0) snprintf(cvalue,20,"%d",pm10);
  else if (strcmp(key, "CO2")==0) snprintf(cvalue,20,"%d",co2);
  else if (strcmp(key, "PREC")==0) snprintf(cvalue,20,"%.2f",prec*resolution);
  else if (strcmp(key, "RATE")==0) snprintf(cvalue,20,"%.2f",rrate*resolution);
  else if (strcmp(key, "RSYM")==0) snprintf(cvalue,20,"%.1f",symmetry);
  else   strcpy(cvalue,"ERROR: Key not found");
  

  /*
  if (key == "TEMP") snprintf(cvalue,20,"%.1f",tmean);
  else if (key == "HUMID") snprintf(cvalue,20,"%.0f",umean);
  else if (key == "PM2") snprintf(cvalue,20,"%d",pm2);
  else if (key == "PM10") snprintf(cvalue,20,"%d",pm10);
  else if (key == "CO2") snprintf(cvalue,20,"%d",co2);
  else if (key == "PREC") snprintf(cvalue,20,"%.2f",prec*resolution);
  else if (key == "RATE") snprintf(cvalue,20,"%.2f",rrate*resolution);
  else if (key == "RSYM") snprintf(cvalue,20,"%.1f",symmetry);
  else   strcpy(cvalue,"ERROR: Key not found");
  */
  return cvalue;
}

void handleReport()
{
#if SENSOR_TEMPLATE == SENSOR_TEMPLATE_PREC
  templateProcessor.processAndSend("/rreport.html", reportKeyProcessor);
#else
  templateProcessor.processAndSend("/report.html", reportKeyProcessor);
#endif
}

void do_display_sensors(){
    u8g2.setFont(fontNameB);
      
    u8g2.setCursor(0, 12); 
    u8g2.print("U:");
    u8g2.setCursor(30, 12); 
    if (isnan(umean)){
      u8g2.print("wait");
    }else{
      u8g2.print(round(umean),0);
    }
    
    u8g2.setCursor(0, 24); 
    u8g2.print("T:");
    u8g2.setCursor(30, 24); 
    if (isnan(tmean)){
      u8g2.print("wait");
    }else{
      u8g2.print(round(tmean*10.)/10.,1);	    	    
    }

    u8g2.setCursor(0, 36); 
    u8g2.print("PM2:");

    u8g2.setCursor(30, 36); 
    u8g2.print(pm2);	    	    

    /*
      u8g2.setCursor(0, 48); 
      u8g2.print("PM10:");
      u8g2.setCursor(30, 48); 
      u8g2.print(pm10);	    	    
    */

    u8g2.setCursor(0, 48); 
    u8g2.print("CO2:");
    u8g2.setCursor(30, 48); 
    u8g2.print(co2);	    	    
}

#ifdef HBRIDGE
// update the display about ventilation
void do_display_vent(){
  u8g2.setFont(fontNameS);
  u8g2.setCursor(0, 49); 
  u8g2.print("Vent:");
  u8g2.setCursor(25, 49); 
  u8g2.print(vent);
  u8g2.setCursor(55, 49); 
  u8g2.print(ventCtrl);
  u8g2.sendBuffer();
}
#else
void do_display_prec(){
  u8g2.setFont(fontNameB);
  u8g2.setCursor(0, 40); 
  u8g2.print("P:");
  u8g2.setCursor(30, 40); 
  u8g2.print(prec*resolution,2);	    	    
  u8g2.setFont(fontNameS);
  u8g2.sendBuffer();
}

void do_display_prec_rate(){
  u8g2.setFont(fontNameB);
  
  u8g2.setCursor(0, 12); 
  u8g2.print("P:");
  u8g2.setCursor(20, 12); 
  u8g2.print(prec*resolution,2);	    	    

  u8g2.setCursor(0, 24); 
  u8g2.print("R:");
  u8g2.setCursor(20, 24); 
  u8g2.print(round(rrate*resolution*100.)/100.,2);	    	    
  
  u8g2.setCursor(0, 36); 
  u8g2.print("S:");
  u8g2.setCursor(20, 36); 
  u8g2.print(round(symmetry*10.)/10.,2);	    	    
}
#endif


void do_display(){
  if (displaydata){

    u8g2.setFontMode(0); // enable transparent mode, which is faster
    u8g2.clearBuffer();
  
#if SENSOR_TEMPLATE == SENSOR_TEMPLATE_PREC
    do_display_prec_rate();
#else
    do_display_sensors();
#endif
  
#ifdef HBRIDGE
    do_display_vent();
#else
#if SENSOR_TEMPLATE == SENSOR_TEMPLATE_HIHADT || SENSOR_TEMPLATE == SENSOR_TEMPLATE_SHT 
    do_display_prec();
#endif
#endif
    
    u8g2.sendBuffer();
    u8g2.setFont(fontNameS);
  }
}


void compute1(){
#ifndef HBRIDGE

  //uint8_t i=p.getCapacity();

  if (p.getSize() > 1){  
    //LOGN(F("P ele %d %l" CR),0,p.peek(0));
    //0LOGN(F("P ele %d %l" CR),1,p.peek(1));

    noInterrupts();
    rrate=60000/(p.peek(0)-p.peek(1));
    interrupts();
    
  }else{
    rrate=NAN;
  }

  if (p.getSize() > 2){  
    //LOGN(F("S ele %d %l" CR),0,p.peek(0));
    //LOGN(F("S ele %d %l" CR),1,p.peek(1));
    //LOGN(F("S ele %d %l" CR),2,p.peek(2));

    noInterrupts();
    symmetry=round((float(p.peek(0)-p.peek(1))/float(p.peek(1)-p.peek(2)))*100.);
    interrupts();
    
  }else{
    symmetry=NAN;
  }


#endif
}

void compute60(){
#ifndef HBRIDGE

  //symmetry++;
  
#endif
}
 
  
// management of H bridge and update display
void do_etc(){

#ifdef HBRIDGE
  hbridge.setpwm(int(vent*255.0/100.0),IBT_2_L_HALF);
#endif

  nav.doInput();
  if (nav.changed(0)) displayredraw=true;

  if (displayredraw) {//only draw if menu changed for gfx device
    displayredraw=false;
    u8g2.firstPage();
    do
      nav.doOutput();
    while(u8g2.nextPage());
#ifdef USESERIAL
      nav.printMenu(outSerial);
#endif
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

      if (json.containsKey("resolution")) resolution=json["resolution"];      
      LOGN(F("resolution   %.2F" CR),resolution);
      
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

  //do not allow web heads to exit, they wont be able to return (for now)
  //we should resume this heads on async requests!
  webNav.canExit=false;
  jsonNav.canExit=false;
  wsNav.canExit=false;

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
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
  p.init(NSAMPLEPREC);
  
  // setup AP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(APName);

  // setup DNS
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  
  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  // setup web server
  webserver.on("/report.html", handleReport);
  webserver.onNotFound(handle_NotFound);
  webserver.on("/",HTTP_GET,mainPage);

  //menu xml server over http
  webserver.on("/menu", HTTP_GET, []() {
    pageStart();
    serverOut<<"<output state=\""<<((int)&webNav.idleTask)<<"\"><![CDATA[";
    _trace(Serial<<"output count"<<webNav.out.cnt<<endl);
    handleMenu(webNav);//do navigation (read input) and produce output messages or reports
    serverOut<<"]]></output>";
    webNav.doOutput();
    pageEnd();
  });

  //menu json server over http
  webserver.on("/json", HTTP_GET, []() {
    _trace(Serial<<"json request!"<<endl);
    jsonStart();
    serverOut<<"{\"output\":\"";
    handleMenu(jsonNav);
    serverOut<<"\",\n\"menu\":";
    jsonNav.doOutput();
    serverOut<<"\n}";
    jsonEnd();
  });

  webserver.serveStatic("/", SPIFFS, "/","max-age=31536000");
  
  webserver.begin();
  LOGN(F("HTTP server started" CR));

  // start sensor machine
  sensor_machine();

  // setup alarms
  Alarm.timerRepeat(SAMPLERATE, start_measure);            // timer for measures    
  Alarm.timerRepeat(1, do_display);                        // timer for display every second    
  Alarm.timerRepeat(1, compute1);                          // timer for compute every second    
  Alarm.timerRepeat(60, compute60);                        // timer for compute every 60 second    
  
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
  wsOut.response.remove(0);     //clear websocket json buffer
  webSocket.loop();
  webserver.handleClient();
  dnsServer.processNextRequest();
  do_etc();
}
