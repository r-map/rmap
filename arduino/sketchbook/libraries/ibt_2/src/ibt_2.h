#ifndef _IBT_2_H
#define _IBT_2_H

#include <Arduino.h>
#include <ArduinoLog.h>
#include "config.h"

#define IBT_2_FULL   0
#define IBT_2_2HALF 1
#define IBT_2_R_HALF 2
#define IBT_2_L_HALF 3
#define CW   0
#define CCW  1
#define BRAKEVCC 2
#define BRAKEGND 3
#define IS_THRESHOLD 1000


#ifndef ARDUINO_ARCH_ESP8266
#include <avr/wdt.h>
#endif

enum domotic_bridge {
  bridge_full,
  bridge_r_half,
  bridge_l_half
};

class domotic {
 public:
  domotic (uint8_t bridge, uint8_t r_pwm=R_PWM, uint8_t l_pwm=L_PWM, uint8_t r_en=R_EN, uint8_t l_en=L_EN, uint8_t r_is=R_IS, uint8_t l_is=L_IS);
  virtual void     stop(uint8_t bridge)=0;
  virtual void     start(uint8_t bridge)=0;
  virtual void     brake(uint8_t brake)=0;
  virtual void     setrotation(uint8_t pwm,uint8_t wise)=0;
  virtual void     setpwm(uint8_t pwm,uint8_t bridge)=0;
  virtual void     readis()=0;
  virtual uint16_t get(domotic_bridge half)=0;
  virtual bool     protect()=0;
  virtual bool     protectdelay(unsigned long int stoptime)=0;

 protected:
  uint16_t _r_is;
  uint16_t _l_is;
  uint8_t _bridge;
  uint8_t _wise;
  uint8_t _r_pwm;
  uint8_t _l_pwm;
  uint8_t _r_pwm_pin;
  uint8_t _l_pwm_pin;
  uint8_t _r_en_pin;
  uint8_t _l_en_pin;
  uint8_t _r_is_pin;
  uint8_t _l_is_pin;
};


class ibt_2: public domotic {

 public:
  ibt_2(uint8_t bridge, uint8_t r_pwm=R_PWM, uint8_t l_pwm=L_PWM, uint8_t r_en=R_EN, uint8_t l_en=L_EN, uint8_t r_is=R_IS, uint8_t l_is=L_IS);
  void stop(uint8_t bridge=IBT_2_FULL);
  void start(uint8_t bridge=IBT_2_FULL);
  void brake(uint8_t brake=BRAKEGND);
  void setrotation(uint8_t pwm=0,uint8_t wise=CW);
  void setpwm(uint8_t pwm=0,uint8_t bridge=IBT_2_R_HALF);
  uint16_t get(domotic_bridge half=bridge_r_half);
  void readis();
  bool protect();
  bool protectdelay(unsigned long int stoptime=5000);  
};

#endif
