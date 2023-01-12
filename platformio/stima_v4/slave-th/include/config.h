/**@file config.h */

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

#ifndef _CONFIG_H
#define _CONFIG_H

#include "sensors_config.h"
#include "stima_config.h"

/*********************************************************************
* MODULE
*********************************************************************/
/*!
\def MODULE_MAIN_VERSION
\brief Module main version.
*/
#define MODULE_MAIN_VERSION   (4)

/*!
\def MODULE_MINOR_VERSION
\brief Module minor version.
*/
#define MODULE_MINOR_VERSION  (0)

/*!
\def MODULE_REVISION_ID
\brief Module revision id.
*/
#define MODULE_REVISION_ID    (0)

/*!
\def MODULE_TYPE
\brief Type of module. It is defined in registers.h.
*/
#if (USE_MODULE_THR)
#define MODULE_TYPE       (STIMA_MODULE_TYPE_THR)
#elif (USE_MODULE_TH)
#define MODULE_TYPE       (STIMA_MODULE_TYPE_TH)
#elif (USE_MODULE_RAIN)
#define MODULE_TYPE       (STIMA_MODULE_TYPE_RAIN)
#endif

// HW device enabled
#define ENABLE_I2C1           (true)
#define ENABLE_I2C2           (true)
#define ENABLE_QSPI           (true)
#define ENABLE_CAN            (true)
// #define ENABLE_ACCELEROMETER  ((MODULE_TYPE == STIMA_MODULE_TYPE_THR) || (MODULE_TYPE == USE_MODULE_RAIN))
#define ENABLE_ACCELEROMETER  (true)

// HW Diag PIN redefine
#define ENABLE_DIAG_PIN       (false)

// Enable (Info Task) and relative Function (Stack, Wdt ecc...)
#define ENABLE_INFO           (true)
#define LOG_STACK_USAGE

// Address EEProm for reserved bootloader flag param (and future used param)
#define START_EEPROM_ADDRESS           (0)
#define SIZE_EEPROM_RESERVED           (200)
#define BOOT_LOADER_STRUCT_ADDR        (START_EEPROM_ADDRESS)
#define BOOT_LOADER_STRUCT_SIZE        (3)
#define BOOT_LOADER_STRUCT_END         (START_EEPROM_ADDRESS + BOOT_LOADER_STRUCT_SIZE)
// Start Standard UAVCAN Register
#define REGISTER_EEPROM_ADDRESS        (START_EEPROM_ADDRESS + SIZE_EEPROM_RESERVED)

// Monitor Serial speed
#define SERIAL_DEBUG_BAUD_RATE         (115200)

// HW I2C Speed BUS and specific config
#if (ENABLE_I2C1 || ENABLE_I2C2)
#define I2C_MAX_DATA_LENGTH (32)
#define I2C_MAX_ERROR_COUNT (3)
#endif

#if (ENABLE_I2C1)
#define I2C1_BUS_CLOCK_HZ (100000L)
#endif
#if (ENABLE_I2C2)
#define I2C2_BUS_CLOCK_HZ (100000L)
#endif

// Queue Lenght
#define SYSTEM_MESSAGE_QUEUE_LENGTH       (4)
#define ELABORATE_DATA_QUEUE_LENGTH       (4)
#define REQUEST_DATA_QUEUE_LENGTH         (1)
#define REPORT_DATA_QUEUE_LENGTH          (1)

// Task system_status queue message
#define ALL_TASK_QUEUE_ID                 (99)      // Send message to ALL Task
#define SUPERVISOR_TASK_QUEUE_ID          (0)       // Send message to scecific task..
#define ACCELEROMETER_TASK_QUEUE_ID       (1)
#define CAN_TASK_QUEUE_ID                 (2)
#define ELABORATE_TASK_QUEUE_ID           (3)
#define SENSOR_TASK_QUEUE_ID              (4)
#define INFO_TASK_QUEUE_ID                (5)

// Sample and default value for elaborate task 
#define SAMPLES_COUNT_MAX                 (3600)
#define SENSORS_ACQUISITION_DELAY_MS      (4000)
#define OBSERVATIONS_TIME_S               (60)
#define REPORTS_TIME_S                    (900)

// Index Sensor
#define TEMPERATURE_MAIN_INDEX            (0)
#define HUMIDITY_MAIN_INDEX               (1)
#define TEMPERATURE_REDUNDANT_INDEX       (2)
#define HUMIDITY_REDUNDANT_INDEX          (3)
#define RAIN_INDEX                        (4)

// Limit range for module sensor
#define MAX_VALID_TEMPERATURE             (100.0)
#define MIN_VALID_TEMPERATURE             (-50.0)
#define MAX_VALID_HUMIDITY                (100.0)
#define MIN_VALID_HUMIDITY                (0.0)

#define SAMPLE_ERROR_PERCENTAGE_MAX       (50.0)
#define OBSERVATION_ERROR_PERCENTAGE_MAX  (50.0)

#endif
