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
 * This program implements elaboration of sds011 inovafitness sensor
 * for pm2.5 and pm 10 exported to i2c interface.
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

#include <limits.h>
#include <avr/wdt.h>
#include "Wire.h"
#include "registers-sds011.h"         //Register definitions
#include "config.h"
#include "IntBuffer.h"
#include "FloatBuffer.h"
#include "Sds011.h"

#include "EEPROMAnything.h"

#define REG_MAP_SIZE            sizeof(I2C_REGISTERS)       //size of register map
#define REG_PM_SIZE           sizeof(pm_t)                  //size of register map for pm
#define REG_WRITABLE_MAP_SIZE   sizeof(I2C_WRITABLE_REGISTERS)       //size of register map

#define MAX_SENT_BYTES     0x0F                      //maximum amount of data that I could receive from a master device (register, plus 15 byte)

char confver[9] = CONFVER; // version of configuration saved on eeprom

sds011::Sds011 sensor(SERIALSDS011);


IntBuffer cbpm2560n;
IntBuffer cbpm1060n;

IntBuffer cbpm2560m;
IntBuffer cbpm1060m;

IntBuffer cbpm2560x;
IntBuffer cbpm1060x;

FloatBuffer cbsum2pm25;
FloatBuffer cbsumpm25;
FloatBuffer cbsum2pm10;
FloatBuffer cbsumpm10;


typedef struct {
  uint8_t    sw_version;                          // Version of the I2C_SDS011 sw
} status_t;

typedef struct {
  uint16_t    pm25;
  uint16_t    pm10;
  uint16_t     minpm25;
  uint16_t     minpm10;
  uint16_t     meanpm25;
  uint16_t     meanpm10;
  uint16_t     maxpm25;
  uint16_t     maxpm10;
  uint16_t     sigmapm25;
  uint16_t     sigmapm10;
} pm_t;

typedef struct {

//Status registers
  status_t     status;                   // 0x00  status register

//pm data
  pm_t                pm;                     // 0x01 pm
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

float meanpm25;
float meanpm10;
long int minpm25;
long int minpm10;
long int maxpm25;
long int maxpm10;
float sum2pm25;
float sumpm25;
float sum2pm10;
float sumpm10;
float sum260;
float sum60;
uint8_t nsample1;

// one shot management
static bool oneshot;
static bool start=false;
static bool stop=false;

volatile unsigned int count;

unsigned long starttime;
boolean forcedefault=false;

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
       if (receivedCommands[0] == I2C_SDS011_COMMAND) {
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
     
     if ((receivedCommands[0]>=I2C_SDS011_MAP_WRITABLE) && (receivedCommands[0] < (I2C_SDS011_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE))) {    
       if ((receivedCommands[0]+(unsigned int)(bytesReceived-1)) <= (I2C_SDS011_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE)) {
	 //Writeable registers
	 // the two buffer should be in sync
	 ptr1 = (uint8_t *)i2c_writabledataset1+receivedCommands[0]-I2C_SDS011_MAP_WRITABLE;
	 ptr2 = (uint8_t *)i2c_writabledataset2+receivedCommands[0]-I2C_SDS011_MAP_WRITABLE;
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

#define SAMPLE1 60000/SAMPLERATE
#define SAMPLE2 10

  meanpm25=0.;
  meanpm10=0.;

  maxpm25=-1;
  maxpm10=-1;

  minpm25=LONG_MAX;
  minpm10=LONG_MAX;

  sum2pm25=0;
  sumpm25=0;

  sum2pm10=0;
  sumpm10=0;

  nsample1=1;

  cbpm2560n.init(SAMPLE2);
  cbpm1060n.init(SAMPLE2);

  cbpm2560m.init(SAMPLE2);
  cbpm1060m.init(SAMPLE2);

  cbpm2560x.init(SAMPLE2);
  cbpm1060x.init(SAMPLE2);

  cbsum2pm25.init(SAMPLE2);
  cbsumpm25.init(SAMPLE2);

  cbsum2pm10.init(SAMPLE2);
  cbsumpm10.init(SAMPLE2);

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
      i2c_writabledataset1->i2c_address = I2C_SDS011_DEFAULTADDRESS;
      i2c_writabledataset2->i2c_address = I2C_SDS011_DEFAULTADDRESS;
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

  SERIALSDS011.begin(9600);

  if (oneshot) sensor.set_sleep(true);
  sensor.set_mode(sds011::QUERY);

  starttime = millis()+SAMPLERATE;

  IF_SDEBUG(Serial.println(F("end setup")));

}

void loop() {

  static uint8_t _command;
  //unsigned int pm25;
  //unsigned int pm10;
  int pm25;
  int pm10;

  float mean, min, max;
  
  uint8_t i;
  bool ok;

  wdt_reset();

  //IF_SDEBUG(Serial.println("writable buffer exchange"));
  // disable interrupts for atomic operation
  noInterrupts();
  //exchange double writable buffer
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
    case I2C_SDS011_COMMAND_ONESHOT_START:
      IF_SDEBUG(Serial.println(F("COMMAND: oneshot start")));
      start=true;
      break;          
    case I2C_SDS011_COMMAND_ONESHOT_STOP:
      IF_SDEBUG(Serial.println(F("COMMAND: oneshot stop")));
      stop=true;
      break;
    case I2C_SDS011_COMMAND_STOP:
      IF_SDEBUG(Serial.println(F("COMMAND: stop")));
      stop=true;
      break;
    case I2C_SDS011_COMMAND_SAVE:
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


  if (stop) {

    // disable interrupts for atomic operation
    noInterrupts();
    //exchange double buffer
    IF_SDEBUG(Serial.println(F("exchange double buffer")));
    i2c_datasettmp=i2c_dataset1;
    i2c_dataset1=i2c_dataset2;
    i2c_dataset2=i2c_datasettmp;
    interrupts();
    // new data published

    IF_SDEBUG(Serial.println(F("clean buffer")));
    uint8_t *ptr;
    //Init to FF i2c_dataset1;
    ptr = (uint8_t *)&i2c_dataset1->pm;
    for (i=0;i<REG_PM_SIZE;i++) { *ptr |= 0xFF; ptr++;}

    stop=false;
  }
  
  if (oneshot) {
    if (!start) return;
  }

  long int timetowait;

  // comment this if you manage continous mode
  // in this case timing is getted from windsonic that send valuer every SAMPLERATE us
  timetowait= SAMPLERATE - (millis() - starttime) ;
  //IF_SDEBUG(Serial.print("elapsed time: "));
  //IF_SDEBUG(Serial.println(millis() - starttime));
  if (timetowait > 0) {
    return;
  }
  else {
    if (timetowait < -10) IF_SDEBUG(Serial.println("WARNIG: timing error , I am late"));    
  }

  starttime = millis()+timetowait;

  if (oneshot) sensor.set_sleep(false);
  delay(1000);
  ok = sensor.query_data_auto(&pm25, &pm10, 3);
  if (oneshot) sensor.set_sleep(true);

  wdt_reset();

  if (ok){

    i2c_dataset1->pm.pm25=pm25;
    i2c_dataset1->pm.pm10=pm10;
    
    IF_SDEBUG(Serial.print("pm25: "));
    IF_SDEBUG(Serial.println(pm25));
    IF_SDEBUG(Serial.print("pm10: "));
    IF_SDEBUG(Serial.println(pm10));
  }
  if (oneshot) {
    //if one shot we have finish
    IF_SDEBUG(Serial.println(F("oneshot end")));
    start=false;    
    return;
  }

  // statistical processing

  // first level mean

  IF_SDEBUG(Serial.print("data in store first: "));
  IF_SDEBUG(Serial.println(nsample1));


  // first level min
  if (minpm25 > pm25) minpm25=pm25;
  if (minpm10 > pm10) minpm10=pm10;

  // first level mean
  meanpm25 += (float(pm25) - meanpm25) / nsample1;
  meanpm10 += (float(pm10) - meanpm10) / nsample1;

  // first level max
  if (maxpm25 < pm25) maxpm25=pm25;
  if (maxpm10 < pm10) maxpm10=pm10;

  // sigma
  sum2pm25+=pm25*pm25;
  sumpm25+=pm25;

  sum2pm10+=pm10*pm10;
  sumpm10+=pm10;

  if (nsample1 == SAMPLE1) {
    IF_SDEBUG(Serial.print("meanpm25: "));
    IF_SDEBUG(Serial.println(meanpm25));
    IF_SDEBUG(Serial.print("meanpm10: "));
    IF_SDEBUG(Serial.println(meanpm10));

    cbpm2560n.autoput(minpm25);
    cbpm1060n.autoput(minpm10);

    cbpm2560m.autoput(meanpm25);
    cbpm1060m.autoput(meanpm10);

    cbpm2560x.autoput(maxpm25);
    cbpm1060x.autoput(maxpm10);

    cbsum2pm25.autoput(sum2pm25);
    cbsumpm25.autoput(sumpm25);

    cbsum2pm10.autoput(sum2pm10);
    cbsumpm10.autoput(sumpm10);

    nsample1=0;

    minpm25=-1;
    meanpm25=0.;
    maxpm25=0.;

    minpm10=-1;
    meanpm10=0.;
    maxpm10=0.;

    sum2pm25=0;
    sumpm25=0;

    sum2pm10=0;
    sumpm10=0;

  }

  // sigma
  float sum260, sum60;

  if (cbsum2pm25.getSize() == cbsum2pm25.getCapacity() && cbsumpm25.getSize() == cbsumpm25.getCapacity()){
    sum260=0;
    for (i=0 ; i < cbsum2pm25.getCapacity() ; i++){
      sum260 += cbsum2pm25.peek(i);
    }

    sum60=0;
    for (i=0 ; i < cbsumpm25.getCapacity() ; i++){
      sum60 += cbsumpm25.peek(i);
    }
	
    i2c_dataset1->pm.sigmapm25=round(sqrt((sum260-(sum60*sum60)/(SAMPLE1*SAMPLE2))/(SAMPLE1*SAMPLE2)))+OFFSET;
      
  }else{
    i2c_dataset1->pm.sigmapm25=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("sigma pm25: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->pm.sigmapm25-OFFSET));


  if (cbsum2pm10.getSize() == cbsum2pm10.getCapacity() && cbsumpm10.getSize() == cbsumpm10.getCapacity()){
    sum260=0;
    for (i=0 ; i < cbsum2pm10.getCapacity() ; i++){
      sum260 += cbsum2pm10.peek(i);
    }

    sum60=0;
    for (i=0 ; i < cbsumpm10.getCapacity() ; i++){
      sum60 += cbsumpm10.peek(i);
    }
	
    i2c_dataset1->pm.sigmapm10=round(sqrt((sum260-(sum60*sum60)/(SAMPLE1*SAMPLE2))/(SAMPLE1*SAMPLE2)))+OFFSET;
      
  }else{
    i2c_dataset1->pm.sigmapm10=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("sigma pm10: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->pm.sigmapm10-OFFSET));


  nsample1++;

  // second level pm25

  IF_SDEBUG(Serial.print("data in store second pm25 min: "));
  IF_SDEBUG(Serial.println(cbpm2560n.getSize()));

  if (cbpm2560n.getSize() == cbpm2560n.getCapacity()){
    min=0;
    for (i=0 ; i < cbpm2560n.getCapacity() ; i++){
      min += (cbpm2560n.peek(i) - min) / (i+1);
    }

    i2c_dataset1->pm.minpm25=round(min)+OFFSET;

  }else{
    i2c_dataset1->pm.minpm25=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("pm25 second min: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->pm.minpm25-OFFSET));


  IF_SDEBUG(Serial.print("data in store second pm25 mean: "));
  IF_SDEBUG(Serial.println(cbpm2560m.getSize()));

  if (cbpm2560m.getSize() == cbpm2560m.getCapacity()){
    mean=0;
    for (i=0 ; i < cbpm2560m.getCapacity() ; i++){
      mean += (cbpm2560m.peek(i) - mean) / (i+1);
    }

    i2c_dataset1->pm.meanpm25=round(mean)+OFFSET;

  }else{
    i2c_dataset1->pm.meanpm25=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("pm25 second mean: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->pm.meanpm25-OFFSET));


  IF_SDEBUG(Serial.print("data in store second pm25 max: "));
  IF_SDEBUG(Serial.println(cbpm2560x.getSize()));

  if (cbpm2560x.getSize() == cbpm2560x.getCapacity()){
    max=0;
    for (i=0 ; i < cbpm2560x.getCapacity() ; i++){
      max += (cbpm2560x.peek(i) - max) / (i+1);
    }

    i2c_dataset1->pm.maxpm25=round(mean)+OFFSET;

  }else{
    i2c_dataset1->pm.maxpm25=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("pm25 second max: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->pm.maxpm25-OFFSET));


  // second level pm10

  IF_SDEBUG(Serial.print("data in store second pm10 min: "));
  IF_SDEBUG(Serial.println(cbpm1060n.getSize()));

  if (cbpm1060n.getSize() == cbpm1060n.getCapacity()){
    min=0;
    for (i=0 ; i < cbpm1060n.getCapacity() ; i++){
      min += (cbpm1060n.peek(i) - min) / (i+1);
    }

    i2c_dataset1->pm.minpm10=round(min)+OFFSET;

  }else{
    i2c_dataset1->pm.minpm10=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("pm10 second min: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->pm.minpm10-OFFSET));


  IF_SDEBUG(Serial.print("data in store second pm10 mean: "));
  IF_SDEBUG(Serial.println(cbpm1060m.getSize()));

  if (cbpm1060m.getSize() == cbpm1060m.getCapacity()){
    mean=0;
    for (i=0 ; i < cbpm1060m.getCapacity() ; i++){
      mean += (cbpm1060m.peek(i) - mean) / (i+1);
    }

    i2c_dataset1->pm.meanpm10=round(mean)+OFFSET;

  }else{
    i2c_dataset1->pm.meanpm10=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("pm10 second mean: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->pm.meanpm10-OFFSET));


  IF_SDEBUG(Serial.print("data in store second pm10 max: "));
  IF_SDEBUG(Serial.println(cbpm1060x.getSize()));

  if (cbpm1060x.getSize() == cbpm1060x.getCapacity()){
    max=0;
    for (i=0 ; i < cbpm1060x.getCapacity() ; i++){
      max += (cbpm1060x.peek(i) - max) / (i+1);
    }

    i2c_dataset1->pm.maxpm10=round(mean)+OFFSET;

  }else{
    i2c_dataset1->pm.maxpm10=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("pm10 second max: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->pm.maxpm10-OFFSET));



  digitalWrite(LEDPIN,!digitalRead(LEDPIN));  // blink Led

  // comment this if you manage continous mode
  // in this case timing is getted from windsonic that send valuer every SAMPLERATE us

}  
