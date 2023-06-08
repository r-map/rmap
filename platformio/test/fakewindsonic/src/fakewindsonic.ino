/**********************************************************************
Copyright (C) 2022  Paolo Paruno <p.patruno@iperbole.bologna.it>
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


/*
Questo programma simula un sensore del vento windsonic option 1.  La
simulazione è solo relativa alla pubblicazione dei dati e supporta le
due modalità continua e poll.

Questo utilizzando le seguent1 due configurazione del windsonic
(corrispondente all'output del comando D3):
continua
M2,U1,O1,L1,P1,B3,H2,NQ,F1,E3,T1,S4,C2,G0,K50,

poll
M4,U1,O1,L1,P1,B3,H2,NQ,F1,E3,T1,S4,C2,G0,K50,

La macro POLL_MODE pernette l'attivazione della modalità poll.

La macro TIMER_INTERVAL_MS definisce l'intervallo di pubblicazione dei
mati in modalità continua (1 per secondo con configurazione M2 e P1 che è il
default)

La porta seriale 0 è utilizzata per il loggin mentre la portaseriale 1
è qualla che simula il windsonic. In un microduino core+ la porta
seriale 1 corrisponde a:
 D2 -> RX1 D3 -> TX1

Le porte possono essere invertite

Attivando la macro PLOT è possibile generare dei grafici delle
variabili del vento simulate.
Il grafico lo si genera tramite il programma listener installabile:
https://github.com/devinaconley/arduino-plotter/wiki/Installation-and-Quickstart#setup-listener

Modificando il programma è possibile generare dei venti simulati con
differenti caratteristiche; è possibile agire su tre componenti:

* componente costante (macro CONSTANT_U_VELOCITY , CONSTANT_V_VELOCITY)
* componente casuale definendo valore minimo e massimo (macro RANDOM_U_VELOCITY,RANDOM_V_VELOCITY)
* componente rotatoria a velocità costante (macro ROTATION_MINUTES per definire il periodo,
   macro ROTATION_VELOCITY per definire la velocità)

L'unità di musura della velocità è m/s

 */

///////////////////////////////////////

// exchange serial port definition
#define MYSERIAL0 Serial1
#define MYSERIAL1 Serial

//switch from M2 and M4 Message format windsonic configuration parameter
// Gill, Polar, Continuous
//  or
// Gill, Polar, Polled
#define POLL_MODE true

// rotation period and velocity
#define ROTATION_MINUTES    10
#define ROTATION_VELOCITY   20.D

// constant u component
//#define CONSTANT_U_VELOCITY 5.D
#define CONSTANT_U_VELOCITY 10.D
// random u component
//#define RANDOM_U_VELOCITY   random(-1000,1001)*0.01D
#define RANDOM_U_VELOCITY   0.0D

// constant v component
//#define CONSTANT_V_VELOCITY 5.D
#define CONSTANT_V_VELOCITY 10.D
// random v component
//#define RANDOM_V_VELOCITY   random(-1000,1001)*0.01D
#define RANDOM_V_VELOCITY   0.0D

// create plot graph with plot library
//#define PLOT 1

// log level
#define LOG_LEVEL LOG_LEVEL_NOTICE

// Continuous MODE : M2 Message format windsonic configuration parameter
// P1  1 per second
#define TIMER_INTERVAL_MS        1000L

// induce crc error if != 0
#define CRC_ERROR 0

/*
Windsonic Status

Code Status                   Condition
00   OK                       Sufficient samples in average period
01   Axis 1 failed            Insufficient samples in average period on U axis
02   Axis 2 failed            Insufficient samples in average period on V axis
04   Axis 1 and 2 failed      Insufficient samples in average period on both axes
08   NVM error                NVM checksum failed
09   ROM error                ROM checksum failed
A    -                        NMEA data Acceptable
V    -                        NMEA data Void
*/
# define WINDSONIC_STATUS 0x00

#ifdef ARDUINO_ARCH_AVR
  // Select the timers you're using, here ITimer1
  #define USE_TIMER_1     true
  #define USE_TIMER_2     false
  #define USE_TIMER_3     false
  #define USE_TIMER_4     false
  #include "ATmega_TimerInterrupt.h"
#else

  HardwareTimer Tim2 = HardwareTimer(TIM2);      

  #if defined (STM32L452xx)
    HardwareSerial MYSERIAL0(PC11, PC10);
  #elif defined (STM32L476xx)
    HardwareSerial MYSERIAL0(PC11, PC10);
  #else
    HardwareSerial MYSERIAL0(D0, D1);
  #endif
#endif


///////////////////////////////////////

#if defined(PLOT)
#define DISABLE_LOGGING true
#endif

#include <ArduinoLog.h>
#if defined(PLOT)
#include "Plotter.h"
#endif

unsigned long starttime;

#if defined(PLOT)
Plotter plot; // create plotter
#endif

// global required by plot lib
double u;
double v;

double dd;
double ff;

uint8_t status=WINDSONIC_STATUS;


void message(void){
  
  int baseu = CONSTANT_U_VELOCITY;
  double rndu = RANDOM_U_VELOCITY;
  double au =   double(millis()-starttime)/(ROTATION_MINUTES*60.D*1000.D)*2.D*PI;
  double hourlyu = sin(au)*ROTATION_VELOCITY;

  // simulate wind with constant + random + rotating components
  u = baseu + rndu + hourlyu;

  LOGV(F("U:\n"));
  LOGV("base value: %d\n",baseu);
  LOGV("random: %D\n",rndu);
  LOGV("hourly cycle: %D\n",hourlyu);
  LOGV(F("value: %D\n"),u);
  
  int basev = CONSTANT_V_VELOCITY;;
  double rndv = RANDOM_V_VELOCITY;
  double hourlyv =   cos(au)*ROTATION_VELOCITY; // fase 90 gradi ripetto a u

  // simulate wind with constant + random + rotating components
  v = basev + rndv + hourlyv;
  
  LOGV(F("V:\n"));
  LOGV("base value: %d\n",basev);
  LOGV("random: %D\n",rndv);
  LOGV("hourly cycle: %D\n",hourlyv);
  LOGV(F("value: %D\n"),v);
  
  /*
    float ar=float(dd)*PI/180.;
    //scambio seno e coseno per rotazione 90 gradi
    u=round(-float(ff)*sin(ar));
    v=round(-float(ff)*cos(ar));
  */
           
  if(abs(u) < 0.05D && abs(v) < 0.05D) {
    dd =  0.D;
    ff =  0.D;
  }else{
    ff=sqrt(u*u+v*v) ;
    //scambio seno e coseno per rotazione 90 gradi
    dd=atan2(-u,-v)*180.D/PI;
    LOGV(F("dd1: %D\n"),dd);
    dd=int(round(dd)) % 360;
    if (round(dd) == 360) dd=0.D ;
    if (dd < 0.) dd=360.D+dd;
  }
  
  #if defined(PLOT)
  plot.Plot();
  #endif

  LOGN(F("U: %D\n"),u);
  LOGN(F("V: %D\n"),v);
  
  LOGN(F("dd: %D\n"),dd);
  LOGN(F("ff: %D\n"),ff);

  char buffer[50];

  
  if (ff < 0.05D){          // K50
    //Q,,200.42,M,00,
    buffer[0]=2;
    sprintf(&buffer[1],"Q,,%06.2f,M,%02X,",ff,status);
    uint8_t myCrc=CRC_ERROR;
    for (uint8_t i =1; i < 16; i++) {
      myCrc ^= buffer[i];
    }
    buffer[16]=3;

    MYSERIAL0.write(buffer,17);
    MYSERIAL1.write(buffer,17);

    sprintf(buffer,"%02X",myCrc);
    MYSERIAL0.write(buffer,2);
    MYSERIAL1.write(buffer,2);

    MYSERIAL0.print("\r\n");
    MYSERIAL1.print("\r\n");
    
  }else{
    //Q,126,200.42,M,00,
    buffer[0]=2;
    sprintf(&buffer[1],"Q,%03d,%06.2f,M,%02X,",int(round(dd)),ff,status);
    uint8_t myCrc=CRC_ERROR;
    for (uint8_t i =1; i < 19; i++) {
      myCrc ^= buffer[i];
    }
    buffer[19]=3;

    MYSERIAL0.write(buffer,20);
    MYSERIAL1.write(buffer,20);

    sprintf(buffer,"%02X",myCrc);
    MYSERIAL0.write(buffer,2);
    MYSERIAL1.write(buffer,2);
 
    MYSERIAL0.print("\r\n");    
    MYSERIAL1.print("\r\n");
    
  }
}


void setup() {

  MYSERIAL0.begin(115200);        // connect to the serial port
  Log.begin(LOG_LEVEL, &MYSERIAL0);

  MYSERIAL1.begin(9600);        // connect to the Windsonic serial port
  
  #if defined(PLOT)
  plot.Begin(); // start plotter  
  plot.AddTimeGraph( "Fake Windsonic", 600, "Direction", dd,"Velocity",ff,"u component", u,"v component",v ); 
  #endif

  starttime = millis();       // start the daily cycle for temperature and humidity

  #ifndef POLL_MODE
    #ifdef ARDUINO_ARCH_AVR
      ITimer1.init();
      // Interval in unsigned long millisecs
      if (ITimer1.attachInterruptInterval(TIMER_INTERVAL_MS, message))
        LOGN("Starting ITimer OK, millis() = %l\n",millis());
      else
        LOGE("Can't set ITimer. Select another freq. or timer\n");
    #else
      
      Tim2.setOverflow(TIMER_INTERVAL_MS*1000, MICROSEC_FORMAT);
      Tim2.attachInterrupt(message);
      
    #endif
  #endif
  
  MYSERIAL0.println(F("end setup"));
  MYSERIAL0.flush();
#ifndef ARDUINO_ARCH_AVR
  Tim2.resume();
#endif
}

void loop() {  

  #ifdef POLL_MODE 
    if(MYSERIAL1.find("?Q!",3))  message();
  #else
    delay(10000);
  #endif
}
