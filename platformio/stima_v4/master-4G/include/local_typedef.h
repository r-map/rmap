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
#include "mqtt_config.h"
#include "ntp_config.h"
#include "gsm_config.h"
#include "ethernet_config.h"
#include "constantdata_config.h"
#include "typedef.h"

/*!
\def CONSTANTDATA_BTABLE_LENGTH
\brief Maximum lenght of btable code plus terminator that describe one constant data.
*/
#define CONSTANTDATA_BTABLE_LENGTH (7)

/*!
\def CONSTANTDATA_VALUE_LENGTH
\brief Maximum lenght of value plus terminator for one constant data.
*/
#define CONSTANTDATA_VALUE_LENGTH (33)

#define DATA_LEVEL_LENGTH  (20)
#define NETWORK_LENGTH     (20)
#define IDENT_LENGTH       (20)

#define CLIENT_PSK_KEY_LENGTH (16)
#define CLIENT_PSK_IDENTITY_LENGTH (MQTT_USERNAME_LENGTH + STATIONSLUG_LENGTH + BOARDSLUG_LENGTH)

/*!
\struct sensordata_t
\brief constant station data (station name, station height ...) parameters.
*/
typedef struct
{
   char btable[CONSTANTDATA_BTABLE_LENGTH]; //!< table B code for constant station data
   char value[CONSTANTDATA_VALUE_LENGTH];   //!< value of constant station data
} constantdata_t;

typedef struct
{
   uint8_t address;                             //!< address of sensor
   uint8_t node;                                //!< sensor's node
   char driver[DRIVER_LENGTH];                  //!< sensor's string driver
   char type[TYPE_LENGTH];                      //!< sensor type
} sensor_configuration_t;

typedef struct
{
   uint8_t module_main_version;                          //!< module main version
   uint8_t module_minor_version;                         //!< module minor version
   uint8_t module_type;                                  //!< module type
   uint8_t sensors_count;                                //!< number of configured sensors
   sensor_configuration_t sensors[SENSORS_COUNT_MAX];    //!< sensors configurations
   uint16_t observation_s;                               //!< observations time in seconds
   uint16_t report_s;                                    //!< report time in seconds

   char data_level[DATA_LEVEL_LENGTH];
   char ident[IDENT_LENGTH];
   int32_t longitude;
   int32_t latitude;
   char network[NETWORK_LENGTH];

   #if (USE_MQTT)
   uint16_t mqtt_port;                             //!< mqtt server port
   char mqtt_server[MQTT_SERVER_LENGTH];           //!< mqtt server
   // char mqtt_root_topic[MQTT_ROOT_TOPIC_LENGTH];   //!< mqtt root path
   // char mqtt_maint_topic[MQTT_MAINT_TOPIC_LENGTH]; //!< mqtt maint path
   // char mqtt_rpc_topic[MQTT_RPC_TOPIC_LENGTH];     //!< mqtt subscribe topic
   char mqtt_username[MQTT_USERNAME_LENGTH];       //!< username to compose mqtt username (username/stationslug/boardslug)
   char mqtt_password[MQTT_PASSWORD_LENGTH];       //!< mqtt password
   char stationslug[STATIONSLUG_LENGTH];           //!< station slug to compose mqtt username (username/stationslug/boardslug)
   char boardslug[BOARDSLUG_LENGTH];               //!< board slug to compose mqtt username (username/stationslug/boardslug)
   uint8_t client_psk_key[CLIENT_PSK_KEY_LENGTH];
   #endif

   #if (USE_NTP)
   char ntp_server[NTP_SERVER_LENGTH]; //!< ntp server
   #endif

   constantdata_t constantdata[USE_CONSTANTDATA_COUNT]; //!< Constantdata buffer for storing constant station data parameter
   uint8_t constantdata_count;                          //!< configured constantdata number

   #if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_ETH)
   bool is_dhcp_enable;                       //!< dhcp status
   uint8_t ethernet_mac[ETHERNET_MAC_LENGTH]; //!< ethernet mac
   uint8_t ip[ETHERNET_IP_LENGTH];            //!< ip address
   uint8_t netmask[ETHERNET_IP_LENGTH];       //!< netmask
   uint8_t gateway[ETHERNET_IP_LENGTH];       //!< gateway
   uint8_t primary_dns[ETHERNET_IP_LENGTH];   //!< primary dns
   #endif

   #if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
   char gsm_apn[GSM_APN_LENGTH];           //!< gsm apn
   char gsm_number[GSM_NUMBER_LENGTH]; //!< gsm number
   char gsm_username[GSM_USERNAME_LENGTH]; //!< gsm username
   char gsm_password[GSM_PASSWORD_LENGTH]; //!< gsm password
   #endif
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

   struct
   {
      bool is_connected;
      bool is_connection_ongoing;
      bool is_disconnected;
      bool is_disconnection_ongoing;
      bool is_ntp_synchronized;
      bool is_ntp_sync_ongoing;
      bool is_mqtt_connected;
      bool is_mqtt_connection_ongoing;
      bool is_mqtt_disconnected;
      bool is_mqtt_disconnection_ongoing;
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
      bool do_mqtt_disconnect;
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
      bool done_mqtt_connected;
      bool done_mqtt_disconnected;
   } connection;
} system_response_t;

#endif
