/**@file local_typedef.h */

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

#ifndef _LOCAL_TYPEDEF_H
#define _LOCAL_TYPEDEF_H

#include "local_typedef_config.h"
#include "typedef.h"

// Sensor configuration
typedef struct
{
   uint8_t i2c_address;             //!< i2c sensor's address
   // uint8_t can_port_id;             //!< port for uavcan services
   // uint8_t can_publish_id;          //!< port for uavcan data publication
   char driver[DRIVER_LENGTH];      //!< sensor's string driver
   char type[TYPE_LENGTH];          //!< sensor type
   bool is_redundant;
} sensor_configuration_t;

// System module configuration
typedef struct
{
   uint8_t module_main_version;                          //!< module main version
   uint8_t module_minor_version;                         //!< module minor version
   uint8_t module_type;                                  //!< module type
   uint8_t sensors_count;                                //!< number of configured sensors
   sensor_configuration_t sensors[SENSORS_COUNT_MAX];    //!< sensors configurations
   uint32_t sensor_acquisition_delay_ms;
} configuration_t;

// System module status
typedef struct
{
   struct
   {
      uint32_t system_time;
      uint32_t next_ptr_time_for_sensors_reading;
   } datetime;

   struct
   {
      bool is_loaded;
      bool is_saved;
   } configuration;

   struct
   {
     bool is_maintenance;
     bool is_inibith_sleep;
   } flags;

} system_status_t;

// System message request for queue
typedef struct
{
   uint8_t task_dest;
   struct
   {
      uint8_t do_init  : 1;
      uint8_t do_load  : 1;
      uint8_t do_save  : 1;
      uint8_t do_run   : 1;
      uint8_t do_maint : 1;
      uint8_t do_sleep : 1;
      uint8_t do_cmd   : 1;   // Using param to determine type of message command
      uint8_t done_cmd : 1;   // Using param to determine type of message response
   } command;
   uint32_t param;   // 32 Bit for generic data or casting to pointer

} system_message_t;

// Report module
typedef struct
{
   value_t humidity;
   value_t temperature;
} report_t;

#endif
