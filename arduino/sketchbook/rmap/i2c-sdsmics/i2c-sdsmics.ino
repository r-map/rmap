/**********************************************************************
Copyright (C) 2017  Paolo Paruno <p.patruno@iperbole.bologna.it>
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

elaborazioni:

a) eseguo 3 misure di fila (1 al secondo per 3 secondi) e butto via il
minimo e il massimo e tengo il valore "centrale" (internal to Sds011 library)

b) ogni 6 secondi memorizzo questo valore

c) ogni 60 secondi faccio media minimo massimo e deviazione standard

d) ogni 60 secondi si elaborano i valori orari della media, minimo,
massimo e deviazione standard dai valori sul periodo di 60 sec, ma che
equivale a farlo sui dati a) 

il sensore ha due modalità di funzionamento:

1) oneshot; dico fai una miura e mi vengono dati i dati a) (poi farò
conti come e quando voglio) posso fare una misura ogni 3 secondi circa

2)continuo: mi vengono forniti a richiesta i valori b) e d)

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

#include <limits.h>
#include <avr/wdt.h>
#include "Wire.h"
#include "registers-sdsmics.h"         //Register definitions
#include "config.h"
#include "IntBuffer.h"
#include "FloatBuffer.h"

#ifdef SDS011PRESENT
#include "Sds011.h"
#endif
#ifdef MICS4514PRESENT
#include "Mics4514.h"
#endif

#include "EEPROMAnything.h"
#include "Calibration.h"

#define REG_MAP_SIZE            sizeof(I2C_REGISTERS)       //size of register map
#define REG_PM_SIZE           sizeof(pm_t)                  //size of register map for pm
#define REG_WRITABLE_MAP_SIZE   sizeof(I2C_WRITABLE_REGISTERS)       //size of register map

#define MAX_SENT_BYTES     0x0F                      //maximum amount of data that I could receive from a master device (register, plus 15 byte)

char confver[9] = CONFVER; // version of configuration saved on eeprom

#ifdef SDS011PRESENT
sds011::Sds011 sensor(SERIALSDS011);
#endif
#ifdef MICS4514PRESENT
mics4514::Mics4514 sensormics(COPIN,NO2PIN,HEATERPIN,SCALE1PIN,SCALE2PIN);
float coconcentrations[]  = { POINT1_PPM_CO, POINT2_PPM_CO, POINT3_PPM_CO };
float coresistences[]     = { POINT1_RES_CO, POINT2_RES_CO, POINT3_RES_CO };

float no2concentrations[] = {POINT1_PPM_NO2, POINT2_PPM_NO2, POINT3_PPM_NO2};
float no2resistences[]    = {POINT1_RES_NO2, POINT2_RES_NO2, POINT3_RES_NO2};

// NO2 Sensor calibration
calibration::Calibration NO2Cal;

// CO Sensor calibration
calibration::Calibration COCal;

#endif

#ifdef SDS011PRESENT
IntBuffer cbpm2560n;
IntBuffer cbpm1060n;
IntBuffer cbpm2560m;
IntBuffer cbpm1060m;
IntBuffer cbpm2560x;
IntBuffer cbpm1060x;
FloatBuffer cbsum2pm25;
FloatBuffer cbsum2pm10;
FloatBuffer cbsumpm25;
FloatBuffer cbsumpm10;
#endif

#ifdef MICS4514PRESENT
IntBuffer cbco60n;
IntBuffer cbno260n;
IntBuffer cbco60m;
IntBuffer cbno260m;
IntBuffer cbco60x;
IntBuffer cbno260x;
FloatBuffer cbsum2co;
FloatBuffer cbsumco;
FloatBuffer cbsum2no2;
FloatBuffer cbsumno2;
#endif

typedef struct {
  uint8_t    sw_version;                          // Version of the I2C_SDS011 sw
} status_t;

typedef struct {
  uint16_t     pm25;
  uint16_t     pm10;
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
  uint16_t     co;
  uint16_t     no2;
  uint16_t     minco;
  uint16_t     minno2;
  uint16_t     meanco;
  uint16_t     meanno2;
  uint16_t     maxco;
  uint16_t     maxno2;
  uint16_t     sigmaco;
  uint16_t     sigmano2;
} cono2_t;

typedef struct {

//Status registers
  status_t     status;                   // 0x00  status register

//data
  pm_t                pm;                     // 0x01 pm
  cono2_t             cono2;                  // 0x15 pm
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

#ifdef SDS011PRESENT
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
#endif

#ifdef MICS4514PRESENT
float meanco;
float meanno2;
long int minco;
long int minno2;
long int maxco;
long int maxno2;
float sum2co;
float sumco;
float sum2no2;
float sumno2;
#endif

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
       if (receivedCommands[0] == I2C_SDSMICS_COMMAND) {
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
     
     if ((receivedCommands[0]>=I2C_SDSMICS_MAP_WRITABLE) && (receivedCommands[0] < (I2C_SDSMICS_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE))) {    
       if ((receivedCommands[0]+(unsigned int)(bytesReceived-1)) <= (I2C_SDSMICS_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE)) {
	 //Writeable registers
	 // the two buffer should be in sync
	 ptr1 = (uint8_t *)i2c_writabledataset1+receivedCommands[0]-I2C_SDSMICS_MAP_WRITABLE;
	 ptr2 = (uint8_t *)i2c_writabledataset2+receivedCommands[0]-I2C_SDSMICS_MAP_WRITABLE;
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
#define SAMPLE2 60

#ifdef SDS011PRESENT
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
#endif
  
#ifdef MICS4514PRESENT
  meanco=0.;
  meanno2=0.;

  maxco=-1;
  maxno2=-1;

  sum2co=0;
  sumco=0;

  sum2no2=0;
  sumno2=0;

  minco=LONG_MAX;
  minno2=LONG_MAX;

  NO2Cal.setCalibrationPoints(no2resistences, no2concentrations, no2numPoints);
  COCal.setCalibrationPoints(coresistences, coconcentrations, conumPoints);

#endif

  nsample1=1;

#ifdef SDS011PRESENT
  cbpm2560n.init(SAMPLE2);
  cbpm1060n.init(SAMPLE2);

  cbpm2560m.init(SAMPLE2);
  cbpm1060m.init(SAMPLE2);

  cbpm2560x.init(SAMPLE2);
  cbpm1060x.init(SAMPLE2);

  cbsum2pm25.init(SAMPLE2);
  cbsum2pm10.init(SAMPLE2);

  cbsumpm25.init(SAMPLE2);
  cbsumpm10.init(SAMPLE2);
#endif

#ifdef MICS4514PRESENT
  cbco60n.init(SAMPLE2);
  cbno260n.init(SAMPLE2);

  cbco60m.init(SAMPLE2);
  cbno260m.init(SAMPLE2);

  cbco60x.init(SAMPLE2);
  cbno260x.init(SAMPLE2);

  cbsum2co.init(SAMPLE2);
  cbsumco.init(SAMPLE2);

  cbsum2no2.init(SAMPLE2);
  cbsumno2.init(SAMPLE2);
#endif

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
      i2c_writabledataset1->i2c_address = I2C_SDSMICS_DEFAULTADDRESS;
      i2c_writabledataset2->i2c_address = I2C_SDSMICS_DEFAULTADDRESS;
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

#ifdef SDS011PRESENT
  SERIALSDS011.begin(9600);
  sensor.set_mode(sds011::QUERY);
#endif
  
  if (oneshot){
#ifdef SDS011PRESENT
    sensor.set_sleep(true);
#endif
#ifdef MICS4514PRESENT
    sensormics.sleep();
#endif    
  }else{
#ifdef MICS4514PRESENT
    sensormics.fast_heat();
#endif    
  }
  starttime = millis()+SAMPLERATE;

  IF_SDEBUG(Serial.println(F("end setup")));

}

void loop() {

  static uint8_t _command;
  //unsigned int pm25;
  //unsigned int pm10;
  int pm25;
  int pm10;
  int co;
  int no2;
  
  float mean;
  
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
    case I2C_SDSMICS_COMMAND_ONESHOT_START:
      IF_SDEBUG(Serial.println(F("COMMAND: oneshot start")));
      start=true;
      break;          
    case I2C_SDSMICS_COMMAND_ONESHOT_STOP:
      IF_SDEBUG(Serial.println(F("COMMAND: oneshot stop")));
      stop=true;
      break;
    case I2C_SDSMICS_COMMAND_STOP:
      IF_SDEBUG(Serial.println(F("COMMAND: stop")));
      stop=true;
      break;
    case I2C_SDSMICS_COMMAND_SAVE:
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
  } else  {
    // comment this if you manage continous mode
    // in this case timing is getted from sensor that send valuer every SAMPLERATE us
    long int timetowait= SAMPLERATE - (millis() - starttime) ;
    //IF_SDEBUG(Serial.print("elapsed time: "));
    //IF_SDEBUG(Serial.println(millis() - starttime));
    if (timetowait > 0) {
      return;
    }

    if (timetowait < -10) IF_SDEBUG(Serial.println("WARNIG: timing error , I am late"));    
    starttime = millis()+timetowait;

  }


  if (oneshot){
#ifdef SDS011PRESENT
    sensor.set_sleep(false);
#endif
#ifdef MICS4514PRESENT
    sensormics.blocking_fast_heat();
#endif
  }
  delay(1000);

#ifdef SDS011PRESENT
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
  }  else {
    IF_SDEBUG(Serial.println(F("ERROR getting sds011 values !")));
  }
#endif

#ifdef MICS4514PRESENT
  ok = sensormics.query_data_auto(&co, &no2, 3);
  if (oneshot) sensormics.sleep();

  wdt_reset();

  if (ok){
    
    float ppm;
    
    if (COCal.getConcentration(float(co),&ppm))
      {
	i2c_dataset1->cono2.co=round(ppm);
      }
    if (NO2Cal.getConcentration(float(no2),&ppm))
      {
	i2c_dataset1->cono2.no2=round(ppm);
      }
    
    IF_SDEBUG(Serial.print("co: "));
    IF_SDEBUG(Serial.println(i2c_dataset1->cono2.co));
    IF_SDEBUG(Serial.print("no2: "));
    IF_SDEBUG(Serial.println(i2c_dataset1->cono2.no2));
  }  else {
    IF_SDEBUG(Serial.println(F("ERROR getting mics4514 values !")));
  }
#endif

  if (oneshot) {
    //if one shot we have finish
    IF_SDEBUG(Serial.println(F("oneshot end")));
    start=false;    
    return;
  }

  // statistical processing

  float sum260, sum60;

  // first level mean

  IF_SDEBUG(Serial.print("data in store first: "));
  IF_SDEBUG(Serial.println(nsample1));


#ifdef SDS011PRESENT
  
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
    IF_SDEBUG(Serial.print("minpm25: "));
    IF_SDEBUG(Serial.println(minpm25));
    IF_SDEBUG(Serial.print("minpm10: "));
    IF_SDEBUG(Serial.println(minpm10));

    IF_SDEBUG(Serial.print("meanpm25: "));
    IF_SDEBUG(Serial.println(meanpm25));
    IF_SDEBUG(Serial.print("meanpm10: "));
    IF_SDEBUG(Serial.println(meanpm10));

    IF_SDEBUG(Serial.print("maxpm25: "));
    IF_SDEBUG(Serial.println(maxpm25));
    IF_SDEBUG(Serial.print("maxpm10: "));
    IF_SDEBUG(Serial.println(maxpm10));

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


    minpm25=LONG_MAX;
    meanpm25=0.;
    maxpm25=0.;

    minpm10=LONG_MAX;
    meanpm10=0.;
    maxpm10=0.;

    sum2pm25=0;
    sumpm25=0;

    sum2pm10=0;
    sumpm10=0;

  }

  // sigma

  if (cbsum2pm25.getSize() == cbsum2pm25.getCapacity() && cbsumpm25.getSize() == cbsumpm25.getCapacity()){
    sum260=0;
    for (i=0 ; i < cbsum2pm25.getCapacity() ; i++){
      sum260 += cbsum2pm25.peek(i);
    }

    sum60=0;
    for (i=0 ; i < cbsumpm25.getCapacity() ; i++){
      sum60 += cbsumpm25.peek(i);
    }
	
    i2c_dataset1->pm.sigmapm25=round(sqrt((sum260-(sum60*sum60)/(SAMPLE1*SAMPLE2))/(SAMPLE1*SAMPLE2)));
      
  }else{
    i2c_dataset1->pm.sigmapm25=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("sigma pm25: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->pm.sigmapm25));


  if (cbsum2pm10.getSize() == cbsum2pm10.getCapacity() && cbsumpm10.getSize() == cbsumpm10.getCapacity()){
    sum260=0;
    for (i=0 ; i < cbsum2pm10.getCapacity() ; i++){
      sum260 += cbsum2pm10.peek(i);
    }

    sum60=0;
    for (i=0 ; i < cbsumpm10.getCapacity() ; i++){
      sum60 += cbsumpm10.peek(i);
    }
	
    i2c_dataset1->pm.sigmapm10=round(sqrt((sum260-(sum60*sum60)/(SAMPLE1*SAMPLE2))/(SAMPLE1*SAMPLE2)));
      
  }else{
    i2c_dataset1->pm.sigmapm10=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("sigma pm10: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->pm.sigmapm10));

#endif


#ifdef MICS4514PRESENT
  
  // first level min
  if (minco > co) minco=co;
  if (minno2 > no2) minno2=no2;

  // first level mean
  meanco += (float(co) - meanco) / nsample1;
  meanno2 += (float(no2) - meanno2) / nsample1;

  // first level max
  if (maxco < co) maxco=co;
  if (maxno2 < no2) maxno2=no2;

  // sigma
  sum2co+=co*co;
  sumco+=co;

  sum2no2+=no2*no2;
  sumno2+=no2;

  if (nsample1 == SAMPLE1) {
    IF_SDEBUG(Serial.print("minco: "));
    IF_SDEBUG(Serial.println(minco));
    IF_SDEBUG(Serial.print("minno2: "));
    IF_SDEBUG(Serial.println(minno2));

    IF_SDEBUG(Serial.print("meanco: "));
    IF_SDEBUG(Serial.println(meanco));
    IF_SDEBUG(Serial.print("meanno2: "));
    IF_SDEBUG(Serial.println(meanno2));

    IF_SDEBUG(Serial.print("maxco: "));
    IF_SDEBUG(Serial.println(maxco));
    IF_SDEBUG(Serial.print("maxno2: "));
    IF_SDEBUG(Serial.println(maxno2));

    cbco60n.autoput(minco);
    cbno260n.autoput(minno2);

    cbco60m.autoput(meanco);
    cbno260m.autoput(meanno2);

    cbco60x.autoput(maxco);
    cbno260x.autoput(maxno2);

    cbsum2co.autoput(sum2co);
    cbsumco.autoput(sumco);

    cbsum2no2.autoput(sum2no2);
    cbsumno2.autoput(sumno2);


    minco=LONG_MAX;
    meanco=0.;
    maxco=0.;

    minno2=LONG_MAX;
    meanno2=0.;
    maxno2=0.;

    sum2co=0;
    sumco=0;

    sum2no2=0;
    sumno2=0;

  }

  // sigma

  if (cbsum2co.getSize() == cbsum2co.getCapacity() && cbsumco.getSize() == cbsumco.getCapacity()){
    sum260=0;
    for (i=0 ; i < cbsum2co.getCapacity() ; i++){
      sum260 += cbsum2co.peek(i);
    }

    sum60=0;
    for (i=0 ; i < cbsumco.getCapacity() ; i++){
      sum60 += cbsumco.peek(i);
    }
	
    i2c_dataset1->cono2.sigmaco=round(sqrt((sum260-(sum60*sum60)/(SAMPLE1*SAMPLE2))/(SAMPLE1*SAMPLE2)));
      
  }else{
    i2c_dataset1->cono2.sigmaco=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("sigma co: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->cono2.sigmaco));


  if (cbsum2no2.getSize() == cbsum2no2.getCapacity() && cbsumno2.getSize() == cbsumno2.getCapacity()){
    sum260=0;
    for (i=0 ; i < cbsum2no2.getCapacity() ; i++){
      sum260 += cbsum2no2.peek(i);
    }

    sum60=0;
    for (i=0 ; i < cbsumno2.getCapacity() ; i++){
      sum60 += cbsumno2.peek(i);
    }
	
    i2c_dataset1->cono2.sigmano2=round(sqrt((sum260-(sum60*sum60)/(SAMPLE1*SAMPLE2))/(SAMPLE1*SAMPLE2)));
      
  }else{
    i2c_dataset1->cono2.sigmano2=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("sigma no2: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->cono2.sigmano2));

#endif

  
  if (nsample1 == SAMPLE1) {
    nsample1=0;
  }

  nsample1++;



#ifdef SDS011PRESENT

  // second level pm25

  IF_SDEBUG(Serial.print("data in store second pm25 min: "));
  IF_SDEBUG(Serial.println(cbpm2560n.getSize()));

  if (cbpm2560n.getSize() == cbpm2560n.getCapacity()){
    i2c_dataset1->pm.minpm25=LONG_MAX;

    for (i=0 ; i < cbpm2560n.getCapacity() ; i++){
      i2c_dataset1->pm.minpm25 = min(cbpm2560n.peek(i), i2c_dataset1->pm.minpm25);
    }

  }else{
    i2c_dataset1->pm.minpm25=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("pm25 second min: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->pm.minpm25));


  IF_SDEBUG(Serial.print("data in store second pm25 mean: "));
  IF_SDEBUG(Serial.println(cbpm2560m.getSize()));

  if (cbpm2560m.getSize() == cbpm2560m.getCapacity()){
    mean=0;
    for (i=0 ; i < cbpm2560m.getCapacity() ; i++){
      mean += (cbpm2560m.peek(i) - mean) / (i+1);
    }

    i2c_dataset1->pm.meanpm25=round(mean);

  }else{
    i2c_dataset1->pm.meanpm25=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("pm25 second mean: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->pm.meanpm25));


  IF_SDEBUG(Serial.print("data in store second pm25 max: "));
  IF_SDEBUG(Serial.println(cbpm2560x.getSize()));

  if (cbpm2560x.getSize() == cbpm2560x.getCapacity()){
    i2c_dataset1->pm.maxpm25=0;
    for (i=0 ; i < cbpm2560x.getCapacity() ; i++){
      i2c_dataset1->pm.maxpm25 = max(cbpm2560x.peek(i), i2c_dataset1->pm.maxpm25);
    }
  }else{
    i2c_dataset1->pm.maxpm25=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("pm25 second max: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->pm.maxpm25));


  // second level pm10

  IF_SDEBUG(Serial.print("data in store second pm10 min: "));
  IF_SDEBUG(Serial.println(cbpm1060n.getSize()));

  if (cbpm1060n.getSize() == cbpm1060n.getCapacity()){
    i2c_dataset1->pm.minpm10=LONG_MAX;
    for (i=0 ; i < cbpm1060n.getCapacity() ; i++){
      i2c_dataset1->pm.minpm10 = min(cbpm1060n.peek(i), i2c_dataset1->pm.minpm10);
    }
  }else{
    i2c_dataset1->pm.minpm10=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("pm10 second min: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->pm.minpm10));


  IF_SDEBUG(Serial.print("data in store second pm10 mean: "));
  IF_SDEBUG(Serial.println(cbpm1060m.getSize()));

  if (cbpm1060m.getSize() == cbpm1060m.getCapacity()){
    mean=0;
    for (i=0 ; i < cbpm1060m.getCapacity() ; i++){
      mean += (cbpm1060m.peek(i) - mean) / (i+1);
    }

    i2c_dataset1->pm.meanpm10=round(mean);

  }else{
    i2c_dataset1->pm.meanpm10=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("pm10 second mean: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->pm.meanpm10));


  IF_SDEBUG(Serial.print("data in store second pm10 max: "));
  IF_SDEBUG(Serial.println(cbpm1060x.getSize()));

  if (cbpm1060x.getSize() == cbpm1060x.getCapacity()){
    i2c_dataset1->pm.maxpm10=0;
    for (i=0 ; i < cbpm1060x.getCapacity() ; i++){
      i2c_dataset1->pm.maxpm10 = max(cbpm1060x.peek(i), i2c_dataset1->pm.maxpm10);
    }
  }else{
    i2c_dataset1->pm.maxpm10=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("pm10 second max: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->pm.maxpm10));

#endif


#ifdef MICS4514PRESENT

  // second level co

  IF_SDEBUG(Serial.print("data in store second co min: "));
  IF_SDEBUG(Serial.println(cbco60n.getSize()));

  if (cbco60n.getSize() == cbco60n.getCapacity()){
    i2c_dataset1->cono2.minco=LONG_MAX;

    for (i=0 ; i < cbco60n.getCapacity() ; i++){
      i2c_dataset1->cono2.minco = min(cbco60n.peek(i), i2c_dataset1->cono2.minco);
    }

  }else{
    i2c_dataset1->cono2.minco=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("co second min: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->cono2.minco));


  IF_SDEBUG(Serial.print("data in store second co mean: "));
  IF_SDEBUG(Serial.println(cbco60m.getSize()));

  if (cbco60m.getSize() == cbco60m.getCapacity()){
    mean=0;
    for (i=0 ; i < cbco60m.getCapacity() ; i++){
      mean += (cbco60m.peek(i) - mean) / (i+1);
    }

    i2c_dataset1->cono2.meanco=round(mean);

  }else{
    i2c_dataset1->cono2.meanco=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("co second mean: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->cono2.meanco));


  IF_SDEBUG(Serial.print("data in store second co max: "));
  IF_SDEBUG(Serial.println(cbco60x.getSize()));

  if (cbco60x.getSize() == cbco60x.getCapacity()){
    i2c_dataset1->cono2.maxco=0;
    for (i=0 ; i < cbco60x.getCapacity() ; i++){
      i2c_dataset1->cono2.maxco = max(cbco60x.peek(i), i2c_dataset1->cono2.maxco);
    }
  }else{
    i2c_dataset1->cono2.maxco=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("co second max: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->cono2.maxco));


  // second level no2

  IF_SDEBUG(Serial.print("data in store second no2 min: "));
  IF_SDEBUG(Serial.println(cbno260n.getSize()));

  if (cbno260n.getSize() == cbno260n.getCapacity()){
    i2c_dataset1->cono2.minno2=LONG_MAX;
    for (i=0 ; i < cbno260n.getCapacity() ; i++){
      i2c_dataset1->cono2.minno2 = min(cbno260n.peek(i), i2c_dataset1->cono2.minno2);
    }
  }else{
    i2c_dataset1->cono2.minno2=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("no2 second min: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->cono2.minno2));


  IF_SDEBUG(Serial.print("data in store second no2 mean: "));
  IF_SDEBUG(Serial.println(cbno260m.getSize()));

  if (cbno260m.getSize() == cbno260m.getCapacity()){
    mean=0;
    for (i=0 ; i < cbno260m.getCapacity() ; i++){
      mean += (cbno260m.peek(i) - mean) / (i+1);
    }

    i2c_dataset1->cono2.meanno2=round(mean);

  }else{
    i2c_dataset1->cono2.meanno2=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("no2 second mean: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->cono2.meanno2));


  IF_SDEBUG(Serial.print("data in store second no2 max: "));
  IF_SDEBUG(Serial.println(cbno260x.getSize()));

  if (cbno260x.getSize() == cbno260x.getCapacity()){
    i2c_dataset1->cono2.maxno2=0;
    for (i=0 ; i < cbno260x.getCapacity() ; i++){
      i2c_dataset1->cono2.maxno2 = max(cbno260x.peek(i), i2c_dataset1->cono2.maxno2);
    }
  }else{
    i2c_dataset1->cono2.maxno2=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("no2 second max: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->cono2.maxno2));

#endif

  
  digitalWrite(LEDPIN,!digitalRead(LEDPIN));  // blink Led

}  
