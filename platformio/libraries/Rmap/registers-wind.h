/**@file registers-wind.h */

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

#ifndef _REGISTERS_WIND_H
#define _REGISTERS_WIND_H

#include "registers.h"

/*!
\def I2C_WIND_DEFAULT_ADDRESS
\brief Default address for i2c-wind module.
*/
#define I2C_WIND_DEFAULT_ADDRESS                  (0x45)

/*!
\def I2C_TH_COMMAND_NONE
\brief NO command for i2c-th module.
*/
#define I2C_WIND_COMMAND_NONE                     (0x00)

/*!
\def I2C_WIND_COMMAND_SAVE
\brief Save command for i2c-wind module.
*/
#define I2C_WIND_COMMAND_SAVE                     (0x01)

/*!
\def I2C_WIND_COMMAND_ONESHOT_START
\brief Oneshot start command for i2c-wind module.
*/
#define I2C_WIND_COMMAND_ONESHOT_START            (0x02)

/*!
\def I2C_WIND_COMMAND_ONESHOT_STOP
\brief Oneshot stop command for i2c-wind module.
*/
#define I2C_WIND_COMMAND_ONESHOT_STOP             (0x03)

/*!
\def I2C_WIND_COMMAND_ONESHOT_START_STOP
\brief Oneshot start-stop command for i2c-wind module.
*/
#define I2C_WIND_COMMAND_ONESHOT_START_STOP       (0x04)

/*!
\def I2C_WIND_COMMAND_CONTINUOUS_START
\brief Continuous start command for i2c-wind module.
*/
#define I2C_WIND_COMMAND_CONTINUOUS_START         (0x05)

/*!
\def I2C_WIND_COMMAND_CONTINUOUS_STOP
\brief Continuous stop command for i2c-wind module.
*/
#define I2C_WIND_COMMAND_CONTINUOUS_STOP          (0x06)

/*!
\def I2C_WIND_COMMAND_CONTINUOUS_START_STOP
\brief Continuous start-stop command for i2c-wind module.
*/
#define I2C_WIND_COMMAND_CONTINUOUS_START_STOP    (0x07)

/*!
\def I2C_WIND_COMMAND_TEST_READ
\brief test command for i2c-wind module.
*/
#define I2C_WIND_COMMAND_TEST_READ                (0x08)

/*********************************************************************
* Readable registers: Specifying the length in bytes of the data by I2C_{MODULE_NAME}_{DATA_NAME}_LENGTH, the corresponding address is calculated automatically
*********************************************************************/
/*!
\def I2C_WIND_TYPE_LENGTH
\brief length of the type variable for i2c-wind module.
*/
#define I2C_WIND_TYPE_LENGTH                      (0x01)

/*!
\def I2C_WIND_TYPE_ADDRESS
\brief address of the type variable for i2c-wind module.
*/
#define I2C_WIND_TYPE_ADDRESS                     (I2C_READ_REGISTER_START_ADDRESS)

/*!
\def I2C_WIND_MAIN_VERSION_LENGTH
\brief length of the main version variable for i2c-wind module.
*/
#define I2C_WIND_MAIN_VERSION_LENGTH                   (0x01)

/*!
\def I2C_WIND_MAIN_VERSION_ADDRESS
\brief address of the main version variable for i2c-wind module.
*/
#define I2C_WIND_MAIN_VERSION_ADDRESS                  (I2C_WIND_TYPE_ADDRESS + I2C_WIND_TYPE_LENGTH)

/*!
\def I2C_WIND_MINOR_VERSION_LENGTH
\brief length of the minor version variable for i2c-wind module.
*/
#define I2C_WIND_MINOR_VERSION_LENGTH                   (0x01)

/*!
\def I2C_WIND_MINOR_VERSION_ADDRESS
\brief address of the minor version variable for i2c-wind module.
*/
#define I2C_WIND_MINOR_VERSION_ADDRESS                  (I2C_WIND_MAIN_VERSION_ADDRESS + I2C_WIND_MAIN_VERSION_LENGTH)

/*!
\def I2C_WIND_SAMPLE_LENGTH
\brief length of the speed and direction SAMPLE variables for i2c-wind module.
*/
#define I2C_WIND_SAMPLE_LENGTH                    (2 * 0x02)

/*!
\def I2C_WIND_SAMPLE_ADDRESS
\brief address  of the speed and direction SAMPLE variables for i2c-wind module.
*/
#define I2C_WIND_SAMPLE_ADDRESS                   (I2C_WIND_MINOR_VERSION_ADDRESS + I2C_WIND_MINOR_VERSION_LENGTH)

/*!
\def I2C_WIND_VAVG10_LENGTH
\brief length of the speed and direction WIND_VAVG10 variables for i2c-wind module.
*/
#define I2C_WIND_VAVG10_LENGTH                    (2 * 0x02)

/*!
\def I2C_WIND_VAVG10_ADDRESS
\brief address  of the speed and direction WIND_VAVG10 variables for i2c-wind module.
*/
#define I2C_WIND_VAVG10_ADDRESS                   (I2C_WIND_SAMPLE_ADDRESS + I2C_WIND_SAMPLE_LENGTH)

/*!
\def I2C_WIND_VAVG_LENGTH
\brief  length of the speed and direction WIND_VAVG10 variables for i2c-wind module.
*/
#define I2C_WIND_VAVG_LENGTH                      (2 * 0x02)

/*!
\def I2C_WIND_VAVG_ADDRESS
\brief address  of the speed and direction WIND_VAVG variables for i2c-wind module.
*/
#define I2C_WIND_VAVG_ADDRESS                     (I2C_WIND_VAVG10_ADDRESS + I2C_WIND_VAVG10_LENGTH)

/*!
\def I2C_WIND_GUST_SPEED_LENGTH
\brief length peak and long gust speed  variables for i2c-wind module.
*/
#define I2C_WIND_GUST_SPEED_LENGTH                      (2 * 0x02)

/*!
\def I2C_WIND_GUST_SPEED_ADDRESS
\brief address peak and long gust speed  variables for i2c-wind module.
*/
#define I2C_WIND_GUST_SPEED_ADDRESS                     (I2C_WIND_VAVG_ADDRESS + I2C_WIND_VAVG_LENGTH)

/*!
\def I2C_WIND_SPEED_LENGTH
\brief length of the average speed for i2c-wind module.
*/
#define I2C_WIND_SPEED_LENGTH                     (0x02)

/*!
\def I2C_WIND_SPEED_ADDRESS
\brief address of the average speed variable for i2c-wind module.
*/
#define I2C_WIND_SPEED_ADDRESS                    (I2C_WIND_GUST_SPEED_ADDRESS + I2C_WIND_GUST_SPEED_LENGTH)

/*!
\def I2C_WIND_CLASS_LENGTH
\brief length of the wind class for i2c-wind module.
*/
#define I2C_WIND_CLASS_LENGTH                     (6 * 0x01)

/*!
\def I2C_WIND_CLASS_ADDRESS
\brief address of the wind class variables for i2c-wind module.
*/
#define I2C_WIND_CLASS_ADDRESS                    (I2C_WIND_SPEED_ADDRESS + I2C_WIND_SPEED_LENGTH)

/*!
\def I2C_WIND_GUST_DIRECTION_LENGTH
\brief length of the version variable for i2c-wind module.
*/
#define I2C_WIND_GUST_DIRECTION_LENGTH                     (2 * 0x02)

/*!
\def I2C_WIND_GUST_DIRECTION_ADDRESS
\brief address of the version variable for i2c-wind module.
*/
#define I2C_WIND_GUST_DIRECTION_ADDRESS                    (I2C_WIND_CLASS_ADDRESS + I2C_WIND_CLASS_LENGTH)

/*!
\def I2C_WIND_READABLE_DATA_LENGTH
\brief length of the readable variables for i2c-wind module. Need to be update with with last 2 define!!!
*/
#define I2C_WIND_READABLE_DATA_LENGTH             (I2C_WIND_CLASS_ADDRESS + I2C_WIND_CLASS_LENGTH - I2C_READ_REGISTER_START_ADDRESS)

/*********************************************************************
* Writable registers: Specifying the length in bytes of the data by I2C_{MODULE_NAME}_{DATA_NAME}_LENGTH, the corresponding address is calculated automatically
*********************************************************************/
/*!
\def I2C_WIND_ADDRESS_LENGTH
\brief length of the address variable for i2c-wind module.
*/
#define I2C_WIND_ADDRESS_LENGTH                   (0x01)

/*!
\def I2C_WIND_ADDRESS_ADDRESS
\brief address of the address variable for i2c-wind module.
*/
#define I2C_WIND_ADDRESS_ADDRESS                  (I2C_WRITE_REGISTER_START_ADDRESS)

/*!
\def I2C_WIND_ONESHOT_LENGTH
\brief length of the oneshot variable for i2c-wind module.
*/
#define I2C_WIND_ONESHOT_LENGTH                   (0x01)

/*!
\def I2C_WIND_ONESHOT_ADDRESS
\brief address of the oneshot variable for i2c-wind module.
*/
#define I2C_WIND_ONESHOT_ADDRESS                  (I2C_WIND_ADDRESS_ADDRESS + I2C_WIND_ADDRESS_LENGTH)

/*!
\def I2C_WIND_WRITABLE_DATA_LENGTH
\brief length of the writable variables for i2c-wind module.
*/
#define I2C_WIND_WRITABLE_DATA_LENGTH             (I2C_WIND_ONESHOT_ADDRESS + I2C_WIND_ONESHOT_LENGTH - I2C_WRITE_REGISTER_START_ADDRESS)

// Readable registers errors checking
#if I2C_WIND_READ_REGISTERS_LENGTH > I2C_READ_REGISTER_LEN
#error "ERROR! Too many readable registers found in WIND module!!!"
#endif

// Writeable registers errors checking
#if I2C_WIND_WRITE_REGISTERS_LENGTH > I2C_WRITE_REGISTER_LEN
#error "ERROR! Too many writable registers found in WIND module!!!"
#endif

#endif
