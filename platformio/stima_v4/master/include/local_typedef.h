/**@file local_typedef.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@digiteco.it>
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
   uint8_t can_address;             //!< can sensor's address [0-127]; 100 master, 127 reserved
   uint8_t can_port_id;             //!< port for uavcan services
   uint8_t can_publish_id;          //!< port for uavcan data publication
   uint64_t serial_number;          //!< Serial number of board (Used from slave for PnP Assign...)
   // uint8_t module_type;             //!< module type (optional also present in unique_id...)
} board_configuration_t;

typedef struct
{
   uint8_t module_main_version;                          //!< module main version
   uint8_t module_minor_version;                         //!< module minor version
   uint8_t configuration_version;                        //!< module configuration version
   uint8_t module_type;                                  //!< module type
   board_configuration_t board_master;                   //!< board configurations local (Master)
   board_configuration_t board_slave[BOARDS_COUNT_MAX];  //!< board configurations remote (Slave)
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
   char mqtt_maint_topic[MQTT_MAINT_TOPIC_LENGTH]; //!< mqtt maint path
   char mqtt_rpc_topic[MQTT_RPC_TOPIC_LENGTH];     //!< mqtt subscribe topic
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
   char gsm_apn[GSM_APN_LENGTH];                //!< gsm apn
   char gsm_number[GSM_NUMBER_LENGTH];          //!< gsm number
   char gsm_username[GSM_USERNAME_LENGTH];      //!< gsm username
   char gsm_password[GSM_PASSWORD_LENGTH];      //!< gsm password
   #endif
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

// System status (Status of Stima)
typedef struct
{
   struct
   {
      uint16_t next_ptr_time_for_sensors_get_istant;
      uint16_t next_ptr_time_for_sensors_get_value;
   } datetime;

   // Info Task && WDT
   task_t tasks[TOTAL_INFO_TASK];

   // Configuration Flag
   struct
   {
      bool is_loaded;
      bool is_saved;
   } configuration;

   // Connection NET Flag
   struct
   {
      bool is_connected;
      bool is_connecting;
      bool is_disconnected;
      bool is_disconnecting;
      bool is_ppp_estabilished;

      bool is_ntp_synchronized;
      bool is_ntp_synchronizing;

      bool is_mqtt_connected;
      bool is_mqtt_connecting;
      bool is_mqtt_subscribed;
      bool is_mqtt_publishing;
      bool is_mqtt_publishing_end;
      bool is_mqtt_disconnected;
      bool is_mqtt_disconnecting;
      
      bool is_http_configuration_updated;
      bool is_http_configuration_updating;
      bool is_http_firmware_upgraded;
      bool is_http_firmware_upgrading;

      uint32_t mqtt_data_published;
   } connection;

   // GSM Flag
   struct
   {
      // Signal status for last connect
      uint8_t rssi;
      uint8_t ber;
      uint8_t creg_n;
      uint8_t creg_stat;
      uint8_t cgreg_n;
      uint8_t cgreg_stat;
      uint8_t cereg_n;
      uint8_t cereg_stat;
      // Indicator state connection sequence (Ok/err)
      uint16_t connection_attempted;
      uint16_t connection_completed;
   } modem;

   // MMC/SD Flag
   struct
   {
      bool is_ready;
   } sd_card;

   // Remote data info && value for local simple direct access (LCD/Trace/Config/Check...)
   struct
   {
      uint32_t data_value_A;     // Data value first chanel (istant value)
      uint32_t data_value_B;     // Data value optional second chanel (istant value)
      uint8_t module_type;       // Type of remote module
      uint8_t module_version;    // Version RMAP
      uint8_t module_revision;   // Revision RMAP
      bool is_online;            // Node current on line
      uint16_t last_acquire;     // Last acquire data (refered to...)
   }
   data_slave[BOARDS_COUNT_MAX];

} system_status_t;

// Queue for Supervisor Request connection (GSM/NET)
typedef struct
{
   bool do_connect;
   bool do_disconnect;

   bool do_ntp_sync;

   bool do_mqtt_connect;

   bool do_http_get_configuration;
   bool do_http_get_firmware;

} connection_request_t;

// Queue for response to supervisor for Connetcion (GSM/NET)
typedef struct
{
   bool error_connected;
   bool done_connected;

   bool error_disconnected;
   bool done_disconnected;

   bool done_ntp_synchronized;
   bool error_ntp_synchronized;

   bool done_mqtt_connected;
   bool error_mqtt_connected;
   
   bool done_http_configuration_getted;
   bool error_http_configuration_getted;

   bool done_http_firmware_getted;
   bool error_http_firmware_getted;

   uint16_t number_of_mqtt_data_sent;

} connection_response_t;

// Queue for generic waiting / response operation private queue message
typedef struct
{
   bool done_operation;
   bool error_operation;

} system_response_t;

// System public message task for queue
typedef struct
{
   uint8_t task_dest;
   struct
   {
      uint8_t do_init    : 1;
      uint8_t do_load    : 1;
      uint8_t do_save    : 1;
      uint8_t do_inibith : 1;   // Request inibith sleep (system_status)
      uint8_t do_maint   : 1;   // Request maintenance (system_status)
      uint8_t do_sleep   : 1;   // Optional param for difference level Sleep
      uint8_t do_cmd     : 1;   // Using param to determine type of message command
      uint8_t done_cmd   : 1;   // Using param to determine type of message response
   } command;
   uint32_t param;   // 32 Bit for generic data or casting to pointer

} system_message_t;

// File type enum for queue file upload block
enum file_block_type {
   file_name      = 0,  // Block is name file (starting block, create file)
   data_chunck    = 1,  // Block is data block (file...)
   end_of_file    = 2,  // Block is end of file (Normal)
   ctrl_checksum  = 3   // Block is end of file (Required checksum control)
};

// Queue message (data) for file (firmware) put/get
typedef struct
{
   file_block_type   block_type;
   uint16_t          block_lenght;
   uint8_t           block[FILE_PUT_DATA_BLOCK_SIZE];
} file_queue_t;

// Backup && Upload Firmware TypeDef
typedef struct
{
  bool request_upload;
  bool backup_executed;
  bool upload_executed;
  bool rollback_executed;
  bool app_executed_ok;
  uint8_t upload_error;
} bootloader_t;

#endif
