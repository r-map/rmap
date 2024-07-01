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
#define MODULE_MINOR_VERSION  (4)

/// @brief rmap protocol version
#define RMAP_PROCOTOL_VERSION (1)

// Random generator value for Local Test
// #define USE_SIMULATOR

// Fill buffer data wuith 900 data value init
// #define INIT_SIMULATOR

// Fill buffer data wuith 360 data value init complete angle 0..359 to check avg vect function
// #define VECT_MED_ON_360_SIMULATOR

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
#elif (USE_MODULE_SOLAR_RADIATION)
#define MODULE_TYPE       (STIMA_MODULE_TYPE_SOLAR_RADIATION)
#elif (USE_MODULE_WIND)
#define MODULE_TYPE       (STIMA_MODULE_TYPE_WIND)
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
#define ELABORATE_DATA_QUEUE_LENGTH (2)
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
#define SAMPLES_COUNT_MAX                 (10800)
/// @brief Default observation (RMAP) time in second 
#define OBSERVATIONS_TIME_S               (60)
/// @brief Default report (RMAP) time in second 
#define REPORTS_TIME_S                    (900)

/*********************************************************************
* Parameter of sensor and elaboration function
*********************************************************************/
/// @brief Default delay from two function acquire data 
#define SENSORS_ACQUISITION_DELAY_MS      (1000)

// Index Sensor
#define WIND_SPEED_INDEX                  (0)
#define WIND_DIRECTION_INDEX              (1)

/// @brief Limit MAX valid speed range for module sensor
#define MAX_VALID_WIND_SPEED              (70.0)
/// @brief Limit MAX valid speed range for module sensor
#define MIN_VALID_WIND_SPEED              (0.0)
/// @brief Limit MAX valid direction range for module sensor
#define MAX_VALID_WIND_DIRECTION          (359.9)
/// @brief Limit MAX valid direction range for module sensor
#define MIN_VALID_WIND_DIRECTION          (0.0)

/// @brief Samples min percent valid on elaboration data
#define SAMPLE_ERROR_PERCENTAGE_MIN       (90.0)
/// @brief Observation min percent valid on elaboration data
#define OBSERVATION_ERROR_PERCENTAGE_MIN  (90.0)

/// @brief Delay after power ON sensor before starting measurement
#define WIND_POWER_ON_DELAY_MS            (5000)

// Mutiply for RMAP and casting value data conversion
#define WIND_CASTING_SPEED_MULT           (10)
#define WIND_CASTING_SPEED_MULT_ACQUIRE   (100)

/// @brief Limit MAX Class 1 Frequency wind speed
#define WIND_CLASS_1_MAX                  (1.0)
/// @brief Limit MAX Class 2 Frequency wind speed
#define WIND_CLASS_2_MAX                  (2.0)
/// @brief Limit MAX Class 3 Frequency wind speed
#define WIND_CLASS_3_MAX                  (4.0)
/// @brief Limit MAX Class 4 Frequency wind speed
#define WIND_CLASS_4_MAX                  (7.0)
/// @brief Limit MAX Class 5 Frequency wind speed
#define WIND_CLASS_5_MAX                  (10.0)

/// @brief Reading delay
#define WIND_MESSAGE_DELAY_MS             (4)

/// @brief Windsonic compose response message delay.
#define WIND_WAITING_RESPONSE_TIMEOUT_MS  (500)

/// @brief Windsonic compose response message timeout next char
#define WIND_WAITING_READCHAR_TIMEOUT_MS  (10)

/// @brief Max delay after retry
#define WIND_RETRY_MAX_DELAY_MS           (4000)

/// @brief Limit MAX value Direction
#define WIND_DIRECTION_MAX                (360.0)
/// @brief Limit MIN value Direction
#define WIND_DIRECTION_MIN                (0.0)

/// @brief Limit MAX value Speed
#define WIND_SPEED_MAX                    (60.0)
/// @brief Limit MIN value Direction
#define WIND_SPEED_MIN                    (0.0)

/// @brief Limit MIN calm Wind for elaboration data
#define CALM_WIND_MAX_MS                  (0.1)
/// @brief Limit MIN calm Wind for elaboration data on getting vector result value
#define CALM_WIND_MAX_MED_VECT            (0.05)
/// @brief Limit MIN ATAN2 Function check for vector elaboration data
#define ATAN2_CHECK_LIMIT                 (0.001)

/// @brief Sensor Gill RS232 Speed
#define GWS_SERIAL_BAUD                   (9600)

/// @brief Sensor STX value index
#define GWS_STX_INDEX                     (0)
/// @brief Sensor ETX value index
#define GWS_ETX_INDEX                     (19)

/// @brief Sensor index value for only speed data
#define GWS_WITHOUT_DIRECTION_OFFSET      (3)
/// @brief Sensor index value for void data
#define GWS_WITHOUT_MEASUREMENT_OFFSET    (9)

// Parameter for RS232 protocol char configuration
#define GWS_DIRECTION_INDEX               (3)
#define GWS_DIRECTION_LENGTH              (3)
#define GWS_SPEED_INDEX                   (7)
#define GWS_SPEED_LENGTH                  (6)
#define GWS_STATUS_INDEX                  (16)
#define GWS_STATUS_LENGTH                 (2)
#define GWS_CRC_INDEX                     (20)
#define GWS_CRC_LENGTH                    (2)
#define STX_VALUE                         (2)
#define ETX_VALUE                         (3)
#define CR_VALUE                          (13)
#define LF_VALUE                          (10)

#endif
