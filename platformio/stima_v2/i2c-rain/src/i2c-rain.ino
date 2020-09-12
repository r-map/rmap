/**********************************************************************
Copyright (C) 2016  Paolo Paruno <p.patruno@iperbole.bologna.it>
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
 * This program implements tipping bucket rain gauge measure 
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

#define VERSION 2             //Software version for cross checking

#ifdef ARDUINO_ARCH_AVR
#include <avr/wdt.h>
#endif
#include "Wire.h"
#include "registers-rain_v2.h"         //Register definitions
#include "config.h"
//#include "circular.h"
//#include "IntBuffer.h"
//#include "FloatBuffer.h"

#include "EEPROMAnything.h"

#define REG_MAP_SIZE            sizeof(I2C_REGISTERS)       //size of register map
#define REG_RAIN_SIZE           sizeof(rain_t)                  //size of register map for rain
#define REG_WRITABLE_MAP_SIZE   sizeof(I2C_WRITABLE_REGISTERS)       //size of register map

#define MAX_SENT_BYTES     0x0F                      //maximum amount of data that I could receive from a master device (register, plus 15 byte)

char confver[9] = CONFVER; // version of configuration saved on eeprom


typedef struct {
  uint8_t    sw_version;     // Version of the I2C_RAIN sw
} status_t;

typedef struct {
  uint16_t    tips;
} rain_t;

typedef struct {

//Status registers
  status_t     status;                   //  status register

//rain data
  rain_t                rain;            // 0x01 rain
} __attribute__((packed)) I2C_REGISTERS;


typedef struct {

//sample mode
  bool                  oneshot;         // one shot active
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
static bool oneshot;
static bool start=false;
static bool stop =false;

volatile unsigned int count;
volatile unsigned long antirimb=0;

volatile unsigned long fallingtime=0;
volatile unsigned long risingtime=0;

boolean forcedefault=false;

#ifdef ETGRAINGAUGE
#define INTERRUPTEVENT CHANGE
void countadd()
{
  unsigned long now=millis();
  
  if (digitalRead(RAINGAUGEPIN)==LOW) fallingtime=now;
  else risingtime=now; 
  if(risingtime>fallingtime){
    if (((risingtime-fallingtime) > MIN_COMMUTATION_TIME)&&((risingtime-fallingtime) < MAX_COMMUTATION_TIME)){
      if ((now-antirimb) > DEBOUNCINGTIME){
        count ++;
        //digitalWrite(LEDPIN,count % 2);
        antirimb=now;
      }
    } 
  }
}
#else
#define INTERRUPTEVENT RISING
void countadd()
{
  unsigned long now=millis();
  
  if ((now-antirimb) > DEBOUNCINGTIME){
    
    count ++;
    antirimb=now;
    //IF_SDEBUG(Serial.print(F("count: ")));IF_SDEBUG(Serial.println(count));
  }
}

#endif


//////////////////////////////////////////////////////////////////////////////////////
// I2C handlers
// Handler for requesting data
//
void requestEvent()
{
  Wire.write(((uint8_t *)i2c_dataset2)+receivedCommands[0],32);
  //Write up to 32 byte, since master is responsible for reading and sending NACK
  //32 byte limit is in the Wire library, we have to live with it unless writing our own wire library

  //Serial.print("receivedCommands: ");
  //Serial.println(receivedCommands[0]);
  //Serial.println(*((uint8_t *)(i2c_dataset2)+receivedCommands[0]));
  //Serial.println(*((uint8_t *)(i2c_dataset2)+receivedCommands[0]+1));
  //Serial.println(*((uint8_t *)(i2c_dataset2)+receivedCommands[0]+2));
  //Serial.println(*((uint8_t *)(i2c_dataset2)+receivedCommands[0]+3));
}

//Handler for receiving data
void receiveEvent( int bytesReceived)
{
  uint8_t  *ptr1, *ptr2;
     //Serial.print("received:");
     for (int a = 0; a < bytesReceived; a++) {
          if (a < MAX_SENT_BYTES) {
               receivedCommands[a] = Wire.read();
	       //Serial.println(receivedCommands[a]);
          } else {
               Wire.read();  // if we receive more data then allowed just throw it away
          }
     }

     if (bytesReceived == 2){
       // check for a command
       if (receivedCommands[0] == I2C_RAIN_COMMAND) {
	 //IF_SDEBUG(Serial.print("received command:"));IF_SDEBUG(Serial.println(receivedCommands[1]));
	 new_command = receivedCommands[1]; return; }
     }

     if (bytesReceived == 1){
       //read address for a given register
       //Addressing over the reg_map fallback to first byte
       if(bytesReceived == 1 && ( (receivedCommands[0] < 0) || (receivedCommands[0] >= REG_MAP_SIZE))) {
	 receivedCommands[0]=0;
       }
       //IF_SDEBUG(Serial.print("set register:"));IF_SDEBUG(Serial.println(receivedCommands[0]));
       return;
     }

     //More than 1 byte was received, so there is definitely some data to write into a register
     //Check for writeable registers and discard data is it's not writeable

     //IF_SDEBUG(Serial.println("data for write: "));
     //IF_SDEBUG(Serial.println(receivedCommands[0]));
     //IF_SDEBUG(Serial.println(receivedCommands[1]));
     
     if ((receivedCommands[0]>=I2C_RAIN_MAP_WRITABLE) && (receivedCommands[0] < (I2C_RAIN_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE))) {    
       if ((receivedCommands[0]+(unsigned int)(bytesReceived-1)) <= (I2C_RAIN_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE)) {
	 //Writeable registers
	 // the two buffer should be in sync
	 ptr1 = (uint8_t *)i2c_writabledataset1+receivedCommands[0]-I2C_RAIN_MAP_WRITABLE;
	 ptr2 = (uint8_t *)i2c_writabledataset2+receivedCommands[0]-I2C_RAIN_MAP_WRITABLE;
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

#ifdef ARDUINO_ARCH_AVR
  /*
  Nel caso di un chip in standalone senza bootloader, la prima
  istruzione che è bene mettere nel setup() è sempre la disattivazione
  del Watchdog stesso: il Watchdog, infatti, resta attivo dopo il
  reset e, se non disabilitato, esso può provare il reset perpetuo del
  microcontrollore
  */
  wdt_disable();

  // enable watchdog with timeout to 8s
  wdt_enable(WDTO_8S);
#endif
  
  IF_SDEBUG(Serial.begin(115200));        // connect to the serial port
  IF_SDEBUG(Serial.print(F("Start firmware version: ")));
  IF_SDEBUG(Serial.println(VERSION));

  // inizialize double buffer
  i2c_dataset1=&i2c_buffer1;
  i2c_dataset2=&i2c_buffer2;

  // inizialize writable double buffer
  i2c_writabledataset1=&i2c_writablebuffer1;
  i2c_writabledataset2=&i2c_writablebuffer2;


  IF_SDEBUG(Serial.println(F("i2c_dataset 1&2 set to 1")));

  uint8_t *ptr;
  uint8_t i;
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
      i2c_writabledataset1->i2c_address = I2C_RAIN_DEFAULTADDRESS;
      i2c_writabledataset2->i2c_address = I2C_RAIN_DEFAULTADDRESS;
    }

  oneshot=i2c_writabledataset2->oneshot;

  IF_SDEBUG(Serial.print(F("i2c_address: ")));
  IF_SDEBUG(Serial.println(i2c_writabledataset1->i2c_address));
  IF_SDEBUG(Serial.print(F("oneshot: ")));
  IF_SDEBUG(Serial.println(i2c_writabledataset1->oneshot));

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

  pinMode(RAINGAUGEPIN,INPUT_PULLUP);  // connected to rain sensor switch

  // initialize counter and fuffer for read
  count=0;
  i2c_dataset2->rain.tips=count;
  IF_SDEBUG(Serial.print(F("interrupt...init")));
  attachInterrupt(digitalPinToInterrupt(RAINGAUGEPIN), countadd, INTERRUPTEVENT);
  //detachInterrupt(digitalPinToInterrupt(RAINGAUGEPIN));

  IF_SDEBUG(Serial.println(F("end setup")));

}

void loop() {

  static uint8_t _command;


#ifdef ARDUINO_ARCH_AVR
  wdt_reset();
#endif
  
  //IF_SDEBUG(Serial.println("writable buffer exchange"));
  // disable interrupts for atomic operation
  noInterrupts();
  //exchange double buffer
  i2c_writabledatasettmp=i2c_writabledataset1;
  i2c_writabledataset1=i2c_writabledataset2;
  i2c_writabledataset2=i2c_writabledatasettmp;
  interrupts();

  //Check for new incoming command on I2C
  if (new_command!=0) {
    _command = new_command;           //save command byte for processing
    new_command = 0;                  //clear it
    //_command = _command & 0x0F;     //empty 4MSB bits   
    switch (_command) {
    case I2C_RAIN_COMMAND_START:
      IF_SDEBUG(Serial.println(F("COMMAND: start")));
      start=true;
      break;          
    case I2C_RAIN_COMMAND_STOP:
      IF_SDEBUG(Serial.println(F("COMMAND: stop")));
      stop=true;
      break;
    case I2C_RAIN_COMMAND_STARTSTOP:
      IF_SDEBUG(Serial.println(F("COMMAND: startstop")));
      start=true;
      stop =true;
      break;         
    case I2C_RAIN_COMMAND_SAVE:
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
  }

  //IF_SDEBUG(Serial.print(F("oneshot status: ")));IF_SDEBUG(Serial.println(oneshot));
  //IF_SDEBUG(Serial.print(F("oneshot start : ")));IF_SDEBUG(Serial.println(start));
  //IF_SDEBUG(Serial.print(F("oneshot stop  : ")));IF_SDEBUG(Serial.println(stop));



  // IF_SDEBUG(Serial.print(F("count: ")));IF_SDEBUG(Serial.println(count));
  // IF_SDEBUG(Serial.print(F("rising: ")));IF_SDEBUG(Serial.println(risingtime));
  //if(risingtime>fallingtime) IF_SDEBUG(Serial.print(F("time: ")));IF_SDEBUG(Serial.println(risingtime-fallingtime));


  if (oneshot) {

    if (start || stop) {
      // disable interrupts for atomic operation
      noInterrupts();
    }
    
    if (stop) {

      //update the last data
      i2c_dataset1->rain.tips=count;
      
      //exchange double buffer
      i2c_datasettmp=i2c_dataset1;
      i2c_dataset1=i2c_dataset2;
      i2c_dataset2=i2c_datasettmp;
    }

    if (start) {
      count=0;
      i2c_dataset1->rain.tips=count;
    }
      
    if (start || stop) {
      // new data published
      interrupts();
    }

    //IF_SDEBUG(Serial.println(F("oneshot end")));
    stop =false;
    start=false;

  }

  digitalWrite(LEDPIN,count % 2);  // blink Led

}  
