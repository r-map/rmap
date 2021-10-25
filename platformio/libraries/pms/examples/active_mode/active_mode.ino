/*
Copyright (C) 2019  Paolo Paruno <p.patruno@iperbole.bologna.it>
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
*/


#include <Arduino.h>
#include <pms.h>
#include <SoftwareSerial.h>

/*
Plantower                           WEMOS
 PIN4     RX    Giallo    PIN_TX    D6
 PIN5     TX    Marrone   PIN_RX    D5
 PIN3     SLEEP Arancione PIN_SLEEP D2   
 PIN6     RESET Bianco    PIN_RESET D1
*/
#define PIN_TX    D6
#define PIN_RX    D5
#define PIN_SLEEP D2
#define PIN_RESET D1

Pmsx003 mypms(PIN_RESET, PIN_SLEEP);
SoftwareSerial pmsserial(PIN_RX, PIN_TX, false, 128);


void setup(void) {
  Serial.begin(115200);
  Serial.println("Start");

  pmsserial.begin(9600);
  mypms.begin(&pmsserial);
  delay(2500);
  mypms.cmd(Pmsx003::cmdWakeup);
  delay(2500);
  mypms.cmd(Pmsx003::cmdModeActive);
  delay(2500);
  Serial.println("end setup");

}


void loop(void) {

  uint16_t data[13];
  
  delay(100);  // required to get the sensor ready
  mypms.cmd(Pmsx003::cmdReadData);

  Serial.println("wait data");
  Pmsx003::PmsStatus status = mypms.waitForData(data, 13);
  while (mypms.available()){
    status = mypms.waitForData(data, 13);
  }

  switch (status) {
  case Pmsx003::OK:
    {
      for (uint8_t i = 0; i < 13; ++i) {
	Serial.println(data[i]);
      }
      break;
    }
  default:
    Serial.print(F("Error status: "));
    Serial.println(status);
  }
  
}

