/**@file config.h */

/*********************************************************************
<h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
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
#define MODULE_MINOR_VERSION  (4)

/// @brief rmap protocol version
#define RMAP_PROCOTOL_VERSION (1)

/// @brief Random generator value for Local Test
// #define USE_SIMULATOR

/// @brief Fill buffer data wuith 900 data value init
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
#define ENABLE_ACCELEROMETER  (false)

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
#define ELABORATE_DATA_QUEUE_LENGTH (4)
/// @brief Request data message queue length
#define REQUEST_DATA_QUEUE_LENGTH   (1)
/// @brief Report data message queue length
#define REPORT_DATA_QUEUE_LENGTH    (1)

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
/// @brief Default delay from two function acquire data
#define SENSORS_ACQUISITION_DELAY_MS      (4000)
/// @brief MIN valid delay from acquire
#define SENSORS_ACQUISITION_DELAY_MS_MIN  (200)
/// @brief MAX valid delay from acquire
#define SENSORS_ACQUISITION_DELAY_MS_MAX  (60000)

// Index Sensor
#define TEMPERATURE_MAIN_INDEX            (0)
#define HUMIDITY_MAIN_INDEX               (1)
#define TEMPERATURE_REDUNDANT_INDEX       (2)
#define HUMIDITY_REDUNDANT_INDEX          (3)

/// @brief Limit MAX valid value for temperature
#define MAX_VALID_TEMPERATURE             (100.0)
/// @brief Limit MIN valid value for temperature
#define MIN_VALID_TEMPERATURE             (-50.0)
/// @brief Limit difference betwen main and redundant sensor check temperature
#define MAX_DIFF_VALUE_TEMPERATURE        (1.5)
/// @brief Limit difference temperature power conversion
#define MAX_DIFF_VALUE_TEMPERATURE_POWER  (100)
/// @brief Limit MAX valid value for humidity
#define MAX_VALID_HUMIDITY                (100.0)
/// @brief Limit MAX valid value for unidity
#define MIN_VALID_HUMIDITY                (0.0)
/// @brief Limit difference betwen main and redundant sensor check humidity
#define MAX_DIFF_VALUE_HUMIDITY           (6)
/// @brief Limit difference temperature power conversion
#define MAX_DIFF_VALUE_HUMIDITY_POWER     (1)
/// @brief MIN Percent flag signal on retrieve sensor data value
#define MIN_PERC_SEND_ERR                 (5)

/// @brief Samples min percent valid on elaboration data
#define SAMPLE_ERROR_PERCENTAGE_MIN       (90.0)
/// @brief Observation min percent valid on elaboration data
#define OBSERVATION_ERROR_PERCENTAGE_MIN  (90.0)

#endif
