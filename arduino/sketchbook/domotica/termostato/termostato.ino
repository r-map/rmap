
#define SENSORS_LEN 1
#define LENVALUES 2
#define MR_PWM   D5
#define ML_PWM   D6
#define MR_EN    D3
#define ML_EN    D4
#define MR_IS    A0
#define ML_IS    A0
#define SCL D1
#define SDA D2
// rotary encoder pins
#define encBtn  D5
#define encA    D6
#define encB    D7

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

int ledCtrl=HIGH;

TOGGLE(ledCtrl,setLed,"Led: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("On",HIGH,doNothing,noEvent)
  ,VALUE("Off",LOW,doNothing,noEvent)
);


float temperature=20.;

MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
  ,FIELD(temperature,"Tem","C",0,100,10,1,doNothing,noEvent,wrapStyle)
  ,SUBMENU(setLed)
  ,EXIT("<Exit")
);

#define MAX_DEPTH 2

encoderIn<encA,encB> encoder;//simple quad encoder driver
encoderInStream<encA,encB> encStream(encoder);// simple encoder Stream


//a keyboard with only one key as the encoder button
keyMap encBtn_map[]={{-encBtn,defaultNavCodes[enterCmd].ch}};//negative pin numbers use internal pull-up, this is on when low
keyIn<1> encButton(encBtn_map);//1 is the number of keys

MENU_INPUTS(in,&encStream,&encButton);

idx_t gfx_tops[MAX_DEPTH];

PANELS(gfxPanels,{0,0,U8_Width/fontX,U8_Height/fontY});
u8g2Out oledOut(u8g2,colors,gfx_tops,gfxPanels,fontX,fontY,offsetX,offsetY,fontMarginX,fontMarginY);

//define outputs controller
menuOut* outputs[]{&oledOut};//list of output devices
outputsList out(outputs,sizeof(outputs)/sizeof(menuOut*));//outputs list controller


NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

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

// ISR for encoder management
void encoderprocess (){
  encoder.process();
}



void setup()
{
  hbridge.start(IBT_2_R_HALF);
  hbridge.setpwm(255,IBT_2_L_HALF);
  
  //turn the PID on
  myPID.SetMode(AUTOMATIC);

  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"HIH");
  sensors[0].address=39;
  // 39	0x27
  // 73	0x49
  
  // start up the serial interface
  Serial.begin(115200);
  Serial.println("PID started");
  Serial.println(Kp);
  Serial.println(Ki);
  Serial.println(Kd);
 
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

  encoder.begin();
  encButton.begin();
  
  delay(1000);

  // encoder with interrupt on the A & B pins
  attachInterrupt(digitalPinToInterrupt(encA), encoderprocess, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB), encoderprocess, CHANGE);
  
  for (int i = 0; i < SENSORS_LEN; i++) {

    sd[i]=SensorDriver::create(sensors[i].driver,sensors[i].type);
    if (sd[i] == 0){
      Serial.print(sensors[i].driver);
      Serial.println(": driver not created !");
    }else{
      sd[i]->setup(sensors[i].driver,sensors[i].address);
    }
  }

  Serial.println("setup done.");

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
	Serial.print(sensors[i].driver);
        Serial.print(" : ");
        Serial.print(sensors[i].type);
	Serial.println(" : Prepare failed !");
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
	Serial.print("Setpoint: ");
	Serial.print(Setpoint); 
	Serial.print("  Temperatura: ");
	Serial.print(Input);
	Serial.print(" ");
      
	myPID.Compute();
	Serial.print("PID command: ");
	Serial.println(Output);
	hbridge.setpwm(int(Output),IBT_2_R_HALF);

      }else{
	Serial.println("Error");
      }
    }
  }

  if (ledCtrl) {
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
