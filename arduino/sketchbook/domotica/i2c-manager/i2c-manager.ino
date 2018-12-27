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
 * This program implements management of remote imput over I2C
 * 
**********************************************************************/
/*
buffer scrivibili da i2c
viene scritto buffer2
viene letto buffer1
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
#include "registers-gpio.h"      //Register definitions
#include "registers-manager.h"      //Register definitions
#include "config.h"
#include "EEPROMAnything.h"
#include <ArduinoLog.h>
#include <avr/sleep.h>

// logging level at compile time
// Available levels are:
// LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
#define LOG_LEVEL   LOG_LEVEL_VERBOSE


#define REG_MAP_SIZE            sizeof(I2C_REGISTERS)                //size of readable register map
#define REG_WRITABLE_MAP_SIZE   sizeof(I2C_WRITABLE_REGISTERS)       //size of writable register map

#define MAX_SENT_BYTES     0x0F   //maximum amount of data that I could receive from a master device 

char confver[10] = CONFVER;       // version of configuration saved on eeprom

typedef struct {
  uint8_t    sw_version;          // Version of the I2C_GPS sw
} status_t;

typedef struct {
  bool          active ;
  unsigned int  long_press;
} button_t;

typedef struct {
  button_t     button;
} values_t;

typedef struct {
  //Status registers
  status_t     status;
  values_t     values;
} I2C_REGISTERS; //Readable registers

typedef struct {

//sample mode
  bool                  oneshot;                  // one shot active
  uint8_t               i2c_address;              // i2c bus address (short unsigned int)
  button_t              button;		          // struct for button registers

  void save (int* p) volatile {                            // save to eeprom
    LOGN(F("oneshot: %T" CR),oneshot);
    LOGN(F("i2c address: %d" CR),i2c_address);
    *p+=EEPROM_writeAnything(*p, oneshot);
    *p+=EEPROM_writeAnything(*p, i2c_address);
    *p+=EEPROM_writeAnything(*p, button.active);
    *p+=EEPROM_writeAnything(*p, button.long_press);
  }
  
  void load (int* p) volatile {                            // load from eeprom
    *p+=EEPROM_readAnything(*p, oneshot);
    *p+=EEPROM_readAnything(*p, i2c_address);
    *p+=EEPROM_readAnything(*p, button.active);
    *p+=EEPROM_readAnything(*p, button.long_press);
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
//volatile static I2C_WRITABLE_REGISTERS* i2c_writabledatasettmp;

volatile static uint8_t         receivedCommands[MAX_SENT_BYTES];
volatile static uint8_t         new_command;                        //new command received (!=0)

boolean forcedefault=false;
volatile unsigned long long counter=millis();
bool take=false;

//////////////////////////////////////////////////////////////////////////////////////
// I2C handlers
// Handler for requesting data
//
void requestEvent()
{
  /*
  LOGN("request event: comm %X : %X %X %X %X " CR,
       receivedCommands[0],
       *((uint8_t *)(i2c_dataset2)+receivedCommands[0]),
       *((uint8_t *)(i2c_dataset2)+receivedCommands[0]+1),
       *((uint8_t *)(i2c_dataset2)+receivedCommands[0]+2),
       *((uint8_t *)(i2c_dataset2)+receivedCommands[0]+3));
  */

  counter = millis();
  sleep_disable();
  
  Wire.write(((uint8_t *)i2c_dataset2)+receivedCommands[0],32);
  //Write up to 32 byte, since master is responsible for reading and sending NACK
  //32 byte limit is in the Wire library, we have to live with it unless writing our own wire library
}

//Handler for receiving data
void receiveEvent( int bytesReceived)
{

  //LOGN("receive event, bytes: %d" CR,bytesReceived);
  
  uint8_t  *ptr2;

  counter = millis();
  sleep_disable();
 
  for (int a = 0; a < bytesReceived; a++) {
    if (a < MAX_SENT_BYTES) {
      receivedCommands[a] = Wire.read();
      //LOGN("received: %X" CR, receivedCommands[a]);
    } else {
      Wire.read();  // if we receive more data then allowed just throw it away
    }
  }
  
  if (bytesReceived == 1){
    //read address for a given register
    //Addressing over the reg_map fallback to first byte
    if(bytesReceived == 1 && (receivedCommands[0] >= REG_MAP_SIZE)) {
      receivedCommands[0]=0;
    }
    //LOGN("set register: %X" CR,receivedCommands[0]);
    return;
  }
  
  if (bytesReceived == 2){
    // check for a command
    if (receivedCommands[0] == I2C_MANAGER_COMMAND) {
      //LOGN("       received command: %X" CR,receivedCommands[1]);
      new_command = receivedCommands[1]; return; }
  }
  
  //More than 1 byte was received, so there is definitely some data to write into a register
  //Check for writeable registers and discard data is it's not writeable

  //LOGN("         check  writable buffer: %X %X %X" CR,receivedCommands[0],I2C_PWM_MAP_WRITABLE,I2C_PWM_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE);
  
  if ((receivedCommands[0]>=I2C_MANAGER_MAP_WRITABLE) && (receivedCommands[0] < (I2C_MANAGER_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE))) {    
    if ((receivedCommands[0]+(unsigned int)(bytesReceived-1)) <= (I2C_MANAGER_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE)) {
      //Writeable registers
      // the two buffer should be in sync
      //ptr1 = (uint8_t *)i2c_writabledataset1+receivedCommands[0]-I2C_MANAGER_MAP_WRITABLE;
      ptr2 = (uint8_t *)i2c_writabledataset2+receivedCommands[0]-I2C_MANAGER_MAP_WRITABLE;
      for (int a = 1; a < bytesReceived; a++) { 
	//LOGN("write in writable buffer: %X ,%X" CR,a,receivedCommands[a]);
	//*ptr1++ = receivedCommands[a];
	*ptr2++ = receivedCommands[a];
      }
      // new data written
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void mgr_command(){

  static uint8_t _command;
  
  //Check for new incoming command on I2C
  if (new_command != 0) {
    _command = new_command;                                                   //save command byte for processing
    new_command = 0;                                                          //clear it
    //_command = _command & 0x0F;                                               //empty 4MSB bits   
    switch (_command) {
    case I2C_MANAGER_COMMAND_SAVE:
      {
	LOGN(F("COMMAND: save" CR));
      
	// save configuration to eeprom
	LOGN(F("save configuration to eeprom" CR));
	int p=0;
	
	// save configuration version on eeprom
	p+=EEPROM_writeAnything(p, confver);
	//save writable registers
	i2c_writabledataset2->save(&p);
      
	break;
      }

    case I2C_MANAGER_COMMAND_BUTTON1_SHORTPRESSED:
      {
	LOGN(F("COMMAND: button 1 short pressed" CR));
      
	//LOGN("writable buffer exchange"));
	// disable interrupts for atomic operation
	//noInterrupts();

	// copy writable registers
	//memcpy ( (void *)i2c_writabledataset1, (void *)i2c_writabledataset2, REG_WRITABLE_MAP_SIZE );
	
	/*
	//exchange double buffer
	i2c_writabledatasettmp=i2c_writabledataset1;
	i2c_writabledataset1=i2c_writabledataset2;
	i2c_writabledataset2=i2c_writabledatasettmp;
	*/

	//interrupts();
	
	take=true;
	
	break;
      }

    case I2C_MANAGER_COMMAND_BUTTON1_LONGPRESSED:
      {
	LOGN(F("COMMAND: BUTTON1_LONGPRESSED" CR));
	break;
      }

    case I2C_MANAGER_COMMAND_ENCODER_RIGHT:
      {
	LOGN(F("COMMAND: ENCODER_RIGHT" CR));
	break;
      }

    case I2C_MANAGER_COMMAND_ENCODER_LEFT:
      {
	LOGN(F("COMMAND: ENCODER_LEFT" CR));
	break;
      }

    default:
      {
	LOGN(F("WRONG command" CR));
	break;
      }	
    } //switch  
  }

  if (!i2c_writabledataset1->oneshot){
    // continuos mode
    LOGN(F("expose new measure in continuos mode" CR));

      // disable interrupts for atomic operation
      noInterrupts();
      //exchange double buffer
      LOGN(F("exchange double buffer" CR));
      i2c_datasettmp=i2c_dataset1;
      i2c_dataset1=i2c_dataset2;
      i2c_dataset2=i2c_datasettmp;
      interrupts();
      // new data published
      /*
      LOGN(F("clean buffer" CR));
      uint8_t *ptr;
      //Init to FF i2c_dataset1;
      ptr = (uint8_t *)&i2c_dataset1->values.analog;
      for (int i=0;i<REG_ANALOG_SIZE;i++) { *ptr |= 0xFF; ptr++;}
      */
  }
  
  //LOGN(F("oneshot : %T" CR),i2c_writabledataset2->oneshot);
  //LOGN(F("oneshot start : %T" CR), start);
  //LOGN(F("oneshot stop  : %T" CR)), stop);

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

  
  Serial.begin(115200);        // connect to the serial port

  // Pass log level, whether to show log level, and print interface.
  // Available levels are:
  // LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
  // Note: if you want to fully remove all logging code, change #define LOG_LEVEL ....
  //       this will significantly reduce your project size

  // set runtime log level to the same of compile time
  Log.begin(LOG_LEVEL, &Serial);

  LOGN(F("Start firmware version: %d" CR),VERSION);

  // inizialize double buffer
  i2c_dataset1=&i2c_buffer1;
  i2c_dataset2=&i2c_buffer2;

  // inizialize writable double buffer
  i2c_writabledataset1=&i2c_writablebuffer1;
  i2c_writabledataset2=&i2c_writablebuffer2;


  LOGN(F("i2c_dataset 1&2 set to 1" CR));

  uint8_t *ptr;
  //Init to FF i2c_dataset1;
  ptr = (uint8_t *)i2c_dataset1;
  for (i=0;i<REG_MAP_SIZE;i++) { *ptr |= 0xFF; ptr++;}

  //Init to FF i2c_dataset2;
  ptr = (uint8_t *)i2c_dataset2;
  for (i=0;i<REG_MAP_SIZE;i++) { *ptr |= 0xFF; ptr++;}



  LOGN(F("i2c_writabledataset 1&2 set to 1" CR));
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
  pinMode(CHANGEADDRESS1, INPUT_PULLUP);
  pinMode(CHANGEADDRESS2, INPUT_PULLUP);
  
  pinMode(LED_PIN, OUTPUT);
  
  if (digitalRead(FORCEDEFAULTPIN) == LOW) {
    digitalWrite(LED_PIN, HIGH);
    forcedefault=true;
  }

  // load configuration saved on eeprom
  LOGN(F("try to load configuration from eeprom" CR));
  int p=0;
  // check for configuration version on eeprom
  char EE_confver[10];
  p+=EEPROM_readAnything(p, EE_confver);

  if((strcmp(EE_confver,confver ) == 0) && !forcedefault)
    {
      //load writable registers
      LOGN(F("load writable registers from eeprom" CR));
      i2c_writabledataset2->load(&p);
    }
  else
    {
      LOGN(F("EEPROM data not useful or set pin activated" CR));
      LOGN(F("set default values for writable registers" CR));
      // set default
      i2c_writabledataset2->oneshot=true;
      i2c_writabledataset2->i2c_address = I2C_MANAGER_DEFAULTADDRESS;
      i2c_writabledataset2->button.active=true;
      i2c_writabledataset2->button.long_press=1000;
    }

  // copy writable registers
  memcpy ( (void *)i2c_writabledataset1, (void *)i2c_writabledataset2, REG_WRITABLE_MAP_SIZE );
  
  uint8_t i2c_address=i2c_writabledataset1->i2c_address;

  if (digitalRead(CHANGEADDRESS1) == LOW) {
    i2c_address += 1;
  }
  if (digitalRead(CHANGEADDRESS2) == LOW) {
    i2c_address += 2;
  }
  
  LOGN(F("i2c soft address: %X" CR),i2c_writabledataset1->i2c_address);
  LOGN(F("i2c hard address: %X" CR),i2c_address);
  LOGN(F("oneshot: %T" CR),i2c_writabledataset1->oneshot);
  
  //Start I2C communication routines
  Wire.begin(i2c_address);
  
  //set the i2c clock 
  //TWBR = ((F_CPU / I2C_CLOCK) - 16) / 2;
  //TWBR =255    //  30418,25 Hz  : minimum freq with prescaler set to 1 and CPU clock to 16MHz
  Wire.setClock(I2C_CLOCK);

  //The Wire library enables the internal pullup resistors for SDA and SCL.
  //You can turn them off after Wire.begin()
  // do not need this with patched Wire library
  //digitalWrite( SDA, LOW);
  //digitalWrite( SCL, LOW);

  digitalWrite( SDA, HIGH);
  digitalWrite( SCL, HIGH);

  Wire.onRequest(requestEvent);          // Set up event handlers
  Wire.onReceive(receiveEvent);

  LOGN(F("end setup" CR));

}

void loop() {

  wdt_reset();

  mgr_command();

  /*
  if (take) {
    
    LOGN(F("onoff2: %T" CR),i2c_writabledataset1->onoff2);
    digitalWrite(ONOFF2_PIN, (i2c_writabledataset1->onoff2 == 0) ? LOW : HIGH );  

    take =false;
  }
  */

  digitalWrite(LED_PIN,HIGH);  //  Led on

  if ((millis()-counter) >= 3000)
    {
      counter=millis();

      digitalWrite(LED_PIN,LOW);  //  Led off
      LOGN(F("Sleep" CR));
      delay(10);
      
      set_sleep_mode(SLEEP_MODE_PWR_DOWN);
      // disable watchdog
      wdt_disable();
      sleep_enable();
      sleep_cpu();
      sleep_disable();
      // enable watchdog with timeout to 8s
      wdt_enable(WDTO_8S);
      LOGN(F(">>>>> Wake up" CR));
    }
}
