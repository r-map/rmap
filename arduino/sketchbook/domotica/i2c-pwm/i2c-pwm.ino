/**********************************************************************
Copyright (C) 2018  Paolo Paruno <p.patruno@iperbole.bologna.it>
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
 * This program implements PWM command over i2c interface.
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

#define VERSION 01             //Software version for cross checking
//#define HUGE 4294967296
//#define SHUGE 2147483647

#include <limits.h>
#include <avr/wdt.h>
#include "Wire.h"
#include "registers-pwm.h"      //Register definitions
#include "config.h"

#include "EEPROMAnything.h"

#define REG_MAP_SIZE            sizeof(I2C_REGISTERS)                //size of register map
#define REG_PWM_SIZE           sizeof(values_t)                  //size of register map for th
#define REG_WRITABLE_MAP_SIZE   sizeof(I2C_WRITABLE_REGISTERS)       //size of register map

#define MAX_SENT_BYTES     0x0F   //maximum amount of data that I could receive from a master device (register, plus 15 byte)

char confver[10] = CONFVER; // version of configuration saved on eeprom

typedef struct {
  uint8_t    sw_version;       // 0x00  Version of the I2C_GPS sw
} status_t;

typedef struct {
  uint16_t     sample;
} values_t;

typedef struct {

//Status registers
  status_t     status;         // 0x00  status register

//analog1 data
  values_t             analog1;               // 0x01

//analog2 data
  values_t             analog2;

} I2C_REGISTERS;


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
// one shot management
static bool oneshot;
static bool start=false;
static bool stop=false;

boolean forcedefault=false;

//////////////////////////////////////////////////////////////////////////////////////
// I2C handlers
// Handler for requesting data
//
void requestEvent()
{
  //IF_SDEBUG(Serial.print("request event: "));
  //IF_SDEBUG(Serial.println(receivedCommands[0]));
  //IF_SDEBUG(Serial.println(*((uint8_t *)(i2c_dataset2)+receivedCommands[0]),HEX));
  //IF_SDEBUG(Serial.println(*((uint8_t *)(i2c_dataset2)+receivedCommands[0]+1),HEX));
  //IF_SDEBUG(Serial.println(*((uint8_t *)(i2c_dataset2)+receivedCommands[0]+2),HEX));
  //IF_SDEBUG(Serial.println(*((uint8_t *)(i2c_dataset2)+receivedCommands[0]+3),HEX));

  Wire.write(((uint8_t *)i2c_dataset2)+receivedCommands[0],4);
  //Write up to 4 byte, since master is responsible for reading and sending NACK
  //32 byte limit is in the Wire library, we have to live with it unless writing our own wire library
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
       if (receivedCommands[0] == I2C_PWM_COMMAND) {
	 //IF_SDEBUG(Serial.print("       received command:"));IF_SDEBUG(Serial.println(receivedCommands[1],HEX));
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

     //IF_SDEBUG(Serial.print("         check  writable buffer:"));
     //IF_SDEBUG(Serial.println(receivedCommands[0]));
     //IF_SDEBUG(Serial.println(I2C_PWM_MAP_WRITABLE));
     //IF_SDEBUG(Serial.println(I2C_PWM_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE));

     if ((receivedCommands[0]>=I2C_PWM_MAP_WRITABLE) && (receivedCommands[0] < (I2C_PWM_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE))) {    
       if ((receivedCommands[0]+(unsigned int)(bytesReceived-1)) <= (I2C_PWM_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE)) {
	 //Writeable registers
	 // the two buffer should be in sync
	 ptr1 = (uint8_t *)i2c_writabledataset1+receivedCommands[0]-I2C_PWM_MAP_WRITABLE;
	 ptr2 = (uint8_t *)i2c_writabledataset2+receivedCommands[0]-I2C_PWM_MAP_WRITABLE;
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

void mgr_command(){

  static uint8_t _command;

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
    _command = new_command;                                                   //save command byte for processing
    new_command = 0;                                                          //clear it
    //_command = _command & 0x0F;                                               //empty 4MSB bits   
    switch (_command) {
    case I2C_PWM_COMMAND_DO:
      IF_SDEBUG(Serial.println("COMMAND: do"));
      start=true;

      analogWrite(LED_PIN,512);  
      
      break;
    case I2C_PWM_COMMAND_ONESHOT_START:
      IF_SDEBUG(Serial.println("COMMAND: oneshot start"));
      start=true;
      break;          
    case I2C_PWM_COMMAND_ONESHOT_STOP:
      IF_SDEBUG(Serial.println("COMMAND: oneshot stop"));
      stop=true;
      break;
    case I2C_PWM_COMMAND_SAVE:
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


  //IF_SDEBUG(Serial.print(F("oneshot : ")));IF_SDEBUG(Serial.println(i2c_writabledataset2->oneshot));
  //IF_SDEBUG(Serial.print(F("oneshot start : ")));IF_SDEBUG(Serial.println(start));
  //IF_SDEBUG(Serial.print(F("oneshot stop  : ")));IF_SDEBUG(Serial.println(stop));

  if (stop) {

    IF_SDEBUG(Serial.println("exchange double buffer"));

    // disable interrupts for atomic operation
    noInterrupts();
    //exchange double buffer
    i2c_datasettmp=i2c_dataset1;
    i2c_dataset1=i2c_dataset2;
    i2c_dataset2=i2c_datasettmp;
    interrupts();
    // new data published

    IF_SDEBUG(Serial.println("clean buffer"));
    uint8_t *ptr;
    //Init to FF i2c_dataset1;
    ptr = (uint8_t *)i2c_dataset1;
    for (int i=0;i<REG_MAP_SIZE;i++) { *ptr |= 0xFF; ptr++;}
 
   stop=false;
  }
}



void setup() {

  uint8_t i;

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

  pinMode(PWM1_PIN, OUTPUT);
  pinMode(PWM2_PIN, OUTPUT);

  if (digitalRead(FORCEDEFAULTPIN) == LOW) {
    digitalWrite(LEDPIN, HIGH);
    forcedefault=true;
  }


  // load configuration saved on eeprom
  IF_SDEBUG(Serial.println(F("try to load configuration from eeprom")));
  int p=0;
  // check for configuration version on eeprom
  char EE_confver[10];
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
      i2c_writabledataset1->oneshot=false;
      i2c_writabledataset2->oneshot=false;
      i2c_writabledataset1->i2c_address = I2C_PWM_DEFAULTADDRESS;
      i2c_writabledataset2->i2c_address = I2C_PWM_DEFAULTADDRESS;
    }

  IF_SDEBUG(Serial.print(F("i2c address: ")));
  IF_SDEBUG(Serial.println(i2c_writabledataset1->i2c_address));
  IF_SDEBUG(Serial.print(F("oneshot: ")));
  IF_SDEBUG(Serial.println(i2c_writabledataset1->oneshot));

  oneshot=i2c_writabledataset2->oneshot;
  
  //Start I2C communication routines
  Wire.begin(i2c_writabledataset1->i2c_address);
  
  //set the i2c clock 
  //TWBR = ((F_CPU / I2C_CLOCK) - 16) / 2;
  //TWBR =255    //  30418,25 Hz  : minimum freq with prescaler set to 1 and CPU clock to 16MHz
  Wire.setClock(I2C_CLOCK);

  //The Wire library enables the internal pullup resistors for SDA and SCL.
  //You can turn them off after Wire.begin()
  // do not need this with patched Wire library
  //digitalWrite( SDA, LOW);
  //digitalWrite( SCL, LOW);
  //digitalWrite( SDA, HIGH);
  //digitalWrite( SCL, HIGH);

  Wire.onRequest(requestEvent);          // Set up event handlers
  Wire.onReceive(receiveEvent);


  IF_SDEBUG(Serial.println(F("end setup")));

}

void loop() {

  wdt_reset();

  mgr_command();

  i2c_dataset1->analog1.sample=LONG_MAX;
  i2c_dataset1->analog2.sample=LONG_MAX;

  IF_SDEBUG(Serial.print(F("analog1: ")));
  IF_SDEBUG(Serial.println(i2c_dataset1->analog1.sample));
  IF_SDEBUG(Serial.print(F("analog2: ")));
  IF_SDEBUG(Serial.println(i2c_dataset1->analog2.sample));

  if (oneshot) {
    //if one shot we have finish
    IF_SDEBUG(Serial.println(F("oneshot end")));
    start=false;    
    return;
  }
  
  digitalWrite(LEDPIN,!digitalRead(LEDPIN));  // blink Led

}
