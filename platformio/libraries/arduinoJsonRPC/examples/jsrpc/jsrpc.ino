// create a simple jsonrpc server with configuration saved on eeprom and watchdog

#include <avr/wdt.h>
#include "config.h"
#include <EEPROM.h>
#include "EEPROMAnything.h"

// include the JsonRPC library
#include <arduinoJsonRPC.h>

// initialize an instance of the JsonRPC library for registering 
// exactly 4 method
JsonRPC rpcserver(false ); //serial port with standard protocol

char confver[7] = CONFVER; // version of configuration saved on eeprom

struct config_t               // configuration to save and load fron eeprom
{
  int pd;
  void save () {
    int p=0;                  // save to eeprom
    p+=EEPROM_writeAnything(p, confver);
    p+=EEPROM_writeAnything(p, pd);
  }
  bool load () {                // load from eeprom
    int p=0;
    char ver[7];
    p+=EEPROM_readAnything(p, ver);
    if (strcmp(ver,confver ) == 0){ 
      p+=EEPROM_readAnything(p, pd);
      return true;
    }
    else{
      return false;
    }
  }
} configuration;

//-------------

const uint8_t outpins [] = {OUTPINS};
const uint8_t inpins  [] = {INPINS};

//-------------

// {"jsonrpc": "2.0","method":"pulse","params":{"pin":0},"id":0}	
int pulse(JsonObject& params, JsonObject& result)
{
  if (params.containsKey("pin")){
    int dstunit = params["pin"];

    if (dstunit >= 0 && dstunit < sizeof(outpins)/sizeof(*outpins)){
	Serial.print(F("#dstunit: "));
	Serial.println(dstunit);

	digitalWrite(outpins[dstunit], 1);
	delay(configuration.pd);
	digitalWrite(outpins[dstunit], 0);
	
    }else{
      Serial.println(F("#wrong pin"));
      return 2;
    }
  }else{
    Serial.println(F("#no pin"));
    return 3;
  }

  result["ok"]= true;  
  return 0;

}

// {"jsonrpc": "2.0","method":"getstatus","params":{"pin":0},"id":0}	
int getstatus(JsonObject& params, JsonObject& result)
{
  int val;
  
  if (params.containsKey("pin")){
    int dstunit = params["pin"];
    
    if (dstunit >= 0 && dstunit < sizeof(inpins)/sizeof(*inpins)){
	Serial.print(F("#dstunit: "));
	Serial.println(dstunit);

	val = digitalRead(inpins[dstunit]);
	
    }else{
      Serial.println(F("#wrong dstunit"));
      return 2;
    }
  }else{
    Serial.println(F("#no dstunit"));
    return 3;
  }

  if (val == HIGH) {
    result["status"]=false;
  }else{
    result["status"]=true;
  }
  
  return 0;
}


int setpd(JsonObject& params, JsonObject& result)
{    
  if (params.containsKey("pd")){
    int pd = params["pd"];

    configuration.pd=pd;
    result["ok"]= true;  
  }else{
    return  1;
  }

  return 0;
}

int save(JsonObject& params, JsonObject& result)
{    
  if (params.containsKey("eeprom")){
    bool eeprom = params["eeprom"];
    
    if (eeprom) configuration.save();
    result["ok"]= true;  
  }else{
    return 1;
  }

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

  wdt_enable(WDTO_8S);
  
  Serial.begin(115200);
  //while (!Serial); // wait for serial port to connect. Needed for native USB
  Serial.println(F("#Started"));

  if (configuration.load()){
    Serial.println(F("#Configuration loaded"));
  } else {
    Serial.println(F("#Configuration not loaded"));
    configuration.pd=PULSEDURATION;
  }
  Serial.print(F("#pd:"));
  Serial.println(configuration.pd);

  // register the local method
  // Serial port
  rpcserver.registerMethod("pulse",     &pulse);
  rpcserver.registerMethod("getstatus", &getstatus);
  rpcserver.registerMethod("setpd",     &setpd);
  rpcserver.registerMethod("save",      &save);
  
  // initialize the digital pin as an output
  //pinMode(13, OUTPUT);

  for (int dstunit=0 ;dstunit  < sizeof(outpins)/sizeof(*outpins); dstunit++)
    {
      pinMode(outpins[dstunit], OUTPUT);
      //digitalWrite(outpins[dstunit], 1);
    }

  for (int dstunit=0 ;dstunit  < sizeof(inpins)/sizeof(*inpins); dstunit++)
    {
      pinMode(inpins[dstunit], INPUT);
      //digitalWrite(outpins[dstunit], 1);
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


void loop()
{
  wdt_reset();
  mgr_serial();
}
