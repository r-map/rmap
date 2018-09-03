#ifndef _I2CDOMOTIC
#define _I2CDOMOTIC

#include <Arduino.h>
#include <ArduinoLog.h>
#include <Wire.h>
#include <registers-pwm.h>         //Register definitions

class i2cdomotic {

public:
  i2cdomotic(unsigned short int address);
  uint8_t digitalWrite(uint8_t pin, uint8_t value);
  uint8_t analogWrite(uint8_t pin, uint8_t value);
  uint16_t analogRead(uint8_t pin);
  
private:
  uint8_t _address;
};

#endif
