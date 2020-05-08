#include <Arduino.h>

/********************
Arduino generic menu system
U8G2 menu example
U8G2: https://github.com/olikraus/u8g2

Maj. 2020 Paolo Patruno https://github.com/r-map/rmap/platformio/test
Oct. 2016 Stephen Denne https://github.com/datacute
Based on example from Rui Azevedo - ruihfazevedo(@rrob@)gmail.com
Original from: https://github.com/christophepersoz

menu on U8G2 device
input:  Serial + encoder + IRremote + serial
output: wemos OLED Shield (SSD1306 I2C)/epaper (IL3820 SPI) + Serial
mcu:
esp8266 wemos D1 mini
STM32 nucleo_l432kc
arduino mega
microduino 1284p16m/644pa16m
*/


//#define I2CDISPLAY

#include <U8g2lib.h>
#include <menu.h>
#include <menuIO/u8g2Out.h>
#include <menuIO/RotaryIn.h>
#include <menuIO/IRremoteIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>

// rotary encoder pins
#define encBtn  6
#define encA    2
#define encB    3

#define IR_PIN 0

#if defined(I2CDISPLAY)
#include <Wire.h>

#define fontName u8g2_font_tom_thumb_4x6_tf
#define fontX 5
#define fontY 8
#define offsetX 1
#define offsetY 1
#define U8_Width 64
#define U8_Height 48
#define fontMarginX 1
#define fontMarginY 1

#if defined(ARDUINO_ARCH_STM32)
#define WIREX Wire1
TwoWire WIREX(PB4, PA7);
U8G2_SSD1306_64X48_ER_F_2ND_HW_I2C  u8g2(U8G2_R0);
#else
#define WIREX Wire
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0);
#endif
#else
#include <SPI.h>

//#define fontName u8g2_font_profont17_tf
//#define fontX 9
//#define fontY 17
#define fontName u8g2_font_10x20_tf
#define fontX 10
#define fontY 20
#define offsetX 0
#define offsetY 0
// problem here: do not set bigger U8_Width
#define U8_Width 200
#define U8_Height 128
#define fontMarginX 2
#define fontMarginY 1

U8G2_IL3820_V2_296X128_F_4W_HW_SPI  u8g2(U8G2_R0,8,9,10);
#endif

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
/*  reverse colors
const colorDef<uint8_t> colors[] ={
  {{1,1},{1,0,0}},//bgColor
  {{0,0},{0,1,1}},//fgColor
  {{0,0},{0,1,1}},//valColor
  {{0,0},{0,1,1}},//unitColor
  {{1,0},{1,1,0}},//cursorColor
  {{0,0},{0,1,1}},//titleColor
};
*/
result doAlert(eventMask e, prompt &item);

int test=55;
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
//MENU_INPUTS(in,&serial);
/*
MENU_OUTPUTS(out,MAX_DEPTH
,U8G2_OUT(u8g2,colors,gfx_tops,gfxPanels,fontX,fontY,offsetX,offsetY,fontMarginX,fontMarginY})
,SERIAL_OUT(Serial)
);
*/
/*
MENU_OUTPUTS(out,MAX_DEPTH
  ,U8G2_OUT(u8g2,colors,fontX,fontY,offsetX,offsetY,{0,0,U8_Width/fontX,U8_Height/fontY})
  ,SERIAL_OUT(Serial)
);
*/

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

//// ISR for encoder management
void encoderprocess (){
  encoder.process();
}

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println("menu 4.x test");Serial.flush();

#if defined(I2CDISPLAY)  
  WIREX.begin();
#else
  SPI.begin();
#endif

  delay(1000);
  #define OLEDI2CADDRESS 0X3C
  u8g2.setI2CAddress(OLEDI2CADDRESS*2);
  u8g2.begin();
  u8g2.setFont(fontName);
  u8g2.setFontMode(0); // enable transparent mode, which is faster
  u8g2.clearBuffer();
  u8g2.setCursor(0, 40); 
  u8g2.print(F("Starting up!"));
  u8g2.sendBuffer();

  encoder.begin();
  ir.begin();
  encButton.begin();
  
  delay(1000);

  // encoder with interrupt on the A & B pins
  attachInterrupt(digitalPinToInterrupt(encA), encoderprocess, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB), encoderprocess, CHANGE);
  
  // disable second option
  //mainMenu[1].enabled=disabledStatus;
  //nav.idleTask=idle;//point a function to be used when menu is suspended

  Serial.println("setup done.");Serial.flush();
}

void loop() {

  // if we do not use interrupt we can poll A & B pins but we need non blocking firmware
  //encoder.process();  // update encoder status
  ir.process();
  //nav.poll();

  nav.doInput();
  if (nav.changed(0)) {//only draw if menu changed for gfx device
    u8g2.firstPage();
    do nav.doOutput(); while(u8g2.nextPage());
  }
}
