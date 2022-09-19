/**
 * @file bufferValue.h
 * @brief Buffer Value
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2022 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneCRYPTO Open.
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
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.1.4
 **/

#ifndef _BUFFER_VALUE_H
#define _BUFFER_VALUE_H

#include <cstdint>

template<typename buffer_g, typename length_v, typename value_v> value_v bufferRead(buffer_g *buffer, length_v length);
template<typename buffer_g, typename length_v, typename value_v> value_v bufferReadBack(buffer_g *buffer, length_v length);
template<typename buffer_g, typename value_v> void bufferWrite(buffer_g *buffer, value_v value);
template<typename buffer_g> void bufferPtrReset(buffer_g *buffer);
template<typename buffer_g, typename length_v> void bufferPtrResetBack(buffer_g *buffer, length_v length);
template<typename buffer_g, typename length_v> void incrementBuffer(buffer_g *buffer, length_v length);
template<typename buffer_g, typename length_v, typename value_v> void bufferReset(buffer_g *buffer, length_v length);
template<typename buffer_g, typename length_v, typename value_v> void addValue(buffer_g *buffer, length_v length, value_v value);

#endif
