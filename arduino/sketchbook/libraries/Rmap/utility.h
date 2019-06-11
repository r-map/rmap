/**@file utility.h */

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

#ifndef _UTILITY_H
#define _UTILITY_H

#include <stdint.h>
#include <string.h>

uint16_t getUINT16FromUINT8(uint8_t *buffer);
uint16_t getUINT16FromUINT8(uint8_t byte_0, uint8_t byte_1);

uint32_t getUINT32FromUINT8(uint8_t *buffer);
uint32_t getUINT32FromUINT8(uint8_t byte_0, uint8_t byte_1, uint8_t byte_2, uint8_t byte_3);

float getIEE754FloatFrom4UINT8(uint8_t *buffer);
float getIEE754FloatFrom4UINT8(uint8_t byte_0, uint8_t byte_1, uint8_t byte_2, uint8_t byte_3);

#endif
