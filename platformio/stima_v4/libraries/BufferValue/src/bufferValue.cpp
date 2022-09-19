/**
* @file bufferValue.cpp
* @brief Main
*
* @section License
*
* SPDX-License-Identifier: GPL-2.0-or-later
*
* Copyright (C) 2022 Marco Baldinetti. All rights reserved.
*
* This file is part of CycloneTCP Open.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
* @author Marco Baldinetti <marco.baldinetti@alling.it>
* @version 0.1
**/

#include "bufferValue.h"

template<typename buffer_g, typename length_v, typename value_v> value_v bufferRead(buffer_g *buffer, length_v length) {
   value_v value = *buffer->read_ptr;

   if (buffer->read_ptr == buffer->value+length-1) {
      buffer->read_ptr = buffer->value;
   }
   else buffer->read_ptr++;

   return value;
}

template<typename buffer_g, typename length_v, typename value_v> value_v bufferReadBack(buffer_g *buffer, length_v length) {
   value_v value = *buffer->read_ptr;

   if (buffer->read_ptr == buffer->value) {
      buffer->read_ptr = buffer->value+length-1;
   }
   else buffer->read_ptr--;

   return value;
}

template<typename buffer_g, typename value_v> void bufferWrite(buffer_g *buffer, value_v value) {
   *buffer->write_ptr = value;
}

template<typename buffer_g> void bufferPtrReset(buffer_g *buffer) {
   buffer->read_ptr = buffer->value;
}

template<typename buffer_g, typename length_v> void bufferPtrResetBack(buffer_g *buffer, length_v length) {
   if (buffer->write_ptr == buffer->value) {
      buffer->read_ptr = buffer->value+length-1;
   }
   else buffer->read_ptr = buffer->write_ptr-1;
}

template<typename buffer_g, typename length_v> void incrementBuffer(buffer_g *buffer, length_v length) {
   if (buffer->count < length) {
      buffer->count++;
   }

   if (buffer->write_ptr+1 < buffer->value + length) {
      buffer->write_ptr++;
   } else buffer->write_ptr = buffer->value;
}

template<typename buffer_g, typename length_v, typename value_v> void bufferReset(buffer_g *buffer, length_v length) {
   memset(buffer->value, UINT8_MAX, length * sizeof(value_v));
   buffer->count = 0;
   buffer->read_ptr = buffer->value;
   buffer->write_ptr = buffer->value;
}

template<typename buffer_g, typename length_v, typename value_v> void addValue(buffer_g *buffer, length_v length, value_v value) {
   *buffer->write_ptr = (value_v) value;
   incrementBuffer<buffer_g, length_v>(buffer, length);
}
