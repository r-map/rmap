/* Arduino mics 4514 Library
 * Copyright (C) 2017 by Paolo Patruno
 *
 * This file is part of the RMAP project https://github.com/r-map/rmap
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with the Arduino SdFat Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "Mics4514.h"
#include "config.h"
#include <avr/wdt.h>
//#include <limits.h>

/*
WARM–UPTIME
The sensor needs a period of time in order for its chemical equilibrium
to be set up. This is due to chemical compounds being desorbed or absorbed on the sensing surface after which point the 
resistance will stabilise. 
The rate of stabilisation is faster when the sensor is operated at higher temperature
consequently pre-heating at a higher voltage then normal operating voltage is 
initially used to achieve stability before the voltage is subsequently reduced. For example,
the NO2 sensor is preheated (normally operated at 40mW) at 80 mW for 30 seconds before use. 
Generally speaking, the longer the warm up phase, the better the precision will be.
*/

#define FASTHEATTIME 5000    // 30000   time required for het the sensor in fast mode
#define HEATTIME 5000        // 60000   time required for het the sensor in slow mode
#define WDTTIMESTEP 5000     // safe time fot atomic operation before all will be restarted by watchdog

#define SCALE0R 1000 // Kohm
#define SCALE1R 100
#define SCALE2R 10

#define CHANGESCALEVALUE 612

using namespace mics4514;

Mics4514::Mics4514(uint8_t copin, uint8_t no2pin,	\
		   uint8_t heaterpin, uint8_t scale1pin,\
		   uint8_t scale2pin)
{
  _copin=copin;
  _no2pin=no2pin;
  _heaterpin=heaterpin;
  _scale1pin=scale1pin;
  _scale2pin=scale2pin;

  _state=cold;
  _lastfastheatertime=0;
  _lastheatertime=0;
  //  _lastfastheatertime=LONG_MAX;
  //_lastheatertime=LONG_MAX;

  pinMode(_heaterpin, OUTPUT);   // sets the pin as output
  pinMode(_scale1pin, OUTPUT);   // sets the pin as output
  pinMode(_scale2pin, OUTPUT);   // sets the pin as output
  analogReference(EXTERNAL);

  sleep();
}

void Mics4514::sleep()
{
  IF_SDEBUG(Serial.println(F("mics4514 sleep")));

 analogWrite(_heaterpin,0);
 digitalWrite(_scale1pin, LOW);  
 digitalWrite(_scale2pin, LOW);  
 _state=cold;
 
}

void Mics4514::fast_heat()
{

  IF_SDEBUG(Serial.println(F("mics4514 fast_heat")));
  //do not heat too much; do it only if required 
  if ((_lastfastheatertime != 0 ) && (millis()-_lastfastheatertime) < (FASTHEATTIME * 2))
    {
      IF_SDEBUG(Serial.println(F("mics4514 disable fast_heat, already done")));
      return;
    }


  if ( _state != cold)
    {
      IF_SDEBUG(Serial.println(F("mics4514 disable fast_heat,  not cold")));

      return;
    }
  
  // switch on heater to max power
  analogWrite(_heaterpin,255);  // analogWrite values from 0 to 255

  _lastfastheatertime=millis();
  _state=heating;

}

void Mics4514::blocking_fast_heat()
{

  IF_SDEBUG(Serial.println(F("mics4514 blocking_fast_heat")));  

  wdt_reset();

  fast_heat();
  
  for (int mytime=0; mytime <=  FASTHEATTIME; mytime+=WDTTIMESTEP)
    {
      delay(WDTTIMESTEP);
      wdt_reset();
    }

  delay(FASTHEATTIME % WDTTIMESTEP);
  wdt_reset();
  
  normal_heat();
  _state=fasthot;

}

void Mics4514::normal_heat()
{

  IF_SDEBUG(Serial.println(F("mics4514 normal_heat")));
  // switch on heater to normal power

  // Rh=66
  // Rtotmax = Rh + 82
  // Rtot = Rh + 133
  // pwm_rate= Rtot/Rtotmax = 79%
    
  //200=255*.79
  
  analogWrite(_heaterpin,200);  // analogWrite values from 0 to 255
  _lastheatertime=millis();
  _state=hot;

}

bool Mics4514::query_data(unsigned int *co, unsigned int *no2)
{

  int dno2, dco;
    
  IF_SDEBUG(Serial.println(F("mics4514 query_data")));

  digitalWrite(_scale1pin, LOW);  
  digitalWrite(_scale2pin, LOW);  
  delay(10);

  switch(_state)
  {
  
  case cold:
      IF_SDEBUG(Serial.println(F("mics4514 disable query_data, I am cold")));

      return false;
      break;

  case hot:
	if ((millis()- _lastheatertime) < HEATTIME)
	{
	  IF_SDEBUG(Serial.println(F("mics4514 disable query_data, hot but few time")));
	  return false;
	}
      break;

  case fasthot:
    // OK
    break;

  case heating:
	if ((millis()- _lastfastheatertime) < FASTHEATTIME)
	{
	  IF_SDEBUG(Serial.println(F("mics4514 disable query_data, I am heating")));
	  return false;
	}
      break;
  }
  
  IF_SDEBUG(Serial.println(F("mics4514 co  scale0 read")));
  dco   = analogRead(_copin);
  delay(10);
  dco   = analogRead(_copin);
  int cor2  = SCALE0R;
  
  IF_SDEBUG(Serial.println(F("mics4514 no2 scale0 read")));
  dno2  = analogRead(_no2pin);
  delay(10);
  dno2  = analogRead(_no2pin);
  int no2r2 = SCALE0R;

  if (dco > CHANGESCALEVALUE)
    {
      digitalWrite(_scale1pin, HIGH);  
      delay(10);
      IF_SDEBUG(Serial.println(F("mics4514 co  scale1 read")));
      dco  = analogRead(_copin);
      delay(10);
      dco  = analogRead(_copin);
      cor2  = round(1./(1./float(SCALE0R)+1./float(SCALE1R)));
    }


  if (dno2 > CHANGESCALEVALUE)
    {
      if (!digitalRead(_scale1pin))
	{
	  IF_SDEBUG(Serial.println(F("mics4514 no2 scale1")));
	  digitalWrite(_scale1pin, HIGH);  
	  delay(10);
	}
      IF_SDEBUG(Serial.println(F("mics4514 no2 scale1 read")));
      dno2  = analogRead(_no2pin);
      delay(10);
      dno2  = analogRead(_no2pin);
      no2r2  = round(1./(1./float(SCALE0R)+1./float(SCALE1R)));
    }

  
  if (dco > CHANGESCALEVALUE)
    {
      if (!digitalRead(_scale1pin))
	{
	  digitalWrite(_scale1pin, HIGH);
	}
      digitalWrite(_scale2pin, HIGH);  
      delay(10);
      IF_SDEBUG(Serial.println(F("mics4514 co  scale2 read")));
      dco  = analogRead(_copin);
      delay(10);
      dco  = analogRead(_copin);
      cor2  = round(1./(1./float(SCALE0R)+1./float(SCALE1R)+1./float(SCALE2R)));
    }

  if (dno2 > CHANGESCALEVALUE)
    {
      if (!digitalRead(_scale1pin))
	{
	  digitalWrite(_scale1pin, HIGH);
	}
      if (!digitalRead(_scale2pin))
	{
	  IF_SDEBUG(Serial.println(F("mics4514 no2 scale2")));
	  digitalWrite(_scale1pin, HIGH);  
	  digitalWrite(_scale2pin, HIGH);
	  delay(10);
	}
      IF_SDEBUG(Serial.println(F("mics4514 no2 scale2 read")));
      dno2  = analogRead(_no2pin);
      delay(10);
      dno2  = analogRead(_no2pin);
      no2r2  = round(1./(1./float(SCALE0R)+1./float(SCALE1R)+1./float(SCALE2R)));
    }    

  IF_SDEBUG(Serial.print(F("mics4514 copin : ")));
  IF_SDEBUG(Serial.println(_copin));
  
  IF_SDEBUG(Serial.print(F("mics4514 no2pin : ")));
  IF_SDEBUG(Serial.println(_no2pin));
  
  IF_SDEBUG(Serial.print(F("mics4514 cor2 : ")));
  IF_SDEBUG(Serial.println(cor2));
  IF_SDEBUG(Serial.print(F("mics4514 no2r2: ")));
  IF_SDEBUG(Serial.println(no2r2));
    
  IF_SDEBUG(Serial.print(F("mics4514 dco : ")));
  IF_SDEBUG(Serial.println(dco));
  IF_SDEBUG(Serial.print(F("mics4514 dno2: ")));
  IF_SDEBUG(Serial.println(dno2));
    
  //compute Rs
  // R1 = ((R2 * Vin) / Vout ) - R2
  // R2 = R1 / (( Vin / Vout) -1)
  
  *co   = round( ((float(cor2)*1023.)/float(dco)) - float(cor2));
  *no2  = round( ((float(no2r2)*1023.)/float(dno2)) - float(no2r2));

  //*co   = round(float(cor2) / ((1023./float(dco)) -1));
  //*no2  = round(float(no2r2) / ((1023./float(dno2)) -1));

  IF_SDEBUG(Serial.print(F("mics4514 co : ")));
  IF_SDEBUG(Serial.println(*co));
  IF_SDEBUG(Serial.print(F("mics4514 no2: ")));
  IF_SDEBUG(Serial.println(*no2));
  
  digitalWrite(_scale1pin, LOW);  
  digitalWrite(_scale2pin, LOW);  

  return true;

  //  bool ok;

  //  ok =_read_response();
  //  if (!ok) {
  //      return false;
  //  }

}

bool Mics4514::query_data_auto(unsigned int *co, unsigned int *no2, int n)
{
  IF_SDEBUG(Serial.println(F("mics4514 query_data_auto")));
    unsigned int co_table[n];
    unsigned int no2_table[n];
    int ok;

    for (int i = 0; i<n; i++) {
        ok = query_data(&co_table[i], &no2_table[i]);
        if (!ok){
            return false;
        }

        delay(100);
    }

    _filter_data(n, co_table, no2_table, co, no2);

    return true;
}

void Mics4514::_filter_data(int n, unsigned int *co_table, unsigned int *no2_table, unsigned int *co, unsigned int *no2)
{
  IF_SDEBUG(Serial.println(F("mics4514 filter_data")));  
    unsigned int co_min, co_max, no2_min, no2_max, co_sum, no2_sum;

    no2_sum = no2_min = no2_max = no2_table[0];
    co_sum = co_min = co_max = co_table[0];

    for (int i=1; i<n; i++) {
        if (no2_table[i] < no2_min) {
            no2_min = no2_table[i];
        }
        if (no2_table[i] > no2_max) {
            no2_max = no2_table[i];
        }
        if (co_table[i] < co_min) {
            co_min = co_table[i];
        }
        if (co_table[i] > co_max) {
            co_max = co_table[i];
        }
        no2_sum += no2_table[i];
        co_sum += co_table[i];
    }

    if (n > 2) {
        *no2 = (no2_sum - no2_max - no2_min)/(n-2);
        *co = (co_sum - co_max - co_min)/(n-2);
    } else if (n > 1) {
        *no2 = (no2_sum - no2_min)/(n-1);
        *co = (co_sum - co_min)/(n-1);
    } else {
        *no2 = no2_sum/n;
        *co = co_sum/n;
    }
}
