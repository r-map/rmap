/**@file i2c_utility.h */

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

#ifndef _I2C_UTILITY_H
#define _I2C_UTILITY_H

#include <Arduino.h>

#define CRC8_GENERATOR      (0x7)
#define CRC16_GENERATOR     (0xA001)

uint8_t crc8(uint8_t *array, uint8_t length);
uint16_t crc16(uint8_t *array, uint8_t length);

uint8_t I2C_ClearBus();

#endif
