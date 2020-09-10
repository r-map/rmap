/**********************************************************************
Copyright (C) 2020  Paolo Paruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Paruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#define OUTPIN  D6

// use 100 microsends here as unit
#define MINTIMEWITHOUTRAIN 50000         // 500 ms
#define MAXTIMEWITHOUTRAIN 80000         // 800 ms
#define MAXTIMEBOUNCEPULSE 3            // 0.3 ms
#define MINTIMETOBOUNCE    10           // 1 ms
#define MAXTIMETOBOUNCE    30           // 3 ms
#define MINTIMETORAIN      500          // 50 ms
#define MAXTIMETORAIN      3000          // 300 ms

#define SECONDSTOPOWERON 10
#define SECONDSTOPOWEROFF 60

#include <ArduinoLog.h>

unsigned long int millisecondi=0;
unsigned long int secondi=0;

// global variables for sensors state machine
enum r_states {
	       UNKNOWN
	       ,IDLE
	       ,NORAINSTART
	       ,NORAIN
	       ,PREBOUNCESTART
	       ,PRBOUNCEOFFSTART
	       ,PRBOUNCEOFF
	       ,PRBOUNCEONSTART
	       ,PRBOUNCEON
	       ,RAINSTART
	       ,RAIN
	       ,POSTBOUNCESTART
	       ,POEBOUNCESTART
	       ,POBOUNCEOFFSTART
	       ,POBOUNCEOFF
	       ,POBOUNCEONSTART
	       ,POBOUNCEON
} r_state= UNKNOWN;

enum r_events {
	       POWERON
	       ,POWEROFF
	       ,NONE
}r_event=NONE;

unsigned long tp=0;

HardwareTimer Tim1 = HardwareTimer(TIM1);      
HardwareTimer Tim2 = HardwareTimer(TIM2);      

// rain machine
void rain_machine(){

  static unsigned int norain_start_wait;
  static unsigned long timewithoutrain;
  static unsigned long timetobounce;
  static unsigned long bounceoff_start_wait;
  static unsigned long timetobounceoff;
  static unsigned long bounce_start_wait;
  static unsigned long bounceon_start_wait;
  static unsigned long timetobounceon;
  static unsigned long rain_start_wait;
  static unsigned long timetorain;
  
  switch(r_state) {
  case UNKNOWN:
  
    Log.trace(F("rain machine UNKNOWN"));
    r_state = IDLE;
    break;
    
  case IDLE:
    Log.trace(F("rain machine IDLE"));
    switch(r_event) {
    case POWERON:
      r_event = NONE;
      r_state = NORAINSTART;
      break;
    default:
      return;
      break;
    }
    break;

  case NORAINSTART:

    Log.trace(F("rain machine NORAINSTART"));
    
    digitalWrite(OUTPIN, LOW);
    
    norain_start_wait=millisecondi;
    timewithoutrain = random(MINTIMEWITHOUTRAIN,MAXTIMEWITHOUTRAIN+1);
    Log.trace(F("time without rain: %d"),timewithoutrain);

    r_state = NORAIN;
    break;

  case NORAIN:

    Log.trace(F("rain machine NORAIN"));

    switch(r_event) {
    case POWEROFF:
      r_event = NONE;
      r_state = IDLE;
      return;
      break;
    }
    
    if ((millisecondi-norain_start_wait) < timewithoutrain) {
      return;
    }

    r_state = PREBOUNCESTART;
    break;


  case PREBOUNCESTART:

    Log.trace(F("rain machine PREBOUNCESTART"));

    bounce_start_wait=millisecondi;
    timetobounce = random(MINTIMETOBOUNCE,MAXTIMETOBOUNCE+1);
    r_state = PRBOUNCEOFFSTART;
    break;

  case PRBOUNCEOFFSTART:

    Log.trace(F("rain machine PRBOUNCEOFFSTART"));

    bounceoff_start_wait=millisecondi;
    timetobounceoff = random(1,MAXTIMEBOUNCEPULSE+1);
    digitalWrite(OUTPIN, LOW);

    r_state = PRBOUNCEOFF;
    break;

  case PRBOUNCEOFF:
    Log.trace(F("rain machine PRBOUNCEOFF"));

    if ((millisecondi-bounce_start_wait) >= timetobounce) {
      r_state = RAINSTART;
      return;
    }
    if ((millisecondi-bounceoff_start_wait) >= timetobounceoff) {
      r_state = PRBOUNCEONSTART;
      return;
    }

    return;
    
    break;

  case PRBOUNCEONSTART:

    Log.trace(F("rain machine PRBOUNCEONSTART"));

    bounceon_start_wait=millisecondi;
    timetobounceon = random(1,MAXTIMEBOUNCEPULSE+1);
    digitalWrite(OUTPIN, HIGH);

    r_state = PRBOUNCEON;
    break;

  case PRBOUNCEON:
    Log.trace(F("rain machine PRBOUNCEON"));

    if ((millisecondi-bounce_start_wait) >= timetobounce) {
      r_state = RAINSTART;
      return;
    }
    if ((millisecondi-bounceon_start_wait) >= timetobounceon) {
      r_state = PRBOUNCEOFFSTART;
      return;
    }

    return;
    
    break;
    
  case RAINSTART:
    Log.trace(F("rain machine RAINSTART"));

    rain_start_wait=millisecondi;
    timetorain = random(MINTIMETORAIN,MAXTIMETORAIN+1);
    digitalWrite(OUTPIN, HIGH);
    tp++;
    
    r_state = RAIN;
    break;

  case RAIN:
    Log.trace(F("rain machine RAIN"));

    if ((millisecondi-rain_start_wait) < timetorain) {
      return;
    }

    r_state = POSTBOUNCESTART;
    break;

  case POSTBOUNCESTART:

    Log.trace(F("rain machine POSTBOUNCESTART"));

    bounce_start_wait=millisecondi;
    timetobounce = random(MINTIMETOBOUNCE,MAXTIMETOBOUNCE+1);
    r_state = POBOUNCEOFFSTART;
    break;

  case POBOUNCEOFFSTART:

    Log.trace(F("rain machine POBOUNCEOFFSTART"));

    bounceoff_start_wait=millisecondi;
    timetobounceoff = random(1,MAXTIMEBOUNCEPULSE+1);
    digitalWrite(OUTPIN, LOW);

    r_state = POBOUNCEOFF;
    break;

  case POBOUNCEOFF:
    Log.trace(F("rain machine POBOUNCEOFF"));

    if ((millisecondi-bounce_start_wait) >= timetobounce) {
      r_state = NORAINSTART;
      return;
    }
    if ((millisecondi-bounceoff_start_wait) >= timetobounceoff) {
      r_state = POBOUNCEONSTART;
      return;
    }

    return;
    
    break;

  case POBOUNCEONSTART:

    Log.trace(F("rain machine POBOUNCEONSTART"));

    bounceon_start_wait=millisecondi;
    timetobounceon = random(1,MAXTIMEBOUNCEPULSE+1);
    digitalWrite(OUTPIN, HIGH);

    r_state = POBOUNCEON;
    break;

  case POBOUNCEON:
    Log.trace(F("rain machine POBOUNCEON"));

    if ((millisecondi-bounce_start_wait) >= timetobounce) {
	r_state = NORAINSTART;
	return;
    }
    if ((millisecondi-bounceon_start_wait) >= timetobounceon) {
      r_state = POBOUNCEOFFSTART;
      return;
    }
    
    return;
    break;
    
  default:
    LOGN(F("Something go wrong in rain_machine"));
    break;
    
  }

  return;
}


void printTimestamp(Print* _logOutput) {
  char c[12];
  sprintf(c, "%10lu ", millisecondi);
  _logOutput->print(c);
}

void printNewline(Print* _logOutput) {
  _logOutput->print('\n');
}


void tick_callback(void)
{ 
  //Log.trace("event 1/10 ms tick");
  millisecondi++;
  rain_machine();
}

void power_callback(void)
{
  Log.trace(F("1s tick"));
  secondi++;

  if (secondi == SECONDSTOPOWERON +1)  poweron();
  if (secondi == SECONDSTOPOWEROFF +1) poweroff();
}


void poweron(void)
{ 
  Log.notice(F("Power On"));
  r_event=POWERON;
}

void poweroff(void)
{ 
  Log.notice(F("Power Off"));
  r_event=POWEROFF;
  Tim2.pause();
}


void setup (void)
{

  // start up the serial interface
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_NOTICE, &Serial);
  Log.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
  Log.setSuffix(printNewline); // Uncomment to get newline as suffix

  //Start logging
  Log.notice(F("Starting Fake Rain Gauge"));
  
  // configure pin in output mode
  pinMode(OUTPIN, OUTPUT);
  
  // start sensor machine
  Log.notice(F("Starting rain_machine"));
  rain_machine();

  Tim1.setOverflow(100, MICROSEC_FORMAT);
  Tim1.attachInterrupt(tick_callback);

  Tim2.setOverflow(1, HERTZ_FORMAT);
  Tim2.attachInterrupt(power_callback);

  Tim1.resume();
  Tim2.resume();

}

void loop()
{
  // sleep some time to do not go tired ;)
  delay(10000);
  noInterrupts();
  Log.notice("Total precipitation: %d",tp );
  interrupts();

}

