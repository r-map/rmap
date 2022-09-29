/**@file sht.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>

This is based on popular lib "arduino-sht"
<https://github.com/Sensirion/arduino-sht>

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

#include "sht.h"

Sht::Sht(TwoWire *_wire, uint8_t _address, uint16_t _accuracy, uint8_t _power_pin) {
  wire = _wire;
  address = _address;
  accuracy = _accuracy;
  power_pin = _power_pin;
};

uint8_t Sht::getAddress() {
  return address;
}

uint8_t Sht::getAcquisitionDelayMs() {
  uint8_t acq_duration_ms = 0;

  switch (accuracy) {
    case SHT_HIGH_ACCURACY_MODE: acq_duration_ms = SHT_HIGH_ACCURACY_CONVERSION_TIME_MS; break;
    case SHT_MEDIUM_ACCURACY_MODE: acq_duration_ms = SHT_MEDIUM_ACCURACY_CONVERSION_TIME_MS; break;
    case SHT_LOW_ACCURACY_MODE: acq_duration_ms = SHT_LOW_ACCURACY_CONVERSION_TIME_MS; break;
    default: acq_duration_ms = SHT_HIGH_ACCURACY_CONVERSION_TIME_MS; break;
  }

  return acq_duration_ms;
}

bool Sht::prepare() {
  wire->beginTransmission(address);

  if (wire->write(accuracy >> 0x08) != 1) {
    return false;
  }

  if (wire->write(accuracy & 0xFF) != 1) {
    return false;
  }

  return !wire->endTransmission();
}

bool Sht::read(float *humidity, float *temperature) {
  *humidity = FLT_MAX;
  *temperature = FLT_MAX;

  uint8_t data[SHT_READ_HT_DATA_LENGTH];

  wire->requestFrom((uint8_t) address, (uint8_t) SHT_READ_HT_DATA_LENGTH);

  // check if the same number of bytes are received that are requested.
  if (wire->available() != SHT_READ_HT_DATA_LENGTH) {
    return false;
  }

  for (uint8_t i = 0; i < SHT_READ_HT_DATA_LENGTH; ++i) {
    data[i] = wire->read();
  }

  if (crc8(&data[0], 2) != data[2] || crc8(&data[3], 2) != data[5]) {
    return false;
  }

  // convert to Temperature/Humidity
  uint16_t val;
  val = (data[0] << 8) + data[1];
  *temperature = -45.0 + 175.0 * (val / 65535.0);

  val = (data[3] << 8) + data[4];
  *humidity = 100.0 * (val / 65535.0);

  return true;
}

uint8_t Sht::crc8(const uint8_t *data, uint8_t len) {
  // adapted from SHT21 sample code from
  // http://www.sensirion.com/en/products/humidity-temperature/download-center/

  uint8_t crc = 0xFF;
  uint8_t byteCtr;
  for (byteCtr = 0; byteCtr < len; ++byteCtr) {
    crc ^= data[byteCtr];
    for (uint8_t bit = 8; bit > 0; --bit) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ 0x31;
      } else {
        crc = (crc << 1);
      }
    }
  }
  return crc;
}

void Sht::initPin() {
  pinMode(power_pin, OUTPUT);
}

void Sht::powerOn() {
  digitalWrite(power_pin, HIGH);
}

void Sht::powerOff() {
  digitalWrite(power_pin, LOW);
}
