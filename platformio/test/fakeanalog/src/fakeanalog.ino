/**********************************************************************
Copyright (C) 2022  Paolo Paruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>

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
 Il test si svolge in questo modo:
 1) avvio con output a 0V per 1000 secondi
 2) inizio semi-sinusoide con valori tra 0V e 3V con semiperiodo di 12h
 3) inizio periodo con valori costanti a 0V per 12h
 4) si ripete dal punto 2)
 
 il modulo dovrebbe avere queste caratteristiche:
 a programmable 20-bit voltage source. The output range is −5 V to +5 V
 with ±1 LSB integral nonlinearity (INL), ±1 LSB differential
 nonlinearity (DNL), and exceptionally low noise and low drift across
 the entire output range.
*/

#include "ad57X1.h"
#include <ArduinoLog.h>

#define CS_AD5791                D10
#define START_SECONDI           1000    // delay to start output tension
#define PERIOD_SECONDI         43200    // 12h emiperiod (daily sun cicle)

AD5791 ad5791(CS_AD5791, &SPI, 1*1000*1000,-1,0);  // If you experience trasmit errors, this might be due to your board layout.

volatile unsigned long int secondi=0;
volatile unsigned long int sun_secondi=0;
volatile int  start_secondi=START_SECONDI;         // delay to start output tension
HardwareTimer Tim2 = HardwareTimer(TIM2);      

void printTimestamp(Print* _logOutput) {
  char c[12];
  sprintf(c, "%10lu ", millis());
  _logOutput->print(c);
}

void printNewline(Print* _logOutput) {
  _logOutput->print('\n');
}

void time_management(void){
  Log.trace(F("1s tick"));
  secondi++;
  if (secondi <= START_SECONDI){
    return;
  }
  start_secondi = 0; // do not delay any more
  sun_secondi++;
  sun_movement();  
}

void sun_movement(void){

  int32_t millivolt = round(sin((sun_secondi/float(PERIOD_SECONDI))*PI)*3000.D);
  if (millivolt <0) millivolt =0;
  Log.notice(F("sun time:%ls ; millivolt: %l"),sun_secondi,millivolt);
  ad5791.setTension(millivolt);

}

void setup() {

  Serial.begin(115200);
  Log.begin(LOG_LEVEL_NOTICE, &Serial);
  Log.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
  Log.setSuffix(printNewline); // Uncomment to get newline as suffix

  //Start logging
  Log.notice(F("Starting Fake analog"));
  
  ad5791.begin(true);   // Set the pin modes
  // we range from 0 to 3V so no amplifier is not needed
  //ad5791.setInternalAmplifier(true); // Enable the internal amplifier. This setup allows connecting an external amplifier in a gain of 2 configuration setting output ranging fron -5V to 5V. See the datasheet for details.
  ad5791.enableOutput();    // Turn on the DAC. After startup the output will be clamped to GND and disconnected (tri-state mode)
                            // No need to call updateControlRegister(), because this is a convenience function, which does all that for you

  ad5791.setTension(0);

  Tim2.setOverflow(1, HERTZ_FORMAT);
  Tim2.attachInterrupt(time_management);
  Tim2.resume();

}

void loop() {

  // sleep some time to do not go tired ;)
  delay(10000);
  noInterrupts();
  long unsigned int mysecondi=sun_secondi;
  interrupts();

  Log.notice("pseduo sun Time : %ds",mysecondi );
  
}
