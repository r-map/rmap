/**@file config.h */

/*********************************************************************
<h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
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
/// @brief Module main version.
#define MODULE_MAIN_VERSION   (4)

/// @brief Module minor version.
#define MODULE_MINOR_VERSION  (3)

/// @brief rmap protocol version
#define RMAP_PROCOTOL_VERSION (1)

// Random generator value for Local Test
// #define USE_SIMULATOR

// Fill buffer data wuith 900 data value init
// #define INIT_SIMULATOR

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

/// @brief Enable control Error for Debug
#define DEBUG_MODE            (false)
#define ERROR_HANDLER_CB      (false)

/*********************************************************************
* HW DEVICES
*********************************************************************/

/// @brief Enable I2C1 interface
#define ENABLE_I2C1           (true)
/// @brief Enable I2C2 interface
#define ENABLE_I2C2           (true)
/// @brief Enable QSPI interface
#define ENABLE_QSPI           (true)
/// @brief Enable CAN BUS interface
#define ENABLE_CAN            (true)
/// @brief Enable I2C Accelerometer
#define ENABLE_ACCELEROMETER  (true)

/// @brief Enable HW Diag PIN redefine
#define ENABLE_DIAG_PIN       (false)

// Enable (Wdt Task and Module) and relative Function (Stack, Info ecc...)
/// @brief Enable WatchDog Task and Module
#define ENABLE_WDT            (true)
/// @brief WatchDog Hardware microseconds timeout
#define WDT_TIMEOUT_BASE_US   (8000000)
/// @brief Init WatchDog Task local milliseconds
#define WDT_STARTING_TASK_MS  (60000)
/// @brief Task milliseconds minimal check
#define WDT_CONTROLLER_MS     (2000)  
/// @brief Enable stack usage
#define ENABLE_STACK_USAGE    (true)
/// @brief Monitor Sub Position not used flag
#define UNUSED_SUB_POSITION   (0)          
/// @brief Monitor No Sleep / No Suspend
#define NORMAL_STATE          (0)  
/// @brief Sleep Task For Wdt or LowPower Check         
#define SLEEP_STATE           (1)   
/// @brief Suspend Task from Wdt        
#define SUSPEND_STATE         (2)           

/*********************************************************************
* Generic Semaphore Time acquire RTC
*********************************************************************/
/// @brief Enable RTC Interface
#define ENABLE_RTC            (true)
/// @brief Delay for RTC in milliseconds
#define RTC_WAIT_DELAY_MS     (100)

/*********************************************************************
* Address EEProm for reserved bootloader flag param (and future used 2000 Bytes)
*********************************************************************/
/// @brief Starting EEPROM address
#define START_EEPROM_ADDRESS           (0)
/// @brief Size EEPROM reserved address. Must be > CONFIGURATION_EEPROM_END
#define SIZE_EEPROM_RESERVED           (450)  
/// @brief Bootloader start address                       
#define BOOT_LOADER_STRUCT_ADDR        (START_EEPROM_ADDRESS)
/// @brief Bootloader struct size
#define BOOT_LOADER_STRUCT_SIZE        (sizeof(bootloader_t))
/// @brief Bootloader struct end address  
#define BOOT_LOADER_STRUCT_END         (START_EEPROM_ADDRESS + BOOT_LOADER_STRUCT_SIZE)

/*********************************************************************
* Private configuration board direct
*********************************************************************/
/// @brief Start Address EEPROM configuration
#define CONFIGURATION_EEPROM_ADDRESS   (20)
/// @brief Start Standard UAVCAN Register
#define REGISTER_EEPROM_ADDRESS        (START_EEPROM_ADDRESS + SIZE_EEPROM_RESERVED)

/// @brief Monitor Debug Serial speed
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

/*********************************************************************
* Queue Lenght
*********************************************************************/
/// @brief Request system message queue length
#define SYSTEM_MESSAGE_QUEUE_LENGTH (4)
/// @brief Elaborate data message queue length
#define ELABORATE_DATA_QUEUE_LENGTH (6)
/// @brief Request data message queue length
#define REQUEST_DATA_QUEUE_LENGTH   (1)
/// @brief Report data message queue length
#define REPORT_DATA_QUEUE_LENGTH    (1)
/// @brief Request sensor queue length
#define RAIN_QUEUE_LENGTH           (1)

/*********************************************************************
* Task system_status and queue ID message
*********************************************************************/
/// @brief All task ID. Send message to ALL Task
#define ALL_TASK_ID                 (99)    
/// @brief Supervisor task ID
#define SUPERVISOR_TASK_ID          (0)    
/// @brief CAN task ID
#define CAN_TASK_ID                 (1)
/// @brief Elaborate data task ID
#define ELABORATE_TASK_ID           (2)
/// @brief Sensor acquire task ID
#define SENSOR_TASK_ID              (3)
/// @brief Accelerometer task ID
#define ACCELEROMETER_TASK_ID       (4)
/// @brief Watch Dog task ID
#define WDT_TASK_ID                 (5)
/// @brief Total Max Task for WDT Task Control
#define TOTAL_INFO_TASK             (WDT_TASK_ID + 1) 

/*********************************************************************
* Global queue wait and other timeout
*********************************************************************/
/// @brief Time to wait pushing data queue
#define WAIT_QUEUE_REQUEST_PUSHDATA_MS    (500)
/// @brief Time to wait pushing command queue
#define WAIT_QUEUE_REQUEST_COMMAND_MS     (500)
/// @brief Time to wait event reset get tips command
#define WAIT_QUEUE_REQUEST_RESET_TIP_MS   (2000)

/// @brief Default value for sensor acquire task delay tipping
#define RAIN_TIPPING_DELAY_MS             (100)
/// @brief Default value for inibith tipping from 2 events
#define RAIN_TIPPING_INIBITH_DELAY_MS     (500)

/// @brief How much mm (scaled RMAP) of rain for one tip of tipping bucket rain gauge.
#define RAIN_TIPS_FOR_EVENT               (2)

/*********************************************************************
* Specific parameter of sensor
*********************************************************************/
/// @brief Use redundant PIN tipping (verify and alert)
#define USE_TIPPING_BUCKET_REDUNDANT      (true)
/// @brief Use clogged UP system control (verify and alert)
#define USE_CLOGGED_UP_CONTROL            (true)    // Use clogged UP system control (verify and alert)
/// @brief Use strict tipping timings mode
#define USE_TIPPING_CONTROL               (false)

/// @brief Input PIN tipping expected event value
#define TIPPING_EVENT_VALUE               HIGH
/// @brief Input PIN tipping
#define TIPPING_BUCKET_PIN                PIN_IN0
/// @brief Redundant PIN tipping (verify and alert)
#define TIPPING_BUCKET_PIN_REDUNDANT      PIN_IN1
/// @brief Clogged Up Alert PIN
#define CLOGGED_UP_PIN                    PIN_IN2
/// @brief Input PIN clogged expected event value
#define CLOGGED_EVENT_VALUE               LOW

/*********************************************************************
* Parameter of buffer data dimension and acquire
*********************************************************************/
/// @brief Sample and default value for elaborate task 
#define SAMPLES_COUNT_MAX                 (3600)
/// @brief Default observation (RMAP) time in second 
#define OBSERVATIONS_TIME_S               (60)
/// @brief Default report (RMAP) time in second 
#define REPORTS_TIME_S                    (900)

/*********************************************************************
* Parameter of sensor and elaboration function
*********************************************************************/
// TPR Elaboration scrolling data timings
/// @brief Using mobile scrolling max -> avg value by 60 sec. on all data buffered consecutive
#define USE_MOBILE_TPR_60_S_AVG_MODE      (false)
/// @brief Minimal timing for 2 report calculate (less can be only a command retry) 
#define REPORT_INVALID_ACQUIRE_MIN_MS     (3000)
/// @brief Timing for acquire and check TPR elaboration
#define SAMPLES_ACQUIRE_MS                (10000)
/// @brief Samples need for TPR over 1'
#define SAMPLES_NEED_TPR_60_S             (60000 / SAMPLES_ACQUIRE_MS)
/// @brief Samples need for TPR over 5'
#define SAMPLES_NEED_TPR_05_M             (300000 / SAMPLES_ACQUIRE_MS)

// Index Sensor
#define RAIN_TIPS_INDEX                   (0)
#define RAIN_RAIN_INDEX                   (1)
#define RAIN_FULL_INDEX                   (2)
#define RAIN_TIPS_RESET                   (3)
#define RAIN_SCROLL_INDEX                 (4)
#define RAIN_SCROLL_RESET                 (5)

/// @brief Limit MAX valid range for module sensor
#define MAX_VALID_RAIN                    (1000.0)
/// @brief Limit MIN valid range for module sensor
#define MIN_VALID_RAIN                    (0.0)
/// @brief RMAP Multiply value for TPR Elaboration 
#define RAIN_RATE_MULTIPLY                (1000)

/// @brief Samples min percent valid on elaboration data
#define SAMPLE_ERROR_PERCENTAGE_MIN       (90.0)
/// @brief Observation min percent valid on elaboration data
#define OBSERVATION_ERROR_PERCENTAGE_MIN  (90.0)

#endif
