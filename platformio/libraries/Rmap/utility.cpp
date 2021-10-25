/**@file utility.cpp */

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

#include "utility.h"

uint16_t getUINT16FromUINT8(uint8_t *buffer) {
  return (uint16_t) (buffer[1] << 8 | buffer[0]);
}

uint16_t getUINT16FromUINT8(uint8_t byte_0, uint8_t byte_1) {
  return (uint16_t) (byte_1 << 8 | byte_0);
}

uint32_t getUINT32FromUINT8(uint8_t *buffer) {
  return (uint32_t) (buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0]);
}

uint32_t getUINT32FromUINT8(uint8_t byte_0, uint8_t byte_1, uint8_t byte_2, uint8_t byte_3) {
  return (uint32_t) (byte_3 << 24 | byte_2 << 16 | byte_1 << 8 | byte_0);
}

float getIEE754FloatFrom4UINT8(uint8_t *buffer) {
  union _float {
    uint8_t _byte[4];
    float _float;
  } val;

  memcpy(val._byte, buffer, sizeof(uint8_t) * 4);
  return val._float;
}

float getIEE754FloatFrom4UINT8(uint8_t byte_0, uint8_t byte_1, uint8_t byte_2, uint8_t byte_3) {
  union _float {
    uint8_t _byte[4];
    float _float;
  } val;

  val._byte[0] = byte_0;
  val._byte[1] = byte_1;
  val._byte[2] = byte_2;
  val._byte[3] = byte_3;

  return val._float;
}
