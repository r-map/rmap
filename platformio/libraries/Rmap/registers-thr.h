/**@file registers-th.h */

/*********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>
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

#ifndef _REGISTERS_THR_H
#define _REGISTERS_THR_H

#include "registers.h"

/*!
\def I2C_THR_DEFAULT_ADDRESS
\brief Default address for i2c-th module.
*/
#define I2C_THR_DEFAULT_ADDRESS                  (0x23)

/*!
\def I2C_THR_TEMPERATURE_DEFAULT_ADDRESS
\brief Default address for temperature sensor for i2c-th module.
*/
#define I2C_THR_TEMPERATURE_DEFAULT_ADDRESS      (0x49)

/*!
\def I2C_THR_HUMIDITY_DEFAULT_ADDRESS
\brief Default address for humidity sensor for i2c-th module.
*/
#define I2C_THR_HUMIDITY_DEFAULT_ADDRESS         (0x27)

/*!
\def I2C_THR_COMMAND_SAVE
\brief Save command for i2c-th module.
*/
#define I2C_THR_COMMAND_SAVE                     (0x01)

/*!
\def I2C_THR_COMMAND_ONESHOT_START
\brief Oneshot start command for i2c-th module.
*/
#define I2C_THR_COMMAND_ONESHOT_START            (0x02)

/*!
\def I2C_THR_COMMAND_ONESHOT_STOP
\brief Oneshot stop command for i2c-th module.
*/
#define I2C_THR_COMMAND_ONESHOT_STOP             (0x03)

/*!
\def I2C_THR_COMMAND_ONESHOT_START_STOP
\brief Oneshot start-stop command for i2c-th module.
*/
#define I2C_THR_COMMAND_ONESHOT_START_STOP       (0x04)

/*!
\def I2C_THR_COMMAND_CONTINUOUS_START
\brief Continuous start command for i2c-th module.
*/
#define I2C_THR_COMMAND_CONTINUOUS_START         (0x05)

/*!
\def I2C_THR_COMMAND_CONTINUOUS_STOP
\brief Continuous stop command for i2c-th module.
*/
#define I2C_THR_COMMAND_CONTINUOUS_STOP          (0x06)

/*!
\def I2C_THR_COMMAND_CONTINUOUS_START_STOP
\brief Continuous start-stop command for i2c-th module.
*/
#define I2C_THR_COMMAND_CONTINUOUS_START_STOP    (0x07)

/*!
\def I2C_THR_COMMAND_READ_TEST
\brief Continuous start-stop command for i2c-th module.
*/
#define I2C_THR_COMMAND_TEST_READ                (0x08)

/*********************************************************************
* Readable registers: Specifying the length in bytes of the data by I2C_{MODULE_NAME}_{DATA_NAME}_LENGTH, the corresponding address is calculated automatically
*********************************************************************/
/*!
\def I2C_THR_TYPE_LENGTH
\brief length of the type variable for i2c-th module.
*/
#define I2C_THR_TYPE_LENGTH                     (0x01)

/*!
\def I2C_THR_TYPE_ADDRESS
\brief address of the type variable for i2c-th module.
*/
#define I2C_THR_TYPE_ADDRESS                    (I2C_READ_REGISTER_START_ADDRESS)

/*!
\def I2C_THR_MAIN_VERSION_LENGTH
\brief length of the version variable for i2c-th module.
*/
#define I2C_THR_MAIN_VERSION_LENGTH             (0x01)

/*!
\def I2C_THR_MAIN_VERSION_ADDRESS
\brief address of the version variable for i2c-th module.
*/
#define I2C_THR_MAIN_VERSION_ADDRESS            (I2C_THR_TYPE_ADDRESS + I2C_THR_TYPE_LENGTH)

/*!
\def I2C_THR_MINOR_VERSION_LENGTH
\brief length of the version variable for i2c-th module.
*/
#define I2C_THR_MINOR_VERSION_LENGTH            (0x01)

/*!
\def I2C_THR_MINOR_VERSION_ADDRESS
\brief address of the version variable for i2c-th module.
*/
#define I2C_THR_MINOR_VERSION_ADDRESS           (I2C_THR_MAIN_VERSION_ADDRESS + I2C_THR_MAIN_VERSION_LENGTH)

/*!
\def I2C_THR_RAIN_TIPS_LENGTH
\brief length of the temperature sample variable for i2c-th module.
*/
#define I2C_THR_RAIN_TIPS_LENGTH                (0x04)

/*!
\def I2C_THR_RAIN_TIPS_ADDRESS
\brief address of the temperature sample variable for i2c-th module.
*/
#define I2C_THR_RAIN_TIPS_ADDRESS               (I2C_THR_MINOR_VERSION_ADDRESS + I2C_THR_MINOR_VERSION_LENGTH)

/*!
\def I2C_THR_RAIN_LENGTH
\brief length of the temperature sample variable for i2c-th module.
*/
#define I2C_THR_RAIN_LENGTH                     (0x04)

/*!
\def I2C_THR_RAIN_ADDRESS
\brief address of the temperature sample variable for i2c-th module.
*/
#define I2C_THR_RAIN_ADDRESS                    (I2C_THR_RAIN_TIPS_ADDRESS + I2C_THR_RAIN_TIPS_LENGTH)

/*!
\def I2C_THR_SAMPLE_LENGTH
\brief length of the temperature sample variable for i2c-th module.
*/
#define I2C_THR_SAMPLE_LENGTH                   (2 * 0x04)

/*!
\def I2C_THR_SAMPLE_ADDRESS
\brief address of the temperature sample variable for i2c-th module.
*/
#define I2C_THR_SAMPLE_ADDRESS                  (I2C_THR_RAIN_ADDRESS + I2C_THR_RAIN_LENGTH)

/*!
\def I2C_THR_IST_LENGTH
\brief length of the temperature observation variable for i2c-th module.
*/
#define I2C_THR_IST_LENGTH                      (2 * 0x04)

/*!
\def I2C_THR_IST_ADDRESS
\brief address of the temperature observation variable for i2c-th module.
*/
#define I2C_THR_IST_ADDRESS                     (I2C_THR_SAMPLE_ADDRESS + I2C_THR_SAMPLE_LENGTH)

/*!
\def I2C_THR_MIN_LENGTH
\brief length of the minimum temperature variable for i2c-th module.
*/
#define I2C_THR_MIN_LENGTH                      (2 * 0x04)

/*!
\def I2C_THR_MIN_ADDRESS
\brief address of the minimum temperature variable for i2c-th module.
*/
#define I2C_THR_MIN_ADDRESS                     (I2C_THR_IST_ADDRESS + I2C_THR_IST_LENGTH)

/*!
\def I2C_THR_AVG_LENGTH
\brief length of the average temperature variable for i2c-th module.
*/
#define I2C_THR_AVG_LENGTH                      (2 * 0x04)

/*!
\def I2C_THR_AVG_ADDRESS
\brief address of the average temperature variable for i2c-th module.
*/
#define I2C_THR_AVG_ADDRESS                     (I2C_THR_MIN_ADDRESS + I2C_THR_MIN_LENGTH)

/*!
\def I2C_THR_MAX_LENGTH
\brief length of the maximum temperature variable for i2c-th module.
*/
#define I2C_THR_MAX_LENGTH                      (2 * 0x04)

/*!
\def I2C_THR_MAX_ADDRESS
\brief address of the maximum temperature variable for i2c-th module.
*/
#define I2C_THR_MAX_ADDRESS                     (I2C_THR_AVG_ADDRESS + I2C_THR_AVG_LENGTH)

/*!
\def I2C_THR_SIGMA_LENGTH
\brief length of the sigma temperature variable for i2c-th module.
*/
#define I2C_THR_SIGMA_LENGTH                    (2 * 0x04)

/*!
\def I2C_THR_SIGMA_ADDRESS
\brief address of the sigma temperature variable for i2c-th module.
*/
#define I2C_THR_SIGMA_ADDRESS                   (I2C_THR_MAX_ADDRESS + I2C_THR_MAX_LENGTH)

/*!
\def I2C_THR_READABLE_DATA_LENGTH
\brief length of the readable variables for i2c-th module. Need to be update with with last 2 define!!!
*/
#define I2C_THR_READABLE_DATA_LENGTH            (I2C_THR_SIGMA_ADDRESS + I2C_THR_SIGMA_LENGTH - I2C_READ_REGISTER_START_ADDRESS)

/*********************************************************************
* Writable registers: Specifying the length in bytes of the data by I2C_{MODULE_NAME}_{DATA_NAME}_LENGTH, the corresponding address is calculated automatically
*********************************************************************/
/*!
\def I2C_THR_ADDRESS_LENGTH
\brief length of the address variable for i2c-th module.
*/
#define I2C_THR_ADDRESS_LENGTH                   (0x01)

/*!
\def I2C_THR_ADDRESS_ADDRESS
\brief address of the address variable for i2c-th module.
*/
#define I2C_THR_ADDRESS_ADDRESS                  (I2C_WRITE_REGISTER_START_ADDRESS)

/*!
\def I2C_THR_ONESHOT_LENGTH
\brief length of the oneshot variable for i2c-th module.
*/
#define I2C_THR_ONESHOT_LENGTH                   (0x01)

/*!
\def I2C_THR_ONESHOT_ADDRESS
\brief address of the oneshot variable for i2c-th module.
*/
#define I2C_THR_ONESHOT_ADDRESS                  (I2C_THR_ADDRESS_ADDRESS + I2C_THR_ADDRESS_LENGTH)

/*!
\def I2C_THR_CONTINUOUS_LENGTH
\brief length of the continuous variable for i2c-th module.
*/
#define I2C_THR_CONTINUOUS_LENGTH                (0x01)

/*!
\def I2C_THR_CONTINUOUS_ADDRESS
\brief address of the continuous variable for i2c-th module.
*/
#define I2C_THR_CONTINUOUS_ADDRESS               (I2C_THR_ONESHOT_ADDRESS + I2C_THR_ONESHOT_LENGTH)

/*!
\def I2C_THR_TEMPERATURE_ADDRESS_LENGTH
\brief length of the temperature address variable for i2c-th module.
*/
#define I2C_THR_TEMPERATURE_ADDRESS_LENGTH       (0x01)

/*!
\def I2C_THR_TEMPERATURE_ADDRESS_ADDRESS
\brief address of the temperature address variable for i2c-th module.
*/
#define I2C_THR_TEMPERATURE_ADDRESS_ADDRESS      (I2C_THR_CONTINUOUS_ADDRESS + I2C_THR_CONTINUOUS_LENGTH)

/*!
\def I2C_THR_HUMIDITY_ADDRESS_LENGTH
\brief length of the humidity address variable for i2c-th module.
*/
#define I2C_THR_HUMIDITY_ADDRESS_LENGTH          (0x01)

/*!
\def I2C_THR_HUMIDITY_ADDRESS_ADDRESS
\brief address of the humidity address variable for i2c-th module.
*/
#define I2C_THR_HUMIDITY_ADDRESS_ADDRESS         (I2C_THR_TEMPERATURE_ADDRESS_ADDRESS + I2C_THR_TEMPERATURE_ADDRESS_LENGTH)

/*!
\def I2C_THR_WRITABLE_DATA_LENGTH
\brief length of the writable variables for i2c-th module.
*/
#define I2C_THR_WRITABLE_DATA_LENGTH             (I2C_THR_HUMIDITY_ADDRESS_ADDRESS + I2C_THR_HUMIDITY_ADDRESS_LENGTH - I2C_WRITE_REGISTER_START_ADDRESS)

// Readable registers errors checking
#if I2C_THR_READ_REGISTERS_LENGTH > I2C_READ_REGISTER_END_ADDRESS
#error "ERROR! Too many readable registers found in TH module!!!"
#endif

// Writeable registers errors checking
#if I2C_THR_WRITE_REGISTERS_LENGTH > I2C_WRITE_REGISTER_END_ADDRESS
#error "ERROR! Too many writable registers found in TH module!!!"
#endif

#endif
