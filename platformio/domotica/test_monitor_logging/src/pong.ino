#include <ArduinoJson.h>
#include "config.h"

HardwareSerial Serial1(SERIAL1RX, SERIAL1TX);


void sendMessage(void){
// Allocate the JSON document
//
// Inside the brackets, 200 is the RAM allocated to this document.
// Don't forget to change this value to match your requirement.
// Use arduinojson.org/v6/assistant to compute the capacity.
StaticJsonDocument<200> doc;

// StaticJsonObject allocates memory on the stack, it can be
// replaced by DynamicJsonDocument which allocates in the heap.
//
// DynamicJsonDocument  doc(200);

  // Add values in the document
  //
  doc["sensor"] = "gps";
  doc["time"] = millis();

  // Add an array.
  //
  JsonArray data = doc.createNestedArray("data");
  data.add(48.756080);
  data.add(2.302038);

  Serial.println("send message");
  // Generate the prettified JSON and send it to the Serial port.
  //
  serializeJsonPretty(doc, Serial);
  // The above line prints:
  // {
  //   "sensor": "gps",
  //   "time": 1351824120,
  //   "data": [
  //     48.756080,
  //     2.302038
  //   ]
  // }

  Serial.println();
  
  // Generate the minified JSON and send it to the Serial port.
  //
  serializeJson(doc, Serial1);
  // The above line prints:
  // {"sensor":"gps","time":1351824120,"data":[48.756080,2.302038]}

  // terminate message
  Serial1.print("#");
}


void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial.println("started");
}

void loop() {

  delay(PINGTIMEOUT/2);
  Serial.println("send pong");
  Serial1.print("pong#");

  delay((PINGTIMEOUT/2)-1000);
  sendMessage();
  
  delay (1000);
  // send second message
  sendMessage();  
}
