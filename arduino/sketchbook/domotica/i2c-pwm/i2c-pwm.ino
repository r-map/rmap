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
You have two operative mode for reading analog values:
* oneshot
  - send command START to do one analog read
  - send command STOP to expose the new analog values for reading
  - write register to read
  - read register

* continous mode
  - write register to read
  - read register to get the last measure done

* to set new value for operator
  - write register to write
  - write new value to set the register
  - send command TAKE to get new values in account

*/

/*
How can I use PWM in power saving mode (ATmega328)?
https://arduino.stackexchange.com/questions/46995/how-can-i-use-pwm-in-power-saving-mode-atmega328
ossia quasi NO

per� posso andare in sleep quando tutti i PWM sono a 0 ossia la maggior parte del tempo;

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
#include "registers-pwm.h"      //Register definitions
#include "config.h"
#include "EEPROMAnything.h"
#include <ArduinoLog.h>
#include <avr/sleep.h>

// logging level at compile time
// Available levels are:
// LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
#define LOG_LEVEL   LOG_LEVEL_NOTICE


#define REG_MAP_SIZE            sizeof(I2C_REGISTERS)                //size of register map
#define REG_PWM_SIZE            sizeof(values_t)                     //size of register map for pwm
#define REG_WRITABLE_MAP_SIZE   sizeof(I2C_WRITABLE_REGISTERS)       //size of register map

#define MAX_SENT_BYTES     0x0F   //maximum amount of data that I could receive from a master device (register, plus 15 byte)

char confver[10] = CONFVER;       // version of configuration saved on eeprom

typedef struct {
  uint8_t    sw_version;          // Version of the I2C_GPS sw
} status_t;

typedef struct {
  uint16_t     analog1;
  uint16_t     analog2;
} values_t;

typedef struct {
  //Status registers
  status_t     status;
  //analog data
  values_t             analog;
} I2C_REGISTERS;


typedef struct {

//sample mode
  bool                  oneshot;                  // one shot active
  uint8_t               i2c_address;              // i2c bus address (short unsigned int)
  uint8_t               pwm1;                     // pwm 1 value
  uint8_t               pwm2;                     // pwm 2 value
  uint8_t               onoff1;                   // on/off 1 value
  uint8_t               onoff2;                   // on/off 2 value

  void save (int* p) volatile {                            // save to eeprom
    LOGN(F("oneshot: %T" CR),oneshot);
    LOGN(F("i2c address: %d" CR),i2c_address);

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
static bool start=false;
static bool stop=false;
static bool take=false;

boolean forcedefault=false;
volatile unsigned long counter=0;
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
  
  Wire.write(((uint8_t *)i2c_dataset2)+receivedCommands[0],32);
  //Write up to 32 byte, since master is responsible for reading and sending NACK
  //32 byte limit is in the Wire library, we have to live with it unless writing our own wire library
}

//Handler for receiving data
void receiveEvent( int bytesReceived)
{

  //LOGN("receive event, bytes: %d" CR,bytesReceived);
  
  uint8_t  *ptr1, *ptr2;
  counter = 0;
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
    if(bytesReceived == 1 && ( (receivedCommands[0] < 0) || (receivedCommands[0] >= REG_MAP_SIZE))) {
      receivedCommands[0]=0;
    }
    //LOGN("set register: %X" CR,receivedCommands[0]);
    return;
  }
  
  if (bytesReceived == 2){
    // check for a command
    if (receivedCommands[0] == I2C_PWM_COMMAND) {
      //LOGN("       received command: %X" CR,receivedCommands[1]);
      new_command = receivedCommands[1]; return; }
  }
  
  //More than 1 byte was received, so there is definitely some data to write into a register
  //Check for writeable registers and discard data is it's not writeable

  //LOGN("         check  writable buffer: %X %X %X" CR,receivedCommands[0],I2C_PWM_MAP_WRITABLE,I2C_PWM_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE);
  
  if ((receivedCommands[0]>=I2C_PWM_MAP_WRITABLE) && (receivedCommands[0] < (I2C_PWM_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE))) {    
    if ((receivedCommands[0]+(unsigned int)(bytesReceived-1)) <= (I2C_PWM_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE)) {
      //Writeable registers
      // the two buffer should be in sync
      //ptr1 = (uint8_t *)i2c_writabledataset1+receivedCommands[0]-I2C_PWM_MAP_WRITABLE;
      ptr2 = (uint8_t *)i2c_writabledataset2+receivedCommands[0]-I2C_PWM_MAP_WRITABLE;
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
    case I2C_PWM_COMMAND_TAKE:
      {
	LOGN(F("COMMAND: take" CR));
      
	//LOGN("writable buffer exchange"));
	// disable interrupts for atomic operation
	noInterrupts();

	// copy writable registers
	memcpy ( (void *)i2c_writabledataset1, (void *)i2c_writabledataset2, REG_WRITABLE_MAP_SIZE );
	
	/*
	//exchange double buffer
	i2c_writabledatasettmp=i2c_writabledataset1;
	i2c_writabledataset1=i2c_writabledataset2;
	i2c_writabledataset2=i2c_writabledatasettmp;
	*/

	interrupts();
	
	take=true;
	
	break;
      }
    case I2C_PWM_COMMAND_ONESHOT_START:
      {
	LOGN(F("COMMAND: oneshot start" CR));
      
	if (!i2c_writabledataset1->oneshot) break;
	
	start=true;
	
	//LOGN(F("reset registers to missing" CR));
	uint8_t *ptr;
	//Init to FF i2c_dataset1;
	ptr = (uint8_t *)&i2c_dataset1->analog;
	for (int i=0;i<REG_PWM_SIZE;i++) { *ptr |= 0xFF; ptr++;}

	// disable interrupts for atomic operation
	noInterrupts();
	//exchange double buffer
	LOGN(F("exchange double buffer" CR));
	i2c_datasettmp=i2c_dataset1;
	i2c_dataset1=i2c_dataset2;
	i2c_dataset2=i2c_datasettmp;
	interrupts();
	// new data published
	//Init to FF i2c_dataset1;
	ptr = (uint8_t *)&i2c_dataset1->analog;
	for (int i=0;i<REG_PWM_SIZE;i++) { *ptr |= 0xFF; ptr++;}	
	
	break;
      }
    case I2C_PWM_COMMAND_ONESHOT_STOP:
      {
	LOGN(F("COMMAND: oneshot stop" CR));
	
	if (!i2c_writabledataset1->oneshot) break;
	
	// disable interrupts for atomic operation
	noInterrupts();
	//exchange double buffer
	LOGN(F("exchange double buffer" CR));
	i2c_datasettmp=i2c_dataset1;
	i2c_dataset1=i2c_dataset2;
	i2c_dataset2=i2c_datasettmp;
	interrupts();
	// new data published
	
	LOGN(F("clean buffer" CR));
	uint8_t *ptr;
	//Init to FF i2c_dataset1;
	ptr = (uint8_t *)&i2c_dataset1->analog;
	for (int i=0;i<REG_PWM_SIZE;i++) { *ptr |= 0xFF; ptr++;}
	
	stop=true;
	break;
      }
    case I2C_PWM_COMMAND_SAVE:
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
      
      LOGN(F("clean buffer" CR));
      uint8_t *ptr;
      //Init to FF i2c_dataset1;
      ptr = (uint8_t *)&i2c_dataset1->analog;
      for (int i=0;i<REG_PWM_SIZE;i++) { *ptr |= 0xFF; ptr++;}
  }
  
  //LOGN(F("oneshot : %T" CR),i2c_writabledataset2->oneshot);
  //LOGN(F("oneshot start : %T" CR), start);
  //LOGN(F("oneshot stop  : %T" CR)), stop);

}



void filter_data(unsigned int n, unsigned int *table, volatile uint16_t *value)
{
    unsigned int value_min, value_max, value_sum;

    value_sum = value_min = value_max = table[0];

    for (int i=1; i<n; i++) {
        if (table[i] < value_min) {
            value_min = table[i];
        }
        if (table[i] > value_max) {
            value_max = table[i];
        }
        value_sum += table[i];
    }

    if (n > 2) {
        *value = (value_sum - value_max - value_min)/(n-2);
    } else if (n > 1) {
        *value = (value_sum - value_min)/(n-1);
    } else {
        *value = value_sum/n;
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

  
  Serial.begin(115200);        // connect to the serial port

  // Pass log level, whether to show log level, and print interface.
  // Available levels are:
  // LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
  // Note: if you want to fully remove all logging code, change #define LOG_LEVEL ....
  //       this will significantly reduce your project size

  // set runtime log level to the same of compile time
  Log.begin(LOG_LEVEL, &Serial);

  LOGN(F("Start firmware version: %s" CR),VERSION);

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

  pinMode(LED_PIN, OUTPUT);
  pinMode(PWM1_PIN, OUTPUT);
  pinMode(PWM2_PIN, OUTPUT);
  pinMode(ONOFF1_PIN, OUTPUT);
  pinMode(ONOFF2_PIN, OUTPUT);
  pinMode(ANALOG1_PIN, INPUT);
  pinMode(ANALOG2_PIN, INPUT);

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
      // set default to oneshot
      i2c_writabledataset2->oneshot=true;
      i2c_writabledataset2->i2c_address = I2C_PWM_DEFAULTADDRESS;
    }

  i2c_writabledataset2->pwm1 = 0;
  i2c_writabledataset2->pwm2 = 0;
  i2c_writabledataset2->onoff1 = 0;
  i2c_writabledataset2->onoff2 = 0;

  // copy writable registers
  memcpy ( (void *)i2c_writabledataset1, (void *)i2c_writabledataset2, REG_WRITABLE_MAP_SIZE );

  
  LOGN(F("i2c address: %X" CR),i2c_writabledataset1->i2c_address);
  LOGN(F("oneshot: %T" CR),i2c_writabledataset1->oneshot);
  
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


  LOGN(F("end setup" CR));

}

void loop() {

  wdt_reset();
  
  mgr_command();

  if (take) {
    
    LOGN(F("pwm1: %d" CR),i2c_writabledataset1->pwm1);
    analogWrite(PWM1_PIN, i2c_writabledataset1->pwm1);  
    LOGN(F("pwm2: %d" CR),i2c_writabledataset1->pwm2);
    analogWrite(PWM2_PIN, i2c_writabledataset1->pwm2);  

    LOGN(F("onoff1: %T" CR),i2c_writabledataset1->onoff1);
    digitalWrite(ONOFF1_PIN, (i2c_writabledataset1->onoff1 == 0) ? LOW : HIGH );  
    LOGN(F("onoff2: %T" CR),i2c_writabledataset1->onoff2);
    digitalWrite(ONOFF2_PIN, (i2c_writabledataset1->onoff2 == 0) ? LOW : HIGH );  

    take =false;
  }

  if (start || !i2c_writabledataset1->oneshot) {

    // start for oneshot or continuos mode
  
    i2c_dataset1->analog.analog1=LONG_MAX;
    i2c_dataset1->analog.analog2=LONG_MAX;

    unsigned int table1[NSAMPLE];
    unsigned int table2[NSAMPLE];

    for (int i =0; i < NSAMPLE; i++){
      table1[i] = analogRead(ANALOG1_PIN);
      //LOGN(F("read1: %d" CR),table1[i]);
      table2[i] = analogRead(ANALOG2_PIN);
      //LOGN(F("read2: %d" CR),table2[i]);
      delay(10);
    }

    filter_data(NSAMPLE, table1,&i2c_dataset1->analog.analog1);
    filter_data(NSAMPLE, table2,&i2c_dataset1->analog.analog2);
    
    LOGN(F("analog1: %d" CR),i2c_dataset1->analog.analog1);
    LOGN(F("analog2: %d" CR),i2c_dataset1->analog.analog2);

    start=false;

  }

  if (stop) {
    // nothing to do
    stop=false;
  }

  digitalWrite(LED_PIN,!digitalRead(LED_PIN));  // blink Led

  if (      i2c_writabledataset1->pwm1 == 0
	 && i2c_writabledataset1->pwm2 == 0
	    && i2c_writabledataset1->onoff1 == 0
	    && i2c_writabledataset1->onoff2 == 0
	    && ++counter >= 500000
	    )
    {

      //LOGN(F("Sleep" CR));
      //delay(10);
      
      set_sleep_mode(SLEEP_MODE_PWR_DOWN);  
      // disable watchdog
      wdt_disable();
      sleep_enable();
      sleep_cpu();
      sleep_disable();
      // enable watchdog with timeout to 8s
      wdt_enable(WDTO_8S);
      LOGN(F("Wake up" CR));
    }
  
}
