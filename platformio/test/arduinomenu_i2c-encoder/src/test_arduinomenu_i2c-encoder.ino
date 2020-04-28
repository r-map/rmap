#include <Arduino.h>

/********************
menu on U8G2 device
input:  i2cencoder +i2cpollencoder
output: wemos OLED Shield (SSD1306 I2C)
mcu: esp8266 wemos D1 mini

require i2c-gpio-server encoder:
https://github.com/r-map/rmap/tree/master/arduino/sketchbook/domotica/i2c-gpio-server

*/
#include <U8g2lib.h>
#include <menu.h>
#include <menuIO/u8g2Out.h>
#include <menuIO/I2C_RotaryIn.h>
//#include <menuIO/I2C_RotaryPollIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <Wire.h>

// rotary encoder address for i2cpollencoder
#define i2caddress  I2C_MANAGER_DEFAULTADDRESS

#define fontName u8g2_font_tom_thumb_4x6_tf
#define fontX 5
#define fontY 8
#define offsetX 1
#define offsetY 1
#define U8_Width 64
#define U8_Height 48
#define fontMarginX 1
#define fontMarginY 1

// this is my i2c address required for multimaster use with i2cencoder (push mode)
#define I2C_ADDRESS 0x06

U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0);


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

result doAlert(eventMask e, prompt &item);

float test=20.;
int ledCtrl=HIGH;

result myLedOn() {
  ledCtrl=HIGH;
  return proceed;
}
result myLedOff() {
  ledCtrl=LOW;
  return proceed;
}

TOGGLE(ledCtrl,setLed,"Led: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("On",HIGH,doNothing,noEvent)
  ,VALUE("Off",LOW,doNothing,noEvent)
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

MENU(subMenu,"Sub-Menu",doNothing,noEvent,noStyle
  ,OP("Sub1",doNothing,noEvent)
  // ,altOP(altPrompt,"",doNothing,noEvent)
  ,EXIT("<Back")
);

uint16_t hrs=0;
uint16_t mins=0;

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
  ,FIELD(test,"Temp"," C",0.,40.,1.0,0.1,doNothing,noEvent,noStyle)
  ,SUBMENU(subMenu)
  ,SUBMENU(setLed)
  ,OP("LED On",myLedOn,enterEvent)
  ,OP("LED Off",myLedOff,enterEvent)
  ,SUBMENU(selMenu)
  ,SUBMENU(chooseMenu)
  ,OP("Alert test",doAlert,enterEvent)
  ,SUBMENU(tempo)
  ,EDIT("Hex",buf1,hexNr,doNothing,noEvent,noStyle)
  ,EXIT("<Exit")
);

#define MAX_DEPTH 2

//rotary encoder i2c push
i2cencoderIn i2cencoder;//simple quad encoder driver
i2cencoderInStream i2cencStream(i2cencoder);// simple encoder Stream

//rotary encoder i2c poll
//i2cpollencoderIn<i2caddress> i2cpollencoder;//simple quad encoder driver
//i2cpollencoderInStream<i2caddress> i2cpollencStream(i2cpollencoder);// simple encoder Stream

//MENU_INPUTS(in,&i2cencStream,&i2cpollencStream);
MENU_INPUTS(in,&i2cencStream);

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
    case idleStart:o.println("suspending menu!");break;
    case idling:o.println("suspended...");break;
    case idleEnd:o.println("resuming menu.");break;
  }
  return proceed;
}

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println("menu 4.x test");Serial.flush();

  //Start I2C communication routines
  //Wire.pins(SDA, SCL);
  Wire.begin(I2C_ADDRESS);
  //Wire.setClock(10);
  //Wire.setClockStretchLimit(1500);
  
  #define OLEDI2CADDRESS 0X3C
  u8g2.setI2CAddress(OLEDI2CADDRESS*2);
  u8g2.begin();
  u8g2.setFont(fontName);
  u8g2.setFontMode(0); // enable transparent mode, which is faster
  u8g2.clearBuffer();
  u8g2.setCursor(0, 10); 
  u8g2.print(F("Starting up!"));
  u8g2.sendBuffer();  
  delay(1000);

  i2cencoder.begin();
  //i2cpollencoder.begin();
  
  // disable second option
  //mainMenu[1].enabled=disabledStatus;
  nav.idleTask=idle;//point a function to be used when menu is suspended

  Serial.println("setup done.");Serial.flush();
}

void loop() {

  nav.doInput();
  if (nav.changed(0)) {//only draw if menu changed for gfx device
    u8g2.firstPage();
    do nav.doOutput(); while(u8g2.nextPage());
  }  
}
