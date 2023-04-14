/**@file config.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@digiteco.it>
authors:
Marco Baldinetti <marco.baldinetti@digiteco.it>

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
\def RMAP_PROCOTOL_VERSION
\brief rmap protocol version
*/
#define RMAP_PROCOTOL_VERSION (1)

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
#define ENABLE_SERIAL2        (false)
#define ENABLE_QSPI           (true)
#define ENABLE_CAN            (true)
#define ENABLE_ACCELEROMETER  (true)

// HW Diag PIN redefine
#define ENABLE_DIAG_PIN       (false)

// Enable (Wdt Task and Module) and relative Function (Stack, Info ecc...)
#define ENABLE_WDT            (true)
#define WDT_TIMEOUT_BASE_US   (8000000)     // WatchDog HW us
#define WDT_STARTING_TASK_MS  (60000)       // Init WDT Task Local ms
#define WDT_CONTROLLER_MS     (2000)        // Task ms minimal check
#define ENABLE_STACK_USAGE    (true)
#define UNUSED_SUB_POSITION   (0)           // Monitor Sub Position Not Used Flag
#define NORMAL_STATE          (0)           // Monitor No Sleep / No Suspend
#define SLEEP_STATE           (1)           // Sleep Task For Wdt or LowPower Check
#define SUSPEND_STATE         (2)           // Suspend Task from WDT

// Generic Semaphore Time acquire RTC
#define ENABLE_RTC            (true)
#define RTC_WAIT_DELAY_MS     (100)

// Address EEProm for reserved bootloader flag param (and future used)
#define START_EEPROM_ADDRESS           (0)
#define SIZE_EEPROM_RESERVED           (450)
#define BOOT_LOADER_STRUCT_ADDR        (START_EEPROM_ADDRESS)
#define BOOT_LOADER_STRUCT_SIZE        (sizeof(bootloader_t))
#define BOOT_LOADER_STRUCT_END         (START_EEPROM_ADDRESS + BOOT_LOADER_STRUCT_SIZE)
// Private configuration board direct
#define CONFIGURATION_EEPROM_ADDRESS   (20)
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
#define SYSTEM_MESSAGE_QUEUE_LENGTH (4)
#define ELABORATE_DATA_QUEUE_LENGTH (4)
#define REQUEST_DATA_QUEUE_LENGTH   (1)
#define REPORT_DATA_QUEUE_LENGTH    (1)
#define RAIN_QUEUE_LENGTH           (1)

// Task system_status and queue ID message
#define ALL_TASK_ID                 (99)      // Send message to ALL Task
#define SUPERVISOR_TASK_ID          (0)       // Send message to specific task..
#define CAN_TASK_ID                 (1)
#define ELABORATE_TASK_ID           (2)
#define SENSOR_TASK_ID              (3)
#define ACCELEROMETER_TASK_ID       (4)
#define WDT_TASK_ID                 (5)
#define TOTAL_INFO_TASK             (WDT_TASK_ID + 1) // Total Max Task for WDT Task Control

// Global queue wait timeout
#define WAIT_QUEUE_REQUEST_ELABDATA_MS    (50)
#define WAIT_QUEUE_RESPONSE_ELABDATA_MS   (50)
#define WAIT_QUEUE_REQUEST_COMMAND_MS     (500)

// Default value for sensor acquire task 
#define SENSORS_TIPPING_DELAY_MS          (200)
#define SENSORS_TIPS_FOR_EVENT            (1)

// Sample and default value for elaborate task 
#define SAMPLES_COUNT_MAX                 (3600)
#define OBSERVATIONS_TIME_S               (60)
#define REPORTS_TIME_S                    (900)

// Index Sensor
#define RAIN_TIPS_INDEX                   (0)
#define RAIN_RAIN_INDEX                   (1)
#define RAIN_FULL_INDEX                   (2)
#define RAIN_RESET_INDEX                  (3)

// Limit range for module sensor
#define MAX_VALID_RAIN                    (1000.0)
#define MIN_VALID_RAIN                    (0.0)

#define SAMPLE_ERROR_PERCENTAGE_MIN       (90.0)
#define OBSERVATION_ERROR_PERCENTAGE_MIN  (90.0)

#endif
