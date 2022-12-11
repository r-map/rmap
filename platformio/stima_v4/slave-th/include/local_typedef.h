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

typedef struct
{
   uint8_t i2c_address;             //!< i2c sensor's address
   // uint8_t can_port_id;             //!< port for uavcan services
   // uint8_t can_publish_id;          //!< port for uavcan data publication
   char driver[DRIVER_LENGTH];      //!< sensor's string driver
   char type[TYPE_LENGTH];          //!< sensor type
   bool is_redundant;
} sensor_configuration_t;

typedef struct
{
   uint8_t module_main_version;                          //!< module main version
   uint8_t module_minor_version;                         //!< module minor version
   uint8_t module_type;                                  //!< module type
   uint8_t sensors_count;                                //!< number of configured sensors
   sensor_configuration_t sensors[SENSORS_COUNT_MAX];    //!< sensors configurations
   uint32_t sensor_acquisition_delay_ms;
} configuration_t;

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

} system_status_t;

typedef struct
{
   struct
   {
      bool do_load;
      bool do_save;
   } configuration;

} system_request_t;

typedef struct
{
   struct
   {
      bool done_loaded;
      bool done_saved;
   } configuration;

} system_response_t;

typedef struct
{
   value_t humidity;
   value_t temperature;
} report_t;

#endif
