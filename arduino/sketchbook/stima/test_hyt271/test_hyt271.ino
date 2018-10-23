/*****************************************************************//**
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>

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

#include <hyt2x1.h>

#define POWER_PIN  (13)

void test_read_ht(int8_t address) {
  float humidity;
  float temperature;

  Serial.print("Sensor HYT2X1 on 0x");
  Serial.print(address,HEX);
  Serial.println("");

  if (Hyt2X1::hyt_read(address, &humidity, &temperature)) {
    Serial.print("---> Humidity: ");
    Serial.print(humidity);
    Serial.print(" % \t\tB13003: ");
    Serial.println(round(humidity));

    Serial.print("---> Temperature: ");
    Serial.print(temperature);
    Serial.print(" °");
    //Serial.write(0xB0); // °
    Serial.print("C \tB12101: ");
    Serial.print(temperature + 273.15);
    Serial.println("");
    Serial.println("");
  }
  else {
    Serial.println("---> Not Found !!!");
    Serial.println("");
  }

  humidity = 0xFFFF;
  temperature = 0xFFFF;
}

void setup() {
  // Hyt2X1::init(POWER_PIN);
  Hyt2X1::hyt_initRead(0x28);
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(50000L);
  //Hyt2X1::changeAddress(POWER_PIN, 0x29, 0x28);
}

void loop() {
  test_read_ht(0x28);
  delay(1000);
  // test_read_ht(0x29);
  // delay(1000);
}
