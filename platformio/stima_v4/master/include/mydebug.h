/**
 * @file net_config.h
 * @brief CycloneTCP configuration file
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2022 Oryx Embedded SARL. All rights reserved.
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
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.1.4
 **/

#ifndef _MY_DEBUG_H
#define _MY_DEBUG_H

#include <Arduino.h>
#include "compiler_port.h"
#include "debug_config.h"

#define SerialDebugInit(baud)   (SERIAL_STREAM.begin(baud))

#define TRACE_PRINTF(...) osSuspendAllTasks(), SERIAL_STREAM.printf(__VA_ARGS__), osResumeAllTasks()
#define TRACE_ARRAY(p, a, n) osSuspendAllTasks(), debugDisplayArray(NULL, p, a, n), osResumeAllTasks()
// #define TRACE_MPI(p, a) osSuspendAllTasks(), mpiDump(stderr, p, a), osResumeAllTasks()

#include "debug.h"

#endif
