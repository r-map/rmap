/**@file hyt.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
<http://www.gnu.org/licenses/>.
**********************************************************************/

#include "hyt.h"

Hyt::Hyt(TwoWire *_wire, uint8_t _address, uint8_t _power_pin) {
  wire = _wire;
  address = _address;
  power_pin = _power_pin;
};

uint8_t Hyt::getAddress() {
  return address;
}

uint8_t Hyt::getAcquisitionDelayMs() {
  return HYT_CONVERSION_TIME_MS;
}

bool Hyt::prepare() {
  wire->beginTransmission(address);
  return !wire->endTransmission();
}

uint8_t Hyt::read(float *humidity, float *temperature) {
  uint16_t humidity_raw_data = UINT16_MAX;
  uint16_t temperature_raw_data = UINT16_MAX;
  bool is_new_data = true;

  *humidity = UINT16_MAX;
  *temperature = UINT16_MAX;

  //! Request 4 bytes: 2 bytes for Humidity and 2 bytes for Temperature
  wire->requestFrom(address, (uint8_t) HYT_READ_HT_DATA_LENGTH);

  //! not enough data
  if (wire->available() < HYT_READ_HT_DATA_LENGTH) {
    return HYT_ERROR;
  }

  humidity_raw_data = (((uint16_t) wire->read()) << 8) | ((uint16_t) wire->read());
  temperature_raw_data = (((uint16_t) wire->read()) << 8) | ((uint16_t) wire->read());

  //! command mode
  if ((humidity_raw_data & HYT_COMMAND_MODE_BIT_MASK) >> 15) {
    return HYT_ERROR;
  }

  //! no new data
  if ((humidity_raw_data & HYT_NO_NEW_DATA_BIT_MASK) >> 14) {
    is_new_data = false;
  }

  //! data not valid
  if (is_new_data && (humidity_raw_data > HYT_READ_MAX)) {
    return HYT_ERROR;
  }

  humidity_raw_data &= HYT_HUMIDITY_MASK;
  temperature_raw_data &= HYT_TEMPERATURE_MASK;
  temperature_raw_data >>= 2;

  *humidity = (100.0 / HYT_READ_MAX) * humidity_raw_data;
  *temperature = ((165.0 / HYT_READ_MAX) * temperature_raw_data) + HYT_TEMPERATURE_MIN;

  if (is_new_data) {
    return HYT_SUCCESS;
  }
  else {
    return HYT_NO_NEW_DATA;
  }
}

bool Hyt::send(uint8_t data_0, uint8_t data_1, uint8_t data_2) {
  wire->beginTransmission(address);
  wire->write(data_0);
  wire->write(data_1);
  wire->write(data_2);
  return !wire->endTransmission();
}

bool Hyt::changeAddress(int8_t new_address) {
  bool is_ok = true;
  powerOff();
  powerOn();
  is_ok |= send(HYT_ENTER_COMMAND_MODE, 0x00, 0x00);
  is_ok |= send(HYT_WRITE_ADDRESS, 0x00, new_address);
  is_ok |= send(HYT_EXIT_COMMAND_MODE, 0x00, 0x00);
  return is_ok;
}

void Hyt::initPin() {
  pinMode(power_pin, OUTPUT);
}

void Hyt::powerOn() {
  digitalWrite(power_pin, HIGH);
}

void Hyt::powerOff() {
  digitalWrite(power_pin, LOW);
}
