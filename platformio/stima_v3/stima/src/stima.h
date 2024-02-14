/**@file stima.h */

/*********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>
Marco Baldinetti <m.baldinetti@digiteco.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#ifndef _STIMA_H
#define _STIMA_H

#include <stima_module.h>
#include "stima-config.h"
#include <typedef.h>
#include <registers.h>
#include <registers.h>
#include <registers-master.h>
#include <debug.h>
#include <ArduinoLog.h>
#include <StreamUtils.h>

#include <i2c_config.h>
#include <json_config.h>
#include <ntp_config.h>
#include <constantdata_config.h>
#include <gsm_config.h>

#if (USE_LCD)
#include <Wire.h>
#include <lcd_config.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c LCD i/o class header
#endif

#ifdef ARDUINO_ARCH_AVR
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#endif

#include <SPI.h>
#include <Wire.h>

#if (USE_SDCARD)
#include <sdcard_utility.h>
#endif

#include <pcf8563.h>

#if (USE_NTP)
#include <ntp.h>
#endif

#include <rmap_utility.h>
#if (USE_JSON)
#include <json_utility.h>
#endif
#include <TimeLib.h>
#include <SensorDriver.h>
#include <i2c_utility.h>
#include <arduinoJsonRPC.h>

#if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
#include <ethernet_config.h>
#include <Ethernet2.h>
#if (USE_MQTT)
#include <IPStack.h>
#endif

#elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
#include <gsm_config.h>
#include <sim800Client.h>
#if (USE_MQTT)
#include <Sim800IPStack.h>
#endif

#endif

#if (USE_MQTT)
#include <Countdown.h>
#include <MQTTClient.h>

#endif

/*********************************************************************
* TYPEDEF
*********************************************************************/

/*!
\struct sensordata_t
\brief constant station data (station name, station height ...) parameters.
*/
typedef struct {
   char btable[CONSTANTDATA_BTABLE_LENGTH];                 //!< table B code for constant station data
   char value[CONSTANTDATA_VALUE_LENGTH];                   //!< value of constant station data
} constantdata_t;

/*!
\struct configuration_t
\brief EEPROM saved configuration.
*/
typedef struct {
   uint8_t module_main_version;                             //!< module main version
   uint8_t module_configuration_version;                    //!< module configuration version
   uint8_t module_type;                                     //!< module type

   #if (USE_MQTT)
   uint16_t mqtt_port;                                      //!< mqtt server port
   char mqtt_server[MQTT_SERVER_LENGTH];                    //!< mqtt server
   char mqtt_root_topic[MQTT_ROOT_TOPIC_LENGTH];            //!< mqtt root path
   char mqtt_maint_topic[MQTT_MAINT_TOPIC_LENGTH];          //!< mqtt maint path
   char mqtt_rpc_topic[MQTT_RPC_TOPIC_LENGTH];              //!< mqtt subscribe topic
   char mqtt_username[MQTT_USERNAME_LENGTH];                //!< username to compose mqtt username (username/stationslug/boardslug)
   char mqtt_password[MQTT_PASSWORD_LENGTH];                //!< mqtt password
   char stationslug[STATIONSLUG_LENGTH];                    //!< station slug to compose mqtt username (username/stationslug/boardslug)
   char boardslug[BOARDSLUG_LENGTH];                        //!< board slug to compose mqtt username (username/stationslug/boardslug)
   #endif

   #if (USE_NTP)
   char ntp_server[NTP_SERVER_LENGTH];                      //!< ntp server
   #endif

   sensor_t sensors[SENSORS_MAX];                           //!< SensorDriver buffer for storing sensors parameter
   uint8_t sensors_count;                                   //!< configured sensors number
   uint16_t report_seconds;                                 //!< seconds for report values

   constantdata_t constantdata[USE_CONSTANTDATA_COUNT];     //!< Constantdata buffer for storing constant station data parameter
   uint8_t constantdata_count;                              //!< configured constantdata number
  
   #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
   bool is_dhcp_enable;                                     //!< dhcp status
   uint8_t ethernet_mac[ETHERNET_MAC_LENGTH];               //!< ethernet mac
   uint8_t ip[ETHERNET_IP_LENGTH];                          //!< ip address
   uint8_t netmask[ETHERNET_IP_LENGTH];                     //!< netmask
   uint8_t gateway[ETHERNET_IP_LENGTH];                     //!< gateway
   uint8_t primary_dns[ETHERNET_IP_LENGTH];                 //!< primary dns

   #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
   char gsm_apn[GSM_APN_LENGTH];                            //!< gsm apn
   char gsm_username[GSM_USERNAME_LENGTH];                  //!< gsm username
   char gsm_password[GSM_PASSWORD_LENGTH];                  //!< gsm password

   #endif
} configuration_t;


/*********************************************************************
* TYPEDEF for Finite State Machine
*********************************************************************/

/*!
\enum state_t
\brief Main loop finite state machine.
*/
typedef enum {
   INIT,                      //!< init tasks and sensors
   #if (USE_POWER_DOWN)
   ENTER_POWER_DOWN,          //!< if no task is running, activate power down
   #endif
   TASKS_EXECUTION,           //!< execute active tasks
   END,                        //!< go to ENTER_POWER_DOWN or TASKS_EXECUTION
   REBOOT                     //!< reboot the machine
} state_t;

/*!
\enum supervisor_state_t
\brief Supervisor task finite state machine.
*/
typedef enum {
   SUPERVISOR_INIT,                          //!< init task variables
   SUPERVISOR_CONNECTION_LEVEL_TASK,         //!< enable hardware related tasks for doing connection
   SUPERVISOR_WAIT_CONNECTION_LEVEL_TASK,    //!< enable hardware related tasks for doing connection
   SUPERVISOR_TIME_LEVEL_TASK,               //!< enable time task for sync time with ntp server
   SUPERVISOR_MANAGE_LEVEL_TASK,             //!< enable tasks for manage data (mqtt)
   SUPERVISOR_TEST_SDCARD,
   SUPERVISOR_END,                           //!< performs end operations and deactivate task
   SUPERVISOR_WAIT_STATE                     //!< non-blocking waiting time
} supervisor_state_t;

#if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
/*!
\enum ethernet_state_t
\brief Ethernet task finite state machine.
*/
typedef enum {
   ETHERNET_INIT,             //!< init task variables
   ETHERNET_CONNECT,          //!< begin ethernet operations
   ETHERNET_OPEN_UDP_SOCKET,  //!< open udp socket
   ETHERNET_END,              //!< performs end operations and deactivate task
   ETHERNET_WAIT_STATE        //!< non-blocking waiting time
} ethernet_state_t;

#elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
/*!
\enum gsm_state_t
\brief GSM task finite state machine.
*/
typedef enum {
   GSM_INIT,                  //!< init task variables
   GSM_SWITCH_ON,             //!< gsm power on
   GSM_AUTOBAUD,              //!< gsm autobaud procedure
   GSM_SETUP,                 //!< gsm setup
   GSM_START_CONNECTION,      //!< gsm open connection
   GSM_CHECK_OPERATION,       //!< check operations (ntp or mqtt)
   GSM_OPEN_UDP_SOCKET,       //!< open udp socket for ntp sync
   GSM_SUSPEND,               //!< wait other tasks for complete its operations with gsm
   GSM_STOP_CONNECTION,       //!< gsm close connection
   GSM_WAIT_FOR_SWITCH_OFF,   //!< wait gsm for power off
   GSM_SWITCH_OFF,            //!< gsm power off
   GSM_END,                   //!< performs end operations and deactivate task
   GSM_WAIT_STATE             //!< non-blocking waiting time
} gsm_state_t;

#endif

/*!
\enum sensors_reading_state_t
\brief Sensors reading task finite state machine.
*/
typedef enum {
   SENSORS_READING_INIT,            //!< init task variables
   SENSORS_SETUP_CHECK,             //!< check errors and if required try a sensor setup
   SENSORS_READING_PREPARE,         //!< prepare sensor
   SENSORS_READING_IS_PREPARED,     //!< check if the sensor has been prepared
   SENSORS_READING_GET,             //!< read and get values from sensor
   SENSORS_READING_IS_GETTED,       //!< check if the sensor has been readed
   SENSORS_READING_READ,            //!< intermediate state (future implementation...)
   SENSORS_READING_NEXT,            //!< go to next sensor
   SENSORS_READING_END,             //!< performs end operations and deactivate task
   SENSORS_READING_WAIT_STATE       //!< non-blocking waiting time
} sensors_reading_state_t;

/*!
\enum time_state_t
\brief Time task finite state machine.
*/
typedef enum {
   TIME_INIT,                    //!< init task variables
   TIME_SEND_ONLINE_REQUEST,     //!< send ntp request
   TIME_WAIT_ONLINE_RESPONSE,    //!< wait ntp response
   TIME_SET_SYNC_NTP_PROVIDER,   //!< set ntp time
   TIME_SET_SYNC_RTC_PROVIDER,   //!< set rtc time
   TIME_END,                     //!< performs end operations and deactivate task
   TIME_WAIT_STATE               //!< non-blocking waiting time
} time_state_t;

#if (USE_SDCARD)
/*!
\enum data_saving_state_t
\brief Data saving task finite state machine.
*/
typedef enum {
   DATA_SAVING_INIT,          //!< init task variables
   DATA_SAVING_OPEN_SDCARD,   //!< if not already open
   DATA_SAVING_OPEN_FILE,     //!< open sdcard file for saving new data
   DATA_SAVING_SENSORS_LOOP,  //!< loop on the sensors
   DATA_SAVING_DATA_LOOP,     //!< loop on the sensors data
   DATA_SAVING_WRITE_FILE,    //!< write data on sdcard file
   DATA_SAVING_CLOSE_FILE,    //!< close sdcard file
   DATA_SAVING_END,           //!< performs end operations and deactivate task
   DATA_SAVING_WAIT_STATE     //!< non-blocking waiting time
} data_saving_state_t;
#endif

#if (USE_MQTT)
/*!
\enum mqtt_state_t
\brief MQTT task finite state machine.
*/
typedef enum {
   MQTT_INIT,              //!< init task variables

   MQTT_OPEN_SDCARD,       //!< if not already open
   MQTT_OPEN_PTR_FILE,     //!< open mqtt data pointer sdcard file
   MQTT_PTR_READ,          //!< read mqtt data pointer
   MQTT_PTR_FIND,          //!< if not exists, find mqtt data pointer
   MQTT_PTR_FOUND,         //!< check if there is data to be send over mqtt
   MQTT_PTR_END,           //!< performs end operations with mqtt data pointer

   MQTT_OPEN,              //!< check mqtt client status
   MQTT_CHECK,             //!< check what kind of data to send: sdcard o current (sdcard fallback)
   MQTT_CONNECT,           //!< connect to mqtt server
   MQTT_ON_CONNECT,        //!< doing on connect event routine
   MQTT_SUBSCRIBE,         //!< subscribe to mqtt topic
   MQTT_CONSTANTDATA,      //!< publish constant station data without retry

   MQTT_OPEN_DATA_FILE,    //!< open sdcard read data file

   MQTT_SENSORS_LOOP,      //!< loop on the sensors
   MQTT_DATA_LOOP,         //!< loop on the sensors data
   MQTT_SD_LOOP,           //!< loop from first row to last row of read data file
   MQTT_PUBLISH,           //!< mqtt publish data

   MQTT_CLOSE_DATA_FILE,   //!< close sdcard read data file

   MQTT_DISCONNECT,        //!< disconnect from mqtt server
   MQTT_RPC_DELAY,         //!< delay after a RPC waiting for other RPC over MQTT (stay MQTT connected)
   MQTT_ON_DISCONNECT,     //!< doing on disconnect event routine

   MQTT_PTR_UPDATE,        //!< update mqtt data file pointer
   MQTT_CLOSE_PTR_FILE,    //!< close mqtt data file pointer
   //   MQTT_CLOSE_SDCARD,      //!< close sdcard

   MQTT_END,               //!< performs end operations and deactivate task
   MQTT_WAIT_STATE,        //!< non-blocking waiting time
   MQTT_WAIT_STATE_RPC     //!< non-blocking waiting time with MQTT management
} mqtt_state_t;
#endif

/*!
\enum rpc_state_t
\brief RPC task finite state machine.
*/
typedef enum {
   RPC_INIT,            //!< init task variables
   RPC_EXECUTE,         //!< execute function loop
   RPC_END              //!< performs end operations
} rpc_state_t;

/*********************************************************************
* GLOBAL VARIABLE
*********************************************************************/

/*!
\var sensors_count
\brief number of created sensors drivers.
*/
uint8_t sensors_count = 0;

/*!
\var is_datetime_set
\brief A valid date and time is setted and usable by station.
*/
bool  is_datetime_set;

/*!
\var have_to_reboot
\brief Request for a reboot as soon as possible.
*/
bool  have_to_reboot;

/*!
\var readable_configuration
\brief Configuration for this module.
*/
configuration_t readable_configuration;

/*!
\var writable_configuration
\brief Configuration for this module.
*/
configuration_t writable_configuration;

/*!
\var ready_tasks_count
\brief Number of tasks ready to execute.
*/
volatile uint8_t ready_tasks_count;

/*!
\var awakened_event_occurred_time_ms
\brief System time (in millisecond) when the system has awakened from power down.
*/
uint32_t awakened_event_occurred_time_ms;

/*!
\var streamRpc(false)
\brief Remote Procedure Call object.
*/
JsonRPC streamRpc(false);

#if (USE_SDCARD)
/*!
\var SD
\brief SD-Card structure.
*/
SdFat SD;

#if (ENABLE_SDCARD_LOGGING)   
/*!
\var logFile
\brief File for logging on SD-Card.
*/
File logFile;


/*!
\var loggingStream
\brief stream for logging on Serial and  SD-Card together.
*/
WriteLoggingStream loggingStream(logFile,Serial);
#endif

/*!
\var read_data_file
\brief File structure for read data stored on SD-Card.
*/
File read_data_file;

/*!
\var write_data_file
\brief File structure for write data stored on SD-Card.
*/
File write_data_file;

/*!
\var write_data_file
\brief File structure for write data stored on SD-Card.
*/
File test_file;
#endif

#if (USE_MQTT)
/*!
\var mqtt_ptr_file
\brief File structure for read and write data pointer stored on SD-Card for mqtt send.
*/
File mqtt_ptr_file;

/*!
\var is_mqtt_rpc_delay; 
\brief An MQTT RPC happened and we have to wait some time before disconnect waiting for some more RPC to come.
*/
bool is_mqtt_rpc_delay; 

/*!
\var rpcpayload; 
\brief The MQTT RPC payload for RPC response.
*/
char rpcpayload[MQTT_RPC_RESPONSE_LENGTH];


/*!
\var mqtt_session_present; 
\brief The MQTT session is present on the broker for a Persistent Session.
*/
bool mqtt_session_present;

#endif

#if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
/*!
\var eth_udp_client
\brief Ethernet UDP client structure.
*/
EthernetUDP eth_udp_client;

/*!
\var eth_tcp_client
\brief Ethernet TCP client structure.
*/
EthernetClient eth_tcp_client;

#if (USE_MQTT)
/*!
\fn IPStack ipstack(eth_tcp_client)
\brief Ethernet IPStack MQTTClient structure.
\return void.
*/
IPStack ipstack(eth_tcp_client);
#endif

#elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
/*!
\var s800
\brief SIMCom SIM800C/SIM800L UDP/TCP client structure.
*/
sim800Client s800;

#if (USE_MQTT)
/*!
\var ipstack
\brief SIMCom SIM800C/SIM800L IPStack MQTTClient structure.
*/
IPStack ipstack(s800);
#endif

#endif

#if (USE_MQTT)
/*!
\var mqtt_client
\brief MQTT Client structure.
*/
MQTT::Client<IPStack, Countdown, MQTT_PACKET_SIZE, 1> mqtt_client = MQTT::Client<IPStack, Countdown, MQTT_PACKET_SIZE, 1>(ipstack, IP_STACK_TIMEOUT_MS);

#endif

#if (USE_LCD)
/*!
\fn hd44780_I2Cexp lcd
\brief LCD object.
\return void.
*/
hd44780_I2Cexp lcd;


/*!
\var display_set
\brief Page to show on display when testing sensors.
*/
uint8_t display_set = 1;

#endif

/*!
\var sensors[SENSORS_MAX]
\brief SensorDriver array structure.
*/
SensorDriver *sensors[SENSORS_MAX];

/*!
\var is_first_run
\brief If true, the first reading of the sensors was performed.
*/
bool is_first_run;

/*!
\var do_reset_first_run
\brief If true, the first reading of the sensors was performed.
*/
bool do_reset_first_run;

/*!
\var is_first_test
\brief If true, the first reading of the sensors was performed.
*/
bool is_first_test;

/*!
\var is_test
\brief If true, reading value from sensors for testing purpose.
*/
bool is_test;

/*!
\var is_time_set
\brief If true, the time was readed from rtc or ntp and was setted in system.
*/
bool is_time_set;

/*!
\var is_time_for_sensors_reading_updated
\brief If true, the next time has been calculated to read the sensors.
*/
bool is_time_for_sensors_reading_updated;

/*!
\var is_client_connected
\brief If true, the client (ethernet or gsm) was connected to socket (TCP or UDP).
*/
bool is_client_connected;

/*!
\var is_client_udp_socket_open
\brief If true, the client (ethernet or gsm) was opened the UDP socket.
*/
bool is_client_udp_socket_open;

/*!
\var is_event_client_executed
\brief If true, the client has executed its task.
*/
bool is_event_client_executed;

/*!
\var is_event_time_executed
\brief If true, the time task has executed.
*/
bool is_event_time_executed;

/*!
\var do_ntp_sync
\brief If true, you must update the time from ntp.
*/
bool do_ntp_sync;

/*!
\var last_ntp_sync
\brief Last date and time when ntp sync was performed.
*/
time_t last_ntp_sync;

#if (USE_LCD)
/*!
\var last_lcd_begin
\brief Last date and time when LCD was initializated.
*/
time_t last_lcd_begin;

/*!
\var lcd_error
\brief Error happen with LCD.
*/
bool lcd_error;

#endif

#if (USE_SDCARD)
/*!
\var is_sdcard_open
\brief If true, the SD-Card is ready.
*/
bool is_sdcard_open;

/*!
\var is_sdcard_error
\brief If true, the SD-Card is in error.
*/
bool is_sdcard_error;
#endif

#if (USE_MQTT)

/*!
\var client_id
\brief MQTT client id.
*/
char client_id[MQTT_CLIENT_ID_LENGTH];

/*!
\var maint_topic
\brief MQTT topic for publish will message.
*/
char maint_topic[MQTT_ROOT_TOPIC_LENGTH + MQTT_SENSOR_TOPIC_LENGTH];
#endif

/*!
\var json_sensors_data
\brief buffer containing the data read by sensors in json text format.
*/
char json_sensors_data[SENSORS_MAX][JSON_BUFFER_LENGTH];

/*!
\var json_sensors_data_test
\brief buffer containing the data read by sensors in json text format for test only.
*/
char json_sensors_data_test[JSON_BUFFER_LENGTH];

/*!
\var system_time
\brief System time since 01/01/1970 00:00:00.
*/
volatile time_t system_time;

/*!
\var next_ptr_time_for_sensors_reading
\brief Next scheduled time (in seconds since 01/01/1970 00:0:00) for sensors reading.
*/
volatile time_t next_ptr_time_for_sensors_reading;

/*!
\var next_ptr_time_for_testing_sensors
\brief Next scheduled time (in seconds since 01/01/1970 00:0:00) for sensors reading.
*/
volatile time_t next_ptr_time_for_testing_sensors;


/*!
\var sensor_reading_failed_count
\brief Counter for failed and skipped sensors.
*/
uint8_t sensor_reading_failed_count;

/*!
\var sensor_reading_time
\brief Date and time corresponding to the last reading of the sensors.
*/
volatile tmElements_t sensor_reading_time;

/*!
\var ptr_time_data
\brief Readed data pointer stored on SD-Card for data send.
*/
time_t ptr_time_data;

/*!
\var stima_name
\brief Name of this module.
*/
char stima_name[20];

/*!
\var state
\brief Current main loop state.
*/
state_t state;

/*!
\var supervisor_state
\brief Supervisor task state.
*/
supervisor_state_t supervisor_state;

#if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
/*!
\var ethernet_state
\brief Ethernet task state.
*/
ethernet_state_t ethernet_state;

#elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
/*!
\var gsm_state
\brief GSM task state.
*/
gsm_state_t gsm_state;

#endif

/*!
\var i2c_error
\brief Number of i2c error.
*/
uint8_t i2c_error;

/*!
\var time_state
\brief Time task state.
*/
time_state_t time_state;

/*!
\var sensors_reading_state
\brief Sensors reading task state.
*/
sensors_reading_state_t sensors_reading_state;

#if (USE_SDCARD)
/*!
\var data_saving_state
\brief Data saving task state.
*/
data_saving_state_t data_saving_state;
#endif

#if (USE_MQTT)
/*!
\var mqtt_state
\brief MQTT task state.
*/
mqtt_state_t mqtt_state;
#endif

/*!
\var rpc_state
\brief RPC task state.
*/
rpc_state_t rpc_state;
/*********************************************************************
* FUNCTIONS
*********************************************************************/
void realreboot();
time_t getSystemTime();

/*!
\fn void init_logging(void)
\brief Init logging system.
\return void.
*/
void init_logging(void);

/*!
\fn void init_power_down(uint32_t *time_ms, uint32_t debouncing_ms)
\brief Enter power down mode.
\param time_ms pointer to a variable to save the last instant you entered power down.
\param debouncing_ms delay to power down.
\return void.
*/
void init_power_down(uint32_t *time_ms, uint32_t debouncing_ms);

/*!
\fn void init_wdt(uint8_t wdt_timer)
\brief Init watchdog.
\param wdt_timer a time value for init watchdog (WDTO_xxxx).
\return void.
*/
void init_wdt(uint8_t wdt_timer);

/*!
\fn void init_system(void)
\brief Init system.
\return void.
*/
void init_system(void);

/*!
\fn void init_rpc(void)
\brief Register RPC.
\return void.
*/
void init_rpc(void);

/*!
\fn void init_buffers(void)
\brief Init buffers.
\return void.
*/
void init_buffers(void);

/*!
\fn void init_tasks(void)
\brief Init tasks variable and state.
\return void.
*/
void init_tasks(void);

/*!
\fn void init_pins(void)
\brief Init hardware pins.
\return void.
*/
void init_pins(void);

/*!
\fn void init_wire(void)
\brief Init wire (i2c) library and performs checks on the bus.
\return void.
*/
void init_wire(void);

/*!
\fn void init_spi(void)
\brief Init SPI library.
\return void.
*/
void init_spi(void);

/*!
\fn void init_rtc(void)
\brief Init RTC module.
\return void.
*/
void init_rtc(void);

#if (USE_TIMER_1)
/*!
\fn void init_timer1(void)
\brief Init Timer1 module.
\return void.
*/
void init_timer1(void);

/*!
\fn void start_timer(void)
\brief Start Timer1 module.
\return void.
*/
void start_timer(void);

/*!
\fn void stop_timer(void)
\brief Stop Timer1 module.
\return void.
*/
void stop_timer(void);
#endif

/*!
\fn void interrupt_task_1s(void)
\brief 1 seconds task.
\return void.
*/
void interrupt_task_1s(void);

/*!
\fn void init_sensors(void)
\brief Create and setup sensors.
\return void.
*/
void init_sensors(void);

/*!
\fn void print_configuration(void)
\brief Print current configuration.
\return void.
*/
void print_configuration(void);

/*!
\fn void load_configuration(void)
\brief Load configuration from EEPROM.
\return void.
*/
void load_configuration(void);

/*!
\fn void save_configuration(bool is_default)
\brief Save configuration to EEPROM.
\param is_default: if true save default configuration; if false save current configuration.
\return void.
*/
void save_configuration(bool);

/*!
\fn void set_default_configuration()
\brief Set default configuration to global configuration variable.
\return void.
*/
void set_default_configuration(void);

/*!
\fn void setNextTimeForSensorReading(time_t *next_time)
\brief Calculate next hour, minute and second for sensors reading.
\param *next_time: Pointer to next scheduled time for sensors reading
\param time_s: next time in seconds
\return void.
*/
void setNextTimeForSensorReading(time_t *next_time, uint16_t time_s);

#if (USE_MQTT)
/*!
\fn bool mqttConnect(char *username, char *password)
\brief Use a open tcp socket to connect to the mqtt server.
\param *username: Username of mqtt server
\param *password: Password of mqtt server
\return true if connection was succesful, false otherwise.
*/
bool mqttConnect(char *username, char *password);

/*!
\fn bool mqttPublish(const char *topic, const char *message, bool is_retained = false)
\brief Publish message on topic
\param *topic: Topic for mqtt publish
\param *message: Message to be publish
\param is_retained: retained message if true
\return true if publish was succesful, false otherwise.
*/
bool mqttPublish(const char *topic, const char *message, bool is_retained = false);

/*!
\fn void mqttRxCallback(MQTT::MessageData &md)
\brief Register a receive callback for incoming mqtt message
\param &md: Received data structure
\return void.
*/
void mqttRxCallback(MQTT::MessageData &md);
#endif

/*!
\fn bool extractSensorsParams(JsonObject &params, char *driver, char *type, uint8_t *address, uint8_t *node)
\brief Extract sensor's parameter.
\param[in] *params: json's params
\param[out] *driver: driver
\param[out] *type: type
\param[out] *address: address
\param[out] *node: node
\return status code.
*/
bool extractSensorsParams(JsonObject &params, char *driver, char *type, uint8_t *address, uint8_t *node);

/*!
\fn int configure(JsonObject &params, JsonObject &result)
\brief RPC configuration.
\param *params: json's params
\param &result: json's response
\return status code.
*/
int configure(JsonObject &params, JsonObject &result);

/*!
\fn int prepare(JsonObject &params, JsonObject &result)
\brief RPC prepare.
\param *params: json's params
\param &result: json's response
\return status code.
*/
int prepare(JsonObject &params, JsonObject &result);

/*!
\fn int getjson(JsonObject &params, JsonObject &result)
\brief RPC get json.
\param *params: json's params
\param &result: json's response
\return status code.
*/
int getjson(JsonObject &params, JsonObject &result);

/*!
\fn int prepandget(JsonObject &params, JsonObject &result)
\brief RPC prepare and get json.
\param *params: json's params
\param &result: json's response
\return status code.
*/
int prepandget(JsonObject &params, JsonObject &result);

/*!
\fn int reboot(JsonObject &params, JsonObject &result)
\brief RPC reboot.
\param *params: json's params
\param &result: json's response
\return status code.
*/
int reboot(JsonObject &params, JsonObject &result);

/*********************************************************************
* TASKS
*********************************************************************/
/*!
\var is_event_supervisor
\brief Enable or disable the Supervisor task.
*/
bool is_event_supervisor;

/*!
\fn void supervisor_task(void)
\brief Supervisor task.
Manage RTC and NTP sync and open/close gsm and ethernet connection.
\return void.
*/
void supervisor_task(void);

/*!
\var is_event_sensors_reading
\brief Enable or disable the Sensors reading task.
*/
volatile bool is_event_sensors_reading;

/*!
\var is_event_sensors_reading_rpc
\brief Enable or disable the Sensors reading task from RPC.
*/
bool is_event_sensors_reading_rpc;

/*!
\fn void sensors_reading_task(bool do_prepare = true, bool do_get = true, char *driver = NULL, char *type = NULL, uint8_t address = 0, uint8_t node = 0, uint8_t *sensor_index = 0, uint32_t *wait_time = NULL)
\brief Sensors reading Task.
Read data from sensors.
\param do_prepare: if true, execute the prepare sensor's procedure
\param do_get: if true, execute the get sensor's procedure
\param *driver: sensor's driver
\param *type: sensor's type
\param *address: sensor's address
\param *node: sensor's node
\param *sensor_index: sensor's index
\param *wait_time: sensor's wait time
\return void.
*/
void sensors_reading_task (bool do_prepare = true, bool do_get = true, char *driver = NULL, char *type = NULL, uint8_t address = 0, uint8_t node = 0, uint8_t *sensor_index = 0, uint32_t *wait_time = NULL);

/*!
\var is_event_rtc
\brief Enable or disable the Real Time Clock task.
*/
volatile bool is_event_rtc;

/*!
\fn void rtc_task(void)
\brief Real Time Clock task.
Read RTC time and sync system time with it.
\return void.
*/
void rtc_task(void);

/*!
\var is_event_time
\brief Enable or disable the Time task.
*/
volatile bool is_event_time;

/*!
\fn void time_task(void)
\brief Time task.
Get time from NTP and sync RTC with it.
\return void.
*/
void time_task(void);

#if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
/*!
\var is_event_ethernet
\brief Enable or disable the Ethernet task.
*/
bool is_event_ethernet;

/*!
\fn void ethernet_task(void)
\brief Ethernet task.
Manage Ethernet operation.
\return void.
*/
void ethernet_task(void);

#elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
/*!
\var is_event_gsm
\brief Enable or disable the GSM task.
*/
bool is_event_gsm;

/*!
\fn void gsm_task(void)
\brief GSM Task.
Manage GSM operation.
\return void.
*/
void gsm_task(void);

#endif

#if (USE_SDCARD)
/*!
\var is_event_data_saving
\brief Enable or disable the Data saving task.
*/
bool is_event_data_saving;

/*!
\fn void data_saving_task(void)
\brief Data Saving Task.
Save acquired sensors data on SD-Card.
\return void.
*/
void data_saving_task(void);
#endif

#if (USE_MQTT)
/*!
\var is_event_mqtt
\brief Enable or disable the MQTT task.
*/
bool is_event_mqtt;

/*!
\var is_event_mqtt_paused
\brief If true, the MQTT task is in pause (need resume).
*/
bool is_event_mqtt_paused;

/*!
\fn void mqtt_task(void)
\brief MQTT Task.
\return void.
Read data stored on SD-Card and send it over MQTT.
*/
void mqtt_task(void);
#endif

/*!
\var is_event_rpc
\brief Indicate if RPC is active or not.
*/
bool is_event_rpc;

/*********************************************************************
* INTERRUPT HANDLER
*********************************************************************/
/*!
\fn void rtc_interrupt_handler(void)
\brief Real Time Clock interrupt handler.
\return void.
*/
void rtc_interrupt_handler(void);


#ifndef ARDUINO_ARCH_AVR
#include <IWatchdog.h>
#include "STM32LowPower.h"
#include <STM32RTC.h>
#include <rtc.h>

/* Get the rtc object */
STM32RTC& rtc = STM32RTC::getInstance();

HardwareSerial Serial1(PB11, PB10);

void wdt_enable(int wdt_timer){
  IWatchdog.begin(wdt_timer*1000000);
};
void wdt_reset(){
  IWatchdog.reload();
};
void wdt_disable(){
  //WARNING: Once started the IWDG timer can not be stopped.
  IWatchdog.begin(32000000);  // set to maximum value
};

void power_adc_disable(){};
void power_spi_disable(){};
void power_timer0_disable(){};
void power_timer1_disable(){};
void power_timer2_disable(){};
void power_adc_enable(){};
void power_spi_enable(){};
void power_timer0_enable(){};

void power_timer1_enable(){};
void power_timer2_enable(){};


// To be done
// manage sleep mode: RTC and timer 1 ....
void set_sleep_mode(int SLEEP_MODE_PWR_DOWN){
  // Select RTC clock source: LSI_CLOCK, LSE_CLOCK or HSE_CLOCK.
  // By default the LSI is selected as source.
  /*
  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin();
  LowPower.begin();
  */
};
void sleep_enable(){};
void sleep_cpu(){
  //LowPower.deepSleep(1000);
};
void sleep_disable(){};

#define WDTO_1S 1
#define SLEEP_MODE_PWR_DOWN 1

#endif


#endif
