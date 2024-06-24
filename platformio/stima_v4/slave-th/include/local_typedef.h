/**@file local_typedef.h */

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

#ifndef _LOCAL_TYPEDEF_H
#define _LOCAL_TYPEDEF_H

#include "local_typedef_config.h"
#include "typedef.h"

/// @brief Power mode (Canard and general Node)
enum Power_Mode : uint8_t {
   pwr_on,         ///< Never (All ON, test o gestione locale)
   pwr_nominal,    ///< Every Second (Nominale base)
   pwr_deep_save,  ///< Deep mode (Very Low Power)
   pwr_critical    ///< Deep mode (Power Critical, Save data, Power->Off)
};

/// @brief Sensor configuration
typedef struct
{
   uint8_t i2c_address;             //!< i2c sensor's address
   char driver[DRIVER_LENGTH];      //!< sensor's string driver
   char type[TYPE_LENGTH];          //!< sensor type
   bool is_redundant;               //!< sensor is the redundant, if enabled
} sensor_configuration_t;

/// @brief System module configuration
typedef struct
{
   uint8_t module_main_version;                          //!< module main version
   uint8_t module_minor_version;                         //!< module minor version
   uint8_t configuration_version;                        //!< module configuration version
   uint64_t serial_number;                               //!< module serial number
   uint8_t module_type;                                  //!< module type
   uint8_t sensors_count;                                //!< number of configured sensors
   sensor_configuration_t sensors[SENSORS_COUNT_MAX];    //!< sensors configurations
   uint32_t sensor_acquisition_delay_ms;                 //!< sensor delay from next acquire
} configuration_t;

/// @brief WatchDog Flag type
enum wdt_flag {
   clear    = 0,  //!< Wdt Reset (From WDT Task Controller)
   set      = 1,  //!< Set WDT   (From Application TASK... All OK)
   timer    = 2   //!< Set Timered WDT (From Application long function WDT...)
};

/// @brief Task state Flag type
enum task_flag {
   normal    = 0,  //!< Normal operation Task controller
   sleepy    = 1,  //!< Task is in sleep mode or longer wait (Inform WDT controller)
   suspended = 2   //!< Task is excluded from WDT Controller or Suspended complete
};

/// @brief Task Info structure
typedef struct
{
   wdt_flag watch_dog;     //!< WatchDog of Task
   int32_t watch_dog_ms;   //!< WatchDog of Task Timer
   uint16_t stack;         //!< Stack Max Usage Monitor
   task_flag state;        //!< Long sleep Task
   uint8_t running_pos;    //!< !=0 (CREATE) Task Started (Generic state of Task)
   uint8_t running_sub;    //!< Optional SubState of Task
} task_t;

/// @brief System module status
typedef struct
{
   ///< DateTime Operation
   struct
   {
      uint32_t time_start_maintenance; //!< Starting time for maintenance mode
   } datetime;

   /// Info Task && WDT
   task_t tasks[TOTAL_INFO_TASK];   //!< Info flag structure

   ///< Module Flags
   struct
   {
     bool is_cfg_loaded;            //!< Is config loaded
     bool is_maintenance;           //!< Module is in maintenance mode
     bool is_inibith_sleep;         //!< Module sleep is inibithed
   } flags;

   ///< Module error or alert
   struct
   {
     #if(ENABLE_ACCELEROMETER)
     bool is_accelerometer_error;   //!< Accelerometer error
     bool is_bubble_level_error;    //!< Bubble software accelerometer error
     #endif
     bool is_main_error;            //!< Main sensor in error
     bool is_redundant_error;       //!< Redundant sensor in error
     uint8_t perc_i2c_error;        //!< Percent of I2C attempt error
     uint16_t measure_count;        //!< Total measure counter
     uint16_t error_count;          //!< Total error counter
   } events;

} system_status_t;

/// @brief System message for queue
typedef struct
{
   uint8_t task_dest;   //!< destination task for message
   ///< Command struct
   struct
   {
      uint8_t do_calib   : 1;   //!< Calibrate accelerometr
      uint8_t do_inibith : 1;   //!< Request inibith sleep (system_status)
      uint8_t do_maint   : 1;   //!< Request maintenance (system_status)
      uint8_t do_sleep   : 1;   //!< Optional param for difference level Sleep
      uint8_t do_cmd     : 1;   //!< Using param to determine type of message command
      uint8_t done_cmd   : 1;   //!< Using param to determine type of message response
   } command;
   uint32_t param;      //!< 32 Bit for generic data or casting to pointer

} system_message_t;

/// @brief Value struct for storing sample, observation and minium, average and maximum measurement.
typedef struct {
  rmapdata_t ist;     //!< last observation or sample on request
  rmapdata_t min;     //!< average values of observations
  rmapdata_t avg;     //!< maximum values of observations
  rmapdata_t max;     //!< minium values of observations
  rmapdata_t quality; //!< quality of observations
} value_t;

/// @brief Report module
typedef struct
{
   value_t humidity;       //!< humidity value
   value_t temperature;    //!< temperature value
} report_t;

/// @brief Backup && Upload Firmware TypeDef (BootLoader)
typedef struct
{
   bool request_upload;    ///< Request an upload of firmware
   bool backup_executed;   ///< Firmware backup is executed
   bool upload_executed;   ///< An upload of firmware was executed
   bool rollback_executed; ///< An rollback of firmware was executed
   bool app_executed_ok;   ///< Flag running APP (setted after new firmware, prevert a rollback operation)
   bool app_forcing_start; ///< Force starting APP from Flash RUN APP Memory Position
   uint8_t upload_error;   ///< Error in upload firmware (ID of Error)
   uint8_t tot_reset;      ///< Number of module reset
   uint8_t wdt_reset;      ///< Number of WatchDog
} bootloader_t;

#endif
