/**@file main.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
<http://www.gnu.org/licenses/>.
**********************************************************************/

#ifndef _MAIN_H
#define _MAIN_H

#define __STDC_LIMIT_MACROS

#include "debug_config.h"
#include "rmap_utility.h"
#include "task_util.h"
#include "drivers/module_slave_hal.hpp"
#include <Wire.h>

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "semaphore.hpp"
#include "queue.hpp"

#include "tasks/prova_task.h"
#include "tasks/supervisor_task.h"

#include "debug_F.h"

using namespace cpp_freertos;

/*********************************************************************
* TYPEDEF
*********************************************************************/


/*********************************************************************
* GLOBAL VARIABLE
*********************************************************************/
#if (ENABLE_I2C1)
BinarySemaphore *wireLock;
#endif

#if (ENABLE_I2C2)
BinarySemaphore *wire2Lock;
#endif

BinarySemaphore *configurationLock;

/*!
\fn void print_configuration(void)
\brief Print current configuration.
\return void.
*/
void print_configuration(void);

/*!
\fn void load_configuration(void)
\brief Load configuration from EEPROM.
\return void.
*/
void load_configuration(void);

/*!
\fn void save_configuration(bool is_default)
\brief Save configuration to EEPROM.
\param is_default: if true save default configuration; if false save current configuration.
\return void.
*/
void save_configuration(bool);

void init_pins(void);
void init_wire(void);
void init_sdcard(void);
void init_registers(void);
void init_can(void);
bool CAN_HW_Init(void);
void init_tasks(void);
void init_sensors(void);

#endif
