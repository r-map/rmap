/**@file local_typedef.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <m.baldinetti@digiteco.it>
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

// Power mode (Canard and general Node) 
enum Power_Mode : uint8_t {
   pwr_on,         // Never (All ON, test o gestione locale)
   pwr_nominal,    // Every Second (Nominale base)
   pwr_deep_save,  // Deep mode (Very Low Power)
   pwr_critical    // Deep mode (Power Critical, Save data, Power->Off)
};

// Type of module type on enum for local usage, and Canard PnP, info Node
enum Module_Type : uint8_t {
   undefined   = STIMA_MODULE_TYPE_UNDEFINED,
   server_eth  = STIMA_MODULE_TYPE_MASTER_ETH,
   server_gsm  = STIMA_MODULE_TYPE_MASTER_GSM,
   rain        = STIMA_MODULE_TYPE_RAIN,
   th          = STIMA_MODULE_TYPE_TH,
   thr         = STIMA_MODULE_TYPE_THR,
   opc         = STIMA_MODULE_TYPE_OPC,
   leaf        = STIMA_MODULE_TYPE_LEAF,
   wind        = STIMA_MODULE_TYPE_WIND,
   radiation   = STIMA_MODULE_TYPE_SOLAR_RADIATION,
   gas         = STIMA_MODULE_TYPE_GAS,
   power       = STIMA_MODULE_TYPE_POWER_MPPT,
   vwc         = STIMA_MODULE_TYPE_VVC
};

/*!
\struct sensordata_t
\brief constant station data (station name, station height ...) parameters.
*/
typedef struct
{
   char btable[CONSTANTDATA_BTABLE_LENGTH];  //!< table B code for constant station data
   char value[CONSTANTDATA_VALUE_LENGTH];    //!< value of constant station data
} constantdata_t;

/*!
\struct board_metadata_t
\brief metadata archive parameters for pnp or configure assign to remote slave module
*/
typedef struct
{
   uint16_t level1;
   uint16_t level2;
   uint16_t levelType1;
   uint16_t levelType2;
   uint16_t timerangeP1;
   uint16_t timerangeP2;
   uint8_t  timerangePindicator;
} board_metadata_t;

typedef struct
{
   uint8_t can_address;       //!< can sensor's address [0-127]; 100 master, 127 reserved
   uint8_t can_port_id;       //!< port for uavcan services
   uint8_t can_publish_id;    //!< port for uavcan data publication
   uint8_t can_sampletime;    //!< Can_Sampletime if module are in publish mode, time to automatic update and send data
   uint64_t serial_number;    //!< Serial number of board (Used from slave for PnP Assign...)
   Module_Type module_type;   //!< module type (optional also present in unique_id...)
   board_metadata_t metadata[CAN_SENSOR_COUNT_MAX];   //!< module metadata (only used for slave board)
   bool is_configured[CAN_SENSOR_COUNT_MAX];          //!< module is configured ? (only used for slave board)
} board_configuration_t;

typedef struct
{
   uint8_t module_main_version;                          //!< module main version
   uint8_t module_minor_version;                         //!< module minor version
   uint8_t configuration_version;                        //!< module configuration version
   Module_Type module_type;                              //!< module type
   board_configuration_t board_master;                   //!< board configurations local (Master)
   board_configuration_t board_slave[BOARDS_COUNT_MAX];  //!< board configurations remote (Slave)
   uint16_t observation_s;                               //!< observations time in seconds (Can_Sampletime to put into request to remote sensor get value)
   uint16_t report_s;                                    //!< report time in seconds (Request data to slave module, same as Connection MQTT Time)

   // char data_level[DATA_LEVEL_LENGTH];
   char ident[IDENT_LENGTH];
   int32_t longitude;
   int32_t latitude;
   char network[NETWORK_LENGTH];

#if (USE_MQTT)
   uint16_t mqtt_port;                    //!< mqtt server port
   char mqtt_server[MQTT_SERVER_LENGTH];  //!< mqtt server
   char mqtt_root_topic[MQTT_ROOT_TOPIC_LENGTH];   //!< mqtt root path
   char mqtt_maint_topic[MQTT_MAINT_TOPIC_LENGTH];  //!< mqtt maint path
   char mqtt_rpc_topic[MQTT_RPC_TOPIC_LENGTH];      //!< mqtt subscribe topic
   char mqtt_username[MQTT_USERNAME_LENGTH];        //!< username to compose mqtt username (username/stationslug/boardslug)
   char mqtt_password[MQTT_PASSWORD_LENGTH];        //!< mqtt password
   char stationslug[STATIONSLUG_LENGTH];            //!< station slug to compose mqtt username (username/stationslug/boardslug)
   char boardslug[BOARDSLUG_LENGTH];                //!< board slug to compose mqtt username (username/stationslug/boardslug)
   uint8_t client_psk_key[CLIENT_PSK_KEY_LENGTH];
#endif

#if (USE_NTP)
   char ntp_server[NTP_SERVER_LENGTH];  //!< ntp server
#endif

   constantdata_t constantdata[USE_CONSTANTDATA_COUNT];  //!< Constantdata buffer for storing constant station data parameter
   uint8_t constantdata_count;                           //!< configured constantdata number

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_ETH)
   bool is_dhcp_enable;                        //!< dhcp status
   uint8_t ethernet_mac[ETHERNET_MAC_LENGTH];  //!< ethernet mac
   uint8_t ip[ETHERNET_IP_LENGTH];             //!< ip address
   uint8_t netmask[ETHERNET_IP_LENGTH];        //!< netmask
   uint8_t gateway[ETHERNET_IP_LENGTH];        //!< gateway
   uint8_t primary_dns[ETHERNET_IP_LENGTH];    //!< primary dns
#endif

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
   char gsm_apn[GSM_APN_LENGTH];            //!< gsm apn
   char gsm_number[GSM_NUMBER_LENGTH];      //!< gsm number
   char gsm_username[GSM_USERNAME_LENGTH];  //!< gsm username
   char gsm_password[GSM_PASSWORD_LENGTH];  //!< gsm password
#endif
} configuration_t;

// WatchDog Flag type
enum wdt_flag {
   clear = 0,  // Wdt Reset (From WDT Task Controller)
   set = 1,    // Set WDT   (From Application TASK... All OK)
   timer = 2   // Set Timered WDT (From Application long function WDT...)
};

// Task state Flag type
enum task_flag {
   normal = 0,    // Normal operation Task controller
   sleepy = 1,    // Task is in sleep mode or longer wait (Inform WDT controller)
   suspended = 2  // Task is excluded from WDT Controller or Suspended complete
};

// Task Info structure
typedef struct
{
   wdt_flag watch_dog;    // WatchDog of Task
   int32_t watch_dog_ms;  // WatchDog of Task Timer
   uint16_t stack;        // Stack Max Usage Monitor
   task_flag state;       // Long sleep Task
   uint8_t running_pos;   // !=0 (CREATE) Task Started (Generic state of Task)
   uint8_t running_sub;   // Optional SubState of Task
} task_t;

// System status (Status of Stima)
#define MIN_ACQUIRE_GET_ISTANT_VALUE_SEC   10      // Amount of sec. min to get istant value for Display
typedef struct
{
   struct
   {
      uint32_t epoch_sensors_get_istant;           // Date Time epoch for data istant value
      uint32_t epoch_sensors_get_value;            // Date time epoch for data archive value
      uint32_t ptr_time_for_sensors_get_istant;    // Divider time ptr to determine next istant
      uint32_t ptr_time_for_sensors_get_value;     // Divider time ptr to determine next acquire
   } datetime;

   // Info Task && WDT
   task_t tasks[TOTAL_INFO_TASK];

   // Configuration Flag
   struct
   {
      bool is_loaded;
      bool is_saved;
   } configuration;

   // Archived module firmware boards (Version/Revision/Type) Need for Checking Uploading Firmware is avaiable
   // When module_type firmware is present on sd card, version revision and module are charged into this struct
   struct
   {
      Module_Type module_type;
      uint8_t version;
      uint8_t revision;
   } boards_update_avaiable[STIMA_MODULE_TYPE_MAX_AVAIABLE];

  // Connection NET Flag
  struct
  {
      bool is_connected;
      bool is_connecting;
      bool is_disconnected;
      bool is_disconnecting;
      bool is_ppp_estabilished;

      bool is_dns_failed_resolve;

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

  // Command Flag
  struct
  {
      bool do_ntp_synchronization;
      bool do_http_configuration_update;
      bool do_http_firmware_download;
      bool do_mqtt_connect;
   } command;

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
      uint8_t perc_modem_connection_valid;
      uint16_t connection_attempted;
      uint16_t connection_completed;
   } modem;

   // Local data info && value for local simple direct access (LCD/Trace/Config/Check...)
   struct
   {
      bool fw_upgradable;           // Fw upgrade flag
      uint32_t connect_run_epoch;   // Connection start epoch (to inibit for retry start... if something wrong)
      uint8_t number_reboot;        // Total reboot
      uint8_t number_wdt;           // Total WDT
   } data_master;

   // Remote data info && value for local simple direct access (LCD/Trace/Config/Check...)
   #define MAX_DATA_VALUE_MEASURE   3
   struct
   {
      bool fw_upgradable;           // Fw upgradable flag
      bool is_fw_upgrading;         // Fw upgrading current flag
      bool is_new_ist_data_ready;   // New ist data available
      bool is_online;               // Node current on line
      bool maintenance_mode;        // Maintenance mode flag
      Module_Type module_type;      // Type of remote module
      uint8_t heartbeat_transf_id;  // Heart beat recived from run_epoch (check transfer)
      uint16_t heartbeat_rx;        // Heart beat recived from run_epoch (check ok)
      uint16_t heartbeat_rx_err;    // Heart beat recived from run_epoch (count error)
      uint8_t perc_can_comm_err;    // Percent of error comunication TX-RX HeartBeat Status
      // Data value data chanel (istant value)
      rmapdata_t data_value[MAX_DATA_VALUE_MEASURE];
      uint8_t module_revision;      // Revision RMAP
      uint8_t module_version;       // Version RMAP
      // BitField Error or State
      uint8_t bit8StateFlag;        // Bit State Remote module Error
      uint8_t byteStateFlag[3];     // Byte State Remote module Error
   } data_slave[BOARDS_COUNT_MAX];

   // Hw/Sw Flags
   struct
   {
      bool sd_card_ready;        // Flag SD CARD Ready and full functional
      bool inibith_reboot;       // Flag inibition at reboot command (Rebot operation must wait flag is donw) 
      bool display_on;           // Display powered ON (Require Istant and other data)
      bool run_connection;       // Flag indicate connection in run (Connection operation ex.must inibith sleep) 
      bool run_module_configure; // Flag indicate require check module configure (at least one module not configured)
      bool full_wakeup_forced;   // Flag indicate request wakeup forced from RPC remote or local GET Data
      bool full_wakeup_request;  // Flag indicate request wakeup for starting operation with slave in full power mode
      bool file_server_running;  // True if file server are running
      bool cmd_server_running;   // True if command server are running
      bool reg_serever_running;  // True if remote configure or register server procedure over CAN are running
      bool rmap_server_running;  // True if get rmap data from slave module procedure over CAN are running
      bool new_data_to_send;     // True if any data are ready to sent vs MQTT Server
      bool config_empty;         // True if configuration missed on system
      bool clean_rpc;            // True if required disable RPC (examples on MQTT First connection)
      bool power_critical;       // True if power critical mode is activated from MPPT Module
      Power_Mode power_state;    // Current state of power for module StimaV4 (Power strategy...)
   } flags;

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

// System public message task for queue
typedef struct
{
   uint8_t task_dest;
   struct
   {
      uint8_t do_reboot       : 1;  // Request reboot from RPC Other in security mode (waiting operation)
      uint8_t do_update_fw    : 1;  // Request update firmware (node or master) from SD file
      uint8_t do_reload_fw    : 1;  // Request reload firmware really upgradable from SD file
      uint8_t done_reload_fw  : 1;  // Request reload firmware really upgradable from SD file
      uint8_t do_update_all   : 1;  // Request starting update all system (all firmware upgradable) and Reboot any boards
      uint8_t do_inibith      : 1;  // Request inibith sleep
      uint8_t undo_inibith    : 1;  // Remove inibith sleep
      uint8_t do_maint        : 1;  // Request maintenance
      uint8_t undo_maint      : 1;  // Remove maintenance
      uint8_t do_calib_acc    : 1;  // Request set calibration accellerometer
      uint8_t do_remotecfg    : 1;  // Request remote node configuration
      uint8_t do_sleep        : 1;  // Optional param for difference level Sleep
      uint8_t do_cmd          : 1;  // Using param to determine type of message command
      uint8_t done_cmd        : 1;  // Using param to determine type of message response
   } command;
   uint32_t param;  // 32 Bit for generic data or casting to pointer

} system_message_t;

// Archive data type of RMAP generic data block for queue
typedef struct
{
   Module_Type module_type;   // 8 Bit Module_type for reitepreter cast block of data
   uint32_t date_time;        // 32 Bit date time epoch_style d
   uint8_t  block[RMAP_DATA_MAX_ELEMENT_SIZE];   // RMAP Type block of data with info block

} rmap_archive_data_t;

// RMAP data archive response block from memory task (SD) to request
typedef struct
{
   struct
   {
      uint8_t done_synch    : 1;  // Done set pointer request
      uint8_t done_get_data : 1;  // Done get data (next data request)
      uint8_t end_of_data   : 1;  // Data is last on queue (next request give an error)
      uint8_t event_error   : 1;  // Signal error generic on read/request data
   } result;
   rmap_archive_data_t rmap_data; // RMAP Archive data value on response

} rmap_get_response_t;

// Older Archive Backup data type of RMAP generic data block for queue
typedef struct
{
   uint32_t date_time;        // 32 Bit date time epoch_style d
   uint8_t  block[RMAP_BACKUP_DATA_MAX_ELEMENT_SIZE];   // RMAP Older Type block of data transparent MQTT

} rmap_backup_data_t;

// RMAP data archive request from any task to memory task (SD)
typedef struct
{
   struct
   {
      uint8_t do_synch_ptr  : 1;  // Request synch pointer data rmap START with param (set pointer)
      uint8_t do_end_ptr    : 1;  // Request synch pointer data rmap END with param (set pointer)
      uint8_t do_get_data   : 1;  // Get first data avaiable and set pointer to next data
      uint8_t do_save_ptr   : 1;  // Request to Save Pointer Data (Optional with All other Request)
   } command;
   uint32_t param;  // 32 Bit for generic data or casting to pointer

} rmap_get_request_t;

// File type enum for queue file get/put block
enum file_block_type {
   file_name = 0,     // Block is name file (starting block, create file)
   data_chunck = 1,   // Block is data block (file...)
   end_of_file = 2,   // Block is end of file (Normal)
   ctrl_checksum = 3  // Block is end of file (Required checksum control)
};

// Queue for response put file operation
typedef struct
{
   bool done_operation;
   bool error_operation;

} file_put_response_t;

// Queue message (data to add) for file put
typedef struct
{
   file_block_type   block_type;
   uint16_t          block_lenght;
   uint8_t           block[FILE_PUT_DATA_BLOCK_SIZE];
} file_put_request_t;

// Queue for file request read data (block and file)
typedef struct
{
   uint8_t board_id;       // Board ID board_id(For create a multi server file rapid access. More open file server 0xFF unused)
   char *file_name;        // Name file (need with first block block_id = 0. Start Reading session file)
   bool block_read_next;   // simply fast read a next block from last
   uint16_t block_id;      // Seek a fixed block into a file

} file_get_request_t;

// Queue for response put file operation
typedef struct
{
   bool done_operation;
   bool error_operation;
   uint16_t block_lenght;
   uint8_t  block[FILE_GET_DATA_BLOCK_SIZE];

} file_get_response_t;

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
