
#define SENSORS_LEN 1
#define LENVALUES 2
#define MR_PWM   D3
#define ML_PWM   0XFF
#define MR_EN    D4
#define ML_EN    0XFF
#define MR_IS    0XFF
#define ML_IS    0XFF
#define SCL D1
#define SDA D2
// rotary encoder pins
#define encBtn  D5
#define encA    D6
#define encB    D7


// logging level at compile time
// Available levels are:
// LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
#define LOG_LEVEL   LOG_LEVEL_NOTICE

//disable debug at compile time but call function anyway
//#define DISABLE_LOGGING disable

#define FILETERMOSTATO "/termostato.json"

#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ArduinoLog.h>
#include <Wire.h>
#include <SensorDriverb.h>
#include <PID_v1.h>
#include <ibt_2.h>
#include <U8g2lib.h>
#include <menu.h>
#include <menuIO/u8g2Out.h>
#include <menuIO/RotaryIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>

#define fontName u8g2_font_tom_thumb_4x6_tf
#define fontX 5
#define fontY 8
#define offsetX 1
#define offsetY 1
#define U8_Width 64
#define U8_Height 48
#define fontMarginX 1
#define fontMarginY 1

struct sensor_t
{
  char driver[5];         // driver name
  char type[5];         // driver name
  int address;            // i2c address
} sensors[SENSORS_LEN];
SensorDriver* sd[SENSORS_LEN];

char* json;
//aJsonObject* aj;

//Define Variables we'll be connecting to
double Setpoint, Input, Output;
//Specify the links and initial tuning parameters

double gain=10./127.;
double ct=60.;
double tau=30;


double Kp=1.2*ct/(gain*tau);
double Ki=Kp/(2.*tau);
double Kd=(0.5*tau)/Kp;

PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

ibt_2 hbridge(IBT_2_2HALF,MR_PWM,ML_PWM,MR_EN ,ML_EN ,MR_IS ,ML_IS);

U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0);


// define menu colors --------------------------------------------------------
//each color is in the format:
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

int ventCtrl=HIGH;

TOGGLE(ventCtrl,setVent,"Vent: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("On",HIGH,doNothing,noEvent)
  ,VALUE("Off",LOW,doNothing,noEvent)
);


float temperature=20.;

MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
  ,FIELD(temperature,"Tem","C",0,100,10,1,doNothing,noEvent,wrapStyle)
  ,SUBMENU(setVent)
  ,EXIT("<Exit")
);

#define MAX_DEPTH 2

encoderIn<encA,encB> encoder;//simple quad encoder driver
encoderInStream<encA,encB> encStream(encoder);// simple encoder Stream


//a keyboard with only one key as the encoder button
keyMap encBtn_map[]={{-encBtn,defaultNavCodes[enterCmd].ch}};//negative pin numbers use internal pull-up, this is on when low
keyIn<1> encButton(encBtn_map);//1 is the number of keys

serialIn serial(Serial);

MENU_INPUTS(in,&encStream,&encButton,&serial);

idx_t gfx_tops[MAX_DEPTH];

PANELS(gfxPanels,{0,0,U8_Width/fontX,U8_Height/fontY});
u8g2Out oledOut(u8g2,colors,gfx_tops,gfxPanels,fontX,fontY,offsetX,offsetY,fontMarginX,fontMarginY);
idx_t serialTops[MAX_DEPTH]={0};
serialOut outSerial(*(Print*)&Serial,serialTops);

//define outputs controller
menuOut* outputs[]{&oledOut,&outSerial};//list of output devices
outputsList out(outputs,sizeof(outputs)/sizeof(menuOut*));//outputs list controller


NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

bool displaydata=false;


//when menu is suspended
result idle(menuOut& o,idleEvent e) {
  o.clear();
  switch(e) {
  case idleStart:
    o.println("suspending menu!");
    {
      StaticJsonBuffer<500> jsonBuffer;
      JsonObject& json = jsonBuffer.createObject();
    
      LOGN(F("temperature %D" CR),temperature);
      json["temperature"] = temperature;
  
      File configFile = SPIFFS.open(FILETERMOSTATO, "w");
      if (!configFile) {
	LOGN(F("failed to open config file for writing" CR));
      }else{
	//json.printTo(Serial);
	json.printTo(configFile);
	configFile.close();
	LOGN(F("saved parameter" CR));
      }
    }
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
  if (SPIFFS.exists(FILETERMOSTATO)) {
    //file exists, reading and loading
    LOGN(F("reading config file" CR));
    File configFile = SPIFFS.open(FILETERMOSTATO, "r");
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
  LOGN(F("PID: %D %D %D" CR),Kp,Ki,Kd);


 // start up the i2c interface
  Wire.begin(SDA,SCL);

  delay(1000);
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
  
  //read configuration from FS json
  LOGN(F("mounting FS..." CR));
  if (!SPIFFS.begin()) {
    LOGN(F("failed to mount FS" CR));
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
	temperature=json["temperature"];
	LOGN(F("temperature %D" CR),temperature);
    }
  }
  
  hbridge.start(IBT_2_R_HALF);
  hbridge.setpwm(255,IBT_2_L_HALF);
  
  //turn the PID on
  myPID.SetMode(AUTOMATIC);

  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"HIH");
  sensors[0].address=39;
  // 39	0x27
  // 73	0x49
  
 
  encoder.begin();
  encButton.begin();
  

  // encoder with interrupt on the A & B pins
  attachInterrupt(digitalPinToInterrupt(encA), encoderprocess, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB), encoderprocess, CHANGE);
  
  for (int i = 0; i < SENSORS_LEN; i++) {
    
    sd[i]=SensorDriver::create(sensors[i].driver,sensors[i].type);
    if (sd[i] == 0){
      LOGN(F("%s: driver not created !" CR),sensors[i].driver);
    }else{
      sd[i]->setup(sensors[i].driver,sensors[i].address);
    }
  }

  nav.idleTask=idle;//point a function to be used when menu is suspended
  
  LOGN(F("setup done." CR));

  Serial.println("Menu 4.x");
  Serial.println("Use keys + - * /");
  Serial.println("to control the menu navigation");
  
}

void loop()
{
  long unsigned int waittime,maxwaittime=0;
  Setpoint = temperature;

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

  //wait sensors to go ready
  //Serial.print("# wait sensors for ms:");  Serial.println(maxwaittime);
  delay(maxwaittime);  // 500 for tmp and 250 for adt and 2500 for davis

  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == 0){
      // get integers values 
      long values[LENVALUES];
      size_t lenvalues=LENVALUES;

      if (sd[i]->get(values,lenvalues) == SD_SUCCESS){
	//for (size_t ii = 0; ii < lenvalues; ii++) {
	//  Serial.println(values[ii]);
	//}

	Input = (float(values[1])/100.)-273.15;
      
	myPID.Compute();
	hbridge.setpwm(int(Output),IBT_2_R_HALF);

	LOGV(F("Setpoint: %D  Temperatura: %D PID output: %D" CR),Setpoint, Input, Output);

	if (displaydata){
	  //u8g2.setFont(fontName);
	  //u8g2.setFontMode(0); // enable transparent mode, which is faster
	  u8g2.clearBuffer();
	  u8g2.setCursor(0, 10); 
	  u8g2.print(Input);
	  u8g2.setCursor(0, 20); 
	  u8g2.print(Setpoint);
	  u8g2.setCursor(0, 30); 
	  u8g2.print(Output);
	  u8g2.sendBuffer();
	}
	//delay(1000);
	//analogWriteFreq(1);
	
      }else{
	Serial.println("Error");
      }
    }
  }

  if (ventCtrl) {
    hbridge.start(IBT_2_L_HALF);
  }else{
    hbridge.stop(IBT_2_L_HALF);
  }
  
  nav.doInput();
  if (nav.changed(0)) {//only draw if menu changed for gfx device
    u8g2.firstPage();
    do nav.doOutput(); while(u8g2.nextPage());
  }  

}
