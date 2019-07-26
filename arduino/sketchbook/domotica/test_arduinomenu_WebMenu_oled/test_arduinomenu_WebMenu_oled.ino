/********************
Arduino generic menu system
XmlServer menu example
based on WebServer:
  https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer
  https://github.com/Links2004/arduinoWebSockets

Dec. 2016 Rui Azevedo - ruihfazevedo(@rrob@)gmail.com

menu on web browser served by esp8266 device
output: ESP8266WebServer -> Web browser
input: ESP8266WebSocket <- Web browser
format: xml, json

IMPORTANT!:
this requires the data folder to be stored on esp8266 spiff
Extra libraries should be present

arduinoWebSockets - https://github.com/Links2004/arduinoWebSockets
ESP8266WiFi - https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
ESP8266WebServer - https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer

for development purposes some files are left external,
therefor requiring an external webserver to provide them (just for dev purposes)
i'm using nodejs http-server (https://www.npmjs.com/package/http-server)
to static serve content from the data folder. This allows me to quick change
the files without having to upload them to SPIFFS
also gateway ssid and password are stored on this code (bellow),
so don't forget to change it.

*/

#include <ESP8266WiFi.h>
#include <DNSServer.h>

#include <menu.h>
#include <menuIO/esp8266Out.h>
#include <Wire.h>
#include <menuIO/u8g2Out.h>
#include <menuIO/RotaryIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/xmlFmt.h>//to write a menu has html page
#include <menuIO/serialIn.h>
#include <menuIO/serialOut.h>
#include <menuIO/xmlFmt.h>//to write a menu has xml page
#include <menuIO/jsonFmt.h>//to write a menu has xml page
#ifndef ARDUINO_STREAMING
  #include <streamFlow.h>//https://github.com/neu-rah/streamFlow
#else
  #include <Streaming.h>//https://github.com/scottdky/Streaming
#endif
#include <menuIO/jsFmt.h>//to send javascript thru web socket (live update)
#include <FS.h>
#include <Hash.h>
extern "C" {
  #include "user_interface.h"
}

// display definitions
#include <U8g2lib.h>

#define OLEDI2CADDRESS 0X3C
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

// rotary encoder pins
#define encBtn  D5
#define encA    D6
#define encB    D7


#define MAX_DEPTH 2

IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
const byte DNS_PORT = 53;

// global variables for display

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

idx_t gfx_tops[MAX_DEPTH];

PANELS(gfxPanels,{0,0,U8_Width/fontX,U8_Height/fontY});
u8g2Out oledOut(u8g2,colors,gfx_tops,gfxPanels,fontX,fontY,offsetX,offsetY,fontMarginX,fontMarginY);

encoderIn<encA,encB> encoder;//simple quad encoder driver
encoderInStream<encA,encB> encStream(encoder);// simple encoder Stream

//a keyboard with only one key as the encoder button
keyMap encBtn_map[]={{-encBtn,defaultNavCodes[enterCmd].ch}};//negative pin numbers use internal pull-up, this is on when low
keyIn<1> encButton(encBtn_map);//1 is the number of keys

// ISR for encoder management
void ICACHE_RAM_ATTR encoderprocess (){
  encoder.process();
}

using namespace Menu;

#ifdef WEB_DEBUG
  // on debug mode I put aux files on external server to allow changes without SPIFF update
  // on this mode the browser MUST be instructed to accept cross domain files
  String xslt("http://neurux:8080/");
#else
  String xslt("");
#endif

menuOut& operator<<(menuOut& o,unsigned long int i) {
  o.print(i);
  return o;
}
menuOut& operator<<(menuOut& o,endlObj) {
  o.println();
  return o;
}

//this version numbers MUST be the same as data/1.2
#define CUR_VERSION "1.5"
#define APName "WebMenu"

int ledCtrl=LOW;
//on my esp12e led pin is 2
#define LEDPIN 2
//this is ok on other boards
// #define LEDPIN LED_BUILTIN
void updLed() {
  Serial.println("update led state!");
  digitalWrite(LEDPIN,!ledCtrl);
}

//#define ANALOG_PIN 4
#define HTTP_PORT 80
#define WS_PORT 81

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

idx_t web_tops[MAX_DEPTH];
PANELS(webPanels,{0,0,80,100});
xmlFmt<esp8266_WebServerStreamOut> serverOut(server,web_tops,webPanels);
jsonFmt<esp8266_WebServerStreamOut> jsonOut(server,web_tops,webPanels);
jsonFmt<esp8266BufferedOut> wsOut(web_tops,webPanels);

//menu action functions
result action1(eventMask event, navNode& nav, prompt &item) {
  Serial.println(" ");
  Serial.println("action A called!");
  serverOut<<"This is action <b>A</b> web report "<<(millis()%1000)<<"<br/>";
  return proceed;
}
result action2(eventMask event, navNode& nav, prompt &item) {
  Serial.println(" ");
  Serial.println("action B called!");
  serverOut<<"This is action <b>B</b> web report "<<(millis()%1000)<<"<br/>";
  return proceed;
}

void debugLedUpd() {
  Serial.println(" ");
  Serial.println("led update!");
}

TOGGLE(ledCtrl,setLed,"Led: ",updLed,(Menu::eventMask)(updateEvent|enterEvent),noStyle
  ,VALUE("Off",LOW,debugLedUpd,noEvent)
  ,VALUE("On",HIGH,debugLedUpd,noEvent)
);

int selTest=0;
SELECT(selTest,selMenu,"Select",doNothing,noEvent,noStyle
  ,VALUE("Zero",0,doNothing,noEvent)
  ,VALUE("One",1,doNothing,noEvent)
  ,VALUE("Two",2,doNothing,noEvent)
);

int chooseTest=-1;
CHOOSE(chooseTest,chooseMenu,"Choose",doNothing,noEvent,noStyle
  ,VALUE("First",1,doNothing,noEvent)
  ,VALUE("Second",2,doNothing,noEvent)
  ,VALUE("Third",3,doNothing,noEvent)
  ,VALUE("Last",-1,doNothing,noEvent)
);

int duty=50;//%
void updAnalog() {
  //analogWrite(ANALOG_PIN,map(duty,0,100,0,255/*PWMRANGE*/));
  Serial.println(" ");
  Serial.print("update duty: ");
  Serial.println(duty);
}

char* constMEM alphaNum MEMMODE=" 0123456789.ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,\\|!\"#$%&/()=?~*^+-{}[]€";
char* constMEM alphaNumMask[] MEMMODE={alphaNum};
char name[]="                                                  ";

uint16_t year=2017;
uint16_t month=10;
uint16_t day=7;

//define a pad style menu (single line menu)
//here with a set of fields to enter a date in YYYY/MM/DD format
//altMENU(menu,birthDate,"Birth",doNothing,noEvent,noStyle,(systemStyles)(_asPad|Menu::_menuData|Menu::_canNav|_parentDraw)
PADMENU(birthDate,"Birth",doNothing,noEvent,noStyle
  ,FIELD(year,"","/",1900,3000,20,1,doNothing,noEvent,noStyle)
  ,FIELD(month,"","/",1,12,1,0,doNothing,noEvent,wrapStyle)
  ,FIELD(day,"","",1,31,1,0,doNothing,noEvent,wrapStyle)
);

//customizing a prompt look!
//by extending the prompt class
class altPrompt:public prompt {
public:
  // altPrompt(constMEM promptShadow& p):prompt(p) {}
  using prompt::prompt;
  Used printTo(navRoot &root,bool sel,menuOut& out, idx_t idx,idx_t len,idx_t) override {
    return out.printRaw(F("special prompt!"),len);
  }
};

/*
Exception 3: LoadStoreError: Processor internal physical address or data error during load or store
PC: 0x40218725: Menu::menuOut::printRaw(char const*, short) at /home/pat1/git/rmap/arduino/sketchbook/libraries/ArduinoMenu/src/menuIo.cpp line 49
EXCVADDR: 0x40253710

Decoding stack results
0x40201030: altPrompt::printTo(Menu::navRoot&, bool, Menu::menuOut&, short, short, short) at /home/pat1/git/rmap/arduino/sketchbook/domotica/test_arduinomenu_WebMenu/test_arduinomenu_WebMenu.ino line 174
0x40206f8b: Menu::menuOut::printMenu(Menu::navNode&, short) at /home/pat1/git/rmap/arduino/sketchbook/libraries/ArduinoMenu/src/menuIo.cpp line 424
0x40215fd8: _umm_free(void*) at /home/pat1/.arduino15/packages/esp8266/hardware/esp8266/2.5.2/cores/esp8266/umm_malloc/umm_malloc.cpp line 1304
0x402071f0: Menu::menuOut::printMenu(Menu::navNode&) at /home/pat1/git/rmap/arduino/sketchbook/libraries/ArduinoMenu/src/menuIo.cpp line 272
0x40215fd8: _umm_free(void*) at /home/pat1/.arduino15/packages/esp8266/hardware/esp8266/2.5.2/cores/esp8266/umm_malloc/umm_malloc.cpp line 1304
0x402166fc: free(void*) at /home/pat1/.arduino15/packages/esp8266/hardware/esp8266/2.5.2/cores/esp8266/umm_malloc/umm_malloc.cpp line 1764
0x402072b9: Menu::outputsList::printMenu(Menu::navNode&) const at /home/pat1/git/rmap/arduino/sketchbook/libraries/ArduinoMenu/src/menuIo.cpp line 145
0x4020b3a8: FunctionRequestHandler::handle(ESP8266WebServer&, HTTPMethod, String) at /home/pat1/.arduino15/packages/esp8266/hardware/esp8266/2.5.2/libraries/ESP8266WebServer/src/detail/RequestHandlersImpl.h line 37
0x4020741c: Menu::navRoot::printMenu() const at /home/pat1/git/rmap/arduino/sketchbook/libraries/ArduinoMenu/src/nav.cpp line 137
0x40207450: Menu::navRoot::doOutput() at /home/pat1/git/rmap/arduino/sketchbook/libraries/ArduinoMenu/src/nav.cpp line 158
0x40202e84: std::_Function_handler ::_M_invoke(const std::_Any_data &) at /home/pat1/git/rmap/arduino/sketchbook/domotica/test_arduinomenu_WebMenu/test_arduinomenu_WebMenu.ino line 443
0x40209eb4: String::operator!=(String const&) const at /home/pat1/.arduino15/packages/esp8266/hardware/esp8266/2.5.2/cores/esp8266/WString.h line 188
0x401000c9: std::function ::operator()() const at /home/pat1/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/2.5.0-3-20ed2b9/xtensa-lx106-elf/include/c++/4.8.2/functional line 2465
0x4020b3e4: FunctionRequestHandler::handle(ESP8266WebServer&, HTTPMethod, String) at /home/pat1/.arduino15/packages/esp8266/hardware/esp8266/2.5.2/libraries/ESP8266WebServer/src/detail/RequestHandlersImpl.h line 43
0x4020eabc: String::String(String const&) at /home/pat1/.arduino15/packages/esp8266/hardware/esp8266/2.5.2/cores/esp8266/WString.cpp line 41
0x4020b4a2: ESP8266WebServer::_handleRequest() at /home/pat1/.arduino15/packages/esp8266/hardware/esp8266/2.5.2/libraries/ESP8266WebServer/src/ESP8266WebServer.cpp line 599
0x401001f6: millis() at /home/pat1/.arduino15/packages/esp8266/hardware/esp8266/2.5.2/cores/esp8266/core_esp8266_wiring.cpp line 186
0x4020b895: ESP8266WebServer::handleClient() at /home/pat1/.arduino15/packages/esp8266/hardware/esp8266/2.5.2/libraries/ESP8266WebServer/src/ESP8266WebServer.cpp line 308
0x40209d2d: WebSocketsServer::loop() at /home/pat1/git/rmap/arduino/sketchbook/libraries/WebSockets/src/WebSocketsServer.cpp line 134
0x402032cd: loop() at /home/pat1/git/rmap/arduino/sketchbook/domotica/test_arduinomenu_WebMenu/test_arduinomenu_WebMenu.ino line 472
0x4020faa0: loop_wrapper() at /home/pat1/.arduino15/packages/esp8266/hardware/esp8266/2.5.2/cores/esp8266/core_esp8266_main.cpp line 125
*/

MENU(subMenu,"Sub-Menu",doNothing,noEvent,noStyle
  ,OP("Sub1",doNothing,noEvent)
  ,OP("Sub2",doNothing,noEvent)
  ,OP("Sub3",doNothing,noEvent)
     //,altOP(altPrompt,"",doNothing,noEvent)
  ,EXIT("<Back")
);

//the menu
MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
  ,SUBMENU(setLed)
  ,OP("Action A",action1,enterEvent)
  ,OP("Action B",action2,enterEvent)
  ,FIELD(duty,"Duty","%",0,100,10,1, updAnalog, anyEvent, noStyle)
  ,EDIT("Name",name,alphaNumMask,doNothing,noEvent,noStyle)
  ,SUBMENU(birthDate)
  ,SUBMENU(selMenu)
  ,SUBMENU(chooseMenu)
  ,SUBMENU(subMenu)
  ,EXIT("Exit!")
);

result idle(menuOut& o,idleEvent e) {
  //if (e==idling)
  Serial.println("suspended");
  o<<"suspended..."<<endl<<"press [select]"<<endl<<"to continue"<<endl<<(millis()%1000);
  return quit;
}

//template<typename T>//some utill to help us calculate array sizes (known at compile time)
//constexpr inline size_t len(T& o) {return sizeof(o)/sizeof(decltype(o[0]));}

//other menu navigation

//MENU_OUTLIST(out,&serverOut)
idx_t serialTops[MAX_DEPTH]={0};
serialOut serialout(*(Print*)&Serial,serialTops);
MENU_OUTLIST(out,&oledOut,&serialout,&serverOut);

serialIn serialin(Serial);
MENU_INPUTS(in,&serialin,&encStream,&encButton);


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

//config myOptions('*','-',defaultNavCodes,false);

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

	u8g2.firstPage();
	do nav.doOutput(); while(u8g2.nextPage());
	
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
          nav.doInput(inp);
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
  serverOut<<xslt;
  serverOut<<CUR_VERSION"/device.xslt";
  serverOut<<"\"?>\r\n<menuLib"
    #ifdef WEB_DEBUG
      <<" debug=\"yes\""
    #endif
    <<" host=\"";
    serverOut.print(APName);
    serverOut<<"\">\r\n<sourceURL ver=\"" CUR_VERSION "/\">";
  if (server.hasHeader("host"))
    serverOut.print(server.header("host"));
  else
    serverOut.print(APName);
  serverOut<<"</sourceURL>";
}

void pageEnd() {
  serverOut<<"</menuLib>";
  server.client().stop();
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
  server.client().stop();
}

bool handleMenu(navRoot& nav){
  _trace(
    uint32_t free = system_get_free_heap_size();
    Serial.print(F("free memory:"));
    Serial.print(free);
    Serial.print(F(" handleMenu "));
    Serial.println(server.arg("at").c_str());
  );
  String at=server.arg("at");
  bool r;
  r=nav.async(server.hasArg("at")?at.c_str():"/");
  return r;
}

//redirect to version folder,
//this allows agressive caching with no need to cache reset on version change
auto mainPage= []() {
  _trace(Serial<<"serving main page from root!"<<endl);
  server.sendHeader("Location", CUR_VERSION "/index.html", true);
  server.send ( 302, "text/plain", "");
  if (server.hasArg("at"))
    nav.async(server.arg("at").c_str());
};

void setup(){
  //check your pins before activating this
  pinMode(LEDPIN,OUTPUT);
  updLed();
  // analogWriteRange(1023);
  //pinMode(ANALOG_PIN,OUTPUT);
  //updAnalog();
  //options=&myOptions;//menu options
  Serial.begin(115200);
  while(!Serial)
  Serial.println("");
  Serial.println("Arduino menu webserver example");
  //Serial.setDebugOutput(true);

  // start up the i2c interface
  Wire.begin(SDA,SCL);

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

  // start up encoder
  encoder.begin();

  // start up button
  encButton.begin();
  
  // encoder with interrupt on the A & B pins
  attachInterrupt(digitalPinToInterrupt(encA), encoderprocess, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB), encoderprocess, CHANGE);
  
  // #ifndef MENU_DEBUG
    //do not allow web heads to exit, they wont be able to return (for now)
    //we should resume this heads on async requests!
    webNav.canExit=false;
    jsonNav.canExit=false;
    wsNav.canExit=false;
  // #endif

    //  for(uint8_t t = 4; t > 0; t--) {
    //      Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    //      Serial.flush();
    //      delay(1000);
    //  }

  // Serial.setDebugOutput(1);
  // Serial.setDebugOutput(0);
  // while(!Serial);
  // delay(10);
  // wifi_station_set_hostname((char*)serverName);

  SPIFFS.begin();
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("Senamhi-EUAID");

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  
  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);
 
/*
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

*/

  
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  nav.idleTask=idle;//point a function to be used when menu is suspended

  server.on("/",HTTP_GET,mainPage);

  //menu xml server over http
  server.on("/menu", HTTP_GET, []() {
    pageStart();
    serverOut<<"<output state=\""<<((int)&webNav.idleTask)<<"\"><![CDATA[";
    _trace(Serial<<"output count"<<webNav.out.cnt<<endl);
    handleMenu(webNav);//do navigation (read input) and produce output messages or reports
    serverOut<<"]]></output>";
    webNav.doOutput();
    pageEnd();
  });

  //menu json server over http
  server.on("/json", HTTP_GET, []() {
    _trace(Serial<<"json request!"<<endl);
    jsonStart();
    serverOut<<"{\"output\":\"";
    handleMenu(jsonNav);
    serverOut<<"\",\n\"menu\":";
    jsonNav.doOutput();
    serverOut<<"\n}";
    jsonEnd();
  });

  server.begin();
  Serial.println("HTTP server started");
  Serial.println("Serving ArduinoMenu example.");
  #ifdef MENU_DEBUG
    server.serveStatic("/", SPIFFS, "/","max-age=30");
  #else
    server.serveStatic("/", SPIFFS, "/","max-age=31536000");
  #endif
}

void loop(void){
  wsOut.response.remove(0);//clear websocket json buffer
  webSocket.loop();
  server.handleClient();
  dnsServer.processNextRequest();
  nav.doInput();
  if (nav.changed(0)) {//only draw if menu changed for gfx device
    u8g2.firstPage();
    do nav.doOutput();
    while(u8g2.nextPage());
  } 
 
}
