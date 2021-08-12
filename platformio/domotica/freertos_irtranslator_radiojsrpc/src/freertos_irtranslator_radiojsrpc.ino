/*
Copyright (C) 2020  Paolo Paruno <p.patruno@iperbole.bologna.it>
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

/*
 * Pin mapping table
 *
 * Platform  backpill & microduino smart RF

nome             blackpill     microduino smartrf
GND                 GND               GND
3.3V                3V3               3V3
IR input            PA2
IR output           PA3
SS1                 PA4               D10
SCLK1               PA5               D13
MISO1               PA6               D12
MOSI1               PA7               D11
GDO0                PB4               D2
Serial1             PA10(RX), PA9(TX)
PUTPIN1             PB5
PUTPIN2             PB6
PUTPIN3             PB7
PUTPIN4             PB8

as alternative defining JSSERIAL macro:
SerialUSB           USB
Serial              PA10 (RX), PA9(TX)

Debuging messages on Serial1
Serial json on  JSSERIAL
 */


#define CONFVER "conf00"
#define VERSION "20200814"

// radio bidirectional comunication
//#define TWOWAY "Yes"
//#define CLIENT "Yes"
//#define SERVER "Yes"

//#define JSSERIAL SerialUSB
#define JSSERIAL Serial

// freq added to standard channel
//#define FREQCORR 0.050
#define FREQCORR 0.0

// define the  pins used
#define PINS PB5,PB6,PB7,PB8

#define SERIALBUFFERSIZE 160
#define SERIALBAUDRATE 115200

#define FEEDBACK_LED_IS_ACTIVE_LOW // The LED on the BluePill is active LOW
#define IRMP_INPUT_PIN   PA2
#define IRSND_OUTPUT_PIN PA3
#define TONE_PIN         PA1
#define IRMP_TIMING_TEST_PIN PA0

#define IRMP_PROTOCOL_NAMES 0 // Enable protocol number mapping to protocol strings - requires some FLASH.

#define IRMP_SUPPORT_NEC_PROTOCOL               1       // NEC + APPLE + ONKYO  >= 10000                 ~300 bytes
#define IRMP_SUPPORT_SIRCS_PROTOCOL             0       // Sony SIRCS           >= 10000                 ~150 bytes
#define IRMP_SUPPORT_SAMSUNG_PROTOCOL           0       // Samsung + Samsg32    >= 10000                 ~300 bytes
#define IRMP_SUPPORT_KASEIKYO_PROTOCOL          0       // Kaseikyo             >= 10000                 ~250 bytes

#define IRSND_SUPPORT_NEC_PROTOCOL              1       // NEC + APPLE          >= 10000                 ~100 bytes
#define IRSND_SUPPORT_SIRCS_PROTOCOL            0       // Sony SIRCS           >= 10000                 ~200 bytes
#define IRSND_SUPPORT_SAMSUNG_PROTOCOL          0       // Samsung + Samsung32  >= 10000                 ~300 bytes
#define IRSND_SUPPORT_MATSUSHITA_PROTOCOL       0       // Matsushita           >= 10000                 ~200 bytes
#define IRSND_SUPPORT_KASEIKYO_PROTOCOL         0       // Kaseikyo             >= 10000                 ~300 bytes
#define IRSND_SUPPORT_RC5_PROTOCOL              0       // RC5                  >= 10000                 ~150 bytes
#define IRSND_SUPPORT_ROOMBA_PROTOCOL           0       // iRobot Roomba        >= 10000                 ~150 bytes

#define IRMP_USE_COMPLETE_CALLBACK       1 // Enable callback functionality
#define IRMP_ENABLE_PIN_CHANGE_INTERRUPT 1 // Enable interrupt functionality
#define USE_ONE_TIMER_FOR_IRMP_AND_IRSND   // otherwise we get an error: redefinition of 'void __vector_8()

#include <irmp.c.h>
#include <irsnd.c.h>

#include "STM32FreeRTOS.h"
//#include <USBSerial.h>
#include <IWatchdog.h>

#include "config.h"
#include <EEPROM.h>
#include "EEPROMAnything.h"

#include <SPI.h>
#include <RH_CC110.h>

// Singleton instance of the radio driver
RH_CC110 cc110(PA4,PB4);

// set RX and TX pins
//HardwareSerial Serial1(PA3, PA2);

#include "task.h"
#include "thread.hpp"
#include "ticks.hpp"
#include "queue.hpp"
#include "timer.hpp"
#include <frtosLog.h>

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
aJsonStream stream(&JSSERIAL);
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
  bool ld () {                // load from eeprom
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
uint8_t pinsstatus [sizeof(pins)];

//-------------



void printTimestamp(Print* _logOutput) {
  char c[12];
  sprintf(c, "%10lu ", millis());
  _logOutput->print(c);
}

void printNewline(Print* _logOutput) {
  _logOutput->print('\n');
}


void Reboot() {
  frtosLog.notice(F("#Reboot"));
  IWatchdog.begin(3000000);  while(1) {}
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
  
  frtosLog.notice(F("#send: "));
  frtosLog.notice(buf);
  
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
	frtosLog.notice(F("#got reply: "));
	frtosLog.notice((char*)buf);
	frtosLog.notice(F("#RSSI: %d"),cc110.lastRssi());

#endif  
	
	//IF_SDEBUG(DBGSERIAL.println("{\"jsonrpc\": \"2.0\", \"result\":true, \"id\": 0}"));	
	aJson.addTrueToObject(serialmsg, "result");

#ifdef TWOWAY
	
      } else {
      frtosLog.notice(F("#recv failed"));
      aJson.addFalseToObject(serialmsg, "result");
      status = 1;
    }
  } else {
    frtosLog.notice(F("#No reply, is cc110_server running?"));
    aJson.addFalseToObject(serialmsg, "result");
    status = 1;
  }
  
#endif  
  char serialbuf[SERIALBUFFERSIZE];

  aJson.print(serialmsg,serialbuf, sizeof(serialbuf));
  JSSERIAL.println(serialbuf);

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
    JSSERIAL.println(buf);
    
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

    IWatchdog.begin(30000000);  
    
    if (eeprom) configuration.save();

    IWatchdog.begin(8000000);  
    
    aJson.addTrueToObject(serialmsg, "result");
    char buf[SERIALBUFFERSIZE];
    aJson.print(serialmsg,buf, sizeof(buf));
    JSSERIAL.println(buf);
    
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
	unsigned int dstunit = dstunitParam -> valueint;

	if (dstunit >= 0 && dstunit < sizeof(pins)/sizeof(*pins)){
	  aJsonObject* onoffParam = aJson.getObjectItem(params, "o");
	  if (onoffParam){
	    boolean onoff = onoffParam -> valuebool;
	    frtosLog.notice(F("#did: %d "),did);
	    frtosLog.notice(F(" dstunit: %d"),dstunit);
	    frtosLog.notice(F(" onoff: %d"),onoff);

	    digitalWrite(pins[dstunit], ! onoff);
	    pinsstatus[dstunit]= ! onoff;
	    
	    aJson.deleteItemFromObject(radiomsg, "m");
	    aJson.deleteItemFromObject(radiomsg, "p");
	    aJson.addTrueToObject(radiomsg, "r");

	  }else{
	    frtosLog.notice(F("#no onoff"));
	  }
	}else{
	  frtosLog.notice(F("#wrong dstunit"));
	}
      }else{
	frtosLog.notice(F("#no dstunit"));
      }
    }else{
      frtosLog.notice(F("#not for me"));
    }
  }else{
    frtosLog.notice(F("#no did"));
  }
  //IF_SDEBUG(DBGSERIAL.println(F("{\"result\": \"OK\"}"));
  //aJson.deleteItem(params);
  return 0;
}
#endif


#ifdef CLIENT
void mgr_serial(){
  unsigned int err;
    
  if (stream.available()) {

    serialmsg = aJson.parse(&stream);
    if (serialmsg){
      frtosLog.notice(F("#rpc.processMessage:"));
      char serialbuf[SERIALBUFFERSIZE];
      aJson.print(serialmsg, serialbuf, sizeof(serialbuf));
      frtosLog.notice(serialbuf);
    
      err=rpcclient.processMessage(serialmsg);
      frtosLog.notice(F("#rpcclient.processMessage return status: %d"),err);
      if (!err){
	aJson.deleteItem(serialmsg);      
      }else{
	err = 1;
      }
      
    }else{
      frtosLog.notice(F("#skip wrong message"));
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
      JSSERIAL.println(serialbuf);
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
      frtosLog.notice(F("#got request: "));
      frtosLog.notice((char*)buf);
      frtosLog.notice(F("#RSSI: %d"),cc110.lastRssi());

      radiomsg = aJson.parse((char*)buf);

      if (radiomsg){
	err=rpcserver.processMessage(radiomsg);
	frtosLog.notice(F("#rpcserver.processMessage return status: %d"),err);
	if (!err) {
#ifdef TWOWAY

	  // Send a reply
	  // "{\"jsonrpc\": \"2.0\", \"result\":true, \"id\": 0}"
	  
	  aJson.print(radiomsg, (char*)buf, sizeof(buf));
	  frtosLog.notice(F("#Send: "));
	  frtosLog.notice((char*)buf);
	  cc110.send(buf, len);
	  cc110.waitPacketSent();
	  frtosLog.notice(F("#Sent a reply"));
#endif
	  
	  aJson.deleteItem(radiomsg);
	}else{
	  err = 1;
	  aJson.deleteItem(radiomsg);
	}
      }else{
	frtosLog.notice(F("#skip wrong message"));
	err = 2;
      }	
    } else {
      frtosLog.notice(F("recv failed"));
      err = 3;
    }
  }
}
#endif


using namespace cpp_freertos;

Queue *irQueue;
Queue *radioQueue;

enum source_t: int8_t { receiver, autocommand };

struct datamsg_t
{
  IRMP_DATA irdata;
  source_t source;
};

void handleReceivedIRData(){
  //IRMP_DATA irmp_data;
  datamsg_t datamsg;
  irmp_get_data(&datamsg.irdata);
  // enable interrupts
  //interrupts();
  //irmp_result_print(&datamsg.irdata);
  //if (! (data.irdata.flags & IRMP_FLAG_REPETITION)){
      // Its a new key  
  datamsg.source = receiver;
  irQueue->EnqueueFromISR (&datamsg,NULL);
  //}
}

class PeriodicTimerTvoff : public Timer {

    public:
        PeriodicTimerTvoff(int id, TickType_t PeriodInTicks) 
            : Timer(PeriodInTicks), Id(id)
        {
        };

    protected:

        virtual void Run() {
	  frtosLog.notice(F("Periodic timer %d"), Id);
	  if (pinsstatus[0] == 1){
	    datamsg_t datamsg;

	    datamsg.irdata.protocol = IRMP_NEC_PROTOCOL;
	    datamsg.irdata.address = 44880;
	    datamsg.irdata.command = 99;
	    datamsg.irdata.flags = 1; // repeat frame 1 time
	    datamsg.source=autocommand;
	    irQueue->EnqueueFromISR (&datamsg,NULL);
	  }	  
	};

    private:
        int Id;

};


class irThread : public Thread {
  
public:
  
  irThread(int i, Queue &q)
    : Thread("Thread Ir", 2000,1), 
      Id (i),
      irQueue(q)
  {
            Start();
  };
  
protected:

  virtual void Run() {
    
    frtosLog.notice("Starting Ir Thread %d", Id);

    irmp_init();
    irsnd_init();

    datamsg_t irmpmsg;
    IRMP_DATA irsnd_data;
 
    frtosLog.notice(F("Ready to receive IR signals at pin %d"), IRMP_INPUT_PIN);
    frtosLog.notice(F("Ready to send IR signals at pin %d"), IRSND_OUTPUT_PIN);

    irmp_register_complete_callback_function(&handleReceivedIRData);
 
    while (true){
      frtosLog.notice(F("IR wait for command"));
      irQueue.Dequeue(&irmpmsg);
      frtosLog.notice("Received protocol:%d address:%d command:%d flags:%d source:%d",irmpmsg.irdata.protocol,irmpmsg.irdata.address,irmpmsg.irdata.command,irmpmsg.irdata.flags,irmpmsg.source);

      if ((pinsstatus[0] == 0 && irmpmsg.source == receiver) || (pinsstatus[0] == 1 && irmpmsg.source ==  autocommand)){

	/*
	  irmp_data.protocol (8 Bit)
	  irmp_data.address (16 Bit)
	  irmp_data.command (16 Bit)
	  irmp_data.flags (8 Bit)
	*/

	irsnd_data.protocol = IRMP_NEC_PROTOCOL;
	irsnd_data.address = irmpmsg.irdata.address;
	irsnd_data.command = irmpmsg.irdata.command;
	irsnd_data.flags = 0; // repeat frame 1 time
	frtosLog.notice("Send %d %d %d %d",irmpmsg.irdata.protocol,irmpmsg.irdata.address,irmpmsg.irdata.command,irmpmsg.irdata.flags);
	irsnd_send_data(&irsnd_data, true); // true = wait for frame to end. This stores timer state and restores it after sending
      }
    }
  };

private:
  int Id;
  Queue &irQueue;
};


class radioThread : public Thread {
  
public:
  
  radioThread(int i, Queue &q)
    : Thread("Thread radio", 2000,1), 
      Id (i),
      radioQueue(q)
  {
            Start();
  };
  
protected:

  virtual void Run() {
    
    frtosLog.notice("Starting Radio Thread %d", Id);

    //JSSERIAL.begin(SERIALBAUDRATE);

    frtosLog.notice(F("#Started: " VERSION));
#ifdef TWOWAY
    JSSERIAL.println(F("#Twovay: " TWOWAY));
#endif
#ifdef CLIENT
    JSSERIAL.println(F("#Client: " CLIENT));
#endif
#ifdef SERVER
    JSSERIAL.println(F("#Server: " SERVER));
#endif
    
  if (configuration.ld()){
    frtosLog.notice(F("#Configuration loaded"));
    frtosLog.notice(F("#did: %d"),configuration.did);
  } else {     
    frtosLog.notice(F("#Configuration not loaded"));
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
    frtosLog.error(F("radio init failed"));
    frtosLog.error(F("reboot"));
    Reboot();
  }

  frtosLog.notice(F("radio init done"));

  // After init(), the following default values apply:
  // TxPower: TransmitPower5dBm
  // Frequency: 915.0
  // Modulation: GFSK_Rb1_2Fd5_2 (GFSK, Data Rate: 1.2kBaud, Dev: 5.2kHz, RX BW 58kHz, optimised for sensitivity)
  // Sync Words: 0xd3, 0x91
  // But you can change them:
  //  cc110.setTxPower(RH_CC110::TransmitPowerM30dBm);
  //  cc110.setModemConfig(RH_CC110::GFSK_Rb250Fd127);
  //cc110.setFrequency(928.0);


  cc110.setTxPower(RH_CC110::TransmitPower0dBm);
  //cc110.setModemConfig(RH_CC110::GFSK_Rb4_8Fd25_4);  // Giacomo
  cc110.setModemConfig(RH_CC110::GFSK_Rb100Fd47);    // Pat1
  
  cc110.setFrequency(434.0+FREQCORR);

 frtosLog.notice(F("radio setup done"));
 
  for (unsigned int dstunit=0 ;dstunit  < sizeof(pins)/sizeof(*pins); dstunit++)
    {
      pinMode(pins[dstunit], OUTPUT);
      digitalWrite(pins[dstunit], 1);
      pinsstatus[dstunit]=1;
    }

#ifdef CLIENT
  if (stream.available()) {
    // skip any accidental whitespace like newlines
    stream.skip();
  }
#endif
    
    while (true){

      IWatchdog.reload();
#ifdef CLIENT
      mgr_serial();
#endif
#ifdef SERVER
      IWatchdog.reload();
      mgr_radio();
#endif  
      
    }
  };

private:
  int Id;
  Queue &radioQueue;
};

void setup (void)
{

  static MutexStandard loggingmutex;

  IWatchdog.begin(8000000);  

  // start up the serial interface
  JSSERIAL.begin(SERIALBAUDRATE);

  //Start logging  
  Serial1.begin(SERIALBAUDRATE);
  frtosLog.begin(LOG_LEVEL_VERBOSE, &Serial1,loggingmutex);
  frtosLog.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
  frtosLog.setSuffix(printNewline); // Uncomment to get newline as suffix


  JSSERIAL.println (F("#0 Testing FreeRTOS IR & radio"));
  Serial1.println(F("#1 Testing FreeRTOS IR & radio"));
  
  irQueue = new Queue(10, sizeof(datamsg_t));
  radioQueue = new Queue(10, 4);
  static PeriodicTimerTvoff tvoff(1, Ticks::SecondsToTicks(10));
  tvoff.Start();
 
  static irThread p1(1,*irQueue);
  static radioThread p2(2,*radioQueue);
  
  Thread::StartScheduler();
  
}

void loop()
{
  // Empty. Things are done in Tasks.
}

