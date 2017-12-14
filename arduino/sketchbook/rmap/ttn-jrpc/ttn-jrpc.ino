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

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <avr/wdt.h>
#include "config.h"
#include <EEPROM.h>
#include "EEPROMAnything.h"
// include the JsonRPC library
#include <arduinoJsonRPC.h>

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


void os_getArtEui (u1_t* buf) { memcpy_P(buf, configuration.appeui, 8);}
void os_getDevEui (u1_t* buf) { memcpy_P(buf, configuration.deveui, 8);}
void os_getDevKey (u1_t* buf) { memcpy_P(buf, configuration.appkey, 16);}

//static uint8_t mydata[] = "Hello, world!";
//static osjob_t sendjob;


int send(JsonObject& params, JsonObject& result)
{
  const char* mydata=params["payload"];
  if (mydata == NULL ){
    Serial.println(F("#no payload present"));
    return 1;
  }
 
  uint8_t payload[51];
  memcpy(payload, mydata, 51*sizeof(uint8_t));
  do_send(payload);
  result["ok"]= true;  
  return 0;
}


int set(JsonObject& params, JsonObject& result)
{    
  int ack = params["ack"];
  if (!(ack == NULL)){
    configuration.ack=ack;
    //}else{
    //return  1;
  }

  JsonArray& arrayappeui = params["appeui"];
  for(JsonArray::iterator it=arrayappeui.begin(); it!=arrayappeui.end(); ++it) {
    int i=0;
    // *it contains the JsonVariant which can be casted as usuals
    configuration.appeui[i] = it->as<uint8_t>();    
    //}else{
    //return  2;
  }
  JsonArray& arraydeveui = params["deveui"];
  for(JsonArray::iterator it=arraydeveui.begin(); it!=arraydeveui.end(); ++it) {
    int i=0;
    // *it contains the JsonVariant which can be casted as usuals
    configuration.deveui[i] = it->as<uint8_t>();    
    //}else{
    //return  3;
  }
  JsonArray& arrayappkey = params["appkey"];
  for(JsonArray::iterator it=arrayappkey.begin(); it!=arrayappkey.end(); ++it) {
    int i=0;
    // *it contains the JsonVariant which can be casted as usuals
    configuration.appkey[i] = it->as<uint8_t>();    
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
    }else{
      return 1;
    }
    return 0;
  }
}


void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));

            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void do_send(uint8_t mydata[]){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        Serial.println(F("Packet queued"));
    }
}

void mgr_serial(){

  StaticJsonBuffer<1000> jsonBuffer;
  if (Serial.available()) {
    JsonObject& msg = jsonBuffer.parse(Serial);
      if (msg.success()){
	int err=rpcserver.processMessage(msg);
	Serial.print(F("#rpc processMessage return status:"));
	Serial.println(err);
	msg.printTo(Serial);
    }else{
      Serial.println("#error decoding msg");      
    }
  }
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
  Serial.println(F("#Started"));
  
  if (configuration.load()){
    Serial.println(F("#Configuration loaded"));
  } else {
    Serial.println(F("#Configuration not loaded"));
    configuration.ack=0;
  }
  Serial.print(F("#ack:"));
  Serial.println(configuration.ack);
  
  // register the local method
  rpcserver.registerMethod("send",      &send);
  rpcserver.registerMethod("set",       &set);
  rpcserver.registerMethod("save",      &save);
  
  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

}

void loop() {
  wdt_reset();
  mgr_serial();
  os_runloop_once();
}
