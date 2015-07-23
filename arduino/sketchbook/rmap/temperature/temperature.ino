/*
  JsonRPC rmap application
 
 Demonstrates how to toggle the status of a LED by sending a
 json message to the arduino over the serial connection.
 
 The following python script can be used to toggle the LED
 after the sketch has been uploaded to the arduino:
 
 import sys
 import serial
 import time
 
 port = '/dev/ttyACM0'
 ser = serial.Serial(port, 9600)
 
 # give the serial connection 2 seconds to settle
 time.sleep(2)
 
 ser.write('{"jsonrpc":"2.0","method": "toggleled", "params": {"status": true}, "id": 0 }')
 time.sleep(5)
 ser.write('{"jsonrpc":"2.0","method": "toggleled", "params": {"status": false} }', "id": 1 )
 
 # wait 2 seconds before closing the serial connection
 time.sleep(2)
 ser.close()
 
 */

// include the aJSON library
#include <aJSON.h>
// include the JsonRPC library
#include <JsonRPC.h>
// include the JsonRPC library
#include <Wire.h>

// driver for sensor
#include <SensorDriver.h>

SensorDriverTmp tmp275_102;
SensorDriverAdt7420 adt7420;

// initialize an instance of the JsonRPC library for registering 
// exactly 1 local method
JsonRPC rpc(3);
// initialize a serial json stream for receiving json objects
// through a serial/USB connection
aJsonStream stream(&Serial);

// on most arduino boards, pin 13 is connected to a LED
int led = 13;

aJsonObject *result;

int toggleLED(aJsonObject* params)
{
  aJsonObject* statusParam = aJson.getObjectItem(params, "status");
  boolean requestedStatus = statusParam -> valuebool;

  if ( requestedStatus)
  {
    digitalWrite(led, HIGH);
  }
  else
  {
    digitalWrite(led, LOW);
  }

  aJson.addNumberToObject(result, "value", requestedStatus);
  aJson.addStringToObject(result, "description", "Led status");

  return E_SUCCESS;  
}

// driver for TMP275 and TMP102
int gettmp275_102values(aJsonObject* params)
{
  aJsonObject* addressParam = aJson.getObjectItem(params, "address");
  int requestedAddress = addressParam -> valueint;

  if (!tmp275_102.setup(requestedAddress) == SD_SUCCESS){
    return E_INTERNAL_ERROR;
  }
  if (!tmp275_102.prepare() == SD_SUCCESS){
    return E_INTERNAL_ERROR;
  }
  delay(500);
  int temperature;
  if (!tmp275_102.get(&temperature) == SD_SUCCESS){
    return E_INTERNAL_ERROR;
  }
  
  aJson.addNumberToObject(result, "B12101", temperature );
  //  aJson.addStringToObject(result, "description", "Temperature K");

  return E_SUCCESS;

}

// driver for ADT7420
int getadt7420values(aJsonObject* params)
{
  aJsonObject* addressParam = aJson.getObjectItem(params, "address");
  int requestedAddress = addressParam -> valueint;

  if (!adt7420.setup(requestedAddress) == SD_SUCCESS){
    return E_INTERNAL_ERROR;
  }
  if (!adt7420.prepare() == SD_SUCCESS){
    return E_INTERNAL_ERROR;
  }
  delay(250);
  int temperature;
  if (!adt7420.get(&temperature) == SD_SUCCESS){
    return E_INTERNAL_ERROR;
  }
  
  aJson.addNumberToObject(result, "B12101", temperature );
  //  aJson.addStringToObject(result, "description", "Temperature K");

  return E_SUCCESS;

}


void setup()
{
  // initialize the digital pin as an output
  pinMode(led, OUTPUT);

  // start up the serial interface
  Serial.begin(9600);

  Wire.begin();
  
  // and register the local toggleLED method
  rpc.registerMethod("toggleled", &toggleLED);
  rpc.registerMethod("gettmpvalues", &gettmp275_102values);
  rpc.registerMethod("getadt7420values", &getadt7420values);
}

void loop()
{
  int err;
  aJsonObject *response,*error;

  if (stream.available()) {
    // skip any accidental whitespace like newlines
    stream.skip();
  }

  if (stream.available()) {
    aJsonObject *msg = aJson.parse(&stream);
    response=aJson.createObject();
    result = aJson.createObject();

    aJsonObject* id = aJson.getObjectItem(msg, "id");
    aJsonObject* jsonrpc = aJson.getObjectItem(msg, "jsonrpc");

    err=rpc.processMessage(msg);
    if (!id) {
      // NOTIFICATION !
      err=E_INTERNAL_ERROR;
    }

    if (strcmp (jsonrpc->valuestring,"2.0" ) != 0) {
      err=E_INTERNAL_ERROR;
    }

    if (err != E_SUCCESS)
    {
      if (!jsonrpc)
      {
        aJson.addStringToObject(response, "jsonrpc","2.0");
      }
      else
      {
        aJson.addStringToObject(response, "jsonrpc",jsonrpc->valuestring);
      }
      
      aJson.addItemToObject(response, "error", error = aJson.createObject());
      aJson.addNumberToObject(error, "code", err);
      aJson.addStringToObject(error,"message", strerror(err));   
      if (!id)
      {
        aJson.addNullToObject(response, "id");
      }
      else
      {
        aJson.addNumberToObject(response, "id", id->valueint);
      }
      //sprintf(c, "{\"jsonrpc\": \"2.0\",\"error\": {\"code\": %i, \"message\": \"Error getting temperaure\"}, \"id\": 0}", ier);

      if (stream.available()) {
      // skip any accidental whitespace like newlines
        stream.flush();
      }



    }
    else
    {
      //aJson.addItemToObject  (response, "jsonrpc",aJson.createItem("2.0"));
      aJson.addStringToObject(response, "jsonrpc",jsonrpc->valuestring);
      aJson.addItemToObject(response, "result", result );

      //aJson.addNumberToObject(response, "result", 1);
      aJson.addNumberToObject(response, "id", id->valueint);
      //sprintf(c, "{\"jsonrpc\": \"2.0\",\"result\":%i, \"id\": 0}", requestedStatus);   
    }

    //Serial.println("{\"jsonrpc\": \"2.0\",\"result\":1, \"id\": 0}");

    char* json = aJson.print(response);
    Serial.println(json);
    free(json);
    aJson.deleteItem(response);
    //    aJson.deleteItem(result);
    aJson.deleteItem(msg);
    //freeMem("in loop");

  }
}


/*
//Code to print out the free memory
 
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
 
 uint16_t biggest;
 
 void freeMem(char* message) {
 Serial.print(message);
 Serial.print(":\t");
 Serial.println(freeMem(&biggest));
 }
 
 */

