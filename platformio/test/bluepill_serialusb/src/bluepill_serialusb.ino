#include <Arduino.h>
#include <USBSerial.h>

void setup() {
  SerialUSB.begin();
}

void loop() {
  SerialUSB.println("Ciao!");
}
