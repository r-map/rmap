/*
  JsonRPC Library - ToggleLED
 
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
 
 ser.write('{"method": "toggle", "params": {"status": true} }')
 time.sleep(5)
 ser.write('{"method": "toggle", "params": {"status": false} }')
 
 # wait 2 seconds before closing the serial connection
 time.sleep(2)
 ser.close()

 */

// include the aJSON library
#include <aJSON.h>
// include the JsonRPC library
#include <JsonRPC.h>

// initialize an instance of the JsonRPC library for registering 
// exactly 1 local method
JsonRPC rpc(1);
// initialize a serial json stream for receiving json objects
// through a serial/USB connection
aJsonStream stream(&Serial);

// on most arduino boards, pin 13 is connected to a LED
int led = 13;

void setup()
{
  // initialize the digital pin as an output
  pinMode(led, OUTPUT);
  
  // start up the serial interface
  Serial.begin(9600);
  
  // and register the local toggleLED method
  rpc.registerMethod("toggle", &toggleLED);
}

void toggleLED(aJsonObject* params)
{
  aJsonObject* statusParam = aJson.getObjectItem(params, "status");
  boolean requestedStatus = statusParam -> valuebool;
  
  if (requestedStatus)
  {
    digitalWrite(led, HIGH);
  }
  else
  {
    digitalWrite(led, LOW);
  }
  Serial.println('{"result": "Eccote", "error": null, "id": 0}')
  
}

void loop()
{
  if (stream.available()) {
    // skip any accidental whitespace like newlines
    stream.skip();
  }

  if (stream.available()) {
    aJsonObject *msg = aJson.parse(&stream);
    rpc.processMessage(msg);
    aJson.deleteItem(msg);
  }
}

