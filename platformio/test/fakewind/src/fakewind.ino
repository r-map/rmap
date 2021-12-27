/**********************************************************************
Copyright (C) 2020  Paolo Paruno <p.patruno@iperbole.bologna.it>
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

/*********************************************************************
 *
 * This program implements fake temperature and humidity sensors 
 * exported to i2c interface.
 * 
**********************************************************************/
/*
buffer scrivibili da i2c
viene scritto buffer1 e buffer2
viene letto buffer2
i puntatori a buffer1 e buffer2 vengono scambiati in una operazione atomica all'inizio del main loop
*/

/*
buffer leggibili da i2c
le elaborazioni scrivono sempre su buffer1
viene sempre letto buffer2
i puntatori a buffer1 e buffer2 vengono scambiati in una operazione atomica al comando stop
*/

#define MAXWAITTIME 400

#define VERSION 01             //Software version for cross checking

#include "Wire.h"
#include "registers-wind_v2.h"      //Register definitions
#include "config.h"

#include "EEPROMAnything.h"#
#if defined(PLOT)
#include "Plotter.h"
#endif

#define REG_MAP_SIZE            sizeof(I2C_REGISTERS)                //size of register map
#define REG_WRITABLE_MAP_SIZE   sizeof(I2C_WRITABLE_REGISTERS)       //size of register map

#define MAX_SENT_BYTES     0x0F   //maximum amount of data that I could receive from a master device (register, plus 15 byte)

char confver[9] = CONFVER; // version of configuration saved on eeprom

typedef struct {
  uint8_t    sw_version;       // 0x00  Version of the I2C_GPS sw
} status_t;

typedef struct {
  uint16_t    dd;
  uint16_t    ff;
  uint16_t     u;
  uint16_t     v;
  uint16_t     meanu;
  uint16_t     meanv;
  uint16_t     peakgustu;
  uint16_t     peakgustv;
  uint16_t     longgustu;
  uint16_t     longgustv;
  uint16_t     meanff;
  uint16_t     sigma;
  uint16_t     sect[9];
} wind_t;


typedef struct {

//Status registers
  status_t     status;         // 0x00  status register

//wind data
  wind_t                wind;                     // 0x01 wind

} __attribute__((packed)) I2C_REGISTERS;    //https://gcc.gnu.org/onlinedocs/gcc-4.7.0/gcc/Variable-Attributes.html


typedef struct {

//sample mode
  bool                  oneshot;                  // one shot active
  uint8_t               i2c_address;              // i2c bus address (short unsigned int)
  void save (int* p) volatile {                            // save to eeprom

    IF_SDEBUG(Serial.print(F("oneshot: "))); IF_SDEBUG(Serial.println(oneshot));
    IF_SDEBUG(Serial.print(F("i2c address: "))); IF_SDEBUG(Serial.println(i2c_address));

    *p+=EEPROM_writeAnything(*p, oneshot);
    *p+=EEPROM_writeAnything(*p, i2c_address);
  }
  
  void load (int* p) volatile {                            // load from eeprom
    *p+=EEPROM_readAnything(*p, oneshot);
    *p+=EEPROM_readAnything(*p, i2c_address);
  }
} I2C_WRITABLE_REGISTERS;

volatile static I2C_REGISTERS    i2c_buffer1;
volatile static I2C_REGISTERS    i2c_buffer2;

volatile static I2C_REGISTERS*   i2c_dataset1;
volatile static I2C_REGISTERS*   i2c_dataset2;
volatile static I2C_REGISTERS*   i2c_datasettmp;

volatile static I2C_WRITABLE_REGISTERS  i2c_writablebuffer1;
volatile static I2C_WRITABLE_REGISTERS  i2c_writablebuffer2;

volatile static I2C_WRITABLE_REGISTERS* i2c_writabledataset1;
volatile static I2C_WRITABLE_REGISTERS* i2c_writabledataset2;
volatile static I2C_WRITABLE_REGISTERS* i2c_writabledatasettmp;

volatile static uint8_t         receivedCommands[MAX_SENT_BYTES];
volatile static uint8_t         new_command;                        //new command received (!=0)

// one shot management
bool start=false;
bool stop=false;

unsigned long starttime, regsettime, inittime;
boolean forcedefault=false;

#if defined(PLOT)
Plotter plot; // create plotter
#endif

// global required by plot lib
int16_t u;
int16_t v;

//////////////////////////////////////////////////////////////////////////////////////
// I2C handlers
// Handler for requesting data
//
void requestEvent()
{

  if ((millis()-regsettime) > 5000) {

    // missing
    //Wire.write(0xFF);
    //Wire.write(0xFF);
    //Wire.write(0xFF);
    //Wire.write(0xFF);

    // zero
    //Wire.write(0);
    //Wire.write(0);
    //Wire.write(0);
    //Wire.write(0);

    IF_SDEBUG(Serial.println("late"));

  }else{

    //IF_SDEBUG(Serial.println(*(((uint8_t *)i2c_dataset2)+receivedCommands[0]),HEX));
    //IF_SDEBUG(Serial.println(*(((uint8_t *)i2c_dataset2)+receivedCommands[0]+1),HEX));
    //IF_SDEBUG(Serial.println(*(((uint8_t *)i2c_dataset2)+receivedCommands[0]+2),HEX));
    //IF_SDEBUG(Serial.println(*(((uint8_t *)i2c_dataset2)+receivedCommands[0]+3),HEX));
    //IF_SDEBUG(Serial.println(i2c_dataset2->temperature.sample));
    
    Wire.write(((uint8_t *)i2c_dataset2)+receivedCommands[0],4);
    //Write up to 4 byte, since master is responsible for reading and sending NACK
    //32 byte limit is in the Wire library, we have to live with it unless writing our own wire library
  }
}

//Handler for receiving data
void receiveEvent( int bytesReceived)
{

  //IF_SDEBUG(Serial.print("receive event, bytes:"));
  //IF_SDEBUG(Serial.println(bytesReceived));

  uint8_t  *ptr1, *ptr2;
     //IF_SDEBUG(SSerial.print("received:"));
     for (int a = 0; a < bytesReceived; a++) {
          if (a < MAX_SENT_BYTES) {
               receivedCommands[a] = Wire.read();
	       //IF_SDEBUG(Serial.println(receivedCommands[a]));
          } else {
               Wire.read();  // if we receive more data then allowed just throw it away
          }
     }

     if (bytesReceived == 2){
       // check for a command
       if (receivedCommands[0] == I2C_WIND_COMMAND) {
	 //IF_SDEBUG(Serial.print("       received command:"));IF_SDEBUG(Serial.println(receivedCommands[1],HEX));
	 new_command = receivedCommands[1]; return; }
     }

     if (bytesReceived == 1){
       //read address for a given register
       //Addressing over the reg_map fallback to first byte
       if( (receivedCommands[0] < 0) || (receivedCommands[0] >= REG_MAP_SIZE)) {
	 receivedCommands[0]=0;
       }
       //IF_SDEBUG(Serial.print("set register:"));IF_SDEBUG(Serial.println(receivedCommands[0]));
       return;
     }

     //More than 1 byte was received, so there is definitely some data to write into a register
     //Check for writeable registers and discard data is it's not writeable

     //IF_SDEBUG(Serial.print("         check  writable buffer:"));
     //IF_SDEBUG(Serial.println(receivedCommands[0]));
     //IF_SDEBUG(Serial.println(I2C_WIND_MAP_WRITABLE));
     //IF_SDEBUG(Serial.println(I2C_WIND_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE));

     if ((receivedCommands[0]>=I2C_WIND_MAP_WRITABLE) && (receivedCommands[0] < (I2C_WIND_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE))) {    
       if ((receivedCommands[0]+(unsigned int)(bytesReceived-1)) <= (I2C_WIND_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE)) {
	 //Writeable registers
	 // the two buffer should be in sync
	 ptr1 = (uint8_t *)i2c_writabledataset1+receivedCommands[0]-I2C_WIND_MAP_WRITABLE;
	 ptr2 = (uint8_t *)i2c_writabledataset2+receivedCommands[0]-I2C_WIND_MAP_WRITABLE;
	 for (int a = 1; a < bytesReceived; a++) { 
	   //IF_SDEBUG(Serial.print("write in writable buffer:"));IF_SDEBUG(Serial.println(a));IF_SDEBUG(Serial.println(receivedCommands[a]));
	   *ptr1++ = receivedCommands[a];
	   *ptr2++ = receivedCommands[a];
	 }
	 // new data written
       }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////


void setup() {

  uint8_t i;

  IF_SDEBUG(Serial.begin(115200));        // connect to the serial port
  IF_SDEBUG(Serial.print(F("Start firmware version: ")));
  IF_SDEBUG(Serial.println(VERSION));

  regsettime=0;

  // inizialize double buffer
  i2c_dataset1=&i2c_buffer1;
  i2c_dataset2=&i2c_buffer2;

  // inizialize writable double buffer
  i2c_writabledataset1=&i2c_writablebuffer1;
  i2c_writabledataset2=&i2c_writablebuffer2;

  IF_SDEBUG(Serial.println(F("i2c_dataset 1&2 set to 1")));

  uint8_t *ptr;
  //Init to FF i2c_dataset1;
  ptr = (uint8_t *)i2c_dataset1;
  for (i=0;i<REG_MAP_SIZE;i++) { *ptr |= 0xFF; ptr++;}

  //Init to FF i2c_dataset1;
  ptr = (uint8_t *)i2c_dataset2;
  for (i=0;i<REG_MAP_SIZE;i++) { *ptr |= 0xFF; ptr++;}


  IF_SDEBUG(Serial.println(F("i2c_writabledataset 1&2 set to 1")));
  //Init to FF i2c_writabledataset1;
  ptr = (uint8_t *)i2c_writabledataset1;
  for (i=0;i<REG_WRITABLE_MAP_SIZE;i++) { *ptr |= 0xFF; ptr++;}

  //Init to FF i2c_writabledataset2;
  ptr = (uint8_t *)i2c_writabledataset2;
  for (i=0;i<REG_WRITABLE_MAP_SIZE;i++) { *ptr |= 0xFF; ptr++;}


  //Set up default parameters
  i2c_dataset1->status.sw_version          = VERSION;
  i2c_dataset2->status.sw_version          = VERSION;

  pinMode(FORCEDEFAULTPIN, INPUT_PULLUP);
  pinMode(LEDPIN, OUTPUT); 

  if (digitalRead(FORCEDEFAULTPIN) == LOW) {
    digitalWrite(LEDPIN, HIGH);
    forcedefault=true;
  }


  // load configuration saved on eeprom
  IF_SDEBUG(Serial.println(F("try to load configuration from eeprom")));
  int p=0;
  // check for configuration version on eeprom
  char EE_confver[9];
  p+=EEPROM_readAnything(p, EE_confver);

  if((strcmp(EE_confver,confver ) == 0) && !forcedefault)
    {
      //load writable registers
      IF_SDEBUG(Serial.println(F("load writable registers from eeprom")));
      i2c_writabledataset1->load(&p);
      i2c_writabledataset2->oneshot=i2c_writabledataset1->oneshot;
      i2c_writabledataset2->i2c_address=i2c_writabledataset1->i2c_address;
    }
  else
    {
      IF_SDEBUG(Serial.println(F("EEPROM data not useful or set pin activated")));
      IF_SDEBUG(Serial.println(F("set default values for writable registers")));
      // set default to oneshot
      i2c_writabledataset1->oneshot=true;
      i2c_writabledataset2->oneshot=true;
      i2c_writabledataset1->i2c_address = I2C_WIND_DEFAULTADDRESS;
      i2c_writabledataset2->i2c_address = I2C_WIND_DEFAULTADDRESS;
    }

  IF_SDEBUG(Serial.print(F("oneshot: ")));
  IF_SDEBUG(Serial.println(i2c_writabledataset1->oneshot));
  IF_SDEBUG(Serial.print(F("i2c address: ")));
  IF_SDEBUG(Serial.println(i2c_writabledataset1->i2c_address));

  //Start I2C communication routines
  Wire.begin(i2c_writabledataset1->i2c_address);
  
  //The Wire library enables the internal pullup resistors for SDA and SCL.
  //You can turn them off after Wire.begin()
  // do not need this with patched Wire library
  //digitalWrite( SDA, LOW);
  //digitalWrite( SCL, LOW);
  //digitalWrite( SDA, HIGH);
  //digitalWrite( SCL, HIGH);

  Wire.onRequest(requestEvent);          // Set up event handlers
  Wire.onReceive(receiveEvent);

#if defined(PLOT)
  plot.Begin(); // start plotter  
  plot.AddTimeGraph( "Wind", 600, "U", u,"V",v ); 
#endif
  
  starttime = millis();       // start the daily cycle for temperature and humidity

  IF_SDEBUG(Serial.println(F("end setup")));

}

void loop() {

  static uint8_t _command;

  //Check for new incoming command on I2C
  if (new_command!=0) {
    _command = new_command;                                                   //save command byte for processing
    new_command = 0;                                                          //clear it
    //_command = _command & 0x0F;                                               //empty 4MSB bits   
    switch (_command) {
    case I2C_WIND_COMMAND_ONESHOT_START:
      IF_SDEBUG(Serial.println("COMMAND: oneshot start"));
      start=true;
      break;          
    case I2C_WIND_COMMAND_ONESHOT_STOP:
      IF_SDEBUG(Serial.println("COMMAND: oneshot stop"));
      stop=true;
      break;
    case I2C_WIND_COMMAND_SAVE:
      IF_SDEBUG(Serial.println(F("COMMAND: save")));

      // save configuration to eeprom
      IF_SDEBUG(Serial.println(F("save configuration to eeprom")));
      int p=0;

      // save configuration version on eeprom
      p+=EEPROM_writeAnything(p, confver);
      //save writable registers
      i2c_writabledataset2->save(&p);

      break;
    } //switch  

    //IF_SDEBUG(Serial.print(F("oneshot : ")));IF_SDEBUG(Serial.println(i2c_writabledataset2->oneshot));
    //IF_SDEBUG(Serial.print(F("oneshot start : ")));IF_SDEBUG(Serial.println(start));
    //IF_SDEBUG(Serial.print(F("oneshot stop  : ")));IF_SDEBUG(Serial.println(stop));
    
    if (!i2c_writabledataset2->oneshot) return;     // manage oneshot mode only


    //IF_SDEBUG(Serial.println("writable buffer exchange"));
    // disable interrupts for atomic operation
    noInterrupts();
    //exchange double buffer
    i2c_writabledatasettmp=i2c_writabledataset1;
    i2c_writabledataset1=i2c_writabledataset2;
    i2c_writabledataset2=i2c_writabledatasettmp;
    interrupts();

    
    if (start) {
      
      //IF_SDEBUG(Serial.print("# you have to wait sensors for ms:");  Serial.println(MAXWAITTIME));
      digitalWrite(LEDPIN,HIGH);  // blink Led

      start=false;
      regsettime=0;      
      inittime = millis();
      
    }

  
    if (stop) {
      
      digitalWrite(LEDPIN,LOW);
      
      
      if (millis() > (inittime + MAXWAITTIME) ) {
	int baseu = 50;
	int rndu = random(-30,31);
	int hourlyu = round(sin((float(millis()-starttime)/(60.*60.*1.*1000.))*2.*PI)*200.);
	u = baseu + rndu + hourlyu;

	IF_SDEBUG(Serial.println("U:"));
	IF_SDEBUG(Serial.print("base value: "));
	IF_SDEBUG(Serial.println(baseu));
	IF_SDEBUG(Serial.print("random: "));
	IF_SDEBUG(Serial.println(rndu));
	IF_SDEBUG(Serial.print("hourly cycle: "));
	IF_SDEBUG(Serial.println(hourlyu));
	IF_SDEBUG(Serial.print(F("value: ")));
	IF_SDEBUG(Serial.println(u));
	
	int basev = 50;
	int rndv = random(-30,31);
	int hourlyv =  round(cos((float(millis()-starttime)/(60.*60.*1.*1000.))*2.*PI)*200.); // fase opposta ripetto a u

	v = basev + rndv + hourlyv;

	IF_SDEBUG(Serial.println("V:"));
	IF_SDEBUG(Serial.print("base value: "));
	IF_SDEBUG(Serial.println(basev));
	IF_SDEBUG(Serial.print("random: "));
	IF_SDEBUG(Serial.println(rndv));
	IF_SDEBUG(Serial.print("hourly cycle: "));
	IF_SDEBUG(Serial.println(hourlyv));
	IF_SDEBUG(Serial.print(F("value: ")));
	IF_SDEBUG(Serial.println(v));

	/*
	float ar=float(dd)*PI/180.;
	//scambio seno e coseno per rotazione 90 gradi
	u=round(-float(ff)*sin(ar));
	v=round(-float(ff)*cos(ar));
	*/

	int16_t dd;
	uint16_t ff;
	
	if(u == 0 && v == 0) {
	  dd =  0;
	  ff =  0;
	}else{
	  ff=round(sqrt(float(u)*float(u)+float(v)*float(v))) ;
	  //scambio seno e coseno per rotazione 90 gradi
	  dd=round(atan2(float(-u),float(-v))*180./PI);
	  IF_SDEBUG(Serial.print(F("dd1: ")));
	  IF_SDEBUG(Serial.println(dd));
	  dd=dd % 360;
	  IF_SDEBUG(Serial.print(F("dd2: ")));
	  IF_SDEBUG(Serial.println(dd));
	  if(dd == 0) dd=360 ;
	  IF_SDEBUG(Serial.print(F("dd3: ")));
	  IF_SDEBUG(Serial.println(dd));
	  if (dd < 0) dd=360+dd;
	  IF_SDEBUG(Serial.print(F("dd4: ")));
	  IF_SDEBUG(Serial.println(dd));
	  
	}

	
#if defined(PLOT)
	plot.Plot();
#endif

	i2c_dataset1->wind.u=u+OFFSET;
	i2c_dataset1->wind.v=v+OFFSET;
	i2c_dataset1->wind.dd=dd;
	i2c_dataset1->wind.ff=ff;

	
      }else{
	IF_SDEBUG(Serial.print("too fast: "));
	IF_SDEBUG(Serial.println(millis() - (inittime + MAXWAITTIME)));
	i2c_dataset1->wind.u=0xFFFF;
	i2c_dataset1->wind.v=0xFFFF;
	i2c_dataset1->wind.dd=0xFFFF;
	i2c_dataset1->wind.ff=0xFFFF;
      }

      
      IF_SDEBUG(Serial.print(F("U: ")));
      IF_SDEBUG(Serial.println(i2c_dataset1->wind.u));
      IF_SDEBUG(Serial.print(F("V: ")));
      IF_SDEBUG(Serial.println(i2c_dataset1->wind.v));

      IF_SDEBUG(Serial.print(F("dd: ")));
      IF_SDEBUG(Serial.println(i2c_dataset1->wind.dd));
      IF_SDEBUG(Serial.print(F("ff: ")));
      IF_SDEBUG(Serial.println(i2c_dataset1->wind.ff));
      
      //IF_SDEBUG(Serial.println("exchange double buffer"));
      
      // disable interrupts for atomic operation
      noInterrupts();
      //exchange double buffer
      i2c_datasettmp=i2c_dataset1;
      i2c_dataset1=i2c_dataset2;
      i2c_dataset2=i2c_datasettmp;
      interrupts();
      // new data published
      
      //IF_SDEBUG(Serial.println("clean buffer"));
      uint8_t *ptr;
      //Init to FF i2c_dataset1;
      ptr = (uint8_t *)i2c_dataset1;
      for (int i=0;i<REG_MAP_SIZE;i++) { *ptr |= 0xFF; ptr++;}
    
      stop=false;
      regsettime=millis();
    }
  }
}
