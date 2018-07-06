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
 * This example sends a valid LoRaWAN packet with payload as in RMAP definition
 * for LoraWan transmissions, using frequency and encryption settings matching those of
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
#ifndef DEEPSLEEP
#include <Time.h>
#include <TimeAlarms.h>
#endif
#include <Wire.h>
#include <SensorDriver.h>
#define SENSORS_LEN 3
//#include <BitBool.h>
#include <bfix.h>
#include <Sleep_n0m1.h>

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
unsigned short int sensors_len;

// initialize an instance of the JsonRPC library for registering 
JsonRPC rpcserver(false ); //standard protocol

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LMIC_UNUSED_PIN,
    .dio = {DIO0, DIO1, DIO2},
};

volatile bool pdown;

const uint8_t pins [] = {OUTPUTPINS};

// layout of session parameters
typedef struct {
  unsigned short int joinstatus; // 2 as joined
  u4_t netid;          // network id
  devaddr_t devaddr;   // device address
  u1_t nwkkey[16];     // network session key
  u1_t artkey[16];     // application session key
  u4_t seqnoUp;        // up sequence counter
  u4_t seqnoDn;        // down sequence counter
} sessparam_t;

char confver[7] = CONFVER; // version of configuration saved on eeprom

struct config_t               // configuration to save and load fron eeprom
{
  bool ack;
  bool mobile;
  short unsigned int sf;
  short unsigned int mytemplate;
  unsigned int sampletime;
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
  sessparam_t session;
  
  void save () {
    int p=0;                  // save to eeprom
    p+=EEPROM_writeAnything(p, confver);
    p+=EEPROM_writeAnything(p, appeui);
    p+=EEPROM_writeAnything(p, deveui);
    p+=EEPROM_writeAnything(p, appkey);
    p+=EEPROM_writeAnything(p, ack);
    p+=EEPROM_writeAnything(p, mobile);
    p+=EEPROM_writeAnything(p, sf);
    p+=EEPROM_writeAnything(p, mytemplate);
    p+=EEPROM_writeAnything(p, session);
    p+=EEPROM_writeAnything(p, sampletime);
  }

  void reset () {
    ack=false;
    mobile=false;
    sf=9;
    mytemplate=1;
    sampletime=900;
    session.joinstatus=0;
    // TODO
    // reset key for security
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
      p+=EEPROM_readAnything(p, mobile);
      p+=EEPROM_readAnything(p, sf);
      p+=EEPROM_readAnything(p, mytemplate);
      p+=EEPROM_readAnything(p, session);
      p+=EEPROM_readAnything(p, sampletime);
      return true;
    }
    else{
      reset();
      return false;
    }
  }
} configuration;

ev_t event;
short unsigned int sendstatus;
short unsigned int sendcompletedstatus;
short unsigned int joinstatus;

void os_getArtEui (u1_t* buf) { memcpy(buf, configuration.appeui, 8);}
void os_getDevEui (u1_t* buf) { memcpy(buf, configuration.deveui, 8);}
void os_getDevKey (u1_t* buf) { memcpy(buf, configuration.appkey, 16);}

//static uint8_t mydata[] = "Hello, world!";
//static osjob_t sendjob;

Sleep sleep;

void printDataRate() {
  switch (LMIC.datarate) {
  case DR_SF12: LOGN(F("Datarate: SF12"CR)); break;
  case DR_SF11: LOGN(F("Datarate: SF11"CR)); break;
  case DR_SF10: LOGN(F("Datarate: SF10"CR)); break;
  case DR_SF9:  LOGN(F("Datarate: SF9"CR)); break;
  case DR_SF8:  LOGN(F("Datarate: SF8"CR)); break;
  case DR_SF7:  LOGN(F("Datarate: SF7"CR)); break;
  case DR_SF7B: LOGN(F("Datarate: SF7B"CR)); break;
  case DR_FSK:  LOGN(F("Datarate: FSK"CR)); break;
  default:      LOGN(F("Datarate Unknown Value: %d"CR), LMIC.datarate); break;
  }
}


int shutdown(JsonObject& params, JsonObject& result) {
  LOGN(F("shutdown"CR));
  delay(100); //delay to allow serial to fully print before sleep  
  setpowerdown();
  return 0;
}

void setpowerdown(){
  pdown=true;
}
void unsetpowerdown(){
  pdown=false;
}
bool checkpowerdown(){
  return pdown;
}

void powerdown(){

  LOGN(F("entering powerdown" CR));
  delay(1000); //debounce time

  noInterrupts ();
  
  unsetpowerdown();

  
  if (digitalRead(POWERPIN) == HIGH) {
    LOGN(F("powerdown canceled" CR));
    interrupts ();
    return;
  }

  if (joinstatus == 2){
    configuration.session.joinstatus=joinstatus;
    configuration.session.netid = LMIC.netid;
    configuration.session.devaddr = LMIC.devaddr;
    memcpy(configuration.session.nwkkey, LMIC.nwkKey, 16);
    memcpy(configuration.session.artkey, LMIC.artKey, 16);
    configuration.session.seqnoUp=LMIC.seqnoUp;
    configuration.session.seqnoDn=LMIC.seqnoDn;
    LOGN(F("save configuration" CR));
    configuration.save();
  }

  if (digitalRead(POWERPIN) == HIGH) {
    LOGN(F("powerdown canceled after save" CR));
    interrupts ();
    return;
  }

  detachInterrupt(digitalPinToInterrupt(POWERPIN));

  LOGN(F("POWERDOWN" CR));
  EIFR |= (1 << INTF1); // | (1 << INTF0);    //https://github.com/arduino/Arduino/issues/510
  wdt_disable();
  
  interrupts ();
  //LMIC_shutdown();
  delay(3000); //flush any serial output
  digitalWrite(POWERLED, 0);
  sleep.pwrDownMode(); //set sleep mode
  //Sleep till interrupt pin equals a particular state.
  sleep.sleepInterrupt(digitalPinToInterrupt(POWERPIN),RISING); //(interrupt Number, interrupt State)

  LOGN(F("WAKEUP" CR));
  wdt_disable();
  wdt_enable(WDTO_8S);
  attachInterrupt(digitalPinToInterrupt(POWERPIN),setpowerdown,FALLING);
  digitalWrite(POWERLED, 1);

  // wait for sensor to go ready (for that powered by other source than mcu)
  delay(1000);
  
  // I need to setup sensors after a powerdown
  for (int i = 0; i < sensors_len; i++) {
    if (sd[i] == NULL){
      LOGN(F("sensor absent %d"CR),i);
    }else{
      LOGN(F("sensor setup %d "CR),i);
      sd[i]->setup(sensors[i].driver,sensors[i].address);
    }
  }

  // restart LMIC
  //LMIC_reset();
  //LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);

}



void reboot(){

  if (joinstatus == 2){
    configuration.session.joinstatus=joinstatus;
    configuration.session.netid = LMIC.netid;
    configuration.session.devaddr = LMIC.devaddr;
    memcpy(configuration.session.nwkkey, LMIC.nwkKey, 16);
    memcpy(configuration.session.artkey, LMIC.artKey, 16);
    configuration.session.seqnoUp=LMIC.seqnoUp;
    configuration.session.seqnoDn=LMIC.seqnoDn;
    LOGN(F("save configuration" CR));
    configuration.save();
  }

  //Reboot mode
  wdt_enable(WDTO_30MS); while(1) {} 
  // Restarts program from beginning but 
  // does not reset the peripherals and registers
  //asm volatile ("  jmp 0");
}

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
  // Timeout when there's no "EV_TXCOMPLETE" event after TXTIMEOUT seconds
  unsigned long starttime=millis();
  while((millis()-starttime) < (TXTIMEOUT*1000UL) && sendcompletedstatus == 0){

    wdt_reset();
    mgr_serial();
    os_runloop_once();
  }
  
  if (LMIC.dataLen) {
    result["payloadlen"]= LMIC.dataLen;
  }
  
  if (sendcompletedstatus == 1){
    LOGN(F("Send completed"CR));
    result["status"]= "OK";
    return 0;
  }else if (sendcompletedstatus == 2){
    LOGN(F("Sent but NO ack"CR));
    result["status"]= "OK";
    result["ack"]= "KO";
    return 1;
  }else{
    LOGE(F("Send NOT completed"CR));
    result["status"]= "KO";
    return 3;
  }
}


int set(JsonObject& params, JsonObject& result)
{
  LOGN(F("set method"CR));

  if (params["reset"])
    configuration.reset();
  
  if (params.containsKey("ack"))
    configuration.ack= params["ack"];

  if (params.containsKey("mobile"))
    configuration.mobile=params["mobile"];

  if (params.containsKey("template"))
    configuration.mytemplate=params["template"];
  
  if (params.containsKey("sampletime"))
    configuration.sampletime=params["sampletime"];
  
  if (params.containsKey("sf")) {
    configuration.sf=params["sf"];
    if (setsf(configuration.sf) == 0) return 5;
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


  if (params.containsKey("netid"))
    configuration.mobile=params["netid"];

  if (params.containsKey("devaddr"))
    configuration.mobile=params["devaddr"];

  if (params.containsKey("seqnoUp"))
    configuration.mobile=params["seqnoUp"];

  if (params.containsKey("seqnoDn"))
    configuration.mobile=params["seqnoDn"];
  
  JsonArray& arraynwkkey = params["nwkkey"];
  i=0;
  for(JsonArray::iterator it=arraynwkkey.begin(); it!=arraynwkkey.end(); ++it) {
    // *it contains the JsonVariant which can be casted as usuals
    configuration.session.nwkkey[i] = it->as<uint8_t>();    
    i++;
  }

  JsonArray& arrayartkey = params["artkey"];
  i=0;
  for(JsonArray::iterator it=arrayartkey.begin(); it!=arrayartkey.end(); ++it) {
    // *it contains the JsonVariant which can be casted as usuals
    configuration.session.artkey[i] = it->as<uint8_t>();    
    i++;
  }
  
  result["ok"]= true;  
  return E_SUCCESS;
}


int save(JsonObject& params, JsonObject& result)
{    
  if (params.containsKey("eeprom")){
    bool eeprom = params["eeprom"];
    if (eeprom){

      if (joinstatus == 2){
	configuration.session.joinstatus=joinstatus;
	configuration.session.netid = LMIC.netid;
	configuration.session.devaddr = LMIC.devaddr;
	memcpy(configuration.session.nwkkey, LMIC.nwkKey, 16);
	memcpy(configuration.session.artkey, LMIC.artKey, 16);
	configuration.session.seqnoUp=LMIC.seqnoUp;
	configuration.session.seqnoDn=LMIC.seqnoDn;
    
      }

      configuration.save();
      result["ok"]= true;  
    }
    return 0;
  }
  return 1;
}

int setsf(int sf){
  unsigned int dr_sf;
  
  switch (sf) {
  case 12: dr_sf=DR_SF12; break;
  case 11: dr_sf=DR_SF11; break;
  case 10: dr_sf=DR_SF10; break;
  case  9: dr_sf=DR_SF9 ; break;
  case  8: dr_sf=DR_SF8 ; break;
  case  7: dr_sf=DR_SF7 ; break;
  case  6: dr_sf=DR_SF7B; break;
  case  5: dr_sf=DR_FSK ; break;
  default: return 0 ; break;
  }
  // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
  //https://github.com/matthijskooijman/arduino-lmic/issues/33
  LMIC_setDrTxpow(dr_sf, 14);
}

// set/return DR_SF parameter
int sf(JsonObject& params, JsonObject& result)
{    
  int sf = params["sf"];
  if (!(sf == NULL)){
    if (setsf(sf) == 0) return  2;
  }

  switch (LMIC.datarate) {
  case DR_SF12: sf=12; break;
  case DR_SF11: sf=11; break;
  case DR_SF10: sf=10; break;
  case DR_SF9:  sf=9 ; break;
  case DR_SF8:  sf=8 ; break;
  case DR_SF7:  sf=7 ; break;
  case DR_SF7B: sf=6 ; break;
  case DR_FSK:  sf=5 ; break;
  default:      sf=0 ; break;
  }
  
  result["sf"]= sf;  
  return 0;
}



void onEvent (ev_t ev) {
  LOGN(F("Time: %l : "CR),os_getTime());
  event=ev;
  switch(ev) {
  case EV_SCAN_TIMEOUT:
    LOGN(F("EV_SCAN_TIMEOUT"CR));
    break;
  case EV_BEACON_FOUND:
    LOGN(F("EV_BEACON_FOUND"CR));
    /*
    if(configuration.ping){
      // send empty frame up to notify server of ping mode and interval!
      LMIC_sendAlive();
    }
    */
    break;
  case EV_BEACON_MISSED:
    LOGN(F("EV_BEACON_MISSED"CR));
    break;
  case EV_BEACON_TRACKED:
    LOGN(F("EV_BEACON_TRACKED"CR));
    LOGN(F("GPS time = "), LMIC.bcninfo.time);
    break;
  case EV_JOINING:
    LOGN(F("EV_JOINING"CR));
    joinstatus=1;
    break;
  case EV_JOINED:
    LOGN(F("EV_JOINED"CR));
    LOGN(F("netid = %d"CR), LMIC.netid);

    joinstatus=2;
    
    //https://github.com/matthijskooijman/arduino-lmic/pull/64    
    if (configuration.mobile){
      // DISABLE THOSE FOR MOBILE STATION
      LMIC_setLinkCheckMode(0);
      LMIC_setAdrMode(0);
      setsf(configuration.sf);
    }else{
      // Enaable link check validation
      LMIC_setLinkCheckMode(1);
      LMIC_setAdrMode(1);
    }

    /*
    if (configuration.beacon) {
      // enable beacon
      LMIC_enableTracking(0);
    }
    
    if (configuration.ping) {
      // enable ping
      LMIC_setPingable(1);
    }
    */
    
    break;
  case EV_RFU1:
    LOGN(F("EV_RFU1"CR));
    break;
  case EV_JOIN_FAILED:
    LOGN(F("EV_JOIN_FAILED"CR));
    joinstatus=3;
    break;
  case EV_REJOIN_FAILED:
    LOGN(F("EV_REJOIN_FAILED"CR));
    joinstatus=4;
    break;
    break;
  case EV_TXCOMPLETE:
    LOGN(F("EV_TXCOMPLETE (includes waiting for RX windows)"CR));
    if (LMIC.txrxFlags & TXRX_ACK){
      LOGN(F("Received ack"CR));
      sendcompletedstatus=1;
    }else{
      if (configuration.ack) {
	sendcompletedstatus=2;
      }else{
	sendcompletedstatus=1;	
      }
    }
    
    //break;    

  case EV_RXCOMPLETE:

    // data received in ping slot
    LOGN(F("EV_RXCOMPLETE"CR));

    if (LMIC.dataLen) {
      LOGN(F("Received %d bytes of payload"CR),LMIC.dataLen);
      // data received in rx slot after tx

      u1_t* buf= LMIC.frame + LMIC.dataBeg;
      u2_t len=LMIC.dataLen;
      LOGN(F("Data Received: "));
      while(len--) {
	LOGN(F(" %X"), *buf++);
      }
      LOGN(F(CR));

      mgr_jsonrpc();

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

    if(LMIC.dataLen == 1) {
      // set PING if exactly one byte is received
      LOGN(F("PING message %d"CR),LMIC.frame[LMIC.dataBeg] & 0x01);
    }
  
    /*
    u1_t f = LMIC.txrxFlags; // EV_XXCOMPLETE,FF[,PP[,DDDDDDDD...]]
    buf = buffer_alloc(3 + len + 1 + 2 + ((f & TXRX_PORT) ? 3 : 0) + (LMIC.dataLen ? 1+2*LMIC.dataLen:0) + 2);
    memcpy(buf, "EV_", 3);
    memcpy(buf+3, evnames[ev], len);
    len += 3;
    buf[len++] = ',';
    // flags
    buf[len++] = (f & TXRX_ACK) ? 'A' : (f & TXRX_NACK) ? 'N' : '0';
    buf[len++] = (f & TXRX_DNW1) ? '1' : (f & TXRX_DNW2) ? '2' : (f & TXRX_PING) ? 'P' : '0';
    if(f & TXRX_PORT) { // port
      buf[len++] = ',';
      len += puthex(buf+len, &LMIC.frame[LMIC.dataBeg-1], 1);
    }
    if(LMIC.dataLen) { // downstream data
      buf[len++] = ',';
      len += puthex(buf+len, LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
    }
    */

    break;

  case EV_LOST_TSYNC:
    LOGN(F("EV_LOST_TSYNC"CR));
    break;
  case EV_RESET:
    LOGN(F("EV_RESET"CR));
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
  sendcompletedstatus=NULL;
  LMIC_clrTxData();
  
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    LOGE(F("OP_TXRXPEND, not sending"CR));
    sendstatus=2;
  } else {

    // reset event status
    event = EV_RESET;
    // Prepare upstream data transmission at the next possible time.
    // send to port 1
    LMIC_setTxData2(1, mydata, nbyte, configuration.ack);
    LOGN(F("Packet queued"CR));
    sendstatus=1;
  }
}
 

 void mgr_jsonrpc(){
   
  const u1_t* buf= LMIC.frame + LMIC.dataBeg;
  unsigned int bitlen=LMIC.dataLen*8;

  unsigned long bit_offset=1; // bfix library start from 1, not 0
  unsigned long bit_len=8;
  
  long jsrpc=bfx(buf, bit_offset, bit_len,2); // template number
  bit_offset+=bit_len;
  LOGN(F(" jsrpc %d"CR), jsrpc);

  switch (jsrpc) {
  case 1: {

    if (bitlen < 25) {
      LOGE(F("template mismach"CR));
      break;
    }
    bit_len=1;
    long save=bfx(buf, bit_offset, bit_len,2); // save parameter
    bit_offset+=bit_len;
    LOGN(F(" save %d"CR), save);
    
    bit_len=16;
    long sampletime=bfx(buf, bit_offset, bit_len,2); // sampletime parameter
    bit_offset+=bit_len;
    LOGN(F(" sampletime %d"CR), sampletime);

    configuration.sampletime=sampletime;
    
    if (save == 1){
      LOGN(F("save configuration" CR));
      configuration.save();
    }
    
    break;
  }

  case 2: {

    if (bitlen < 13) {
      LOGE(F("template mismach"CR));
      break;
    }

    while (bit_offset+5 <= bitlen+1 ) {
      //LOGN("bit_offset %d; bitlen %d"CR, bit_offset+5, bitlen);
      bit_len=4;
      long pin=bfx(buf, bit_offset, bit_len,2); // pin parameter
      bit_offset+=bit_len;
      LOGN(F(" pin %d"CR), pin);
    
      bit_len=1;
      long state=bfx(buf, bit_offset, bit_len,2); // state parameter
      bit_offset+=bit_len;
      LOGN(F(" state %d"CR), state);

      if (pin == 0){
	LOGN(F(" SKIP missed pin %d"CR), pin);
	continue;
      }

      bool pinok = false;
      for (uint8_t i=0;i < sizeof(pins)/sizeof(*pins) ;i++) 
	if (pin == pins[i]) pinok=true;
      if (pinok){
	digitalWrite(pin, state);
      }else{
	LOGN(F(" SKIP not mapped pin %d"CR), pin);
      }
    }

    //LOGN(F("final bit_offset %d ;  bitlen %d"CR), bit_offset+5, bitlen);
      
    break;
  }

  default:
    LOGN(F("Unknown jsrpc: %d"CR), jsrpc);
    break;
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
	LOGN(F("error decoding msg"CR));      
    }
  }
}

void mgr_sensors(){

  long unsigned int waittime,maxwaittime=0;

  LOGN(F("mgr_sensors"CR));
  
  // prepare sensors to measure
  for (int i = 0; i < sensors_len; i++) {
    if (!sd[i] == NULL){
      LOGN(F("prepare for: %s %s %d"CR),sensors[i].driver,sensors[i].type,sensors[i].address);
      if (sd[i]->prepare(waittime) == SD_SUCCESS){
	maxwaittime=max(maxwaittime,waittime);
      }else{
	LOGN(F("Error"CR));
      }
    }
  }

  //wait sensors to go ready
  //Serial.print("# wait sensors for ms:");  Serial.println(maxwaittime);
  unsigned long starttime=millis();
  while((millis()-starttime) < maxwaittime){
    wdt_reset();
    mgr_serial();
    os_runloop_once();
  }

  unsigned long data;
  unsigned short width;
  size_t nbyte=0;
  
  // inizialize template
  if (configuration.mytemplate == 1){
    nbyte=4;  // (16+7)/8 +1
  }else if (configuration.mytemplate == 2){
    nbyte=7; // (16+7+20)/8 +1
  }
  unsigned char dtemplate[nbyte]={0xFF};

  // set template number

  unsigned long bit_offset=1; // bfix library start from 1, not 0
  unsigned long bit_len=8;

  bfi(dtemplate, bit_offset, bit_len, configuration.mytemplate,2); // template number

  bit_offset+=bit_len;
    
  for (int i = 0; i < sensors_len; i++) {
    if (!sd[i] == NULL){

      // get  values 
      LOGN(F("getdata for: %s %s %d"CR),sensors[i].driver,sensors[i].type,sensors[i].address);

      if (sd[i]->getdata(data,width) == SD_SUCCESS){
	LOGN(F("%d OK"CR),i);	
	LOGN(F("%d data: %B"CR),i,data);
      }else{
	//data=0xFFFFFFFF;  // not required
	LOGN(F("Error"CR));
      }

      bit_len=width;
      bfi(dtemplate, bit_offset, bit_len, data,2); // template number
      bit_offset+=bit_len;

    }
  }

  for (int i = 0; i < nbyte; i++) {
    LOGN(F("template: %B"CR),dtemplate[i]);	
  }    

  do_send(dtemplate,nbyte);

  if (sendstatus != 1 ){
    LOGE(F("no packet send"CR));
  }

  printDataRate();
  
  // Timeout when there's no "EV_TXCOMPLETE" event after TXTIMEOUT seconds
  starttime=millis();
  while((millis()-starttime) < (TXTIMEOUT*1000UL) && sendcompletedstatus == 0){
    wdt_reset();
    mgr_serial();
    os_runloop_once();
  }

  if (sendcompletedstatus == 1){
    LOGN(F("Send completed"CR));
  }else if (sendcompletedstatus == 2){
    LOGN(F("Sent but NO ack"CR));
  }else{
    LOGE(F("Send NOT completed"CR));
  }
}

void sleep_mgr_sensors() {

  mgr_sensors();
  LOGN(F("sleep %d seconds"CR),configuration.sampletime);
  delay(1000);
  os_runloop_once();
  // Enter sleep mode
  sleep.pwrDownMode(); //set sleep mode

  unsigned int timetosleep=configuration.sampletime;
  addsleepedtime(configuration.sampletime*1000UL);
  #define SLEEPSTEP 60    // max delay between a powerdown request and effectictive powerdown 
  while(timetosleep > SLEEPSTEP){
    //LOGN(F("sleep..."CR));
    sleep.sleepDelay(SLEEPSTEP*1000UL); //sleep for SAMPLETIME
    timetosleep -= SLEEPSTEP;
    //LOGN(F("have to sleep %d seconds"CR),timetosleep);
    if (checkpowerdown()) powerdown();
  }
  sleep.sleepDelay(timetosleep*1000UL); //sleep for SAMPLETIME

  delay(1000);
  LOGN(F("wake up"CR));  
  os_runloop_once();
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

  bool waitforconf=false;

  // register the local method
  rpcserver.registerMethod("send",      &send);
  rpcserver.registerMethod("set",       &set);
  rpcserver.registerMethod("save",      &save);
  rpcserver.registerMethod("sf",        &sf);
  rpcserver.registerMethod("shutdown",  &shutdown);
  
  Serial.begin(19200);
  //while (!Serial); // wait for serial port to connect. Needed for native USB

  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  LOGN(F("Started"CR));
  
  if (configuration.load()){
    LOGN(F("Configuration loaded" CR));
  } else {
    LOGN(F("Configuration not loaded" CR));
    waitforconf=true;
  }

  pinMode(FORCECONFIGPIN, INPUT_PULLUP);
  pinMode(POWERPIN, POWERPIN_PULL);

  if (digitalRead(FORCECONFIGPIN) == LOW) {
    LOGN(F("force configuration by serial" CR));
    waitforconf=true;
  }
  
  while(waitforconf) {
    mgr_serial();
    wdt_reset();
  }

  noInterrupts ();
  unsetpowerdown();
  attachInterrupt(digitalPinToInterrupt(POWERPIN),setpowerdown,FALLING);
  interrupts ();
  
  LOGN(F("sampletime: %d"CR),configuration.sampletime);
  LOGN(F("ack: %d"CR),configuration.ack);
  LOGN(F("mobile: %d"CR),configuration.mobile);
  LOGN(F("sf: %d"CR),configuration.sf);
  LOGN(F("template: %d"CR),configuration.mytemplate);
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


  LOGN(F("joinstatus: %d"CR),configuration.session.joinstatus);
  LOGN(F("netid: %d"CR),configuration.session.netid);
  LOGN(F("devaddr: %d"CR),configuration.session.devaddr);

  LOGN(F("nwkkey: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x"CR),configuration.session.nwkkey[0]
       ,configuration.session.nwkkey[1]
       ,configuration.session.nwkkey[2]
       ,configuration.session.nwkkey[3]
       ,configuration.session.nwkkey[4]
       ,configuration.session.nwkkey[5]
       ,configuration.session.nwkkey[6]
       ,configuration.session.nwkkey[7]
       ,configuration.session.nwkkey[8]
       ,configuration.session.nwkkey[9]
       ,configuration.session.nwkkey[10]
       ,configuration.session.nwkkey[11]
       ,configuration.session.nwkkey[12]
       ,configuration.session.nwkkey[13]
       ,configuration.session.nwkkey[14]
       ,configuration.session.nwkkey[15]);

  LOGN(F("artkey: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x"CR),configuration.session.artkey[0]
       ,configuration.session.artkey[1]
       ,configuration.session.artkey[2]
       ,configuration.session.artkey[3]
       ,configuration.session.artkey[4]
       ,configuration.session.artkey[5]
       ,configuration.session.artkey[6]
       ,configuration.session.artkey[7]
       ,configuration.session.artkey[8]
       ,configuration.session.artkey[9]
       ,configuration.session.artkey[10]
       ,configuration.session.artkey[11]
       ,configuration.session.artkey[12]
       ,configuration.session.artkey[13]
       ,configuration.session.artkey[14]
       ,configuration.session.artkey[15]);

  LOGN(F("seqnoUp: %d"CR),configuration.session.seqnoUp);
  LOGN(F("seqnoDn: %d"CR),configuration.session.seqnoDn);

  // start up the i2c interface
  Wire.begin();

  // set the frequency 
#define I2C_CLOCK 50000

  //set the i2c clock 
  TWBR = ((F_CPU / I2C_CLOCK) - 16) / 2;

#ifndef DEEPSLEEP
  setTime(12,0,0,1,1,17); // set time to 12:00:00am Jan 1 2017
#endif

  // initialize the digital pin as an output
  for (uint8_t i=0; i< sizeof(pins)/sizeof(*pins) ; i++){
    pinMode(pins[i], OUTPUT);
    LOGN(F("set pins for ATTUATORE: %d"CR),pins[i]);
  }

  pinMode(POWERLED, OUTPUT);
  digitalWrite(POWERLED, 1);
  
  if (configuration.mytemplate == 1 || configuration.mytemplate == 2){
    sensors_len=2;
    strcpy(sensors[0].driver,"I2C");
    strcpy(sensors[0].type,"ADT");
    sensors[0].address=73;
    strcpy(sensors[1].driver,"I2C");
    strcpy(sensors[1].type,"HIH");
    sensors[1].address=39;
  }
  
  if (configuration.mytemplate == 2){ 
    sensors_len=3;
    strcpy(sensors[2].driver,"SERI");
    strcpy(sensors[2].type,"HPM");
    sensors[2].address=36;
  }
  
  for (int i = 0; i < sensors_len; i++) {

    sd[i]=SensorDriver::create(sensors[i].driver,sensors[i].type);
    LOGN(F("setup for %s %s %d"CR) ,sensors[i].driver,sensors[i].type,sensors[i].address);
    if (sd[i] == NULL){
      LOGN(F("driver not created !"CR));
    }else{
      LOGN(F("driver created"CR));
      sd[i]->setup(sensors[i].driver,sensors[i].address);
    }
  }
  

  //Unused IO pins on the microcontroller must not be left floating in
  //an unknown state. Floating IO pins can consumes at least few tens
  //of μA and that can add up quite a bit if you have few of them
  //floating around. Configure unused IO pins to enable the build-in
  //internal pull up.

  // ***** Put unused pins into known state *****
  //pinMode(0, INPUT_PULLUP)
  
  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
  LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);

// not shure if we need this
#if defined(CFG_eu868)
  // Set up the channels used by the Things Network, which corresponds
  // to the defaults of most gateways. Without this, only three base
  // channels from the LoRaWAN specification are used, which certainly
  // works, so it is good for debugging, but can overload those
  // frequencies, so be sure to configure the full frequency range of
  // your network here (unless your network autoconfigures them).
  // Setting up channels should happen after LMIC_setSession, as that
  // configures the minimal channel set.
  // NA-US channels 0-71 are configured automatically
  /*
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
  */
  // For single channel gateways: Restrict to channel 0 when defined above
#ifdef CHANNEL0
  LMIC_disableChannel(1);
  LMIC_disableChannel(2);
  LMIC_disableChannel(3);
  LMIC_disableChannel(4);
  LMIC_disableChannel(5);
  LMIC_disableChannel(6);
  LMIC_disableChannel(7);
  LMIC_disableChannel(8);
#endif
  
  // TTN defines an additional channel at 869.525Mhz using SF9 for class B
  // devices' ping slots. LMIC does not have an easy way to define set this
  // frequency and support for class B is spotty and untested, so this
  // frequency is not configured here.
  
#elif defined(CFG_us915)
  // NA-US channels 0-71 are configured automatically
  // but only one group of 8 should (a subband) should be active
  // TTN recommends the second sub band, 1 in a zero based count.
  // https://github.com/TheThingsNetwork/gateway-conf/blob/master/US-global_conf.json
  LMIC_selectSubBand(1);
#endif


  // Set data rate and transmit power for uplink
  setsf(configuration.sf);

  // Use a medium spread factor. This can be increased up to SF12 for
  // better range, but then the interval should be (significantly)
  // lowered to comply with duty cycle limits as well.
  //LMIC.datarate = DR_SF9;
  // This sets CR 4/5, BW125 (except for DR_SF7B, which uses BW250)
  //LMIC.rps = updr2rps(LMIC.datarate);

  if (configuration.session.joinstatus == 2) {

    LMIC_setSession (configuration.session.netid,
		     configuration.session.devaddr,
		     configuration.session.nwkkey,
		     configuration.session.artkey);
    LMIC.seqnoDn = configuration.session.seqnoDn;
    LMIC.seqnoUp = configuration.session.seqnoUp + 2; // avoid reuse of seq numbers

    // TTN uses SF9 for its RX2 window.
    //https://github.com/matthijskooijman/arduino-lmic#downlink-datarate
    //https://github.com/matthijskooijman/arduino-lmic/issues/20
    //https://github.com/matthijskooijman/arduino-lmic/pull/23
    
    LMIC.dn2Dr = DR_SF9;

    /*
    if (configuration.beacon) {
      // enable beacon
      LMIC_enableTracking(0);
    }
    
    if (configuration.ping) {
      // enable ping
      LMIC_setPingable(1);
    }
    */
    
    joinstatus = 2;

  }else{
  
    joinstatus=0;
    
    while (joinstatus != 2){
      LOGN(F("startJoining"CR));  
      LMIC_startJoining();
      setsf(configuration.sf);
      //setDrJoin(DRCHG_SET, DR_SF12);
      
      //unsigned long starttime=millis();
      //while((millis()-starttime) < 60000UL){
      while( joinstatus < 2) {
	wdt_reset();
	mgr_serial();
	os_runloop_once();
      }
      
      if (joinstatus != 2){
#if defined(DEEPSLEEP)
	// Enter sleep mode
	LOGN(F("sleep"CR));  
	delay(100);
	sleep.pwrDownMode(); //set sleep mode
	sleep.sleepDelay(JOINRETRYDELAY*1000UL); //sleep
	addsleepedtime(JOINRETRYDELAY*1000UL);
	delay(100);
	LOGN(F("wake up"CR));  
	os_runloop_once();
#else
	unsigned long starttime=millis();
	while((millis()-starttime) < (JOINRETRYDELAY*1000UL)){
	  wdt_reset();
	  mgr_serial();
	}
#endif    
      }
    }
  }
  
  
  
  // query and send data
#ifndef DEEPSLEEP
  
  Alarm.timerRepeat(configuration.sampletime, mgr_sensors);             // timer for every tr seconds
  // millis() and other can have overflow problem
  // so we reset everythings one time a week
  //Alarm.alarmRepeat(dowMonday,8,0,0,reboot);          // 8:00:00 every Monday

  // upgrade LMIC
  //Alarm.alarmRepeat(4,0,0,LMIC_tryRejoin);          // 4:00:00 every day  
#endif
    
  LOGN(F("End setup"CR));
  
}

void loop() {
  if (checkpowerdown()) powerdown();
  wdt_reset();
  mgr_serial();
  os_runloop_once();
#if defined(DEEPSLEEP)
  sleep_mgr_sensors();
#else
  Alarm.delay(0);
#endif
}
