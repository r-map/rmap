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

#include "config.h"
#include "task_util.h"

#if (HARDWARE_I2C == ENABLE)
#include <Wire.h>
#endif

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "semaphore.hpp"
#include "queue.hpp"

#include "report.h"

#include "os_port.h"
#include "net_config.h"
#include "cpu_endian.h"
#include "error.h"
#include "debug.h"

#include "register.hpp"
#include "bxcan.h"
#include "module_config.hpp"

#include "tasks/led_task.h"
#include "tasks/th_sensor_task.h"
#include "tasks/elaborate_data_task.h"
#include "tasks/can_task.h"

using namespace cpp_freertos;

/*********************************************************************
* TYPEDEF
*********************************************************************/
typedef struct {
  uint8_t module_main_version;                        //!< module main version
  uint8_t module_minor_version;                       //!< module minor version
  uint8_t module_type;                                //!< module type
  uint8_t sensors_count;                              //!< number of configured sensors
  sensor_configuration_t sensors[SENSORS_COUNT_MAX];  //!< sensors configurations
  uint32_t sensor_acquisition_delay_ms;               //!< delay between 2 sensors acquisitions
  uint8_t observation_time_s;                         //!< observations time in seconds
} configuration_t;

/*********************************************************************
* GLOBAL VARIABLE
*********************************************************************/
configuration_t configuration;

#if (HARDWARE_I2C == ENABLE)
BinarySemaphore *wireLock;
#endif

Queue *elaborataDataQueue;
Queue *requestDataQueue;
Queue *reportDataQueue;

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
