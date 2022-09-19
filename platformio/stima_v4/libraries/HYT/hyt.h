/**@file hyt2x1.h */

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

#ifndef _HYT_H
#define _HYT_H

#include <Arduino.h>
#include <Wire.h>

/*!
\def HYT_DEFAULT_ADDRESS
\brief Default address.
*/
#define HYT_DEFAULT_ADDRESS       (0x28)

/*!
\def HYT_READ_HT_DATA_LENGTH
\brief Number of bytes to be read.
*/
#define HYT_READ_HT_DATA_LENGTH   (4)

/*!
\def HYT_ENTER_COMMAND_MODE
\brief Command mode.
*/
#define HYT_ENTER_COMMAND_MODE    (0xA0)

/*!
\def HYT_EXIT_COMMAND_MODE
\brief Exit command mode.
*/
#define HYT_EXIT_COMMAND_MODE     (0x80)

/*!
\def HYT_WRITE_ADDRESS
\brief Write address.
*/
#define HYT_WRITE_ADDRESS         (0x5C)

/*!
\def HYT_CONVERSION_TIME_MS
\brief Conversion time in milliseconds.
*/
#define HYT_CONVERSION_TIME_MS    (100)

/*!
\def HYT_TEMPERATURE_MIN
\brief Minimum temperature.
*/
#define HYT_TEMPERATURE_MIN       (-40)

/*!
\def HYT_TEMPERATURE_MAX
\brief Maximum temperature.
*/
#define HYT_TEMPERATURE_MAX       (125)

/*!
\def HYT_HUMIDITY_MIN
\brief Minimum humidity.
*/
#define HYT_HUMIDITY_MIN          (0)

/*!
\def HYT_HUMIDITY_MAX
\brief Maximum humidity.
*/
#define HYT_HUMIDITY_MAX          (100)

/*!
\def HYT_READ_MAX
\brief Maximum acceptable readable value.
*/
#define HYT_READ_MAX              (0x3FFF)

/*!
\def HYT_COMMAND_MODE_BIT_MASK
\brief Masks for command mode bit status.
*/
#define HYT_COMMAND_MODE_BIT_MASK (0x8000)

/*!
\def HYT_NO_NEW_DATA_BIT_MASK
\brief Masks for no new data bit status.
*/
#define HYT_NO_NEW_DATA_BIT_MASK  (0x4000)

/*!
\def HYT_HUMIDITY_MASK
\brief last 2 MSB bit = 0.
*/
#define HYT_HUMIDITY_MASK         (0x3FFF)

/*!
\def HYT_TEMPERATURE_MASK
\brief first 2 LSB bit = 0.
*/
#define HYT_TEMPERATURE_MASK      (0xFFFC)

/*!
\def HYT_ERROR
\brief returning error value.
*/
#define HYT_ERROR                  (0)

/*!
\def HYT_SUCCESS
\brief returning success value.
*/
#define HYT_SUCCESS                (1)

/*!
\def HYT_NO_NEW_DATA
\brief returning no new data value.
*/
#define HYT_NO_NEW_DATA            (2)

class Hyt {
  public:
    Hyt(uint8_t _address = HYT_DEFAULT_ADDRESS, uint8_t _power_pin = 0);
    uint8_t getAddress();
    bool prepare();
    uint8_t read(float *humidity, float *temperature);
    bool send(uint8_t data_0, uint8_t data_1, uint8_t data_2);
    bool changeAddress(int8_t new_address);
    void initPin();
    void powerOn();
    void powerOff();

  protected:

  private:
    uint8_t address;
    uint8_t power_pin;
};


#endif
