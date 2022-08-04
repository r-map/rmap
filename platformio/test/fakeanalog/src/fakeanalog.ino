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

#include "ad57X1.h"
#include <ArduinoLog.h>

#define CS_AD5791              D10
#define PERIOD_SECONDI         43200    // 12h emiperiod (daily sun cicle)

AD5791 ad5791(CS_AD5791, &SPI, 1*1000*1000,-1,0);  // If you experience trasmit errors, this might be due to your board layout.

volatile unsigned long int secondi=0;
volatile int  START_SECONDI=60;                    // delay to start output tension
HardwareTimer Tim2 = HardwareTimer(TIM2);      

void printTimestamp(Print* _logOutput) {
  char c[12];
  sprintf(c, "%10lu ", millis());
  _logOutput->print(c);
}

void printNewline(Print* _logOutput) {
  _logOutput->print('\n');
}

void sun_movement(void){
  Log.trace(F("1s tick"));
  secondi++;

  if (secondi >= (START_SECONDI + PERIOD_SECONDI)){
    secondi = 0;
    START_SECONDI= 0; // do not delay any more
  }

  if (secondi < START_SECONDI) return;

  int32_t millivolt = round(sin(((secondi-START_SECONDI)/float(PERIOD_SECONDI))*PI)*1000.D);

  if (millivolt <0) millivolt =0;
  
  Log.notice(F("millivolt: %l"),millivolt);
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
  ad5791.setInternalAmplifier(true); // Enable the internal amplifier. This setup allows connecting an external amplifier in a gain of 2 configuration. See the datasheet for details.
  ad5791.enableOutput();    // Turn on the DAC. After startup the output will be clamped to GND and disconnected (tri-state mode)
                            // No need to call updateControlRegister(), because this is a convenience function, which does all that for you

  ad5791.setTension(0);

  Tim2.setOverflow(1, HERTZ_FORMAT);
  Tim2.attachInterrupt(sun_movement);
  Tim2.resume();

}

void loop() {

  // sleep some time to do not go tired ;)
  delay(10000);
  noInterrupts();
  long unsigned int mysecondi=secondi;
  interrupts();

  Log.notice("pseduo sun Time : %ds",mysecondi );
  
}
