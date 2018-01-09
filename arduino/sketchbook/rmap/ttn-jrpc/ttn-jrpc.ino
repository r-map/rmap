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

// Derived by a work of:
/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *******************************************************************************/

/*******************************************************************************
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * This uses OTAA (Over-the-air activation), where where a DevEUI and
 * application key is configured, which are used in an over-the-air
 * activation procedure where a DevAddr and session keys are
 * assigned/generated for use with all further communication.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy.
 * To use this sketch, first register your application and device with
 * the things network, to set or generate an AppEUI, DevEUI and AppKey.
 * Multiple devices can use the same AppEUI, but each device has its own
 * DevEUI and AppKey.
 *
 *******************************************************************************/

#define SAMPLETIME 30

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <avr/wdt.h>
#include "config.h"
#include <EEPROM.h>
#include "EEPROMAnything.h"
#include <ArduinoLog.h>
// include the JsonRPC library
#include <arduinoJsonRPC.h>
#include <Time.h>
#include <TimeAlarms.h>

#include <Wire.h>
#include <SensorDriver.h>
#define SENSORS_LEN 2
//#include <BitBool.h>
#include <bfix.h>

struct sensor_t
{
  char driver[5];         // driver name
  char type[5];         // driver name
  int address;            // i2c address
} sensors[SENSORS_LEN];


/*
struct bittemplate_t
{
  BitBool<32> bits;
};
*/

 /*
struct dtemplate1_t
{
  uint8_t templaten;
  uint8_t data[3];
};

struct dtemplate2_t
{
  uint8_t templaten;
  uint8_t data[5];
};
  
union {
  //  bittemplate_t bittemplate;
  dtemplate1_t dtemplate1;
  dtemplate2_t dtemplate2;  
} dtemplate;
 */

SensorDriver* sd[SENSORS_LEN];


// initialize an instance of the JsonRPC library for registering 
JsonRPC rpcserver(false ); //standard protocol

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LMIC_UNUSED_PIN,
    .dio = {2, 3, 4},
};

char confver[7] = CONFVER; // version of configuration saved on eeprom

struct config_t               // configuration to save and load fron eeprom
{
  int ack;
  // This EUI must be in little-endian format, so least-significant-byte
  // first. When copying an EUI from ttnctl output, this means to reverse
  // the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
  // 0x70.
  u1_t  appeui[8];
  // This should also be in little endian format, see above.
  u1_t  deveui[8];
  // This key should be in big endian format (or, since it is not really a
  // number but a block of memory, endianness does not really apply). In
  // practice, a key taken from ttnctl can be copied as-is.
  u1_t appkey[16];
  void save () {
    int p=0;                  // save to eeprom
    p+=EEPROM_writeAnything(p, confver);
    p+=EEPROM_writeAnything(p, appeui);
    p+=EEPROM_writeAnything(p, deveui);
    p+=EEPROM_writeAnything(p, appkey);
    p+=EEPROM_writeAnything(p, ack);
  }
  bool load () {                // load from eeprom
    int p=0;
    char ver[7];
    p+=EEPROM_readAnything(p, ver);
    if (strcmp(ver,confver ) == 0){ 
      p+=EEPROM_readAnything(p, appeui);
      p+=EEPROM_readAnything(p, deveui);
      p+=EEPROM_readAnything(p, appkey);
      p+=EEPROM_readAnything(p, ack);
      return true;
    }
    else{
      return false;
    }
  }
} configuration;

ev_t event;
int sendstatus;

void os_getArtEui (u1_t* buf) { memcpy(buf, configuration.appeui, 8);}
void os_getDevEui (u1_t* buf) { memcpy(buf, configuration.deveui, 8);}
void os_getDevKey (u1_t* buf) { memcpy(buf, configuration.appkey, 16);}

//static uint8_t mydata[] = "Hello, world!";
//static osjob_t sendjob;


int send(JsonObject& params, JsonObject& result)
{
  JsonArray& a_mydata = params["payload"];
  const size_t nbyte=a_mydata.size();

  if (nbyte > 51) {
    LOGE(F("payload too big"CR));
    return 1;
  }
  uint8_t mydata[nbyte];

  size_t i=0;
  for(JsonArray::iterator it=a_mydata.begin(); it!=a_mydata.end(); ++it) {
    // *it contains the JsonVariant which can be casted as usuals
    mydata[i] = it->as<uint8_t>();    
    LOGN(F("payload %d : %d"CR),i,mydata[i]);    
    i++;
  }
  /*if (mydata == NULL ){
    LOGN(F("no payload present"CR));
    return 1;
  }
*/

  // memcpy now is not usefull but you need it if you want payload permanent for async operation (non in stack memory)
  uint8_t payload[nbyte];
  memcpy(payload, mydata, nbyte);
  
  do_send(payload,nbyte);

  if (sendstatus != 1 ){
    LOGE(F("no packet send"CR));
    return 2;
  }

  //while (!(event == NULL)){
  unsigned long starttime=millis();
  while((millis())-starttime < 30000){

    wdt_reset();
    os_runloop_once();
    
    if (event == EV_TXCOMPLETE){
      if (LMIC.txrxFlags & TXRX_ACK)
	result["ack"]= "OK";  
      if (LMIC.dataLen) {
	result["payloadlen"]= LMIC.dataLen;
      }
      result["status"]= "OK";  
      result["event"]= event;  
      return 0;
    }
  }
  return 3;
}


int set(JsonObject& params, JsonObject& result)
{    
  int ack = params["ack"];
  if (!(ack == NULL)){
    configuration.ack=ack;
    //}else{
    //return  1;
  }
  int i=0;
  JsonArray& arrayappeui = params["appeui"];
  i=0;
  for(JsonArray::iterator it=arrayappeui.begin(); it!=arrayappeui.end(); ++it) {
    // *it contains the JsonVariant which can be casted as usuals
    configuration.appeui[i] = it->as<uint8_t>();    
    i++;
    //}else{
    //return  2;
  }
  JsonArray& arraydeveui = params["deveui"];
  i=0;
  for(JsonArray::iterator it=arraydeveui.begin(); it!=arraydeveui.end(); ++it) {
    // *it contains the JsonVariant which can be casted as usuals
    configuration.deveui[i] = it->as<uint8_t>();    
    i++;
    //}else{
    //return  3;
  }
  JsonArray& arrayappkey = params["appkey"];
  i=0;
  for(JsonArray::iterator it=arrayappkey.begin(); it!=arrayappkey.end(); ++it) {
    // *it contains the JsonVariant which can be casted as usuals
    configuration.appkey[i] = it->as<uint8_t>();    
    i++;
    //}else{
    //return  4;
  }

  result["ok"]= true;  
  return 0;
}


int save(JsonObject& params, JsonObject& result)
{    
  if (params.containsKey("eeprom")){
    bool eeprom = params["eeprom"];
    if (eeprom){
      configuration.save();
      result["ok"]= true;  
    }
    return 0;
  }
  return 1;
}


void onEvent (ev_t ev) {
  LOGN(F("Time: %l : "),os_getTime());
  event=ev;
  switch(ev) {
  case EV_SCAN_TIMEOUT:
    LOGN(F("EV_SCAN_TIMEOUT"CR));
    break;
  case EV_BEACON_FOUND:
    LOGN(F("EV_BEACON_FOUND"CR));
    break;
  case EV_BEACON_MISSED:
    LOGN(F("EV_BEACON_MISSED"CR));
    break;
  case EV_BEACON_TRACKED:
    LOGN(F("EV_BEACON_TRACKED"CR));
    break;
  case EV_JOINING:
    LOGN(F("EV_JOINING"CR));
    break;
  case EV_JOINED:
    LOGN(F("EV_JOINED"CR));
    
    // Disable link check validation (automatically enabled
    // during join, but not supported by TTN at this time).
    // DISABLE FOR MOBILE STATION
    //LMIC_setLinkCheckMode(0);
    break;
  case EV_RFU1:
    LOGN(F("EV_RFU1"CR));
    break;
  case EV_JOIN_FAILED:
    LOGN(F("EV_JOIN_FAILED"CR));
    break;
  case EV_REJOIN_FAILED:
    LOGN(F("EV_REJOIN_FAILED"CR));
    break;
    break;
  case EV_TXCOMPLETE:
    LOGN(F("EV_TXCOMPLETE (includes waiting for RX windows)"CR));
    if (LMIC.txrxFlags & TXRX_ACK)
      LOGN(F("Received ack"CR));
    if (LMIC.dataLen) {
      LOGN(F("Received %d bytes of payload"CR),LMIC.dataLen);
      // data received in rx slot after tx
      //LOGN(F("Data Received: "));
      //Serial.write(LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
      //LOGN(CR);
      /*
      if (LMIC.dataLen > 1) {
	switch ((LMIC.frame + LMIC.dataBeg)[1]) {
	case 7: LMIC_setDrTxpow(DR_SF7, 14); break;
	case 8: LMIC_setDrTxpow(DR_SF8, 14); break;
	case 9: LMIC_setDrTxpow(DR_SF9, 14); break;
	case 10: LMIC_setDrTxpow(DR_SF10, 14); break;
	case 11: LMIC_setDrTxpow(DR_SF11, 14); break;
	case 12: LMIC_setDrTxpow(DR_SF12, 14); break;
	}

      }
            */
    }	    
    
  case EV_LOST_TSYNC:
    LOGN(F("EV_LOST_TSYNC"CR));
    break;
  case EV_RESET:
    LOGN(F("EV_RESET"CR));
    break;
  case EV_RXCOMPLETE:
    // data received in ping slot
    LOGN(F("EV_RXCOMPLETE"CR));
    break;
  case EV_LINK_DEAD:
    LOGN(F("EV_LINK_DEAD"CR));
    break;
  case EV_LINK_ALIVE:
    LOGN(F("EV_LINK_ALIVE"CR));
    break;
  default:
    LOGN(F("Unknown event"CR));
    break;
  }
}

void do_send(uint8_t mydata[],size_t nbyte){
  sendstatus=NULL;
  LMIC_clrTxData();
  
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    LOGN(F("OP_TXRXPEND, not sending"CR));
    sendstatus=2;
  } else {
    // Prepare upstream data transmission at the next possible time.
   // LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
   // LMIC_setTxData2(1, mydata, 4, 0);
    LMIC_setTxData2(1, mydata, nbyte, 0);
   
    LOGN(F("Packet queued"CR));
    sendstatus=1;
  }
}

void mgr_serial(){

  StaticJsonBuffer<1000> jsonBuffer;
  if (Serial.available()) {
    JsonObject& msg = jsonBuffer.parse(Serial);
      if (msg.success()){
	int err=rpcserver.processMessage(msg);
	LOGN(F("rpc processMessage return status: %d"CR),err);
	msg.printTo(Serial);
	Serial.println("");
    }else{
      LOGN("error decoding msg"CR);      
    }
  }
}

void mgr_sensors(){

  long unsigned int waittime,maxwaittime=0;

  LOGN(F("mgr_sensors"CR));
  
  // prepare sensors to measure
  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == NULL){
      if (sd[i]->prepare(waittime) == SD_SUCCESS){
	maxwaittime=max(maxwaittime,waittime);
      }else{
	Serial.print(sensors[i].driver);
	Serial.println(": prepare failed !");
      }
    }
  }

  //wait sensors to go ready
  //Serial.print("# wait sensors for ms:");  Serial.println(maxwaittime);
  delay(maxwaittime);  // 500 for tmp and 250 for adt and 2500 for davis

  unsigned long data;
  unsigned short width;

  /*
  unsigned short  mytemplate=1;
  auto &b_mytemplate = toBitBool( mytemplate );

  BitBool<32> bits;
  for( int i_Index = 8 - 1 ; i_Index ; --i_Index )
      bits[ i_Index ] = b_mytemplate[ i_Index ];

  
  //for( int i_Index = 8 - 1 ; i_Index ; --i_Index )
  //    dtemplate.bittemplate[ i_Index ] = b_mytemplate[ i_Index ];
  */
  /*
  dtemplate.dtemplate1.templaten=1;
  dtemplate.dtemplate1.data[0]=1;
  dtemplate.dtemplate1.data[1]=2;
  dtemplate.dtemplate1.data[2]=3;
  LOGN(F("-> %B"),dtemplate.dtemplate1.templaten);	
  LOGN(F("-> %B"CR),dtemplate.dtemplate1.data);	
  */

  size_t nbyte=4;
  unsigned char dtemplate[nbyte];
  unsigned long bit_offset=1;
  unsigned long bit_len=8;
  bfi(dtemplate, bit_offset, bit_len, 1,2); // template number
  bit_offset+=bit_len;
    
  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == NULL){

      // get  values 

      if (sd[i]->getdata(data,width) == SD_SUCCESS){
	LOGN(F("%d OK"CR),i);	
	LOGN(F("%d data: %B"CR),i,data);	
	//Serial.println(width);

	bit_len=width;
	bfi(dtemplate, bit_offset, bit_len, data,2); // template number
	bit_offset+=bit_len;
	
      }else{
	LOGN(F("Error"CR));	
      }      
    }
  }

  for (int i = 0; i < nbyte; i++) {
    LOGN(F("template: %B"CR),dtemplate[i]);	
  }    

  do_send(dtemplate,nbyte);

  
}

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
  wdt_enable(WDTO_8S);


  
  Serial.begin(19200);
  //while (!Serial); // wait for serial port to connect. Needed for native USB

  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  LOGN(F("Started"CR));
  
  if (configuration.load()){
    LOGN(F("Configuration loaded"CR));
  } else {
    LOGN(F("Configuration not loaded"CR));
    configuration.ack=0;
  }
  LOGN(F("ack: %d"CR),configuration.ack);
  LOGN(F("deveui: %x,%x,%x,%x,%x,%x,%x,%x"CR),configuration.deveui[0]
       ,configuration.deveui[1]
       ,configuration.deveui[2]
       ,configuration.deveui[3]
       ,configuration.deveui[4]
       ,configuration.deveui[5]
       ,configuration.deveui[6]
       ,configuration.deveui[7]);
  LOGN(F("appeui: %x,%x,%x,%x,%x,%x,%x,%x"CR),configuration.appeui[0]
       ,configuration.appeui[1]
       ,configuration.appeui[2]
       ,configuration.appeui[3]
       ,configuration.appeui[4]
       ,configuration.appeui[5]
       ,configuration.appeui[6]
       ,configuration.appeui[7]);
  LOGN(F("appkey: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x"CR),configuration.appkey[0]
       ,configuration.appkey[1]
       ,configuration.appkey[2]
       ,configuration.appkey[3]
       ,configuration.appkey[4]
       ,configuration.appkey[5]
       ,configuration.appkey[6]
       ,configuration.appkey[7]
       ,configuration.appkey[8]
       ,configuration.appkey[9]
       ,configuration.appkey[10]
       ,configuration.appkey[11]
       ,configuration.appkey[12]
       ,configuration.appkey[13]
       ,configuration.appkey[14]
       ,configuration.appkey[15]);


  // start up the i2c interface
  Wire.begin();

  // set the frequency 
#define I2C_CLOCK 50000

  //set the i2c clock 
  TWBR = ((F_CPU / I2C_CLOCK) - 16) / 2;

  setTime(12,0,0,1,1,17); // set time to 12:00:00am Jan 1 2017

  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"ADT");
  sensors[0].address=73;
  strcpy(sensors[1].driver,"I2C");
  strcpy(sensors[1].type,"HIH");
  sensors[1].address=39;
  
  for (int i = 0; i < SENSORS_LEN; i++) {

    sd[i]=SensorDriver::create(sensors[i].driver,sensors[i].type);
    if (sd[i] == NULL){
      Serial.print(sensors[i].driver);
      Serial.println(": driver not created !");
    }else{
      Serial.print(sensors[i].driver);
      Serial.println(": driver created");
      sd[i]->setup(sensors[i].driver,sensors[i].address);
    }
  }
  
  // register the local method
  rpcserver.registerMethod("send",      &send);
  rpcserver.registerMethod("set",       &set);
  rpcserver.registerMethod("save",      &save);
  
  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
  LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);
  //LMIC_setDrTxpow(DR_SF12, 14);
  LMIC_setAdrMode(1);
  LMIC_startJoining();

  // query and send data
  Alarm.timerRepeat(SAMPLETIME, mgr_sensors);             // timer for every tr seconds

  // millis() and other can have overflow problem
  // so we reset everythings one time a week
  //Alarm.alarmRepeat(dowMonday,8,0,0,reboot);          // 8:00:00 every Monday

  // upgrade LMIC
  Alarm.alarmRepeat(4,0,0,LMIC_tryRejoin);          // 4:00:00 every day  

  LOGN(F("End setup"CR));
  
}

void loop() {
  wdt_reset();
  mgr_serial();
  os_runloop_once();
  Alarm.delay(0);
}
