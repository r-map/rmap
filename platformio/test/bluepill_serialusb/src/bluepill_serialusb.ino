#include <Arduino.h>
//#include <USBSerial.h>

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println("Ciao!");
}
