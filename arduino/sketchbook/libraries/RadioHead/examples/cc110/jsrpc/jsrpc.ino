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

// include the aJSON library
#include <aJSON.h>

// include the JsonRPC library
#include <JsonRPC.h>

// initialize an instance of the JsonRPC library for registering 
// exactly 3 method

JsonRPC rpcclient(5,false); //serial port
JsonRPC rpcserver(3,true ); //radio port with compact protocoll

// initialize a serial json stream for receiving json objects
// through a serial/USB connection
aJsonStream stream(&Serial);
aJsonObject *serialmsg = NULL;
aJsonObject *radiomsg = NULL;

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
  Serial.println(F("#Reboot"));
  wdt_enable(WDTO_30MS); while(1) {} 
}


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
  
  Serial.print(F("#send: "));
  Serial.println(buf);
  
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
	Serial.print(F("#got reply: "));
	Serial.println((char*)buf);
	Serial.print(F("#RSSI: "));
	Serial.println(cc110.lastRssi(), DEC);    

#endif  
	
	//Serial.println("{\"jsonrpc\": \"2.0\", \"result\":true, \"id\": 0}");	
	aJson.addTrueToObject(serialmsg, "result");

#ifdef TWOWAY
	
      } else {
      Serial.println(F("#recv failed"));
      aJson.addFalseToObject(serialmsg, "result");
      status = 1;
    }
  } else {
    Serial.println(F("#No reply, is cc110_server running?"));
    aJson.addFalseToObject(serialmsg, "result");
    status = 1;
  }
  
#endif  
  char serialbuf[SERIALBUFFERSIZE];

  aJson.print(serialmsg,serialbuf, sizeof(serialbuf));
  Serial.println(serialbuf);

  return status;

}


int setdid(aJsonObject* params)
{    
  uint8_t status=1; 
  aJson.deleteItemFromObject(serialmsg, "method");
  
  aJsonObject* myparams = aJson.detachItemFromObject(serialmsg, "params");
  aJsonObject* didParam = aJson.getObjectItem(myparams, "did");
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


int changedidserver(aJsonObject* params)
{    
  aJsonObject* olddidParam = aJson.getObjectItem(params, "olddid");
  if (olddidParam){
    int olddid = olddidParam -> valueint;
    if (olddid == configuration.did || olddid == 0 ){

      aJsonObject* didParam = aJson.getObjectItem(params, "did");
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

  aJsonObject* didParam = aJson.getObjectItem(params, "did");
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
  //{"jsonrpc":"2.0","method":"single","params":{"did":1,"dstunit":1,"onoff":true},"id":0}
  
  aJsonObject* didParam = aJson.getObjectItem(params, "did");
  if (didParam){
    int did = didParam -> valueint;
    if (did == configuration.did || did == 0 ){     //my did or broadcast
    
      aJsonObject* dstunitParam = aJson.getObjectItem(params, "dstunit");
      if (dstunitParam){
	int dstunit = dstunitParam -> valueint;

	if (dstunit >= 0 && dstunit < sizeof(pins)/sizeof(*pins)){
	  aJsonObject* onoffParam = aJson.getObjectItem(params, "onoff");
	  if (onoffParam){
	    boolean onoff = onoffParam -> valuebool;
	    Serial.print(F("#did: "));
	    Serial.print(did);
	    Serial.print(F(" dstunit: "));
	    Serial.print(dstunit);
	    Serial.print(F(" onoff: "));
	    Serial.println(onoff);

	    digitalWrite(pins[dstunit], ! onoff);

	    aJson.deleteItemFromObject(radiomsg, "m");
	    aJson.deleteItemFromObject(radiomsg, "p");
	    aJson.addTrueToObject(radiomsg, "r");

	  }else{
	    Serial.println(F("#no onoff"));
	  }
	}else{
	  Serial.println(F("#wrong dstunit"));
	}
      }else{
	Serial.println(F("#no dstunit"));
      }
    }else{
      Serial.println(F("#not for me"));
    }
  }else{
    Serial.println(F("#no did"));
  }
  //Serial.println(F("{\"result\": \"OK\"}"));
  //aJson.deleteItem(params);
  return 0;
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
  
  Serial.begin(115200);
  while (!Serial); // wait for serial port to connect. Needed for native USB
  Serial.println(F("#Started"));

  if (configuration.load()){
    Serial.println(F("#Configuration loaded"));
    Serial.print(F("#did:"));
    Serial.println(configuration.did);
  } else {     
    Serial.println(F("#Configuration not loaded"));
  }

  // register the local single method
  // Radio port
  rpcserver.registerMethod("single",    &singleserver);
  rpcserver.registerMethod("changedid", &changedidserver);
  rpcserver.registerMethod("remotesave",&saveserver);
  
  // Serial port
  rpcclient.registerMethod("single",    &client);
  rpcclient.registerMethod("changedid", &client);
  rpcclient.registerMethod("remotesave",&client);
  rpcclient.registerMethod("setdid",    &setdid);
  rpcclient.registerMethod("save",      &save);
  
  // CC110L may be equipped with either 26 or 27MHz crystals. You MUST
  // tell the driver if a 27MHz crystal is installed for the correct configuration to
  // occur. Failure to correctly set this flag will cause incorrect frequency and modulation
  // characteristics to be used. You can call this function, or pass it to the constructor
  //cc110.setIs27MHz(true); // Anaren 430BOOST-CC110L Air BoosterPack test boards have 27MHz

  if (!cc110.init()){
    Serial.println(F("init failed"));
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

  cc110.setTxPower(RH_CC110::TransmitPower10dBm);
  cc110.setModemConfig(RH_CC110::GFSK_Rb1_2Fd5_2);
  cc110.setFrequency(433.300);

  // initialize the digital pin as an output
  pinMode(13, OUTPUT);



  for (int dstunit=0 ;dstunit  < sizeof(pins)/sizeof(*pins); dstunit++)
    {
      pinMode(pins[dstunit], OUTPUT);
      digitalWrite(pins[dstunit], 1);
    }
}

void mgr_serial(){
  uint8_t err;
    
  if (stream.available()) {
    // skip any accidental whitespace like newlines
    stream.skip();
  }

  if (stream.available()) {

    serialmsg = aJson.parse(&stream);
    if (serialmsg){
      Serial.print(F("#rpc.processMessage:"));
      char serialbuf[SERIALBUFFERSIZE];
      aJson.print(serialmsg, serialbuf, sizeof(serialbuf));
      Serial.println(serialbuf);
    
      err=rpcclient.processMessage(serialmsg);
      Serial.print(F("#rpcserver.processMessage return status:"));
      Serial.print(err);
      if (!err){
	aJson.deleteItem(serialmsg);      
      }else{
	err = 1;
      }
      
    }else{
      Serial.println(F("#skip wrong message"));	
      err = 2;
    }
    if (stream.available()) {
      stream.flush();
    }

    if (err == 1){
      aJsonObject *result = aJson.createObject();
      aJson.addItemToObject(serialmsg, "error", result);
      aJson.addNumberToObject(result, "code", E_INTERNAL_ERROR);
      aJson.addStringToObject(result,"message", strerror(E_INTERNAL_ERROR));   
      
      /*
      if (!rpcid || !msg){
	IF_SDEBUG(DBGSERIAL.println(F("#add null id in response")));
	aJson.addNullToObject(serialmsg, "id");
      } else {
	IF_SDEBUG(DBGSERIAL.println(F("#add id in response")));
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


void mgr_radio(){
  uint8_t err;
  if (cc110.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_CC110_MAX_MESSAGE_LEN+1];
    uint8_t len = RH_CC110_MAX_MESSAGE_LEN;
    if (cc110.recv(buf, &len))
    {
      buf[len]=NULL; // terminate the string
      //RH_CC110::printBuffer("#request: ", buf, len);
      Serial.print(F("#got request: "));
      Serial.println((char*)buf);
//      Serial.print(F("RSSI: "));
//      Serial.println(cc110.lastRssi(), DEC);

      radiomsg = aJson.parse((char*)buf);

      if (radiomsg){
	err=rpcserver.processMessage(radiomsg);
	Serial.print(F("#rpcserver.processMessage return status:"));
	Serial.print(err);
	     if (!err) {
#ifdef TWOWAY

	  // Send a reply
	  // "{\"jsonrpc\": \"2.0\", \"result\":true, \"id\": 0}"
	  
	  aJson.print(radiomsg, (char*)buf, sizeof(buf));
	  Serial.print(F("#Send: "));
	  Serial.println((char*)buf);
	  cc110.send(buf, len);
	  cc110.waitPacketSent();
	  Serial.println(F("#Sent a reply"));
#endif
	  
	  aJson.deleteItem(radiomsg);
	}else{
	  err = 1;
	}
      }else{
	Serial.println(F("#skip wrong message"));	
	err = 2;
      }	
    } else {
      Serial.println(F("recv failed"));
      err = 3;
    }

    if (err == 1){
      aJsonObject *result = aJson.createObject();
      aJson.addItemToObject(radiomsg, "error", result);
      aJson.addNumberToObject(result, "code", E_INTERNAL_ERROR);
      aJson.addStringToObject(result,"message", strerror(E_INTERNAL_ERROR));   

      /*
      if (!rpcid || !msg){
	IF_SDEBUG(DBGSERIAL.println(F("#add null id in response")));
	aJson.addNullToObject(serialmsg, "id");
      } else {
	IF_SDEBUG(DBGSERIAL.println(F("#add id in response")));
        aJson.addNumberToObject(serialmsg, "id", rpcid->valueint);
      }
      */

      aJson.print(radiomsg, (char*)buf, sizeof(buf));
      Serial.print(F("#Send: "));
      Serial.println((char*)buf);
      cc110.send(buf, len);
      cc110.waitPacketSent();
      Serial.println(F("#Sent a errorreply"));
      aJson.deleteItem(radiomsg);

    }
  }
}

void loop()
{
  wdt_reset();
  mgr_serial();
  wdt_reset();
  mgr_radio(); 
}
