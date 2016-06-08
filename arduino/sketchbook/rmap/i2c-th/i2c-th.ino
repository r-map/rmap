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
 * This program implements temperature and humidity sensors 
 * elaboration exported to i2c interface.
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
#include <SensorDriver.h>
#include <avr/wdt.h>
#include "Wire.h"
#include "registers-th.h"      //Register definitions
#include "config.h"
#include "LongIntBuffer.h"
#include "FloatBuffer.h"

#define REG_MAP_SIZE            sizeof(I2C_REGISTERS)                //size of register map
#define REG_WRITABLE_MAP_SIZE   sizeof(I2C_WRITABLE_REGISTERS)       //size of register map

#define MAX_SENT_BYTES     0x0F   //maximum amount of data that I could receive from a master device (register, plus 15 byte)

#define SENSORS_LEN 2     // number of sensors
#define LENVALUES 2
size_t lenvalues=LENVALUES;       // max of values for sensor
long int values[LENVALUES];

struct sensor_t
{
  char driver[5];         // driver name
  char type[5];           // driver name
  int address;            // i2c address
} sensors[SENSORS_LEN];
SensorDriver* sd[SENSORS_LEN];


LongIntBuffer cbt60mean;
LongIntBuffer cbh60mean;

int pinLed=13;

typedef struct {
  uint8_t    sw_version;       // 0x00  Version of the I2C_GPS sw
} status_t;

typedef struct {
  uint16_t     sample;
  uint16_t     mean60;
  uint16_t     mean;
  uint16_t     max;
  uint16_t     min;
  uint16_t     sigma;
} values_t;

typedef struct {

//Status registers
  status_t     status;         // 0x00  status register

//temperature data
  values_t             temperature;               // 0x01

//humidity data
  values_t             humidity;

} I2C_REGISTERS;


typedef struct {

//sample mode
  bool                  oneshot;                  // one shot active
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

float meanft;
float meanfh;
float sum2;
float sum;
uint8_t nsample1;

// one shot management
static bool start=false;
static bool stop=false;

volatile unsigned long antirimb=0;
unsigned long starttime;


//////////////////////////////////////////////////////////////////////////////////////
// I2C handlers
// Handler for requesting data
//
void requestEvent()
{
  IF_SDEBUG(Serial.print("request event: "));
  IF_SDEBUG(Serial.println(receivedCommands[0]));
  //IF_SDEBUG(Serial.println(*((uint8_t *)(i2c_dataset2)+receivedCommands[0]),HEX));
  //IF_SDEBUG(Serial.println(*((uint8_t *)(i2c_dataset2)+receivedCommands[0]+1),HEX));
  //IF_SDEBUG(Serial.println(*((uint8_t *)(i2c_dataset2)+receivedCommands[0]+2),HEX));
  //IF_SDEBUG(Serial.println(*((uint8_t *)(i2c_dataset2)+receivedCommands[0]+3),HEX));

  Wire.write(((uint8_t *)i2c_dataset2)+receivedCommands[0],4);
  //Write up to 32 byte, since master is responsible for reading and sending NACK
  //32 byte limit is in the Wire library, we have to live with it unless writing our own wire library

}

//Handler for receiving data
void receiveEvent( int bytesReceived)
{

     IF_SDEBUG(Serial.print("receive event, bytes:"));
     IF_SDEBUG(Serial.println(bytesReceived));

     uint8_t  *ptr;
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
       if (receivedCommands[0] == I2C_TH_COMMAND) {
	 //IF_SDEBUG(Serial.print("       received command:"));IF_SDEBUG(Serial.println(receivedCommands[1],HEX));
	 new_command = receivedCommands[1]; return;
	 return;   //Just one byte, ignore all others
       }
     }

     if (bytesReceived == 1){
       //read address for a given register
       //Addressing over the reg_map fallback to first byte
       if((receivedCommands[0] < 0) || (receivedCommands[0] >= REG_MAP_SIZE)) {
	 receivedCommands[0]=0;
       }
       //IF_SDEBUG(Serial.print("       set register:"));IF_SDEBUG(Serial.println(receivedCommands[0],HEX));
       return;
     }

     //More than 1 byte was received, so there is definitely some data to write into a register
     //Check for writeable registers and discard data is it's not writeable

     //IF_SDEBUG(Serial.print("         check  writable buffer:"));
     //IF_SDEBUG(Serial.println(receivedCommands[0]));
     //IF_SDEBUG(Serial.println(I2C_TH_MAP_WRITABLE));
     //IF_SDEBUG(Serial.println(I2C_TH_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE));

     if ((receivedCommands[0]>=I2C_TH_MAP_WRITABLE) && (receivedCommands[0] < (I2C_TH_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE))) {    
       if ((receivedCommands[0]+(unsigned int)(bytesReceived-1)) <= (I2C_TH_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE)) {
	 //Writeable registers
	 ptr = (uint8_t *)i2c_writabledataset1+receivedCommands[0];
	 for (int a = 1; a < bytesReceived; a++) { 
	   //IF_SDEBUG(Serial.print("write in writable buffer:"));IF_SDEBUG(Serial.print(a));IF_SDEBUG(Serial.println(receivedCommands[a],HEX));
	   *ptr++ = receivedCommands[a];
	 }

	 /*
	 // the two buffer should be in sync
	 ptr = (uint8_t *)i2c_writabledataset2+receivedCommands[0];
	 for (int a = 1; a < bytesReceived; a++) { *ptr++ = receivedCommands[a]; }
	 // new data written


	 IF_SDEBUG(Serial.println("writable buffer exchange"));
	 // disable interrupts for atomic operation
	 noInterrupts();
	 //exchange double buffer
	 i2c_writabledatasettmp=i2c_writabledataset1;
	 i2c_writabledataset1=i2c_writabledataset2;
	 i2c_writabledataset2=i2c_writabledatasettmp;
	 interrupts();
	 */

       }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void mgr_command(){

  static uint8_t _command;

  //Check for new incoming command on I2C
  if (new_command!=0) {
    _command = new_command;                                                   //save command byte for processing
    new_command = 0;                                                          //clear it
    //_command = _command & 0x0F;                                               //empty 4MSB bits   
    switch (_command) {
    case I2C_TH_COMMAND_ONESHOT_START:
      IF_SDEBUG(Serial.println("COMMAND: oneshot start"));
      start=true;
      break;          
    case I2C_TH_COMMAND_ONESHOT_STOP:
      IF_SDEBUG(Serial.println("COMMAND: oneshot stop"));
      stop=true;
      break;
    case I2C_TH_COMMAND_STOP:
      IF_SDEBUG(Serial.println("COMMAND: stop"));
      stop=true;      
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
 
    // resets the buffer into an original state (with no data)	
    cbt60mean.clear();
    cbh60mean.clear();

   stop=false;
  }
}



void setup() {

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


  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"ADT");
  sensors[0].address=TEMPERATURE_ADDRESS;

  strcpy(sensors[1].driver,"I2C");
  strcpy(sensors[1].type,"HIH");
  sensors[1].address=HUMIDITY_ADDRESS;


  // inizialize double buffer
  i2c_dataset1=&i2c_buffer1;
  i2c_dataset2=&i2c_buffer2;

  // inizialize writable double buffer
  i2c_writabledataset1=&i2c_writablebuffer1;
  i2c_writabledataset2=&i2c_writablebuffer2;

  meanft=0.;
  meanfh=0.;
  nsample1=1;

#define SAMPLE1 60000/SAMPLERATE
#define SAMPLE2 180

  cbt60mean.init(SAMPLE2);
  cbh60mean.init(SAMPLE2);

  IF_SDEBUG(Serial.begin(115200));        // connect to the serial port

  IF_SDEBUG(Serial.println(F("Start")));
  IF_SDEBUG(Serial.println(F("i2c_dataset 1&2 set to 1")));

  uint8_t *ptr;
  //Init to FF i2c_dataset1;
  ptr = (uint8_t *)i2c_dataset1;
  for (int i=0;i<REG_MAP_SIZE;i++) { *ptr |= 0xFF; ptr++;}

  //Init to FF i2c_dataset1;
  ptr = (uint8_t *)i2c_dataset2;
  for (int i=0;i<REG_MAP_SIZE;i++) { *ptr |= 0xFF; ptr++;}



  IF_SDEBUG(Serial.println(F("i2c_writabledataset 1&2 set to 1")));
  //Init to FF i2c_writabledataset1;
  ptr = (uint8_t *)i2c_writabledataset1;
  for (int i=0;i<REG_WRITABLE_MAP_SIZE;i++) { *ptr |= 0xFF; ptr++;}

  //Init to FF i2c_writabledataset2;
  ptr = (uint8_t *)i2c_writabledataset2;
  for (int i=0;i<REG_WRITABLE_MAP_SIZE;i++) { *ptr |= 0xFF; ptr++;}


  //Set up default parameters
  i2c_dataset1->status.sw_version          = VERSION;
  i2c_dataset2->status.sw_version          = VERSION;

  // set default for oneshot
  i2c_writabledataset1->oneshot=false;
  i2c_writabledataset2->oneshot=false;

  //Start I2C communication routines
  Wire.begin(I2C_TH_ADDRESS);

  // set the frequency 
  #define I2C_CLOCK 50000
  
  //set the i2c clock 
  TWBR = ((F_CPU / I2C_CLOCK) - 16) / 2;

  Wire.onRequest(requestEvent);          // Set up event handlers
  Wire.onReceive(receiveEvent);


  for (int i = 0; i < SENSORS_LEN; i++) {

    sd[i]=SensorDriver::create(sensors[i].driver,sensors[i].type);
    if (sd[i] == NULL){
      IF_SDEBUG(Serial.print(sensors[i].driver));
      IF_SDEBUG(Serial.print(F("-")));
      IF_SDEBUG(Serial.print(sensors[i].type));
      IF_SDEBUG(Serial.println(F(": driver not created !")));
    }else{
      sd[i]->setup(sensors[i].driver,sensors[i].address);
    }
  }

  starttime = millis();

  IF_SDEBUG(Serial.println(F("end setup")));

}

void loop() {

  unsigned long int t;
  unsigned long int h;
  float mean;
  unsigned long int maxv,minv;
  long unsigned int value;

  uint8_t i;

  wdt_reset();

  mgr_command();

  if (i2c_writabledataset2->oneshot) {
    if (! start) return;
  }


  long int timetowait;
  long unsigned int waittime,maxwaittime=0;

  timetowait= SAMPLERATE - (millis() - starttime) ;
  //IF_SDEBUG(Serial.print("elapsed time: "));
  //IF_SDEBUG(Serial.println(millis() - starttime));
  if (timetowait > 0) {
    return;
  }
  else {
    if (timetowait < -10) IF_SDEBUG(Serial.print("WARNIG: timing error , I am late"));    
  }

  starttime = millis()+timetowait;

  // prepare sensors to measure
  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == NULL){
      if (sd[i]->prepare(waittime) == SD_SUCCESS){
	maxwaittime=max(maxwaittime,waittime);
      }else{
	IF_SDEBUG(Serial.print(sensors[i].driver));
	IF_SDEBUG(Serial.println(": prepare failed !"));
      }
    }
  }

  //wait sensors to go ready
  IF_SDEBUG(Serial.print("# wait sensors for ms:");  Serial.println(maxwaittime));

  //delay(maxwaittime);

  long unsigned int inittime = millis();
  while ((inittime + maxwaittime) > millis()){
    mgr_command();
  }


  t=ULONG_MAX;
  h=ULONG_MAX;

  for (int i = 0; i < SENSORS_LEN; i++) 
    {
      if (!sd[i] == NULL){
	
	// get integers values       
	for (int ii = 0; ii < lenvalues; ii++) {
	  values[ii]=LONG_MAX;
	}
	
	IF_SDEBUG(Serial.println(sensors[i].type));
	if (sd[i]->get(values,lenvalues) == SD_SUCCESS){
	  for (int ii = 0; ii < lenvalues; ii++) {
	    IF_SDEBUG(Serial.print(F("value read: ")));IF_SDEBUG(Serial.println(values[ii]));
	  }
	  if (strcmp(sensors[i].type,"ADT") == 0) t=values[0];
	  if (strcmp(sensors[i].type,"HIH") == 0) h=values[0];
	}else{
	  IF_SDEBUG(Serial.println(F("Error")));
	}
      }
    }


  i2c_dataset1->temperature.sample=t;
  i2c_dataset1->humidity.sample=h;

  IF_SDEBUG(Serial.print(F("temperature: ")));
  IF_SDEBUG(Serial.println(i2c_dataset1->temperature.sample));
  IF_SDEBUG(Serial.print(F("humidity: ")));
  IF_SDEBUG(Serial.println(i2c_dataset1->humidity.sample));

  if (i2c_writabledataset2->oneshot) {
    //if one shot we have finish
    IF_SDEBUG(Serial.println(F("oneshot end")));
    start=false;    
    return;
  }

  // statistical processing

  // first level mean:   sample => observation

  IF_SDEBUG(Serial.print("data in store first: "));
  IF_SDEBUG(Serial.println(nsample1));

  // temperature and humidity mean

  meanft += (float(t) - meanft) / nsample1;
  meanfh += (float(h) - meanfh) / nsample1;

  if (nsample1 == SAMPLE1) {
    IF_SDEBUG(Serial.print("T mean: "));
    IF_SDEBUG(Serial.println(meanft));
    IF_SDEBUG(Serial.print("H mean: "));
    IF_SDEBUG(Serial.println(meanfh));

    cbt60mean.autoput(round(meanft));
    cbh60mean.autoput(round(meanfh));

    i2c_dataset1->temperature.mean60=round(meanft);
    i2c_dataset1->humidity.mean60=round(meanfh);

    meanft=0.;
    meanfh=0.;
    nsample1=0;

  }

  mean=0.;
  maxv=0;
  minv= ULONG_MAX;
  for (i=0 ; i < cbt60mean.getSize() ; i++){
    value=cbt60mean.peek(i);
    mean += float(value - mean) / (i+1);
    maxv = max(maxv, value);
    minv = min(minv, value);
    
  }
  if (cbt60mean.getSize() >= MINUTEFORREPORT)
    {
      i2c_dataset1->temperature.mean=round(mean);
      i2c_dataset1->temperature.max=maxv;
      i2c_dataset1->temperature.min=minv;
    }

  mean=0;
  maxv=0;
  minv= ULONG_MAX;
  for (i=0 ; i < cbh60mean.getSize() ; i++){
    value=cbh60mean.peek(i);
    mean += float(value - mean) / (i+1);
    maxv = max(maxv, value);
    minv = min(minv, value);
  }
  if (cbt60mean.getSize() >= MINUTEFORREPORT)
    {
      i2c_dataset1->humidity.mean=round(mean);
      i2c_dataset1->humidity.max=maxv;
      i2c_dataset1->humidity.min=minv;
    }

  // sigma
  sum2=0;
  sum=0;
  unsigned short int n=cbt60mean.getSize();
  for (i=0 ; i < n ; i++){
    sum2 += cbt60mean.peek(i)*cbt60mean.peek(i);
    sum += cbt60mean.peek(i);
  }
  if (cbt60mean.getSize() >= MINUTEFORREPORT)
    {
      i2c_dataset1->temperature.sigma=round(sqrt((sum2-(sum*sum)/n)/n))+OFFSET;
    }

  sum2=0;
  sum=0;
  n=cbh60mean.getSize();
  for (i=0 ; i < n ; i++){
    sum2 += cbh60mean.peek(i)*cbh60mean.peek(i);
    sum += cbh60mean.peek(i);
  }
  if (cbt60mean.getSize() >= MINUTEFORREPORT)
    {
      i2c_dataset1->humidity.sigma=round(sqrt((sum2-(sum*sum)/n)/n))+OFFSET;
    }

  IF_SDEBUG(Serial.print("T mean  second order: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->temperature.mean));
  IF_SDEBUG(Serial.print("T max   second order: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->temperature.max));
  IF_SDEBUG(Serial.print("T min   second order: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->temperature.min));
  IF_SDEBUG(Serial.print("T sigma second order: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->temperature.sigma));
  
  IF_SDEBUG(Serial.print("H mean second order: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->humidity.mean));
  IF_SDEBUG(Serial.print("H max second order: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->humidity.max));
  IF_SDEBUG(Serial.print("H min   second order: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->humidity.min));
  IF_SDEBUG(Serial.print("H sigma second order: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->humidity.sigma));


  nsample1++;


  digitalWrite(pinLed,!digitalRead(pinLed));  // blink Led

}  
