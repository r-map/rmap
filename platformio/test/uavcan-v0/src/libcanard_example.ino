/*
Copyright (C) 2020  Paolo Paruno <p.patruno@iperbole.bologna.it>
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

#include "Arduino.h"
#include "canard.h"
#include "uavcan.h"

unsigned long previousMillis = 0;     // stores last time output was updated
const long interval = 1000;           // interval at which to print output (milliseconds)

void setup() {

  Serial.begin(115200);
  Serial.print("Initializing...");
  Serial.println(HAL_RCC_GetHCLKFreq());

  // initialize digital pins
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  CAN_HW_Init();
  uavcanInit();

}

void loop() {

  unsigned long currentMillis;

  currentMillis = millis();

  sendCanard();
  receiveCanard();
  spinCanard();
  publishCanard();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    showRcpwmonUart();
  }

}
