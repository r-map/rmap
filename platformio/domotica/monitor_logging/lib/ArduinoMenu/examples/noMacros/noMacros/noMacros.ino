#include <Arduino.h>

#include <menu.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>

using namespace Menu;

#ifndef DEBUG
Print& operator<<(Print&o, Menu::prompt&p) {
  print_P(o,p.getText());
  return o;
}
#endif

//define "Op 1"
void op1Func();
constMEM char op1Text[] MEMMODE="Op 1";
constMEM promptShadowRaw op1InfoRaw MEMMODE={(callback)op1Func,_noStyle,op1Text,enterEvent,noStyle};
constMEM promptShadow& op1Info=*(promptShadow*)&op1InfoRaw;
//or just this line on non AVR devices like teensy or esp8266 instead of the above three
//promptShadow op1Info("Op 1",(callback)op1Func,enterEvent);
prompt op1(op1Info);

//define "Op 2"
void op2Func();
constMEM char op2Text[] MEMMODE="Op 2";
constMEM promptShadowRaw op2InfoRaw MEMMODE={(callback)op2Func,_noStyle,op2Text,enterEvent,noStyle};
constMEM promptShadow& op2Info=*(promptShadow*)&op2InfoRaw;
//or just this line on non MEMMODE devices like teensy or esp8266 instead of the above three
//promptShadow op2Info("Op 2",(callback)op2Func,enterEvent);
prompt op2(op2Info);

//define the menu
prompt* constMEM menuData[] MEMMODE={&op1,&op2};
//or just prompt* menuData[]={&op1,&op2}; on non avr devices
constMEM char menuTitle[] MEMMODE="Main menu";
constMEM menuNodeShadowRaw menuInfoRaw MEMMODE={
  (callback)doNothing,
  (systemStyles)(_menuData|_canNav),
  menuTitle,
  noEvent,
  wrapStyle,
  sizeof(menuData)/sizeof(prompt*),
  menuData
};
constMEM menuNodeShadow& menuInfo=*(menuNodeShadow*)&menuInfoRaw;
//or just this line on non AVR devices like teensy or esp8266 instead of the above three
//menuNodeShadow menuInfo("Main menu",2,menuData,(callback)doNothing,noEvent,wrapStyle);
menuNode mainMenu(menuInfo);

#define MAX_DEPTH 1

//define input device
serialIn serial(Serial);

//define output device
idx_t serialTops[MAX_DEPTH]={0};
serialOut outSerial(Serial,serialTops);

//define outputs controller
menuOut* constMEM outputs[] MEMMODE={&outSerial};//list of output devices
outputsList out(outputs,1);//outputs list controller

//define navigation root and aux objects
navNode nav_cursors[MAX_DEPTH];//aux objects to control each level of navigation
navRoot nav(mainMenu, nav_cursors, MAX_DEPTH, serial, out);

//implement the menu actions
void op1Func() {Serial.println("Op 1 executed");}
void op2Func() {Serial.println("Op 2 executed");}

#include <Streaming.h>

/////////////////////////////////////////////////////////////////////////////
// arduino sketch
void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Menu initialized without macros.");
}

void loop() {
  nav.poll();
  delay(100);
}
