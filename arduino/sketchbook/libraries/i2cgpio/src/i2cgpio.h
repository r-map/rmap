#ifndef _I2CGPIO
#define _I2CGPIO

#include <Arduino.h>
#include <ArduinoLog.h>
#include <Wire.h>
#include <registers-pwm.h>         //Register definitions

class i2cgpio {

public:
  i2cgpio(unsigned short int address=I2C_PWM_DEFAULTADDRESS);
  uint8_t digitalWrite(uint8_t pin, uint8_t value);
  uint8_t analogWrite(uint8_t pin, uint8_t value);
  uint16_t analogRead(uint8_t pin);
  
private:
  uint8_t _address;
};

#endif
