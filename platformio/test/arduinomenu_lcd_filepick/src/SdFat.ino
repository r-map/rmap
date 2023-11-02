#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>
#include <menu.h>
#include <menuIO/hd44780_I2CexpOut.h>
#include <menuIO/serialIO.h>
#include <plugin/SdFatMenu.h>
#include <menuIO/RotaryIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>

using namespace Menu;

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
#define encA 2
#define encB 3
//this encoder has a button here
#define encBtn 6

encoderIn<encA,encB> encoder;//simple encoder driver
encoderInStream<encA,encB> encStream(encoder);// simple quad encoder fake Stream

//a keyboard with only one key as the encoder button
keyMap encBtn_map[]={{-encBtn,defaultNavCodes[enterCmd].ch}};//negative pin numbers use internal pull-up, this is on when low
keyIn<1> encButton(encBtn_map);//1 is the number of keys


SDMenuT<CachedFSO<SdFat,32>> filePickMenu(sd,"SD Card","/",filePick,enterEvent);

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
      }
  //     break;
  // }
  return proceed;
}

//input from the encoder + encoder button + serial
serialIn serial(Serial);
menuIn* inputsList[]={&encStream,&encButton,&serial};
chainStream<3> in(inputsList);//3 is the number of inputs

//// ISR for encoder management
void encoderprocess (){
  encoder.process();
}

const panel panels[] MEMMODE={{0,0,20,4}};
navNode* nodes[sizeof(panels)/sizeof(panel)];
panelsList pList(panels,nodes,1);

#define MAX_DEPTH 2

MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
  ,SUBMENU(filePickMenu)
  ,OP("Something else...",doNothing,noEvent)
  ,EXIT("<Back")
);

//define output device serial
idx_t serialTops[MAX_DEPTH];
serialOut outSerial(Serial,serialTops);

idx_t lcdTops[MAX_DEPTH];
liquidCrystalOut outLcd(lcd,lcdTops,pList);//output device for LCD

//define outputs controller
menuOut* const outputs[] MEMMODE={&outSerial,&outLcd};//list of output devices
outputsList out(outputs,sizeof(outputs)/sizeof(menuOut*));//outputs list controller

//MENU_OUTPUTS(out, MAX_DEPTH
//  ,LIQUIDCRYSTAL_OUT(lcd,{0,0,20,4})
//  ,SERIAL_OUT(Serial)
//);

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);//the navigation root object

result idle(menuOut& o,idleEvent e) {
  switch(e) {
    case idleStart:o.print("suspending menu!");break;
    case idling:o.print("suspended...");break;
    case idleEnd:o.print("resuming menu.");break;
  }
  return proceed;
}

void setup() {

  pinMode(encBtn,INPUT_PULLUP);
  Wire.begin();
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Arduino Menu Library");Serial.flush();
  encoder.begin();
  encButton.begin();
  lcd.begin(20,4);
  Serial.print("Initializing SD card...");
  if (!sd.begin(SDCARD_SS)) {
    Serial.println(F("Error SD card"));
    delay(100000);
  }
  filePickMenu.begin();//need this after sd begin

    // encoder with interrupt on the A & B pins
  attachInterrupt(digitalPinToInterrupt(encA), encoderprocess, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB), encoderprocess, CHANGE);
  
  nav.idleTask=idle;//point a function to be used when menu is suspended
  //nav.showTitle=false; // do not use with filepick
  nav.useAccel=false;
  lcd.setCursor(0, 0);
  lcd.print("Menu 4.x LCD");
  lcd.setCursor(0, 1);
  lcd.print("rmap.cc");
  delay(2000);
  Serial.println("initialization done.");

}

constexpr int menuFPS=25;
unsigned long nextPool=0;
void loop() {
  unsigned long now=millis();
  if(now>=nextPool) {
    nav.poll();
    nextPool=now+1000/menuFPS;
  }
}
