// create a simple messageing
// with the RH_CC110 class. RH_CC110 class does not provide for addressing or
// reliability, so you should only use RH_CC110 if you do not need the higher
// level messaging abilities.

#include <avr/wdt.h>

#define CONFVER "conf00"


#include "config.h"
#include <EEPROM.h>
#include "EEPROMAnything.h"

#include <avr/wdt.h>
#include <SPI.h>
#include <RH_CC110.h>

// Singleton instance of the radio driver
RH_CC110 cc110;

//  GFSK_Rb1_2Fd5_2 = 0,   ///< GFSK, Data Rate: 1.2kBaud, Dev: 5.2kHz, RX BW 58kHz, optimised for sensitivity
//  GFSK_Rb2_4Fd5_2,       ///< GFSK, Data Rate: 2.4kBaud, Dev: 5.2kHz, RX BW 58kHz, optimised for sensitivity
//  GFSK_Rb4_8Fd25_4,      ///< GFSK, Data Rate: 4.8kBaud, Dev: 25.4kHz, RX BW 100kHz, optimised for sensitivity
//  GFSK_Rb10Fd19,         ///< GFSK, Data Rate: 10kBaud, Dev: 19kHz, RX BW 100kHz, optimised for sensitivity
//  GFSK_Rb38_4Fd20,       ///< GFSK, Data Rate: 38.4kBaud, Dev: 20kHz, RX BW 100kHz, optimised for sensitivity
//  GFSK_Rb76_8Fd32,       ///< GFSK, Data Rate: 76.8kBaud, Dev: 32kHz, RX BW 232kHz, optimised for sensitivity
//  GFSK_Rb100Fd47,        ///< GFSK, Data Rate: 100kBaud, Dev: 47kHz, RX BW 325kHz, optimised for sensitivity
//  GFSK_Rb250Fd127,       ///< GFSK, Data Rate: 250kBaud, Dev: 127kHz, RX BW 540kHz, optimised for sensitivity


// include the aJSON library
#include <aJSON.h>

// include the JsonRPC library
#include <JsonRPC.h>

// initialize an instance of the JsonRPC library for registering 
// exactly 3 method


#ifdef CLIENT
JsonRPC rpcclient(5,false); //serial port
#endif
#ifdef SERVER
JsonRPC rpcserver(3,true ); //radio port with compact protocoll
#endif

#ifdef CLIENT
// initialize a serial json stream for receiving json objects
// through a serial/USB connection
aJsonStream stream(&Serial);
aJsonObject *serialmsg = NULL;
#endif

#ifdef SERVER
aJsonObject *radiomsg = NULL;
#endif

char confver[7] = CONFVER; // version of configuration saved on eeprom

struct config_t               // configuration to save and load fron eeprom
{
  int did;                     // sample time for mqtt  (seconds)
  void save () {
    int p=0;                  // save to eeprom
    p+=EEPROM_writeAnything(p, confver);
    p+=EEPROM_writeAnything(p, did);
  }
bool load () {                // load from eeprom
    int p=0;
    char ver[7];
    p+=EEPROM_readAnything(p, ver);
    if (strcmp(ver,confver ) == 0){ 
      p+=EEPROM_readAnything(p, did);
      return true;
    }
    else{
      return false;
    }
  }
} configuration;

//-------------

const uint8_t pins [] = {PINS};

//-------------

void Reboot() {
  IF_SDEBUG(DBGSERIAL.println(F("#Reboot")));
  wdt_enable(WDTO_30MS); while(1) {} 
}

#ifdef CLIENT
int client(aJsonObject* params)
{

  uint8_t status=0;
  aJsonObject *newrpc=NULL ;
  newrpc = aJson.createObject();
  //aJson.addStringToObject(newrpc, "m", method);

#ifdef TWOWAY
  aJsonObject* id = aJson.getObjectItem(serialmsg, "id");
  if(id){
    aJson.addNumberToObject(newrpc, "i",id -> valueint );
  }
#endif
  
  aJsonObject* mymethod = aJson.detachItemFromObject(serialmsg, "method");
  aJson.addItemToObject(newrpc, "m",mymethod );

  aJsonObject* myparams = aJson.detachItemFromObject(serialmsg, "params");
  aJson.addItemToObject(newrpc, "p",myparams );

  char buf[RH_CC110_MAX_MESSAGE_LEN];
  aJson.print(newrpc,buf, sizeof(buf));
  aJson.deleteItem(newrpc);
  
  IF_SDEBUG(DBGSERIAL.print(F("#send: ")));
  IF_SDEBUG(DBGSERIAL.println(buf));
  
  cc110.send((uint8_t*)buf, strlen(buf));
  cc110.waitPacketSent();

#ifdef TWOWAY
  
  // Now wait for a reply
  uint8_t len = sizeof(buf);

  if (cc110.waitAvailableTimeout(3000))
  { 
    // Should be a reply message for us now   
    if (cc110.recv((uint8_t*)buf, &len))
      {
	IF_SDEBUG(DBGSERIAL.print(F("#got reply: ")));
	IF_SDEBUG(DBGSERIAL.println((char*)buf));
	IF_SDEBUG(DBGSERIAL.print(F("#RSSI: ")));
	IF_SDEBUG(DBGSERIAL.println(cc110.lastRssi(), DEC));

#endif  
	
	//IF_SDEBUG(DBGSERIAL.println("{\"jsonrpc\": \"2.0\", \"result\":true, \"id\": 0}"));	
	aJson.addTrueToObject(serialmsg, "result");

#ifdef TWOWAY
	
      } else {
      IF_SDEBUG(DBGSERIAL.println(F("#recv failed")));
      aJson.addFalseToObject(serialmsg, "result");
      status = 1;
    }
  } else {
    IF_SDEBUG(DBGSERIAL.println(F("#No reply, is cc110_server running?")));
    aJson.addFalseToObject(serialmsg, "result");
    status = 1;
  }
  
#endif  
  char serialbuf[SERIALBUFFERSIZE];

  aJson.print(serialmsg,serialbuf, sizeof(serialbuf));
  Serial.println(serialbuf);

  return status;

}
#endif

#ifdef CLIENT
int setdid(aJsonObject* params)
{    
  uint8_t status=1; 
  aJson.deleteItemFromObject(serialmsg, "method");
  
  aJsonObject* myparams = aJson.detachItemFromObject(serialmsg, "params");
  aJsonObject* didParam = aJson.getObjectItem(myparams, "d");
  if (didParam){
    int did = didParam -> valueint;
    configuration.did=did;

    aJson.addTrueToObject(serialmsg, "result");
    char buf[SERIALBUFFERSIZE];
    aJson.print(serialmsg,buf, sizeof(buf));
    Serial.println(buf);
    
    status= 0;
  }
  aJson.deleteItem(params);
  return status;
}

int save(aJsonObject* params)
{    
  uint8_t status=1; 
  aJson.deleteItemFromObject(serialmsg, "method");

  aJsonObject* myparams = aJson.detachItemFromObject(serialmsg, "params");
  
  aJsonObject* saveParam = aJson.getObjectItem(myparams, "eeprom");
  if (saveParam){
    bool eeprom = saveParam -> valuebool;
    
    if (eeprom) configuration.save();
    
    aJson.addTrueToObject(serialmsg, "result");
    char buf[SERIALBUFFERSIZE];
    aJson.print(serialmsg,buf, sizeof(buf));
    Serial.println(buf);
    
    status = 0;
  }

  aJson.deleteItem(params);
  return status;

}
#endif
#ifdef SERVER
int changedidserver(aJsonObject* params)
{    
  aJsonObject* olddidParam = aJson.getObjectItem(params, "olddid");
  if (olddidParam){
    int olddid = olddidParam -> valueint;
    if (olddid == configuration.did || olddid == 0 ){

      aJsonObject* didParam = aJson.getObjectItem(params, "d");
      if (didParam){
	int did = didParam -> valueint;
	configuration.did=did;

	aJson.deleteItemFromObject(radiomsg, "m");
	aJson.deleteItemFromObject(radiomsg, "p");
	aJson.addTrueToObject(radiomsg, "r");

      }
    }
  }
  //aJson.deleteItem(params);
  return 0;
}

int saveserver(aJsonObject* params)
{

  aJsonObject* didParam = aJson.getObjectItem(params, "d");
  if (didParam){
    int did = didParam -> valueint;
    if (did == configuration.did || did == 0 ){     //my did or broadcast
  
      aJsonObject* saveParam = aJson.getObjectItem(params, "eeprom");
      if (saveParam){
	boolean eeprom = saveParam -> valuebool;
	
	if (eeprom) configuration.save();
	aJson.deleteItemFromObject(radiomsg, "m");
	aJson.deleteItemFromObject(radiomsg, "p");
	aJson.addTrueToObject(radiomsg, "r");

      }
    }
  }
  //aJson.deleteItem(params);
  return 0;
}

int singleserver(aJsonObject* params)
{
  //{"jsonrpc":"2.0","method":"single","params":{"d":1,"u":1,"o":true},"id":0}
  
  aJsonObject* didParam = aJson.getObjectItem(params, "d");
  if (didParam){
    int did = didParam -> valueint;
    if (did == configuration.did || did == 0 ){     //my did or broadcast
    
      aJsonObject* dstunitParam = aJson.getObjectItem(params, "u");
      if (dstunitParam){
	int dstunit = dstunitParam -> valueint;

	if (dstunit >= 0 && dstunit < sizeof(pins)/sizeof(*pins)){
	  aJsonObject* onoffParam = aJson.getObjectItem(params, "o");
	  if (onoffParam){
	    boolean onoff = onoffParam -> valuebool;
	    IF_SDEBUG(DBGSERIAL.print(F("#did: ")));
	    IF_SDEBUG(DBGSERIAL.print(did));
	    IF_SDEBUG(DBGSERIAL.print(F(" dstunit: ")));
	    IF_SDEBUG(DBGSERIAL.print(dstunit));
	    IF_SDEBUG(DBGSERIAL.print(F(" onoff: ")));
	    IF_SDEBUG(DBGSERIAL.println(onoff));

	    digitalWrite(pins[dstunit], ! onoff);

	    aJson.deleteItemFromObject(radiomsg, "m");
	    aJson.deleteItemFromObject(radiomsg, "p");
	    aJson.addTrueToObject(radiomsg, "r");

	  }else{
	    IF_SDEBUG(DBGSERIAL.println(F("#no onoff")));
	  }
	}else{
	  IF_SDEBUG(DBGSERIAL.println(F("#wrong dstunit")));
	}
      }else{
	IF_SDEBUG(DBGSERIAL.println(F("#no dstunit")));
      }
    }else{
      IF_SDEBUG(DBGSERIAL.println(F("#not for me")));
    }
  }else{
    IF_SDEBUG(DBGSERIAL.println(F("#no did")));
  }
  //IF_SDEBUG(DBGSERIAL.println(F("{\"result\": \"OK\"}"));
  //aJson.deleteItem(params);
  return 0;
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

  wdt_enable(WDTO_8S);
  
  IF_SDEBUG(DBGSERIAL.begin(SERIALBAUDRATE));
  Serial.begin(SERIALBAUDRATE);
  while (!Serial); // wait for serial port to connect. Needed for native USB
  Serial.println(F("#Started: "VERSION));
#ifdef TWOWAY
  Serial.println(F("#Twovay: "TWOWAY));
#endif
#ifdef CLIENT
  Serial.println(F("#Client: "CLIENT));
#endif
#ifdef SERVER
  Serial.println(F("#Server: "SERVER));
#endif

  if (configuration.load()){
    IF_SDEBUG(DBGSERIAL.println(F("#Configuration loaded")));
    IF_SDEBUG(DBGSERIAL.print(F("#did:")));
    IF_SDEBUG(DBGSERIAL.println(configuration.did));
  } else {     
    IF_SDEBUG(DBGSERIAL.println(F("#Configuration not loaded")));
  }

  // register the local single method
#ifdef SERVER
  // Radio port
  rpcserver.registerMethod("single",    &singleserver);
  rpcserver.registerMethod("changedid", &changedidserver);
  rpcserver.registerMethod("remotesave",&saveserver);
#endif

#ifdef CLIENT  
  // Serial port
  rpcclient.registerMethod("single",    &client);
  rpcclient.registerMethod("changedid", &client);
  rpcclient.registerMethod("remotesave",&client);
  rpcclient.registerMethod("setdid",    &setdid);
  rpcclient.registerMethod("save",      &save);
#endif
  
  // CC110L may be equipped with either 26 or 27MHz crystals. You MUST
  // tell the driver if a 27MHz crystal is installed for the correct configuration to
  // occur. Failure to correctly set this flag will cause incorrect frequency and modulation
  // characteristics to be used. You can call this function, or pass it to the constructor
  //cc110.setIs27MHz(true); // Anaren 430BOOST-CC110L Air BoosterPack test boards have 27MHz

  if (!cc110.init()){
    IF_SDEBUG(DBGSERIAL.println(F("init failed")));
    Reboot();
  }
  // After init(), the following default values apply:
  // TxPower: TransmitPower5dBm
  // Frequency: 915.0
  // Modulation: GFSK_Rb1_2Fd5_2 (GFSK, Data Rate: 1.2kBaud, Dev: 5.2kHz, RX BW 58kHz, optimised for sensitivity)
  // Sync Words: 0xd3, 0x91
  // But you can change them:
  //  cc110.setTxPower(RH_CC110::TransmitPowerM30dBm);
  //  cc110.setModemConfig(RH_CC110::GFSK_Rb250Fd127);
  //cc110.setFrequency(928.0);

/*
Canale 	Frequenza (MHz) Canale 	Frequenza (MHz)	Canale 	Frequenza (MHz)
1 	433.075 	24 	433.650 	47 	434.225
2 	433.100 	25 	433.675 	48 	434.250
3 	433.125 	26 	433.700 	49 	434.275
4 	433.150 	27 	433.725 	50 	434.300
5 	433.175 	28 	433.750 	51 	434.325
6 	433.200 	29 	433.775 	52 	434.350
7 	433.225 	30 	433.800 	53 	434.375
8 	433.250 	31 	433.825 	54 	434.400
9 	433.275 	32 	433.850 	55 	434.425
10 	433.300 	33 	433.875 	56 	434.450
11 	433.325 	34 	433.900 	57 	434.475
12 	433.350 	35 	433.925 	58 	434.500
13 	433.375 	36 	433.950 	59 	434.525
14 	433.400 	37 	433.975 	60 	434.550
15 	433.425 	38 	434.000 	61 	434.575
16 	433.450 	39 	434.025 	62 	434.600
17 	433.475 	40 	434.050 	63 	434.625
18 	433.500 	41 	434.075 	64 	434.650
19 	433.525 	42 	434.100 	65 	434.675
20 	433.550 	43 	434.125 	66 	434.700
21 	433.575 	44 	434.150 	67 	434.725
22 	433.600 	45 	434.175 	68 	434.750
23 	433.625 	46 	434.200 	69 	434.775
*/

  cc110.setTxPower(RH_CC110::TransmitPower0dBm);
  //cc110.setModemConfig(RH_CC110::GFSK_Rb4_8Fd25_4);  // Giacomo
  cc110.setModemConfig(RH_CC110::GFSK_Rb100Fd47);    // Pat1

  // For 26MHz crystals
  //PROGMEM static const RH_CC110::ModemConfig GFSK_R1_2Fd25_4 =
  static const RH_CC110::ModemConfig GFSK_R1_2Fd25_4 =
  // 0B    0C    10    11    12    15    19    1A    1B    1C    1D    21    22    23    24    25    26    2C    2D    2E
  {0x06, 0x00, 0xC5, 0x83, 0x13, 0x40, 0x16, 0x6c, 0x43, 0x40, 0x91, 0x56, 0x10, 0xe9, 0x2a, 0x00, 0x1f, 0x81, 0x35, 0x09}; // GFSK_R1_2Fd47 GFSK, Data Rate: 1.2kBaud, Dev: 47kHz, RX BW 325kHz, optimised for sensitivity
  //{0x06, 0x00, 0xc7, 0x83, 0x13, 0x40, 0x16, 0x6c, 0x43, 0x40, 0x91, 0x56, 0x10, 0xe9, 0x2a, 0x00, 0x1f, 0x81, 0x35, 0x09}; // GFSK_Rb4_8Fd25_4

  //cc110.setModemRegisters(&GFSK_R1_2Fd25_4);
  
  cc110.setFrequency(434.0+FREQCORR);


  for (int dstunit=0 ;dstunit  < sizeof(pins)/sizeof(*pins); dstunit++)
    {
      pinMode(pins[dstunit], OUTPUT);
      digitalWrite(pins[dstunit], 1);
    }

#ifdef CLIENT
  if (stream.available()) {
    // skip any accidental whitespace like newlines
    stream.skip();
  }
#endif
}

#ifdef CLIENT
void mgr_serial(){
  unsigned int err;
    
  if (stream.available()) {

    serialmsg = aJson.parse(&stream);
    if (serialmsg){
      IF_SDEBUG(DBGSERIAL.print(F("#rpc.processMessage:")));
      char serialbuf[SERIALBUFFERSIZE];
      aJson.print(serialmsg, serialbuf, sizeof(serialbuf));
      IF_SDEBUG(DBGSERIAL.println(serialbuf));
    
      err=rpcclient.processMessage(serialmsg);
      IF_SDEBUG(DBGSERIAL.print(F("#rpcclient.processMessage return status:")));
      IF_SDEBUG(DBGSERIAL.println(err));
      if (!err){
	aJson.deleteItem(serialmsg);      
      }else{
	err = 1;
      }
      
    }else{
      IF_SDEBUG(DBGSERIAL.println(F("#skip wrong message")));
      err = 2;
      if (stream.available()) {
	stream.flush();
      }
    }

    if (err == 1){
      aJsonObject *result = aJson.createObject();
      aJson.addItemToObject(serialmsg, "error", result);
      aJson.addNumberToObject(result, "code", E_INTERNAL_ERROR);
      aJson.addStringToObject(result,"message", strerror(E_INTERNAL_ERROR));   
      
      /*
      if (!rpcid || !msg){
	IF_SDEBUG(IF_SDEBUG(DBGSERIAL.println(F("#add null id in response"))));
	aJson.addNullToObject(serialmsg, "id");
      } else {
	IF_SDEBUG(IF_SDEBUG(DBGSERIAL.println(F("#add id in response"))));
        aJson.addNumberToObject(serialmsg, "id", rpcid->valueint);
      }
      */

      char serialbuf[SERIALBUFFERSIZE];

      aJson.print(serialmsg,serialbuf, sizeof(serialbuf));
      Serial.println(serialbuf);
      aJson.deleteItem(serialmsg);

    }
  }
}
#endif

#ifdef SERVER
void mgr_radio(){
  unsigned int err;
  if (cc110.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_CC110_MAX_MESSAGE_LEN+1];
    uint8_t len = RH_CC110_MAX_MESSAGE_LEN;
    if (cc110.recv(buf, &len))
    {
      buf[len]=NULL; // terminate the string
      //RH_CC110::printBuffer("#request: ", buf, len);
      IF_SDEBUG(DBGSERIAL.print(F("#got request: ")));
      IF_SDEBUG(DBGSERIAL.println((char*)buf));
      IF_SDEBUG(DBGSERIAL.print(F("#RSSI: ")));
      IF_SDEBUG(DBGSERIAL.println(cc110.lastRssi(), DEC));

      radiomsg = aJson.parse((char*)buf);

      if (radiomsg){
	err=rpcserver.processMessage(radiomsg);
	IF_SDEBUG(DBGSERIAL.print(F("#rpcserver.processMessage return status:")));
	IF_SDEBUG(DBGSERIAL.println(err));
	     if (!err) {
#ifdef TWOWAY

	  // Send a reply
	  // "{\"jsonrpc\": \"2.0\", \"result\":true, \"id\": 0}"
	  
	  aJson.print(radiomsg, (char*)buf, sizeof(buf));
	  IF_SDEBUG(DBGSERIAL.print(F("#Send: ")));
	  IF_SDEBUG(DBGSERIAL.println((char*)buf));
	  cc110.send(buf, len);
	  cc110.waitPacketSent();
	  IF_SDEBUG(DBGSERIAL.println(F("#Sent a reply")));
#endif
	  
	  aJson.deleteItem(radiomsg);
	}else{
	  err = 1;
	  aJson.deleteItem(radiomsg);
	}
      }else{
	IF_SDEBUG(DBGSERIAL.println(F("#skip wrong message")));
	err = 2;
      }	
    } else {
      IF_SDEBUG(DBGSERIAL.println(F("recv failed")));
      err = 3;
    }
  }
}
#endif

void loop()
{
  wdt_reset();
#ifdef CLIENT
  mgr_serial();
#endif
#ifdef SERVER
  wdt_reset();
  mgr_radio();
#endif  
}
