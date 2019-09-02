#include <Arduino.h>
#include "sensor.h"

void debug(const char *s) {
  //  Print a message to the Arduino serial console. This code is located here because 
  //  Serial API may only be used in a C++ module.  Declared in sensor.h
  Serial.begin (9600);
  Serial.println (s);
  Serial.flush (); // let serial printing finish
}
