#ifndef _IBT_2_H
#define _IBT_2_H

#include <Arduino.h>
#include <ArduinoLog.h>

#define IBT_2_FULL   0
#define IBT_2_2HALF 1
#define IBT_2_R_HALF 2
#define IBT_2_L_HALF 3
#define CW   0
#define CCW  1
#define BRAKEVCC 2
#define BRAKEGND 3
#define IS_THRESHOLD 1000

#define R_PWM   10
#define L_PWM   11
#define R_EN    4
#define L_EN    5
#define R_IS    0
#define L_IS    1

#define NSAMPLE 5

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
  domotic (unsigned short int bridge);
  virtual void     stop(unsigned short int bridge)=0;
  virtual void     start(unsigned short int bridge)=0;
  virtual void     brake(unsigned short int brake)=0;
  virtual void     setrotation(unsigned short int pwm,unsigned short int wise)=0;
  virtual void     setpwm(unsigned short int pwm,unsigned short int bridge)=0;
  virtual bool     readis()=0;
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
};


class ibt_2: public domotic {

 public:
  ibt_2(unsigned short int bridge);
  void stop(unsigned short int bridge=IBT_2_FULL);
  void start(unsigned short int bridge=IBT_2_FULL);
  void brake(unsigned short int brake=BRAKEGND);
  void setrotation(unsigned short int pwm=0,unsigned short int wise=CW);
  void setpwm(unsigned short int pwm=0,unsigned short int bridge=IBT_2_R_HALF);
  uint16_t get(domotic_bridge half=bridge_r_half);
  bool readis();
  bool protect();
  bool protectdelay(unsigned long int stoptime=5000);  
};

#endif
