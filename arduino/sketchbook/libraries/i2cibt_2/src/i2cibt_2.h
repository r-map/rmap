#ifndef _I2CIBT_2_H
#define _I2CIBT_2_H

#include <ibt_2.h>
#include <i2cgpio.h>

class i2cibt_2: public domotic {

 public:
  i2cibt_2(uint8_t bridge,i2cgpio gpio);
  void stop(uint8_t bridge=IBT_2_FULL);
  void start(uint8_t bridge=IBT_2_FULL);
  void brake(uint8_t brake=BRAKEVCC);
  void setrotation(uint8_t pwm=0,uint8_t wise=CW);
  void setpwm(uint8_t pwm=0,uint8_t bridge=IBT_2_R_HALF);
  uint16_t get(domotic_bridge half=bridge_r_half);
  void readis();
  bool protect();
  bool protectdelay(unsigned long int stoptime=5000);  


 protected:
  uint8_t _bridge;
  i2cgpio _gpio;
  
};

#endif
