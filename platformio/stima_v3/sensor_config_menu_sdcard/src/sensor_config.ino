/**********************************************************************
Copyright (C) 2023  Paolo Paruno <p.patruno@iperbole.bologna.it>
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

#include "Wire.h"
#include <Arduino.h>
#include <ArduinoLog.h>
#include "registers-radiation.h"           //Register definitions
#include "registers-wind.h"                //Register definitions
#include "registers-th.h"                  //Register definitions
#include "registers-rain.h"                //Register definitions
#include "registers-power.h"               //Register definitions
#include <i2c_utility.h>
#include <stima-config.h>
#include <sensors_config.h>
#include <ethernet_config.h>
#include <typedef.h>
#include <EEPROM.h>

#define LOG_LEVEL LOG_LEVEL_VERBOSE

#define WIND_POWER_ON_DELAY_MS                          (5000)
#define WIND_POWER_PIN                                  (4)
#define GWS_SERIAL_BAUD                                 (9600)
#define GWS_SERIAL_TIMEOUT_MS                           (500)
#define UART_RX_BUFFER_LENGTH                           (120)
uint16_t uart_rx_buffer_length;
char uart_rx_buffer[UART_RX_BUFFER_LENGTH];
#ifndef ARDUINO_ARCH_AVR
//HardwareSerial Serial1(PB11, PB10);
HardwareSerial Serial1(D0, D1);
#endif

const char version[] = "4.0";

#include <menu.h>
#include <menuIO/hd44780_I2CexpOut.h>
#include <menuIO/RotaryIn.h>
#include <menuIO/IRremoteIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>
#include <SPI.h>
#include <SdFat.h>
#include <plugin/SdFatMenu.h>

#define SDCARD_SS 7
SdFat sd;

//function to handle file select
// declared here and implemented bellow because we need
// to give it as event handler for `filePickMenu`
// and we also need to refer to `filePickMenu` inside the function
result filePick(eventMask event, navNode& nav, prompt &item);

// LCD /////////////////////////////////////////
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c LCD i/o class header

// Note, i2c address can be specified or automatically located
// If you wish to use a specific address comment out this constructor
// and use the constructor below that specifies the address

// declare the lcd object for auto i2c address location
hd44780_I2Cexp lcd;

// Encoder /////////////////////////////////////
// rotary encoder pins
#define encA 2
#define encB 3
//this encoder has a button here
#define encBtn 6

#define IR_PIN 0

void scanI2CBus(byte from_addr, byte to_addr,void(*callback)(byte address, byte result) );
result scan_i2c_bus();

result i2c_master_save_all(eventMask e, prompt &item);

result i2c_solar_radiation_address(eventMask e, prompt &item);
result i2c_solar_radiation_oneshot(eventMask e, prompt &item);
result i2c_solar_radiation_offset(eventMask e, prompt &item);
result i2c_solar_radiation_gain(eventMask e, prompt &item);
result i2c_solar_radiation_sensor_voltage(eventMask e, prompt &item);
result i2c_solar_radiation_sensor_radiation(eventMask e, prompt &item);
result i2c_solar_radiation_save_all(eventMask e, prompt &item);

result i2c_th_address(eventMask e, prompt &item);
result i2c_th_oneshot(eventMask e, prompt &item);
result i2c_th_sensor_type1(eventMask e, prompt &item);
result i2c_th_sensor_address1(eventMask e, prompt &item);
result i2c_th_sensor_type2(eventMask e, prompt &item);
result i2c_th_sensor_address2(eventMask e, prompt &item);
result i2c_th_save_all(eventMask e, prompt &item);

result i2c_rain_address(eventMask e, prompt &item);
result i2c_rain_oneshot(eventMask e, prompt &item);
result i2c_rain_tipping_bucket_time(eventMask e, prompt &item);
result i2c_rain_rain_for_tip(eventMask e, prompt &item);
result i2c_rain_save_all(eventMask e, prompt &item);

result i2c_power_address(eventMask e, prompt &item);
result i2c_power_oneshot(eventMask e, prompt &item);
result i2c_power_voltage_max_panel(eventMask e, prompt &item);
result i2c_power_voltage_max_battery(eventMask e, prompt &item);
result i2c_power_save_all(eventMask e, prompt &item);


result i2c_wind_address(eventMask e, prompt &item);
result i2c_wind_oneshot(eventMask e, prompt &item);
//result i2c_wind_type(eventMask e, prompt &item);
result i2c_wind_save_all(eventMask e, prompt &item);

result windsonic_sconfigurator(eventMask e, prompt &item);
result windsonic_configurator(eventMask e, prompt &item);

bool last_status;
bool true_idle_status=false;

uint8_t radiationAddress=I2C_SOLAR_RADIATION_DEFAULT_ADDRESS;
bool radiationOneshot=false;
float radiationOffset=0.0;
float radiationGain=1.0;
float radiationSensorVoltage=5000.0;
float radiationSensorRadiation=2000.0;

uint8_t thAddress=I2C_TH_DEFAULT_ADDRESS;
bool thOneshot=false;
char thSensorType1[]="HYT";
uint8_t thSensorAddress1=40;
char thSensorType2[]="   ";
uint8_t thSensorAddress2=0;

uint8_t rainAddress=I2C_RAIN_DEFAULT_ADDRESS;
bool rainOneshot=true;
uint16_t rainTippingBucketTime=50;
uint8_t rainRainForTip=1;

uint8_t powerAddress=I2C_POWER_DEFAULT_ADDRESS;
bool powerOneshot=false;
uint16_t powerVoltageMaxPanel=30000;
uint16_t powerVoltageMaxBattery=15000;

uint8_t windAddress=I2C_WIND_DEFAULT_ADDRESS;
bool windOneshot=false;
uint8_t windType=1;

char filename[100];
char path[100];

SDMenuT<CachedFSO<SdFat,32>> filePickMenu(sd,"Select conf file","/",filePick,enterEvent);

//implementing the handler here after filePick is defined...
result filePick(eventMask event, navNode& nav, prompt &item) {
  // switch(event) {//for now events are filtered only for enter, so we dont need this checking
  //   case enterCmd:
      if (nav.root->navFocus==(navTarget*)&filePickMenu) {
        Serial.println();
        Serial.print("selected file:");
        Serial.println(filePickMenu.selectedFile);
        Serial.print("from folder:");
        Serial.println(filePickMenu.selectedFolder);
	strncpy(filename,filePickMenu.selectedFile.c_str(),100);
	strncpy(path,filePickMenu.selectedFolder.c_str(),100);	  
      }
  //     break;
  // }
  return proceed;
}

MENU(subMenuMasterSave,"Save configuration",doNothing,noEvent,noStyle
	,OP("Yes write on EEPROM",i2c_master_save_all,enterEvent)
	,EXIT("<Back")
	);

MENU(subMenuMaster,"Master",doNothing,noEvent,noStyle
     ,SUBMENU(filePickMenu)
     ,SUBMENU(subMenuMasterSave)
     ,EXIT("<Back")
     );

TOGGLE(radiationOneshot,subMenuRadiationOneshot,"Oneshot: ",doNothing,noEvent,noStyle
       ,VALUE("True",true,i2c_solar_radiation_oneshot,exitEvent)
       ,VALUE("False",false,i2c_solar_radiation_oneshot,exitEvent)
       )

MENU(subMenuRadiationSave,"Save configuration",doNothing,noEvent,noStyle
	,OP("Yes",i2c_solar_radiation_save_all,enterEvent)
	,EXIT("<Back")
	);

MENU(subMenuRadiation,"i2c_radiation",doNothing,noEvent,noStyle
     ,FIELD(radiationAddress,"I2C address","",0,127,1,0,i2c_solar_radiation_address,exitEvent,noStyle)
     ,SUBMENU(subMenuRadiationOneshot)
     ,FIELD(radiationOffset,"ADC offset","",-127.0,127.0,1.0,0,i2c_solar_radiation_offset,exitEvent,noStyle)
     ,altFIELD(decPlaces<3>::menuField,radiationGain,"ADC gain","",0.,2.,0.1,0.001,i2c_solar_radiation_gain,exitEvent,noStyle)
     ,altFIELD(decPlaces<0>::menuField,radiationSensorVoltage  ,"Volt MAX"," mV",0.0,10000.0,100.0,1.0,i2c_solar_radiation_sensor_voltage,exitEvent,noStyle)
     ,altFIELD(decPlaces<0>::menuField,radiationSensorRadiation,"Radi MAX"," w/m^2",0.0,10000.0,100.0,1.0,i2c_solar_radiation_sensor_radiation,exitEvent,noStyle)
     ,SUBMENU(subMenuRadiationSave)
     ,EXIT("<Back")
     );

TOGGLE(thOneshot,subMenuThOneshot,"Oneshot: ",doNothing,noEvent,noStyle
       ,VALUE("True",true,i2c_th_oneshot,exitEvent)
       ,VALUE("False",false,i2c_th_oneshot,exitEvent)
       )

MENU(subMenuThSave,"Save configuration",doNothing,noEvent,noStyle
	,OP("Yes",i2c_th_save_all,enterEvent)
	,EXIT("<Back")
	);

const char* const alpha[] MEMMODE = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
MENU(subMenuTh,"i2c_th",doNothing,noEvent,noStyle
     ,FIELD(thAddress,"I2C address","",0,127,1,0,i2c_th_address,exitEvent,noStyle)
     ,SUBMENU(subMenuThOneshot)
     ,EDIT("sensor1 type",thSensorType1,alpha,i2c_th_sensor_type1,exitEvent,noStyle)
     ,FIELD(thSensorAddress1,"sensor1 address","",0,127,1,0,i2c_th_sensor_address1,exitEvent,noStyle)
     ,EDIT("sensor2 type",thSensorType2,alpha,i2c_th_sensor_type2,exitEvent,noStyle)
     ,FIELD(thSensorAddress2,"sensor2 address","",0,127,1,0,i2c_th_sensor_address2,exitEvent,noStyle)
     ,SUBMENU(subMenuThSave)
     ,EXIT("<Back")
     );

TOGGLE(rainOneshot,subMenuRainOneshot,"Oneshot: ",doNothing,noEvent,noStyle
       ,VALUE("True",true,i2c_rain_oneshot,exitEvent)
       ,VALUE("False",false,i2c_rain_oneshot,exitEvent)
       )

MENU(subMenuRainSave,"Save configuration",doNothing,noEvent,noStyle
	,OP("Yes",i2c_rain_save_all,enterEvent)
	,EXIT("<Back")
	);

MENU(subMenuRain,"i2c_rain",doNothing,noEvent,noStyle
     ,FIELD(rainAddress,"I2C address","",0,127,1,0,i2c_rain_address,exitEvent,noStyle)
     ,SUBMENU(subMenuRainOneshot)
     ,FIELD(rainTippingBucketTime,"Tip time","ms",0,1000,10,1,i2c_rain_tipping_bucket_time,exitEvent,noStyle)
     ,FIELD(rainRainForTip,"Tip value","mm/10",1,20,1,0,i2c_rain_rain_for_tip,exitEvent,noStyle)
     ,SUBMENU(subMenuRainSave)
     ,EXIT("<Back")
     );

TOGGLE(powerOneshot,subMenuPowerOneshot,"Oneshot: ",doNothing,noEvent,noStyle
       ,VALUE("True",true,i2c_power_oneshot,exitEvent)
       ,VALUE("False",false,i2c_power_oneshot,exitEvent)
       )

MENU(subMenuPowerSave,"Save configuration",doNothing,noEvent,noStyle
	,OP("Yes",i2c_power_save_all,enterEvent)
	,EXIT("<Back")
	);

MENU(subMenuPower,"i2c_power",doNothing,noEvent,noStyle
     ,FIELD(powerAddress,"I2C address","",0,127,1,0,i2c_power_address,exitEvent,noStyle)
     ,SUBMENU(subMenuPowerOneshot)
     ,FIELD(powerVoltageMaxPanel,"Panel max","mV",1,50000,100,1,i2c_power_voltage_max_panel,exitEvent,noStyle)
     ,FIELD(powerVoltageMaxBattery,"Battery max","mV",1,30000,100,1,i2c_power_voltage_max_battery,exitEvent,noStyle)
     ,SUBMENU(subMenuPowerSave)
     ,EXIT("<Back")
     );

TOGGLE(windOneshot,subMenuWindOneshot,"Oneshot: ",doNothing,noEvent,noStyle
       ,VALUE("True",true,i2c_wind_oneshot,exitEvent)
       ,VALUE("False",false,i2c_wind_oneshot,exitEvent)
       )
/*
SELECT(windType,subMenuWindType,"Sensor: ",i2c_wind_type,exitEvent,noStyle
       ,VALUE("Davis",1,doNothing,noEvent)
       ,VALUE("Inspeed",2,doNothing,noEvent)
       )
*/
MENU(subMenuWindSave,"Save configuration",doNothing,noEvent,noStyle
	,OP("Yes",i2c_wind_save_all,enterEvent)
	,EXIT("<Back")
	);

MENU(subMenuWind,"i2c_wind",doNothing,noEvent,noStyle
     ,FIELD(windAddress,"I2C address","",0,127,1,0,i2c_wind_address,exitEvent,noStyle)
     ,SUBMENU(subMenuWindOneshot)
     //,SUBMENU(subMenuWindType)
     ,SUBMENU(subMenuWindSave)
     ,EXIT("<Back")
     );

MENU(mainMenu,"Configuration",doNothing,noEvent,noStyle
     ,SUBMENU(subMenuMaster)
     ,SUBMENU(subMenuRadiation)
     ,SUBMENU(subMenuTh)
     ,SUBMENU(subMenuRain)
     ,SUBMENU(subMenuPower)
     ,SUBMENU(subMenuWind)
     ,OP("configure windsonic",windsonic_configurator,enterEvent)
     ,OP("sconfigure windsonic",windsonic_sconfigurator,enterEvent)
     ,OP("Scan i2c bus",scan_i2c_bus,enterEvent)
     ,EXIT("<Exit go to serial port")
     );

encoderIn<encA,encB> encoder;//simple quad encoder driver
encoderInStream<encA,encB> encStream(encoder);// simple encoder Stream

irIn<IR_PIN> ir;
irInStream<IR_PIN> irStream(ir);// simple IR Stream

//a keyboard with only one key as the encoder button
keyMap encBtn_map[]={{-encBtn,defaultNavCodes[enterCmd].ch}};//negative pin numbers use internal pull-up, this is on when low
keyIn<1> encButton(encBtn_map);//1 is the number of keys

menuIn* inputsList[]={&encButton};
//chainStream<1> in(inputsList);//1 is the number of inputs

serialIn serial(Serial);
//menuIn* inputsList[]={&serial};
//chainStream<1> in(inputsList);//1 is the number of inputs

MENU_INPUTS(in,&irStream,&encStream,&encButton,&serial);


const panel panels[] MEMMODE={{0,0,20,4}};
navNode* nodes[sizeof(panels)/sizeof(panel)];
panelsList pList(panels,nodes,1);

#define MAX_DEPTH 3

//define output device serial
idx_t serialTops[MAX_DEPTH];
serialOut outSerial(Serial,serialTops);

idx_t lcdTops[MAX_DEPTH];
liquidCrystalOut outLcd(lcd,lcdTops,pList);//output device for LCD

//define outputs controller
menuOut* const outputs[] MEMMODE={&outSerial,&outLcd};//list of output devices
outputsList out(outputs,sizeof(outputs)/sizeof(menuOut*));//outputs list controller

/*
MENU_OUTPUTS(out, MAX_DEPTH
  ,LIQUIDCRYSTAL_OUT(lcd,{0,0,20,4})
  ,NONE
);
*/

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

result display_status(menuOut& o,idleEvent e) {
  if (e==idling) {
    o.setCursor(0,0);
    o.print("Operation status:");
    o.setCursor(0,1);
    if (last_status){
      o.print("Success");
    }else{
      o.print("Error!");
    }
  }
  return proceed;
}

result display_nostatus(menuOut& o,idleEvent e) {
  if (e==idling) {
    o.setCursor(0,0);
    o.print("Operation status:");
    o.setCursor(0,1);
    o.print("Unknown");
    o.setCursor(0,2);
    o.print("Check serial output");
    o.setCursor(0,3);
    o.print("and wait");
  }
  return proceed;
}

//when menu is suspended
result idle(menuOut& o,idleEvent e) {
  o.clear();
  switch(e) {
  case idleStart:
    o.setCursor(0,0);
    o.print("suspending menu!");
    delay(1000);
    break;
  case idling:
    o.setCursor(0,0);
    o.print("suspended...");
    o.setCursor(0,1);
    o.print("go to serial port");
    true_idle_status=true;
  break;
  case idleEnd:
    o.setCursor(0,0);
    o.print("resuming menu.");
    true_idle_status=false;
    delay(1000);
    break;    
  }
  return proceed;
}

result look_at_serial_message(menuOut& o,idleEvent e) {
  if (e==idling) {
    o.setCursor(0,0);
    o.print("Go to serial port");
    o.setCursor(0,1);
    o.print("to see output");
  }
  return proceed;
}

result scan_i2c_bus() {

  nav.idleOn(look_at_serial_message);
  
  byte start_address = 1;
  byte end_address = 127;

  Serial.println("\nI2CScanner ready!");
  Serial.print("starting scanning of I2C bus from ");
  Serial.print(start_address,HEX);
  Serial.print(" to ");
  Serial.print(end_address,HEX);
  Serial.println("...Hex");
  // start the scan, will call "scanFunc()" on result from each address
  scanI2CBus( start_address, end_address, scanFunc ); 
  Serial.println("\ndone");
 
  //nav.idleOff();
  return proceed;
}

//// ISR for encoder management
void encoderprocess (){
  encoder.process();
}

// Scan the I2C bus between addresses from_addr and to_addr.
// On each address, call the callback function with the address and result.
// If result==0, address was found, otherwise, address wasn't found
// (can use result to potentially get other status on the I2C bus, see twi.c)
// Assumes Wire.begin() has already been called
void scanI2CBus(byte from_addr, byte to_addr,
                void(*callback)(byte address, byte result) )
{
  byte rc;
  //byte data = 0; // not used, just an address to feed to twi_writeTo()
  for( byte addr = from_addr; addr <= to_addr; addr++ ) {

    Wire.beginTransmission (addr);
    rc = (Wire.endTransmission () != 0);
    //rc = twi_writeTo(addr, &data, 0, 1, 0);
    callback( addr, rc );
  }
}

// Called when address is found in scanI2CBus()
// Feel free to change this as needed
// (like adding I2C comm code to figure out what kind of I2C device is there)
void scanFunc( byte addr, byte result ) {
  Serial.print("addr: ");
  Serial.print(addr,HEX);
  Serial.print(" ");
  Serial.print(addr);
  Serial.print( (result==0) ? " Found!":"       ");
  Serial.print( (addr%4) ? "\t":"\n\r");
}

void windsonicSerialReset() {
  // reset serial settings
  Serial1.setTimeout(GWS_SERIAL_TIMEOUT_MS);
  Serial1.begin(GWS_SERIAL_BAUD);
  windsonicFlush();
}

void windsonicReceiveTerminatedMessage(const char terminator){
  // receive message on serial until terminator is found and terminate message
  uart_rx_buffer_length=Serial1.readBytesUntil(terminator, uart_rx_buffer, UART_RX_BUFFER_LENGTH);
  uart_rx_buffer[uart_rx_buffer_length] = '\0';
  uart_rx_buffer_length++;
  Serial.print(F("receive:"));
  Serial.println(uart_rx_buffer);
}

void windsonicReceiveMessage(const char terminator){
  uart_rx_buffer_length=Serial1.readBytesUntil(terminator, uart_rx_buffer, UART_RX_BUFFER_LENGTH);
}

void windsonicPowerOff () {
  digitalWrite(WIND_POWER_PIN, LOW);
}

void windsonicPowerOn () {
  digitalWrite(WIND_POWER_PIN, HIGH);
}

void windsonicFlush(void){
  while (Serial1.available() > 0){
    //Serial1.read();
    Serial.write((uint8_t)Serial1.read());
    Serial.flush();
  }
  memset(uart_rx_buffer, 0, UART_RX_BUFFER_LENGTH);
  uart_rx_buffer_length = 0;
}

bool windsonicEnterConfigMode(void){
  // try to enter in config mode
  // windsonic can be in polled or continuous mode
  // return true on success
  
  uint8_t count=0;
  bool config_mode=false;
  Serial1.setTimeout(1000);  
  Serial.println(F("try to enter configure mode"));
  windsonicFlush();
  do {
    /*
    if ((count % 2) == 0) {
      Serial.println("send:*Q");
      Serial1.print("*Q");
      Serial1.flush();
    }else{
      Serial.println("send:*");
      Serial1.print("*");
      Serial1.flush();
    }
    */

    Serial.println("send:**Q");
    Serial1.print("**Q");
    delay(100);
    Serial.println("send:**Q");
    Serial1.print("**Q");
    delay(100);
    Serial.println("send:**Q");
    Serial1.print("**Q");
    Serial1.flush();
    
    #define CONFMSG "CONFIGURATION MODE"
    uint8_t countr=0;
    do{
      uart_rx_buffer_length=Serial1.readBytesUntil('\r', uart_rx_buffer, UART_RX_BUFFER_LENGTH-1);
      countr++;
      if (uart_rx_buffer_length >= strlen(CONFMSG)){
	uart_rx_buffer[uart_rx_buffer_length] = '\0';
	uart_rx_buffer_length++;
	Serial.print(F("receive:"));
	Serial.println(uart_rx_buffer);
	if (strcmp(&uart_rx_buffer[uart_rx_buffer_length-strlen(CONFMSG)-1],CONFMSG)==0){
	  Serial.println(F("entered configure mode"));
	  config_mode=true;
	  break;
	}
      }
      //} while ((uart_rx_buffer_length > 0) || (countr < 10));
    } while (countr < 3);
    if (!config_mode) {
      count++;
      delay(1000);
      Serial.println("send:Q");
      Serial1.print("Q\r\n");
      Serial1.flush();
      delay(1000);
      windsonicFlush();
    }
  } while ((!config_mode) && (count < 2));
  
  windsonicFlush();
  return config_mode;
}


// this is required to reset windsonic to default configuration and baud
bool windsonicEnterConfigModeAllBaudrate() {
  // try to enter in config mode
  // windsonic can be in polled or continuous mode
  // no matter about witch baudrate windsonic want to comunicate
  // return true on success

  // change baudrate on winsonic is a very strange procedure!
  // it need confirmation and we need to set other things before
  // quitting to force windsonic to save parameters
  
  /* Initialize serial for wind sensor comunication
     WindSonic default settings are :
     Bits per second            option 1: 9600 ; option 2 : 19200
     Data bits                  8
     Parity                     None
     Stop bits                  1
     Flow Control(Handshaking)  None
  */

  // try different fixed baud rate
  long int baudrate []={9600,2400,4800,19200,38400};

  for (byte i=0; (i<(sizeof(baudrate) / sizeof(long int))); i++) {

    //ATTENTION here all is blocking!

    Serial.print(F("TRY BAUDRATE: "));
    Serial.println(baudrate[i]);
    Serial1.begin(baudrate[i]);
    
    if (windsonicEnterConfigMode()){   
      Serial.println(F("send: D3"));
      Serial1.print("D3\r\n");
      windsonicReceiveTerminatedMessage('\r');
      delay(500);
      windsonicFlush();

      Serial.println(F("send: M2"));
      Serial1.print("M2\r\n");
      windsonicReceiveTerminatedMessage('\r');
      delay(500);
      windsonicFlush();

      Serial.println(F("send: M4"));
      Serial1.print("M4\r\n");
      windsonicReceiveTerminatedMessage('\r');
      delay(500);
      windsonicFlush();
      
      Serial.println(F("send: D3"));
      Serial1.print("D3\r\n");
      windsonicReceiveTerminatedMessage('\r');
      delay(500);
      windsonicFlush();
      
      Serial.println(F("baudrate found"));
          
      //// set Communications protocol RS232
      //Serial.println(F("send: E3"));
      //Serial1.print("E3\r\n");
      //windsonicReceiveTerminatedMessage('\r');
      //delay(500);
      //windsonicFlush();

      // set Baud rate 9600
      Serial.println(F("send: B3"));
      Serial1.print("B3\r\n");
      delay(1000);
      windsonicFlush();
      Serial1.end();
      Serial1.begin(GWS_SERIAL_BAUD);
      delay(1000);
      Serial.println(F("send: B at 9600"));
      Serial1.print("B\r\n");
      delay(1000);
      windsonicFlush();

      Serial.println(F("send: M2"));
      Serial1.print("M2\r\n");
      windsonicReceiveTerminatedMessage('\r');
      delay(500);
      windsonicFlush();
      
      Serial.println("send:Q");
      Serial1.print("Q\r\n");
      Serial1.flush();
      delay(1000);
      windsonicFlush();

      if (!windsonicEnterConfigMode()){
	Serial.println("failed");
      } else { 
	return true;
      }
    }

    windsonicPowerOff();
    delay(1000);
    windsonicPowerOn();
    delay(WIND_POWER_ON_DELAY_MS);
    windsonicSerialReset();
       
  }
  Serial.println(F("inizialize failed"));
  return false;
}

// this is required to reset windsonic to default configuration and baud
bool windsonicInitSafeMode() {
  //use safe mode to reset windsonic to comunicate on RS232
  // safe mode in windsonic is very strange
  // only some paramters are taken in account for save
  // for example baudrate can be changed only in configuration mode ...
  
  Serial.println(F("TRY SAFE MODE"));
  Serial1.end();
  Serial1.begin(19200);
  Serial1.setTimeout(10);  
  windsonicFlush();

  uint8_t count=0;
  bool config_mode=false;
  #define SAFEMSG "SAFE MODE (RS232 ONLY)"
  do {
    Serial.println(F("try to enter safe mode"));
    windsonicPowerOff();
    delay(2000);
    windsonicPowerOn();
    uint16_t countr=0;
    while (countr < 400){
      Serial1.print("*****************");      // enter in setup
      Serial1.flush();
      countr++;
      uart_rx_buffer_length=Serial1.readBytesUntil('\r', uart_rx_buffer, UART_RX_BUFFER_LENGTH-1);
      if (uart_rx_buffer_length >= strlen(SAFEMSG)){
	uart_rx_buffer[uart_rx_buffer_length] = '\0';
	uart_rx_buffer_length++;
	Serial.print(F("receive:"));
	Serial.println(uart_rx_buffer);
	if (strcmp(&uart_rx_buffer[uart_rx_buffer_length-strlen(SAFEMSG)-1],SAFEMSG)==0){
	  Serial.println(F("entered safe mode"));
	  config_mode=true;
	  break;
	}
      }
    }
    count++;
  } while ((!config_mode) && (count < 3));
  
  Serial1.print("\r\n");
  delay(500);
  windsonicFlush();
  
  if (config_mode){


    Serial.println(F("send: M2"));
    Serial1.print("M2\r\n");
    windsonicReceiveTerminatedMessage('\r');
    delay(500);
    windsonicFlush();
    
    Serial.println(F("send: M4"));
    Serial1.print("M4\r\n");
    windsonicReceiveTerminatedMessage('\r');
    delay(500);
    windsonicFlush();
      
    Serial.println(F("send: D3"));
    Serial1.print("D3\r\n");
    windsonicReceiveTerminatedMessage('\r');
    delay(500);
    windsonicFlush();

    // set Communications protocol RS232
    Serial.println(F("send: E3"));
    Serial1.print("E3\r\n");
    windsonicReceiveTerminatedMessage('\r');
    delay(2000);
    windsonicFlush();

    // set Baud rate 9600
    Serial.println(F("send: B3"));
    Serial1.print("B3\r\n");
    delay(1000);
    windsonicFlush();
    /*
    Serial1.end();
    Serial1.begin(GWS_SERIAL_BAUD);
    delay(1000);
    Serial.println(F("send: B at 9600"));
    Serial1.print("B\r\n");
    delay(1000);
    windsonicFlush();
    */
    
    Serial.println(F("send: M2"));
    Serial1.print("M2\r\n");
    windsonicReceiveTerminatedMessage('\r');
    delay(500);
    windsonicFlush();
    
    Serial.println("send:Q");
    Serial1.print("Q\r\n");
    Serial1.flush();
    delay(1000);
    windsonicFlush();

    Serial1.end();
    Serial1.begin(GWS_SERIAL_BAUD);

    windsonicPowerOff();
    delay(1000);
    windsonicPowerOn();
    delay(WIND_POWER_ON_DELAY_MS);
    windsonicSerialReset();

    
    if (!windsonicEnterConfigMode()){
      Serial.println("failed");
    } else { 
      return true;
    }
  }
    
  windsonicPowerOff();
  delay(1000);
  windsonicPowerOn();
  delay(WIND_POWER_ON_DELAY_MS);
  windsonicSerialReset();
  Serial.println("failed enter safe mode");
  return false;

}

void windsonicConfigure(void){
  // configure windsonic starting by any unknow settings
  // enter in safe mode to change comunication port
  // change RS232 baudrate
  // enter in config mode and set all parameters
  
  Serial.println(F("go to configure Windsonic"));

  while (!windsonicInitSafeMode()) {
    if (windsonicEnterConfigModeAllBaudrate()) break;
  }
  
  Serial.println(F("send: L1"));
  Serial1.print("L1\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: C2"));
  Serial1.print("C2\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: H2"));
  Serial1.print("H2\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: K50"));
  Serial1.print("K50\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: M4"));
  Serial1.print("M4\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
    
  Serial.println(F("send: NQ"));
  Serial1.print("NQ\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: O1"));
  Serial1.print("O1\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: U1"));
  Serial1.print("U1\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
    
  Serial.println(F("send: Y1"));
  Serial1.print("Y1\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: P2"));
  Serial1.print("P2\r\n");   // 2 per second;  required by i2c-wind setted for 1 measure by sec.
  windsonicReceiveMessage('\r');
  delay(500);
  windsonicFlush();

  Serial.println(F("exit configure mode"));
  Serial1.print("Q\r\n");
  delay(10);
  Serial1.print("Q\r\n");
  windsonicFlush();
  delay(1000);
  windsonicFlush();
}

void windsonicSconfigure(void){
  // unset any usefull settings on windsonic
  
  Serial.println(F("go to sconfigure Windsonic"));

  while (!windsonicEnterConfigModeAllBaudrate()) {
    windsonicInitSafeMode();
  }
  Serial.println(F("send: M4"));
  Serial1.print("M4\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: E2"));
  Serial1.print("E2\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: B2"));
  Serial1.print("B2\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  Serial1.end();
  Serial1.begin(4800);
  Serial1.print("B\r\n");
  delay(500);
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();

  Serial.println(F("send: M2"));
  Serial1.print("M2\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();

  Serial.println(F("exit configure mode"));
  Serial1.print("Q\r\n");
  delay(10);
  Serial1.print("Q\r\n");
  windsonicFlush();
  delay(1000);
  windsonicFlush();
}

char getCommand()
{
  while (Serial.available())
  {
    Serial.read();
  }
  
  while (!Serial.available())
  {
    delay(1);
  }
  return Serial.read();
}

void displayHelp()
{
  Serial.print(F("\nSensor configuration - "));
  Serial.println(version);
  Serial.println();
  Serial.println(F("scan I2C bus:"));
  Serial.println(F("\ti = scan one time"));
  Serial.println();
  Serial.println(F("Sensor to config:"));
  Serial.println(F("\ts = i2c-radiation"));
  Serial.println(F("\tt = i2c-th"));
  Serial.println(F("\tr = i2c-rain"));
  Serial.println(F("\tp = i2c-power"));
  Serial.println(F("\tw = i2c-wind"));
  Serial.println(F("\tu = windsonic sconfigurator ! (use to sconfigure your sensor!)"));
  Serial.println(F("\tv = windsonic setup"));
  Serial.println(F("\tz = windsonic transparent mode"));
  Serial.println();
  Serial.println(F("\t# = return to LCD display for commands"));
  //Serial.println(F("Output:"));
  //Serial.println(F("\tp = toggle printAll - printFound."));
  //Serial.println(F("\th = toggle header - noHeader."));
  Serial.println(F("\n\? = help - this page"));
  Serial.println();
}

configuration_t configuration;

void print_configuration() {
   LOGN(F("--> type: %d"), configuration.module_type);
   LOGN(F("--> version: %d.%d"), MODULE_MAIN_VERSION, MODULE_MINOR_VERSION);
   LOGN(F("--> configuration version: %d.%d"), configuration.module_main_version, configuration.module_configuration_version);
   LOGN(F("--> sensors: %d"), configuration.sensors_count);
   for (uint8_t i=0; i<configuration.sensors_count; i++) {
     LOGN(F("--> SD %d:  %s : %s"), i,configuration.sensors[i].driver,configuration.sensors[i].type);
   }
   LOGN(F("--> ConstantData: %d"), configuration.constantdata_count);
   for (uint8_t i=0; i<configuration.constantdata_count; i++) {
     LOGN(F("--> CD %d:  %s : %s"), i,configuration.constantdata[i].btable,configuration.constantdata[i].value);
   }
   #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
   LOGN(F("--> dhcp: %s"), configuration.is_dhcp_enable ? "on" : "off");
   LOGN(F("--> ethernet mac: %02X:%02X:%02X:%02X:%02X:%02X"), configuration.ethernet_mac[0], configuration.ethernet_mac[1], configuration.ethernet_mac[2], configuration.ethernet_mac[3], configuration.ethernet_mac[4], configuration.ethernet_mac[5]);

   #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
   LOGN(F("--> gsm apn: %s"), configuration.gsm_apn);
   LOGN(F("--> gsm username: %s"), configuration.gsm_username);
   LOGN(F("--> gsm password: %s"), configuration.gsm_password);

   #endif

   #if (USE_NTP)
   LOGN(F("--> ntp server: %s"), configuration.ntp_server);
   #endif

   #if (USE_MQTT)
   LOGN(F("--> mqtt server: %s"), configuration.mqtt_server);
   LOGN(F("--> mqtt port: %d"), configuration.mqtt_port);
   LOGN(F("--> mqtt root topic: %s"), configuration.mqtt_root_topic);
   LOGN(F("--> mqtt maint topic: %s"), configuration.mqtt_maint_topic);
   LOGN(F("--> mqtt rpc topic: %s"), configuration.mqtt_rpc_topic);
   LOGN(F("--> mqtt username: %s"), configuration.mqtt_username);
   LOGN(F("--> mqtt password: %s"), configuration.mqtt_password);
   LOGN(F("--> station slug: %s"), configuration.stationslug);
   LOGN(F("--> board   slug: %s"), configuration.boardslug);
   #endif
}


result i2c_master_save_all(eventMask e, prompt &item) {
  
  
  Serial.println("");
  Serial.println("read file");
  Serial.println(path);
  Serial.println(filename);

  last_status = false;
  sd.chdir(path);
  if (sd.exists(filename)) {
    Serial.println(F("file exists"));
    // try to open file. if ok, read configuration data.
    File configurationFile;
    configurationFile = sd.open(filename, O_RDONLY);
    if (configurationFile) {
      if (configurationFile.read(&configuration, sizeof(configuration)) == sizeof(configuration)) {
	configurationFile.close();
	last_status = true;
	Serial.println(F("configuration file read"));

	print_configuration();
	EEPROM.put(CONFIGURATION_EEPROM_ADDRESS, configuration);

      }else{
	Serial.println(F("error reading configuration file"));
      }
      configurationFile.close();
    } else {
      Serial.println(F("error opening configuration file"));
    }	 
  } else {
    Serial.println(F("file  do not exists"));
  }

  /*
  Serial.println(sizeof(configuration));
  uint8_t* data = (uint8_t*)&configuration;
  for (uint16_t i=0; i < sizeof(configuration); i++){
    Serial.println(*data);
    data++;
  }
  */
  
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_solar_radiation_address(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_SOLAR_RADIATION_DEFAULT_ADDRESS);
  buffer[0]=I2C_SOLAR_RADIATION_ADDRESS_ADDRESS;
  buffer[1]=radiationAddress;
  buffer[I2C_SOLAR_RADIATION_ADDRESS_LENGTH+1]=crc8(buffer, I2C_SOLAR_RADIATION_ADDRESS_LENGTH+1);
  Wire.write(buffer,I2C_SOLAR_RADIATION_ADDRESS_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_solar_radiation_address(eventMask e, prompt &item) {
  last_status=do_i2c_solar_radiation_address();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_solar_radiation_oneshot(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_SOLAR_RADIATION_DEFAULT_ADDRESS);
  buffer[0]=I2C_SOLAR_RADIATION_ONESHOT_ADDRESS;
  buffer[1]=(bool)(radiationOneshot);
  buffer[I2C_SOLAR_RADIATION_ONESHOT_LENGTH+1]=crc8(buffer, I2C_SOLAR_RADIATION_ONESHOT_LENGTH+1);
  Wire.write(buffer,I2C_SOLAR_RADIATION_ONESHOT_LENGTH+2);
  return(Wire.endTransmission() == 0);
}
result i2c_solar_radiation_oneshot(eventMask e, prompt &item) {
  last_status=do_i2c_solar_radiation_oneshot();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_solar_radiation_offset(void){
    uint8_t buffer[32];
  Wire.beginTransmission(I2C_SOLAR_RADIATION_DEFAULT_ADDRESS);
  buffer[0]=I2C_SOLAR_RADIATION_ADC_CALIBRATION_OFFSET_ADDRESS+0x04;
  memcpy( &buffer[1],&radiationOffset, sizeof(radiationOffset));
  buffer[sizeof(radiationOffset)+1]=crc8(buffer, sizeof(radiationOffset)+1);
  Wire.write(buffer,sizeof(radiationOffset)+2);
  return (Wire.endTransmission() == 0);
}
result i2c_solar_radiation_offset(eventMask e, prompt &item) {
  last_status=do_i2c_solar_radiation_offset();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_solar_radiation_gain(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_SOLAR_RADIATION_DEFAULT_ADDRESS);
  buffer[0]=I2C_SOLAR_RADIATION_ADC_CALIBRATION_GAIN_ADDRESS+0x04;
  memcpy( &buffer[1],&radiationGain, sizeof(radiationGain));
  buffer[sizeof(radiationGain)+1]=crc8(buffer, sizeof(radiationGain)+1);
  Wire.write(buffer,sizeof(radiationGain)+2);
  return (Wire.endTransmission() == 0);
}
result i2c_solar_radiation_gain(eventMask e, prompt &item) {
  last_status=do_i2c_solar_radiation_gain();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_solar_radiation_sensor_voltage(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_SOLAR_RADIATION_DEFAULT_ADDRESS);
  buffer[0]=I2C_SOLAR_RADIATION_SENSOR_VOLTAGE_MAX_ADDRESS+0x04;
  memcpy( &buffer[1],&radiationSensorVoltage, sizeof(radiationSensorVoltage));
  buffer[sizeof(radiationSensorVoltage)+1]=crc8(buffer, sizeof(radiationSensorVoltage)+1);
  Wire.write(buffer,sizeof(radiationSensorVoltage)+2);
  return (Wire.endTransmission() == 0);
}
result i2c_solar_radiation_sensor_voltage(eventMask e, prompt &item) {
  last_status=do_i2c_solar_radiation_sensor_voltage();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_solar_radiation_sensor_radiation(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_SOLAR_RADIATION_DEFAULT_ADDRESS);
  buffer[0]=I2C_SOLAR_RADIATION_SENSOR_RADIATION_MAX_ADDRESS+0x04;
  memcpy( &buffer[1],&radiationSensorRadiation, sizeof(radiationSensorRadiation));
  buffer[sizeof(radiationSensorRadiation)+1]=crc8(buffer, sizeof(radiationSensorRadiation)+1);
  Wire.write(buffer,sizeof(radiationSensorRadiation)+2);
  return (Wire.endTransmission() == 0);
}
result i2c_solar_radiation_sensor_radiation(eventMask e, prompt &item) {
  last_status=do_i2c_solar_radiation_sensor_radiation();
  nav.idleOn(display_status);
  return proceed;
}


bool do_i2c_solar_radiation_save(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_SOLAR_RADIATION_DEFAULT_ADDRESS);
  buffer[0]=I2C_COMMAND_ID;
  buffer[1]=I2C_SOLAR_RADIATION_COMMAND_SAVE;
  buffer[2]=crc8(buffer, 2);
  Wire.write(buffer,3);
  return (Wire.endTransmission() == 0);
}
/*
result i2c_solar_radiation_save(eventMask e, prompt &item) {
  last_status=do_i2c_solar_radiation_save();
  nav.idleOn(display_status);
  return proceed;
}
*/
result i2c_solar_radiation_save_all(eventMask e, prompt &item) {
  last_status                =do_i2c_solar_radiation_address();
  if(last_status) last_status=do_i2c_solar_radiation_oneshot();
  if(last_status) last_status=do_i2c_solar_radiation_offset();
  if(last_status) last_status=do_i2c_solar_radiation_gain();
  if(last_status) last_status=do_i2c_solar_radiation_sensor_voltage();
  if(last_status) last_status=do_i2c_solar_radiation_sensor_radiation();
  if(last_status) last_status=do_i2c_solar_radiation_save();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_th_address(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_TH_DEFAULT_ADDRESS);
  buffer[0]=I2C_TH_ADDRESS_ADDRESS;
  buffer[1]=thAddress;
  buffer[I2C_TH_ADDRESS_LENGTH+1]=crc8(buffer, I2C_TH_ADDRESS_LENGTH+1);
  Wire.write(buffer,I2C_TH_ADDRESS_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_th_address(eventMask e, prompt &item) {
  last_status=do_i2c_th_address();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_th_oneshot(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_TH_DEFAULT_ADDRESS);
  buffer[0]=I2C_TH_ONESHOT_ADDRESS;
  buffer[1]=(bool)(thOneshot);
  buffer[I2C_TH_ONESHOT_LENGTH+1]=crc8(buffer, I2C_TH_ONESHOT_LENGTH+1);
  Wire.write(buffer,I2C_TH_ONESHOT_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_th_oneshot(eventMask e, prompt &item) {
  last_status=do_i2c_th_oneshot();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_th_sensor_type1(void){
  char charbuffer[32];
  Wire.beginTransmission(I2C_TH_DEFAULT_ADDRESS);
  charbuffer[0]=I2C_TH_SENSOR1_TYPE_ADDRESS;
  strncpy(&charbuffer[1],thSensorType1,4);
  charbuffer[I2C_TH_SENSOR1_TYPE_LENGTH+1]=crc8((uint8_t*)charbuffer, I2C_TH_SENSOR1_TYPE_LENGTH+1);
  Wire.write(charbuffer,I2C_TH_SENSOR1_TYPE_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_th_sensor_type1(eventMask e, prompt &item) {
  last_status=do_i2c_th_sensor_type1();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_th_sensor_address1(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_TH_DEFAULT_ADDRESS);
  buffer[0]=I2C_TH_SENSOR1_I2C_ADDRESS_ADDRESS;
  buffer[1]=thSensorAddress1;
  buffer[I2C_TH_SENSOR1_I2C_ADDRESS_LENGTH+1]=crc8(buffer, I2C_TH_SENSOR1_I2C_ADDRESS_LENGTH+1);
  Wire.write(buffer,I2C_TH_SENSOR1_I2C_ADDRESS_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_th_sensor_address1(eventMask e, prompt &item) {
  last_status=do_i2c_th_sensor_address1();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_th_sensor_type2(void){
  char charbuffer[32];
  Wire.beginTransmission(I2C_TH_DEFAULT_ADDRESS);
  charbuffer[0]=I2C_TH_SENSOR2_TYPE_ADDRESS;
  strncpy(&charbuffer[1],thSensorType2,4);
  charbuffer[I2C_TH_SENSOR2_TYPE_LENGTH+1]=crc8((uint8_t*)charbuffer, I2C_TH_SENSOR2_TYPE_LENGTH+1);
  Wire.write(charbuffer,I2C_TH_SENSOR2_TYPE_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_th_sensor_type2(eventMask e, prompt &item) {
  last_status=do_i2c_th_sensor_type2();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_th_sensor_address2(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_TH_DEFAULT_ADDRESS);
  buffer[0]=I2C_TH_SENSOR2_I2C_ADDRESS_ADDRESS;
  buffer[1]=thSensorAddress2;
  buffer[I2C_TH_SENSOR2_I2C_ADDRESS_LENGTH+1]=crc8(buffer, I2C_TH_SENSOR2_I2C_ADDRESS_LENGTH+1);
  Wire.write(buffer,I2C_TH_SENSOR2_I2C_ADDRESS_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_th_sensor_address2(eventMask e, prompt &item) {
  last_status=do_i2c_th_sensor_address2();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_th_save(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_TH_DEFAULT_ADDRESS);
  buffer[0]=I2C_COMMAND_ID;
  buffer[1]=I2C_TH_COMMAND_SAVE;
  buffer[2]=crc8(buffer, 2);
  Wire.write(buffer,3);
  return (Wire.endTransmission() == 0);
}
/*
result i2c_th_save(eventMask e, prompt &item) {
  last_status=do_i2c_th_save();
  nav.idleOn(display_status);
  return proceed;
}
*/
result i2c_th_save_all(eventMask e, prompt &item) {
  last_status                =do_i2c_th_address();
  if(last_status) last_status=do_i2c_th_oneshot();
  if(last_status) last_status=do_i2c_th_sensor_type1();
  if(last_status) last_status=do_i2c_th_sensor_address1();
  if(last_status) last_status=do_i2c_th_sensor_type2();
  if(last_status) last_status=do_i2c_th_sensor_address2();
  if(last_status) last_status=do_i2c_th_save();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_rain_address(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_RAIN_DEFAULT_ADDRESS);
  buffer[0]=I2C_RAIN_ADDRESS_ADDRESS;
  buffer[1]=rainAddress;
  buffer[I2C_RAIN_ADDRESS_LENGTH+1]=crc8(buffer, I2C_RAIN_ADDRESS_LENGTH+1);
  Wire.write(buffer,I2C_RAIN_ADDRESS_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_rain_address(eventMask e, prompt &item) {
  last_status=do_i2c_rain_address();
  nav.idleOn(display_status);
  return proceed;
}


bool do_i2c_rain_oneshot(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_RAIN_DEFAULT_ADDRESS);
  buffer[0]=I2C_RAIN_ONESHOT_ADDRESS;
  buffer[1]=(bool)(rainOneshot);
  buffer[I2C_RAIN_ONESHOT_LENGTH+1]=crc8(buffer, I2C_RAIN_ONESHOT_LENGTH+1);
  Wire.write(buffer,I2C_RAIN_ONESHOT_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_rain_oneshot(eventMask e, prompt &item) {
  last_status=do_i2c_rain_oneshot();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_rain_rain_for_tip(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_RAIN_DEFAULT_ADDRESS);
  buffer[0]=I2C_RAIN_RAINFORTIP_ADDRESS;
  buffer[1]=rainRainForTip;
  buffer[I2C_RAIN_RAINFORTIP_LENGTH+1]=crc8(buffer, I2C_RAIN_RAINFORTIP_LENGTH+1);
  Wire.write(buffer,I2C_RAIN_RAINFORTIP_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_rain_rain_for_tip(eventMask e, prompt &item) {
  last_status=do_i2c_rain_rain_for_tip();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_rain_tipping_bucket_time(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_RAIN_DEFAULT_ADDRESS);
  buffer[0]=I2C_RAIN_TIPTIME_ADDRESS;
  buffer[1]=(uint8_t)rainTippingBucketTime;
  buffer[2]=(uint8_t)(rainTippingBucketTime >> 8); // Get upper byte of 16-bit var
  buffer[I2C_RAIN_TIPTIME_LENGTH+1]=crc8(buffer, I2C_RAIN_TIPTIME_LENGTH+1);
  Wire.write(buffer,I2C_RAIN_TIPTIME_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_rain_tipping_bucket_time(eventMask e, prompt &item) {
  last_status=do_i2c_rain_tipping_bucket_time();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_rain_save(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_RAIN_DEFAULT_ADDRESS);
  buffer[0]=I2C_COMMAND_ID;
  buffer[1]=I2C_RAIN_COMMAND_SAVE;
  buffer[2]=crc8(buffer, 2);
  Wire.write(buffer,3);
  return (Wire.endTransmission() == 0);
}
/*
result i2c_rain_save(eventMask e, prompt &item) {
  last_status=do_i2c_rain_save();
  nav.idleOn(display_status);
  return proceed;
}
*/
result i2c_rain_save_all(eventMask e, prompt &item) {
  last_status                =do_i2c_rain_address();
  if(last_status) last_status=do_i2c_rain_oneshot();
  if(last_status) last_status=do_i2c_rain_rain_for_tip();
  if(last_status) last_status=do_i2c_rain_tipping_bucket_time();
  if(last_status) last_status=do_i2c_rain_save();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_power_address(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_POWER_DEFAULT_ADDRESS);
  buffer[0]=I2C_POWER_ADDRESS_ADDRESS;
  buffer[1]=powerAddress;
  buffer[I2C_POWER_ADDRESS_LENGTH+1]=crc8(buffer, I2C_POWER_ADDRESS_LENGTH+1);
  Wire.write(buffer,I2C_POWER_ADDRESS_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_power_address(eventMask e, prompt &item) {
  last_status=do_i2c_power_address();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_power_oneshot(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_POWER_DEFAULT_ADDRESS);
  buffer[0]=I2C_POWER_ONESHOT_ADDRESS;
  buffer[1]=(bool)(powerOneshot);
  buffer[I2C_POWER_ONESHOT_LENGTH+1]=crc8(buffer, I2C_POWER_ONESHOT_LENGTH+1);
  Wire.write(buffer,I2C_POWER_ONESHOT_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_power_oneshot(eventMask e, prompt &item) {
  last_status=do_i2c_power_oneshot();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_power_voltage_max_panel(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_POWER_DEFAULT_ADDRESS);
  buffer[0]=I2C_POWER_VOLTAGE_MAX_PANEL_ADDRESS;
  memcpy( &buffer[1], &powerVoltageMaxPanel,sizeof(powerVoltageMaxPanel));
  buffer[I2C_POWER_VOLTAGE_MAX_PANEL_LENGTH+1]=crc8(buffer, I2C_POWER_VOLTAGE_MAX_PANEL_LENGTH+1);
  Wire.write(buffer,I2C_POWER_VOLTAGE_MAX_PANEL_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_power_voltage_max_panel(eventMask e, prompt &item) {
  last_status=do_i2c_power_voltage_max_panel();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_power_voltage_max_battery(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_POWER_DEFAULT_ADDRESS);
  buffer[0]=I2C_POWER_VOLTAGE_MAX_BATTERY_ADDRESS;
  memcpy( &buffer[1],&powerVoltageMaxBattery, sizeof(powerVoltageMaxBattery));
  buffer[I2C_POWER_VOLTAGE_MAX_BATTERY_LENGTH+1]=crc8(buffer, I2C_POWER_VOLTAGE_MAX_BATTERY_LENGTH+1);
  Wire.write(buffer,I2C_POWER_VOLTAGE_MAX_BATTERY_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_power_voltage_max_battery(eventMask e, prompt &item) {
  last_status=do_i2c_power_voltage_max_battery();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_power_save(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_POWER_DEFAULT_ADDRESS);
  buffer[0]=I2C_COMMAND_ID;
  buffer[1]=I2C_POWER_COMMAND_SAVE;
  buffer[2]=crc8(buffer, 2);
  Wire.write(buffer,3);
  return (Wire.endTransmission() == 0);
}
/*
result i2c_power_save(eventMask e, prompt &item) {
  last_status=do_i2c_power_save();
  nav.idleOn(display_status);
  return proceed;
}
*/
result i2c_power_save_all(eventMask e, prompt &item) {
  last_status                =do_i2c_power_address();
  if(last_status) last_status=do_i2c_power_oneshot();
  if(last_status) last_status=do_i2c_power_voltage_max_panel();
  if(last_status) last_status=do_i2c_power_voltage_max_battery();
  if(last_status) last_status=do_i2c_power_save();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_wind_address(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_WIND_DEFAULT_ADDRESS);
  buffer[0]=I2C_WIND_ADDRESS_ADDRESS;
  buffer[1]=windAddress;
  buffer[I2C_WIND_ADDRESS_LENGTH+1]=crc8(buffer, I2C_WIND_ADDRESS_LENGTH+1);
  Wire.write(buffer,I2C_WIND_ADDRESS_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_wind_address(eventMask e, prompt &item) {
  last_status=do_i2c_wind_address();
  nav.idleOn(display_status);
  return proceed;
}

bool do_i2c_wind_oneshot(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_WIND_DEFAULT_ADDRESS);
  buffer[0]=I2C_WIND_ONESHOT_ADDRESS;
  buffer[1]=(bool)(windOneshot);
  buffer[I2C_WIND_ONESHOT_LENGTH+1]=crc8(buffer, I2C_WIND_ONESHOT_LENGTH+1);
  Wire.write(buffer,I2C_WIND_ONESHOT_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_wind_oneshot(eventMask e, prompt &item) {
  last_status=do_i2c_wind_oneshot();
  nav.idleOn(display_status);
  return proceed;
}

void do_windsonic_sconfigurator(void){
  Serial1.begin(GWS_SERIAL_BAUD);
  Serial1.setTimeout(500);
  Serial.setTimeout(500);
  Serial.println("Windsonic sconfiguration");
  pinMode(WIND_POWER_PIN, OUTPUT);
  windsonicPowerOff();
  delay(1000);
  windsonicPowerOn();
  delay(WIND_POWER_ON_DELAY_MS);
  Serial.println("windsonic ON");
  
  windsonicSerialReset();
  windsonicSconfigure();
  
  while(true){
    delay(1000);
    windsonicFlush();
    Serial1.print("?Q!\r\n");
    windsonicReceiveMessage('\r');
    Serial.println(uart_rx_buffer);
  }
}
result windsonic_sconfigurator(eventMask e, prompt &item) {
  nav.idleOn(display_nostatus);
  do_windsonic_sconfigurator();
  nav.idleOff();
  return proceed;
}

void do_windsonic_configurator(void){	
  
  Serial1.begin(GWS_SERIAL_BAUD);
  Serial1.setTimeout(500);
  Serial.setTimeout(500);
  Serial.println("Windsonic configuration");
  pinMode(WIND_POWER_PIN, OUTPUT);
  windsonicPowerOff();
  delay(1000);
  windsonicPowerOn();
  delay(WIND_POWER_ON_DELAY_MS);
  Serial.println("windsonic ON");
  windsonicSerialReset();
  
  windsonicConfigure();
  
  while(true){
    delay(1000);
    windsonicFlush();
    Serial1.print("?Q!\n");
    windsonicReceiveMessage('\n');
    Serial.println(uart_rx_buffer);
  }
}
result windsonic_configurator(eventMask e, prompt &item) {
  nav.idleOn(display_nostatus);
  do_windsonic_configurator();
  nav.idleOff();
  return proceed;
}

/*
//This is not defined in writable registers
bool do_i2c_wind_type(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_WIND_DEFAULT_ADDRESS);
  buffer[0]=I2C_WIND_SENSOR_TYPE_ADDRESS;
  buffer[1]=(uint8_t)windType;
  buffer[I2C_WIND_SENSOR_TYPE_LENGTH+1]=crc8(buffer, I2C_WIND_SENSOR_TYPE_LENGTH+1);
  Wire.write(buffer,I2C_WIND_SENSOR_TYPE_LENGTH+2);
  return (Wire.endTransmission() == 0);
}
result i2c_wind_type(eventMask e, prompt &item) {
  last_status=do_i2c_wind_type();
  nav.idleOn(display_status);
  return proceed;
}
*/

bool do_i2c_wind_save(void){
  uint8_t buffer[32];
  Wire.beginTransmission(I2C_WIND_DEFAULT_ADDRESS);
  buffer[0]=I2C_COMMAND_ID;
  buffer[1]=I2C_WIND_COMMAND_SAVE;
  buffer[2]=crc8(buffer, 2);
  Wire.write(buffer,3);
  return (Wire.endTransmission() == 0);
}
/*
result i2c_wind_save(eventMask e, prompt &item) {
  last_status=do_i2c_wind_save();
  nav.idleOn(display_status);
  return proceed;
}
*/
result i2c_wind_save_all(eventMask e, prompt &item) {
  last_status                =do_i2c_wind_address();
  if(last_status) last_status=do_i2c_wind_oneshot();
  if(last_status) last_status=do_i2c_wind_save();
  //do_i2c_wind_type();
  nav.idleOn(display_status);
  return proceed;
}

/*
void logPrefix(Print* _logOutput) {
  char m[12];
  sprintf(m, "%10lu ", millis());
  _logOutput->print("#");
  _logOutput->print(m);
  _logOutput->print(": ");
}
*/

void logSuffix(Print* _logOutput) {
  _logOutput->print('\n');
  //_logOutput->flush();  // we use this to flush every log message
}


void setup() {

  Serial.begin(115200);        // connect to the serial port
  //Serial.setTimeout(60000);
  Log.begin(LOG_LEVEL, &Serial);
  //Log.setPrefix(logPrefix);
  Log.setSuffix(logSuffix);

  Serial.print(F("Start sensor config"));

  //Start I2C communication routines
  Wire.begin();

  //The Wire library enables the internal pullup resistors for SDA and SCL.
  //You can turn them off after Wire.begin()
  // do not need this with patched Wire library
  //digitalWrite( SDA, LOW);
  //digitalWrite( SCL, LOW);
  pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
  pinMode(SCL, INPUT_PULLUP);
  pinMode(encBtn,INPUT_PULLUP);
  encoder.begin();
  encButton.begin();
  lcd.begin(20,4);
  ir.begin();

  Serial.print("Initializing SD card...");
  if (!sd.begin(SDCARD_SS)) {
    Serial.println(F("Error SD card"));
    delay(100000);
  }
  filePickMenu.begin();//need this after sd begin

  delay(1000);
  
  // encoder with interrupt on the A & B pins
  attachInterrupt(digitalPinToInterrupt(encA), encoderprocess, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB), encoderprocess, CHANGE);
  
  nav.idleTask=idle;//point a function to be used when menu is suspended
  //mainMenu[1].enabled=disabledStatus;
  nav.showTitle=true;
  //nav.timeOut=10;
  
  lcd.setCursor(0, 0);
  lcd.print("Sensor Configurator ");
  lcd.print(version);

  lcd.setCursor(0, 1);
  lcd.print("http://rmap.cc");

  Serial.println("setup done.");Serial.flush();
  Serial.print("Sensor Configurator ");
  Serial.println(version);
  Serial.println("http://rmap.cc");
  delay(2000);

  Serial.println(version);
  Serial.println("Use keys + - * /");
  Serial.println("or arrows keys as alternative");
  Serial.println("to control the menu navigation");
  Serial.println("you can select line by number too");

  Serial.println("edit text value operation:");
  Serial.println("1 - first * -> enter field navigation use +/- to select character position");
  Serial.println("2 - second * -> enter character edit use +/- to select character value");
  Serial.println("3 - third * -> return to field navigation (1)");
  Serial.println("4 - fourth * without changing position -> exit edit mode");
  Serial.println("You can also use keyboard to enter characters, enter and escape keys recognized.");
}

void loop(){
  if (nav.sleepTask and true_idle_status) loop_serial();
  loop_menu();
}

void loop_menu() {
  // if we do not use interrupt we can poll A & B pins but we need non blocking firmware
  //encoder.process();  // update encoder status
  ir.process();
  nav.poll();
}

void loop_serial() {

  displayHelp();

  char command = getCommand();
  switch (command)
    {

    case 'i':
      {
	byte start_address = 1;
	byte end_address = 127;
	
	Serial.println("\nI2CScanner ready!");
	Serial.print("starting scanning of I2C bus from ");
	Serial.print(start_address,HEX);
	Serial.print(" to ");
	Serial.print(end_address,HEX);
	Serial.println("...Hex");
	
	// start the scan, will call "scanFunc()" on result from each address
	scanI2CBus( start_address, end_address, scanFunc );
	
	Serial.println("\ndone");

	break;
      }

    case 's':
      {
	
	int new_address;
	int oneshot;
	
	new_address= -1;
	while (new_address < 1 || new_address > 127){
	  Serial.print(F("digit new i2c address for i2c-radiation (1-127) default: "));
	  Serial.println(I2C_SOLAR_RADIATION_DEFAULT_ADDRESS);
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	delay(1000);
	radiationAddress=new_address;
	if(!do_i2c_solar_radiation_address()) Serial.println(F("Wire Error"));             // End Write Transmission
	delay(1000);
	
	oneshot=-1;
	while (oneshot < 0 || oneshot > 1){
	  Serial.println(F("digit 1 for oneshotmode; 0 for continous mode for i2c-radiation (0/1) (default 0)"));
	  oneshot=Serial.parseInt();
	  Serial.println(oneshot);
	}
	delay(1000);
	radiationOneshot=oneshot;
	if (!do_i2c_solar_radiation_oneshot()) Serial.println(F("Wire Error"));             // End Write Transmission

	Serial.println(F("set AIN1 registers only !!!"));
	float new_value= -1;
	while (new_value < 0. || new_value > 32767){
	  Serial.print(F("digit new value for ADC calibration offset(0/32767) (default 0): "));
	  new_value=Serial.parseFloat();
	  Serial.println(new_value,5);
	}
	delay(1000);
	radiationOffset=new_value;
	if (!do_i2c_solar_radiation_offset()) Serial.println(F("Wire Error"));             // End Write Transmission
	delay(1000);

	new_value= -32768;
	while (new_value < -32767. || new_value > 32767.){
	  Serial.print(F("digit new value for ADC calibration gain(-32767./32767.) (default 1.0): "));
	  new_value=Serial.parseFloat();
	  Serial.println(new_value,5);
	}
	delay(1000);
	radiationGain=new_value;
	if (!do_i2c_solar_radiation_gain()) Serial.println(F("Wire Error"));             // End Write Transmission
	delay(1000);

	new_value= -1;
	while (new_value < 1. || new_value > 10000.){
	  Serial.print(F("digit new value for sensor voltage max(1./10000.) (default 5000.): "));
	  new_value=Serial.parseFloat();
	  Serial.println(new_value);
	}
	delay(1000);
	radiationSensorVoltage=new_value;
	if (!do_i2c_solar_radiation_sensor_voltage()) Serial.println(F("Wire Error"));             // End Write Transmission
	delay(1000);

	new_value= -1;
	while (new_value < 1. || new_value > 3000.){
	  Serial.print(F("digit new value for sensor radiation max(1./3000.) (default 2000.): "));
	  new_value=Serial.parseFloat();
	  Serial.println(new_value);
	}
	delay(1000);
	radiationSensorRadiation=new_value;
	if (!do_i2c_solar_radiation_sensor_radiation()) Serial.println(F("Wire Error"));             // End Write Transmission
	delay(1000);

	Serial.println("save configuration");
	if (!do_i2c_solar_radiation_save())  Serial.println(F("Wire Error"));             // End Write Transmission
	
	Serial.println(F("Done; switch off"));
	delay(10000);

	break;
      }

    case 't':
      {
	int new_address;
	int oneshot;
	
	new_address=-1;	
	while (new_address < 1 || new_address > 127){
	  Serial.print(F("digit new i2c address for i2c-th (1-127) default: "));
	  Serial.println(I2C_TH_DEFAULT_ADDRESS);
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	delay(1000);
	thAddress=new_address;
	if (!do_i2c_th_address()) Serial.println(F("Wire Error"));             // End Write Transmission
	delay(1000);

	oneshot=-1;
	while (oneshot < 0 || oneshot > 1){
	  Serial.println(F("digit 1 for oneshotmode; 0 for continous mode for i2c-th  (0/1) (default 0)"));
	  oneshot=Serial.parseInt();
	  Serial.println(oneshot);
	}
	delay(1000);
	thOneshot=oneshot;
	if (!do_i2c_th_oneshot()) Serial.println(F("Wire Error"));             // End Write Transmission
	delay(1000);
	
	String new_type;	
	Serial.println(F("digit new i2c sensor1 TYPE i2c-th (3 char uppercase)"));
	while (new_type.length() != 4) {
	  new_type = Serial.readStringUntil('\n');
	}
	Serial.println(new_type);
	delay(1000);
	new_type.toCharArray(thSensorType1, 4);
	if (!do_i2c_th_sensor_type1()) Serial.println(F("Wire Error"));             // End Write Transmission
	delay(1000);
	
	new_address=-1;
	while (new_address < 1 || new_address > 127){
	  Serial.println(F("digit new i2c sensor1 address for i2c-th (1-127)"));
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	delay(1000);
	thSensorAddress1=new_address;
	if (!do_i2c_th_sensor_address1()) Serial.println(F("Wire Error"));             // End Write Transmission
	delay(1000);

	Serial.println(F("digit new i2c sensor2 TYPE i2c-th (3 char uppercase)"));
	while (new_type.length() != 4) {
	  new_type = Serial.readStringUntil('\n');
	}
	Serial.println(new_type);
	delay(1000);
	new_type.toCharArray(thSensorType2, 4);
	if (!do_i2c_th_sensor_type2()) Serial.println(F("Wire Error"));             // End Write Transmission
	delay(1000);
	
	new_address=-1;
	while (new_address < 1 || new_address > 127){
	  Serial.println(F("digit new i2c sensor2 address for i2c-th (1-127)"));
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	delay(1000);
	thSensorAddress2=new_address;
	if (!do_i2c_th_sensor_address2()) Serial.println(F("Wire Error"));             // End Write Transmission
	delay(1000);

	Serial.println("save configuration");
	if (!do_i2c_th_save())  Serial.println(F("Wire Error"));             // End Write Transmission
	
	Serial.println(F("Done; switch off"));
	delay(10000);

	break;
      }
    case 'r':
      {
	int new_address=-1;
	int oneshot;
	uint16_t tipping_bucket_time_ms=0;
	uint8_t rain_for_tip=0;
	
	while (new_address < 1 || new_address > 127){
	  Serial.print(F("digit new i2c address for i2c-rain (1-127) default: "));
	  Serial.println(I2C_RAIN_DEFAULT_ADDRESS);
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	delay(1000);
	rainAddress=new_address;
	if (!do_i2c_rain_address()) Serial.println(F("Wire Error"));             // End Write Transmission	
	delay(1000);
	
	oneshot=-1;
	while (oneshot < 0 || oneshot > 1){
	  Serial.println(F("digit 1 for oneshotmode; 0 for continous mode for i2c-rain  (0/1) (default 1) (0 is not supported for now)"));
	  oneshot=Serial.parseInt();
	  Serial.println(oneshot);
	}
	delay(1000);
	rainOneshot=oneshot;
	if (!do_i2c_rain_oneshot()) Serial.println(F("Wire Error"));             // End Write Transmission

	
	while (tipping_bucket_time_ms < 2 || tipping_bucket_time_ms > 1000){
	  Serial.println(F("Tipping bucket time in milliseconds for i2c-rain (2-1000)"));
	  tipping_bucket_time_ms=Serial.parseInt();
	  Serial.println(tipping_bucket_time_ms);
	}	
	delay(1000);
	rainTippingBucketTime=tipping_bucket_time_ms;
	if (!do_i2c_rain_tipping_bucket_time()) Serial.println(F("Wire Error"));             // End Write Transmission
	
	while (rain_for_tip < 1 || rain_for_tip > 20){
	  Serial.println(F("Rain for tip for i2c-rain (1-20) Hg/m^2 or Kg/m^2/10"));
	  rain_for_tip=Serial.parseInt();
	  Serial.println(rain_for_tip);
	}
	delay(1000);
	rainRainForTip=rain_for_tip;
	if (!do_i2c_rain_rain_for_tip()) Serial.println(F("Wire Error"));             // End Write Transmission	
	delay(1000);
	
	Serial.println("save configuration");
	if (!do_i2c_rain_save())  Serial.println(F("Wire Error"));             // End Write Transmission
	
	Serial.println(F("Done; switch off"));
	delay(10000);
	
	break;
      }

    case 'p':
      {
	
	int new_address;
	int oneshot;
	
	new_address= -1;
	while (new_address < 1 || new_address > 127){
	  Serial.print(F("digit new i2c address for i2c-power (1-127) default: "));
	  Serial.println(I2C_POWER_DEFAULT_ADDRESS);
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	delay(1000);
	powerAddress=new_address;
	if (!do_i2c_power_address()) Serial.println(F("Wire Error"));             // End Write Transmission
	delay(1000);
	
	oneshot=-1;
	while (oneshot < 0 || oneshot > 1){
	  Serial.println(F("digit 1 for oneshotmode; 0 for continous mode for i2c-power (0/1) (default 0)"));
	  oneshot=Serial.parseInt();
	  Serial.println(oneshot);
	}
	delay(1000);
	powerOneshot=oneshot;
	if (!do_i2c_power_oneshot()) Serial.println(F("Wire Error"));             // End Write Transmission

	uint16_t new_value= 0;
	while (new_value < 1 || new_value > 32767){
	  Serial.print(F("digit new value for max voltage input for panel  for i2c-power module (millivolt) (0/32767) (default 30000): "));
	  new_value=Serial.parseInt();
	  Serial.println(new_value);
	}
	delay(1000);
	powerVoltageMaxPanel=new_value;
	if (!do_i2c_power_voltage_max_panel()) Serial.println(F("Wire Error"));             // End Write Transmission
	delay(1000);

	new_value= 0;
	while (new_value < 1 || new_value > 32767.){
	  Serial.print(F("digit new value for max voltage input for battery for i2c-power module (millivolt) (0/32767) (default 15000): "));
	  new_value=Serial.parseInt();
	  Serial.println(new_value);
	}
	delay(1000);
	powerVoltageMaxBattery=new_value;
	if (!do_i2c_power_voltage_max_battery()) Serial.println(F("Wire Error"));             // End Write Transmission
	delay(1000);

	Serial.println("save configuration");
	if (!do_i2c_power_save())  Serial.println(F("Wire Error"));             // End Write Transmission
	
	Serial.println(F("Done; switch off"));
	delay(10000);

	break;
      }
      
    case 'w':
      {
	
	int new_address;
	int oneshot;
	
	new_address= -1;
	while (new_address < 1 || new_address > 127){
	  Serial.print(F("digit new i2c address for i2c-wind (1-127) default: "));
	  Serial.println(I2C_WIND_DEFAULT_ADDRESS);
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	delay(1000);
	windAddress=new_address;
	if (!do_i2c_wind_address()) Serial.println(F("Wire Error"));             // End Write Transmission
	delay(1000);

	oneshot=-1;
	while (oneshot < 0 || oneshot > 1){
	  Serial.println(F("digit 1 for oneshotmode; 0 for continous mode for i2c-wind  (0/1) (default 0)"));
	  oneshot=Serial.parseInt();
	  Serial.println(oneshot);
	}
	delay(1000);
	windOneshot=oneshot;
	if (!do_i2c_wind_oneshot()) Serial.println(F("Wire Error"));             // End Write Transmission
	delay(1000);

	/*
	int sensortype;
	sensortype=-1;
	while (sensortype < 1 || sensortype > 2){
	  Serial.println(F("digit sensortype code for i2c-wind (1 Davis, 2 Inspeed)"));
	  sensortype=Serial.parseInt();
	  Serial.println(sensortype);
	}
	delay(1000);
	windType=sensortype;
	if (!do_i2c_wind_type()) Serial.println(F("Wire Error"));             // End Write 
	delay(1000);
	*/
	
	Serial.println("save configuration");
	if (!do_i2c_wind_save())  Serial.println(F("Wire Error"));             // End Write Transmission
	
	Serial.println(F("Done; switch off"));
	delay(10000);

	break;
      }

    case 'u':
      {
	do_windsonic_sconfigurator();	
	break;
      }

    case 'v':
      {
	do_windsonic_sconfigurator();		  
	break;
      }
      
    case 'z':
      {

	long int baudrate = -1;
	while (baudrate < 2400 || baudrate > 38400){
	  Serial.print(F("digit new i2c serial port Baud rate (2400-38400) default: "));
	  Serial.println(GWS_SERIAL_BAUD);
	  baudrate=Serial.parseInt();
	  Serial.println(baudrate);
	}
	
	Serial1.begin(baudrate);
	Serial1.setTimeout(500);
	Serial.setTimeout(500);
	Serial.println("transparent mode with windsonic at:");
	Serial.println(baudrate);
	
	pinMode(WIND_POWER_PIN, OUTPUT);
	windsonicPowerOff();
	delay(1000);
	windsonicPowerOn();
	//delay(WIND_POWER_ON_DELAY_MS);
	Serial.println("windsonic ON");
 	
	while (true){
	  while (Serial1.available() > 0){
	    Serial.write((uint8_t)Serial1.read());
	  }
	  Serial.flush();
	  while (Serial.available() > 0){
	    Serial1.write((uint8_t)Serial.read());
	  }
	  Serial1.flush();
	}

	break;
      }

    case '#':
      Serial.println(F("\tgo to LCD display"));
      nav.idleOff();
      break;
      
    case '?':
      break;

    case '\0':
      break;

    default:
      Serial.println(F("\tinvalid"));
      break;
    }
}
