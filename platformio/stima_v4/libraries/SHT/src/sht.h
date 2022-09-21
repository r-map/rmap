/**@file sht2x1.h */

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

#ifndef _SHT_H
#define _SHT_H

#include <Arduino.h>
#include <Wire.h>
#include <float.h>

/*!
\def SHT_DEFAULT_ADDRESS
\brief Default address.
*/
#define SHT_DEFAULT_ADDRESS       (0x44)
#define SHT_ALTERNATE_ADDRESS     (0x45)

/*!
\def SHT_READ_HT_DATA_LENGTH
\brief Number of bytes to be read.
*/
#define SHT_READ_HT_DATA_LENGTH   (6)

#define SHT_WRITE_DATA_LENGTH     (2)

/*!
\def SHT_CONVERSION_TIME_MS
\brief Conversion time in milliseconds.
*/
#define SHT_HIGH_ACCURACY_CONVERSION_TIME_MS    (15)
#define SHT_MEDIUM_ACCURACY_CONVERSION_TIME_MS  (6)
#define SHT_LOW_ACCURACY_CONVERSION_TIME_MS     (4)

#define SHT_HIGH_ACCURACY_MODE                  (0x2400)
#define SHT_MEDIUM_ACCURACY_MODE                (0x240b)
#define SHT_LOW_ACCURACY_MODE                   (0x2416)

/*!
\def SHT_TEMPERATURE_MIN
\brief Minimum temperature.
*/
#define SHT_TEMPERATURE_MIN       (-40)

/*!
\def SHT_TEMPERATURE_MAX
\brief Maximum temperature.
*/
#define SHT_TEMPERATURE_MAX       (125)

/*!
\def SHT_HUMIDITY_MIN
\brief Minimum humidity.
*/
#define SHT_HUMIDITY_MIN          (0)

/*!
\def SHT_HUMIDITY_MAX
\brief Maximum humidity.
*/
#define SHT_HUMIDITY_MAX          (100)

class Sht {
  public:
    Sht(TwoWire *_wire = &Wire, uint8_t _address = SHT_DEFAULT_ADDRESS, uint16_t _accuracy = SHT_HIGH_ACCURACY_MODE, uint8_t _power_pin = 0);
    uint8_t getAddress();
    uint8_t getAcquisitionDelayMs();
    bool prepare();
    bool read(float *humidity, float *temperature);
    void initPin();
    void powerOn();
    void powerOff();

  protected:

  private:
    TwoWire *wire;
    uint8_t address;
    uint16_t accuracy;
    uint8_t power_pin;

    uint8_t crc8(const uint8_t *data, uint8_t len);
};


#endif
