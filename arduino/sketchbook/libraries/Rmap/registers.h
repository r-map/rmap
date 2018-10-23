/**@file registers.h */

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

#ifndef _REGISTERS_H
#define _REGISTERS_H

/*!
\def I2C_READ_REGISTER_START_ADDRESS
\brief First i2c read register address.
*/
#define I2C_READ_REGISTER_START_ADDRESS     (0x00)

/*!
\def I2C_READ_REGISTER_END_ADDRESS
\brief Last i2c read register address.
*/
#define I2C_READ_REGISTER_END_ADDRESS       (0x1E)

/*!
\def I2C_WRITE_REGISTER_START_ADDRESS
\brief First i2c write register address.
*/
#define I2C_WRITE_REGISTER_START_ADDRESS    (0x1F)

/*!
\def I2C_WRITE_REGISTER_END_ADDRESS
\brief Last i2c write register address.
*/
#define I2C_WRITE_REGISTER_END_ADDRESS      (0xFE)

/*!
\def CONFIGURATION_EEPROM_ADDRESS
\brief First configuration register address.
*/
#define CONFIGURATION_EEPROM_ADDRESS        (0x00)

/*!
\def I2C_COMMAND_ID
\brief ID for i2c command
*/
#define I2C_COMMAND_ID                      (0xFF)

/*!
\def is_readable_register(register)
\brief Check if register is a readable register.
*/
#define is_readable_register(register)      (register <= I2C_READ_REGISTER_END_ADDRESS)

/*!
\def is_writable_register(register)
\brief Check if register is a writable register.
*/
#define is_writable_register(register)      (I2C_WRITE_REGISTER_START_ADDRESS <= register && register <= I2C_WRITE_REGISTER_END_ADDRESS)

/*!
\def is_command(value)
\brief Check if value is an i2c command.
*/
#define is_command(value)                   (value == I2C_COMMAND_ID)

#endif
