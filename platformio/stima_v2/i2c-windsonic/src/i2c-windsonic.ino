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
 * This program implements elaboration of wind speed and direction for
 * Gill Windsonic exported to i2c interface.
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
#ifdef ARDUINO_ARCH_AVR
#include <avr/wdt.h>
#endif
#include "Wire.h"
#include "registers-windsonic.h"         //Register definitions
#include "config.h"
#include "IntBuffer.h"
#include "FloatBuffer.h"

#include "EEPROMAnything.h"

#define REG_MAP_SIZE            sizeof(I2C_REGISTERS)       //size of register map
#define REG_WIND_SIZE           sizeof(wind_t)                  //size of register map for wind
#define REG_WRITABLE_MAP_SIZE   sizeof(I2C_WRITABLE_REGISTERS)       //size of register map

#define MAX_SENT_BYTES     0x0F                      //maximum amount of data that I could receive from a master device (register, plus 15 byte)

char confver[9] = CONFVER; // version of configuration saved on eeprom


IntBuffer cbu60m;
IntBuffer cbv60m;

IntBuffer cbu60p;
IntBuffer cbv60p;

IntBuffer cb60m;

FloatBuffer cbsum2;
FloatBuffer cbsum;
IntBuffer cbsect[9];

int cnt;

typedef struct {
  uint8_t    sw_version;                          // Version of the I2C_WINDSONIC sw
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
} __attribute__((packed)) I2C_REGISTERS;


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
static bool oneshot;
static bool start=false;
static bool stop=false;

volatile unsigned int count;
volatile unsigned long antirimb=0;

unsigned long starttime;
boolean forcedefault=false;

#ifdef ARDUINO_ARCH_STM32
HardwareSerial Serial1(USART2);
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
       if (receivedCommands[0] == I2C_WINDSONIC_COMMAND) {
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
     
     if ((receivedCommands[0]>=I2C_WINDSONIC_MAP_WRITABLE) && (receivedCommands[0] < (I2C_WINDSONIC_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE))) {    
       if ((receivedCommands[0]+(unsigned int)(bytesReceived-1)) <= (I2C_WINDSONIC_MAP_WRITABLE+REG_WRITABLE_MAP_SIZE)) {
	 //Writeable registers
	 // the two buffer should be in sync
	 ptr1 = (uint8_t *)i2c_writabledataset1+receivedCommands[0]-I2C_WINDSONIC_MAP_WRITABLE;
	 ptr2 = (uint8_t *)i2c_writabledataset2+receivedCommands[0]-I2C_WINDSONIC_MAP_WRITABLE;
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

// this is required to reset windsonic to default configuration and baud
bool init_baud() {

  /*
    statup message (terminator <CR><LF>):

WINDSONIC (Gill Instruments Ltd)

2368-106-01

RS232 (CFG)

CHECKSUM ROM:7D3C 7D3C *PASS*
CHECKSUM FAC:09EE 09EE *PASS*
CHECKSUM ENG:17FB 17FB *PASS*
CHECKSUM CAL:CC55 CC55 *PASS*

  */

  /* commands to set defaults:

     *     to enter in setup mode

     M2,U1,O1,L1,P3,B3,H2,NQ,F1,E3,T1,S4,C2,G0,K50,

     Q     exit from setup mode

     M1 Gill, UV, Continuous

  */

  IF_SDEBUG(Serial.println(F("#initializing modem fixbaud ...")));

  /* Initialize serial for wind sensor comunication
     WindSonic default settings are :
     Bits per second            9600
     Data bits                  8
     Parity                     None
     Stop bits                  1
     Flow Control(Handshaking)  None
  */

  // try different fixed baud rate
  long int baudrate []={1200,2400,4800,9600,19200,38400,57600,115200};

  for (byte i=0; (i<(sizeof(baudrate) / sizeof(long int))); i++) {
#ifdef ARDUINO_ARCH_AVR
    wdt_reset();
#endif

    IF_SDEBUG(Serial.print(F("#TRY BAUDRATE:")));
    IF_SDEBUG(Serial.println(baudrate[i]));
    SERIALWIND.begin(baudrate[i]);

    SERIALWIND.print("");

    if (SERIALWIND.read()){
      // TODO

      IF_SDEBUG(Serial.println(F("#baudrate found")));
	return true;
    }
  }

  IF_SDEBUG(Serial.println(F("inizialize failed")));
#ifdef ARDUINO_ARCH_AVR
  wdt_reset();
#endif
  return false;
}

bool init_setup()
{
  /*  Configuraion string
*
M4
U1
O1
L1
P3
B3
H2
NQ
F1
E3
T1
S4
C2
G0
K50
Q
  */

  /*

  // clean serial buffer
  while (SERIALWIND.available() > 0) {
    byte incomingByte = SERIALWIND.read();
    // read the incoming byte:
    IF_SDEBUG(Serial.print(F("received: ")));
    IF_SDEBUG(Serial.write(incomingByte));
    IF_SDEBUG(Serial.print(F(" : ")));
    IF_SDEBUG(Serial.println(incomingByte, HEX));
  }

  SERIALWIND.print("Q\n");   // exit from setup in poll mode
  delay(500);
  SERIALWIND.print("Q\n");   // exit from setup in poll mode
  delay(500);
  SERIALWIND.print("*");     // enter in setup
  delay(1000);
  SERIALWIND.print("*");     // enter in setup
  delay(500);
  SERIALWIND.print("*Q");    // enter in setup from poll mode

 #define BUF_LENGTH 100

  char buf[BUF_LENGTH];
  byte count = 0;
  bool ok = false;
  unsigned long timeIsOut;
  char *rec;
  rec=buf;
  *rec = 0;

  timeIsOut = millis() + 3000;
  while (timeIsOut > millis() && count < (BUF_LENGTH - 1) && !ok) {  
    wdt_reset();

    if (SERIALWIND.available())
      {
	count++;
	*rec++ = SERIALWIND.read();
	*rec = 0;           // terminate the string
	ok=strstr(buf,"CONFIGURATION MODE");
      }
  }

  IF_SDEBUG(Serial.println(F("#windsonic:RECEIVED:")));
  IF_SDEBUG(Serial.println(buf));

  if (ok){
    IF_SDEBUG(Serial.println(F("#setup mode:->ok")));
  }else{
    IF_SDEBUG(Serial.println(F("#setup mode:->not ok")));
  }

  SERIALWIND.print("M4\r");
  delay(100);
  SERIALWIND.print("U1\r");
  delay(100);
  SERIALWIND.print("O1\r");
  delay(100);
  SERIALWIND.print("L1\r");
  delay(100);
  SERIALWIND.print("P3\r");
  delay(100);
  SERIALWIND.print("B3\r");
  delay(100);
  SERIALWIND.print("H2\r");
  delay(100);
  SERIALWIND.print("NQ\r");
  delay(100);
  SERIALWIND.print("F1\r");
  delay(100);
  SERIALWIND.print("E3\r");
  delay(100);
  SERIALWIND.print("T1\r");
  delay(100);
  SERIALWIND.print("S4\r");
  delay(100);
  SERIALWIND.print("C2\r");
  delay(100);
  SERIALWIND.print("G0\r");
  delay(100);
  SERIALWIND.print("K50\r");
  delay(100);
  SERIALWIND.print("Q\r");
  delay(100);
  */

}


// read dd & ff from windsonic in Polled mode
bool readMessage(String& myString)
{
#ifdef ARDUINO_ARCH_AVR
  wdt_reset();
#endif
  SERIALWIND.setTimeout(SAMPLETIME);
  SERIALWIND.find(2);                        // wait for <STX> = Start of string character (ASCII value 2)
  SERIALWIND.setTimeout(100);
  myString=SERIALWIND.readStringUntil(10);   //read until <LF> ASCII character
  IF_SDEBUG(Serial.println(myString));

#ifdef ARDUINO_ARCH_AVR
  wdt_reset();
#endif
  return true;
}

bool readPolledMessage(String& myString)
{

  /*
    When in the Polled mode, an output is only generated when the host system sends a Poll 
    signal to the WindSonic consisting of the WindSonic Unit Identifier that is, the relevant 
    letter A - Z.
    The commands available in this mode are:
    Description                       Command            WindSonic response
    WindSonic Unit Identifier         A ..... Z          Wind speed output generated
    Enable Polled mode                ?                  (None)
    Disable Polled mode               !                  (None)
    Request WindSonic Unit Identifier ?&                 A ..... Z (as configured)
    Enter Configuration mode          *<N>               CONFIGURATION MODE

    Where <N> is the unit identifier, if used in a multidrop system then it is recommended that 
    ID's A to F and KMNP are not used as these characters can be present in the data string.
 
    It is suggested that in polled mode the following sequence is used for every poll for 
    information.
    ? Ensures that the Sensor is enabled to cover the event that a power down has occurred.
    A-Z Appropriate unit designator sent to retrieve a line of data.
    ! Sent to disable poll mode and reduce possibility of erroneous poll generation.

    When in polled mode the system will respond to the data command within 130mS with the 
    last valid data sample as calculated by the Output rate (P Mode Setting).
  */

#ifdef ARDUINO_ARCH_AVR
  wdt_reset();
#endif
  // query and read one message
  SERIALWIND.print("?");                     // Enaable Polled mode
  SERIALWIND.print("Q");                     // Wind speed output generated

  bool status=readMessage(myString);

  SERIALWIND.print("!");                     // Disable Polled mode
#ifdef ARDUINO_ARCH_AVR
  wdt_reset();
#endif
  return status;
}

  
bool decodeValue(const String& myString,  unsigned int& dd, unsigned int& ff)
{
    
  String strmychecksum;

  //TODO parse periodic windsonic serial messages
  // max 8 sec watchdog timer

  /* sample messages:
     Q,,000.03,M,00,2D
     Q,,000.04,M,00,2A
     Q,349,000.05,M,00,15
     Q,031,000.06,M,00,1A
     Q,103,000.06,M,00,1A
     

     Gill format Polar, Continuous (Default format)
     
     <STX>Q, 229, 002.74, M, 00, <ETX>16
     
     Where:
     <STX> = Start of string character (ASCII value 2)
     WindSonic node address = Unit identifier
     Wind direction = Wind Direction
     Wind speed = Wind Speed
     Units = Units of measure (knots, m/s etc)
     Status = Anemometer status code (see Section 11.5 for further details)
     <ETX> = End of string character (ASCII value 3)
     Checksum = This is the EXCLUSIVE OR of the bytes between (and not including) the <STX> and <ETX> characters.
     <CR> ASCII character
     <LF> ASCII characte
     
     The Status code is sent as part of each wind measurement message 
     Code  Status                 Condition
     00    OK                     Sufficient samples in average period
     01    Axis 1 failed          Insufficient samples in average period on U axis
     02    Axis 2 failed          Insufficient samples in average period on V axis
     04    Axis 1 and 2 failed    Insufficient samples in average period on both axes
     08    NVM error              NVM checksum failed
     09    ROM error              ROM checksum failed
     
  */

  int firstDelimiter = myString.indexOf(',');
  if (firstDelimiter == -1){
    IF_SDEBUG(Serial.println(F("1 , not found in windsonic message")));
    return false;
  }
    
  String value = myString.substring(0, firstDelimiter);

  /*
  for(int i = 0; i < value.length(); i++)
    {
      IF_SDEBUG(Serial.println(value[i],HEX));
    }
  */

  char unitidentifier=value[0];

  if ( unitidentifier != 'Q'){
    IF_SDEBUG(Serial.println(F("Q not found in windsonic message")));
    return false;
  }

  //  Search for the next comma just after the first
  int secondDelimiter = myString.indexOf(',', firstDelimiter+1);
  if (secondDelimiter == -1){
    IF_SDEBUG(Serial.println(F("2 , not found in windsonic message")));
    return false;
  }
  value = myString.substring(firstDelimiter+1, secondDelimiter);
  int direction = value.toFloat();

  //IF_SDEBUG(Serial.println(value));
  IF_SDEBUG(Serial.print(F("direction :")));
  IF_SDEBUG(Serial.println(direction));

  firstDelimiter = secondDelimiter;
  secondDelimiter = myString.indexOf(',', firstDelimiter+1);
  if (secondDelimiter == -1){
    IF_SDEBUG(Serial.println(F("3 , not found in windsonic message")));
    return false;
  }
  value = myString.substring(firstDelimiter+1, secondDelimiter);
  float speed = value.toFloat();

  //IF_SDEBUG(Serial.println(value));
  IF_SDEBUG(Serial.print(F("speed :")));
  IF_SDEBUG(Serial.println(speed));

  firstDelimiter = secondDelimiter;
  secondDelimiter = myString.indexOf(',', firstDelimiter+1);
  if (secondDelimiter == -1){
    IF_SDEBUG(Serial.println(F("4 , not found in windsonic message")));
    return false;
  }
  value = myString.substring(firstDelimiter+1, secondDelimiter);

  /*
    for(int i = 0; i < value.length(); i++)
    {
      IF_SDEBUG(Serial.println(value[i],HEX));
    }
  */

  char units =value[0];

  if ( units != 'M'){

    /*
      Metres per second (default) M
      Knots                       N
      Miles per hour              P
      Kilometres per hour         K
      Feet per minute             F
    */

    IF_SDEBUG(Serial.println(F("M not found in windsonic message")));
    return false;
  }

  firstDelimiter = secondDelimiter;
  secondDelimiter = myString.indexOf(',', firstDelimiter+1);
  if (secondDelimiter == -1){
    IF_SDEBUG(Serial.println(F("5 , not found in windsonic message")));
    return false;
  }
  value = myString.substring(firstDelimiter+1, secondDelimiter);
  int status = value.toFloat();

  //IF_SDEBUG(Serial.println(value));
  //IF_SDEBUG(Serial.print(F("status :")));
  //IF_SDEBUG(Serial.println(status));

  /*
  if (status == 0){
    IF_SDEBUG(Serial.println(F("OK                     Sufficient samples in average period")));
  }
  */

  if (status == 1){
    IF_SDEBUG(Serial.println(F("Axis 1 failed          Insufficient samples in average period on U axis")));
  }
  if (status == 2){
    IF_SDEBUG(Serial.println(F("Axis 2 failed          Insufficient samples in average period on V axis")));
  }
  if (status == 4){
    IF_SDEBUG(Serial.println(F("Axis 1 and 2 failed    Insufficient samples in average period on both axes")));
  }
  if (status == 8){
    IF_SDEBUG(Serial.println(F("NVM error              NVM checksum failed")));
  }
  if (status == 9){
    IF_SDEBUG(Serial.println(F("ROM error              ROM checksum failed")));
  }

  if (status != 0){
    IF_SDEBUG(Serial.println(F("status error found  in windsonic message")));
    return false;
  }

  if (myString[secondDelimiter+1] != char(3)){
    IF_SDEBUG(Serial.println(F("<ETX> not found  in windsonic message")));
    return false;
  }

  unsigned char mychecksum = myString[0] ;
  //IF_SDEBUG(Serial.println(F("check: ")));
  //IF_SDEBUG(Serial.println(myString[0]));
  //IF_SDEBUG(Serial.println(myString[0],HEX));
  for (unsigned int i=1; i <= secondDelimiter; i++){ // iterates through the string to checksum starting from second char
    //IF_SDEBUG(Serial.println(F("check: ")));
    //IF_SDEBUG(Serial.println(myString[i]));
    //IF_SDEBUG(Serial.println(myString[i],HEX));
    mychecksum=(unsigned char)(mychecksum ^ myString[i]) ; // ^ - XOR operator in C++
    // using an int and a base (hexadecimal):
    strmychecksum = String(mychecksum, HEX);

  }

  firstDelimiter = secondDelimiter;
  secondDelimiter = myString.indexOf('\r', firstDelimiter+1);
  if (secondDelimiter == -1){
    IF_SDEBUG(Serial.println(F("<CR> not found in windsonic message")));
    return false;
  }

  String checksum = myString.substring(firstDelimiter+2,secondDelimiter);

  if (!strmychecksum.equalsIgnoreCase(checksum)){
    IF_SDEBUG(Serial.print(F("mychecksum :")));
    IF_SDEBUG(Serial.println(strmychecksum));
    IF_SDEBUG(Serial.print(F("  checksum :")));
    IF_SDEBUG(Serial.println(checksum));
    IF_SDEBUG(Serial.println(F("checksum error in windsonic message")));
    return false;
  }

  if (myString[myString.length()-1] != char(13)){
    IF_SDEBUG(Serial.println(F("<CR> not found  in windsonic message")));
    return false;
  }

  /*
    Low Wind Speeds (below 0.05ms)
    Whilst the wind speed is below 0.05 metres/sec, the wind direction will not be calculated.
    In both CSV mode and in Fixed Field mode, Channel 2 wind direction output
    will freeze at
    the last known valid direction value until a new valid value can be calculated.
    The above applies with the K command set for K50. If K for instance is set for 100 then the
    above applies at 0.1m/s.
  */

  dd=direction;
  ff=int(round(speed*10.)); // m/s *10

  if (dd == 0) dd=360;      // traslate 0 -> 360
  //dd=max(dd,1);
  //dd=min(dd,360);
  if (ff == 0) dd=0;        // wind calm

  return true;

}

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
      i2c_writabledataset1->i2c_address = I2C_WINDSONIC_DEFAULTADDRESS;
      i2c_writabledataset2->i2c_address = I2C_WINDSONIC_DEFAULTADDRESS;
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

  SERIALWIND.begin(9600);

  /*
    Al momento la configurazione sia del baud rate che delle impostazioni non funziona
    Il windsonic non risponde come ci si aspetta

    Quindi questo firmware funziona se le conf vengono fatte a mano sul
    windsonic come riportato in init_setup

    Quindi in modalità non oneshot comunque i dati vengono richiesti
    come nella modalità oneshot e la temporarizzazione è interna a
    questo firmware con una delay alla fine del main loop

    In via definitiva bisognerebbe gestire il cambio di modalità di
    funzionamento configurando runtime il windsonic in modalità
    continua quando si passa alla modalità non oneshot e usare il
    timing del windsonic per ottenere una misura ogni 250 us

    Attualmente le elaborazioni fatte in modalità non oneshot sono
    fatte con un dato ogni 500us; in fondo poco male visto che il
    tutto risulta molto più stabile
  */

  //TODO windsonic initialization
  //init_baud();
  //init_setup();

  delay(500);

  // clean serial buffer
  while (SERIALWIND.available() > 0) {
    byte incomingByte = SERIALWIND.read();
    // read the incoming byte:
    IF_SDEBUG(Serial.print(F("received: ")));
    IF_SDEBUG(Serial.write(incomingByte));
    IF_SDEBUG(Serial.print(F(" : ")));
    IF_SDEBUG(Serial.println(incomingByte, HEX));
  }

  starttime = millis()+SAMPLERATE;

  IF_SDEBUG(Serial.println(F("end setup")));

}

void loop() {

  static uint8_t _command;
  unsigned int dd;
  unsigned int ff;
  int u;
  int v;
  float mean;

  unsigned int sector;
  
  uint8_t i;

#ifdef ARDUINO_ARCH_AVR
  wdt_reset();
#endif

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
    case I2C_WINDSONIC_COMMAND_ONESHOT_START:
      IF_SDEBUG(Serial.println(F("COMMAND: oneshot start")));
      start=true;
      break;          
    case I2C_WINDSONIC_COMMAND_ONESHOT_STOP:
      IF_SDEBUG(Serial.println(F("COMMAND: oneshot stop")));
      stop=true;
      break;
    case I2C_WINDSONIC_COMMAND_STOP:
      IF_SDEBUG(Serial.println(F("COMMAND: stop")));
      stop=true;
      break;
    case I2C_WINDSONIC_COMMAND_SAVE:
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
    ptr = (uint8_t *)&i2c_dataset1->wind;
    for (i=0;i<REG_WIND_SIZE;i++) { *ptr |= 0xFF; ptr++;}

    stop=false;
  }
  
  if (oneshot) {
    if (start)
      {
	// clean serial buffer
	byte incomingByte;
	while (SERIALWIND.available() > 0) {
	  incomingByte = SERIALWIND.read();
	  // read the incoming byte:
	  IF_SDEBUG(Serial.print(F("received: ")));
	  IF_SDEBUG(Serial.write(incomingByte));
	  IF_SDEBUG(Serial.print(F(" : ")));
	  IF_SDEBUG(Serial.println(incomingByte, HEX));
	}
      }
    else {
      return;
    }
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
    if (timetowait < -10) IF_SDEBUG(Serial.print("WARNIG: timing error , I am late"));    
  }

  starttime = millis()+timetowait;


  String myString;

  //for now we work ever in polled mode
  // uncomment if you introduce continuous mode
  //  if (oneshot) {
  if (! readPolledMessage(myString)) return;
    //}else{
    //if (! readMessage(myString)) return;
    //}    

  if (! decodeValue(myString, dd, ff)) return;

#ifdef ARDUINO_ARCH_AVR
  wdt_reset();
#endif

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
	
    i2c_dataset1->wind.sigma=round(sqrt((sum260-(sum60*sum60)/(SAMPLE1*SAMPLE2))/(SAMPLE1*SAMPLE2)));
      
  }else{
    i2c_dataset1->wind.sigma=MISSINTVALUE;
  }

  for (i=0; i<9 ; i++){
    if (cbsect[i].getSize() == cbsect[i].getCapacity() ){
      i2c_dataset1->wind.sect[i]=0;
      for (int ii=0 ; ii < cbsum2.getCapacity() ; ii++){
	i2c_dataset1->wind.sect[i]+=cbsect[i].peek(ii);
      }
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

    i2c_dataset1->wind.meanff=round(mean);

  }else{
    i2c_dataset1->wind.meanff=MISSINTVALUE;
  }

  IF_SDEBUG(Serial.print("mean FF second: "));
  IF_SDEBUG(Serial.println(i2c_dataset1->wind.meanff));


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
  IF_SDEBUG(Serial.println(i2c_dataset1->wind.sigma));

  for (i=0; i<9 ; i++){
    IF_SDEBUG(Serial.print("sect: "));
    IF_SDEBUG(Serial.print(i));
    IF_SDEBUG(Serial.print("->"));
    IF_SDEBUG(Serial.println(i2c_dataset1->wind.sect[i]));
  }

  digitalWrite(LEDPIN,!digitalRead(LEDPIN));  // blink Led

  // comment this if you manage continous mode
  // in this case timing is getted from windsonic that send valuer every SAMPLERATE us

}  
