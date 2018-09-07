#ifndef _I2CIBT_2_H
#define _I2CIBT_2_H

#include <ibt_2.h>
#include <i2cgpio.h>

class i2cibt_2: public domotic {

 public:
  i2cibt_2(unsigned short int bridge,i2cgpio gpio);
  void stop(unsigned short int bridge=IBT_2_FULL);
  void start(unsigned short int bridge=IBT_2_FULL);
  void brake(unsigned short int brake=BRAKEGND);
  void setrotation(unsigned short int pwm=0,unsigned short int wise=CW);
  void setpwm(unsigned short int pwm=0,unsigned short int bridge=IBT_2_R_HALF);
  uint16_t get(domotic_bridge half=bridge_r_half);
  bool readis();
  bool protect();
  bool protectdelay(unsigned long int stoptime=5000);  

 protected:
  unsigned short int _bridge;
  i2cgpio _gpio;
  
};

#endif
