/**@file sim7600.h */

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

#ifndef _SIM7600_H
#define _SIM7600_H

#include "debug_config.h"
#include "core/net.h"
#include "drivers/uart/uart_driver.h"
#include "debug_F.h"

/*!
\def SIM7600_NAME
\brief name of sim7600
*/
#define SIM7600_NAME                                     ("SIM7600")

/*!
\def SIM7600_AT_TX_CMD_DEBUG_PREFIX
\brief string prefix for printing AT command on debug interface
*/
#define SIM7600_AT_TX_CMD_DEBUG_PREFIX                    (">>")

/*!
\def SIM7600_AT_RX_CMD_DEBUG_PREFIX
\brief string prefix for printing AT command on debug interface
*/
#define SIM7600_AT_RX_CMD_DEBUG_PREFIX                    ("<<")

/*!
\def SIM7600_BUFFER_LENGTH
\brief Length of sim7600 buffer.
*/
#define SIM7600_BUFFER_LENGTH                             (150)

/*!
\def SIM7600_DEFAULT_BAUDRATE
\brief default baudrade.
*/
#define SIM7600_DEFAULT_BAUDRATE                         (115200)

/*!
\def AT_OK_STRING
\brief AT command: ok message.
*/
#define AT_OK_STRING                                     ("OK")

/*!
\def AT_ERROR_STRING
\brief AT command: error message.
*/
#define AT_ERROR_STRING                                  ("ERROR")

/*!
\def AT_PB_DONE_STRING
\brief AT command: pb done message.
*/
#define AT_PB_DONE_STRING                                ("PB DONE")

/*!
\def AT_CONNECT_OK_STRING
\brief AT command: on connect message.
*/
#define AT_CONNECT_OK_STRING                             ("CONNECT")

/*!
\def AT_CONNECT_FAIL_STRING
\brief AT command: connect fail message.
*/
#define AT_CONNECT_FAIL_STRING                           ("CONNECT FAIL")

/*!
\def AT_CIPCLOSE_OK_STRING
\brief AT command: cip close message.
*/
#define AT_CIPCLOSE_OK_STRING                            ("CLOSE OK")

/*!
\def AT_CIPCLOSE_ERROR_STRING
\brief AT command: cip close error message.
*/
#define AT_CIPCLOSE_ERROR_STRING                         ("ERROR")

/*!
\def AT_CIPSHUT_OK_STRING
\brief AT command: cip shiut ok message.
*/
#define AT_CIPSHUT_OK_STRING                             ("SHUT OK")

/*!
\def AT_CIPSHUT_ERROR_STRING
\brief AT command: cip shut error message.
*/
#define AT_CIPSHUT_ERROR_STRING                          ("ERROR")

/*!
\def SIM7600_CONNECTION_UDP
\brief UDP type.
*/
#define SIM7600_CONNECTION_UDP                            ("UDP")

/*!
\def SIM7600_CONNECTION_TCP
\brief TCP type.
*/
#define SIM7600_CONNECTION_TCP                            ("TCP")

/*!
\def SIM7600_AT_DEFAULT_TIMEOUT_MS
\brief Default AT command response timeout in milliseconds.
*/
#define SIM7600_AT_DEFAULT_TIMEOUT_MS                     (10000)

/*!
\def SIM7600_AT_DELAY_MS
\brief Waiting time in milliseconds between two AT command.
*/
#define SIM7600_AT_DELAY_MS                               (10)

/*!
\def SIM7600_GENERIC_RETRY_COUNT_MAX
\brief Number of retry in case of error.
*/
#define SIM7600_GENERIC_RETRY_COUNT_MAX                   (3)

/*!
\def SIM7600_GENERIC_WAIT_DELAY_MS
\brief Waiting time in milliseconds between two retry in milliseconds.
*/
#define SIM7600_GENERIC_WAIT_DELAY_MS                     (3000)

/*!
\def SIM7600_GENERIC_STATE_DELAY_MS
\brief Waiting time in milliseconds between two machine state.
*/
#define SIM7600_GENERIC_STATE_DELAY_MS                   (10)

/*!
\def SIM7600_WAIT_FOR_NETWORK_DELAY_MS
\brief Waiting time in milliseconds network availability.
*/
#define SIM7600_WAIT_FOR_NETWORK_DELAY_MS                 (5000)

/*!
\def SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX
\brief Max number of retry for checking network availability.
*/
#define SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX          (8)

/*!
\def SIM7600_WAIT_FOR_SETUP_DELAY_MS
\brief Waiting time in milliseconds for sim7600 setup.
*/
#define SIM7600_WAIT_FOR_SETUP_DELAY_MS                   (5000)

/*!
\def SIM7600_WAIT_FOR_AUTOBAUD_DELAY_MS
\brief Waiting time in milliseconds for autobaud.
*/
#define SIM7600_WAIT_FOR_AUTOBAUD_DELAY_MS                (3000)

/*!
\def SIM7600_WAIT_FOR_ATTACH_GPRS_DELAY_MS
\brief Waiting time in milliseconds for attaching gprs.
*/
#define SIM7600_WAIT_FOR_ATTACH_GPRS_DELAY_MS             (5000)

/*!
\def SIM7600_WAIT_FOR_CONNECTION_DELAY_MS
\brief Waiting time in milliseconds for setting up connection.
*/
#define SIM7600_WAIT_FOR_CONNECTION_DELAY_MS              (2000)

/*!
\def SIM7600_WAIT_FOR_GET_SIGNAL_QUALITY_DELAY_MS
\brief Waiting time in milliseconds for getting signal quality.
*/
#define SIM7600_WAIT_FOR_GET_SIGNAL_QUALITY_DELAY_MS      (3000)

/*!
\def SIM7600_POWER_ON_STABILIZATION_DELAY_MS
\brief Waiting time in milliseconds for power stabilization.
*/
#define SIM7600_POWER_ON_STABILIZATION_DELAY_MS             (2000)

/*!
\def SIM7600_POWER_ON_IMPULSE_DELAY_MS
\brief Waiting time in milliseconds for impulse powering sim7600.
*/
#define SIM7600_POWER_ON_IMPULSE_DELAY_MS                   (1000)

/*!
\def SIM7600_POWER_OFF_IMPULSE_DELAY_MS
\brief Waiting time in milliseconds for impulse powering sim7600.
*/
#define SIM7600_POWER_OFF_IMPULSE_DELAY_MS                  (4000)

/*!
\def SIM7600_WAIT_FOR_POWER_CHANGE_DELAY_MS
\brief Waiting time in milliseconds for powering sim7600.
*/
#define SIM7600_WAIT_FOR_POWER_CHANGE_DELAY_MS              (20000)

/*!
\def SIM7600_WAIT_FOR_POWER_OFF_DELAY_MS
\brief Waiting time in milliseconds for powering sim7600.
*/
#define SIM7600_WAIT_FOR_POWER_OFF_DELAY_MS                 (30000)

/*!
\def SIM7600_WAIT_FOR_EXIT_TRANSPARENT_MODE_DELAY_MS
\brief Waiting time in milliseconds for exiting trasparent mode.
*/
#define SIM7600_WAIT_FOR_EXIT_TRANSPARENT_MODE_DELAY_MS   (1000)

/*!
\def SIM7600_POWER_OFF_BY_SWITCH
\brief Execute sim7600 poweroff by pulling down relative pin.
*/
#define SIM7600_POWER_OFF_BY_SWITCH                       (0x01)

/*!
\def SIM7600_POWER_OFF_BY_AT_COMMAND
\brief Execute sim7600 poweroff by sending relative AT command.
*/
#define SIM7600_POWER_OFF_BY_AT_COMMAND                   (0x02)

/*!
\def SIM7600_IP_LENGTH
\brief Length of IP string buffer.
*/
#define SIM7600_IP_LENGTH                                 (16)

/*!
\def SIM7600_IMEI_LENGTH
\brief Length of IMEI string buffer.
*/
#define SIM7600_IMEI_LENGTH                               (20)

/*!
\def SIM7600_RSSI_MIN
\brief Minimum value of RSSI (low signal).
*/
#define SIM7600_RSSI_MIN                                  (0)

/*!
\def SIM7600_RSSI_MAX
\brief Minimum value of RSSI (high signal).
*/
#define SIM7600_RSSI_MAX                                  (199)

/*!
\def SIM7600_RSSI_UNKNOWN
\brief Unknown value of RSSI.
*/
#define SIM7600_RSSI_UNKNOWN                              (199)

/*!
\def SIM7600_BER_MIN
\brief Minimum value of BER.
*/
#define SIM7600_BER_MIN                                   (0)

/*!
\def SIM7600_BER_MAX
\brief Maximum value of BER.
*/
#define SIM7600_BER_MAX                                   (7)

/*!
\def SIM7600_BER_UNKNOWN
\brief Unknown value of BER.
*/
#define SIM7600_BER_UNKNOWN                               (99)

/*!
\def SIM7600_CGATT_RESPONSE_TIME_MAX_MS
\brief Maximum CGATT AT command response time in milliseconds.
*/
#define SIM7600_CGATT_RESPONSE_TIME_MAX_MS                (10000)

/*!
\def SIM7600_CIICR_RESPONSE_TIME_MAX_MS
\brief Maximum CIICR AT command response time in milliseconds.
*/
#define SIM7600_CIICR_RESPONSE_TIME_MAX_MS                (85000)

/*!
\def SIM7600_CIPSTART_RESPONSE_TIME_MAX_MS
\brief Maximum CIPSTART AT command response time in milliseconds.
*/
#define SIM7600_CIPSTART_RESPONSE_TIME_MAX_MS             (160000)

/*!
\def SIM7600_CIPSHUT_RESPONSE_TIME_MAX_MS
\brief Maximum CIPSHUT AT command response time in milliseconds.
*/
#define SIM7600_CIPSHUT_RESPONSE_TIME_MAX_MS              (65000)

/*!
\def found(str, check)
\brief Return true or false if check string is found in str string.
*/
#define found(str, check)                                (strstr(str, check))

/*!
\def printStatus(status, ok, error)
\brief Check if status is ok and print relative message.
*/
#define printStatus(status, ok, error)                   (status == SIM7600_OK ? ok : error)

/*!
\def getImei(imei)
\brief Return IMEI of simcard.
*/
#define getImei(imei)                                    (getGsn(imei))

/*!
\def getIp(ip)
\brief Return IP.
*/
#define getIp(ip)                                        (getCifsr(ip))

/*!
\def getNetworkStatus(n, stat)
\brief Return network status.
*/
#define getNetworkStatus(n, stat)                        (getCreg(n, stat))

/*!
\def getSignalQuality(rssi, ber)
\brief Return signal quality.
*/
#define getSignalQuality(rssi, ber)                      (getCsq(rssi, ber))

/*!
\def isGprsAttached(is_attached)
\brief Check if gprs is attached.
*/
#define isGprsAttached(is_attached)                      (getCgatt(is_attached))

/*!
\enum sim7600_power_state_t
\brief Main loop finite state machine.
*/
typedef enum
{
   SIM7600_POWER_INIT,         //!< init task variables
   SIM7600_POWER_ENABLE,       //!< set sim7600 poweron/poweroff pin low
   SIM7600_POWER_IMPULSE_UP,   //!< set sim7600 poweron/poweroff pin high
   SIM7600_POWER_IMPULSE_DOWN,   //!< set sim7600 poweron/poweroff pin high
   SIM7600_POWER_CHECK_STATUS, //!< check if sim7600 is on or is off
   SIM7600_POWER_END,          //!< performs end operations and deactivate task
   #ifndef USE_FREERTOS
   SIM7600_POWER_WAIT_STATE //!< non-blocking waiting time
   #endif
} sim7600_power_state_t;

/*!
\enum sim7600_power_state_t
\brief Main loop finite state machine.
*/
typedef enum
{
   SIM7600_POWER_OFF_INIT,         //!< init task variables
   SIM7600_POWER_OFF_END,          //!< performs end operations and deactivate task
   #ifndef USE_FREERTOS
   SIM7600_POWER_OFF_WAIT_STATE //!< non-blocking waiting time
   #endif
} sim7600_power_off_state_t;

/*!
\enum sim7600_setup_state_t
\brief Main loop finite state machine.
*/
typedef enum {
   SIM7600_SETUP_INIT,                  //!< init task variables
   SIM7600_SETUP_RESET,                 //!< reset sim7600 to default state
   SIM7600_SETUP_ECHO_MODE,             //!< disable sim7600 echo mode
   SIM7600_SETUP_GET_SIGNAL_QUALITY,    //!< get signal quality
   SIM7600_SETUP_WAIT_NETWORK,          //!< wait for network availability
   SIM7600_SETUP_END,                   //!< performs end operations and deactivate task
   SIM7600_SETUP_WAIT_STATE             //!< non-blocking waiting time
} sim7600_setup_state_t;

/*!
\enum sim7600_connection_start_state_t
\brief Main loop finite state machine.
*/
typedef enum {
   SIM7600_CONNECTION_START_INIT,                      //!< init task variables
   SIM7600_CONNECTION_START_CHECK_GPRS,                //!< check if sim7600 is attached to gprs
   SIM7600_CONNECTION_START_ATTACH_GPRS,               //!< if not, attach it to gprs
   SIM7600_CONNECTION_START_SINGLE_IP,                 //!< enable single ip mode
   SIM7600_CONNECTION_START_TRANSPARENT_MODE,          //!< enable trasparent mode
   SIM7600_CONNECTION_START_TRANSPARENT_MODE_CONFIG,   //!< configuring trasparent mode
   SIM7600_CONNECTION_START_APN_USERNAME_PASSWORD,     //!< settting apn, username and password
   SIM7600_CONNECTION_START_CONNECT,                   //!< starting up connection
   SIM7600_CONNECTION_START_GET_IP,                    //!< get connection ip
   SIM7600_CONNECTION_START_END,                       //!< performs end operations and deactivate task
   SIM7600_CONNECTION_START_WAIT_STATE                 //!< non-blocking waiting time
} sim7600_connection_start_state_t;

/*!
\enum sim7600_connection_state_t
\brief Main loop finite state machine.
*/
typedef enum {
   SIM7600_CONNECTION_INIT,             //!< init task variables
   SIM7600_CONNECTION_OPEN,             //!< open udp or tcp socket
   SIM7600_CONNECTION_CHECK_STATUS,     //!< check socket status
   SIM7600_CONNECTION_END,              //!< performs end operations and deactivate task
   SIM7600_CONNECTION_WAIT_STATE        //!< non-blocking waiting time
} sim7600_connection_state_t;

/*!
\enum sim7600_connection_stop_state_t
\brief Main loop finite state machine.
*/
typedef enum {
   SIM7600_CONNECTION_STOP_INIT,                       //!< init task variables
   SIM7600_CONNECTION_STOP_SWITCH_TO_COMMAND_MODE,     //!< exit transparent mode
   SIM7600_CONNECTION_STOP_CLOSE,                      //!< close socket
   SIM7600_CONNECTION_STOP_CLOSE_PDP,                  //!< close pdp context
   SIM7600_CONNECTION_STOP_DETACH_GPRS,                //!< detach gprs
   SIM7600_CONNECTION_STOP_END,                        //!< performs end operations and deactivate task
   SIM7600_CONNECTION_STOP_WAIT_STATE                  //!< non-blocking waiting time
} sim7600_connection_stop_state_t;

/*!
\enum sim7600_exit_transparent_mode_state_t
\brief Main loop finite state machine.
*/
typedef enum {
   SIM7600_EXIT_TRANSPARENT_MODE_INIT,                    //!< init task variables
   SIM7600_EXIT_TRANSPARENT_MODE_SEND_ESCAPE_SEQUENCE,    //!< send escape sequence for exiting transparent mode
   SIM7600_EXIT_TRANSPARENT_MODE_END,                     //!< performs end operations and deactivate task
   SIM7600_EXIT_TRANSPARENT_MODE_WAIT_STATE               //!< non-blocking waiting time
} sim7600_exit_transparent_mode_state_t;

/*!
\enum sim7600_state_t
\brief sim7600 finite state machine.
*/
typedef enum
{
   SIM7600_STATE_NONE = 0b00000000,             //!< default state at power on
   SIM7600_STATE_ON = 0b00000001,               //!< module is on
   SIM7600_STATE_INITIALIZED = 0b00000010,      //!< module is initialized
   SIM7600_STATE_SETTED = 0b00000100,           //!< module is is setted
   SIM7600_STATE_REGISTERED = 0b00001000,       //!< module is is registered on network
   SIM7600_STATE_CONNECTED = 0b00010000,        //!< module is is connected
#ifndef USE_FREERTOS
   SIM7600_AT_WAIT_STATE //!< non-blocking waiting time
#endif
} sim7600_state_t;

/*!
\enum sim7600_at_state_t
\brief Main loop finite state machine.
*/
typedef enum
{
   SIM7600_AT_INIT,    //!< init task variables
   SIM7600_AT_SEND,    //!< send AT command
   SIM7600_AT_RECEIVE, //!< wait for AT response
   SIM7600_AT_END,     //!< performs end operations and deactivate task
   #ifndef USE_FREERTOS
   SIM7600_AT_WAIT_STATE //!< non-blocking waiting time
   #endif
} sim7600_at_state_t;

/*!
\enum sim7600_status_t
\brief Main loop finite state machine.
*/
typedef enum {
   SIM7600_BUSY,      //!< busy: sim7600 is doing something
   SIM7600_OK,        //!< operation complete with success
   SIM7600_ERROR      //!< operation abort due to error
} sim7600_status_t;

/*!
\class SIM7600
\brief SIM7600 class.
*/
class SIM7600
{
public:
   SIM7600();
   
   #ifndef USE_FREERTOS
   SIM7600(HardwareSerial *serial, uint8_t _enable_power_pin, uint8_t _power_pin, uint8_t _ring_indicator_pin);
   #endif
   
   #ifdef USE_FREERTOS
   SIM7600(NetInterface *_interface, uint8_t _enable_power_pin, uint8_t _power_pin, uint8_t _ring_indicator_pin);
   #endif

   /*!
   \fn bool isOn()
   \brief Check if sim7600 is on or is off.
   \return true if it is on, false if is off.
   */
   bool isOn();

   /*!
   \fn bool isInitialized()
   \brief Check if sim7600 is initialized.
   \return true if it is initialized, false otherwise.
   */
   bool isInitialized();

   /*!
   \fn bool isSetted()
   \brief Check if sim7600 is setted.
   \return true if it is setted, false otherwise.
   */
   bool isSetted();

   /*!
   \fn bool isRegistered()
   \brief Check if sim7600 is registered on network.
   \return true if it is registered, false otherwise.
   */
   bool isRegistered();

   /*!
   \fn void setSerial(HardwareSerial *serial, uint32_t _baud_rate)
   \brief Set serial port for sim7600.
   \param[in] *serial pointer to serial stream.
   \param[in] _baud_rate baud rate for serial stream.
   \return void.
   */
   #ifndef USE_FREERTOS
   void setSerial(HardwareSerial *serial, uint32_t _baud_rate = SIM7600_DEFAULT_BAUDRATE);
   #endif

   /*!
   \fn void setInterface(NetInterface *_interface)
   \brief Set cyclonetcp interface for sim7600.
   \param[in] *interface pointer to interface.
   \return void.
   */
   #ifdef USE_FREERTOS
   void setInterface(NetInterface *_interface);
   #endif

   /*!
   \fn void setPins(uint8_t _enable_power_pin, uint8_t _power_pin, uint8_t _ring_inicator_pin)
   \brief set pins for module.
   \param[in] _enable_power_pin on/off pin for module.
   \param[in] _power_pin enable power for module.
   \param[in] _ring_inicator_pin input ring indicator for module.
   */
   void setPins(uint8_t _enable_power_pin, uint8_t _power_pin, uint8_t _ring_inicator_pin);

   /*!
   \fn void init()
   \brief Init module.
   */
   void init();

   /*!
   \fn uint32_t getDelayMs()
   \brief get waiting ms to delay.
   */
   uint32_t getDelayMs();

   /*!
   \fn sim7600_status_t getGsn(char *imei)
   \brief Send GSN AT command for reading simcard IMEI.
   \param[out] *imei pointer to char buffer containing imei.
   \return sim7600 status on each call.
   */
   sim7600_status_t getGsn(char *imei);

   /*!
   \fn sim7600_status_t getCreg(uint8_t *n, uint8_t *stat)
   \brief Send CREG AT command for reading network status.
   \param[out] *n pointer to variable containing n value.
   \param[out] *stat pointer to variable containing stat value.
   \return sim7600 status on each call.
   */
   sim7600_status_t getCreg(uint8_t *n, uint8_t *stat);


   /*!
   \fn sim7600_status_t getCsq(uint8_t *rssi, uint8_t *ber)
   \brief Send CSQ AT command for reading signal quality.
   \param[out] *rssi pointer to variable containing rssi value.
   \param[out] *ber pointer to variable containing ber value.
   \return sim7600 status on each call.
   */
   sim7600_status_t getCsq(uint8_t *rssi, uint8_t *ber);

   /*!
   \fn void getLastCsq(uint8_t *rssi, uint8_t *ber)
   \brief return signal quality when connected.
   \param[out] *rssi pointer to variable containing rssi value.
   \param[out] *ber pointer to variable containing ber value.
   */
   void getLastCsq(uint8_t *rssi, uint8_t *ber);
  
   /*!
   \fn sim7600_status_t getCgatt(bool *is_attached)
   \brief Send CGATT AT command for check if gprs is attached.
   \param[out] *is_attached pointer to bool variable indicating if gprs is attach (true) or not (false).
   \return sim7600 status on each call.
   */
   sim7600_status_t getCgatt(bool *is_attached);

   /*!
   \fn sim7600_status_t getCifsr(char *ip)
   \brief Send CIFSR AT command for reading IP.
   \param[out] *ip pointer to char buffer containing ip.
   \return sim7600 status on each call.
   */
   sim7600_status_t getCifsr(char *ip);

   /*!
   \fn sim7600_status_t switchOn()
   \brief Power on module.
   \return sim7600 status on each call.
   */
   sim7600_status_t switchOn();

   /*!
   \fn sim7600_status_t switchOff(uint8_t power_off_method = SIM7600_POWER_OFF_BY_AT_COMMAND)
   \brief Power off module.
   \param[in] power_off_method indicate what method you want to use for power off module (hardware switch or AT command). Default is hardware switch.
   \return sim7600 status on each call.
   */
   sim7600_status_t switchOff(uint8_t power_off_method = SIM7600_POWER_OFF_BY_AT_COMMAND);

   /*!
   \fn sim7600_status_t setup()
   \brief Execute setup sequence.
   \return sim7600 status on each call.
   */
   sim7600_status_t setup();

   /*!
   \fn sim7600_status_t startConnection(const char *apn, const char *username, const char *password)
   \brief Execute start connection sequence.
   \param[in] *apn apn for simcard network operation
   \param[in] *username username for simcard network operation
   \param[in] *password password for simcard network operation
   \return sim7600 status on each call.
   */
   sim7600_status_t startConnection(const char *apn, const char *username, const char *password);

   /*!
   \fn sim7600_status_t connection(const char *tipo, const char *server, const int port)
   \brief Execute connection sequence.
   \param[in] *tipo indicate what type of connection you want to open. Supported type are "UDP" and "TCP".
   \param[in] *server IP address or hostname of server.
   \param[in] port connection port.
   \return sim7600 status on each call.
   */
   sim7600_status_t connection(const char *tipo, const char *server, const int port);

   /*!
   \fn sim7600_status_t stopConnection()
   \brief Execute stop connection sequence.
   \return sim7600 status on each call.
   */
   sim7600_status_t stopConnection();

   /*!
   \fn sim7600_status_t exitTransparentMode()
   \brief Execute exiting trasparent mode sequence.
   \return sim7600 status on each call.
   */
   sim7600_status_t exitTransparentMode();

   /*!
   \fn void cleanInput()
   \brief Clear read serial port stream.
   \return void.
   */
   void cleanInput();

   /*!
   \fn uint8_t receive(char *rx_buffer, const char *at_ok_string = AT_OK_STRING, const char *at_error_string = AT_ERROR_STRING)
   \brief Read serial port stream and check if response message is a success message or error message.
   \param[out] *rx_buffer pointer to readed data buffer.
   \param[in] *at_ok_string success message.
   \param[in] *at_error_string error message.
   \return number of bytes readed from stream.
   */
   #ifndef USE_FREERTOS
   uint8_t receive(char *rx_buffer, const char *at_ok_string = AT_OK_STRING, const char *at_error_string = AT_ERROR_STRING);
   #endif

   /*!
   \fn sendAtCommand(const char *command, char *buf, const char *at_ok_string = AT_OK_STRING, const char *at_error_string = AT_ERROR_STRING, uint32_t timeout_ms = SIM7600_AT_DEFAULT_TIMEOUT_MS)
   \brief Write AT command to serial stream, managing timeout.
   \param[in] *command AT command to be send.
   \param[out] *response pointer to response data buffer.
   \param[in] response_length response buffer lenght.
   \param[in] *at_ok_string success message.
   \param[in] *at_error_string error message.
   \param[in] timeout_ms timeout in milliseconds for receive a response.
   \return sim7600 status on each call.
   */
   sim7600_status_t sendAtCommand(const char *command, char *response, size_t response_length, const char *at_ok_string = AT_OK_STRING, const char *at_error_string = AT_ERROR_STRING, uint32_t timeout_ms = SIM7600_AT_DEFAULT_TIMEOUT_MS);

protected:
   /*!
   \var *modem
   \brief pointer to modem serial stream.
   */
   #ifndef USE_FREERTOS
   HardwareSerial *modem;
   #endif

private:
   /*!
   \var buffer_ext
   \brief Buffer for send AT command and receive response.
   */
   char buffer_ext[SIM7600_BUFFER_LENGTH];

   /*!
   \var buffer_ext2
   \brief Buffer for send AT command and receive response.
   */
   char buffer_ext2[SIM7600_BUFFER_LENGTH];

   /*!
   \fn sim7600_status_t switchModem(bool is_switching_on)
   \brief Switch on to off or off to on state.
   \param[in] is_switching_on if true, switch module off to on, if false, switch module on to off.
   \return sim7600 status on each call.
   */
   sim7600_status_t switchModem(bool is_switching_on);

   /*!
   \var baud_rate
   \brief baud rate for serial port of sim7600.
   */
   uint32_t baud_rate;

   /*!
   \var interface
   \brief cylocnetcp interface for sim7600.
   */
   #ifdef USE_FREERTOS
   NetInterface *interface;
   #endif

   /*!
   \var delay_ms
   \brief waiting delay in ms.
   */
   uint32_t delay_ms;

   /*!
   \var enable_power_pin
   \brief pin for enable power on sim7600.
   */
   uint8_t enable_power_pin;

   /*!
   \var power_pin
   \brief pin for power on/off sim7600.
   */
   uint8_t power_pin;

   /*!
   \var ring_indicator_pin
   \brief ring indicator pin on sim7600.
   */
   uint8_t ring_indicator_pin;

   /*!
   \var state
   \brief SIM7600 state.
   */
   sim7600_state_t state;

   /*!
   \var sim7600_power_state
   \brief sim7600 power sequence state.
   */
   sim7600_power_state_t sim7600_power_state;

   /*!
   \var sim7600_power_off_state
   \brief sim7600 power off sequence state.
   */
   sim7600_power_off_state_t sim7600_power_off_state;

   /*!
   \var sim7600_setup_state
   \brief sim7600 setup sequence state.
   */
   sim7600_setup_state_t sim7600_setup_state;

   /*!
   \var sim7600_connection_start_state
   \brief sim7600 connection start sequence state.
   */
   sim7600_connection_start_state_t sim7600_connection_start_state;

   /*!
   \var sim7600_connection_state
   \brief sim7600 connection sequence state.
   */
   sim7600_connection_state_t sim7600_connection_state;

   /*!
   \var sim7600_at_state
   \brief sim7600 at sequence state.
   */
   sim7600_at_state_t sim7600_at_state;

   /*!
   \var sim7600_connection_stop_state
   \brief sim7600 connection stop sequence state.
   */
   sim7600_connection_stop_state_t sim7600_connection_stop_state;

   /*!
   \var sim7600_exit_transparent_mode_state
   \brief sim7600 exit transparent mode sequence state.
   */
   sim7600_exit_transparent_mode_state_t sim7600_exit_transparent_mode_state;

   /*!
   \var sim7600_rssi
   \brief sim7600 rssi of the active connection.
   */
   uint8_t sim7600_rssi;

   /*!
    \var sim7600_ber
    \brief sim7600 ber of the active connection.
    */
   uint8_t sim7600_ber;

   /*!
    \var sim7600_imei
    \brief sim7600 IMEI.
    */
   char sim7600_imei[16];

   /*!
    \var sim7600_ip
    \brief sim7600 IP of the active connection.
    */
   char sim7600_ip[16];
};

#endif
