/**@file registers-master.h */

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

#ifndef _REGISTERS_MASTER_H
#define _REGISTERS_MASTER_H

#include "registers.h"

/*!
\def I2C_MASTER_DEFAULT_ADDRESS
\brief Default address for i2c-master module.
*/
#define I2C_MASTER_DEFAULT_ADDRESS                  (0x20)

/*!
\def I2C_MASTER_COMMAND_NONE
\brief NO command for i2c-master module.
*/
#define I2C_MASTER_COMMAND_NONE                 (0x00)

/*!
\def I2C_MASTER_COMMAND_SAVE
\brief Save command for i2c-master module.
*/
#define I2C_MASTER_COMMAND_SAVE                     (0x01)


/*********************************************************************
* Readable registers: Specifying the length in bytes of the data by I2C_{MODULE_NAME}_{DATA_NAME}_LENGTH, the corresponding address is calculated automatically
*********************************************************************/

/*********************************************************************
* Writable registers: Specifying the length in bytes of the data by I2C_{MODULE_NAME}_{DATA_NAME}_LENGTH, the corresponding address is calculated automatically
*********************************************************************/
/*!
\def I2C_MASTER_ADDRESS_LENGTH
\brief length of the address variable for i2c-master module.
*/
#define I2C_MASTER_ADDRESS_LENGTH                   (0x01)

/*!
\def I2C_MASTER_ADDRESS_ADDRESS
\brief address of the address variable for i2c-master module.
*/
#define I2C_MASTER_ADDRESS_ADDRESS                  (I2C_WRITE_REGISTER_START_ADDRESS)



/*!
\def I2C_MASTER_CONFIGURATION_INDEX_LENGTH
\brief length of the configuration index variable for i2c-master module.
*/
#define I2C_MASTER_CONFIGURATION_INDEX_LENGTH                   (0x02)

/*!
\def I2C_MASTER_CONFIGURATION_INDEX_ADDRESS
\brief address of the configuration index variable for i2c-master module.
*/
#define I2C_MASTER_CONFIGURATION_INDEX_ADDRESS                  (I2C_MASTER_ADDRESS_ADDRESS + I2C_MASTER_ADDRESS_LENGTH)

/*!
\def I2C_MASTER_CONFIGURATION_LENGTH
\brief length of the configuration variable for i2c-master module.
*/
#define I2C_MASTER_CONFIGURATION_LENGTH                   (0x24)

/*!
\def I2C_MASTER_CONFIGURATION_ADDRESS
\brief address of the oneshot variable for i2c-master module.
*/
#define I2C_MASTER_CONFIGURATION_ADDRESS                  (I2C_MASTER_CONFIGURATION_INDEX_ADDRESS + I2C_MASTER_CONFIGURATION_INDEX_LENGTH)

/*!
\def I2C_MASTER_WRITABLE_DATA_LENGTH
\brief length of the writable variables for i2c-master module.
*/
#define I2C_MASTER_WRITABLE_DATA_LENGTH             (I2C_MASTER_CONFIGURATION_ADDRESS + I2C_MASTER_CONFIGURATION_LENGTH - I2C_WRITE_REGISTER_START_ADDRESS)


// Readable registers errors checking
#if I2C_MASTER_READ_REGISTERS_LENGTH > I2C_READ_REGISTER_LEN
#error "ERROR! Too many readable registers found in MASTER module!!!"
#endif

// Writeable registers errors checking
#if I2C_MASTER_WRITE_REGISTERS_LENGTH > I2C_WRITE_REGISTER_LEN
#error "ERROR! Too many writable registers found in MASTER module!!!"
#endif

#endif
