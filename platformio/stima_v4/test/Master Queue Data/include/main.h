/**@file main.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>

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
#include "stima_utility.h"
#include "task_util.h"
#include "drivers/module_master_hal.hpp"

#include <STM32RTC.h>
#include "STM32LowPower.h"

#include <IWatchdog.h>

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "semaphore.hpp"
#include "queue.hpp"

#include "tasks/supervisor_task.h"
#include "tasks/wdt_task.h"
#include "tasks/sd_task.h"

#include "debug_F.h"

using namespace cpp_freertos;

void init_wire(void);
void init_sdcard(void);
void init_rtc(bool init);

#endif
