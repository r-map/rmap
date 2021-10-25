/*
Copyright (C) 2018  Paolo Paruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Freeg Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*

  Library driver for IBT_2 over I2C (use i2c-pwm firmware)

  schematics
  https://www.elecrow.com/download/IBT-2%20Schematic.pdf

  BTS 7960
  https://homotix_it.e-mind.it/upld/repository/File/bts7960.pdf

  sn54ahc244
  http://www.ti.com/lit/ds/symlink/sn54ahc244-sp.pdf

*/

#include "i2cibt_2.h"

#define I2C_R_EN 1
#define I2C_L_EN 2

#define I2C_R_PWM 1
#define I2C_L_PWM 2

#define I2C_R_IS 1
#define I2C_L_IS 2


i2cibt_2::i2cibt_2(uint8_t bridge,i2cgpio gpio):
  domotic(bridge),
  _bridge(bridge),
  _gpio(gpio)
{
}
  
void i2cibt_2::stop(uint8_t bridge){

  if (bridge == IBT_2_FULL || bridge == IBT_2_2HALF){
    _gpio.digitalWrite(I2C_R_EN, LOW);
    delay(10);
    _gpio.digitalWrite(I2C_L_EN, LOW);
  } else if (bridge == IBT_2_R_HALF){
    _gpio.digitalWrite(I2C_R_EN, LOW);
  } else if (bridge == IBT_2_L_HALF){
    _gpio.digitalWrite(I2C_L_EN, LOW);
  }
}

void i2cibt_2::start(uint8_t bridge){

  // with this commented full activate all half bridge
  //if (_bridge != IBT_2_FULL && bridge == IBT_2_FULL ) return;
  
  if (bridge == IBT_2_FULL ||  bridge == IBT_2_2HALF){
    _gpio.digitalWrite(I2C_R_EN, HIGH);
    delay(10);
    _gpio.digitalWrite(I2C_L_EN, HIGH);
  } else if (bridge == IBT_2_R_HALF){
    _gpio.digitalWrite(I2C_R_EN, HIGH);
  } else if (bridge == IBT_2_L_HALF){
    _gpio.digitalWrite(I2C_L_EN, HIGH);
  }
}


void i2cibt_2::brake(uint8_t brake){

  if (_bridge != IBT_2_FULL ) return;
  
  if (brake == BRAKEGND){
    _gpio.analogWrite(I2C_R_PWM, 254);
    delay(10);
    _gpio.analogWrite(I2C_L_PWM, 254);
  } else if (brake == BRAKEVCC){
    _gpio.analogWrite(I2C_R_PWM, 0);
    delay(10);
    _gpio.analogWrite(I2C_L_PWM, 0);

  }

  delay(10);
  _gpio.digitalWrite(I2C_R_EN, HIGH);
  delay(10);
  _gpio.digitalWrite(I2C_L_EN, HIGH);

}


void i2cibt_2::setrotation(uint8_t pwm,uint8_t wise){


  /*
  The H-bridge does provide a delay (switch on/off delay / slew rate),
  which would prevent the cross-conduction issue if you could send
  both PWM signals at the same time. But because of the time arduino
  takes to process the line of code (analogWrite in this case) it will
  still cause cross-conduction.

  so attention at analogwrite order!
  */
  

  
  _wise = wise;

  if (_bridge != IBT_2_FULL ) return;

    if (_wise == CW)
    {
      // forward rotation
      _r_pwm= pwm;
      _l_pwm= 0;
      
      _gpio.analogWrite(I2C_L_PWM, _l_pwm);
      delay(10);
      _gpio.analogWrite(I2C_R_PWM, _r_pwm);

    }
  else
    {
      // reverse rotation
      _r_pwm= 0;
      _l_pwm= pwm;
      _gpio.analogWrite(I2C_R_PWM, _r_pwm);
      delay(10);
      _gpio.analogWrite(I2C_L_PWM, _l_pwm);
    }
}


void i2cibt_2::setpwm(uint8_t pwm,uint8_t bridge){

  if (_bridge != IBT_2_2HALF ) return;

  if (bridge == IBT_2_R_HALF) {
    _r_pwm= pwm;
    _gpio.analogWrite(I2C_R_PWM, _r_pwm);
    
  } else if (bridge == IBT_2_L_HALF) {
    _l_pwm= pwm;
    _gpio.analogWrite(I2C_L_PWM, _l_pwm);
  }

}

uint16_t i2cibt_2::get(domotic_bridge half) {
  
  if(half == bridge_r_half){ 
    uint16_t is=_r_is;
    _r_is=0xFFFF;
    return is;
  }else if(half == bridge_l_half){
    uint16_t is=_l_is;
    _l_is=0xFFFF;
    return is;
  }
  return 0xFFFF;
}

void i2cibt_2::readis(){

    _r_is = _gpio.analogRead(I2C_R_IS);
    delay(10);
    _l_is = _gpio.analogRead(I2C_L_IS);
}

bool i2cibt_2::protect(){

  readis();
  if (get(bridge_r_half) > IS_THRESHOLD || get(bridge_l_half) > IS_THRESHOLD){
    stop();
    return true;
  }
  return false;
}

bool i2cibt_2::protectdelay(unsigned long int stoptime){

  bool status = false;
  readis();
  while (protect()){
    delay (stoptime);
    status = true;
  }
  return status;
}

