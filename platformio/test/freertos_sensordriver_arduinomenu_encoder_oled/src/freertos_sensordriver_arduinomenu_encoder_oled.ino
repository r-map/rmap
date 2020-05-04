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
  This test use:
  * MCU board
  * encoder with button (see at homotix)
  * wemos oled display V2.1.0
  * temperature sensor ADT I2C connected
  * temeprature and humidity sensor HIH6100 I2C connected

  Tested on nucleo_l432kc
  should work on:
  * arduino mega  (or same MCU based board)
  * microduino core 1284p   (or same MCU based board)
  * nucleo_l476rg

  The software use FreeRtos with the c++ wrapper all ported to arduino
  for AVR and STM32

  There are tree thread: 
  * manage display to show data received on message queue and a
    salmple menu and only one function actually do an action switching
    a led light
  * one thread for sensor to quey data and send data on message queue
    each sensor can return multiple data

  Is possible to use one or two I2C busses.  If you use two busses the
  first is shared by sensor threads and the second is dedicated to
  display.
  there are two mutex:
  * for the serial port used by logging system
  * for the first I2C bus

  Only one queue is used to send data to be displayed
 */

// rotary encoder pins
#define encBtn  6
#define encA    2
#define encB    3
#define LEDPIN 13

#define OLEDI2CADDRESS 0X3C  // 60

#define SENSORS_LEN 2
#define LENVALUES 3

#include <Arduino.h>
#ifdef ARDUINO_ARCH_AVR
#include <ArduinoSTL.h>
#include <Arduino_FreeRTOS.h>
#else 
#ifdef ARDUINO_ARCH_STM32
#include "STM32FreeRTOS.h"
#else
#include "FreeRTOS.h"
#endif
#endif
#include "task.h"
#include "thread.hpp"
#include "ticks.hpp"
#include "queue.hpp"
#include <frtosLog.h>
#include <Wire.h>
#include <frtosSensorDriverb.h>

#include <U8g2lib.h>
#include <menu.h>
#include <menuIO/u8g2Out.h>
#include <menuIO/RotaryIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>

#define fontNameS u8g2_font_tom_thumb_4x6_tf
#define fontNameB u8g2_font_t0_11_tf
#define fontName u8g2_font_tom_thumb_4x6_tf
#define fontX 5
#define fontY 8
#define offsetX 1
#define offsetY 1
#define U8_Width 64
#define U8_Height 48
#define fontMarginX 1
#define fontMarginY 1

// use two I2C bus where available
#if defined(ARDUINO_ARCH_STM32)
#define WIREX Wire1
TwoWire WIREX(PB4, PA7);
U8G2_SSD1306_64X48_ER_F_2ND_HW_I2C  u8g2(U8G2_R0);
#else
#define WIREX Wire
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0);
#endif

// exchange message for sensors data
struct message_t
{
  char type[5];         // driver name
  uint8_t ind;
  unsigned long int  value;
};

// type to define sensors
struct sensor_t
{
  char driver[5];         // driver name
  char type[5];         // driver name
  uint8_t address;            // i2c address
};

// prepend to logging
void printTimestamp(Print* _logOutput) {
  char c[12];
  sprintf(c, "%10lu ", millis());
  _logOutput->print(c);
}

// postpone to logging
void printNewline(Print* _logOutput) {
  _logOutput->print('\n');
}

// ---------------------   used by menu thread -------------- //
/* 
   those definition should be inside task class definition
   so it will be allocated in stack and deallocated when task end
   but there are some problems:
   macro used to declare menu have problems with class members
   ISR are not istance specific: https://forum.arduino.cc/index.php?topic=311968.0
 */

bool displaydata=false;
int test=55;
uint16_t hrs=0;
uint16_t mins=0;
int ledCtrl=HIGH;


result myLedOn() {
  ledCtrl=HIGH;
  frtosLog.notice(F("Set LED: %d"),ledCtrl);
  digitalWrite(LEDPIN,ledCtrl);
  return proceed;
}
result myLedOff() {
  ledCtrl=LOW;
  frtosLog.notice(F("Set LED: %d"),ledCtrl);
  digitalWrite(LEDPIN,ledCtrl);
  return proceed;
}

// define menu colors
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

result doAlert(eventMask e, prompt &item);

TOGGLE(ledCtrl,setLed,"Led: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
       ,VALUE("On",HIGH,myLedOn,noEvent)
       ,VALUE("Off",LOW,myLedOff,noEvent)
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

// //customizing a prompt look!
// //by extending the prompt class
// class altPrompt:public prompt {
// public:
//   altPrompt(constMEM promptShadow& p):prompt(p) {}
//   Used printTo(navRoot &root,bool sel,menuOut& out, idx_t idx,idx_t len,idx_t panelNr) override {
//     return out.printRaw(F("special prompt!"),len);;
//   }
// };

MENU(subMenu,"Sub-Menu",doNothing,noEvent,noStyle
     ,OP("Sub1",doNothing,noEvent)
     // ,altOP(altPrompt,"",doNothing,noEvent)
     ,EXIT("<Back")
     );

//define a pad style menu (single line menu)
//here with a set of fields to enter a date in YYYY/MM/DD format
altMENU(menu,tempo,"Time",doNothing,noEvent,noStyle,(systemStyles)(_asPad|Menu::_menuData|Menu::_canNav|_parentDraw)
	,FIELD(hrs,"",":",0,11,1,0,doNothing,noEvent,noStyle)
	,FIELD(mins,"","",0,59,10,1,doNothing,noEvent,wrapStyle)
	);

char* constMEM hexDigit MEMMODE="0123456789ABCDEF";
char* constMEM hexNr[] MEMMODE={"0","x",hexDigit,hexDigit};
char buf1[]="0x11";

MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
     ,OP("Op1",doNothing,noEvent)
     ,OP("Op2",doNothing,noEvent)
     //,FIELD(test,"Test","%",0,100,10,1,doNothing,noEvent,wrapStyle)
     ,SUBMENU(tempo)
     ,SUBMENU(subMenu)
     ,SUBMENU(setLed)
     ,OP("LED On",myLedOn,enterEvent)
     ,OP("LED Off",myLedOff,enterEvent)
     ,SUBMENU(selMenu)
     ,SUBMENU(chooseMenu)
     ,OP("Alert test",doAlert,enterEvent)
     ,EDIT("Hex",buf1,hexNr,doNothing,noEvent,noStyle)
     ,EXIT("<Exit")
     );

#define MAX_DEPTH 2

encoderIn<encA,encB> encoder;//simple quad encoder driver
encoderInStream<encA,encB> encStream(encoder);// simple encoder Stream

//a keyboard with only one key as the encoder button
keyMap encBtn_map[]={{-encBtn,defaultNavCodes[enterCmd].ch}};//negative pin numbers use internal pull-up, this is on when low
keyIn<1> encButton(encBtn_map);//1 is the number of keys

menuIn* inputsList[]={&encButton};

MENU_INPUTS(in,&encStream,&encButton);

idx_t gfx_tops[MAX_DEPTH];

PANELS(gfxPanels,{0,0,U8_Width/fontX,U8_Height/fontY});
u8g2Out oledOut(u8g2,colors,gfx_tops,gfxPanels,fontX,fontY,offsetX,offsetY,fontMarginX,fontMarginY);

//define outputs controller
menuOut* outputs[]{&oledOut};//list of output devices
outputsList out(outputs,sizeof(outputs)/sizeof(menuOut*));//outputs list controller


NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);


result alert(menuOut& o,idleEvent e) {
  if (e==idling) {
    o.setCursor(0,0);
    o.print("alert test");
    o.setCursor(0,1);
    o.print("press [select]");
    o.setCursor(0,2);
    o.print("to continue...");
  }
  return proceed;
}

result doAlert(eventMask e, prompt &item) {
  nav.idleOn(alert);
  return proceed;
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
    u8g2.clearBuffer();
    break;
  case idleEnd:
    o.println("resuming menu.");
    displaydata=false;
    break;
  }
  return proceed;
}

//// ISR for encoder management
void encoderprocess (){
  encoder.process();
}


// --------------------------------- //

using namespace cpp_freertos;


//                                 Thread to manage menu and display data
class menuThread : public Thread {

public:
  
  menuThread(MutexStandard& mutex,Queue &q)
    : Thread("Thread Menu", 250, 1), 
      MessageQueue(q),
      sdmutex(mutex)
  {
    Start();
  };
  
protected:

  virtual void Run() {

    frtosLog.notice("Starting Thread menu");

    pinMode(LEDPIN, OUTPUT);       // initialize LED status
    digitalWrite(LEDPIN,ledCtrl);
    
    u8g2.setI2CAddress(OLEDI2CADDRESS*2);
    u8g2.begin();
    u8g2.setFont(fontName);
    u8g2.setFontMode(0); // enable transparent mode, which is faster
    u8g2.clearBuffer();
    u8g2.setCursor(0, 10); 
    u8g2.print(F("Starting up!"));
    u8g2.sendBuffer();
    
    Delay(Ticks::SecondsToTicks(3));

    encButton.begin();
    encoder.begin();
    
    // encoder with interrupt on the A & B pins
    attachInterrupt(digitalPinToInterrupt(encA), encoderprocess, CHANGE);
    attachInterrupt(digitalPinToInterrupt(encB), encoderprocess, CHANGE);

    // setup menu
    nav.idleTask=idle;//point a function to be used when menu is suspended
    nav.timeOut=10;
    nav.exit();

    message_t Message;
    
    while (true) {

      // get messages from the queue
      if (MessageQueue.Dequeue(&Message,0)){
	frtosLog.notice(F("ricevo dalla coda : %s %d %d"),Message.type,Message.ind,Message.value);

	if (displaydata){               // we have new messages and hav to display it

#ifndef ARDUINO_ARCH_STM32
	  LockGuard guard(sdmutex);     // use mutex when we have only oene I2C bus
#endif
	  //u8g2.setFontMode(0); // enable transparent mode, which is faster
	  //u8g2.clearBuffer();
	  u8g2.setFont(fontNameB);
	  u8g2.setFontMode(0);
	  
	  if (strcmp(Message.type,"ADT")==0 && (Message.ind == 0)){
	    u8g2.setCursor(0, 12); 
	    u8g2.print("T:");
	    u8g2.setCursor(20, 12); 
	    if (Message.value == 0xFFFFFFFF){
	      u8g2.print("NO data");
	    }else{
	      u8g2.setDrawColor(0);
	      u8g2.drawBox(20, 0, 50, 12);
	      u8g2.setDrawColor(1);
	      u8g2.setCursor(20, 12);
	      u8g2.print(round((float(Message.value)/100.-273.15)*10.)/10.,1);	    	    
	    }
	  }

	  if (strcmp(Message.type,"HIH")==0 && (Message.ind == 0)){
	    u8g2.setCursor(0, 24); 
	    u8g2.print("U:");
	    u8g2.setCursor(20, 24); 
	    if (Message.value == 0xFFFFFFFF){
	      u8g2.print("NO data");
	    }else{
	      u8g2.setDrawColor(0);
	      u8g2.drawBox(20, 12, 50, 24);
	      u8g2.setDrawColor(1);
	      u8g2.setCursor(20, 24); 
	      u8g2.print(round(float(Message.value)),0);
	    }
	  }
	  u8g2.sendBuffer();
	  u8g2.setFont(fontNameS);
	  u8g2.setFontMode(1);
	}
      }
      
      nav.doInput();
      if (nav.changed(0)) {   //only draw if menu changed for gfx device
	u8g2.firstPage();
#ifndef ARDUINO_ARCH_STM32
	sdmutex.Lock();       // use mutex when we have only oene I2C bus
#endif
	do nav.doOutput(); while(u8g2.nextPage());
#ifndef ARDUINO_ARCH_STM32
	sdmutex.Unlock();
#endif
	frtosLog.notice(F("D:Free stack bytes : %d" ),uxTaskGetStackHighWaterMark( NULL ));
      }  
    }
  };

private:
  Queue &MessageQueue;
  MutexStandard &sdmutex;
};



//                                 Thread to get data from one sensor
class sensorThread : public Thread {
  
public:
  
  sensorThread(int i, int delayInSeconds,sensor_t mysensor,MutexStandard& sdmutex,Queue &q)
    : Thread("Thread Sensor", 200, 2), 
      Id (i),                                     // id of the thread
      DelayInSeconds(delayInSeconds),             // sample time for the sensor
      sensor(mysensor),                           // sensor definition RMAP stype
      sd(nullptr),                                // sensor driver (SensorDriver library)
      MessageQueue(q)                             // queue to send data
  {
    sd=frtosSensorDriver::create(sensor.driver,sensor.type,sdmutex);  // create driver
	Start();
  };
  
protected:

  virtual void Run() {
    
    frtosLog.notice("Starting Thread sensor %d %s %s",Id,sd->driver,sd->type);

    if (sd == nullptr){
      frtosLog.error(F("%d:%s %s : driver not created !"),Id,sensor.driver,sd->type);
    }else{
      frtosLog.notice(F("%d:%s %s : driver created !"),Id,sensor.driver,sd->type);
    }

    if (sd->setup(sensor.driver,sensor.address) != SD_SUCCESS){
      frtosLog.error("%d:%s %s setup failed !", Id,sd->driver,sd->type);
    }
	
    while (true) {
      Delay(Ticks::SecondsToTicks(DelayInSeconds));

      if (sd != nullptr){
	unsigned long waittime=0;
	message_t Message;

	if (sd->prepare(waittime) != SD_SUCCESS){
	  frtosLog.error("%d:%s %s prepare failed !", Id,sd->driver,sd->type);

	  for (uint8_t ii = 0; ii < LENVALUES; ii++) {      // send missed data message
	      memcpy(Message.type,sd->type, sizeof(sd->type));
	      Message.ind=ii;
	      Message.value=0xFFFFFFFF;
	      MessageQueue.Enqueue(&Message);
	  }

	}else{

	  //wait sensors to go ready
	  frtosLog.notice("%d:%s %s wait sensors for ms: %d",Id,sensor.driver,sd->type,waittime);
	  TickType_t ticks=Ticks::MsToTicks(waittime);
	  Delay( ticks ? ticks : 1 );            /* Minimum delay = 1 tick */
	  
	  // get integers values 
	  long values[LENVALUES];
	  
	  for (uint8_t ii = 0; ii < LENVALUES; ii++) {
	    values[ii]=0xFFFFFFFF;                   // initialize to missed
	  }
	  
	  if (sd->get(values,LENVALUES) == SD_SUCCESS){
	    for (uint8_t ii = 0; ii < LENVALUES; ii++) {
	      frtosLog.notice("%d:%s %s value: %d",Id,sd->driver,sd->type,values[ii]);
	    }
	  }else{
	    frtosLog.error("%d:%s %s Error",Id,sd->driver,sd->type);
	  }

	  for (uint8_t ii = 0; ii < LENVALUES; ii++) {   // send data
	    memcpy(Message.type,sd->type, sizeof(sd->type));
	    Message.ind=ii;
	    Message.value=values[ii];
	    MessageQueue.Enqueue(&Message);
	    
	  }
	}
	frtosLog.notice(F("S:Free stack bytes : %d" ),uxTaskGetStackHighWaterMark( NULL ));
      }
    }
  };

private:
  int Id;
  int DelayInSeconds;
  frtosSensorDriver* sd;
  sensor_t sensor;
  Queue &MessageQueue;
};

void setup (void)
{

  /*  Warning
      in this function il you want use a variable after scheduler is started
      you have to allocate il in heap becouse averithings allocated in stack
      will be deleted by freertos starting scheduler
      so use static or new to allocate
  */
  
  static MutexStandard loggingmutex;  // shared serial for logging
  static MutexStandard sdmutex;       // shared I2C bus
  sensor_t sensors[SENSORS_LEN];      // not static, we lost it after StartScheduler
  
  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"ADT");
  sensors[0].address=73;

  strcpy(sensors[1].driver,"I2C");
  strcpy(sensors[1].type,"HIH");
  sensors[1].address=39;
  
  // start up the i2c interface
  Wire.begin();
  WIREX.begin();       // it could be the same I2C or second I2C
  
  // start up the serial interface
  Serial.begin(115200);

  //Start logging
  frtosLog.begin(LOG_LEVEL_VERBOSE, &Serial,loggingmutex);
  frtosLog.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
  frtosLog.setSuffix(printNewline); // Uncomment to get newline as suffix
  
  frtosLog.notice(F("Testing FreeRTOS C++ wrappers to SensorDriver"));

  Queue *MessageQueue;
  MessageQueue = new Queue(SENSORS_LEN*LENVALUES, sizeof(message_t));
  
  menuThread* MT;
  MT=new menuThread(sdmutex,*MessageQueue);     // we can allocate it as static too

  sensorThread* ST[SENSORS_LEN];
  // create threads
  for (int i = 0; i < SENSORS_LEN; i++) {
    ST[i]=new sensorThread(i, i+1,sensors[i],sdmutex, *MessageQueue);
  }
  
  Thread::StartScheduler();
  
}

void loop()
{
  // Empty. Things are done in Tasks.
}

