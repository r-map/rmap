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

  Library driver for IBT_2

  schematics
  https://www.elecrow.com/download/IBT-2%20Schematic.pdf

  BTS 7960
  https://homotix_it.e-mind.it/upld/repository/File/bts7960.pdf

  sn54ahc244
  http://www.ti.com/lit/ds/symlink/sn54ahc244-sp.pdf



  IS pin has a dual purpose: 

  a) During normal run, current to resistor Ris is proportional to
  load current (actually current of high side switch). Nominal/typical
  scaling factor listed on the datasheet is 8500. So with 15 A
  current, current to resistor Ris is 15 A/8500 = 1,8 mA. If this is
  supplied to 1000 ohm resistor as in ibt_2 board, voltage to Arduino 
  analog input is 1,8 mA*1000 = 1.8 V.

  b) On fault condition, about 7 mA is supplied to Ris. This makes 7
  mA*1000 = 7 V on analog input.

  This shoud be dangerous for arduino input that have 0-5V max input range.

  Add a resistance in parallel to 1Kohm ibt_2 resistor to get a total value of
  714ohm 

  If you do not put a resistance in parallel with IS_THRESHOLD 1000
  you get alarm at 4.88V that is equivalent to a current of 41.5 A or an
  alarm conditions (7V output dangerous for arduino input a zener diode of 5V
 should be required).

*/

#include "ibt_2.h"

void filter_is(unsigned int n, unsigned int *table,  uint16_t *value)
{
    unsigned int value_min, value_max, value_sum;

    value_sum = value_min = value_max = table[0];

    for (int i=1; i<n; i++) {
        if (table[i] < value_min) {
            value_min = table[i];
        }
        if (table[i] > value_max) {
            value_max = table[i];
        }
        value_sum += table[i];
    }

    if (n > 2) {
        *value = (value_sum - value_max - value_min)/(n-2);
    } else if (n > 1) {
        *value = (value_sum - value_min)/(n-1);
    } else {
        *value = value_sum/n;
    }
}



domotic::domotic(unsigned short int bridge):
  _r_is(0xFFFFFFFF),
  _l_is(0xFFFFFFFF),
  _wise(CW),
  _r_pwm(0),
  _l_pwm(0),
  _bridge(bridge)
{
}


ibt_2::ibt_2(unsigned short int bridge):
  domotic(bridge)
{
  pinMode(R_PWM, OUTPUT);
  pinMode(L_PWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);
  pinMode(R_IS, INPUT);
  pinMode(L_IS, INPUT);

  stop();
  setrotation();
}


void ibt_2::stop(unsigned short int bridge){

  if (bridge == IBT_2_FULL){
    digitalWrite(R_EN, LOW);
    digitalWrite(L_EN, LOW);
  } else if (bridge == IBT_2_R_HALF){
    digitalWrite(R_EN, LOW);
  } else if (bridge == IBT_2_L_HALF){
    digitalWrite(L_EN, LOW);
  }
}

void ibt_2::start(unsigned short int bridge){

  // with this commented full activate all half bridge
  //if (_bridge != IBT_2_FULL && bridge == IBT_2_FULL ) return;
  
  if (bridge == IBT_2_FULL){
    digitalWrite(R_EN, HIGH);
    digitalWrite(L_EN, HIGH);
  } else if (bridge == IBT_2_R_HALF){
    digitalWrite(R_EN, HIGH);
  } else if (bridge == IBT_2_L_HALF){
    digitalWrite(L_EN, HIGH);
  }
}


void ibt_2::brake(unsigned short int brake){

  if (_bridge != IBT_2_FULL ) return;
  
  if (brake == BRAKEGND){
    digitalWrite(R_PWM, HIGH);
    digitalWrite(L_PWM, HIGH);
  } else if (brake == BRAKEVCC){
    digitalWrite(R_PWM, LOW);
    digitalWrite(L_PWM, LOW);

  }

  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);

}


void ibt_2::setrotation(unsigned short int pwm,unsigned short int wise){


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
      
      digitalWrite(L_PWM, _l_pwm);
      analogWrite(R_PWM, _r_pwm);

    }  else {
    // reverse rotation
    _r_pwm= 0;
    _l_pwm= pwm;
    digitalWrite(R_PWM,_r_pwm);
    analogWrite(L_PWM, _l_pwm);
  }
}


void ibt_2::setpwm(unsigned short int pwm,unsigned short int bridge){

  if (_bridge != IBT_2_2HALF ) return;

  if (_bridge == IBT_2_R_HALF) {
    _r_pwm= pwm;
    analogWrite(R_PWM, _r_pwm);
    
  } else if (_bridge == IBT_2_L_HALF) {
    // reverse rotation
    _l_pwm= 0;
    analogWrite(L_PWM, _l_pwm);
  }

}

uint16_t ibt_2::get(domotic_bridge half) {
  
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

bool ibt_2::readis(){


    unsigned int table1[NSAMPLE];
    unsigned int table2[NSAMPLE];

    for (int i =0; i < NSAMPLE; i++){
      table1[i] = analogRead(R_IS);
      //LOGN(F("read1: %d" CR),table1[i]);
      table2[i] = analogRead(L_IS);
      //LOGN(F("read2: %d" CR),table2[i]);
      delay(10);
    }

  
    filter_is(NSAMPLE, table1,&_r_is);
    filter_is(NSAMPLE, table2,&_l_is);
  

}

bool ibt_2::protect(){

  readis();
  if (get(bridge_r_half) > IS_THRESHOLD || get(bridge_l_half) > IS_THRESHOLD){
    stop();
    return true;
  }
  return false;
}

bool ibt_2::protectdelay(unsigned long int stoptime){

  bool status = false;
  readis();
  while (protect()){
    delay (stoptime);
    status = true;
  }
  return status;
}

