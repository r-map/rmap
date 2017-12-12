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
 
 ser.write('{"jsonrpc":"2.0","method":"toggle","params":{"status":true},"id":0}')
 time.sleep(5)
 ser.write('{"jsonrpc":"2.0","method":"toggle","params":{"status":false},"id":0}')
 
 # wait 2 seconds before closing the serial connection
 time.sleep(2)
 ser.close()


use those for radio mode:

 ser.write('{"m":"toggle","p": {"status": true},"i":0}')
 time.sleep(5)
 ser.write('{"m": "toggle", "p": {"status": false},"i":0}')


 */

// include the arduinoJsonRPC library
#include <arduinoJsonRPC.h>

// initialize an instance of the JsonRPC library for registering 
// exactly 1 local method
//radio mode is false; do not use compact protocoll
JsonRPC rpc(false);

// on most arduino boards, pin 13 is connected to a LED
int led = 13;

void setup()
{
  // initialize the digital pin as an output
  pinMode(led, OUTPUT);
  
  // start up the serial interface
  Serial.begin(9600);
  Serial.println("#Started");

  // set timeout for stream read in parse json
  //Serial.setTimeout(3000);
  
  // and register the local toggleLED method
  rpc.registerMethod("toggle", &toggle);
}


int toggle(JsonObject& params, JsonObject& result)
{
  if (params.containsKey("status")){
    boolean requestedStatus = params["status"];
    
    if (requestedStatus)
      {
	Serial.println("#switch on LED");
	digitalWrite(led, HIGH);
      }
    else
      {
	Serial.println("#switch off LED");
	digitalWrite(led, LOW);
      }
    result["state"]="done";
    return 0;
    
  }else{
    return 1;
  }
}


void loop()
{
  StaticJsonBuffer<1000> jsonBuffer;
  if (Serial.available()) {
    JsonObject& msg = jsonBuffer.parse(Serial);
      if (msg.success()){
	int err=rpc.processMessage(msg);
	Serial.print(F("#rpc.processMessage return status:"));
	Serial.println(err);
	msg.printTo(Serial);
    }else{
      Serial.println("#error decoding msg");      
    }
  }
}
