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

#include "core/net.h"
#include "hardware/stm32l4xx/stm32l4xx_crypto.h"
#include "rng/trng.h"
#include "rng/yarrow.h"

#include "tasks/supervisor_task.h"

#if (ENABLE_WDT)
#include "tasks/wdt_task.h"
#endif

#if (ENABLE_LCD)
#include "tasks/lcd_task.h"
#endif

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
#include "tasks/modem_task.h"
#endif

#if (USE_NTP)
#include "tasks/ntp_task.h"
#endif

#if (USE_HTTP)
#include "tasks/http_task.h"
#endif

#if (USE_MQTT)
#include "tasks/mqtt_task.h"
#endif

#if (ENABLE_CAN)
#include "tasks/can_task.h"
#endif

#include "debug_F.h"

using namespace cpp_freertos;

/*********************************************************************
* GLOBAL VARIABLE
*********************************************************************/
#if (ENABLE_I2C1)
BinarySemaphore *wireLock;
#endif

#if (ENABLE_I2C2)
BinarySemaphore *wire2Lock;
#endif

Queue *systemRequestQueue;
Queue *systemResponseQueue;

BinarySemaphore *configurationLock;
BinarySemaphore *systemStatusLock;

configuration_t configuration;
system_status_t system_status;

YarrowContext yarrowContext;
uint8_t seed[SEED_LENGTH];

void init_pins(void);
void init_wire(void);
void init_sdcard(void);
void init_registers(void);
void init_can(void);
void init_tasks(void);
void init_sensors(void);
bool init_net(YarrowContext *yarrowContext, uint8_t *seed, size_t seed_length);
bool CAN_HW_Init(void);

void input_pin_encoder_A();
void input_pin_encoder_B();
void input_pin_encoder_C();

#endif
