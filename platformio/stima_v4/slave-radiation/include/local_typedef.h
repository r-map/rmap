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

#define MAX_ADC_CHANELS (4)

// Gestione modalità Power ( x Canard e Nodo in generale)
enum Power_Mode : uint8_t {
   pwr_on,         // Never (All ON, test o gestione locale)
   pwr_nominal,    // Normal Sleep mode (Nominale base)
   pwr_deep_save,  // Deep mode (Very Low Power)
   pwr_critical    // Deep mode (Power Critical, Save data, Power->Off)
};

// Gestione modalità Power ( x Canard e Nodo in generale)
enum Adc_Mode : uint8_t {
   mVolt,   // Low Volt (0-3.3V) Input
   Volt,    // High Volt (0-14) Input
   mA       // mA (4-20) Input
};

// Sensor configuration
typedef struct
{
   float adc_offset;       //!< adc offset
   float adc_gain;         //!< adc gain
   float analog_min;       //!< min sensor analog value for selected range
   float analog_max;       //!< max sensor analog value for selòected range
   uint8_t is_active;      //!< Chanel active, for Radiation is selected first active (One chanel)
   Adc_Mode adc_type;      //!< adc input type selection
} sensor_configuration_t;

// System module configuration
typedef struct
{
   uint8_t module_main_version;                          //!< module main version
   uint8_t module_minor_version;                         //!< module minor version
   uint8_t configuration_version;                        //!< module configuration version
   uint64_t serial_number;                               //!< module serial number
   uint8_t module_type;                                  //!< module type
   uint8_t sensors_count;                                //!< number of configured sensors
   sensor_configuration_t sensors[MAX_ADC_CHANELS];      //!< max input ADC
   uint32_t sensor_acquisition_delay_ms;
} configuration_t;

// WatchDog Flag type
enum wdt_flag {
   clear    = 0,  // Wdt Reset (From WDT Task Controller)
   set      = 1,  // Set WDT   (From Application TASK... All OK)
   timer    = 2   // Set Timered WDT (From Application long function WDT...)
};

// Task state Flag type
enum task_flag {
   normal    = 0,  // Normal operation Task controller
   sleepy    = 1,  // Task is in sleep mode or longer wait (Inform WDT controller)
   suspended = 2   // Task is excluded from WDT Controller or Suspended complete
};

// Task Info structure
typedef struct
{
   wdt_flag watch_dog;     // WatchDog of Task
   int32_t watch_dog_ms;   // WatchDog of Task Timer
   uint16_t stack;         // Stack Max Usage Monitor
   task_flag state;        // Long sleep Task
   uint8_t running_pos;    // !=0 (CREATE) Task Started (Generic state of Task)
   uint8_t running_sub;    // Optional SubState of Task
} task_t;

// System module status
typedef struct
{
   // DateTime Operation
   struct
   {
      uint32_t next_ptr_time_for_sensors_reading;
   } datetime;

   // Info Task && WDT
   task_t tasks[TOTAL_INFO_TASK];

   // Module Flasg
   struct
   {
     bool is_cfg_loaded;
     bool is_maintenance;
     bool is_inibith_sleep;
   } flags;

   // Module error or alert
   struct
   {
     bool is_adc_unit_error;
     bool is_adc_unit_overflow;
   } events;

} system_status_t;

// System message for queue
typedef struct
{
   uint8_t task_dest;
   struct
   {
      uint8_t do_calib   : 1;   // Calibrate accelerometr
      uint8_t do_inibith : 1;   // Request inibith sleep (system_status)
      uint8_t do_maint   : 1;   // Request maintenance (system_status)
      uint8_t do_sleep   : 1;   // Optional param for difference level Sleep
      uint8_t do_cmd     : 1;   // Using param to determine type of message command
      uint8_t done_cmd   : 1;   // Using param to determine type of message response
   } command;
   uint32_t param;   // 32 Bit for generic data or casting to pointer

} system_message_t;

// Report module
typedef struct
{
   rmapdata_t avg;
   rmapdata_t quality;
} report_t;

// Backup && Upload Firmware TypeDef
typedef struct
{
  bool request_upload;
  bool backup_executed;
  bool upload_executed;
  bool rollback_executed;
  bool app_executed_ok;
  bool app_forcing_start;
  uint8_t upload_error;
  uint8_t tot_reset;
  uint8_t wdt_reset;
} bootloader_t;

#endif
