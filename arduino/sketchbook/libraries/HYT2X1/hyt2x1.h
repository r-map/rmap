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

#ifndef _HYT2X1_H
#define _HYT2X1_H

#include "Arduino.h"
#include <Wire.h>

/*!
\def HYT2X1_DEFAULT_ADDRESS
\brief Default address.
*/
#define HYT2X1_DEFAULT_ADDRESS      (0x28)

/*!
\def HYT2X1_READ_HT_DATA_LENGTH
\brief Number of bytes to be read.
*/
#define HYT2X1_READ_HT_DATA_LENGTH  (4)

/*!
\def HYT2X1_ENTER_COMMAND_MODE
\brief Command mode.
*/
#define HYT2X1_ENTER_COMMAND_MODE   (0xA0)

/*!
\def HYT2X1_EXIT_COMMAND_MODE
\brief Exit command mode.
*/
#define HYT2X1_EXIT_COMMAND_MODE    (0x80)

/*!
\def HYT2X1_WRITE_ADDRESS
\brief Write address.
*/
#define HYT2X1_WRITE_ADDRESS        (0x5C)

/*!
\def HYT2X1_CONVERSION_TIME_MS
\brief Conversion time in milliseconds.
*/
#define HYT2X1_CONVERSION_TIME_MS   (100)

/*!
\def HYT2X1_TEMPERATURE_MIN
\brief Minimum temperature.
*/
#define HYT2X1_TEMPERATURE_MIN      (-40)

/*!
\def HYT2X1_TEMPERATURE_MAX
\brief Maximum temperature.
*/
#define HYT2X1_TEMPERATURE_MAX      (125)

/*!
\def HYT2X1_HUMIDITY_MIN
\brief Minimum humidity.
*/
#define HYT2X1_HUMIDITY_MIN         (0)

/*!
\def HYT2X1_HUMIDITY_MAX
\brief Maximum humidity.
*/
#define HYT2X1_HUMIDITY_MAX         (100)

/*!
\namespace Hyt2X1
\brief HYT2X1 namespace.
*/
namespace Hyt2X1 {
  /*!
  \fn void hyt_init(uint8_t power_pin)
  \brief Init sensor.
  \param[in] power_pin sensors power pin.
  \return void.
  */
  void hyt_init(uint8_t power_pin);

  /*!
  \fn void hyt_on(uint8_t power_pin)
  \brief Power on sensor.
  \param[in] power_pin sensors power pin.
  \return void.
  */
  void hyt_on(uint8_t power_pin);

  /*!
  \fn void hyt_off(uint8_t power_pin)
  \brief Power off sensor.
  \param[in] power_pin sensors power pin.
  \return void.
  */
  void hyt_off(uint8_t power_pin);

  /*!
  \fn void hyt_changeAddress(uint8_t power_pin, int8_t address, int8_t new_address)
  \brief Change sensor address.
  \param[in] power_pin sensors power pin.
  \param[in] address sensors i2c address.
  \param[in] new_address sensors i2c new address.
  \return void.
  */
  void hyt_changeAddress(uint8_t power_pin, int8_t address, int8_t new_address);

  /*!
  \fn bool hyt_initRead(uint8_t address)
  \brief Init sensor read.
  \param[in] address sensors i2c address.
  \return true if success.
  */
  bool hyt_initRead(uint8_t address);

  /*!
  \fn bool hyt_read(int8_t address, float *humidity, float *temperature)
  \brief Returns the humidty and temperature from hyt2X1 sensor at specified address.
  \param[in] address sensor i2c address.
  \param[out] *humidity pointer to readed humidity variable.
  \param[out] *temperature pointer to readed temperature variable.
  \return true if success.
  */
  bool hyt_read(int8_t address, float *humidity, float *temperature);

  /*!
  \fn void hyt_send(int8_t address, uint8_t data_0, uint8_t data_1, uint8_t data_2)
  \brief Send sensor command.
  \param[in] address sensor i2c address.
  \param[in] data_0 first byte data.
  \param[in] data_1 second byte data.
  \param[in] data_2 third byte data.
  \return void.
  */
  void hyt_send(int8_t address, uint8_t data_0, uint8_t data_1, uint8_t data_2);
};

#endif
