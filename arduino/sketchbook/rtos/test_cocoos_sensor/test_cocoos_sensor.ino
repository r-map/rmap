#include <Arduino.h>

void mysetup() {
  //  Print a message to the Arduino serial console. This code is located here because 
  //  Serial API may only be used in a C++ module.
  Serial.begin (9600);
  Serial.println("Started");
}


void debug(const char *s) {
  //  Print a message to the Arduino serial console. This code is located here because 
  //  Serial API may only be used in a C++ module.
  Serial.println (s);
  Serial.flush (); // let serial printing finish
}

