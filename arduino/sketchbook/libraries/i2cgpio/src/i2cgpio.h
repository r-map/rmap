#ifndef _I2CGPIO
#define _I2CGPIO

#include <Arduino.h>
#include <ArduinoLog.h>
#include <Wire.h>
#include <registers-gpio.h>         //Register definitions
#include <config.h>

class i2cgpio {

public:
  i2cgpio(unsigned short int address=I2C_GPIO_DEFAULTADDRESS);
  uint8_t digitalWrite(uint8_t pin, uint8_t value);
  uint8_t analogWrite(uint8_t pin, uint8_t value);
  uint16_t analogRead(uint8_t pin);
  uint8_t stepper_poweroff();
  uint8_t stepper_goto_position(int16_t value);
  uint8_t stepper_read_position(int16_t& position);
  uint8_t stepper_relative_steps(int16_t value);
  uint8_t stepper_rotate(int16_t value);
  uint8_t stepper_gohome();
  uint8_t servo_goto_position(uint8_t motor,int16_t value);
    
private:
  uint8_t _address;
};

#endif
