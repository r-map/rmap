/**@file local_typedef.h */

/*********************************************************************
<h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>
Moreno Gasperini <m.gasperini@digiteco.it>

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

/// @brief Type of module type on enum for local usage, and Canard PnP, info Node
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
   vwc         = STIMA_MODULE_TYPE_VWC
};

/*!
\struct sensordata_t
\brief constant station data (station name, station height ...) parameters.
*/
typedef struct
{
   char btable[CONSTANTDATA_BTABLE_LENGTH];  ///< table B code for constant station data
   char value[CONSTANTDATA_VALUE_LENGTH];    ///< value of constant station data
} constantdata_t;

/*!
\struct board_metadata_t
\brief metadata archive parameters for pnp or configure assign to remote slave module
*/
typedef struct
{
   uint16_t level1;              ///< Metadata RMAP level1
   uint16_t level2;              ///< Metadata RMAP level2
   uint16_t levelType1;          ///< Metadata RMAP levelType1
   uint16_t levelType2;          ///< Metadata RMAP levelType2
   uint16_t timerangeP1;         ///< Metadata RMAP timerangeP1
   uint16_t timerangeP2;         ///< Metadata RMAP timerangeP2
   uint8_t  timerangePindicator; ///< Metadata RMAP timerangeIndicator
} board_metadata_t;

/// @brief board_configuration_t CAN Board configuration
typedef struct
{
   uint8_t can_address;       ///< can sensor's address [0-127]; 100 master, 127 reserved
   uint8_t can_port_id;       ///< port for uavcan services
   uint8_t can_publish_id;    ///< port for uavcan data publication
   uint8_t can_sampletime;    ///< Can_Sampletime if module are in publish mode, time to automatic update and send data
   uint64_t serial_number;    ///< Serial number of board (Used from slave for PnP Assign...)
   Module_Type module_type;   ///< module type
   char boardslug[BOARDSLUG_LENGTH];                  ///< boardslug (module name)boardslug
   board_metadata_t metadata[CAN_SENSOR_COUNT_MAX];   ///< module metadata (only used for slave board)
   bool is_configured[CAN_SENSOR_COUNT_MAX];          ///< module is configured ? (only used for slave board)
} board_configuration_t;

/// @brief Enable Monitor MQTT Error exit server response
#define NETWORK_FLAG_MONITOR_MQTT   0x01

/// @brief configuration_t General StimaV4 system Configuration
typedef struct
{
   uint8_t module_main_version;                          ///< module main version
   uint8_t module_minor_version;                         ///< module minor version
   uint8_t configuration_version;                        ///< module configuration version
   Module_Type module_type;                              ///< module type
   board_configuration_t board_master;                   ///< board configurations local (Master)
   board_configuration_t board_slave[BOARDS_COUNT_MAX];  ///< board configurations remote (Slave)
   uint16_t observation_s;                               ///< observations time in seconds (Can_Sampletime to put into request to remote sensor get value)
   uint16_t report_s;                                    ///< report time in seconds (Request data to slave module, same as Connection MQTT Time)

   char ident[IDENT_LENGTH];                             ///< RMAP ident
   int32_t longitude;                                    ///< RMAP longitude
   int32_t latitude;                                     ///< RMAP latitude
   char network[NETWORK_LENGTH];                         ///< RMAP network

#if (USE_MQTT)
   uint16_t mqtt_port;                              ///< mqtt server port
   char mqtt_server[MQTT_SERVER_LENGTH];            ///< mqtt server
   char mqtt_root_topic[MQTT_ROOT_TOPIC_LENGTH];    ///< mqtt root path
   char mqtt_maint_topic[MQTT_MAINT_TOPIC_LENGTH];  ///< mqtt maint path
   char mqtt_rpc_topic[MQTT_RPC_TOPIC_LENGTH];      ///< mqtt subscribe topic
   char mqtt_username[MQTT_USERNAME_LENGTH];        ///< username to compose mqtt username (username/stationslug/boardslug)
   char mqtt_password[MQTT_PASSWORD_LENGTH];        ///< mqtt password
   char stationslug[STATIONSLUG_LENGTH];            ///< station slug to compose mqtt username (username/stationslug/boardslug)
   uint8_t client_psk_key[CLIENT_PSK_KEY_LENGTH];   ///< RMAP assigned client PSK Key for remote connection
#endif

#if (USE_NTP)
   char ntp_server[NTP_SERVER_LENGTH];  ///< ntp server
#endif

   constantdata_t constantdata[USE_CONSTANTDATA_COUNT];  ///< Constantdata buffer for storing constant station data parameter
   uint8_t constantdata_count;                           ///< configured constantdata number

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_ETH)
   bool is_dhcp_enable;                        ///< dhcp status
   uint8_t ethernet_mac[ETHERNET_MAC_LENGTH];  ///< ethernet mac
   uint8_t ip[ETHERNET_IP_LENGTH];             ///< ip address
   uint8_t netmask[ETHERNET_IP_LENGTH];        ///< netmask
   uint8_t gateway[ETHERNET_IP_LENGTH];        ///< gateway
   uint8_t primary_dns[ETHERNET_IP_LENGTH];    ///< primary dns
#endif

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
   char gsm_apn[GSM_APN_LENGTH];            ///< gsm apn
   char gsm_number[GSM_NUMBER_LENGTH];      ///< gsm number
   char gsm_username[GSM_USERNAME_LENGTH];  ///< gsm username
   char gsm_password[GSM_PASSWORD_LENGTH];  ///< gsm password
   uint8_t monitor_flags;                   ///< using monitor function flags operation param
   uint8_t network_type;                    ///< prefered type of network radio gsm
   uint8_t network_regver;                  ///< end of validation register network ready connect
   char network_order[GSM_ORDER_NETWORK_LENGTH];  ///< network preferred type order list
#endif
} configuration_t;

/// @brief WatchDog enum Flag type (set type of calling watchDog on Task)
enum wdt_flag {
   clear = 0,  ///< Wdt Reset (From WDT Task Controller)
   set = 1,    ///< Set WDT   (From Application TASK... All OK)
   timer = 2   ///< Set Timered WDT (From Application long function WDT...)
};

/// @brief WatchDog enum Task state Flag type (set mode for relative Task)
enum task_flag {
   normal = 0,    ///< Normal operation Task controller
   sleepy = 1,    ///< Task is in sleep mode or longer wait (Inform WDT controller)
   suspended = 2  ///< Task is excluded from WDT Controller or Suspended complete
};

/// @brief task_t Task Info status structure for StimaV4
typedef struct
{
   wdt_flag watch_dog;    ///< WatchDog of Task
   int32_t watch_dog_ms;  ///< WatchDog of Task Timer
   uint16_t stack;        ///< Stack Max Usage Monitor
   task_flag state;       ///< Long sleep Task
   uint8_t running_pos;   ///< !=0 (CREATE) Task Started (Generic state of Task)
   uint8_t running_sub;   ///< Optional SubState of Task
} task_t;

/// @brief systemstatus_t General StimaV4 system Status
typedef struct
{
   ///< Datetime parameter
   struct
   {
      uint32_t epoch_sensors_get_istant;           ///< Date Time epoch for data istant value
      uint32_t epoch_sensors_get_value;            ///< Date time epoch for data archive value
      uint32_t epoch_connect_run;                  ///< Connection start epoch (to inibit for retry start... if something wrong)
      uint32_t ptr_time_for_sensors_get_istant;    ///< Divider time ptr to determine next istant
      uint32_t ptr_time_for_sensors_get_value;     ///< Divider time ptr to determine next acquire
      uint32_t epoch_mqtt_last_connection;         ///< Date Time epoch for last valid mqtt connection
   } datetime;

   ///< Info Task && WDT
   task_t tasks[TOTAL_INFO_TASK];

   ///< Configuration Flag
   struct
   {
      bool is_loaded;   ///< Configuration is loaded
      bool is_saved;    ///< Configuratuon is saved
   } configuration;

   ///< Archived module firmware boards (Version/Revision/Type) Need for Checking Uploading Firmware is avaiable
   ///< When module_type firmware is present on sd card, version revision and module are charged into this struct
   struct
   {
      Module_Type module_type;   ///< Module type for board
      uint8_t version;           ///< Version of board
      uint8_t revision;          ///< Revision of board
   } boards_update_avaiable[STIMA_MODULE_TYPE_MAX_AVAIABLE];

   ///< Connection NET Flag
   struct
   {
      bool is_connected;               ///< Modem is connected
      bool is_connecting;              ///< Modem is in connection sequence
      bool is_disconnected;            ///< Modem is disconnected
      bool is_disconnecting;           ///< Modem is in disconnection sequence
      bool is_ppp_estabilished;        ///< PPP Connection ready

      bool is_dns_failed_resolve;      ///< DNS Resolve error

      bool is_ntp_synchronized;        ///< RTC with NTP is now Syncronized
      bool is_ntp_synchronizing;       ///< Try to access server NTP

      bool is_mqtt_connected;          ///< MQTT Server connected
      bool is_mqtt_connecting;         ///< Try to connect with MQTT Server
      bool is_mqtt_subscribed;         ///< MQTT Client is subscribed
      bool is_mqtt_publishing;         ///< MQTT Client is publishing data
      bool is_mqtt_publishing_end;     ///< MQTT Client End of all data to publish
      bool is_mqtt_disconnected;       ///< MQTT Server is disconnected
      bool is_mqtt_disconnecting;      ///< MQTT Server is in disconnecting sequence

      bool is_http_configuration_updated;    ///< HTTP Server configuration updated
      bool is_http_configuration_updating;   ///< Try to connect HTTP Server for download configuration
      bool is_http_firmware_upgraded;        ///< HTTP Server firmware donloaded
      bool is_http_firmware_upgrading;       ///< Try to connect HTTP Server for download firmware

      uint32_t mqtt_data_published;          ///< Amount of MQTT Data published
      uint8_t  mqtt_data_exit_error;         ///< Index of MQTT Data procedure published with exit error code
   } connection;

   ///< Command Flag to Task NET connection
   struct
   {
      bool do_ntp_synchronization;           ///< Request a NTP Syncronifation
      bool do_http_configuration_update;     ///< Request a HTTP configuration download
      bool do_http_firmware_download;        ///< Request a HTTP firmware download
      bool do_mqtt_connect;                  ///< Request a MQTT Connection for publish data or system state
   } command;

   ///< GSM Flag
   struct
   {
      // Signal status for last connect
      uint8_t rssi;          ///< RSSI, signal quality Index of modem
      uint8_t ber;           ///< BER, bit error Index of modem
      uint8_t creg_n;        ///< Registration for GSM
      uint8_t creg_stat;     ///< State Registration for GSM
      uint8_t cgreg_n;       ///< Registration for GPRS
      uint8_t cgreg_stat;    ///< State Registration for GPRS
      uint8_t cereg_n;       ///< Registration for EUTRAN
      uint8_t cereg_stat;    ///< State Registration for EUTRAN
      // Indicator state connection sequence (Ok/err)
      uint8_t perc_modem_connection_valid;   ///< Number of valid percentage connection
      uint16_t connection_attempted;         ///< Modem connection started
      uint16_t connection_completed;         ///< Modem connection estabilished
   } modem;

   ///< Local data info && value for local simple direct access (LCD/Trace/Config/Check...)
   struct
   {
      bool fw_upgradable;           ///< Fw upgrade flag
      uint8_t number_reboot;        ///< Total reboot
      uint8_t number_wdt;           ///< Total WDT
   } data_master;

   ///< Remote data info && value for local simple direct access (LCD/Trace/Config/Check...)
   #define MAX_DATA_VALUE_MEASURE   3
   struct
   {
      bool fw_upgradable;           ///< Fw upgradable flag
      bool is_fw_upgrading;         ///< Fw upgrading current flag
      bool is_new_ist_data_ready;   ///< New ist data available
      bool is_online;               ///< Node current on line
      bool maintenance_mode;        ///< Maintenance mode flag
      Module_Type module_type;      ///< Type of remote module
      uint8_t heartbeat_transf_id;  ///< Heart beat recived from run_epoch (check transfer)
      uint16_t heartbeat_rx;        ///< Heart beat recived from run_epoch (check ok)
      uint16_t heartbeat_rx_err;    ///< Heart beat recived from run_epoch (count error)
      uint8_t perc_can_comm_err;    ///< Percent of error comunication TX-RX HeartBeat Status
      rmapdata_t data_value[MAX_DATA_VALUE_MEASURE];  ///< Data value data chanel (istant value)
      uint8_t module_revision;      ///< Revision RMAP
      uint8_t module_version;       ///< Version RMAP
      // BitField Error or State
      uint8_t bit8StateFlag;        ///< Bit State Remote module Error
      uint8_t byteStateFlag[3];     ///< Byte State Remote module Error
   } data_slave[BOARDS_COUNT_MAX];

   ///< Hw/Sw Flags
   struct
   {
      bool sd_card_ready;        ///< Flag SD CARD Ready and full functional
      bool inibith_reboot;       ///< Flag inibition at reboot command (Rebot operation must wait flag is donw) 
      bool display_on;           ///< Display powered ON (Require Istant and other data)
      bool run_connection;       ///< Flag indicate connection in run (Connection operation ex.must inibith sleep) 
      bool run_module_configure; ///< Flag indicate require check module configure (at least one module not configured)
      bool full_wakeup_forced;   ///< Flag indicate request wakeup forced from RPC remote or local GET Data
      bool full_wakeup_request;  ///< Flag indicate request wakeup for starting operation with slave in full power mode
      bool file_server_running;  ///< True if file server are running
      bool cmd_server_running;   ///< True if command server are running
      bool reg_server_running;   ///< True if register server are running
      bool cfg_server_running;   ///< True if remote configure server procedure over CAN are running
      bool rmap_server_running;  ///< True if get rmap data from slave module procedure over CAN are running
      bool new_data_to_send;     ///< True if any data are ready to sent vs MQTT Server
      bool new_start_connect;    ///< True if passed time of report data (synch wit new_data)
      bool config_empty;         ///< True if configuration missed on system
      bool clean_session;        ///< True if required clean session (examples on MQTT First connection remove RPC message)
      bool power_warning;        ///< True if power warning mode is activated from MPPT Module
      bool power_critical;       ///< True if power critical mode is activated from MPPT Module
      Power_Mode power_state;    ///< Current state of power for module StimaV4 (Power strategy...)
      ///< Flags error state for LCD Display
      bool ppp_error;            ///< error in connection PPP
      bool dns_error;            ///< error in resolve DNS
      bool ntp_error;            ///< error in connection NTP Server
      bool mqtt_error;           ///< error in connection MQTT Server
      bool http_error;           ///< error in connection HTTP Server
      bool mqtt_wait_link;       ///< Mqtt waiting response
      bool http_wait_cfg;        ///< Http waiting resonse (config page)
      bool http_wait_fw;         ///< Http waiting resonse (firmware page)
      bool fw_updating;          ///< Firmware updating method called
      uint8_t pnp_request;       ///< Cyphal pnp request recived (TYPE_OF_MODULE)
      uint8_t gsm_rssi;          ///< Gsm RSSI Signal quality index value
   } flags;

} system_status_t;

/// @brief struct called for Queue Request connection (GSM/NET)
typedef struct
{
   bool do_connect;        ///< need connection begin
   bool do_disconnect;     ///< need disconnection

   bool do_ntp_sync;       ///< require syncronization with NTP Server

   bool do_mqtt_connect;   ///< require connection to MQTT Server

   bool do_http_get_configuration;  ///< Try to get configuration with HTTP connection
   bool do_http_get_firmware;       ///< Try to get new firmware with HTTP Connection

} connection_request_t;

/// @brief Queue for response to supervisor for Connetcion (GSM/NET)
typedef struct
{
   bool error_connected;         ///< error in connection request
   bool done_connected;          ///< connection executed

   bool error_disconnected;      ///< error in disconnection sequence
   bool done_disconnected;       ///< disconnection executed

   bool done_ntp_synchronized;   ///< Ntp is now syncronized
   bool error_ntp_synchronized;  ///< error in syncronization NTP

   bool done_mqtt_connected;     ///< MQTT is now connected
   bool error_mqtt_connected;    ///< error in connection MQTT

   bool done_http_configuration_getted;   ///< Configuration recived from http chanel
   bool error_http_configuration_getted;  ///< error in connection http get configuration

   bool done_http_firmware_getted;        ///< Firmware donloaded from http chanel
   bool error_http_firmware_getted;       ///< error in connection http get girmware

   uint16_t number_of_mqtt_data_sent;     ///< Index number of data sended to MQTT Server

} connection_response_t;

// The parameters depend on the request and may vary in meaning depending on the type of call
// Specific ID Param generic send/recive queue message
#define CMD_PARAM_REQUIRE_RESPONSE  0xF0
#define CMD_PARAM_MASTER_ADDRESS    0xFF

// Register value to set remote register on RPC request
// Register are to be setted with corect type configured (otherwise Uavcan retrieve an error on setup)
#define RVS_TYPE_UNKNOWN            0x00
#define RVS_TYPE_EMPTY              0x01
#define RVS_TYPE_BIT                0x02
#define RVS_TYPE_INTEGER_8          0x03
#define RVS_TYPE_INTEGER_16         0x04
#define RVS_TYPE_INTEGER_32         0x05
#define RVS_TYPE_INTEGER_64         0x06
#define RVS_TYPE_NATURAL_8          0x07
#define RVS_TYPE_NATURAL_16         0x08
#define RVS_TYPE_NATURAL_32         0x09
#define RVS_TYPE_NATURAL_64         0x0A
#define RVS_TYPE_REAL_16            0x0B
#define RVS_TYPE_REAL_32            0x0C
#define RVS_TYPE_REAL_64            0x0D
#define RVS_TYPE_STRING             0x0E
#define RVS_TYPE_UNSTRUCTURED       0x0F
#define RVS_TYPE_ONLY_TYPE          0x0F
#define RVS_TYPE_IS_ARRAY           0x10

/// @brief System public message task for queue
typedef struct
{
   uint8_t task_dest; ///< ID Task destination of message
   ///< struct of command
   struct command
   {
      uint8_t do_reboot       : 1;  ///< Request reboot from RPC Other in security mode configure (waiting operation)
      uint8_t do_update_fw    : 1;  ///< Request update firmware (node or master) from SD file
      uint8_t do_reload_fw    : 1;  ///< Request reload firmware really upgradable from SD file
      uint8_t done_reload_fw  : 1;  ///< Response reload firmware really upgradable from SD file
      uint8_t do_reinit_fw    : 1;  ///< Request reinit firmware really upgradable from SD file (destroy all file struct)
      uint8_t done_reinit_fw  : 1;  ///< Response reinit firmware really upgradable from SD file (destroy all file struct)
      uint8_t do_update_all   : 1;  ///< Request starting update all system (all firmware upgradable) and Reboot any boards
      uint8_t do_trunc_sd     : 1;  ///< Request truncate and reinit data into SD Card
      uint8_t done_trunc_sd   : 1;  ///< Response truncate and reinit data into SD Card
      uint8_t do_inibith      : 1;  ///< Request inibith sleep
      uint8_t undo_inibith    : 1;  ///< Remove inibith sleep
      uint8_t do_maint        : 1;  ///< Request maintenance
      uint8_t undo_maint      : 1;  ///< Remove maintenance
      uint8_t do_calib_acc    : 1;  ///< Request set calibration accellerometer
      uint8_t do_factory      : 1;  ///< Request reset register uavcan to factory value (complete reset remote node)
      uint8_t do_reset_flags  : 1;  ///< Request reset remote signal/error flags
      uint8_t do_reboot_node  : 1;  ///< Request reboot from RPC Other in direct mode (without regulation configure node)
      uint8_t do_remote_cfg   : 1;  ///< Request update remote node configuration
      uint8_t do_remote_reg   : 1;  ///< Request update remote node register
      uint8_t do_sleep        : 1;  ///< Optional param for difference level Sleep
      uint8_t do_cmd          : 1;  ///< Using param to determine type of message command
      uint8_t done_cmd        : 1;  ///< Using param to determine type of message response
   } command;
   uint8_t node_id;  ///< Optional node_id destination message
   uint8_t param;    ///< Optional param message
   ///< union of optional parameter value
   union value
   {
      bool     bool_val;   ///< bool for generic data user access
      uint8_t  uint8_val;  ///< Uint 8 Bit for generic data user access
      uint16_t uint16_val; ///< Uint 16 Bit for generic data user access
      uint32_t uint32_val; ///< Uint 32 Bit for generic data user access
      int8_t   int8_val;   ///< int 8 Bit for generic data user access
      int16_t  int16_val;  ///< int 16 Bit for generic data user access
      int32_t  int32_val;  ///< int 32 Bit for generic data user access
      float    float_val;  ///< Float for generic data user access
   } value;
   char message[64]; ///< Optional message string data user access

} system_message_t;

/// @brief Archive data type of RMAP generic data block for queue
typedef struct
{
   Module_Type module_type;   ///< 8 Bit Module_type for reitepreter cast block of data
   uint32_t date_time;        ///< 32 Bit date time epoch_style d
   uint8_t  block[RMAP_DATA_MAX_ELEMENT_SIZE];   ///< RMAP Type block of data with info block

} rmap_archive_data_t;

/// @brief RMAP data archive response block from memory task (SD) to request
typedef struct
{
   ///< Type of response
   struct
   {
      uint8_t done_synch    : 1;  ///< Done set pointer request
      uint8_t done_get_data : 1;  ///< Done get data (next data request)
      uint8_t end_of_data   : 1;  ///< Data is last on queue (next request give an error)
      uint8_t event_error   : 1;  ///< Signal error generic on read/request data
   } result;
   rmap_archive_data_t rmap_data; ///< RMAP Archive data value on response

} rmap_get_response_t;

/// @brief Older Archive Backup data type of RMAP generic data block for queue
typedef struct
{
   uint32_t date_time;        ///< 32 Bit date time epoch_style d
   uint8_t  block[RMAP_BACKUP_DATA_MAX_ELEMENT_SIZE];   ///< RMAP Older Type block of data transparent MQTT

} rmap_backup_data_t;

/// @brief RMAP data archive request from any task to memory task (SD)
typedef struct
{
   ///< Command struct
   struct
   {
      uint8_t do_synch_ptr    : 1;  ///< Request synch pointer data rmap START with param (set pointer)
      uint8_t do_end_ptr      : 1;  ///< Request synch pointer data rmap END with param (set pointer)
      uint8_t do_get_data     : 1;  ///< Get first data avaiable and set pointer to next data
      uint8_t do_save_ptr     : 1;  ///< Request to Save Pointer Data (Optional with All other Request)
      uint8_t do_reset_ptr    : 1;  ///< Request to Reset Pointer Data
      uint8_t do_previous_ptr : 1;  ///< Request to Reset previous position (something fail in send data)
   } command;
   uint32_t param;  ///< 32 Bit for generic data or casting to pointer

} rmap_get_request_t;

/// @brief File type enum for queue file get/put block
enum file_block_type {
   file_name = 0,     ///< Block is name file (starting block, create file)
   data_chunck = 1,   ///< Block is data block (file...)
   kill_file = 2,     ///< Block is command for kill file (remove for something wrong)
   end_of_file = 3,   ///< Block is end of file (Normal)
   ctrl_checksum = 4  ///< Block is end of file (Required checksum control)
};

/// @brief Queue for response put file operation
typedef struct
{
   bool done_operation;    ///< Done operation with queue SD
   bool error_operation;   ///< Error operation with queue SD

} file_put_response_t;

/// @brief Queue message (data to add) for file put
typedef struct
{
   file_block_type   block_type;    ///< type of block data
   uint16_t          block_lenght;  ///< lenght of block
   uint8_t           block[FILE_PUT_DATA_BLOCK_SIZE]; ///< block data
} file_put_request_t;

/// @brief Queue for file request read data (block and file)
typedef struct
{
   uint8_t board_id;       ///< Board ID board_id(For create a multi server file rapid access. More open file server 0xFF unused)
   char *file_name;        ///< Name file (need with first block block_id = 0. Start Reading session file)
   bool block_read_next;   ///< simply fast read a next block from last
   uint16_t block_id;      ///< Seek a fixed block into a file

} file_get_request_t;

/// @brief Queue for response put file operation
typedef struct
{
   bool done_operation;    ///< Operation is done
   bool error_operation;   ///< Operation is error
   uint16_t block_lenght;  ///< lenght of block
   uint8_t  block[FILE_GET_DATA_BLOCK_SIZE]; ///< Block of data

} file_get_response_t;

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
