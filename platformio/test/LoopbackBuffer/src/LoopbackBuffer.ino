#include <LoopbackStream.h>
#include <ArduinoJson.h>

LoopbackStream lstream;

void setup() {
  Serial.begin(115200);
  Serial.println("Hello, World");
  lstream.clear();
}

int i=0;
StaticJsonDocument<1024> doc;

void loop() {
  i++;
  /*
    while (lstream.available()) {
    Serial.write(lstream.read());
    }
    lstream << "Bye bye \n";
    lstream.println("hello!");
  */

  
  doc["value"]=i;
  serializeJson(doc, lstream);
  serializeJson(doc, Serial);
    
  DeserializationError error = deserializeJson(doc, lstream);
  if (error) {
    Serial.println("error");
  }else{
    int value=doc["value"];
    Serial.print("read value: ");
    Serial.println(value);
  }

  delay(1000);

}
