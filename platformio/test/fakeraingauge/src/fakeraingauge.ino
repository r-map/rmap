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
#ifdef ARDUINO_NUCLEO_L432KC
#define OUTPIN  D6
#else
#define OUTPIN  PC13
#endif

// reverse output on OUTPIN !  usefull for simulatea switch closed on GND
//#define MYHIGH  LOW  
//#define MYLOW   HIGH

#define MYHIGH  HIGH
#define MYLOW   LOW


// use 100 microsends here as unit

#define MINTIMEWITHOUTRAIN 15000         // 1,5s
#define MAXTIMEWITHOUTRAIN 30000         // 3s

// bounce definitions
#define MAXTIMEBOUNCEPULSE 3            // 0.3 ms
#define MINTIMETOBOUNCE    10           // 1 ms
//#define MAXTIMETOBOUNCE    30           // 3 ms

// pulse definitions
//#define MINTIMETORAIN      500          // 50 ms
//#define MAXTIMETORAIN      3000          // 300 ms

// Digiteco rain gauge Stima V3
//#define MAXTIMETOBOUNCE    8            // 0.8 ms
//#define MINTIMETORAIN      400          // 40 ms
//#define MAXTIMETORAIN      600          // 60 ms

// Digiteco rain gauge Stima V4
#define MAXTIMETOBOUNCE    8            // 0.8 ms
#define MINTIMETORAIN      800          // 80 ms
#define MAXTIMETORAIN      120          // 120 ms

// rain period definitions
#define SECONDSTOPOWERON   30
#define SECONDSTOPOWEROFF  3600*24
#define MAXPRECIPITATION   5000



// spike definitions
#define MINTIMEWITHOUTSPIKE  200000        // 20s
#define MAXTIMEWITHOUTSPIKE  300000        // 30s

#define MAXTIMESPIKE            10         // 1 ms
#define MINTIMESPIKE            3          // 0.3 ms

#define SECONDSTOPOWERONSPIKE    20
#define SECONDSTOPOWEROFFSPIKE  600

#include <ArduinoLog.h>

unsigned long int decmillisecondi=0;
unsigned long int secondi=0;
bool pinlocked=false;

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


enum s_states {
	       UNKNOWNSPIKE
	       ,IDLESPIKE
	       ,NOSPIKESTART
	       ,NOSPIKE
	       ,SPIKESTART
	       ,SPIKESTOP
} s_state= UNKNOWNSPIKE;

enum s_events {
	       POWERONSPIKE
	       ,POWEROFFSPIKE
	       ,NONESPIKE
}s_event=NONESPIKE;


unsigned long tp=0;

HardwareTimer Tim1 = HardwareTimer(TIM1);      
HardwareTimer Tim2 = HardwareTimer(TIM2);      

// rain machine
void rain_machine(){

  static unsigned long norain_start_wait;
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
      Log.notice(F("Power On"));
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
    
    pinlocked=false;   // disable spike
    digitalWrite(OUTPIN, MYLOW);
    
    norain_start_wait=decmillisecondi;
    timewithoutrain = random(MINTIMEWITHOUTRAIN,MAXTIMEWITHOUTRAIN+1);
    Log.trace(F("time without rain: %d"),timewithoutrain);

    r_state = NORAIN;
    break;

  case NORAIN:

    Log.trace(F("rain machine NORAIN"));

    if (r_event == POWEROFF) {
      Log.notice(F("Power Off"));
      r_event = NONE;
      r_state = IDLE;
      return;
    }
    
    if ((decmillisecondi-norain_start_wait) < timewithoutrain) {
      return;
    }

    r_state = PREBOUNCESTART;
    break;


  case PREBOUNCESTART:

    Log.trace(F("rain machine PREBOUNCESTART"));

    pinlocked=true;   // disable spike
    bounce_start_wait=decmillisecondi;
    timetobounce = random(MINTIMETOBOUNCE,MAXTIMETOBOUNCE+1);
    r_state = PRBOUNCEOFFSTART;
    break;

  case PRBOUNCEOFFSTART:

    Log.trace(F("rain machine PRBOUNCEOFFSTART"));

    bounceoff_start_wait=decmillisecondi;
    timetobounceoff = random(1,MAXTIMEBOUNCEPULSE+1);
    digitalWrite(OUTPIN, MYLOW);

    r_state = PRBOUNCEOFF;
    break;

  case PRBOUNCEOFF:
    Log.trace(F("rain machine PRBOUNCEOFF"));

    if ((decmillisecondi-bounce_start_wait) >= timetobounce) {
      r_state = RAINSTART;
      return;
    }
    if ((decmillisecondi-bounceoff_start_wait) >= timetobounceoff) {
      r_state = PRBOUNCEONSTART;
      return;
    }

    return;
    
    break;

  case PRBOUNCEONSTART:

    Log.trace(F("rain machine PRBOUNCEONSTART"));

    bounceon_start_wait=decmillisecondi;
    timetobounceon = random(1,MAXTIMEBOUNCEPULSE+1);
    digitalWrite(OUTPIN, MYHIGH);

    r_state = PRBOUNCEON;
    break;

  case PRBOUNCEON:
    Log.trace(F("rain machine PRBOUNCEON"));

    if ((decmillisecondi-bounce_start_wait) >= timetobounce) {
      r_state = RAINSTART;
      return;
    }
    if ((decmillisecondi-bounceon_start_wait) >= timetobounceon) {
      r_state = PRBOUNCEOFFSTART;
      return;
    }

    return;
    
    break;
    
  case RAINSTART:
    Log.trace(F("rain machine RAINSTART"));

    rain_start_wait=decmillisecondi;
    timetorain = random(MINTIMETORAIN,MAXTIMETORAIN+1);
    digitalWrite(OUTPIN, MYHIGH);
    tp++;
    
    r_state = RAIN;
    break;

  case RAIN:
    Log.trace(F("rain machine RAIN"));

    if ((decmillisecondi-rain_start_wait) < timetorain) {
      return;
    }

    r_state = POSTBOUNCESTART;
    break;

  case POSTBOUNCESTART:

    Log.trace(F("rain machine POSTBOUNCESTART"));

    bounce_start_wait=decmillisecondi;
    timetobounce = random(MINTIMETOBOUNCE,MAXTIMETOBOUNCE+1);
    r_state = POBOUNCEOFFSTART;
    break;

  case POBOUNCEOFFSTART:

    Log.trace(F("rain machine POBOUNCEOFFSTART"));

    bounceoff_start_wait=decmillisecondi;
    timetobounceoff = random(1,MAXTIMEBOUNCEPULSE+1);
    digitalWrite(OUTPIN, MYLOW);

    r_state = POBOUNCEOFF;
    break;

  case POBOUNCEOFF:
    Log.trace(F("rain machine POBOUNCEOFF"));

    if ((decmillisecondi-bounce_start_wait) >= timetobounce) {
      r_state = NORAINSTART;
      return;
    }
    if ((decmillisecondi-bounceoff_start_wait) >= timetobounceoff) {
      r_state = POBOUNCEONSTART;
      return;
    }

    return;
    
    break;

  case POBOUNCEONSTART:

    Log.trace(F("rain machine POBOUNCEONSTART"));

    bounceon_start_wait=decmillisecondi;
    timetobounceon = random(1,MAXTIMEBOUNCEPULSE+1);
    digitalWrite(OUTPIN, MYHIGH);

    r_state = POBOUNCEON;
    break;

  case POBOUNCEON:
    Log.trace(F("rain machine POBOUNCEON"));

    if ((decmillisecondi-bounce_start_wait) >= timetobounce) {
	r_state = NORAINSTART;
	return;
    }
    if ((decmillisecondi-bounceon_start_wait) >= timetobounceon) {
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



// spike machine
void spike_machine(){

  static unsigned long nospike_start_wait;
  static unsigned long timewithoutspike;
  static unsigned long spike_start_wait;
  static unsigned long timetospike;
    
  switch(s_state) {
  case UNKNOWNSPIKE:
  
    Log.trace(F("spike machine UNKNOWN"));
    s_state = IDLESPIKE;
    break;
    
  case IDLESPIKE:
    Log.trace(F("spike machine IDLE"));
    switch(s_event) {
    case POWERON:
      Log.notice(F("Power On spike"));
      s_event = NONESPIKE;
      s_state = NOSPIKESTART;
      break;
    default:
      return;
      break;
    }
    break;

  case NOSPIKESTART:

    Log.trace(F("spike machine NOSPIKESTART"));
    nospike_start_wait=decmillisecondi;
    timewithoutspike = random(MINTIMEWITHOUTSPIKE,MAXTIMEWITHOUTSPIKE+1);
    Log.trace(F("time without spike: %d"),timewithoutspike);
    s_state = NOSPIKE;
    break;
    
  case NOSPIKE:

    Log.trace(F("rain machine NOSPIKE"));
    if (s_event == POWEROFFSPIKE) {
      Log.notice(F("Power Off spike"));
      s_event = NONESPIKE;
      s_state = IDLESPIKE;
      return;
    }
    
    if ((decmillisecondi-nospike_start_wait) < timewithoutspike) {
      return;
    }

    s_state = SPIKESTART;
    break;


  case SPIKESTART:

    if (pinlocked) {
      s_state=NOSPIKESTART;
      break;
    }
    
    Log.trace(F("rain machine SPIKESTART"));
    spike_start_wait=decmillisecondi;
    timetospike = random(MINTIMESPIKE,MAXTIMESPIKE+1);
    s_state = SPIKESTOP;

    Log.notice(F("Spike!"));

    digitalWrite(OUTPIN, MYHIGH);
    break;

  case SPIKESTOP:

    Log.trace(F("rain machine SPIKESTOP"));


    if ((decmillisecondi-spike_start_wait) < timetospike) {
      return;
    }

    digitalWrite(OUTPIN, MYLOW);

    s_state = NOSPIKESTART;
    break;

  default:
    LOGN(F("Something go wrong in rain_machine"));
    break;
    
  }

  return;
}


void printTimestamp(Print* _logOutput) {
  char c[12];
  sprintf(c, "%10lu ", decmillisecondi/10);
  _logOutput->print(c);
}

void printNewline(Print* _logOutput) {
  _logOutput->print('\n');
}


void tick_callback(void)
{ 
  //Log.trace("event 1/10 ms tick");
  decmillisecondi++;
  if ( r_state != IDLE && tp >= MAXPRECIPITATION){
    poweroff();
  }
  rain_machine();
  spike_machine();
}


void power_callback(void)
{
  Log.trace(F("1s tick"));
  secondi++;

  if (secondi == SECONDSTOPOWERON +1)  poweron();
  if (secondi == SECONDSTOPOWEROFF +1) poweroff();

  //if (secondi == SECONDSTOPOWERONSPIKE +1)  poweronspike();
  //if (secondi == SECONDSTOPOWEROFFSPIKE +1) poweroffspike();
}


void poweron(void)
{ 
  r_event=POWERON;
}

void poweroff(void)
{ 
  r_event=POWEROFF;
  Tim2.pause();
}

void poweronspike(void)
{ 
  s_event=POWERONSPIKE;
}

void poweroffspike(void)
{ 
  s_event=POWEROFFSPIKE;
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
  digitalWrite(OUTPIN, MYLOW);
  pinMode(OUTPIN, OUTPUT);
  digitalWrite(OUTPIN, MYLOW);  // in reverse mode here is possible we have a spurious tick ! 
    
  randomSeed(analogRead(0));    // on STM32F1 we do not have a true random generator
  
  // start sensor machine
  Log.notice(F("Starting rain_machine"));
  rain_machine();
  spike_machine();

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
  long unsigned int totalp=tp;
  interrupts();

  Log.notice("Total precipitation: %d",totalp );

}

