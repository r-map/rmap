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

#include "config.h"
#include "typedef.h"

typedef struct {
   uint8_t module_main_version;                       //!< module main version
   uint8_t module_minor_version;                      //!< module minor version
   uint8_t module_type;                               //!< module type
   uint8_t sensors_count;                             //!< number of configured sensors
   sensor_configuration_t sensors[SENSORS_COUNT_MAX]; //!< sensors configurations
   uint32_t sensor_acquisition_delay_ms;              //!< delay between 2 sensors acquisitions
   uint8_t observation_time_s;                        //!< observations time in seconds
} configuration_t;

typedef struct
{
   struct {
      bool is_loaded;
      bool is_saved;
   } configuration;

   struct {
      bool is_connected;
      bool is_connection_ongoing;
      bool is_ntp_synchronized;
      bool is_ntp_sync_ongoing;
   } connection;
} system_status_t;

typedef struct
{
   struct
   {
      bool do_load;
      bool do_save;
   } configuration;

   struct
   {
      bool do_connect;
      bool do_disconnect;
      bool do_ntp_sync;
      bool do_mqtt_connect;
   } connection;
} system_request_t;

typedef struct
{
   struct
   {
      bool done_loaded;
      bool done_saved;
   } configuration;

   struct
   {
      bool done_connected;
      bool done_disconnected;
      bool done_ntp_synchronized;
      bool done_mqtt_connect;
   } connection;
} system_response_t;

#endif
