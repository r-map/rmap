/**@file local_typedef.h */

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

/// @brief System module configuration
typedef struct
{
   uint8_t module_main_version;                          //!< module main version
   uint8_t module_minor_version;                         //!< module minor version
   uint8_t configuration_version;                        //!< module configuration version
   uint64_t serial_number;                               //!< module serial number
   uint8_t module_type;                                  //!< module type
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
     bool is_windsonic_hardware_error;    //!< Windsonic hardware error
     bool is_windsonic_unit_error;        //!< Windonic unit measure error
     bool is_windsonic_crc_error;         //!< Windsonic RS232 CRC error
     bool is_windsonic_axis_error;        //!< Windsonic error axis
     bool is_windsonic_responding_error;  //!< Windsonic not respond
     uint8_t perc_rs232_error;            //!< Percent of session error
     uint16_t measure_count;              //!< Total measure on session
     uint16_t error_count;                //!< Total error on session
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

/// @brief Report module
typedef struct
{
   // DWA
   rmapdata_t vavg10_speed;         //!< velocità media vettoriale su 10'
   rmapdata_t vavg10_direction;     //!< direzione media vettoriale su 10'

   // DWB
   rmapdata_t vavg_speed;           //!< velocità media vettoriale su 15'
   rmapdata_t vavg_direction;       //!< direzione media vettoriale su 15'

   // DWC
   rmapdata_t peak_gust_speed;      //!< velocità della raffica sui samples
   rmapdata_t long_gust_speed;      //!< velocità della raffica sulle osservazioni

   // DWD
   rmapdata_t avg_speed;            //!< velocità media scalare

   // DWE: classi di vento
   rmapdata_t class_1;              //!< Frequenza classe 1
   rmapdata_t class_2;              //!< Frequenza classe 2
   rmapdata_t class_3;              //!< Frequenza classe 3
   rmapdata_t class_4;              //!< Frequenza classe 4
   rmapdata_t class_5;              //!< Frequenza classe 5
   rmapdata_t class_6;              //!< Frequenza classe 6

   // DWF
   rmapdata_t peak_gust_direction;  //!< direzione della raffica sui samples
   rmapdata_t long_gust_direction;  //!< direzione della raffica sulle osservazioni

   rmapdata_t quality;              //!< common quality for all DWA DWB DWC DWD DWE DWF
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
