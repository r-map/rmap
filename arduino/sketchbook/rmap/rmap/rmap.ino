/*
Copyright (C) 2015  Paolo Paruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Freeg Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

///////////////////////////////////////////////////////////////////////

#define FIRMSERIAL 1
#define FIRMETHERNET 2
#define FIRMALL 3
#define FIRMRADIORF24 4
#define FIRMGSM 5

///////////////////////////////////////////////////////////////////////

#include "rmap_config.h"

/////////////////////////////////////////////////////////////////////////
// START Tricky solution for arduino ide 1.6
// those require manual adjustment to make things work
/////////////////////////////////////////////////////////////////////////

//// this is migrated in common.h to have arduino ide 1.6 happy
//// buffer for aJson print output and internal global_buffer
// static char mainbuf[MAIN_BUFFER_SIZE];

// added to have arduino ide 1.6 happy

#ifdef ETHERNETON
  #include <SPI.h>
  #ifdef ENC28J60
    #include <UIPEthernet.h>
    #include <UIPUdp.h>
    #ifdef TCPSERVER
      #include <UIPServer.h>
    #endif
  #else
    // comment/uncomment according to ENC28J60 definition
    //#include <SPI.h>         
    //#include <Ethernet.h>
    //#include <EthernetUdp.h>
    #ifdef TCPSERVER
      // comment/uncomment according to ENC28J60 definition
      //#include <EthernetServer.h>
    #endif
  #endif
#endif
/////////////////////////////////////////////////////////////////////////
// END Tricky solution for arduino ide 1.6
/////////////////////////////////////////////////////////////////////////

#include <Time.h>
#include <aJSON.h>
#include <Wire.h>

#ifdef REPEATTASK
#include <TimeAlarms.h>
#endif

#if defined(ETHERNETMQTT) || defined(GSMGPRSMQTT)
#include <PubSubClient.h>
#endif

#ifdef SDCARD
#include <SPI.h>
#include <SdFat.h>
SdFat SD;
File dataFile;
File logFile;

// Data file base name.  Must be six characters or less.
#define FILE_BASE_NAME "RMAP_"
#define MAX_FILESIZE 8388608 // 44h for 5s sampletime
const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
char fileName[BASE_NAME_SIZE+4];
char fullfileName[BASE_NAME_SIZE+8];
char newfileName[BASE_NAME_SIZE+8];

#define STRLEN 63
typedef struct Records{
  bool done;
  char topic[STRLEN] ;
  char separator = ';';
  char payload[STRLEN] ;
} Record;

Record record;
uint32_t pos;

// check if filename with two extensions (.que and .don) exixts
bool exists(char* fileName)
{
  strcpy(fullfileName,fileName);
  strcat (fullfileName,".que");
  IF_SDEBUG(DBGSERIAL.print(F("#check:")));
  IF_SDEBUG(DBGSERIAL.println(fullfileName));
  if (SD.exists(fullfileName)) return true;
  strcpy(newfileName,fileName);
  strcat (newfileName,".don");
  IF_SDEBUG(DBGSERIAL.print(F("#check:")));
  IF_SDEBUG(DBGSERIAL.println(newfileName));
  if (SD.exists(newfileName)) return true;
  IF_SDEBUG(DBGSERIAL.println(F("#check false")));
  return false;
}

// compute numbered filename from 000 to 999 
void nextName(char* fileName)
{

  IF_SDEBUG(DBGSERIAL.print(F("#nextName  in:")));
  IF_SDEBUG(DBGSERIAL.println(fileName));

  if (fileName[BASE_NAME_SIZE + 2] != '9') {
    fileName[BASE_NAME_SIZE + 2]++;
  } else if (fileName[BASE_NAME_SIZE +1] != '9') {
    fileName[BASE_NAME_SIZE + 2] = '0';
    fileName[BASE_NAME_SIZE+1]++;
  } else if (fileName[BASE_NAME_SIZE] != '9') {
	  fileName[BASE_NAME_SIZE + 2] = '0';
	  fileName[BASE_NAME_SIZE + 1] = '0';
	  fileName[BASE_NAME_SIZE]++;
  } else {
    IF_SDEBUG(DBGSERIAL.println("#Can't create file name"));
    // Wait forever since we cant write data
    //while (1) ;
  }
  IF_SDEBUG(DBGSERIAL.print(F("#nextName out:")));
  IF_SDEBUG(DBGSERIAL.println(fileName));
}

#endif
#ifdef FREERAM
/*
// this function will return the number of bytes currently free in RAM
// written by David A. Mellis
// based on code by Rob Faludi http://www.faludi.com
int freeRam() {
  int size = 2048; // Use 2048 with ATmega328
  byte *buf;

  while ((buf = (byte *) malloc(--size)) == NULL)
    ;

  free(buf);

  return size;
}
*/

// this compute the the difference fron heap and stack
// ehwn heap and stack overload an crash happen
int freeRam ()
{
  //DBGSERIAL.println(__malloc_margin);

  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

#endif

#ifdef FREEMEM
//Code to print out the free memory
// more sophisticated method to compute free memory also fragmented by C memory management 
struct __freelist {
  size_t sz;
  struct __freelist *nx;
};

extern char * const __brkval;
extern struct __freelist *__flp;

uint16_t freeMem(uint16_t *biggest)
{
  char *brkval;
  char *cp;
  unsigned freeSpace;
  struct __freelist *fp1, *fp2;
  
  brkval = __brkval;
  if (brkval == 0) {
    brkval = __malloc_heap_start;
  }
  cp = __malloc_heap_end;
  if (cp == 0) {
    cp = ((char *)AVR_STACK_POINTER_REG) - __malloc_margin;
  }
  if (cp <= brkval) return 0;
   
  freeSpace = cp - brkval;
  
  for (*biggest = 0, fp1 = __flp, fp2 = 0;
       fp1;
       fp2 = fp1, fp1 = fp1->nx) {
    if (fp1->sz > *biggest) *biggest = fp1->sz;
    freeSpace += fp1->sz;
  }
   
  return freeSpace;
}


void freeMem(char* message) {

  uint16_t biggest;

  IF_SDEBUG(DBGSERIAL.print(message));
  IF_SDEBUG(DBGSERIAL.print(F(":\t")));
  IF_SDEBUG(DBGSERIAL.print(freeMem(&biggest)));
  IF_SDEBUG(DBGSERIAL.print(F(" biggest:")));
  IF_SDEBUG(DBGSERIAL.println(biggest));
}

#endif

#ifdef SERIAL_DEBUG
// for RF24* dubug
//http://playground.arduino.cc/Main/Printf
// we need fundamental FILE definitions and printf declarations
#include <stdio.h>

// create a FILE structure to reference our UART output function
static FILE uartout = {0} ;

// create a output function
// This works because Serial.write, although of
// type virtual, already exists.
static int uart_putchar (char c, FILE *stream)
{
  DBGSERIAL.write(c) ;
  return 0 ;
}
#endif

#ifdef LCD
#include <Wire.h>
#include <YwrobotLiquidCrystal_I2C.h>
/* Initialise the LiquidCrystal library. The default address is 0x27 and this is a 16x2 or 16x4 line display */
LiquidCrystal_I2C lcd(0x27,20,4);
#endif

#ifdef RTCPRESENT
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
#endif

#ifdef I2CGPSPRESENT
#include <GPSRTC.h>  // a library that returns GPS time as a time_t
#endif

#include <EEPROM.h>
#include "EEPROMAnything.h"

#if defined (JSONRPCON)
// include the JsonRPC library
#include <JsonRPC.h>
#endif

#if defined (SENSORON)
#include <SensorDriver.h>

  #if defined (RADIORF24)
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>


// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(RF24CEPIN,RF24CSPIN);

// Network uses that radio
RF24Network network(radio);

    // AES is defined inside RF24Network library
    #if defined (AES)
#include <AESLib.h>

// default AES key and iv
uint8_t key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
uint8_t iv[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

void aes_enc(uint8_t* key, uint8_t* iv, char* mainbuf, size_t* buflen){
  if (*buflen % 16 != 0) *buflen = (int(*buflen/16)*16) + 16;
  //IF_SDEBUG(DBGSERIAL.print(F("#encode string  :")));
  //IF_SDEBUG(DBGSERIAL.println(*buflen));
  //IF_SDEBUG(DBGSERIAL.println(mainbuf));
  #ifdef FREEMEM
    freeMem("#free mem in aes_enc");
  #endif
  #ifdef FREERAM
    IF_SDEBUG(DBGSERIAL.print(F("#free ram on aes_enc: ")));
    IF_SDEBUG(DBGSERIAL.println(freeRam()));
  #endif
  aes128_cbc_enc(key, iv, mainbuf, *buflen);
  //IF_SDEBUG(DBGSERIAL.print(F("#encoded string :")));
  //IF_SDEBUG(DBGSERIAL.write(mainbuf,*buflen));
  //IF_SDEBUG(DBGSERIAL.println(F("#")));
}
void aes_dec(uint8_t* key, uint8_t* iv, char* mainbuf, size_t* buflen){
  if (*buflen % 16 != 0) *buflen = (int(*buflen/16)*16) + 16;
  //IF_SDEBUG(DBGSERIAL.print(F("#decode string :")));
  //IF_SDEBUG(DBGSERIAL.write(mainbuf,*buflen));
  //IF_SDEBUG(DBGSERIAL.println(F("#")));
  #ifdef FREEMEM
    freeMem("#free mem in aes_dec");
  #endif
  #ifdef FREERAM
  IF_SDEBUG(DBGSERIAL.print(F("#free ram on aes_dec: ")));
  IF_SDEBUG(DBGSERIAL.println(freeRam()));
  #endif
  aes128_cbc_dec(key, iv, mainbuf, *buflen);
  //IF_SDEBUG(mainbuf[*buflen-1]='\0');
  //IF_SDEBUG(DBGSERIAL.print(F("#decoded string:")));
  //IF_SDEBUG(DBGSERIAL.println(mainbuf));
}
    #endif
  #endif

#include <avr/wdt.h>

  #ifdef I2CGPSPRESENT
#include "registers.h"
  #endif

  #ifdef GSMGPRSMQTT
#include "sim800Client.h"
sim800Client s800;
#define IMEICODE_LEN 16
char imeicode[IMEICODE_LEN];
int rssi, ber;
  #endif

  #ifdef GSMGPRSHTTP
#include "sim800.h"
SIM800 s800;
int rssi, ber;
  #endif


  #if defined(GSMGPRSRTC) || defined(GSMGPRSRTCBOOT)
time_t scantime(const char *buf)
{
  tmElements_t tm;
  int token_count = sscanf(buf,"%02hhd/%02hhd/%02hhd,%02hhd:%02hhd:%02hhd+00\n",&tm.Year,&tm.Month,&tm.Day,&tm.Hour,&tm.Minute,&tm.Second);
  //tm.Wday
  if (token_count == 6){
    tm.Year = y2kYearToTm(tm.Year);
    return(makeTime(tm));
  }
  return 0UL;
}
  #endif

// sensor information
struct sensor_t
{
  char driver[SENSORDRIVER_DRIVER_LEN];         // driver name
  int node;                                 // RF24Nework node id
  char type[SENSORDRIVER_TYPE_LEN];         // sensor type name
  int address;                              // i2c address
  char mqttpath[SENSORDRIVER_MQTTPATH_LEN]; // path for mqtt pubblish
  sensor_t() : address(-1) {
       driver[0]='\0';
       node = -1;
       type[0]='\0';
       mqttpath[0]='\0';
  }
};

struct driver_t   // use this to instantiate a driver
{
  SensorDriver* manager;
  driver_t() : manager(NULL) {}

  int setup(const char* driver, int node, const char* type, int address
    #if defined (AES)
		       , uint8_t* key, uint8_t* iv
    #endif
	      )
  {
    if (manager != NULL)
      delete manager;
    manager = SensorDriver::create(driver,type);
    if (manager == NULL)
      return -2;

    if (manager->setup(driver, address, node, type
      #if defined (RADIORF24)
		       , mainbuf,sizeof(mainbuf), &network
        #if defined (AES)
		       , key, iv
        #endif
      #endif
		       ) != 0) return -1;
    return 0;
  }
} drivers[SENSORS_LEN];
#endif

char confver[7] = CONFVER; // version of configuration saved on eeprom

struct config_t                   // configuration to save and load fron eeprom
{
  int rt;                                // sample time for mqtt  (seconds)
  char mqttrootpath[MQTTROOTPATH_LEN];   // root path for mqtt publish
  char ntpserver[SERVER_LEN];            // ntp server
  char mqttserver[SERVER_LEN];           // server for mqtt publish
  char mqttuser[SERVER_LEN];             // user for mqtt publish
  char mqttpassword[SERVER_LEN];         // password for mqtt publish
#if defined (AES)
  uint8_t key[16];                       // AES key
  uint8_t iv[16];                        // AES CBC iv
#endif
  uint16_t thisnode;                     // Address of our RF24 Network node
  int channel;                           // Channel for RF24 Network
  byte mac[6];                           // Ethernet mac address

#if defined (SENSORON)
  sensor_t sensors[SENSORS_LEN];  // vector with sensor configuration
  // Return the index (>= 0) of the new sensors driver,
  // -1 if the sensors array is full
  // -2 if something wrong happened during sensor creation
  int add_device(const char* driver,int node,const char* type,int address, const char* mqttpath) {      // ad a device
    for (int i = 0; i < SENSORS_LEN; i++) {
      if (sensors[i].address == -1) {
	strncpy(sensors[i].driver, driver, SENSORDRIVER_DRIVER_LEN-1);
	sensors[i].driver[SENSORDRIVER_DRIVER_LEN-1]='\0';
	strncpy(sensors[i].type, type, SENSORDRIVER_TYPE_LEN-1);
	sensors[i].type[SENSORDRIVER_TYPE_LEN-1]='\0';
	sensors[i].address=address;
	sensors[i].node=node;
	strncpy(sensors[i].mqttpath, mqttpath, SENSORDRIVER_MQTTPATH_LEN-1);
	sensors[i].mqttpath[SENSORDRIVER_MQTTPATH_LEN-1]='\0';
	return i;
      }
    }
    return -1;
  }
  // Return the index (>= 0) of the requested sensor
  // -1 if not found
  int get_device(const char* driver, int node, const char* type, int address) {          // return a device
    for (int i = 0; i < SENSORS_LEN; i++) {
      if (strcmp(sensors[i].driver, driver) == 0 &&
	  strcmp(sensors[i].type, type) == 0 &&
	  sensors[i].address == address &&
	  sensors[i].node == node) return i;
    }
    return -1;
  }
#endif

  void save () {
    int p=0;                                                              // save to eeprom
    p+=EEPROM_writeAnything(p, confver);
    p+=EEPROM_writeAnything(p, rt);
    p+=EEPROM_writeAnything(p, mqttrootpath);
    p+=EEPROM_writeAnything(p, ntpserver);
    p+=EEPROM_writeAnything(p, mqttserver);
    p+=EEPROM_writeAnything(p, mqttuser);
    p+=EEPROM_writeAnything(p, mqttpassword);
#if defined (AES)
    p+=EEPROM_writeAnything(p, key);
    p+=EEPROM_writeAnything(p, iv);
#endif
    p+=EEPROM_writeAnything(p, thisnode);
    p+=EEPROM_writeAnything(p, channel);
    p+=EEPROM_writeAnything(p, mac);
#if defined (SENSORON)
    p+=EEPROM_writeAnything(p, sensors);
#endif
  }
  
  bool load () {                                                          // load from eeprom
    int p=0;
    char ver[7];
    p+=EEPROM_readAnything(p, ver);
    if (strcmp(ver,confver ) == 0){ 
      p+=EEPROM_readAnything(p, rt);
      p+=EEPROM_readAnything(p, mqttrootpath);
      p+=EEPROM_readAnything(p, ntpserver);
      p+=EEPROM_readAnything(p, mqttserver);
      p+=EEPROM_readAnything(p, mqttuser);
      p+=EEPROM_readAnything(p, mqttpassword);
#if defined (AES)
      p+=EEPROM_readAnything(p, key);
      p+=EEPROM_readAnything(p, iv);
#endif
      p+=EEPROM_readAnything(p, thisnode);
      p+=EEPROM_readAnything(p, channel);
      p+=EEPROM_readAnything(p, mac);
#if defined (SENSORON)
      p+=EEPROM_readAnything(p, sensors);
#endif
      return true;
    }
    else{
      return false;
    }
  }
} configuration;

boolean configured;
byte mac[]={0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

#if defined(ETHERNETMQTT) || defined(GSMGPRSMQTT)
//declare functions used below
void mqttcallback(char* topic, byte* payload, unsigned int length);
#endif

#if defined(GSMGPRSMQTT)
PubSubClient mqttclient(configuration.mqttserver, 1883, mqttcallback, s800);
#endif

#ifdef ETHERNETMQTT
EthernetClient ethclient;
PubSubClient mqttclient(configuration.mqttserver, 1883, mqttcallback, ethclient);
#endif

#if defined(ETHERNETMQTT) || defined(GSMGPRSMQTT)

bool rmapconnect()
{

#ifdef ETHERNETON
  // set mqttid to username+mac address
  /*
    The client identifier (short ClientId) is an identifier of each MQTT client 
    connecting to a MQTT broker. As the word identifier already suggests, 
    it should be unique per broker. The broker uses it for identifying the 
    client and the current state of the client. If you don’t need a state to be 
    hold by the broker, in MQTT 3.1.1 (current standard) it is also possible 
    to send an empty ClientId, which results in a connection without any state. 
    A condition is that clean session is true, otherwise the connection will be 
    rejected.
  */
  // TODO: try to use  empty ClientId

  char mqttid[SERVER_LEN+13];

  snprintf(mqttid,sizeof(mqttid), "%s-%02x%02x%02x%02x%02x%02x", configuration.mqttuser,configuration.mac[0], configuration.mac[1], configuration.mac[2], configuration.mac[3], configuration.mac[4], configuration.mac[5]);

#endif //ETHERNETON

#ifdef GSMGPRSMQTT
  // IMEI code from sim800
  char mqttid[IMEICODE_LEN];
  snprintf(mqttid,sizeof(mqttid), "%s", imeicode);
#endif

  IF_SDEBUG(DBGSERIAL.print(F("#try connect mqtt id: ")); DBGSERIAL.println(mqttid));
  strcpy (mainbuf,configuration.mqttrootpath);
  strcat (mainbuf,"-,-,-/-,-,-,-/B01213");
  if (mqttclient.connect(mqttid,configuration.mqttuser,configuration.mqttpassword,mainbuf,1,1,"{\"v\":\"error01\"}")){
    wdt_reset();
    IF_SDEBUG(DBGSERIAL.println(F("#mqtt connected")));
    IF_LCD(lcd.setCursor(0,3)); 
    IF_LCD(lcd.print(F("MQTT: connected")));
    
    // subcribe to incoming topic

#ifdef ETHERNETON

    char topiccom [strlen(MQTTRPCPREFIX)+(SERVER_LEN-1)+6*2+5+1];
    snprintf(topiccom,sizeof(topiccom),"%s%s/%02x%02x%02x%02x%02x%02x/com", MQTTRPCPREFIX,configuration.mqttuser,configuration.mac[0], configuration.mac[1], configuration.mac[2], configuration.mac[3], configuration.mac[4], configuration.mac[5]);
#endif

#ifdef GSMGPRSMQTT

    char topiccom [strlen(MQTTRPCPREFIX)+(SERVER_LEN-1)+strlen(imeicode)+5+1];
    // IMEI code from sim800
    snprintf(topiccom,sizeof(topiccom), "%s%s/%s/com", MQTTRPCPREFIX,configuration.mqttuser,imeicode);
#endif

    // QoS=1
    mqttclient.subscribe(topiccom,1);
    IF_SDEBUG(DBGSERIAL.print(F("#mqtt subscribed to: ")));
    IF_SDEBUG(DBGSERIAL.println(topiccom));
    wdt_reset();
    
    if (!mqttclient.publish(mainbuf,(uint8_t*)"{\"v\":\"conn\"}", 12,1)){
      IF_SDEBUG(DBGSERIAL.print(F("#mqtt ERROR publish status")));
    }
    return true;
  }else{
    wdt_reset();
    IF_SDEBUG(DBGSERIAL.println(F("#mqtt connection failed")));
    IF_LCD(lcd.setCursor(0,3)); 
    IF_LCD(lcd.print(F("MQTT: error    ")));
    return false;
  }
}

bool rmapdisconnect()
{
  strcpy (mainbuf,configuration.mqttrootpath);
  strcat (mainbuf,"-,-,-/-,-,-,-/B01213");
  if (!mqttclient.publish(mainbuf,(uint8_t*)"{\"v\":\"disconn\"}", 15,1)){
    IF_SDEBUG(DBGSERIAL.print(F("#mqtt ERROR publish status")));
  }
  mqttclient.disconnect();
  IF_SDEBUG(DBGSERIAL.println(F("#mqtt disconnected")));
  wdt_reset();
  
  IF_LCD(lcd.setCursor(0,3)); 
  IF_LCD(lcd.print(F("MQTT: disconnec")));
  return true;
}

#endif

#ifdef ETHERNETON
#ifdef DEBUGONSERIAL
// Just a utility function to nicely format an IP address.
const char* ip_to_str(const IPAddress& ipAddr)
{
  static char buf[16];
  snprintf(buf,sizeof(buf),"%d.%d.%d.%d", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
  return buf;
}
#endif

// A UDP and TCP instance
EthernetUDP Udp;

#ifdef TCPSERVER
EthernetServer ethServer = EthernetServer(ETHERNETPORT);
#endif

#endif

time_t t;

#if defined (JSONRPCON)
aJsonObject *response=NULL ,*result=NULL ;  // ,*error=NULL;

// initialize an instance of the JsonRPC library for registering 
// local method
// compute how many RPC I have

#define NJSRPCATT
#define NJSRPCSEN
#define NJSRPCRAD
#define NJSRPCSDC
#define NJSRPCREB

#if defined (ATTUATORE)
#define NJSRPCATT  +1
#endif
#if defined (SENSORON)
#define NJSRPCSEN  +4
#endif
#if defined (RADIORF24)
#define NJSRPCRAD  +1
#endif
#if defined (SDCARD)
#define NJSRPCSDC  +1
#endif
#if defined (REBOOTRPC)
#define NJSRPCREB  +1
#endif

JsonRPC rpc(0 NJSRPCATT NJSRPCSEN NJSRPCRAD NJSRPCSDC NJSRPCREB);

// initialize a serial json stream for receiving json objects
// through a serial/USB connection
  #ifdef SERIALJSONRPC
  aJsonStream stream(&RPCSERIAL);
  #endif
#endif

#ifdef GSMGPRSHTTP

/* Utility function: Prepends t into s. Assumes s has enough space allocated
** for the combined string.
*/
void prepend(char* s, const char* t)
{
  size_t len = strlen(t);
  size_t i;
  
  memmove(s + len, s, strlen(s) + 1);
  
  for (i = 0; i < len; ++i)
    {
      s[i] = t[i];
    }
}
#endif


#ifdef NTPON
/*
 Udp NTP Client
 Get the time from a Network Time Protocol (NTP) time server

 created 4 Sep 2010 
 by Michael Margolis
 modified 9 Apr 2012
 by Tom Igoe
*/

#define NTP_PACKET_SIZE 48               // NTP time stamp is in the first 48 bytes of the message

// send an NTP request to the time server
//unsigned long sendNTPpacket(IPAddress& address)
void sendNTPpacket()
{
  byte packetBuffer[ NTP_PACKET_SIZE];        //buffer to hold incoming and outgoing packets 

  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49; 
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp: 		   
  Udp.beginPacket(configuration.ntpserver, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket(); 
  IF_SDEBUG(DBGSERIAL.println(F("#ntp packet sended")));
}

time_t receiveNTPpacket(){
  
  byte packetBuffer[ NTP_PACKET_SIZE];        //buffer to hold incoming and outgoing packets 

  int count  = 0;
  while (  count++  < 10 ){
    if ( Udp.parsePacket() > 0 ) {
      // We've received a packet, read the data from it
      if (Udp.read(packetBuffer,NTP_PACKET_SIZE)< NTP_PACKET_SIZE ){   // read the packet into the buffer
	IF_SDEBUG(DBGSERIAL.println(F("#error getting short ntp packet response")));
	return 0UL;
      }

      //the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, esxtract the two words:
      
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;  
      IF_SDEBUG(DBGSERIAL.print(F("#NTP response: seconds since Jan 1 1900 = ")));
      IF_SDEBUG(DBGSERIAL.println(secsSince1900));
      const unsigned long seventy_years = 2208988800UL;
      return secsSince1900 -  seventy_years;
    }
    delay(500);
  }
      
  IF_SDEBUG(DBGSERIAL.println(F("#error getting ntp packet response")));
  return 0UL;
}

time_t getNtpTime()
{
  sendNTPpacket();            // send an NTP packet to a time server
  return receiveNTPpacket();  // get a reply if it is available
}

#endif

#if defined(DEBUGONSERIAL)     

// utility function to debug
void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
    DBGSERIAL.print('0');
  DBGSERIAL.print(digits);
}

// utility function to debug
void digitalClockDisplay(time_t t ){
  // digital clock display of the time

  DBGSERIAL.print(F("#"));

  if (t == 0UL){
    DBGSERIAL.println(F("NULL"));
  }
  else{	  
    printDigits(hour(t));
    DBGSERIAL.print(":");
    printDigits(minute(t));
    DBGSERIAL.print(":");
    printDigits(second(t));
    DBGSERIAL.print(" ");
    printDigits(day(t));
    DBGSERIAL.print(" ");
    printDigits(month(t));
    DBGSERIAL.print(" ");
    DBGSERIAL.print(year(t)); 
    DBGSERIAL.println(); 
  }
}

#endif

#if defined(LCD)     

void lcdDigits(int digits){
  // utility function for digital clock display: lcd display preceding colon and leading 0
  if(digits < 10)
    lcd.print('0');
  lcd.print(digits);
}

// utility function to print to LCD
void LcdDigitalClockDisplay(time_t t ){
  // digital clock display of the time

  if (t == 0UL){
    lcd.print(F("time was not set"));
  }
  else{	  
    lcdDigits(hour(t));
    lcd.print(":");
    lcdDigits(minute(t));
    lcd.print(":");
    lcdDigits(second(t));
    lcd.print(" ");
    lcdDigits(day(t));
    lcd.print(" ");
    lcdDigits(month(t));
    lcd.print(" ");
    lcd.print(year(t)); 
  }
}
#endif

#ifdef SDCARDLOGFILE

bool msgconnected=false;

void logDigits(int digits){
  // utility function for digital clock display: log display preceding colon and leading 0
  char str[(sizeof(int)*8+1)];
  if(digits < 10)
    IF_LOGFILE("0");
  sprintf(str,"%d",digits);
  IF_LOGFILE(str);
}


// utility function to print to Logfile
void LogDigitalClockDisplay(){
  // digital clock display of the time

  IF_SDEBUG(DBGSERIAL.print(F("#write on the log file: ")));

  if(timeStatus()== timeNotSet) {
    IF_SDEBUG(DBGSERIAL.println(F("#The time has never been set")));
    //return;
    time_t t = 0; // Store the current time in time variable t 

  }else{
    time_t t = now(); // Store the current time in time variable t 
  }

  if (t == 0UL){
    IF_LOGFILE("#time was not set:");
  }
  else{	  
    logDigits(year(t)); 
    IF_LOGFILE("-");
    logDigits(month(t));
    IF_LOGFILE("-");
    logDigits(day(t));
    IF_LOGFILE(" ");
    logDigits(hour(t));
    IF_LOGFILE(":");
    logDigits(minute(t));
    IF_LOGFILE(":");
    logDigits(second(t));
    IF_LOGFILE("> ");
  }
}

#define IF_LOGDATEFILE(x) ({LogDigitalClockDisplay(); IF_LOGFILE(x);logFile.flush();})
#else
#define IF_LOGDATEFILE(x)
#endif

#if defined (JSONRPCON)

#if defined (REBOOTRPC)
int rebootrpc(aJsonObject* params)
{

  IF_LOGDATEFILE("jrpc reboot\n");
  Reboot();

  //result = aJson.createObject();
  //return E_SUCCESS;  
}
#endif

#if defined (ATTUATORE)

// This switch on/off pins on board
// call rpc example:
// {"jsonrpc": "2.0", "method": "togglepin", "params": [{"n":4,"s":true},{"n":5,"s":false}], "id": 0}
// {"jsonrpc": "2.0", "method": "togglepin", "params": [{"n":4,"s":false},{"n":5,"s":true}], "id": 0}
//

const uint8_t pins [] = {OUTPUTPINS};

int togglePin(aJsonObject* params)
{
  aJsonObject *pinobj = params->child;
  if (!pinobj)   return E_INTERNAL_ERROR;

  do {
    aJsonObject* numberParam = aJson.getObjectItem(pinobj, "n");
    if (!numberParam)   return E_INTERNAL_ERROR;
    
    aJsonObject* statusParam = aJson.getObjectItem(pinobj, "s");
    if (!statusParam)   return E_INTERNAL_ERROR;
	
    int requestedPin = numberParam -> valueint;
    boolean requestedStatus = statusParam -> valuebool;

    bool pinok = false;
    for (uint8_t i=0;i < sizeof(pins)/sizeof(*pins) ;i++) 
      if (requestedPin == pins[i]) pinok=true;
    if (!pinok) return E_INTERNAL_ERROR;

    IF_SDEBUG(DBGSERIAL.print(F("#togglepin : pin: ")));
    IF_SDEBUG(DBGSERIAL.print(requestedPin));
    IF_SDEBUG(DBGSERIAL.print(F(" status: ")));
    IF_SDEBUG(DBGSERIAL.println(requestedStatus));

    IF_LOGDATEFILE("jrpc togglepin\n");

    digitalWrite(requestedPin, requestedStatus);
    pinobj=pinobj->next;
  } while ( pinobj );

  result = aJson.createObject();

  // aJson.addNumberToObject(result, "value", requestedStatus);
  // aJson.addStringToObject(result, "description", "Led status");

  return E_SUCCESS;  
}
#endif


#if defined (RADIORF24)

bool radioavailabletimeout(unsigned long timeout) {

  unsigned long start_at = millis();

  while (true){
    network.update();
    if (network.available())              return true;
    if ( ( millis() - start_at) > timeout ) return false;
  }
}


// RPC for an rpc for an other module like a nexted RPC over RF24 transport
// call rpc example:
// {"jsonrpc":"2.0","method":"rf24rpc","params":{"node":1,"jsonrpc":"2.0","method":"configure","params":{"reset":true},"id":0},"id":0}

int rf24rpc(aJsonObject* params)
{
  aJsonObject* nodeParam = aJson.getObjectItem(params, "node");
  if (nodeParam) {
    int node = nodeParam -> valueint;
    aJson.deleteItemFromObject(params,"node");
    IF_SDEBUG(DBGSERIAL.print(F("#rf24rpc : node -> ")); DBGSERIAL.println(node));

    IF_LOGDATEFILE("jrpc rf24rpc\n");

    /*
    aJsonObject* remotecmdParam = aJson.getObjectItem(params, "remotecmd");

    if (remotecmdParam) {
      char* message = {remotecmdParam -> valuestring};
    */

    RF24NetworkHeader header( node,0);

    aJson.print(params,mainbuf, sizeof(mainbuf));
    IF_SDEBUG(DBGSERIAL.print(F("#message: ")));
    IF_SDEBUG(DBGSERIAL.println(mainbuf));

    size_t buflen=strlen(mainbuf)+1;
#if defined (AES)
    aes_enc(configuration.key, configuration.iv, mainbuf, &buflen);
#endif
    bool ok = network.write(header,mainbuf,buflen);

    result = aJson.createObject();
    if (ok) {
      aJson.addTrueToObject(result, "send");
    }else{
      aJson.addFalseToObject(result, "send");
      return E_SUCCESS;  
      //return E_INTERNAL_ERROR;
    }

    size_t size = 0;
    if (radioavailabletimeout(500)){
      size = network.read(header,mainbuf,sizeof(mainbuf));
#if defined (AES)
    aes_dec(configuration.key, configuration.iv, mainbuf,&size);
#endif
    }
    if (size >0){
      mainbuf[size-1]='\0';
      aJson.addTrueToObject(result, "receive");
      aJsonObject *noderesponse = aJson.parse(mainbuf);
      aJson.addItemToObject(result, "noderesponse",noderesponse );
    }else{
      aJson.addFalseToObject(result, "receive");
      return E_SUCCESS;  
      //return E_INTERNAL_ERROR;
    }
  }
  return E_SUCCESS;  
}
#endif


// this reset/configure/save parameters
// call rpc example:
// {"jsonrpc": "2.0", "method": "configure", "params": {"reset":true}, "id": 0}
// {"jsonrpc": "2.0", "method": "configure", "params": {"mqttrootpath":"meteo/-/1012345,4412345/rmap/"}, "id": 0}
// {"jsonrpc": "2.0", "method": "configure", "params": {"ntpserver":"0.fedora.pool.ntp.org"}, "id": 0}
// {"jsonrpc": "2.0", "method": "configure", "params": {"ntpserver":"pat1"}, "id": 0}
// {"jsonrpc": "2.0", "method": "configure", "params": {"mqttserver":"r-map.org"}, "id": 0}
// {"jsonrpc": "2.0", "method": "configure", "params": {"mqttserver":"asus-pat1.pat1.dpn"}, "id": 0}
// {"jsonrpc": "2.0", "method": "configure", "params": {"type":"TMP","node":0,"address":72 ,"mqttpath":"254,0,0/105,2000,-,-/"}, "id": 0}
// {"jsonrpc": "2.0", "method": "configure", "params": {"type":"ADT","node":0,"address":75 ,"mqttpath":"254,0,0/105,1000,-,-/"}, "id": 0}
// {"jsonrpc": "2.0", "method": "configure", "params": {"type":"TMP","node":1,"address":72 ,"mqttpath":"254,0,0/105,2000,-,-/"}, "id": 0}
// {"jsonrpc": "2.0", "method": "configure", "params": {"mqttsampletime":5}, "id": 0}
// {"jsonrpc": "2.0", "method": "configure", "params": {"save":true}, "id": 0}
// {"jsonrpc": "2.0", "method": "configure", "params": {"date":[2014,2,10,18,45,18]}, "id": 0}
//

 int mgrConfiguration(aJsonObject* params)
{
  boolean reset=false;
  boolean save=false;
  result = aJson.createObject();

  IF_SDEBUG(DBGSERIAL.println(F("#mgrConfiguration")));
  IF_LOGDATEFILE("jrpc configure\n");

  // set config version
  strcpy (confver,CONFVER);

  aJsonObject* resetParam = aJson.getObjectItem(params, "reset");
  if (resetParam)
     reset = resetParam -> valuebool;

  if (reset){

    IF_SDEBUG(DBGSERIAL.println(F("#reset")));

    configured=false;

    // reset config version
    confver[0]='\0';
    configuration.rt=5;
    configuration.ntpserver[0]='\0';
    configuration.mqttserver[0]='\0';
    configuration.mqttuser[0]='\0';
    configuration.mqttpassword[0]='\0';
#if defined(AES)
    memcpy(configuration.key, key, 16*sizeof(uint8_t));
    memcpy(configuration.iv, iv, 16*sizeof(uint8_t));
#endif
    configuration.mqttrootpath[0]='\0';
    configuration.thisnode=-1;
    configuration.channel=93;
    // Enter a MAC address for your controller below.
    // Newer Ethernet shields have a MAC address printed on a sticker on the shield
    memcpy(configuration.mac, mac, 6*sizeof(byte));


#if defined(SENSORON)    
    for (int i = 0; i < SENSORS_LEN; i++) {
        configuration.sensors[i].type[0]='\0';
        configuration.sensors[i].address= -1 ;
    }
#endif
    aJson.addTrueToObject(result, "reset");
  }

#if defined (SENSORON)
  aJsonObject* driverParam = aJson.getObjectItem(params, "driver");

  if (driverParam) {
    IF_SDEBUG(DBGSERIAL.println(F("#sensor")));

    char* driver = {driverParam -> valuestring};

    aJsonObject* typeParam = aJson.getObjectItem(params, "type");
    char* type = typeParam -> valuestring;

    aJsonObject* addressParam = aJson.getObjectItem(params, "address");
    int address = addressParam -> valueint;

    aJsonObject* nodeParam = aJson.getObjectItem(params, "node");
    int node = nodeParam -> valueint;

    aJsonObject* mqttpathParam = aJson.getObjectItem(params, "mqttpath");
    char* mqttpath = mqttpathParam -> valuestring;

    int id = configuration.add_device(driver,node,type,address,mqttpath);

    if (!drivers[id].setup(driver, node, type, address
        #if defined (AES)
			   , configuration.key, configuration.iv
        #endif
			   ) == SD_SUCCESS) {			   
			   IF_SDEBUG(DBGSERIAL.print(F("#sensor not present or broken")));
			   // comment the next line to be less restrictive
			   //return E_INTERNAL_ERROR;
			   }
    aJson.addNumberToObject(result, "id",id);

#endif
  }
  aJsonObject* saveParam = aJson.getObjectItem(params, "save");
  if (saveParam)
    save = saveParam -> valuebool;
  
  //aJson.deleteItem(saveParam);

  if (save){
    IF_SDEBUG(DBGSERIAL.println(F("#save")));
    configuration.save();
    //EEPROM_writeAnything(0, configuration);
    aJson.addTrueToObject(result, "save");
  }
  
  aJsonObject* mqttSampleTime = aJson.getObjectItem(params, "mqttsampletime");
  if (mqttSampleTime){
    configuration.rt = mqttSampleTime -> valueint;
    // need a reset to take it in account
    aJson.addNumberToObject(result, "mqttsampletime",configuration.rt);
  }

  aJsonObject* mqttRootPath = aJson.getObjectItem(params, "mqttrootpath");
  if (mqttRootPath){
    strncpy(configuration.mqttrootpath,mqttRootPath -> valuestring,MQTTROOTPATH_LEN-1);
    configuration.mqttrootpath[MQTTROOTPATH_LEN-1]='\0';
    aJson.addStringToObject(result, "mqttrootpath",configuration.mqttrootpath);
  }

  aJsonObject* ntpServer = aJson.getObjectItem(params, "ntpserver");
  if (ntpServer){
    strncpy(configuration.ntpserver,ntpServer -> valuestring,SERVER_LEN-1);
    configuration.ntpserver[SERVER_LEN-1]='\0';
    aJson.addStringToObject(result, "ntpserver",configuration.ntpserver);
  }

  aJsonObject* mqttServer = aJson.getObjectItem(params, "mqttserver");
  if (mqttServer){
    strncpy(configuration.mqttserver,mqttServer -> valuestring,SERVER_LEN-1);
    configuration.mqttserver[SERVER_LEN-1]='\0';
    aJson.addStringToObject(result, "mqttserver",configuration.mqttserver);
  }

  aJsonObject* mqttUser = aJson.getObjectItem(params, "mqttuser");
  if (mqttUser){
    strncpy(configuration.mqttuser,mqttUser -> valuestring,SERVER_LEN-1);
    configuration.mqttuser[SERVER_LEN-1]='\0';
    aJson.addStringToObject(result, "mqttuser",configuration.mqttuser);
  }

  aJsonObject* mqttPassword = aJson.getObjectItem(params, "mqttpassword");
  if (mqttPassword){
    strncpy(configuration.mqttpassword,mqttPassword -> valuestring,SERVER_LEN-1);
    configuration.mqttpassword[SERVER_LEN-1]='\0';
    aJson.addStringToObject(result, "mqttpassword",configuration.mqttpassword);
  }

#if defined(RADIORF24)
#if defined (AES)
  aJsonObject* keyo = aJson.getObjectItem(params, "key");
  if (keyo){
    if (aJson.getArraySize(keyo) != 16)  return E_INTERNAL_ERROR;
    
    aJsonObject* element;
    for (int i = 0; i < 16; i++) {
      element = aJson.getArrayItem(keyo,i);
      configuration.key[i] = element-> valueint;
    }
    aJson.addStringToObject(result, "key","OK");
  }

  aJsonObject* ivo = aJson.getObjectItem(params, "iv");
  if (ivo){
    
    if (aJson.getArraySize(ivo) != 16)  return E_INTERNAL_ERROR;

    aJsonObject* element;
    for (int i = 0; i < 16; i++) {
      element = aJson.getArrayItem(ivo,i);
      configuration.iv[i] = element-> valueint;
    }
    aJson.addStringToObject(result, "iv","OK");
  }
#endif  
  aJsonObject* thisnode = aJson.getObjectItem(params, "thisnode");
  if (thisnode){
    configuration.thisnode = thisnode -> valueint;
    aJson.addNumberToObject(result, "thisnode",(int)configuration.thisnode);
  }

  aJsonObject* channel = aJson.getObjectItem(params, "channel");
  if (channel){
    configuration.channel = channel -> valueint;
    aJson.addNumberToObject(result, "channel",configuration.channel);
  }

#endif

  aJsonObject* maco = aJson.getObjectItem(params, "mac");
  if (maco){
    
    if (aJson.getArraySize(maco) != 6)  return E_INTERNAL_ERROR;

    aJsonObject* element;
    for (int i = 0; i < 6; i++) {
      element = aJson.getArrayItem(maco,i);
      configuration.mac[i] = element-> valueint;
    }
    aJson.addStringToObject(result, "mac","OK");
  }

#if defined(RTCPRESENT) || defined(GSMGPRSRTC) || defined(GSMGPRSRTCBOOT)

  aJsonObject* Date = aJson.getObjectItem(params, "date");
  if (Date){
     IF_SDEBUG(DBGSERIAL.println(F("#settime")));

     if (aJson.getArraySize(Date) != 6)  return E_INTERNAL_ERROR;

     aJsonObject* element;
     element = aJson.getArrayItem(Date,0);
     int yy = element-> valueint;
     element = aJson.getArrayItem(Date,1);
     int mo = element-> valueint;
     element = aJson.getArrayItem(Date,2);
     int dd = element-> valueint;
     element = aJson.getArrayItem(Date,3);
     int hh = element-> valueint;
     element = aJson.getArrayItem(Date,4);
     int mm = element-> valueint;
     element = aJson.getArrayItem(Date,5);
     int ss = element-> valueint;
    
     setTime(hh,mm,ss,dd,mo,yy); 
#if defined(RTCPRESENT) || defined(GPSRTC)
     if (RTC.set(now()) != 0)  return E_INTERNAL_ERROR;
#endif

     //this for GPSGPRSRTCBOOT work only at boot before tcpstart
#if defined(GPSGPRSRTC) || defined(GPSGPRSRTCBOOT) 
     if (s800.RTCset(now()) != 0)  return E_INTERNAL_ERROR;
#endif
     aJson.addNumberToObject(result, "date",ss);
  }
#endif

  return E_SUCCESS;

}

// RPC that prepare sensors for successive read
// call rpc example:
// {"jsonrpc": "2.0", "method": "prepare", "params": {"node":1, "driver":"TMP", "type":"TMP", "address":72}, "id": 0}
//
// return waittime as a time requested to make a value ready for reading on sensor buffer

int prepare(aJsonObject* params)
{
  IF_SDEBUG(DBGSERIAL.println(F("#rpc prepare")));
  IF_LOGDATEFILE("jrpc prepare\n");

  aJsonObject* nodeParam = aJson.getObjectItem(params, "node");
  if (!nodeParam) {
    IF_SDEBUG(DBGSERIAL.println(F("#error node")));
    return E_INTERNAL_ERROR;
  }
  int node = nodeParam -> valueint;

  aJsonObject* driverParam = aJson.getObjectItem(params, "driver");
  if (!driverParam) {
    IF_SDEBUG(DBGSERIAL.println(F("#error driver")));
    return E_INTERNAL_ERROR;
  }
  char*  driver = driverParam -> valuestring;

  aJsonObject* typeParam = aJson.getObjectItem(params, "type");
  if (!typeParam) {
    IF_SDEBUG(DBGSERIAL.println(F("#error type")));
    return E_INTERNAL_ERROR;
  }
  char*  type = typeParam -> valuestring;

  aJsonObject* addressParam = aJson.getObjectItem(params, "address");
  if (!addressParam) {
    IF_SDEBUG(DBGSERIAL.println(F("#error address")));
    return E_INTERNAL_ERROR;
  }
  int address = addressParam -> valueint;

  // if we are on the same node we change transport
  //if (node == configuration.thisnode)  driver=type;
  if (node == configuration.thisnode)  driver="I2C";

  int id= configuration.get_device(driver, node, type, address);
  IF_SDEBUG(DBGSERIAL.print(F("#driver id:")));
  IF_SDEBUG(DBGSERIAL.println(id));

  if (id == -1){
    return E_INTERNAL_ERROR;
  }

  unsigned long waittime;
  if (!drivers[id].manager->prepare(waittime) == SD_SUCCESS){
    IF_SDEBUG(DBGSERIAL.println(F("#error in prepare")));
    return E_INTERNAL_ERROR;
  }

  result = aJson.createObject();
  aJson.addNumberToObject(result, "waittime", (int)waittime);
  return E_SUCCESS;

}


// RPC that get values from sensors after a prepare RPC
// call rpc example:
// {"jsonrpc": "2.0", "method": "getjson", "params": {"node":1, "driver":"TMP", "type":"TMP", "address":72}, "id": 0}
//
// return json with btable and values

int getjson(aJsonObject* params)
{

  IF_SDEBUG(DBGSERIAL.println(F("#rpc getjson")));
  IF_LOGDATEFILE("jrpc getjson\n");

  aJsonObject* nodeParam = aJson.getObjectItem(params, "node");
  if (!nodeParam) {
    IF_SDEBUG(DBGSERIAL.println(F("#error node")));
    return E_INTERNAL_ERROR;
  }
  int node = nodeParam -> valueint;

  aJsonObject* driverParam = aJson.getObjectItem(params, "driver");
  if (!driverParam) {
    IF_SDEBUG(DBGSERIAL.println(F("#error driver")));
    return E_INTERNAL_ERROR;
  }
  char*  driver = driverParam -> valuestring;

  aJsonObject* typeParam = aJson.getObjectItem(params, "type");
  if (!typeParam) {
    IF_SDEBUG(DBGSERIAL.println(F("#error type")));
    return E_INTERNAL_ERROR;
  }
  char*  type = typeParam -> valuestring;

  aJsonObject* addressParam = aJson.getObjectItem(params, "address");
  if (!addressParam) {
    IF_SDEBUG(DBGSERIAL.println(F("#error address")));
    return E_INTERNAL_ERROR;
  }
  int address = addressParam -> valueint;

  // if we are on the same node we change transport
  //if (node == configuration.thisnode)  driver=type;
  if (node == configuration.thisnode)  driver="I2C";

  int id= configuration.get_device(driver, node, type, address);
  IF_SDEBUG(DBGSERIAL.print(F("#driver id:")));
  IF_SDEBUG(DBGSERIAL.println(id));

  if (id == -1){
    return E_INTERNAL_ERROR;
  }

  result = drivers[id].manager->getJson();

  return E_SUCCESS;

}


// this get the measure from sensor
// example calling rpc:

// {"jsonrpc": "2.0", "method": "prepandget", "params": {"node":1, "driver":"TMP", "type":"TMP", "address":72}, "id": 0}
//
int prepandget(aJsonObject* params)
{
  IF_SDEBUG(DBGSERIAL.println(F("#rpc getvalues")));
  IF_LOGDATEFILE("jrpc prepandget\n");

  aJsonObject* driverParam = aJson.getObjectItem(params, "driver");
  if (!driverParam) {
    IF_SDEBUG(DBGSERIAL.println(F("#error driver")));
    return E_INTERNAL_ERROR;
  }
  char*  driver = driverParam -> valuestring;

  aJsonObject* nodeParam = aJson.getObjectItem(params, "node");
  if (!nodeParam) {
    IF_SDEBUG(DBGSERIAL.println(F("#error node")));
    return E_INTERNAL_ERROR;
  }
  int node = nodeParam -> valueint;

  aJsonObject* typeParam = aJson.getObjectItem(params, "type");
  if (!typeParam) {
    IF_SDEBUG(DBGSERIAL.println(F("#error type")));
    return E_INTERNAL_ERROR;
  }
  char*  type = typeParam -> valuestring;

  aJsonObject* addressParam = aJson.getObjectItem(params, "address");
  if (!addressParam) {
    IF_SDEBUG(DBGSERIAL.println(F("#error address")));
    return E_INTERNAL_ERROR;
  }
  int address = addressParam -> valueint;

  // if we are on the same node we change transport
  //if (node == configuration.thisnode)  driver=type;
  if (node == configuration.thisnode)  driver="I2C";

  int id= configuration.get_device(driver, node, type, address);
  IF_SDEBUG(DBGSERIAL.print(F("#driver id:")));
  IF_SDEBUG(DBGSERIAL.println(id));

  if (id == -1){
    return E_INTERNAL_ERROR;
  }

  unsigned long waittime;
  if (!drivers[id].manager->prepare(waittime) == SD_SUCCESS){
    IF_SDEBUG(DBGSERIAL.println(F("#error in prepare")));
    return E_INTERNAL_ERROR;
  }

  delay(waittime);

  /*
  int temperature;
  if (!drivers[id].manager->get(&temperature) == SD_SUCCESS){
    IF_SDEBUG(DBGSERIAL.println("error in get"));
    return E_INTERNAL_ERROR;
  }
  aJson.addNumberToObject(result, "B12101", temperature );
  //  aJson.addStringToObject(result, "description", "Temperature K");
  */

  result = aJson.createObject();
  result = drivers[id].manager->getJson();

  return E_SUCCESS;

}
#endif


#ifdef REPEATTASK

#ifdef GSMGPRSRTCBOOT
/*
  this function return time from rmap.cc server with http get
*/
time_t gsmhttpRTC() {

  IF_SDEBUG(DBGSERIAL.println(F("#gsmhttpRTC")));

  time_t t=0UL;

  if (s800.isRegistered()) {
    wdt_reset();
    // compose URL
    for (int i = 0; ((i < 3) & (t == 0UL)); i++){ 
      strcpy (mainbuf, "/http2mqtt/?time=t");
      IF_SDEBUG(DBGSERIAL.print(F("#GSM send get:")));
      IF_SDEBUG(DBGSERIAL.println(mainbuf));
      if ((s800.httpGET("rmap.cc", 80,mainbuf, mainbuf, sizeof(mainbuf)))){
	//Print the results.
	IF_SDEBUG(DBGSERIAL.println(F("#GSM Data received:")));
	IF_SDEBUG(DBGSERIAL.println(mainbuf));
	t=scantime(mainbuf);
	IF_SDEBUG(DBGSERIAL.println(F("#time from http")));
	IF_SDEBUG(digitalClockDisplay(t));
      }
    }
  } 
  else {
    IF_SDEBUG(DBGSERIAL.println(F("#s800 not registered cannot get time from http"))); 
  }
  return t;
}

/*
  this function disconnect from mqtt
  get time from GSM RTC
  get time with gsmhttpRTC
  reconnect to mqtt
  return the better time available
*/

time_t periodicResyncGSMRTC() {

  IF_SDEBUG(DBGSERIAL.println(F("#periodicResyncGSMRTC")));

  time_t tt;
  time_t t;

#if defined(ETHERNETMQTT) || defined(GSMGPRSMQTT)
  bool mc;
  mc=mqttclient.connected();
  
  if (mc){
    //disconn clean
    rmapdisconnect();
    s800.TCPstop();
  }
  //s800.stopNetwork();
#endif
  
  // get first guess time from sim800 RTC
  t = s800.RTCget();
  IF_SDEBUG(DBGSERIAL.println(F("#time from sim800 RTC")));
  IF_SDEBUG(digitalClockDisplay(t));

  s800.startNetwork(GSMAPN, GSMUSER, GSMPASSWORD);
  for (int i = 0; ((i < 10) &  !s800.checkNetwork()); i++) {
    s800.stopNetwork();
    delay(1000);
    s800.startNetwork(GSMAPN, GSMUSER, GSMPASSWORD);
  }

  s800.getSignalQualityReport(&rssi,&ber);
  IF_SDEBUG(DBGSERIAL.print(F("#s800 rssi:")));
  IF_SDEBUG(DBGSERIAL.println(rssi));
  IF_SDEBUG(DBGSERIAL.print(F("#s800 ber:")));
  IF_SDEBUG(DBGSERIAL.println(ber));

  // do not use LOGDATEFILE here becouse it call time and go in infinite recursive loop !
  sprintf(mainbuf,"rssi:%d,ber:%d\n",rssi,ber);
  IF_LOGFILE(mainbuf);

  wdt_reset();

  if ((tt=gsmhttpRTC()) != 0UL){
      t=tt;
      s800.RTCset(t);
      IF_SDEBUG(DBGSERIAL.println(F("#set to system time  and sim800 RTC")));
      IF_SDEBUG(digitalClockDisplay(t));
    }
  
  s800.stopNetwork();

#if defined(ETHERNETMQTT) || defined(GSMGPRSMQTT)
  if (mc)    {
      s800.TCPstart(GSMAPN,GSMUSER,GSMPASSWORD);
      for (int i = 0; ((i < 3) & !rmapconnect()); i++) {
        IF_SDEBUG(DBGSERIAL.println("#MQTT connect failed"));
        s800.stopNetwork();
	delay(1000);
        s800.TCPstart(GSMAPN,GSMUSER,GSMPASSWORD);
      }

    }
#endif

  return t;

}

#endif

void Reboot() {
  IF_SDEBUG(DBGSERIAL.println(F("#Reboot")));

  #if defined(ETHERNETMQTT) || defined(GSMGPRSMQTT)
  if (mqttclient.connected()){
    //disconn clean
    rmapdisconnect();

    #ifdef GSMGPRSMQTT
    s800.TCPstop();
    #endif
  }
  #ifdef GSMGPRSHTTP
  s800.stopNetwork();
  #endif
  #endif

  IF_LOGDATEFILE("programmed Reboot\n");

  wdt_enable(WDTO_30MS); while(1) {} 

  // Restarts program from beginning but 
  // does not reset the peripherals and registers
  //asm volatile ("  jmp 0");
}

#ifdef I2CGPSPRESENT

// read current coordinate from i2c GPS
void GPS_latlon_read(int32_t* lat,int32_t* lon)
{  

  Wire.beginTransmission(I2C_GPS_ADDRESS);
  Wire.write(I2C_GPS_LOCATION);
  Wire.endTransmission();

  Wire.requestFrom(I2C_GPS_ADDRESS, 8);

  long start=millis();
  while((millis()-start) < 100){
    if (Wire.available()>=8) break;
  }
  if (Wire.available()<8){
    IF_SDEBUG(DBGSERIAL.println(F("#error getting I2CGPS LON LAT"))) ;
    *lat=0;
    *lon=0;
    return;
  }

  *lat = Wire.read();
  *lat = *lat | ((int32_t)Wire.read())<<8 ;
  *lat = *lat | ((int32_t)Wire.read())<<16 ;
  *lat = *lat | ((int32_t)Wire.read())<<24 ;

  *lon = Wire.read();
  *lon = *lon | ((int32_t)Wire.read())<<8 ;
  *lon = *lon | ((int32_t)Wire.read())<<16 ;
  *lon = *lon | ((int32_t)Wire.read())<<24 ;

  Wire.endTransmission();
  
}
#endif

// this is the routine called by a active board
// do all periodic task 
// will be called every tr seconds
void Repeats() {

  wdt_reset();

  IF_SDEBUG(DBGSERIAL.println(F("#Repeats")));

#ifdef ETHERNETON

  // Allows for the renewal of DHCP leases
  switch ( Ethernet.maintain() ) {
#if defined(DEBUGONSERIAL)
  case 0 : 
    DBGSERIAL.println(F("#ethernet maintain: nothing happened"));
    break;
  case 1 : 
    DBGSERIAL.println(F("#ethernet maintain: renew failed"));
    break;
  case 2 : 
    DBGSERIAL.println(F("#ethernet maintain: renew success"));
    break;
  case 3 : 
    DBGSERIAL.println(F("#ethernet maintain: rebind fail"));
    break;
  case 4 : 
    DBGSERIAL.println(F("#ethernet maintain: rebind success"));
    break;
    //  default : 
#endif
  }
  
  wdt_reset();
#endif
    
  if (timeStatus() == timeNeedsSync) {
    IF_SDEBUG(DBGSERIAL.print(F("#The time had been set but a sync attempt did not succeed")));
    IF_SDEBUG(DBGSERIAL.println(timeStatus()));
    //return;  
    // this is more restrictive if uncommented 
    t = 0; // Store the current time in time variable t 
  }

  if(timeStatus()== timeNotSet) {
    IF_SDEBUG(DBGSERIAL.println(F("#The time has never been set")));
    //return;
    t = 0; // Store the current time in time variable t 

  }else{
    t = now(); // Store the current time in time variable t 
  }

  IF_SDEBUG(digitalClockDisplay(t));
  IF_LCD(lcd.setCursor(0,2)); 
  IF_LCD(LcdDigitalClockDisplay(t));

  wdt_reset();

  unsigned long waittime;
  unsigned long maxwaittime = 0;

  //   prepare sensors

  for (int i = 0; i < SENSORS_LEN; i++) {
    //IF_SDEBUG(DBGSERIAL.println(i));
    //IF_SDEBUG(DBGSERIAL.println((int)drivers[i].manager));
    //IF_SDEBUG(DBGSERIAL.println(configuration.sensors[i].node));

    if (drivers[i].manager == NULL) continue;

    //if (configuration.sensors[i].node > 0) continue;
    int ok = drivers[i].manager->prepare(waittime);
    IF_SDEBUG(DBGSERIAL.print(F("#prepare: "))); 
    IF_SDEBUG(DBGSERIAL.print(i));
    if (ok){
      IF_SDEBUG(DBGSERIAL.println(F(" ok.")));
      // max value
      if (maxwaittime < waittime) maxwaittime = waittime ;
    } else {
      IF_SDEBUG(DBGSERIAL.println(F(" failed.")));
    }

    wdt_reset();

  }

  // we have time to wait for sensor to take measuremts (for example 500 for tmp102; 250 for adt7420)
  IF_SDEBUG(DBGSERIAL.print(F("#wait for ms: ")));
  IF_SDEBUG(DBGSERIAL.println(maxwaittime));

#if defined (RADIORF24)
  unsigned long starttime=millis();
  while (millis()-starttime < maxwaittime)
    {
      //IF_SDEBUG(DBGSERIAL.println(F("#RF24Network update")));
      network.update();
      wdt_reset();
    }
#else
  delay(maxwaittime);
#endif
  
  for (int i = 0; i < SENSORS_LEN; i++) {
    if (drivers[i].manager == NULL) continue;

    IF_SDEBUG(DBGSERIAL.print(F("#getjson: ")));
    IF_SDEBUG(DBGSERIAL.println(i));

    aJsonObject *valuesobj = drivers[i].manager->getJson();
    if (!valuesobj) continue;
    aJsonObject *valueobj = valuesobj->child;
    if (!valueobj) continue;

    do {

      wdt_reset();
      sprintf ( mainbuf, "%04u-%02u-%02uT%02u:%02u:%02u",year(t),month(t),day(t),hour(t),minute(t),second(t));

      IF_SDEBUG(DBGSERIAL.println(F("#looping over ajson object: ")));

      aJsonObject *payloadobj = aJson.createObject();
      //char cval[10];
      //sprintf ( cval, "%i",valueobj->valueint);
      //aJson.addStringToObject(payloadobj, "v", cval);
      if (valueobj->type == aJson_NULL) {

	IF_SDEBUG(DBGSERIAL.println(F("#missing")));

	IF_LCD(lcd.setCursor(0,i)); 
	IF_LCD(lcd.print(F("missing value       ")));

	//skip
	aJson.deleteItem(payloadobj);
	valueobj=valueobj->next;
	continue;

	// or missing value
	//aJson.addNullToObject(payloadobj, "v");
	
      }else{
	aJson.addNumberToObject(payloadobj, "v", valueobj->valueint);

	IF_SDEBUG(DBGSERIAL.print(F("#")));
	IF_SDEBUG(DBGSERIAL.print(valueobj->name));
	IF_SDEBUG(DBGSERIAL.print(F(":")));
	IF_SDEBUG(DBGSERIAL.println(valueobj->valueint,DEC));

	IF_LCD(lcd.setCursor(0,i)); 
	IF_LCD(lcd.print(F("                    ")));
	IF_LCD(lcd.setCursor(0,i)); 
	IF_LCD(lcd.print(valueobj->name));
	IF_LCD(lcd.setCursor(10,i)); 
	IF_LCD(lcd.print(valueobj->valueint));

      }
      // if time was never setted I suppose I have no time and I do not pubblish time
      if ( t != 0 ){
	aJson.addStringToObject(payloadobj, "t", mainbuf);
	// uncomment if you want to be more restrictive and do not want server to add the timestamp
	//      }else{
	//return;
      }
      // here I use char
      //char payload[50];
      //aJson.print(payloadobj,payload, sizeof(payload));
      
      // here I use malloc (aJson.print) 40 will be enought
      char *payload=aJson.print(payloadobj,40);
      IF_SDEBUG(DBGSERIAL.print("#"));
      IF_SDEBUG(DBGSERIAL.println(payload));
      // send it to mqtt server appendig path to rootpath

      wdt_reset();

#ifdef I2CGPSPRESENT

      int32_t lat;
      int32_t lon;
      GPS_latlon_read(&lat,&lon);

      // gcc BUG !!!!!!!!!!!!!!!!!! (4.3, 4.8 and 4.9 versions)
      // sprintf(mainbuf,configuration.mqttrootpath, lon/100,lat/100);

      // char *format;
      // format=configuration.mqttrootpath;
      // sprintf(mainbuf,format, lon/100,lat/100);

      char format[MQTTROOTPATH_LEN];
      strcpy(format,configuration.mqttrootpath);
      sprintf(mainbuf,format, lon/100,lat/100);

#else
      strcpy (mainbuf,configuration.mqttrootpath);
#endif

      strcat (mainbuf,configuration.sensors[i].mqttpath);
      strcat (mainbuf,valueobj->name);

      IF_SDEBUG(DBGSERIAL.print(F("#topic:")));
      IF_SDEBUG(DBGSERIAL.println(mainbuf));
      IF_SDEBUG(DBGSERIAL.print(F("#payload:")));
      IF_SDEBUG(DBGSERIAL.println(payload));

#if defined(ETHERNETMQTT) || defined(GSMGPRSMQTT) || defined(GSMGPRSHTTP)
      bool sendstatus;

#ifdef SDCARD
      strcpy(record.topic, mainbuf);
      //strcat( record.separator, ";");
      strcpy( record.payload, payload);
#endif

#endif

#if defined(ETHERNETMQTT) || defined(GSMGPRSMQTT)

      wdt_reset();
      if (!mqttclient.publish(mainbuf, payload))
	{

	  sendstatus=false;

	  IF_SDEBUG(DBGSERIAL.println(F("#error mqtt publish")));
	  
#ifdef GSMGPRSMQTT
	  rmapdisconnect();
	  IF_SDEBUG(DBGSERIAL.println(F("#try to restart sim800 TCP")));
	  // try to restart sim800
	  wdt_reset();
	  // escape sequence
	  //s800.stop();        //require too time in loop
	  s800.TCPstop();
	  // fast restart
	  if (s800.init_onceautobaud()){
	    if (s800.setup()){
	      wdt_reset();
	      s800.TCPstart(GSMAPN,GSMUSER,GSMPASSWORD);
	      wdt_reset();

	      s800.getSignalQualityReport(&rssi,&ber);
	      IF_SDEBUG(DBGSERIAL.print(F("#s800 rssi:")));
	      IF_SDEBUG(DBGSERIAL.println(rssi));
	      IF_SDEBUG(DBGSERIAL.print(F("#s800 ber:")));
	      IF_SDEBUG(DBGSERIAL.println(ber));
	      wdt_reset();

	      rmapconnect();
	    }
          }
#endif
	}
      else
	{
	  sendstatus=true;
	}
      wdt_reset();
#endif


      #ifdef GSMGPRSHTTP

      // compose URL
      prepend(mainbuf, "/http2mqtt/?topic=");
      strcat (mainbuf,"&payload=");
      strcat (mainbuf,payload);
      strcat (mainbuf,"&user=");
      strcat (mainbuf,configuration.mqttuser);
      strcat (mainbuf,"&password=");
      strcat (mainbuf,configuration.mqttpassword);

      #ifdef GSMGPRSRTC
      strcat (mainbuf,"&time=t");
      #endif
      
      IF_SDEBUG(DBGSERIAL.print("#GSM send get:"));
      IF_SDEBUG(DBGSERIAL.println(mainbuf));
 

      //reattach gsm if needed
      //if (!gsm.IsRegistered()) gsmgprsstart();

      sendstatus=false;

      //TCP Client GET, send a GET request to the server and save the reply.
      if (s800.httpGET(configuration.mqttserver, 80,mainbuf, mainbuf, sizeof(mainbuf))){
	//Print the results.
	IF_SDEBUG(DBGSERIAL.println(F("#GSM Data received:")));
	IF_SDEBUG(DBGSERIAL.println(mainbuf));

        #ifdef GSMGPRSRTC
	if (s800.RTCset(scantime(mainbuf)) != 0){
	  IF_SDEBUG(DBGSERIAL.println(F("#GSM ERROR setting RTC time")));
	}else{
	  setSyncProvider(s800.RTCget);   // the function to get the time from the RTC
	}
        #endif
	if (strstr(mainbuf,"OK") != NULL){
	  sendstatus=true;
	}else{
	  IF_SDEBUG(DBGSERIAL.println(F("#GSM ERROR in httpget response")));
	  IF_LOGDATEFILE("GSM ERROR in httpget response\n");
	}

      }else{

	IF_SDEBUG(DBGSERIAL.println(F("#GSM ERROR in httpget")));
	IF_LOGDATEFILE("GSM ERROR in httpget\n");
	if (!s800.checkNetwork()){
	  IF_SDEBUG(DBGSERIAL.println("#GSM try to restart network"));
	  s800.startNetwork(GSMAPN, GSMUSER, GSMPASSWORD);
        }
	if (!s800.checkNetwork()){
	  IF_SDEBUG(DBGSERIAL.println("#GSM try to restart sim800"));

	  // fast restart
	  wdt_reset();
	  if (s800.init_onceautobaud()){
	    if (s800.setup()){
	      s800.stopNetwork();
	      s800.startNetwork(GSMAPN, GSMUSER, GSMPASSWORD);
	    }
	  }
        }

	IF_SDEBUG(DBGSERIAL.println(F("#Retry httpget")));
	if (s800.httpGET(configuration.mqttserver, 80,mainbuf, mainbuf, sizeof(mainbuf))){
	  //Print the results.
	  IF_SDEBUG(DBGSERIAL.println(F("#GSM Data received:")));
	  IF_SDEBUG(DBGSERIAL.println(mainbuf));
	  
          #ifdef GSMGPRSRTC
	  if (s800.RTCset(scantime(mainbuf)) != 0){
	    IF_SDEBUG(DBGSERIAL.println(F("#GSM ERROR setting RTC time")));
	  }
          #endif
	  if (strstr(mainbuf,"OK") != NULL){
	    sendstatus=true;
	  }else{
	    IF_SDEBUG(DBGSERIAL.println(F("#GSM ERROR in httpget response")));
	    IF_LOGDATEFILE("GSM ERROR in httpget response\n");
	  }

	}else{
	  IF_SDEBUG(DBGSERIAL.println(F("#GSM ERROR in httpget")));
	  IF_LOGDATEFILE("GSM ERROR in httpget\n");
	}

      }

      s800.getSignalQualityReport(&rssi,&ber);
      IF_SDEBUG(DBGSERIAL.print(F("#s800 rssi:")));
      IF_SDEBUG(DBGSERIAL.println(rssi));
      IF_SDEBUG(DBGSERIAL.print(F("#s800 ber:")));
      IF_SDEBUG(DBGSERIAL.println(ber));
      wdt_reset();
      
      #endif

#ifdef SDCARD

      // write data on SD if time is set only
      if ( t != 0 )
	{

	  record.done=sendstatus;
	  IF_SDEBUG(DBGSERIAL.print(F("#write:"))); 
	  IF_SDEBUG(DBGSERIAL.print(record.done)); 
	  IF_SDEBUG(DBGSERIAL.print(record.separator)); 
	  IF_SDEBUG(DBGSERIAL.print(record.topic)); 
	  IF_SDEBUG(DBGSERIAL.println(record.payload)); 
	  
	  if (dataFile.write(&record,sizeof(record)) == -1)
	    {
	      IF_SDEBUG(DBGSERIAL.println(F("#WRITE ERROR")));
	    }
	  dataFile.flush();
	  pos+= sizeof(record);
	  IF_SDEBUG(DBGSERIAL.println(F("#written record at end")));
	  
	  if (pos >= MAX_FILESIZE)
	    {
	      dataFile.close();
	      nextName(fileName);
	      
	      strcpy(fullfileName,fileName);
	      strcat (fullfileName,".que");
	      
	      IF_SDEBUG(DBGSERIAL.print(F("#open file: ")));
	      IF_SDEBUG(DBGSERIAL.println(fullfileName));
	      
	      // Open up the file we're going to log to!
	      dataFile = SD.open(fullfileName, FILE_WRITE);
	      if (! dataFile) {
		IF_SDEBUG(DBGSERIAL.print(F("#error opening: ")));
		IF_SDEBUG(DBGSERIAL.println(fullfileName));
		// Wait forever since we cant write data
		//while (1) ;
	      }
	      dataFile.seekSet(0);
	      pos=0;	  
	    }
	}

#endif

      // free object in same malloc order !!!
      free(payload);
      aJson.deleteItem(payloadobj);
      
      valueobj=valueobj->next;
    } while ( valueobj );
    
    // free object in same malloc order !!!
    aJson.deleteItem(valuesobj);

    wdt_reset();

  }
}
#endif

#ifdef JSONRPCON
// this is the main routine to manage json rpc messages
void mgrjsonrpc(aJsonObject *msg)
{
  int err = E_SUCCESS;
  aJsonObject* rpcid=NULL ;
  aJsonObject* jsonrpc=NULL;

  if (msg == NULL){
    IF_SDEBUG(DBGSERIAL.println(F("#error: json is wrong")));
    err=E_INTERNAL_ERROR;
  }else{

    IF_SDEBUG(aJson.print(msg,mainbuf, sizeof(mainbuf)));
    IF_SDEBUG(DBGSERIAL.print(F("#msg: ")));
    IF_SDEBUG(DBGSERIAL.println(mainbuf));
    
    // parse rpc information
    rpcid = aJson.getObjectItem(msg, "id");
    jsonrpc = aJson.getObjectItem(msg, "error");      //ignore response to rpc
    if (jsonrpc){
      err=E_INVALID_REQUEST;
    }else{
      jsonrpc = aJson.getObjectItem(msg, "response"); //ignore response to rpc
      if (jsonrpc){
	err=E_INVALID_REQUEST;
      }else
	{
	  jsonrpc = aJson.getObjectItem(msg, "jsonrpc");
        
	  //  call RPC
	  err=rpc.processMessage(msg);
	  IF_SDEBUG(DBGSERIAL.print(F("#rpc.processMessage return status:")));
	  IF_SDEBUG(DBGSERIAL.println(err));
    
	  if (!rpcid){
	    IF_SDEBUG(DBGSERIAL.println(F("#id not found")));
	    err=E_INTERNAL_ERROR;
	  }
    
	  if (!jsonrpc){
	    IF_SDEBUG(DBGSERIAL.println(F("#jsonrpc not found")));
	    err=E_INTERNAL_ERROR;
	  } else {
	    if (strcmp (jsonrpc->valuestring,"2.0" ) != 0) {
	      IF_SDEBUG(DBGSERIAL.print(F("#jsonrpc version is wrong:")));
	      IF_SDEBUG(DBGSERIAL.println(jsonrpc->valuestring));
	      err=E_INTERNAL_ERROR;
	    }
	  }
	}
    }
  }


  response=aJson.createObject();
  if (response == NULL) {
    IF_SDEBUG(DBGSERIAL.println(F("#error building response")));
    err = E_INVALID_REQUEST;     // cannot build response
  }


  if ( err == E_INVALID_REQUEST){

    IF_SDEBUG(DBGSERIAL.println(F("#do not fill a response to an error mesage")));
    aJson.deleteItem(response);
    mainbuf[0]='\0';
    return;

  } else if (err != E_SUCCESS ){

    // manage error message 
    IF_SDEBUG(DBGSERIAL.println(F("#return error")));
    if (!jsonrpc || !msg) {
      IF_SDEBUG(DBGSERIAL.println(F("#add default jsonrpc version in response")));
      aJson.addStringToObject(response, "jsonrpc","2.0");
    } else {
      IF_SDEBUG(DBGSERIAL.println(F("#add jsonrpc version in response")));
      aJson.addStringToObject(response, "jsonrpc",jsonrpc->valuestring);
    }

    result = aJson.createObject();
    aJson.addItemToObject(response, "error", result);
    aJson.addNumberToObject(result, "code", err);
    aJson.addStringToObject(result,"message", strerror(err));   

    if (!rpcid || !msg){
      IF_SDEBUG(DBGSERIAL.println(F("#add null id in response")));
      aJson.addNullToObject(response, "id");
    } else {
	IF_SDEBUG(DBGSERIAL.println(F("#add id in response")));
        aJson.addNumberToObject(response, "id", rpcid->valueint);
    }

  } else {
 
    // manage good message
    IF_SDEBUG(DBGSERIAL.println(F("#its a valid method")));
    //aJson.addItemToObject  (response, "jsonrpc",aJson.createItem("2.0"));
    aJson.addStringToObject(response, "jsonrpc",jsonrpc->valuestring);
    aJson.addItemToObject(response, "result", result );
    //aJson.addNumberToObject(response, "result", 1);
    aJson.addNumberToObject(response, "id", rpcid->valueint);
    //sprintf(c, "{\"jsonrpc\": \"2.0\",\"result\":%i, \"id\": 0}", requestedStatus);   
  }
    
  //IF_SDEBUG(DBGSERIAL.println("{\"jsonrpc\": \"2.0\",\"result\":1, \"id\": 0}"));
    
  aJson.print(response,mainbuf, sizeof(mainbuf));
  IF_SDEBUG(DBGSERIAL.print(F("#response: ")));
  IF_SDEBUG(DBGSERIAL.println(mainbuf));
  //free(json);
  aJson.deleteItem(response);
}

#if defined(GSMGPRSHTTP) || defined(GSMGPRSMQTT)

void StartModem() {

  IF_SDEBUG(DBGSERIAL.println(F("#GSM try to init sim800")));
  // static hardwareserial defined at compile time in sim800 library 
  //if (s800.init(&GSMSERIAL , GSMONOFFPIN, GSMRESETPIN)){
  wdt_reset();  
  if (s800.init(GSMONOFFPIN, GSMRESETPIN)){
    s800.setup();
    IF_SDEBUG(DBGSERIAL.println(F("#GSM sim800 initialized")));
  }else{
    IF_SDEBUG(DBGSERIAL.println(F("#GSM ERROR init sim800; retry")));
    
    // In Stima configuration sim800 will be always on becouse PWRKEY
    // Pin should be pulled down 
    //s800.switchOn();
    s800.resetModem();
    wdt_reset();
    s800.setup();
    wdt_reset();
    #ifdef GSMGPRSMQTT
    s800.stop();
    #endif
    wdt_reset();
    s800.TCPstop();
    wdt_reset();
    s800.stopNetwork();
  }
  wdt_reset();
}

void RestartModem() {

  IF_SDEBUG(DBGSERIAL.println("#RestartModem"));
  
  #if defined(GSMGPRSHTTP)
  if (!s800.checkNetwork()) {
    IF_SDEBUG(DBGSERIAL.println("#GSM try to restart network"));
  #endif

  #if defined(GSMGPRSMQTT)
  if (!mqttclient.connected()) {
    IF_SDEBUG(DBGSERIAL.println(F("#GSM try to start TCP")));
  #endif

    IF_LOGDATEFILE("hard GSM restart\n");
    StartModem();

    #if defined(GSMGPRSHTTP)
    s800.startNetwork(GSMAPN, GSMUSER, GSMPASSWORD);
    #endif

    #if defined(GSMGPRSMQTT)
    s800.TCPstart(GSMAPN,GSMUSER,GSMPASSWORD);
    #endif

  sprintf(mainbuf,"rssi:%d,ber:%d\n",rssi,ber);
  IF_LOGDATEFILE(mainbuf);

  }
}
#endif


#if defined(ETHERNETMQTT) || defined(GSMGPRSMQTT)


// mqtt Callback function
// the payload have to be somelike:
// {"method": "togglepin", "params": [{"n":4,"s":true},{"n":5,"s":false}]}
// it's a subset of jsonrpc specification

void mqttcallback(char* topic, byte* payload, unsigned int length) {
  // In order to republish this payload, a copy must be made
  // as the orignal payload buffer will be overwritten whilst
  // constructing the PUBLISH packet.
  
  // we use a global buffer so we don't:
  // Allocate the correct amount of memory for the payload copy
  // char* p = (char*)malloc(length+1);

  // Copy the payload to the new buffer
  memcpy(mainbuf,payload,length);
  memcpy(mainbuf+length,"\0",1);
  IF_SDEBUG(DBGSERIAL.print(F("#MQTT callback topic: ")));
  IF_SDEBUG(DBGSERIAL.println(topic));
  IF_SDEBUG(DBGSERIAL.print(F("#payload: ")));
  IF_SDEBUG(DBGSERIAL.println(mainbuf));

  aJsonObject *msg = aJson.parse(mainbuf);

  mgrjsonrpc(msg);

  #ifdef ETHERNETMQTT
  char topicres [SERVER_LEN+21];

  snprintf(topicres,sizeof(topicres),"%s%s/%02x%02x%02x%02x%02x%02x/res", MQTTRPCPREFIX,configuration.mqttuser,configuration.mac[0], configuration.mac[1], configuration.mac[2], configuration.mac[3], configuration.mac[4], configuration.mac[5]);
  #endif

  #ifdef GSMGPRSMQTT
  char topicres [SERVER_LEN+21];
  // IMEI code from sim800
  snprintf(topicres,sizeof(topicres), "%s%s/%s/res", MQTTRPCPREFIX,configuration.mqttuser,imeicode);
  #endif

  if (!mqttclient.publish(topicres,mainbuf)){
    IF_SDEBUG(DBGSERIAL.print(F("#mqtt ERROR publish rpc reponse")));
  }

  // Free the memory
  aJson.deleteItem(msg);
}
#endif

#ifdef SERIALJSONRPC
// receive RPC messages on serial transport
void mgrserialjsonrpc(void)
{

#ifdef FREERAM
  IF_SDEBUG(DBGSERIAL.print(F("#free ram on mgrserialjsonrpc: ")));
  IF_SDEBUG(DBGSERIAL.println(freeRam()));
#endif

  if (stream.available()) {
    // skip any accidental whitespace like newlines
    stream.skip();
  }

  if (stream.available()) {
    IF_SDEBUG(DBGSERIAL.println(F("#stream available")));
    aJsonObject *msg = aJson.parse(&stream);
    mgrjsonrpc(msg);
    aJson.deleteItem(msg);

    if (strlen(mainbuf) > 0)
#ifdef SERIALJSONRPC
      RPCSERIAL.println(mainbuf);
#endif

    if (stream.available()) {
      stream.flush();
    }
  }
}

#endif


#ifdef TCPSERVER
void mgrethserver(void)
// receive RPC messages on TCP/IP transport
{
  size_t size;

  if (EthernetClient client = ethServer.available())
    {
      if (client)
        {
          while((size = client.available()) > 0)
            {
              size = client.read((uint8_t *)mainbuf,size);
	      aJsonObject *msg = aJson.parse(mainbuf);
	      mgrjsonrpc(msg);
	      aJson.deleteItem(msg);
	      if (strlen(mainbuf) > 0)
		client.write((uint8_t *)mainbuf,strlen(mainbuf));
            }

	  //	  if (!client.connected())
	  //	    client.stop();
        }
    }
}
#endif


#ifdef RF24JSONRPC
// receive RPC messages on RF24Network transport
void mgrrf24jsonrpc(void)
{

#ifdef FREERAM
  IF_SDEBUG(DBGSERIAL.print(F("#free ram in mgrrf24jsonrpc: ")));
  IF_SDEBUG(DBGSERIAL.println(freeRam()));
#endif

  // Pump the network regularly
  //IF_SDEBUG(DBGSERIAL.println(F("#RF24Network update")));
  network.update();

  // Is there anything ready for us?
  while ( network.available() ){
    wdt_reset();
    IF_SDEBUG(DBGSERIAL.println(F("#receiving rf24 jsonrpc")));
    RF24NetworkHeader header;
    size_t size = network.read(header,mainbuf,sizeof(mainbuf));
#if defined (AES)
    aes_dec(configuration.key, configuration.iv, mainbuf,&size);
#endif
    IF_SDEBUG(DBGSERIAL.print(F("#size:")));
    IF_SDEBUG(DBGSERIAL.println(size));

    wdt_reset();
    if (size >0){
      mainbuf[size-1]='\0';

      IF_SDEBUG(DBGSERIAL.println(F("#rf24 available")));
      IF_SDEBUG(DBGSERIAL.print(F("#strlen:")));
      IF_SDEBUG(DBGSERIAL.println(strlen(mainbuf)));
      IF_SDEBUG(DBGSERIAL.println(mainbuf));

      aJsonObject *msg = aJson.parse(mainbuf);
      mgrjsonrpc(msg);
      aJson.deleteItem(msg);
      wdt_reset();

      if (strlen(mainbuf) > 0){
	IF_SDEBUG(DBGSERIAL.println(F("#sendiging rf24 jsonrpc respose")));
	RF24NetworkHeader sendheader(header.from_node,0);

	size_t buflen=strlen(mainbuf)+1;      
#if defined (AES)
	aes_enc(configuration.key, configuration.iv, mainbuf, &buflen);
#endif
	bool ok= network.write(sendheader,mainbuf,buflen);
	if (!ok){
	  IF_SDEBUG(DBGSERIAL.println(F("#error sendiging rf24 jsonrpc respose")));
	}
	wdt_reset();
      }
    }
    
    //IF_SDEBUG(DBGSERIAL.println(F("#RF24Network update")));
    network.update();
    wdt_reset();

  }
}

#endif
#endif

#if defined(ETHERNETMQTT) || defined(GSMGPRSMQTT)
// manage works for ethernet like reconnect to mqtt broker and subscribe to topic for RPC
void mgrmqtt()
{
  wdt_reset();

  if (mqttclient.loop()){
#ifdef FREEMEM
    freeMem("#free mem in mgrmqtt");
#endif
#ifdef FREERAM
    IF_SDEBUG(DBGSERIAL.print(F("#free ram on mgrmqtt: ")));
    IF_SDEBUG(DBGSERIAL.println(freeRam()));
#endif

    wdt_reset();

#ifdef SDCARDLOGFILE
    if (!msgconnected){
      IF_LOGDATEFILE("mqtt connected\n");
      msgconnected=true;
    }
    wdt_reset();
#endif

  }else{
      
    IF_SDEBUG(digitalClockDisplay(now()));
    IF_SDEBUG(DBGSERIAL.println(F("#mqtt disconnected")));
#ifdef SDCARDLOGFILE
    if (msgconnected){
      IF_LOGDATEFILE("mqtt disconnected\n");
      msgconnected=false;
    }
    wdt_reset();
#endif


    /*
#ifdef GSMGPRSMQTT
    IF_SDEBUG(DBGSERIAL.println(F("#try to restart sim800 TCP")));
    // try to restart sim800
    wdt_reset();
    // escape sequence
    //s800.stop();        //require too time in loop
    s800.TCPstop();
    // fast restart
    if (s800.init_onceautobaud()){
      if (s800.setup()){
	wdt_reset();
	s800.TCPstart(GSMAPN,GSMUSER,GSMPASSWORD);
	wdt_reset();
      }
    }
#endif
    */

    rmapconnect();

  }
}
#endif


#if defined(SDCARD)

			   // recovery data from SD card
			   // set write pointer to the end of last file
			   // all parameter not used for now
void mgrsdcard()
{
  wdt_reset();

  strcpy(fileName,FILE_BASE_NAME);
  strcat (fileName,"000");

  // find exixting file name
  while (exists(fileName))
    {
	
      wdt_reset();

#if defined(ETHERNETMQTT) || defined(GSMGPRSMQTT)
      // poll mqtt connection if required
      mgrmqtt();
#endif

      IF_SDEBUG(DBGSERIAL.print(F("#file exist: ")));
      IF_SDEBUG(DBGSERIAL.println(fileName));
      
      //strcpy(fullfileName,fileName);
      //strcat (fullfileName,".que");
      
      // check if .que filename exists
      if (SD.exists(fullfileName))
	{
	  if (dataFile.isOpen()) dataFile.close();

	  IF_SDEBUG(DBGSERIAL.print(F("#found que file; open: ")));
	  IF_SDEBUG(DBGSERIAL.println(fullfileName));
	  
	  dataFile = SD.open(fullfileName, FILE_WRITE);
	  if (! dataFile) {
	    IF_SDEBUG(DBGSERIAL.print(F("error opening: ")));
	    IF_SDEBUG(DBGSERIAL.println(fullfileName));
	    // Wait forever since we cant write data
	    //while (1) ;
	  }
	  
	  dataFile.seekSet(0);
	  uint32_t size=dataFile.fileSize();
	  bool success = true;

	  pos=0;
	  while (pos < size)
	    {
		
	      wdt_reset();
		
	      if (dataFile.read(&record,sizeof(record)) != sizeof(record) )
		{
		  IF_SDEBUG(DBGSERIAL.println(F("#READ ERROR")));
		  break;
		}
	      else
		{
		  //IF_SDEBUG(DBGSERIAL.print(F("#read:"))); 
		  //IF_SDEBUG(DBGSERIAL.print(record.done)); 
		  //IF_SDEBUG(DBGSERIAL.print(record.separator)); 
		  //IF_SDEBUG(DBGSERIAL.print(record.topic));
		  //IF_SDEBUG(DBGSERIAL.println(record.payload)); 

		  if (record.done == false)
		    {
		      
#if defined(ETHERNETMQTT) || defined(GSMGPRSMQTT)

		      wdt_reset();
		      mgrmqtt();
		      wdt_reset();
		      IF_SDEBUG(DBGSERIAL.println(F("#recover mqtt publish"))); 
		      if (!mqttclient.publish(record.topic, record.payload))
			{
			  IF_SDEBUG(DBGSERIAL.println(F("#error mqtt publish")));
			}
		      else
			{
			  record.done=true;
			}

		      mgrmqtt();

#endif
#ifdef GSMGPRSHTTP
		      IF_SDEBUG(DBGSERIAL.println(F("#recover http publish"))); 
		      // compose URL
		      strcpy (mainbuf, "/http2mqtt/?topic=");
		      strcat (mainbuf,record.topic);
		      strcat (mainbuf,"&payload=");
		      strcat (mainbuf,record.payload);
		      strcat (mainbuf,"&user=");
		      strcat (mainbuf,configuration.mqttuser);
		      strcat (mainbuf,"&password=");
		      strcat (mainbuf,configuration.mqttpassword);

		      IF_SDEBUG(DBGSERIAL.print(F("#GSM send get:")));
		      IF_SDEBUG(DBGSERIAL.println(mainbuf));

		      //reattach gsm if needed
		      //if (!gsm.IsRegistered()) gsmgprsstart();

		      wdt_reset();
		      //TCP Client GET, send a GET request to the server and save the reply.
		      if (s800.httpGET(configuration.mqttserver, 80,mainbuf, mainbuf, sizeof(mainbuf))){
			//Print the results.
			IF_SDEBUG(DBGSERIAL.println(F("#GSM Data received:")));
			IF_SDEBUG(DBGSERIAL.print("#"));
			IF_SDEBUG(DBGSERIAL.println(mainbuf));

			if (strstr(mainbuf,"OK") != NULL){
			  record.done=true;
			}else{
			  record.done=false;
			  IF_SDEBUG(DBGSERIAL.println(F("#GSM ERROR in httpget response")));
			  IF_LOGDATEFILE("GSM ERROR recovery from SD in httpget response\n");
			}

		      }else{
			IF_SDEBUG(DBGSERIAL.println(F("#error http publish")));
		      }

#endif
		      
		      wdt_reset();
		      if (record.done==true)
			{
			  dataFile.seekSet(pos);

			  IF_SDEBUG(DBGSERIAL.print(F("#write:"))); 
			  IF_SDEBUG(DBGSERIAL.print(record.done)); 
			  IF_SDEBUG(DBGSERIAL.print(record.separator)); 
			  IF_SDEBUG(DBGSERIAL.print(record.topic)); 
			  IF_SDEBUG(DBGSERIAL.println(record.payload)); 

			  if (dataFile.write(&record,sizeof(record)) == -1)
			    {
			      IF_SDEBUG(DBGSERIAL.println(F("#WRITE ERROR")));
			      success=false;
			    }
			  IF_SDEBUG(DBGSERIAL.println(F("#done"))); 
			  wdt_reset();
			}
		      else
			{
			  success=false;
			}
		    }
		}
	      pos+= sizeof(record);
	    }
	  
	  dataFile.close();

	  wdt_reset();
	  
	  // check file size
	  dataFile = SD.open(fullfileName, O_READ);
	  size = dataFile.fileSize();
	  dataFile.close();
	  wdt_reset();
	  
	  IF_SDEBUG(DBGSERIAL.print(F("#filesize: ")));
	  IF_SDEBUG(DBGSERIAL.println(size));
	  
	  if (size >= MAX_FILESIZE)
	    {
	      // if dequeued move to archive
	      if (success)
		{		  
		  strcpy(newfileName,fileName);
		  strcat (newfileName,".don");
		  dataFile = SD.open(fullfileName, O_WRITE | O_CREAT);
		  IF_SDEBUG(DBGSERIAL.print(F("#RENAME: ")));
		  IF_SDEBUG(DBGSERIAL.print(fullfileName));
		  IF_SDEBUG(DBGSERIAL.println(newfileName));
		  dataFile.rename(SD.vwd(),newfileName);
		  dataFile.close();
		  }
	    }
	  else
	    {
	      // Found an not full file name.
	      // go to append new data
	      IF_SDEBUG(DBGSERIAL.println(F("#SD append data")));
	      break;
	      }
	}
      // check new file
      nextName(fileName);
    }
  // Found an unused file name.
  
  wdt_reset();
#if defined(ETHERNETMQTT) || defined(GSMGPRSMQTT)
  // poll mqtt connection if required
  mgrmqtt();
#endif

  wdt_reset();
  IF_SDEBUG(DBGSERIAL.print(F("#open file: ")));
  IF_SDEBUG(DBGSERIAL.println(fullfileName));
  
  dataFile = SD.open(fullfileName, FILE_WRITE);
  if (! dataFile) {
    IF_SDEBUG(DBGSERIAL.print(F("#error opening: ")));
    IF_SDEBUG(DBGSERIAL.println(fullfileName));
    // Wait forever since we cant write data
    //while (1) ;
  }

  wdt_reset();
  dataFile.seekEnd(0);
  pos = dataFile.curPosition();
  // check if position is phased
  int phase=pos % sizeof(record);
  if (phase != 0 ){

    IF_LOGDATEFILE("datafile trunkated\n");
    IF_SDEBUG(DBGSERIAL.print(F("#ERROR datafile trunkated: ")));
    IF_SDEBUG(DBGSERIAL.print(phase));

    pos-=phase;
    dataFile.seekSet(pos);
  }
}

int sdrecoveryrpc(aJsonObject* params)
{
  //boolean all=false;
  //aJsonObject* allParam = aJson.getObjectItem(params, "all");
  //if (allParam)  all = allParam -> valuebool;
  //IF_SDEBUG(DBGSERIAL.print(F("#sdrecoveryrpc : all: ")));
  //IF_SDEBUG(DBGSERIAL.print(all));
  //mgrsdcard(all);

  IF_LOGDATEFILE("jrpc sdrecovery\n");

  mgrsdcard();

  result = aJson.createObject();

  // aJson.addNumberToObject(result, "value", requestedStatus);
  // aJson.addStringToObject(result, "description", "Led status");

  return E_SUCCESS;  
}

#endif

void setup() 
{

/*
  Nel caso di un chip in standalone senza bootloader, la prima
  istruzione che è bene mettere nel setup() è sempre la disattivazione
  del Watchdog stesso: il Watchdog, infatti, resta attivo dopo il
  reset e, se non disabilitato, esso può provare il reset perpetuo del
  microcontrollore
*/
  wdt_disable();


#ifndef RF24SLEEP
  // enable watchdog with timeout to 8s
  wdt_enable(WDTO_8S);
#endif

  //TODO: here DBGSERIAL and RPCSERIAL shoud be the same
  //check to do not "rebegin"

#if defined (DEBUGONSERIAL)
  // Open serial communications and wait for port to open:
  DBGSERIAL.begin(DBGSERIALBAUDRATE);
  while (!DBGSERIAL) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

#ifdef SERIAL_DEBUG
// fill in the UART file descriptor with pointer to writer.
   fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);

   // The uart is the standard output device STDOUT.
   stdout = &uartout ;
#endif

#endif

#ifdef SERIALJSONRPC
  RPCSERIAL.begin(RPCSERIALBAUDRATE);
  while (!RPCSERIAL) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
#endif

  //#endif

  // inside witch ifdef ?
  // start up the i2c interface
  Wire.begin();

  //The Wire library enables the internal pullup resistors for SDA and SCL.
  //You can turn them off after Wire.begin()
  digitalWrite( SDA, LOW);
  digitalWrite( SCL, LOW);

  //set the i2c clock 
  TWBR = ((F_CPU / I2C_CLOCK) - 16) / 2;
  //Wire.setclock(31000L)

#if defined(LCD)
  /* Initialise the LCD */
  lcd.init();
  /* Make sure the backlight is turned on */
  lcd.backlight();
  delay(1000);
  /* Output the test message to the LCD */
  lcd.setCursor(0,0); 
  lcd.print(F("R-map project"));
#endif

  wdt_reset();

  // print a summary of compile time configuration
#if defined(DEBUGONSERIAL)
  DBGSERIAL.print(F("#Started; version: "));
  DBGSERIAL.print(F(STR(FIRMVERSION)));
#if defined (JSONRPCON)
  DBGSERIAL.print(F(" jsonrpc"));
#endif
#if defined (ATTUATORE)
  DBGSERIAL.print(F(" attuatore"));
#endif
#if defined (SENSORON)
  DBGSERIAL.print(F(" sensor"));
#endif
#if defined (ETHERNETON)
  DBGSERIAL.print(F(" ethernet"));
#endif
#if defined (NTPON)
  DBGSERIAL.print(F(" ntp"));
#endif
#if defined (RTCPRESENT)
  DBGSERIAL.print(F(" rtc"));
#endif
#if defined (I2CGPSPRESENT)
  DBGSERIAL.print(F(" I2CGPS"));
#endif
#if defined (RADIORF24)
  DBGSERIAL.print(F(" radiorf24"));
#endif
#if defined (AES)
  DBGSERIAL.print(F(" aes"));
#endif
#if defined (GSMGPRSRTC)
  DBGSERIAL.print(F(" gsm-rtc"));
#endif
#if defined (GSMGPRSHTTP)
  DBGSERIAL.print(F(" gsm-http"));
#endif
#if defined (GSMGPRSMQTT)
  DBGSERIAL.print(F(" gsm-mqtt"));
#endif
#if defined (LCD)
  DBGSERIAL.print(F(" lcd"));
#endif
#if defined (SDCARD)
  DBGSERIAL.print(F(" sdcard"));
#endif
#if defined (FREEMEM)
  DBGSERIAL.print(F(" freemem"));
#endif
#if defined (FREERAM)
  DBGSERIAL.print(F(" freeram"));
#endif
  DBGSERIAL.println("");

#ifdef FREERAM
  DBGSERIAL.print(F("#free ram on start: "));
  DBGSERIAL.println(freeRam());
#endif
#endif

  wdt_reset();

#if defined (JSONRPCON)
                                                      // register function for jsonrpc 
#if defined (ATTUATORE)

  // initialize the digital pin as an output
  IF_SDEBUG(DBGSERIAL.print(F("#set pins for ATTUATORE: ")));
  for (uint8_t i=0; i< sizeof(pins)/sizeof(*pins) ; i++){

    /*
    uint8_t bit = digitalPinToBitMask(pins[i]);
    uint8_t port = digitalPinToPort(pins[i]);
    volatile uint8_t *reg = portModeRegister(port);

    //https://github.com/r-map/rmap/issues/47
    //How to read pinMode for digital pin
    //http://arduino.stackexchange.com/questions/13165/how-to-read-pinmode-for-digital-pin
    if (*reg & bit) {
      // It's an output
    } else {
      // It's an input
      pinMode(pins[i], OUTPUT);
    }
    */

    pinMode(pins[i], OUTPUT);
    IF_SDEBUG(DBGSERIAL.print(pins[i]));
    IF_SDEBUG(DBGSERIAL.print(F(" ")));
  }
  IF_SDEBUG(DBGSERIAL.println(F("")));

  // and register the local togglepin method
  rpc.registerMethod("togglepin", &togglePin);
#endif

#if defined (RADIORF24)
  // and register the local rf24rpc method
  rpc.registerMethod("rf24rpc", &rf24rpc);
#endif

#if defined (SENSORON)

  rpc.registerMethod("prepare", &prepare);
  rpc.registerMethod("getjson", &getjson);
  rpc.registerMethod("prepandget", &prepandget);
  rpc.registerMethod("configure", &mgrConfiguration);
#endif
#if defined (SDCARD)
  // and register the local sdcardrpc method
  rpc.registerMethod("sdrecovery", &sdrecoveryrpc);
#endif

#if defined (REBOOTRPC)
  // and register the local reset method
  rpc.registerMethod("reboot", &rebootrpc);
#endif

#endif

  // check FORCECONFIGPIN to force configuration by serial port
  wdt_reset();

#ifdef SERIALJSONRPC
  pinMode(FORCECONFIGPIN, INPUT_PULLUP);
  pinMode(FORCECONFIGLED, OUTPUT); 

  if (digitalRead(FORCECONFIGPIN) == LOW) {
    digitalWrite(FORCECONFIGLED, HIGH);
    configured=false;
    IF_SDEBUG(DBGSERIAL.println(F("#force configuration by serial")));
    IF_LCD(lcd.setCursor(0,1)); 
    IF_LCD(lcd.print(F("Wait configuration")));

    while(!configured){                    // wait for configuration
      mgrserialjsonrpc();
    wdt_reset();
    }
  }
  else {
    digitalWrite(FORCECONFIGLED, LOW);
  }

#endif

                                                  // load configuration aand create drivers
  if (configuration.load()){

#ifdef FREERAM
    IF_SDEBUG(DBGSERIAL.print(F("#free ram on configuration load: ")));
    IF_SDEBUG(DBGSERIAL.println(freeRam()));
#endif

#if defined(DEBUGONSERIAL)     
    DBGSERIAL.println(F("#Configuration loaded"));
    DBGSERIAL.print(F("#sampletime:"));
    DBGSERIAL.println(configuration.rt);
    DBGSERIAL.print(F("#mqttrootpath:"));
    DBGSERIAL.println(configuration.mqttrootpath);
    DBGSERIAL.print(F("#mqttserver:"));
    DBGSERIAL.println(configuration.mqttserver);

    DBGSERIAL.print(F("#mqttuser:"));
    DBGSERIAL.println(configuration.mqttuser);
    DBGSERIAL.print(F("#mqttpassword:"));
    DBGSERIAL.println(configuration.mqttpassword);

#if defined (AES)
    DBGSERIAL.print(F("#key:"));
    for (int i=0;i<16;i++){
      DBGSERIAL.print(configuration.key[i]);      
      DBGSERIAL.print(F(","));
    }
    DBGSERIAL.println(F(" "));

    DBGSERIAL.print(F("#iv:"));
    for (int i=0;i<16;i++){
      DBGSERIAL.print(configuration.iv[i]);      
      DBGSERIAL.print(F(","));
    }
    DBGSERIAL.println(F(" "));
#endif
    DBGSERIAL.print(F("#ntpserver: "));
    DBGSERIAL.println(configuration.ntpserver);
    DBGSERIAL.print(F("#thisnode: "));
    DBGSERIAL.println(configuration.thisnode);
    DBGSERIAL.print(F("#channel: "));
    DBGSERIAL.println(configuration.channel);

#endif

#if defined(ETHERNETON) || defined(RADIORF24) || defined(SDCARD)

    //disable all chips
#if defined(ENC28J60)
    //pinMode(ENC28J60_CONTROL_CS, OUTPUT);
    pinMode(8, OUTPUT);
    //digitalWrite(ENC28J60_CONTROL_CS, HIGH);
    digitalWrite(8, HIGH);
#endif
#if defined(RADIORF24)
    pinMode(RF24CSPIN, OUTPUT);
    digitalWrite(RF24CSPIN, HIGH);
#endif
#if defined(SDCARD)
    pinMode(SDCHIPSELECT, OUTPUT);
    digitalWrite(SDCHIPSELECT, HIGH);
#endif

    SPI.begin();

#endif

    // start rf24 radio 
#ifdef RADIORF24
    radio.begin();
    network.begin(configuration.channel, configuration.thisnode);
    radio.setRetries(1,15);
    network.txTimeout=500;

    // this is for a board that sleep all the time and wait an interrupt on the rf24 when receive signals
#ifdef RF24SLEEP
    radio.maskIRQ(1,1,0);
    //sleep.pwrDownMode(); //sets the Arduino into power Down Mode sleep, the most power saving, all systems are powered down except the watch dog timer and external reset
    pinMode (INTERUPIN, INPUT);
#endif
    radio.powerUp();

#endif

    wdt_reset();

    // now do all setup for all configurated sensors

    for (int id = 0; id < SENSORS_LEN; id++) {
      if (configuration.sensors[id].type[0] != '\0') {

	IF_SDEBUG(DBGSERIAL.print(F("# >> CONFIGURE NEW SENSOR: ")));
	IF_SDEBUG(DBGSERIAL.println(id));
	  
#if defined(DEBUGONSERIAL)     
	DBGSERIAL.print(F("#driver:"));
	DBGSERIAL.println(configuration.sensors[id].driver);
	DBGSERIAL.print(F("#node:"));
	DBGSERIAL.println(configuration.sensors[id].node);
	DBGSERIAL.print(F("#type:"));
	DBGSERIAL.println(configuration.sensors[id].type);
	DBGSERIAL.print(F("#address:"));
	DBGSERIAL.println(configuration.sensors[id].address);
	DBGSERIAL.print(F("#mqttpath:"));
	DBGSERIAL.println(configuration.sensors[id].mqttpath);
#endif
	if (!drivers[id].setup(configuration.sensors[id].driver,
			       configuration.sensors[id].node,
			       configuration.sensors[id].type,
			       configuration.sensors[id].address
			       
        #if defined (AES)
			       , configuration.key, configuration.iv
        #endif
			       ) == SD_SUCCESS){
	  IF_SDEBUG(DBGSERIAL.println(F("error in setup Sensors")));
	}
      }
    }
    configured=true;
  } else {     
    IF_SDEBUG(DBGSERIAL.println(F("#Configuration not loaded")));
    configured=false;
  }

  wdt_reset();

#ifdef ETHERNETON

  IF_SDEBUG(DBGSERIAL.println(F("#Try to configure Ethernet using DHCP")));
                                                                    // start Ethernet
  while (Ethernet.begin(configuration.mac,ENCCEPIN) == 0) {
    IF_SDEBUG(DBGSERIAL.println(F("#Failed to configure Ethernet using DHCP")));

    wdt_reset();

    IF_LCD(lcd.setCursor(0,1)); 
    IF_LCD(lcd.print(F("DHCP failed")));

#ifdef SERIALJSONRPC
    mgrserialjsonrpc();
#endif

#ifdef RF24JSONRPC
    mgrrf24jsonrpc();
#endif

    wdt_reset();

  }

  wdt_reset();

#if defined(DEBUGONSERIAL)

  DBGSERIAL.print(F("#My IP address is "));
  DBGSERIAL.println(ip_to_str(Ethernet.localIP()));
  
  DBGSERIAL.print(F("#Netmask is "));
  DBGSERIAL.println(ip_to_str(Ethernet.subnetMask()));
  
  DBGSERIAL.print(F("#Gateway IP address is "));
  DBGSERIAL.println(ip_to_str(Ethernet.gatewayIP()));
  
  DBGSERIAL.print(F("#DNS IP address is "));
  DBGSERIAL.println(ip_to_str(Ethernet.dnsServerIP()));

#endif

  IF_LCD(lcd.setCursor(0,1)); 
  IF_LCD(lcd.print(ip_to_str(Ethernet.localIP())));

#endif

#ifdef FREERAM
  IF_SDEBUG(DBGSERIAL.print(F("#free ram in setup: ")));
  IF_SDEBUG(DBGSERIAL.println(freeRam()));
#endif

  wdt_reset();

#ifdef TCPSERVER
  // activate ethernet server
  wdt_reset();
  ethServer.begin();
#endif

  while(!configured){                    // if not configured wait for configuration for ever
#ifdef SERIALJSONRPC
    mgrserialjsonrpc();
#endif

#ifdef RF24JSONRPC
    //    mgrrf24jsonrpc();
#endif

#ifdef TCPSERVER
    mgrethserver();
#endif

    wdt_reset();

  }

#if defined(GSMGPRSHTTP) || defined(GSMGPRSMQTT)

  StartModem();

#ifdef GSMGPRSMQTT
  if (!s800.getIMEI(imeicode)){
    IF_SDEBUG(DBGSERIAL.println(F("#GSM ERROR getting IMEI; reboot")));
    // I cannot continue without IMEI
    Reboot();
  }
  wdt_reset();
#endif
#endif

#ifdef GSMGPRSRTCBOOT

  IF_SDEBUG(DBGSERIAL.println(F("#GSM set system time from RTC at boot")));

  setSyncInterval(3600*24); // set the number of seconds between re-sync
  // sync to RTC only at boot as required by you
  setSyncProvider(periodicResyncGSMRTC);   // reset the function to get the time from the RTC

#endif

#ifdef GSMGPRSHTTP
  wdt_reset();

  //GPRS attach, put in order APN, username and password.
  //If no needed auth let them blank.
  //s800.stopNetwork();
  //wdt_reset();
  // if already connected reuse it
  IF_SDEBUG(DBGSERIAL.println(F("#GSM start network")));
  s800.startNetwork(GSMAPN, GSMUSER, GSMPASSWORD);
  for (int i = 0; (i < 10 & !s800.checkNetwork()); i++) {
    s800.stopNetwork();
    wdt_reset();
    delay(3000);
    wdt_reset();
    s800.startNetwork(GSMAPN, GSMUSER, GSMPASSWORD);
    wdt_reset();
  }

  s800.getSignalQualityReport(&rssi,&ber);
  IF_SDEBUG(DBGSERIAL.print(F("#s800 rssi:")));
  IF_SDEBUG(DBGSERIAL.println(rssi));
  IF_SDEBUG(DBGSERIAL.print(F("#s800 ber:")));
  IF_SDEBUG(DBGSERIAL.println(ber));
  wdt_reset();

#endif

#ifdef GSMGPRSMQTT
  wdt_reset();
  for (int i = 0; ((i < 10) & !s800.TCPGetMyIP(mainbuf)); i++) {
    IF_SDEBUG(DBGSERIAL.println(F("#GSM try to start TCP")));
    s800.TCPstart(GSMAPN,GSMUSER,GSMPASSWORD);
    wdt_reset();
  }

#endif

  // here we sync and get the current time

#ifdef NTPON
  wdt_reset();
  IF_SDEBUG(DBGSERIAL.println(F("#waiting for sync")));
  Udp.begin(123);
  setSyncProvider(getNtpTime);
#endif


#if defined (RTCPRESENT)
  if(timeStatus()== timeSet){
    // if time is set and is synced set the RTC
    // set the rtc clock if we have one
    if (RTC.set(t) != 0) IF_SDEBUG(DBGSERIAL.println(F("#error setting RTC time")));
    }

  if (timeStatus() != timeSet){
    IF_SDEBUG(DBGSERIAL.println(F("#RTC try set the system time")));
    setSyncProvider(RTC.get);   // the function to get the time from the RTC
  }
#endif


#ifdef I2CGPSPRESENT
  if (timeStatus() != timeSet){
    IF_SDEBUG(DBGSERIAL.println(F("#GPS try set the system time")));
    setSyncProvider(GpsRtc.get);   // the function to get the time from the RTC
  }
#endif

#ifdef GSMGPRSRTC
  if (timeStatus() != timeSet){
    IF_SDEBUG(DBGSERIAL.println(F("#GPRS try set the system time")));
    setSyncInterval(3600*6); // set the number of seconds between re-sync
    setSyncProvider(s800.RTCget);   // the function to get the time from the RTC
  }
#endif

  t=now(); // get system time

#if defined(DEBUGONSERIAL)
  if(timeStatus()== timeNotSet){
    DBGSERIAL.println(F("#time was not set by the sync provider"));
  }else{
    digitalClockDisplay(t);
  }
#endif

#if defined(LCD)
  if(timeStatus()== timeNotSet){
    lcd.setCursor(0,2); 
    lcd.print(F("time was not set"));
  }else{
    lcd.setCursor(0,2); 
    LcdDigitalClockDisplay(t);
  }
#endif

  wdt_reset();

// connect to mqtt server
 #if defined(GSMGPRSMQTT)
  wdt_reset();
  s800.TCPstart(GSMAPN,GSMUSER,GSMPASSWORD);
  for (int i = 0; ((i < 10) & !rmapconnect()); i++) {
    IF_SDEBUG(DBGSERIAL.println("#MQTT connect failed"));
    s800.TCPstop();
    wdt_reset();
    delay(3000);
    wdt_reset();
    s800.TCPstart(GSMAPN,GSMUSER,GSMPASSWORD);
    wdt_reset();
  }
#endif

#if defined(ETHERNETMQTT)
  rmapconnect();
#endif

#if defined(SDCARD)

  IF_SDEBUG(DBGSERIAL.println(F("#Initializing SD card...")));
  
  // see if the card is present and can be initialized:
  if (!SD.begin(SDCHIPSELECT)) {
    IF_SDEBUG(DBGSERIAL.println(F("#Card failed, or not present")));
    // don't do anything more:
    //while (1) ;
  }else{
    IF_SDEBUG(DBGSERIAL.println(F("#card initialized.")));
  }

#if defined(SDCARDLOGFILE)
  // Open up the file we're going to log to!
  IF_SDEBUG(DBGSERIAL.println(F("#opening log data file on SD")));
  logFile = SD.open("rmap_log.dat", FILE_WRITE);
  if (! logFile) {
    IF_SDEBUG(DBGSERIAL.println(F("#error opening log data file")));
    // Wait forever since we cant write data
    //while (1) ;
  }

  logFile.seekEnd(0);

#endif

  IF_LOGDATEFILE("Start\n");

  mgrsdcard();

#endif

  wdt_reset();
#if defined(ETHERNETMQTT) || defined(GSMGPRSMQTT)
  // poll mqtt connection if required
  mgrmqtt();
#endif
  
#if defined(REPEATTASK)
  Alarm.timerRepeat(configuration.rt, Repeats);             // timer for every tr seconds

  // system clock and other can have overflow problem
  // so we reset everythings one time a week
  Alarm.alarmRepeat(dowMonday,8,0,0,Reboot);                // 8:00:00 every Monday

  #if defined(ETHERNETMQTT) || defined(GSMGPRSMQTT)
  Alarm.timerRepeat(60*60*3, RestartModem);                     // timer for restart GSM modem
  #endif

#endif

  IF_SDEBUG(DBGSERIAL.println(F("#setup terminated")));

}

// the main loop
// it's very simple:
// receive and manage messages over all transport and wait for interrupt by radio if required
 
void loop()
{  
  wdt_reset();

#ifdef REPEATTASK

  //IF_SDEBUG(DBGSERIAL.println(F("#loop")));
  //IF_SDEBUG(Alarm.delay(100));
  // call repeat when required
  Alarm.delay(0);
  wdt_reset();
#endif


#ifdef FREEMEM
  freeMem("#free mem in loop");
#endif
#ifdef FREERAM
  IF_SDEBUG(DBGSERIAL.print(F("#free ram on loop: ")));
  IF_SDEBUG(DBGSERIAL.println(freeRam()));
#endif

#ifdef SERIALJSONRPC
  mgrserialjsonrpc();
  wdt_reset();
#endif

#ifdef RF24JSONRPC
  // go to sleep waiting for interupt
#ifdef RF24SLEEP
  IF_SDEBUG(DBGSERIAL.println(F("#sleep")));
  IF_SDEBUG((delay(50)));
  network.sleep(INTERU,LOW,SLEEP_MODE_PWR_DOWN);
#endif
  mgrrf24jsonrpc();
  wdt_reset();
#endif

#if defined(ETHERNETMQTT) || defined(GSMGPRSMQTT)
  if (configured) mgrmqtt();
  wdt_reset();
#endif

#ifdef TCPSERVER
  mgrethserver();
  wdt_reset();
#endif

}
