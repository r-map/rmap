/**********************************************************************
Copyright (C) 2014  Paolo Paruno <p.patruno@iperbole.bologna.it>
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
 * This program implements wind intensity and direction 
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

#define VERSION 03             //Software version for cross checking

#include <avr/wdt.h>
#include "Wire.h"
#include "registers-wind.h"         //Register definitions
#include "config.h"
//#include "circular.h"
#include "IntBuffer.h"
#include "FloatBuffer.h"

#define REG_MAP_SIZE            sizeof(I2C_REGISTERS)       //size of register map
#define REG_WRITABLE_MAP_SIZE   sizeof(I2C_WRITABLE_REGISTERS)       //size of register map

#define MAX_SENT_BYTES     0x0F                      //maximum amount of data that I could receive from a master device (register, plus 15 byte)


//#include <FreqCounter.h>

IntBuffer cbu60m;
IntBuffer cbv60m;

IntBuffer cbu60p;
IntBuffer cbv60p;

IntBuffer cb60m;

FloatBuffer cbsum2;
FloatBuffer cbsum;
IntBuffer cbsect[9];

int cnt;
int pinLed=13;

typedef struct {
  uint8_t    sw_version;     // 0x00  Version of the I2C_GPS sw
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
  status_t     status;                   // 0x00  status register

//wind data
  wind_t                wind;                     // 0x01 wind
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

float meanff;
float meanu;
float meanv;
float peakgust;
int peakgustu;
int peakgustv;
float sum2;
float sum;
float sum260;
float sum60;
uint8_t nsample1;
uint16_t sect[9];

// one shot management
static bool start=false;
static bool stop=false;

volatile unsigned int count;
volatile unsigned long antirimb=0;

void countadd()
{

#if defined(DAVIS)
  unsigned long now=millis();

  if ((now-antirimb) > SAMPLETIME/150){
    count ++;
    antirimb=now;
  }

#elif defined (INSPEED)

  count ++;

#endif

}


//////////////////////////////////////////////////////////////////////////////////////
// I2C handlers
// Handler for requesting data
//
void requestEvent()
{
  Wire.write(((uint8_t *)i2c_dataset2)+receivedCommands[0],32);
  //Write up to 32 byte, since master is responsible for reading and sending NACK
  //32 byte limit is in the Wire library, we have to live with it unless writing our own wire library

  //Serial.println(*((uint8_t *)(i2c_dataset2)+receivedCommands[0]));
  //Serial.println(*((uint8_t *)(i2c_dataset2)+receivedCommands[0]+1));
  //Serial.println(*((uint8_t *)(i2c_dataset2)+receivedCommands[0]+2));
  //Serial.println(*((uint8_t *)(i2c_dataset2)+receivedCommands[0]+3));
}

//Handler for receiving data
void receiveEvent( int bytesReceived)
{
     uint8_t  *ptr;
     for (int a = 0; a < bytesReceived; a++) {
          if (a < MAX_SENT_BYTES) {
               receivedCommands[a] = Wire.read();
          } else {
               Wire.read();  // if we receive more data then allowed just throw it away
          }
     }

     if (bytesReceived == 2){
       // check for a command
       if (receivedCommands[0] == I2C_WIND_COMMAND) {
	 //IF_SDEBUG(Serial.print("received command:"));IF_SDEBUG(Serial.println(receivedCommands[1]));
	 new_command = receivedCommands[1]; return; }  //Just one byte, ignore all others
     }

     if (bytesReceived == 1){
       //read address for a given register
       //Addressing over the reg_map fallback to first byte
       if(bytesReceived == 1 && ( (receivedCommands[0] < 0) || (receivedCommands[0] >= REG_MAP_SIZE))) {
	 //IF_SDEBUG(Serial.print("set register:"));IF_SDEBUG(Serial.println(receivedCommands[0]));
	 receivedCommands[0]=0;
	 return;
       }
     }

     //More than 1 byte was received, so there is definitely some data to write into a register
     //Check for writeable registers and discard data is it's not writeable
     if ((receivedCommands[0]>=I2C_WIND_MAP_WRITABLE) && (receivedCommands[0] < (I2C_WIND_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE))) {    
       if ((receivedCommands[0]+(unsigned int)bytesReceived) <= (I2C_WIND_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE)) {
	 //Writeable registers
	 ptr = (uint8_t *)i2c_writabledataset1+receivedCommands[0];
	 for (int a = 1; a < bytesReceived; a++) { 
	   //IF_SDEBUG(Serial.print("write in writable buffer:"));IF_SDEBUG(Serial.print(a));IF_SDEBUG(Serial.println(receivedCommands[a]));
	   *ptr++ = receivedCommands[a];
	 }

	 // the two buffer should be in sync
	 ptr = (uint8_t *)i2c_writabledataset2+receivedCommands[0];
	 for (int a = 1; a < bytesReceived; a++) { *ptr++ = receivedCommands[a]; }
	 // new data written

       }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////


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

  // inizialize double buffer
  i2c_dataset1=&i2c_buffer1;
  i2c_dataset2=&i2c_buffer2;

  // inizialize writable double buffer
  i2c_writabledataset1=&i2c_writablebuffer1;
  i2c_writabledataset2=&i2c_writablebuffer2;

  meanff=0.;
  meanu=0.;
  meanv=0.;
  peakgust=-1;
  peakgustu=0;
  peakgustv=0;
  sum2=0;
  sum=0;

  uint8_t i;
  for (i=0; i<9; i++){
    sect[i]=0;
  }

  nsample1=1;

#define SAMPLE1 60000/SAMPLERATE
#define SAMPLE2 10

  cbu60m.init(SAMPLE2);
  cbv60m.init(SAMPLE2);

  cbu60p.init(SAMPLE2);
  cbv60p.init(SAMPLE2);

  cb60m.init(SAMPLE2);
  cbsum2.init(SAMPLE2);
  cbsum.init(SAMPLE2);

  for(i=0; i<9 ;i++){
    cbsect[i].init(SAMPLE2);
  }

  analogReference(DEFAULT);
  pinMode(pinLed, OUTPUT);
  IF_SDEBUG(Serial.begin(9600));        // connect to the serial port

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

  // set default to oneshot
  i2c_writabledataset1->oneshot=true;
  i2c_writabledataset2->oneshot=true;

  //Start I2C communication routines
  Wire.begin(I2C_WIND_ADDRESS);
  Wire.onRequest(requestEvent);          // Set up event handlers
  Wire.onReceive(receiveEvent);

  /*
  // mmm needed to iniziatize FreqCounter: if not the first read is wrong
  FreqCounter::start(1);    // ms Gate Time
  while (FreqCounter::f_ready == 0) { }
  // TODO the the next read is 1 with calm too
  */

  pinMode(2,INPUT_PULLUP);  // connected to wind intensity sensor

  IF_SDEBUG(Serial.println(F("end setup")));

}

void loop() {

  static uint8_t _command;
  unsigned int dd;
  unsigned int ff;
  int u;
  int v;
  float mean;
  bool oneshot;

  unsigned int sector;
  
  unsigned long starttime;
  long waittime;
  uint8_t i;

  wdt_reset();

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
    case I2C_WIND_COMMAND_ONESHOT_START:
      IF_SDEBUG(Serial.println(F("COMMAND: oneshot start")));
      start=true;
      break;          
    case I2C_WIND_COMMAND_ONESHOT_STOP:
      IF_SDEBUG(Serial.println(F("COMMAND: oneshot stop")));
      stop=true;
      break;
    case I2C_WIND_COMMAND_STOP:
      IF_SDEBUG(Serial.println(F("COMMAND: stop")));
      stop=true;      
      break;
    } //switch  
  }

  oneshot=i2c_writabledataset2->oneshot;

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
    ptr = (uint8_t *)i2c_dataset1;
    for (i=0;i<REG_MAP_SIZE;i++) { *ptr |= 0xFF; ptr++;}
    //Init to FF i2c_dataset2;
    //ptr = (uint8_t *)i2c_dataset2;
    //for (i=0;i<REG_MAP_SIZE;i++) { *ptr |= 0xFF; ptr++;}
    stop=false;
  }
  
  if (oneshot) {
    if (! start) return;
  }

  //FreqCounter::f_comp=10;          // Cal Value / Calibrate with professional Freq Counter
  //FreqCounter::start(SAMPLETIME);    // ms Gate Time

  count=0;
  starttime = millis();
  attachInterrupt(digitalPinToInterrupt(INTERRUPTPIN), countadd, RISING);

  // wait to go in the middle of sampletime to get direction
  // will be better to do a mean but direction is not continueous function
  IF_SDEBUG(Serial.print(F("delay for middle: ")));IF_SDEBUG(Serial.println(SAMPLERATE/2));
  delay(SAMPLERATE/2);

  int ana=analogRead(ANALOGPIN);    // read the input pin


#if defined(DAVIS)
  /*
    The potentiometer in the wind vane is free to move through 360 degrees,
    but there is a dead band of about 10 degrees at the crossover point in the potentiometer travel. 
    As shown in the graph, the output signal is zero volts for 5 degrees before the signal starts its 
    linear rise, and the signal is maximum voltage through 5 degrees at the other extreme of the travel.
    The dead band varies some from unit to unit. If you are looking for the greatest possible accuracy, 
    then you will need to find out exactly what the dead band is. You can also find out exactly what the 
    dead band is by cutting a hole and a slit in a sheet of polar graph paper and putting it around the 
    vane, and taking readings of w1 as a function of position of the vane in degrees. If you do not need 
    great accuracy (5%) then simply use the typical value, 10 for the dead band calibration factor.
    
    http://www.emesystems.com/OL2wind.htm
  */
  
  if (ana ==0){             //first half od dead band
    dd=2;
  }else if (ana ==1023){    //second half od dead band
    dd=358;
  }else{
    dd=round(ana*(350./1021.)+5);  // linear for no dead band
  }

#elif defined (INSPEED)
  //IF_SDEBUG(Serial.print(F("ana: ")));IF_SDEBUG(Serial.println(ana));
  dd=round((ana-50)*(360./921.));  // linear for no dead band fron 5% to 95% of full range (0-1023)

#endif

  dd=max(dd,1);
  dd=min(dd,360);
  

  //while (FreqCounter::f_ready == 0) { }
  //IF_SDEBUG(Serial.print(F("freq: ")));IF_SDEBUG(Serial.println(FreqCounter::f_freq));
  //count=FreqCounter::f_freq

  IF_SDEBUG(Serial.print(F("delay for end: ")));IF_SDEBUG(Serial.println(SAMPLERATE-(millis()-starttime)));
  delay(SAMPLERATE-(millis()-starttime));

  detachInterrupt(digitalPinToInterrupt(INTERRUPTPIN));
  IF_SDEBUG(Serial.print(F("count: ")));IF_SDEBUG(Serial.println(count));


  // I have to put timing here becouse the timer is used by FreqCounter
  //starttime = millis();

#if defined(DAVIS)
  /*
    We'd like to count the number of pulses in a time interval, which is directly proportional to windspeed.
    Counting the signal from the Davis 6410 for 2.25 seconds will give a result directly in units of mph,
     mph = 2.25 * counts / seconds     ' (so counts=mph when seconds=2.25)
  */

  ff=round(count/(SAMPLETIME/2250.0)*0.44704*10.);   // m/s *10

#elif defined (INSPEED)
  /*
    2.5  mph per Hz (1 Hz = 1 pulse/second) 
  */

  ff=round(count/(SAMPLETIME/2500.0)*0.44704*10.);   // m/s *10

#endif

  if (ff == 0) dd=0;     //wind calm
  
  i2c_dataset1->wind.ff=ff;
  i2c_dataset1->wind.dd=dd;

  //#define PI 3.14159265    // arduino defined
  float ar=float(dd)*PI/180.;

  //scambio seno e coseno per rotazione 90 gradi
  u=round(-float(ff)*sin(ar));
  i2c_dataset1->wind.u=u+OFFSET;

  v=round(-float(ff)*cos(ar));
  i2c_dataset1->wind.v=v+OFFSET;

  IF_SDEBUG(Serial.print("dd: "));
  IF_SDEBUG(Serial.println(dd));
  IF_SDEBUG(Serial.print("ff: "));
  IF_SDEBUG(Serial.println(ff));
  IF_SDEBUG(Serial.print("u: "));
  IF_SDEBUG(Serial.println(u));
  IF_SDEBUG(Serial.print("v: "));
  IF_SDEBUG(Serial.println(v));

  if (oneshot) {
    //if one shot we have finish
    IF_SDEBUG(Serial.println(F("oneshot end")));
    start=false;    
    return;
  }

  // 8 sector  (sector 1  =>  -22.5 to +22.5 North)
  if (ff == 0){
    sect[8]++;
  }else{
    sector=((dd+22.5)/45)+1;
    if (sector >8 ) sector=1;
    sect[sector]++;
  }

  // statistical processing

  // first level mean

  IF_SDEBUG(Serial.print("data in store first: "));
  IF_SDEBUG(Serial.println(nsample1));

  // FF mean
  float fff;
  fff = sqrt(float(u)*float(u) + float(v)*float(v));
  meanff += (fff - meanff) / (nsample1);

  // U and V mean
  meanu += (float(u) - meanu) / (nsample1);
  meanv += (float(v) - meanv) / (nsample1);

  // first level peak gust
  if (peakgust < fff){
    peakgust=fff;
    peakgustu=u;
    peakgustv=v;
  }

  // sigma
  sum2+=fff*fff;
  sum+=fff;

  if (nsample1 == SAMPLE1) {
    IF_SDEBUG(Serial.print("meanff: "));
    IF_SDEBUG(Serial.println(meanff));
    IF_SDEBUG(Serial.print("meanu: "));
    IF_SDEBUG(Serial.println(meanu));
    IF_SDEBUG(Serial.print("meanv: "));
    IF_SDEBUG(Serial.println(meanv));

    cb60m.autoput(round(meanff));
    cbu60m.autoput(round(meanu));
    cbv60m.autoput(round(meanv));
    cbu60p.autoput(peakgustu);
    cbv60p.autoput(peakgustv);
    cbsum2.autoput(sum2);
    cbsum.autoput(sum);
    for (i=0; i<9; i++){
      cbsect[i].autoput(sect[i]);
    }

    meanff=0.;
    meanu=0.;
    meanv=0.;
    nsample1=0;
    peakgust=-1;
    peakgustu=0;
    peakgustv=0;
    sum2=0;
    sum=0;
    for (i=0; i<9; i++){
      sect[i]=0;
    }
  }


  if (cbsum2.getSize() == cbsum2.getCapacity() && cbsum.getSize() == cbsum.getCapacity()){
    sum260=0;
    for (i=0 ; i < cbsum2.getCapacity() ; i++){
      sum260 += cbsum2.peek(i);
    }

    sum60=0;
    for (i=0 ; i < cbsum.getCapacity() ; i++){
      sum60 += cbsum.peek(i);
    }
	
    i2c_dataset1->wind.sigma=round(sqrt((sum260-(sum60*sum60)/(SAMPLE1*SAMPLE2))/(SAMPLE1*SAMPLE2)))+OFFSET;
      
  }else{
    i2c_dataset1->wind.sigma=MISSINTVALUE;
  }

  for (i=0; i<9 ; i++){
    if (cbsect[i].getSize() == cbsect[i].getCapacity() ){
      i2c_dataset1->wind.sect[i]=0;
      for (int ii=0 ; ii < cbsum2.getCapacity() ; ii++){
	i2c_dataset1->wind.sect[i]+=cbsect[i].peek(ii);
      }

      i2c_dataset1->wind.sect[i]+=OFFSET;

    }else{
      i2c_dataset1->wind.sect[i]=MISSINTVALUE;
    }
  }

  nsample1++;

  // second level mean

  // FF mean

  IF_SDEBUG(Serial.print("data in store second FF: "));
  IF_SDEBUG(Serial.println(cb60m.getSize()));
  IF_SDEBUG(Serial.print("data in store second U: "));
  IF_SDEBUG(Serial.println(cbu60m.getSize()));
  IF_SDEBUG(Serial.print("data in store second V: "));
  IF_SDEBUG(Serial.println(cbv60m.getSize()));

  if (cb60m.getSize() == cb60m.getCapacity()){
    mean=0;
    for (i=0 ; i < cb60m.getCapacity() ; i++){
      mean += (cb60m.peek(i) - mean) / (i+1);
    }

    i2c_dataset1->wind.meanff=round(mean)+OFFSET;

  }else{
    i2c_dataset1->wind.meanff=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("mean FF second: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->wind.meanff-OFFSET));


  // U and V mean

  if (cbu60m.getSize() == cbu60m.getCapacity()){
    mean=0;
    for ( i=0 ; i < cbu60m.getCapacity() ; i++){
      mean += (cbu60m.peek(i) - mean) / (i+1);
    }
    i2c_dataset1->wind.meanu=round(mean)+OFFSET;
  }else{
    i2c_dataset1->wind.meanu=MISSINTVALUE;
  }

  if (cbv60m.getSize() == cbv60m.getCapacity()){
    mean=0;
    for ( i=0 ; i < cbv60m.getCapacity() ; i++){
      mean += (cbv60m.peek(i) - mean) / (i+1);
    }
    i2c_dataset1->wind.meanv=round(mean)+OFFSET;
  }else{
    i2c_dataset1->wind.meanv=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("meanu: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->wind.meanu-OFFSET));
  IF_SDEBUG(Serial.print("meanv: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->wind.meanv-OFFSET));

  //second level peak gust

  if ((cbu60p.getSize() == cbu60p.getCapacity()) && (cbv60p.getSize() == cbv60p.getCapacity())){

    float peakgust=-1;
    float gust;

    for ( i=0 ; i < cbv60p.getCapacity() ; i++){

      //IF_SDEBUG(Serial.println(cbu60p.peek(i)));
      //IF_SDEBUG(Serial.println(cbv60p.peek(i)));
      float u = float(cbu60p.peek(i));
      float v = float(cbv60p.peek(i));

      gust = sqrt(u*u + v*v);
      if (peakgust < gust){
	peakgust= gust;
	i2c_dataset1->wind.peakgustu=cbu60p.peek(i)+OFFSET;
	i2c_dataset1->wind.peakgustv=cbv60p.peek(i)+OFFSET;
      }
    }
  }else{
    i2c_dataset1->wind.peakgustu=MISSINTVALUE;
    i2c_dataset1->wind.peakgustv=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("peakgustu: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->wind.peakgustu-OFFSET));
  IF_SDEBUG(Serial.print("peakgustv: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->wind.peakgustv-OFFSET));


  //second level long gust

  if ((cbu60m.getSize() == cbu60m.getCapacity()) && (cbv60m.getSize() == cbv60m.getCapacity())){

    float peakgust=-1;
    float gust;

    for ( i=0 ; i < cbv60m.getCapacity() ; i++){

      //IF_SDEBUG(Serial.println(cbu60m.peek(i)));
      //IF_SDEBUG(Serial.println(cbv60m.peek(i)));
      
      float u = float(cbu60m.peek(i));
      float v = float(cbv60m.peek(i));

      gust = sqrt(u*u +v*v);
      if (peakgust < gust){
	peakgust= gust;
	i2c_dataset1->wind.longgustu=cbu60m.peek(i)+OFFSET;
	i2c_dataset1->wind.longgustv=cbv60m.peek(i)+OFFSET;
      }
    }
  }else{
    i2c_dataset1->wind.longgustu=MISSINTVALUE;
    i2c_dataset1->wind.longgustv=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("longgustu: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->wind.longgustu-OFFSET));
  IF_SDEBUG(Serial.print("longgustv: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->wind.longgustv-OFFSET));

  IF_SDEBUG(Serial.print("sigma: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->wind.sigma-OFFSET));

  for (i=0; i<9 ; i++){
    IF_SDEBUG(Serial.print("sect: "));
    IF_SDEBUG(Serial.print(i));
    IF_SDEBUG(Serial.print("->"));
    IF_SDEBUG(Serial.println(i2c_dataset1->wind.sect[i]-OFFSET));
  }

  digitalWrite(pinLed,!digitalRead(pinLed));  // blink Led

  //waittime= 750 - (millis() - starttime) ;
  waittime= SAMPLERATE - (millis() - starttime) ;
  //IF_SDEBUG(Serial.print("elapsed time: "));
  //IF_SDEBUG(Serial.println(millis() - starttime));
  if (waittime > 0) {
    IF_SDEBUG(Serial.print("wait for: "));
    IF_SDEBUG(Serial.println(waittime));
    delay(waittime); 
  }else{
    IF_SDEBUG(Serial.print("WARNIG: timing error , I am late"));    
  }

}  
