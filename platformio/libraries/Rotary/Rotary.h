/*
 * Rotary encoder library for Arduino.
 */

#ifndef Rotary_h
#define Rotary_h

#include "Arduino.h"

// Enable this to emit codes twice per step.
// #define HALF_STEP

// Values returned by 'process'
// No complete step yet.
#define DIR_NONE 0x0
// Clockwise step.
#define DIR_CW 0x10
// Counter-clockwise step.
#define DIR_CCW 0x20


#if defined(ESP8266) || defined(ESP32)
  #define ISR_PREFIX IRAM_ATTR
#else
  #define ISR_PREFIX
#endif

class Rotary
{
  public:
    Rotary(char, char);
    ISR_PREFIX unsigned char process();
    void begin(bool pullup=true);
  private:
    unsigned char state;
    unsigned char pin1;
    unsigned char pin2;
};

#endif
 
