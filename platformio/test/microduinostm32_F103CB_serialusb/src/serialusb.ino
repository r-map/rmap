//#include <Arduino.h>
//#include <USBSerial.h>

void setup() {
  //SerialUSB.begin();
  Serial.begin();
}

void loop() {
  //SerialUSB.println("Ciao!");
  Serial.println("Ciao!");
  delay(1000);
}
