/**@file hyt2x1.cpp */

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

#include "hyt2x1.h"

namespace Hyt2X1 {
  bool hyt_initRead(uint8_t address) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission()) {
      return false;
    }
    return true;
  }

  uint8_t hyt_read(int8_t address, float *humidity, float *temperature) {
    uint16_t humidity_raw_data = UINT16_MAX;
    uint16_t temperature_raw_data = UINT16_MAX;
    bool is_new_data = true;

    *humidity = UINT16_MAX;
    *temperature = UINT16_MAX;

    //! Request 4 bytes: 2 bytes for Humidity and 2 bytes for Temperature
    Wire.requestFrom(address, HYT2X1_READ_HT_DATA_LENGTH);

    //! not enough data
    if (Wire.available() < HYT2X1_READ_HT_DATA_LENGTH) {
      LOGE(F("hyt_read %d < %d"),Wire.available(),HYT2X1_READ_HT_DATA_LENGTH);
      return HYT2X1_ERROR;
    }

    humidity_raw_data = (((uint16_t) Wire.read()) << 8) | ((uint16_t) Wire.read());
    temperature_raw_data = (((uint16_t) Wire.read()) << 8) | ((uint16_t) Wire.read());

    //! command mode
    if ((humidity_raw_data & HYT2X1_COMMAND_MODE_BIT_MASK) >> 15) {
      LOGE(F("hyt_read command mode error"));
      return HYT2X1_ERROR;
    }

    //! no new data
    if ((humidity_raw_data & HYT2X1_NO_NEW_DATA_BIT_MASK) >> 14) {
      LOGT(F("hyt_read no new data"));
      is_new_data = false;
    }

    //! data not valid
    if (is_new_data && (humidity_raw_data > HYT2X1_READ_MAX)) {
      LOGE(F("hyt_read data not valid error"));
      return HYT2X1_ERROR;
    }

    humidity_raw_data &= HYT2X1_HUMIDITY_MASK;
    temperature_raw_data &= HYT2X1_TEMPERATURE_MASK;
    temperature_raw_data >>= 2;

    *humidity = (100.0 / HYT2X1_READ_MAX) * humidity_raw_data;
    *temperature = ((165.0 / HYT2X1_READ_MAX) * temperature_raw_data) + HYT2X1_TEMPERATURE_MIN;

    if (is_new_data) {
      return HYT2X1_SUCCESS;
    }
    else {
      return HYT2X1_NO_NEW_DATA;
    }
  }

  void hyt_send(int8_t address, uint8_t data_0, uint8_t data_1, uint8_t data_2) {
    Wire.beginTransmission(address);
    Wire.write(data_0);
    Wire.write(data_1);
    Wire.write(data_2);
    Wire.endTransmission();
  }

  void hyt_changeAddress(uint8_t power_pin, int8_t address, int8_t new_address) {
    hyt_off(power_pin);
    hyt_on(power_pin);
    hyt_send(address, HYT2X1_ENTER_COMMAND_MODE, 0x00, 0x00);
    hyt_send(address, HYT2X1_WRITE_ADDRESS, 0x00, new_address);
    hyt_send(address, HYT2X1_EXIT_COMMAND_MODE, 0x00, 0x00);
  }

  void hyt_init(uint8_t power_pin) {
    pinMode(power_pin, OUTPUT);
  }

  void hyt_on(uint8_t power_pin) {
    digitalWrite(power_pin, HIGH);
  }

  void hyt_off(uint8_t power_pin) {
    digitalWrite(power_pin, LOW);
  }
}
