/**@file digiteco_power.cpp */

/*********************************************************************
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

#include "digiteco_power.h"

namespace DigitecoPower {
   bool de_read(uint8_t address, float *value) {
      uint8_t buffer[DIGITECO_POWER_READ_DATA_LENGTH];
      uint8_t i = 0;

      Wire.requestFrom(address, (uint8_t) DIGITECO_POWER_READ_DATA_LENGTH);

      if (Wire.available() < DIGITECO_POWER_READ_DATA_LENGTH) {
         return false;
      }

      while (Wire.available() && i < DIGITECO_POWER_READ_DATA_LENGTH) {
         buffer[i++] = Wire.read();
      }

      if (Wire.endTransmission()) {
         return false;
      }

      memcpy(value, (float *) buffer, sizeof (float));
      return true;
   }

   bool de_send(uint8_t address, uint8_t data) {
      Wire.beginTransmission(address);
      Wire.write(data);
      return (Wire.endTransmission() == false);
   }
}
