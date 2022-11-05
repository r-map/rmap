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

#include "local_typedef.h"
#include "rmap_utility.h"
#include "task_util.h"
#include "drivers/module_master_hal.hpp"
#include <Wire.h>

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "semaphore.hpp"
#include "queue.hpp"

#include "os_port.h"
#include "net_config.h"
#include "cpu_endian.h"
#include "error.h"
#include "debug.h"
#include "core/net.h"
#include "drivers/spi/arduino_spi_driver.h"
#include "drivers/ext/arduino_interrupt_driver.h"
#include "drivers/eth/enc28j60_driver.h"
#include "dhcp/dhcp_client.h"
#include "tls.h"
#include "tls_cipher_suites.h"
#include "hardware/stm32l4xx/stm32l4xx_crypto.h"

// #include "register.hpp"
// #include "bxcan.h"
// #include "module_config.hpp"

// #include "tasks/led_task.h"
// #include "tasks/hardware_task.h"
// #include "tasks/ethernet_task.h"
// #include "tasks/mqtt_task.h"
// #include "tasks/can_task.h"
#include "tasks/prova_task.h"
#include "tasks/supervisor_task.h"

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
configuration_t configuration;

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
