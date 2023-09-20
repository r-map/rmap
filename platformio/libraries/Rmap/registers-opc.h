/**@file registers-opc.h */

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

#ifndef _REGISTERS_OPC_H
#define _REGISTERS_OPC_H

#include "registers.h"

/*!
\def I2C_OPC_DEFAULT_ADDRESS
\brief Default address for i2c-opc module.
*/
#define I2C_OPC_DEFAULT_ADDRESS                  (0x55)

/*!
\def I2C_OPC_COMMAND_SAVE
\brief Save command for i2c-opc module.
*/
#define I2C_OPC_COMMAND_SAVE                     (0x01)

/*!
\def I2C_OPC_COMMAND_ONESHOT_START
\brief Oneshot start command for i2c-opc module.
*/
#define I2C_OPC_COMMAND_ONESHOT_START            (0x02)

/*!
\def I2C_OPC_COMMAND_ONESHOT_STOP
\brief Oneshot stop command for i2c-opc module.
*/
#define I2C_OPC_COMMAND_ONESHOT_STOP             (0x03)

/*!
\def I2C_OPC_COMMAND_ONESHOT_START_STOP
\brief Oneshot start-stop command for i2c-opc module.
*/
#define I2C_OPC_COMMAND_ONESHOT_START_STOP       (0x04)

/*!
\def I2C_OPC_COMMAND_CONTINUOUS_START
\brief Continuous start command for i2c-opc module.
*/
#define I2C_OPC_COMMAND_CONTINUOUS_START         (0x05)

/*!
\def I2C_OPC_COMMAND_CONTINUOUS_STOP
\brief Continuous stop command for i2c-opc module.
*/
#define I2C_OPC_COMMAND_CONTINUOUS_STOP          (0x06)

/*!
\def I2C_OPC_COMMAND_CONTINUOUS_START_STOP
\brief Continuous start-stop command for i2c-opc module.
*/
#define I2C_OPC_COMMAND_CONTINUOUS_START_STOP    (0x07)

/*********************************************************************
* Readable registers: Specifying the length in bytes of the data by I2C_{MODULE_NAME}_{DATA_NAME}_LENGTH, the corresponding address is calculated automatically
*********************************************************************/
/*!
\def I2C_OPC_TYPE_LENGTH
\brief length of the type variable for i2c-opc module.
*/
#define I2C_OPC_TYPE_LENGTH                       (0x01)

/*!
\def I2C_OPC_TYPE_ADDRESS
\brief address of the type variable for i2c-opc module.
*/
#define I2C_OPC_TYPE_ADDRESS                      (I2C_READ_REGISTER_START_ADDRESS)

/*!
\def I2C_OPC_VERSION_LENGTH
\brief length of the version variable for i2c-opc module.
*/
#define I2C_OPC_VERSION_LENGTH                    (0x01)

/*!
\def I2C_OPC_VERSION_ADDRESS
\brief address of the version variable for i2c-opc module.
*/
#define I2C_OPC_VERSION_ADDRESS                   (I2C_OPC_TYPE_ADDRESS + I2C_OPC_TYPE_LENGTH)

/*!
\def I2C_OPC_PM_MED_LENGTH
\brief length of the PM average variable for i2c-opc module.
*/
#define I2C_OPC_PM_MED_LENGTH                     (3 * 0x04)

/*!
\def I2C_OPC_PM_MED_ADDRESS
\brief address of the PM average variable for i2c-opc module.
*/
#define I2C_OPC_PM_MED_ADDRESS                    (I2C_OPC_VERSION_ADDRESS + I2C_OPC_VERSION_LENGTH)

/*!
\def I2C_OPC_PM_SIGMA_LENGTH
\brief length of the PM sigma variable for i2c-opc module.
*/
#define I2C_OPC_PM_SIGMA_LENGTH                   (3 * 0x04)

/*!
\def I2C_OPC_PM_SIGMA_ADDRESS
\brief address of the PM sigma variable for i2c-opc module.
*/
#define I2C_OPC_PM_SIGMA_ADDRESS                  (I2C_OPC_PM_MED_ADDRESS + I2C_OPC_PM_MED_LENGTH)

/*!
\def I2C_OPC_BIN_0_5_MED_LENGTH
\brief length of the bin [0-5] average variable for i2c-opc module.
*/
#define I2C_OPC_BIN_0_5_MED_LENGTH                (6 * 0x04)

/*!
\def I2C_OPC_BIN_0_5_MED_ADDRESS
\brief address of the bin [0-5] average variable for i2c-opc module.
*/
#define I2C_OPC_BIN_0_5_MED_ADDRESS               (I2C_OPC_PM_SIGMA_ADDRESS + I2C_OPC_PM_SIGMA_LENGTH)

/*!
\def I2C_OPC_BIN_6_11_MED_LENGTH
\brief length of the bin [6-11] average variable for i2c-opc module.
*/
#define I2C_OPC_BIN_6_11_MED_LENGTH               (6 * 0x04)

/*!
\def I2C_OPC_BIN_6_11_MED_ADDRESS
\brief address of the bin [6-11] average variable for i2c-opc module.
*/
#define I2C_OPC_BIN_6_11_MED_ADDRESS              (I2C_OPC_BIN_0_5_MED_ADDRESS + I2C_OPC_BIN_0_5_MED_LENGTH)

/*!
\def I2C_OPC_BIN_12_17_MED_LENGTH
\brief length of the bin [12-17] average variable for i2c-opc module.
*/
#define I2C_OPC_BIN_12_17_MED_LENGTH              (6 * 0x04)

/*!
\def I2C_OPC_BIN_12_17_MED_ADDRESS
\brief address of the bin [12-17] average variable for i2c-opc module.
*/
#define I2C_OPC_BIN_12_17_MED_ADDRESS             (I2C_OPC_BIN_6_11_MED_ADDRESS + I2C_OPC_BIN_6_11_MED_LENGTH)

/*!
\def I2C_OPC_BIN_18_23_MED_LENGTH
\brief length of the bin [18-23] average variable for i2c-opc module.
*/
#define I2C_OPC_BIN_18_23_MED_LENGTH              (6 * 0x04)

/*!
\def I2C_OPC_BIN_18_23_MED_ADDRESS
\brief address of the bin [18-23] average variable for i2c-opc module.
*/
#define I2C_OPC_BIN_18_23_MED_ADDRESS             (I2C_OPC_BIN_12_17_MED_ADDRESS + I2C_OPC_BIN_12_17_MED_LENGTH)

/*!
\def I2C_OPC_BIN_0_5_SIGMA_LENGTH
\brief length of the bin [0-5] sigma variable for i2c-opc module.
*/
#define I2C_OPC_BIN_0_5_SIGMA_LENGTH              (6 * 0x04)

/*!
\def I2C_OPC_BIN_0_5_SIGMA_ADDRESS
\brief address of the bin [0-5] sigma variable for i2c-opc module.
*/
#define I2C_OPC_BIN_0_5_SIGMA_ADDRESS             (I2C_OPC_BIN_18_23_MED_ADDRESS + I2C_OPC_BIN_18_23_MED_LENGTH)

/*!
\def I2C_OPC_BIN_6_11_SIGMA_LENGTH
\brief length of the bin [6-11] sigma variable for i2c-opc module.
*/
#define I2C_OPC_BIN_6_11_SIGMA_LENGTH             (6 * 0x04)

/*!
\def I2C_OPC_BIN_6_11_SIGMA_ADDRESS
\brief address of the bin [6-11] sigma variable for i2c-opc module.
*/
#define I2C_OPC_BIN_6_11_SIGMA_ADDRESS            (I2C_OPC_BIN_0_5_SIGMA_ADDRESS + I2C_OPC_BIN_0_5_SIGMA_LENGTH)

/*!
\def I2C_OPC_BIN_12_17_SIGMA_LENGTH
\brief length of the bin [12-17] sigma variable for i2c-opc module.
*/
#define I2C_OPC_BIN_12_17_SIGMA_LENGTH            (6 * 0x04)

/*!
\def I2C_OPC_BIN_12_17_SIGMA_ADDRESS
\brief address of the bin [12-17] sigma variable for i2c-opc module.
*/
#define I2C_OPC_BIN_12_17_SIGMA_ADDRESS           (I2C_OPC_BIN_6_11_SIGMA_ADDRESS + I2C_OPC_BIN_6_11_SIGMA_LENGTH)

/*!
\def I2C_OPC_BIN_18_23_SIGMA_LENGTH
\brief length of the bin [18-23] sigma variable for i2c-opc module.
*/
#define I2C_OPC_BIN_18_23_SIGMA_LENGTH            (6 * 0x04)

/*!
\def I2C_OPC_BIN_18_23_SIGMA_ADDRESS
\brief address of the bin [18-23] sigma variable for i2c-opc module.
*/
#define I2C_OPC_BIN_18_23_SIGMA_ADDRESS           (I2C_OPC_BIN_12_17_SIGMA_ADDRESS + I2C_OPC_BIN_12_17_SIGMA_LENGTH)

/*!
\def I2C_OPC_TH_MED_LENGTH
\brief length of the temperature and humidity average variable for i2c-opc module.
\*/
#define I2C_OPC_TH_MED_LENGTH								      (2 * 0x04)

/*!
\def I2C_OPC_TH_MED_ADDRESS
\brief address of the temperature and humidity average variable for i2c-opc module.
\*/
#define I2C_OPC_TH_MED_ADDRESS								    (I2C_OPC_BIN_18_23_SIGMA_ADDRESS + I2C_OPC_BIN_18_23_SIGMA_LENGTH)

/*!
\def I2C_OPC_READABLE_DATA_LENGTH
\brief length of the readable variables for i2c-opc module. Need to be update with with last 2 define!!!
*/
#define I2C_OPC_READABLE_DATA_LENGTH              (I2C_OPC_TH_MED_ADDRESS + I2C_OPC_TH_MED_LENGTH - I2C_READ_REGISTER_START_ADDRESS)

/*********************************************************************
* Writable registers: Specifying the length in bytes of the data by I2C_{MODULE_NAME}_{DATA_NAME}_LENGTH, the corresponding address is calculated automatically
*********************************************************************/
/*!
\def I2C_OPC_ADDRESS_LENGTH
\brief length of the address variable for i2c-opc module.
*/
#define I2C_OPC_ADDRESS_LENGTH                   (0x01)

/*!
\def I2C_OPC_ADDRESS_ADDRESS
\brief address of the address variable for i2c-opc module.
*/
#define I2C_OPC_ADDRESS_ADDRESS                  (I2C_WRITE_REGISTER_START_ADDRESS)

/*!
\def I2C_OPC_ONESHOT_LENGTH
\brief length of the oneshot variable for i2c-opc module.
*/
#define I2C_OPC_ONESHOT_LENGTH                   (0x01)

/*!
\def I2C_OPC_ONESHOT_ADDRESS
\brief address of the oneshot variable for i2c-opc module.
*/
#define I2C_OPC_ONESHOT_ADDRESS                  (I2C_OPC_ADDRESS_ADDRESS + I2C_OPC_ADDRESS_LENGTH)

/*!
\def I2C_OPC_CONTINUOUS_LENGTH
\brief length of the continuous variable for i2c-opc module.
*/
#define I2C_OPC_CONTINUOUS_LENGTH                (0x01)

/*!
\def I2C_OPC_CONTINUOUS_ADDRESS
\brief address of the continuous variable for i2c-opc module.
*/
#define I2C_OPC_CONTINUOUS_ADDRESS               (I2C_OPC_ONESHOT_ADDRESS + I2C_OPC_ONESHOT_LENGTH)

/*!
\def I2C_OPC_WRITABLE_DATA_LENGTH
\brief length of the writable variables for i2c-opc module.
*/
#define I2C_OPC_WRITABLE_DATA_LENGTH             (I2C_OPC_CONTINUOUS_ADDRESS + I2C_OPC_CONTINUOUS_LENGTH - I2C_WRITE_REGISTER_START_ADDRESS)




// Readable registers errors checking
#if I2C_OPC_READ_REGISTERS_LENGTH > I2C_READ_REGISTER_LEN
#error "ERROR! Too many readable registers found in opc module!!!"
#endif

// Writeable registers errors checking
#if I2C_OPC_WRITABLE_DATA_LENGTH > I2C_WRITE_REGISTER_LEN
#error "ERROR! Too many writable registers found in opc module!!!"
#endif

#endif
