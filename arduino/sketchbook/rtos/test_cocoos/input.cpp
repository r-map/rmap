#include <Arduino.h> ////
#include "sensor.h"
#include "input.h"
#include <cocoos.h>


bool input_available(){
  return Serial.available();
}

uint8_t input_getcommand(){
    char c2 = Serial.read();
    //debug(c2);

    if ( c2 == '[') {
      char c3 = Serial.read();
      //debug(c3);
      if (c3 == 'A') {
        // arrow up pressed
	return 1;
      }
      else if (c3 == 'B') {
        // arrow down pressed
	return -1;
      }
    }
    return 0;
}

