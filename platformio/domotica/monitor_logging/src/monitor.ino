/*
Copyright (C) 2020  Paolo Paruno <p.patruno@iperbole.bologna.it>
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
  Serial Menu
  edit operation:
  1 - first * -> enter field navigation use +/- to select character position
  2 - second * -> enter character edit use +/- to select character value
  3 - third * -> return to field navigation (1)
  4 - fourth * without changing position -> exit edit mode
*/

#include <IWatchdog.h>
//#include <Time.h>
#include <Wire.h>
//#include <TimeAlarms.h>
#include <ArduinoLog.h>
#include <SPI.h>
#include <SD.h>
#include <menu.h>
#include <menuIO/RotaryIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>
#include "config.h"
#if defined (EXTERNALRTC)
#include <DS1307RTC.h>
#else
#include <STM32RTC.h>
#endif

#include <menuIO/u8g2Out.h>
#define fontName u8g2_font_tom_thumb_4x6_tf
#define fontX 5
#define fontY 8
#define offsetX 1
#define offsetY 1
#define U8_Width 64
#define U8_Height 48
#define fontMarginX 1
#define fontMarginY 1

// define menu colors --------------------------------------------------------
//each color is in the format:
//  {{disabled normal,disabled selected},{enabled normal,enabled selected, enabled editing}}
// this is a monochromatic color table
const colorDef<uint8_t> colors[] ={
  {{0,0},{0,1,1}},//bgColor
  {{1,1},{1,0,0}},//fgColor
  {{1,1},{1,0,0}},//valColor
  {{1,1},{1,0,0}},//unitColor
  {{0,1},{0,0,1}},//cursorColor
  {{1,1},{1,0,0}},//titleColor
};

//TwoWire Wire1(PB4, PA7);
//U8G2_SSD1306_64X48_ER_F_2ND_HW_I2C  u8g2(U8G2_R0);

U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0);

int alarmConfig=HIGH;

TOGGLE(alarmConfig,alarmSet,"Alarm: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("On",HIGH,doNothing,noEvent)
  ,VALUE("Off",LOW,doNothing,noEvent)
);

uint16_t myyear=2020;
uint16_t mymonth=12;
uint16_t myday=1;
uint16_t myhrs=0;
uint16_t mymins=0;

MENU(mydate,"Date Set",doNothing,noEvent,noStyle
     ,FIELD(myyear,"Year: ","",2020,2050,20,1,doNothing,noEvent,noStyle)
     ,FIELD(mymonth,"Month: ","",1,12,1,0,doNothing,noEvent,wrapStyle)
     ,FIELD(myday,"Day: ","",1,31,1,0,doNothing,noEvent,wrapStyle)
     ,EXIT("<Back")
     );

MENU(mytime,"Time Set",doNothing,noEvent,noStyle
     ,FIELD(myhrs,"Hour: ","",0,23,1,0,doNothing,noEvent,noStyle)
     ,FIELD(mymins,"Min: ","",0,59,10,1,doNothing,noEvent,wrapStyle)
     ,EXIT("<Back")
     );

result datetimeSave(eventMask e, prompt &item);
result alarmReset(eventMask e, prompt &item);
result alarmTest(eventMask e, prompt &item);


MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
     ,OP("Alarm Reset", alarmReset,enterEvent)
     ,OP("Alarm test",  alarmTest,enterEvent)
     ,SUBMENU(alarmSet)
     ,SUBMENU(mydate)
     ,SUBMENU(mytime)
     ,OP("DateTime Save", datetimeSave,enterEvent)
     ,EXIT("<Exit")
     );

#define MAX_DEPTH 2

encoderIn<encA,encB> encoder;//simple quad encoder driver
encoderInStream<encA,encB> encStream(encoder);// simple encoder Stream

//a keyboard with only one key as the encoder button
keyMap encBtn_map[]={{-encBtn,defaultNavCodes[enterCmd].ch}};//negative pin numbers use internal pull-up, this is on when low
keyIn<1> encButton(encBtn_map);//1 is the number of keys

menuIn* inputsList[]={&encButton};

serialIn serial(Serial);
MENU_INPUTS(in,&encStream,&encButton,&serial);

//define output device serial
idx_t serialTops[MAX_DEPTH]={0};
serialOut outSerial(*(Print*)&Serial,serialTops);

idx_t gfx_tops[MAX_DEPTH];

PANELS(gfxPanels,{0,0,U8_Width/fontX,U8_Height/fontY});
u8g2Out oledOut(u8g2,colors,gfx_tops,gfxPanels,fontX,fontY,offsetX,offsetY,fontMarginX,fontMarginY);

//define outputs controller
menuOut* outputs[]{&outSerial,&oledOut};//list of output devices
outputsList out(outputs,sizeof(outputs)/sizeof(menuOut*));//outputs list controller

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

File dataFile;
  
HardwareSerial Serial1(SERIAL1RX, SERIAL1TX);
//HardwareTimer Tim1 = HardwareTimer(TIM1);
/* Get the rtc object */
#if defined (EXTERNALRTC)
DS1307RTC DSRTC = DS1307RTC();            //instantiate an RTC object
#else
STM32RTC& rtc = STM32RTC::getInstance();
#endif
int alarmStatus=LOW;
int alarmSetted=LOW;
bool sdinitialized=false;

//when menu is suspended
result idle(menuOut& o,idleEvent e) {
  o.clear();
  switch(e) {
    case idleStart:o.println("suspending menu!");break;
    case idling:o.println("Monitoring...");break;
    case idleEnd:o.println("resuming menu.");break;
  }
  return proceed;
}

//// ISR for encoder management
void encoderprocess (){
  encoder.process();
}

result datetimeSave(eventMask e, prompt &item) {

 
#if defined (EXTERNALRTC)
  setTime(myhrs,mymins,0,myday,mymonth,myyear); // set time to Saturday 8:29:00am Jan 1 2011
  
  if (DSRTC.set(now()) != 0){
    Log.error(F("error setting RTC time"));
  }else{
    Log.notice(F("RTC time setted"));
  }
#else
  rtc.setTime(myhrs,mymins,0);
  rtc.setDate(1, myday,mymonth,myyear-2000);
  Log.notice(F("RTC time setted"));
#endif

  dataFile.close();
  SDinitialize();
  
  return proceed;
}

result alarmReset(eventMask e, prompt &item) {
  alarmStatus=LOW;
  alarmSetted=HIGH;
  return proceed;
}

result alarmTest(eventMask e, prompt &item) {
  tone(BUZZERPIN, 988);    // Nota B5
  delay(3000);
  noTone(BUZZERPIN);
  return proceed;
}

void printTimestamp(Print* _logOutput) {

  char c[20];

#if defined (EXTERNALRTC)
  if(!(timeStatus()== timeNotSet)) {
    time_t t = now(); // Store the current time in time variable t 
    sprintf (c, "%04u-%02u-%02uT%02u:%02u:%02u",year(t),month(t),day(t),hour(t),minute(t),second(t));
  }else{
    //The time has never been set")));
    sprintf(c, "%10lu ", millis());
  }
#else
  if (rtc.isTimeSet()) {
    sprintf (c, "%04u-%02u-%02uT%02u:%02u:%02u",rtc.getYear()+2000,rtc.getMonth(),rtc.getDay(),rtc.getHours(), rtc.getMinutes(), rtc.getSeconds());    
  }else{
    sprintf(c, "%10lu ", millis());
  }
#endif
  
  _logOutput->print(c);

}

void printNewline(Print* _logOutput) {
  _logOutput->print('\n');
}


void SDinitialize(void){
  
  Log.notice(F("Initializing SD card..."));
  if (!SD.begin(SDSELECT)) {
    Log.error(F("SD initialization failed"));
  }else{
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    char name[13];
    getFileName(name);
    dataFile = SD.open(name, FILE_WRITE);

    if (!dataFile) {
      // if the file isn't open, pop up an error:
      Log.error(F("opening %s file"),name);
    }else{
      sdinitialized=true;
    }
  }
}


void getFileName(char* name){
#if defined (EXTERNALRTC)
  if(!(timeStatus()== timeNotSet)) {
    time_t t = now(); // Store the current time in time variable t 
    sprintf (name, "%02u%02u%02u%02u.log",year(t)-2000,month(t),day(t),hour(t));
  }else{
    sprintf (name, "%08lu.log",millis());
  }
#else
  if (rtc.isTimeSet()) {
    sprintf (name, "%02u%02u%02u%02u.log",rtc.getYear(),rtc.getMonth(),rtc.getDay(),rtc.getHours());
  }else{
    sprintf (name, "%08lu.log",millis());
  }
#endif
}


void SD_append_message(const String &message ){


  if (!sdinitialized) return;

  // if the file is available, write to it:
  if (dataFile) {
    char c[20];

#if defined (EXTERNALRTC)
    if(!(timeStatus()== timeNotSet)) {
      time_t t = now(); // Store the current time in time variable t 
      sprintf (c, "%04u-%02u-%02uT%02u:%02u:%02u",year(t),month(t),day(t),hour(t),minute(t),second(t));
    }else{
      sprintf (c, "%10lu",millis());
    }
#else
    if (rtc.isTimeSet()) {
      sprintf (c, "%04u-%02u-%02uT%02u:%02u:%02u",rtc.getYear()+2000,rtc.getMonth(),rtc.getDay(),rtc.getHours(),rtc.getMinutes(),rtc.getSeconds());
    }else{
      sprintf (c, "%10lu",millis());
    }
#endif
 
    Log.notice(F("timestamp: %s"), c);
    if (dataFile.print(c)<=0){
      Log.error(F("timestamp not written on file"));
    }else{
      if (dataFile.println(message) > 0){
	// print to the serial port too
	Log.notice(F("write on file: %s"), message.c_str());
      }else{
	Log.error(F("data not written on file"));
      }
    }
  }
}  


void heartBeat(void){

  static unsigned long startmillis=millis();
  char name[13];
  uint32_t size;

  if ((millis()-startmillis)/1000 < HEARTBEATTIME) return;
  startmillis = millis();

  if (digitalRead(CARDDETECT) == LOW){
    sdinitialized=false;
    if(dataFile) dataFile.close();
    Log.notice(F("NO card present in slot"));
    return ;
  }

  if (!sdinitialized) SDinitialize();

  String message="Still alive";
  SD_append_message(message );
  
  strcpy(name,dataFile.name());
  dataFile.close();
  Log.notice(F("flush data on file"));
  Log.notice(F("current file name: %s"),name);
  
  // check file size
  dataFile = SD.open(name, O_READ);
  if (dataFile){
    size = dataFile.size();
    dataFile.close();
    Log.notice(F("filesize: %d"),size);
  }else{
    size=SIZEFILEMAX;
  }
  if (size >= SIZEFILEMAX) {
    // get new filename
    getFileName(name);
    Log.notice(F("new file name: %s"),name);
  }
  dataFile = SD.open(name, FILE_WRITE);

}

void ping(){

  //Log.notice(F("read pong"));
  static unsigned long int lastpingtime=millis();

  while (Serial1.available() > 0){
    String message = Serial1.readStringUntil('#');
    if (message.equals("pong")){
      Log.notice(F("received pong"));
      if (alarmStatus==HIGH){
	alarmStatus=LOW;
	alarmSetted=HIGH;
      }
      lastpingtime=millis();
    }else{
      //if (message.equals("")){
	////flush incoming data
	//while (Serial1.available() > 0) {
	//  Serial1.read();
	//}
      //}else{
	Log.notice(F("receive: %s"),message.c_str());
	SD_append_message(message);
      //}
    }
  }

  if ((millis() - lastpingtime) > PINGTIMEOUT){
    alarmStatus=HIGH;
    alarmSetted=HIGH;
    lastpingtime=millis();
  }
}

void manageAlarm(void){
  if (alarmSetted == HIGH){
    alarmSetted=LOW;      
    if (alarmConfig == HIGH){
      if (alarmStatus == HIGH) {
	tone(BUZZERPIN,1319);    // Nota E6
	SD_append_message("ALARM!" );
	Log.notice(F("ALARM!"));
      }
    }

    if (alarmStatus == LOW) {
      noTone(BUZZERPIN);
      SD_append_message("END alarm" );
      Log.notice(F("END alarm"));
    }
  }
}

void setup() {

  pinMode(LEDPIN, OUTPUT);
  pinMode(BUZZERPIN, OUTPUT);
  pinMode(CARDDETECT, INPUT_PULLUP);
  
  if (IWatchdog.isReset()) {
    // buzzer
    tone(BUZZERPIN, 988);    // Nota B5
    //tone(BUZZERPIN,1319,850);    // Nota E6
  }


  //Start logging
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.setSuffix(printNewline); // Uncomment to get newline as suffix


  if (IWatchdog.isReset(true)) {
    Log.error(F("reset by watchdog"));
    // LED blinks to indicate reset
    for (uint8_t idx = 0; idx < 5; idx++) {
      digitalWrite(LEDPIN, HIGH);
      delay(1000);
      digitalWrite(LEDPIN, LOW);
      delay(1000);
    }
  }  

  // Init the watchdog timer with 10 seconds timeout
  IWatchdog.begin(10000000);
  
  if (!IWatchdog.isEnabled()) {
    Log.error(F("watchdog not started"));
    // buzzer
    //tone(BUZZERPIN, 988,100);    // Nota B5
    tone(BUZZERPIN,1319);    // Nota E6

    // LED blinks indefinitely
    while (1) {
      digitalWrite(LEDPIN, HIGH);
      delay(500);
      digitalWrite(LEDPIN, LOW);
      delay(500);
    }
  }

  Log.notice(F(CR "************ STARTED ****************"));

  Wire.begin();
  //Wire1.begin();
  Serial1.begin(115200);
  Serial1.setTimeout(300);
      
  // create the alarms 
  //Alarm.timerRepeat(1, ping);            // timer for every 5 seconds    
  //Alarm.timerRepeat(60, heartBeat);      // timer for every 60 seconds    

  //Tim1.setOverflow(1000000, MICROSEC_FORMAT);
  //Tim1.attachInterrupt(heartBeat);
  //Tim1.resume();
   
#if defined (EXTERNALRTC)  
  setSyncInterval(900); // set the number of seconds between re-sync
  if (timeStatus() != timeSet){
    Log.notice(F("try set the system time from RTC"));
    setSyncProvider(DSRTC.get);   // the function to get the time from the RTC
  }
#else
  // Select RTC clock source: LSI_CLOCK, LSE_CLOCK or HSE_CLOCK.
  // By default the LSI is selected as source.
  //rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin(); // initialize RTC 24H format

#endif

  SDinitialize();
  
  u8g2.setI2CAddress(OLEDI2CADDRESS*2);
  u8g2.begin();
  u8g2.setFont(fontName);
  u8g2.setFontMode(0); // enable transparent mode, which is faster
  u8g2.clearBuffer();
  u8g2.setCursor(0, 40); 
  u8g2.print(F("Starting up!"));
  u8g2.sendBuffer();

  encoder.begin();
  encButton.begin();

  delay(1000);

  // encoder with interrupt on the A & B pins
  attachInterrupt(digitalPinToInterrupt(encA), encoderprocess, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB), encoderprocess, CHANGE);

  nav.timeOut=10;
  nav.idleTask=idle;//point a function to be used when menu is suspended
  
  Log.notice(F("setup done."));
  Serial.println("Use keys + - * /");  
  Serial.println("to control the menu navigation");

  Log.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix

}


void loop() {

  IWatchdog.reload();      // Reload the watchdog
  //Alarm.delay(0);
  heartBeat();
  ping();
  manageAlarm();
  
  nav.doInput();
  if (nav.changed(0)) {    //only draw if menu changed for gfx device
    u8g2.firstPage();
    do nav.doOutput(); while(u8g2.nextPage());
  }
}
