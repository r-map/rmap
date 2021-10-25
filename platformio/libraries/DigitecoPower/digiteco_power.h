/**@file digiteco_power.h */

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

#ifndef _DIGITECO_POWER_H
#define _DIGITECO_POWER_H

#include <Arduino.h>
#include <Wire.h>

/*!
\def DIGITECO_POWER_DEFAULT_ADDRESS
\brief I2C address.
*/
#define DIGITECO_POWER_DEFAULT_ADDRESS             (0x30)

/*!
\def DIGITECO_POWER_READ_DATA_LENGTH
\brief Read data length in numbers of bytes.
*/
#define DIGITECO_POWER_READ_DATA_LENGTH            (4)

/*!
\def DIGITECO_POWER_INPUT_VOLTAGE_ADDRESS
\brief I2C input voltage read address.
*/
#define DIGITECO_POWER_INPUT_VOLTAGE_ADDRESS       (0)

/*!
\def DIGITECO_POWER_INPUT_CURRENT_ADDRESS
\brief I2C input current read address.
*/
#define DIGITECO_POWER_INPUT_CURRENT_ADDRESS       (1)

/*!
\def DIGITECO_POWER_BATTERY_VOLTAGE_ADDRESS
\brief I2C input battery voltage read address.
*/
#define DIGITECO_POWER_BATTERY_VOLTAGE_ADDRESS     (2)

/*!
\def DIGITECO_POWER_BATTERY_CURRENT_ADDRESS
\brief I2C input battery current read address.
*/
#define DIGITECO_POWER_BATTERY_CURRENT_ADDRESS     (3)

/*!
\def DIGITECO_POWER_BATTERY_CHARGE_ADDRESS
\brief I2C input battery charge percentage read address.
*/
#define DIGITECO_POWER_BATTERY_CHARGE_ADDRESS   (4)

/*!
\def DIGITECO_POWER_OUTPUT_VOLTAGE_ADDRESS
\brief I2C output voltage read address.
*/
#define DIGITECO_POWER_OUTPUT_VOLTAGE_ADDRESS      (5)

/*!
\namespace DigitecoPower
\brief DigitecoPower namespace.
*/
namespace DigitecoPower {
   /*!
   \fn bool de_read(uint8_t address, float *value)
   \brief Read value at specified i2c-address.
   \param[in] address i2c-address.
   \param[out] *value pointer to readed value.
   \return true if success, false if not.
   */
   bool de_read(uint8_t address, float *value);

   /*!
   \fn bool de_send(uint8_t address, uint8_t data)
   \brief Send data at specified i2c-address.
   \param[in] address i2c-address.
   \param[in] data value (0-5) relative to 6 measurement of voltage and current.
   \return true if success, false if not.
   */
   bool de_send(uint8_t address, uint8_t data);
};

#endif
