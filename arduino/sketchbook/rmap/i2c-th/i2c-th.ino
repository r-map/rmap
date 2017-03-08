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

#define VERSION 02             //Software version for cross checking
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

#include "EEPROMAnything.h"

#define REG_MAP_SIZE            sizeof(I2C_REGISTERS)                //size of register map
#define REG_TH_SIZE           sizeof(values_t)                  //size of register map for th
#define REG_WRITABLE_MAP_SIZE   sizeof(I2C_WRITABLE_REGISTERS)       //size of register map

#define MAX_SENT_BYTES     0x0F   //maximum amount of data that I could receive from a master device (register, plus 15 byte)

char confver[9] = CONFVER; // version of configuration saved on eeprom

#define SENSORS_LEN           SENSORS_COUNT     // number of sensors
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
  uint8_t               i2c_address;              // i2c bus address (short unsigned int)
  uint8_t               i2c_temperature_address ; // i2c bus address of temperature sensor (short unsigned int)
  uint8_t               i2c_humidity_address ;    // i2c bus address of humidity sensor (short unsigned int)
  void save (int* p) volatile {                            // save to eeprom

    IF_SDEBUG(Serial.print(F("oneshot: "))); IF_SDEBUG(Serial.println(oneshot));
    IF_SDEBUG(Serial.print(F("i2c address: "))); IF_SDEBUG(Serial.println(i2c_address));
    IF_SDEBUG(Serial.print(F("i2c temperature address: "))); IF_SDEBUG(Serial.println(i2c_temperature_address));
    IF_SDEBUG(Serial.print(F("i2c humidity address: ")));    IF_SDEBUG(Serial.println(i2c_humidity_address));

    *p+=EEPROM_writeAnything(*p, oneshot);
    *p+=EEPROM_writeAnything(*p, i2c_address);
    *p+=EEPROM_writeAnything(*p, i2c_temperature_address);
    *p+=EEPROM_writeAnything(*p, i2c_humidity_address);
  }
  
  void load (int* p) volatile {                            // load from eeprom
    *p+=EEPROM_readAnything(*p, oneshot);
    *p+=EEPROM_readAnything(*p, i2c_address);
    *p+=EEPROM_readAnything(*p, i2c_temperature_address);
    *p+=EEPROM_readAnything(*p, i2c_humidity_address);
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

float meanft;
float meanfh;
float sum2;
float sum;
uint8_t nsamplet,nsampleh,nsample1;
int lastsize=-1;

// one shot management
static bool oneshot;
static bool start=false;
static bool stop=false;

unsigned long starttime, regsettime;
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

    Wire.write(((uint8_t *)i2c_dataset2)+receivedCommands[0],4);
    //Write up to 4 byte, since master is responsible for reading and sending NACK
    //32 byte limit is in the Wire library, we have to live with it unless writing our own wire library
  }
  regsettime=0;
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
       if (receivedCommands[0] == I2C_TH_COMMAND) {
	 //IF_SDEBUG(Serial.print("       received command:"));IF_SDEBUG(Serial.println(receivedCommands[1],HEX));
	 new_command = receivedCommands[1]; return; }
     }

     if (bytesReceived == 1){
       //read address for a given register
       //Addressing over the reg_map fallback to first byte
       if(bytesReceived == 1 && ( (receivedCommands[0] < 0) || (receivedCommands[0] >= REG_MAP_SIZE))) {
	 receivedCommands[0]=0;
	 regsettime=0;
       }else{
	 regsettime=millis();
       }
       //IF_SDEBUG(Serial.print("set register:"));IF_SDEBUG(Serial.println(receivedCommands[0]));
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
	 // the two buffer should be in sync
	 ptr1 = (uint8_t *)i2c_writabledataset1+receivedCommands[0]-I2C_TH_MAP_WRITABLE;
	 ptr2 = (uint8_t *)i2c_writabledataset2+receivedCommands[0]-I2C_TH_MAP_WRITABLE;
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
    case I2C_TH_COMMAND_ONESHOT_START:
      IF_SDEBUG(Serial.println("COMMAND: oneshot start"));
      start=true;
      break;          
    case I2C_TH_COMMAND_ONESHOT_STOP:
      IF_SDEBUG(Serial.println("COMMAND: oneshot stop"));
      stop=true;
      break;
    case I2C_TH_COMMAND_START:
      IF_SDEBUG(Serial.println("COMMAND: start"));
      start=true;
      starttime = millis();
      break;
    case I2C_TH_COMMAND_STOP:
      IF_SDEBUG(Serial.println("COMMAND: stop"));
      stop=true;
      start=false;
      break;
    case I2C_TH_COMMAND_STOP_START:
      IF_SDEBUG(Serial.println("COMMAND: stop and start"));
      stop=true;      
      start=true;
      starttime = millis();
      break;
    case I2C_TH_COMMAND_SAVE:
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
 
    // resets the buffer into an original state (with no data)	
    cbt60mean.clear();
    cbh60mean.clear();

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

  regsettime=0;

  // inizialize double buffer
  i2c_dataset1=&i2c_buffer1;
  i2c_dataset2=&i2c_buffer2;

  // inizialize writable double buffer
  i2c_writabledataset1=&i2c_writablebuffer1;
  i2c_writabledataset2=&i2c_writablebuffer2;

  meanft=0.;
  meanfh=0.;
  nsamplet=0;
  nsampleh=0;
  nsample1=0;

#define SAMPLE1 60000/SAMPLERATE
#define SAMPLE2 180

  cbt60mean.init(SAMPLE2);
  cbh60mean.init(SAMPLE2);


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
      i2c_writabledataset2->i2c_temperature_address=i2c_writabledataset1->i2c_temperature_address;
      i2c_writabledataset2->i2c_humidity_address=i2c_writabledataset1->i2c_humidity_address;
    }
  else
    {
      IF_SDEBUG(Serial.println(F("EEPROM data not useful or set pin activated")));
      IF_SDEBUG(Serial.println(F("set default values for writable registers")));
      // set default to oneshot
      i2c_writabledataset1->oneshot=false;
      i2c_writabledataset2->oneshot=false;
      i2c_writabledataset1->i2c_address = I2C_TH_DEFAULTADDRESS;
      i2c_writabledataset2->i2c_address = I2C_TH_DEFAULTADDRESS;
      i2c_writabledataset1->i2c_temperature_address = TEMPERATURE_DEFAULTADDRESS;
      i2c_writabledataset2->i2c_temperature_address = TEMPERATURE_DEFAULTADDRESS;
      i2c_writabledataset1->i2c_humidity_address = HUMIDITY_DEFAULTADDRESS;
      i2c_writabledataset2->i2c_humidity_address = HUMIDITY_DEFAULTADDRESS;
    }

  IF_SDEBUG(Serial.print(F("i2c address: ")));
  IF_SDEBUG(Serial.println(i2c_writabledataset1->i2c_address));
  IF_SDEBUG(Serial.print(F("i2c temperature address: ")));
  IF_SDEBUG(Serial.println(i2c_writabledataset1->i2c_temperature_address));
  IF_SDEBUG(Serial.print(F("i2c humidity address: ")));
  IF_SDEBUG(Serial.println(i2c_writabledataset1->i2c_humidity_address));
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

  unsigned char sensors_count = 0;

  #if (USE_SENSORS_ADT == 1)
    strcpy(sensors[sensors_count].driver,"I2C");
    strcpy(sensors[sensors_count].type,"ADT");
    sensors[sensors_count].address=i2c_writabledataset1->i2c_temperature_address;
    sensors_count++;
  #endif

  #if (USE_SENSORS_HIH == 1)
    strcpy(sensors[sensors_count].driver,"I2C");
    strcpy(sensors[sensors_count].type,"HIH");
    sensors[sensors_count].address=i2c_writabledataset1->i2c_humidity_address;
    sensors_count++;
  #endif

  #if (USE_SENSORS_HYT == 1)
    strcpy(sensors[sensors_count].driver,"I2C");
    strcpy(sensors[sensors_count].type,"HYT");
    sensors[sensors_count].address=i2c_writabledataset1->i2c_temperature_address;
    sensors_count++;
  #endif

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

  long int t;
  long int h;
  float mean;
  long int maxv,minv;
  long int value;

  uint8_t i;

  wdt_reset();

  mgr_command();

  if (! start) return;

  long int timetowait;
  long unsigned int waittime,maxwaittime=0;

  timetowait= SAMPLERATE - (millis() - starttime) ;
  //IF_SDEBUG(Serial.print("elapsed time: "));
  //IF_SDEBUG(Serial.println(millis() - starttime));
  if (timetowait > 0) {
    return;
  }
  else {
    if (timetowait < -10) {
      IF_SDEBUG(Serial.print(F("WARNIG: timing error , I am late ")));    
      IF_SDEBUG(Serial.println(timetowait));    
    }
  }

  //starttime = millis()+timetowait;
  starttime += SAMPLERATE;

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


  t=LONG_MAX;
  h=LONG_MAX;

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
    if (strcmp(sensors[i].type,"HYT") == 0) {
      h = values[0];
      t = values[1];
    }
	}else{
	  //IF_SDEBUG(Serial.println(F("Error: RETRY")));
	  //delay(20);
	  //if (sd[i]->get(values,lenvalues) == SD_SUCCESS){
	  //  for (int ii = 0; ii < lenvalues; ii++) {
	  //    IF_SDEBUG(Serial.print(F("value read: ")));IF_SDEBUG(Serial.println(values[ii]));
	  //  }
	  //  if (strcmp(sensors[i].type,"ADT") == 0) t=values[0];
	  //  if (strcmp(sensors[i].type,"HIH") == 0) h=values[0];
	  //}else{
	    IF_SDEBUG(Serial.println(F("Error")));
	  //}
	}
      }
    }
  

  i2c_dataset1->temperature.sample=t;
  i2c_dataset1->humidity.sample=h;

  IF_SDEBUG(Serial.print(F("temperature: ")));
  IF_SDEBUG(Serial.println(i2c_dataset1->temperature.sample));
  IF_SDEBUG(Serial.print(F("humidity: ")));
  IF_SDEBUG(Serial.println(i2c_dataset1->humidity.sample));

  if (oneshot) {
    //if one shot we have finish
    IF_SDEBUG(Serial.println(F("oneshot end")));
    start=false;    
    return;
  }

  ///////////////////////////////////////////////
  // statistical processing
  ///////////////////////////////////////////////

  // first level mean:   sample => observation

  nsample1++;

  // temperature and humidity mean

  if (t != LONG_MAX) {	
    nsamplet++;
    meanft += (float(t) - meanft) / nsamplet;
  }
  if (h != LONG_MAX) {	
    nsampleh++;
    meanfh += (float(h) - meanfh) / nsampleh;
  }

  IF_SDEBUG(Serial.print("data in store first: "));
  IF_SDEBUG(Serial.print(nsample1));
  IF_SDEBUG(Serial.print(" : "));
  IF_SDEBUG(Serial.print(nsamplet));
  IF_SDEBUG(Serial.print(" : "));
  IF_SDEBUG(Serial.println(nsampleh));

  // after 1 minute store it in circular buffer and expose to i2c
  if (nsample1 == SAMPLE1) {

    if (nsamplet < (nsample1-MAXMISSING)){
      i2c_dataset1->temperature.mean60=UINT_MAX;
      cbt60mean.autoput(LONG_MAX);
    }else{
      i2c_dataset1->temperature.mean60=round(meanft);
      cbt60mean.autoput(i2c_dataset1->temperature.mean60);
    }


    if (nsampleh < (nsample1-MAXMISSING)){
      i2c_dataset1->humidity.mean60=UINT_MAX;
      cbh60mean.autoput(LONG_MAX);
    }else{
      i2c_dataset1->humidity.mean60=round(meanfh);
      cbh60mean.autoput(i2c_dataset1->humidity.mean60);
    }

    IF_SDEBUG(Serial.print("T mean: "));
    IF_SDEBUG(Serial.println(i2c_dataset1->temperature.mean60));
    IF_SDEBUG(Serial.print("H mean: "));
    IF_SDEBUG(Serial.println(i2c_dataset1->humidity.mean60));

    meanft=0.;
    meanfh=0.;
    nsamplet=0;
    nsampleh=0;
    nsample1=0;

  }

  // second level statistical processing

  if (cbt60mean.getSize() != lastsize)  {

    lastsize=cbt60mean.getSize();
    
    //temperature

    if (cbt60mean.getSize() >= MINUTEFORREPORT) {
      
      int ndata=0;
      mean=0.;
      maxv=0;
      minv= LONG_MAX;
      for (i=0 ; i < cbt60mean.getSize() ; i++){
	value=cbt60mean.peek(i);
	
	if (value != LONG_MAX) {	
	  ndata++;
	  IF_SDEBUG(Serial.print("cbt60mean: "));
	  IF_SDEBUG(Serial.print(i));
	  IF_SDEBUG(Serial.print(" : "));
	  IF_SDEBUG(Serial.println(value));
	  
	  mean += float(value - mean) / (i+1);
	  maxv = max(maxv, value);
	  minv = min(minv, value);
	}
	
      }
      
      if (ndata >= MINUTEFORREPORT) {
	i2c_dataset1->temperature.mean=round(mean);
	i2c_dataset1->temperature.max=maxv;
	i2c_dataset1->temperature.min=minv;
      }	else{
	i2c_dataset1->temperature.mean=UINT_MAX;
	i2c_dataset1->temperature.max=UINT_MAX;
	i2c_dataset1->temperature.min=UINT_MAX;	
      }

      // sigma temperature
      if (ndata >= MINUTEFORREPORT) {
	sum2=0;
	sum=0;
	unsigned short int n=cbt60mean.getSize();
	for (i=0 ; i < n ; i++){
	  sum2 += cbt60mean.peek(i)*cbt60mean.peek(i);
	  sum += cbt60mean.peek(i);
	}
	i2c_dataset1->temperature.sigma=round(sqrt((sum2-(sum*sum)/n)/n))+OFFSET;
      }else{
	i2c_dataset1->temperature.sigma=UINT_MAX;
      }
    }

    //humidity
      
    if (cbh60mean.getSize() >= MINUTEFORREPORT) {
    
      int ndata=0;
      mean=0;
      maxv=0;
      minv= LONG_MAX;
      for (i=0 ; i < cbh60mean.getSize() ; i++){
	value=cbh60mean.peek(i);
	
	if (value != LONG_MAX) {	
	  ndata++;
	  IF_SDEBUG(Serial.print("cbh60mean: "));
	  IF_SDEBUG(Serial.print(i));
	  IF_SDEBUG(Serial.print(" : "));
	  IF_SDEBUG(Serial.println(value));
	  
	  mean += float(value - mean) / (i+1);
	  maxv = max(maxv, value);
	  minv = min(minv, value);
	}
      }	
      if (ndata >= MINUTEFORREPORT) {
	i2c_dataset1->humidity.mean=round(mean);
	i2c_dataset1->humidity.max=maxv;
	i2c_dataset1->humidity.min=minv;
      }	else{
	i2c_dataset1->humidity.mean=UINT_MAX;
	i2c_dataset1->humidity.max=UINT_MAX;
	i2c_dataset1->humidity.min=UINT_MAX;	
      }

      // sigma humidity
    
      if (ndata >= MINUTEFORREPORT) {
	sum2=0;
	sum=0;
	unsigned short int n=cbh60mean.getSize();
	for (i=0 ; i < n ; i++){
	  sum2 += cbh60mean.peek(i)*cbh60mean.peek(i);
	  sum += cbh60mean.peek(i);
	}
	i2c_dataset1->humidity.sigma=round(sqrt((sum2-(sum*sum)/n)/n))+OFFSET;
      }else{
	i2c_dataset1->humidity.sigma=UINT_MAX;
      }

    }
    
    
    IF_SDEBUG(Serial.print(F("T mean  second order: ")));
    IF_SDEBUG(Serial.println(i2c_dataset1->temperature.mean));
    IF_SDEBUG(Serial.print(F("T max   second order: ")));
    IF_SDEBUG(Serial.println(i2c_dataset1->temperature.max));
    IF_SDEBUG(Serial.print(F("T min   second order: ")));
    IF_SDEBUG(Serial.println(i2c_dataset1->temperature.min));
    IF_SDEBUG(Serial.print(F("T sigma second order: ")));
    IF_SDEBUG(Serial.println(i2c_dataset1->temperature.sigma));
    
    IF_SDEBUG(Serial.print(F("H mean second order: ")));
    IF_SDEBUG(Serial.println(i2c_dataset1->humidity.mean));
    IF_SDEBUG(Serial.print(F("H max second order: ")));
    IF_SDEBUG(Serial.println(i2c_dataset1->humidity.max));
    IF_SDEBUG(Serial.print(F("H min   second order: ")));
    IF_SDEBUG(Serial.println(i2c_dataset1->humidity.min));
    IF_SDEBUG(Serial.print(F("H sigma second order: ")));
    IF_SDEBUG(Serial.println(i2c_dataset1->humidity.sigma));
  }

  digitalWrite(LEDPIN,!digitalRead(LEDPIN));  // blink Led

}
