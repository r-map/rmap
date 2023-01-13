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
#include "canard_config.hpp"
#include "stima_utility.h"
#include "task_util.h"
#include "drivers/module_slave_hal.hpp"

#include <STM32RTC.h>
#include "STM32LowPower.h"

#include <IWatchdog.h>

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"

#include "os_port.h"

#if (ENABLE_ACCELEROMETER)
#include "tasks/accelerometer_task.h"
#endif

#if (ENABLE_CAN)
#include "tasks/can_task.h"
#endif

#if ((MODULE_TYPE == STIMA_MODULE_TYPE_THR) || (MODULE_TYPE == STIMA_MODULE_TYPE_TH))
#include "tasks/th_sensor_task.h"
#endif

#include "tasks/wdt_task.h"
#include "tasks/supervisor_task.h"
#include "tasks/elaborate_data_task.h"

#include "debug_F.h"

using namespace cpp_freertos;

void init_pins(void);
void init_wire(void);
void init_sdcard(void);
void init_rtc(bool init);

#endif
