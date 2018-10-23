/**@file sim800.h */

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

#ifndef _SIM800_H
#define _SIM800_H

#include "Arduino.h"
#include <Time.h>
#include "IPAddress.h"

#include <debug.h>

/*!
\def SIM800_BUFFER_LENGTH
\brief Length of sim800 buffer.
*/
#define SIM800_BUFFER_LENGTH                             (150)

/*!
\def SIM800_SERIAL_PORT
\brief Default sim800 serial port.
*/
#define SIM800_SERIAL_PORT                               (Serial1)

/*!
\def SIM800_SERIAL_PORT_BAUD_RATE
\brief Baud rate of sim800 serial port.
*/
#define SIM800_SERIAL_PORT_BAUD_RATE                     (115200)

/*!
\def SIM800_STATE_NONE
\brief sim800 state none: default state at power on.
*/
#define SIM800_STATE_NONE                                (0b00000000)

/*!
\def SIM800_STATE_ON
\brief sim800 is on.
*/
#define SIM800_STATE_ON                                  (0b00000001)

/*!
\def SIM800_STATE_INITIALIZED
\brief sim800 is initialized.
*/
#define SIM800_STATE_INITIALIZED                         (0b00000010)

/*!
\def SIM800_STATE_SETTED
\brief sim800 is setted.
*/
#define SIM800_STATE_SETTED                              (0b00000100)

/*!
\def SIM800_STATE_REGISTERED
\brief sim800 is registered on network.
*/
#define SIM800_STATE_REGISTERED                          (0b00001000)

/*!
\def SIM800_STATE_HTTP_INITIALIZED
\brief sim800 is connected.
*/
#define SIM800_STATE_HTTP_INITIALIZED                    (0b00010000)

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
\def AT_NORMAL_POWER_DOWN_STRING
\brief AT command: power down message.
*/
#define AT_NORMAL_POWER_DOWN_STRING                      ("NORMAL POWER DOWN")

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
\def SIM800_CONNECTION_UDP
\brief UDP type.
*/
#define SIM800_CONNECTION_UDP                            ("UDP")

/*!
\def SIM800_CONNECTION_TCP
\brief TCP type.
*/
#define SIM800_CONNECTION_TCP                            ("TCP")

/*!
\def SIM800_AT_DEFAULT_TIMEOUT_MS
\brief Default AT command response timeout in milliseconds.
*/
#define SIM800_AT_DEFAULT_TIMEOUT_MS                     (1000)

/*!
\def SIM800_AT_DELAY_MS
\brief Waiting time in milliseconds between two AT command.
*/
#define SIM800_AT_DELAY_MS                               (200)

/*!
\def SIM800_GENERIC_RETRY_COUNT_MAX
\brief Number of retry in case of error.
*/
#define SIM800_GENERIC_RETRY_COUNT_MAX                   (3)

/*!
\def SIM800_GENERIC_WAIT_DELAY_MS
\brief Waiting time in milliseconds between two retry in milliseconds.
*/
#define SIM800_GENERIC_WAIT_DELAY_MS                     (3000)

/*!
\def SIM800_WAIT_FOR_NETWORK_DELAY_MS
\brief Waiting time in milliseconds network availability.
*/
#define SIM800_WAIT_FOR_NETWORK_DELAY_MS                 (5000)

/*!
\def SIM800_WAIT_FOR_NETWORK_RETRY_COUNT_MAX
\brief Max number of retry for checking network availability.
*/
#define SIM800_WAIT_FOR_NETWORK_RETRY_COUNT_MAX          (8)

/*!
\def SIM800_WAIT_FOR_SETUP_DELAY_MS
\brief Waiting time in milliseconds for sim800 setup.
*/
#define SIM800_WAIT_FOR_SETUP_DELAY_MS                   (5000)

/*!
\def SIM800_WAIT_FOR_AUTOBAUD_DELAY_MS
\brief Waiting time in milliseconds for autobaud.
*/
#define SIM800_WAIT_FOR_AUTOBAUD_DELAY_MS                (3000)

/*!
\def SIM800_WAIT_FOR_ATTACH_GPRS_DELAY_MS
\brief Waiting time in milliseconds for attaching gprs.
*/
#define SIM800_WAIT_FOR_ATTACH_GPRS_DELAY_MS             (5000)

/*!
\def SIM800_WAIT_FOR_CONNECTION_DELAY_MS
\brief Waiting time in milliseconds for setting up connection.
*/
#define SIM800_WAIT_FOR_CONNECTION_DELAY_MS              (2000)

/*!
\def SIM800_WAIT_FOR_GET_SIGNAL_QUALITY_DELAY_MS
\brief Waiting time in milliseconds for getting signal quality.
*/
#define SIM800_WAIT_FOR_GET_SIGNAL_QUALITY_DELAY_MS      (3000)

/*!
\def SIM800_POWER_ON_OFF_SWITCH_DELAY_MS
\brief Waiting time in milliseconds for putting sim800 on or off by pulling down on/off pin.
*/
#define SIM800_POWER_ON_OFF_SWITCH_DELAY_MS              (1200)

/*!
\def SIM800_POWER_ON_OFF_DONE_DELAY_MS
\brief Waiting time in milliseconds for sim800 startup or shutdown sequence.
*/
#define SIM800_POWER_ON_OFF_DONE_DELAY_MS                (2000)

/*!
\def SIM800_POWER_ON_TO_OFF_DELAY_MS
\brief Minimum time in milliseconds for shutdown sim800 after a poweron.
*/
#define SIM800_POWER_ON_TO_OFF_DELAY_MS                  (10000)

/*!
\def SIM800_WAIT_FOR_POWER_OFF_DELAY_MS
\brief Waiting time in milliseconds after sim800 poweroff (useful for printing message in cpu powerdown).
*/
#define SIM800_WAIT_FOR_POWER_OFF_DELAY_MS               (100)

/*!
\def SIM800_WAIT_FOR_EXIT_TRANSPARENT_MODE_DELAY_MS
\brief Waiting time in milliseconds for exiting trasparent mode.
*/
#define SIM800_WAIT_FOR_EXIT_TRANSPARENT_MODE_DELAY_MS   (1000)

/*!
\def SIM800_POWER_OFF_BY_SWITCH
\brief Execute sim800 poweroff by pulling down relative pin.
*/
#define SIM800_POWER_OFF_BY_SWITCH                       (0x01)

/*!
\def SIM800_POWER_OFF_BY_AT_COMMAND
\brief Execute sim800 poweroff by sending relative AT command.
*/
#define SIM800_POWER_OFF_BY_AT_COMMAND                   (0x02)

/*!
\def SIM800_IP_LENGTH
\brief Length of IP string buffer.
*/
#define SIM800_IP_LENGTH                                 (16)

/*!
\def SIM800_IMEI_LENGTH
\brief Length of IMEI string buffer.
*/
#define SIM800_IMEI_LENGTH                               (20)

/*!
\def SIM800_RSSI_MIN
\brief Minimum value of RSSI (low signal).
*/
#define SIM800_RSSI_MIN                                  (0)

/*!
\def SIM800_RSSI_MAX
\brief Minimum value of RSSI (high signal).
*/
#define SIM800_RSSI_MAX                                  (31)

/*!
\def SIM800_RSSI_UNKNOWN
\brief Unknown value of RSSI.
*/
#define SIM800_RSSI_UNKNOWN                              (99)

/*!
\def SIM800_BER_MIN
\brief Minimum value of BER.
*/
#define SIM800_BER_MIN                                   (0)

/*!
\def SIM800_BER_MAX
\brief Maximum value of BER.
*/
#define SIM800_BER_MAX                                   (7)

/*!
\def SIM800_BER_UNKNOWN
\brief Unknown value of BER.
*/
#define SIM800_BER_UNKNOWN                               (99)

/*!
\def SIM800_CGATT_RESPONSE_TIME_MAX_MS
\brief Maximum CGATT AT command response time in milliseconds.
*/
#define SIM800_CGATT_RESPONSE_TIME_MAX_MS                (10000)

/*!
\def SIM800_CIICR_RESPONSE_TIME_MAX_MS
\brief Maximum CIICR AT command response time in milliseconds.
*/
#define SIM800_CIICR_RESPONSE_TIME_MAX_MS                (85000)

/*!
\def SIM800_CIPSTART_RESPONSE_TIME_MAX_MS
\brief Maximum CIPSTART AT command response time in milliseconds.
*/
#define SIM800_CIPSTART_RESPONSE_TIME_MAX_MS             (160000)

/*!
\def SIM800_CIPSHUT_RESPONSE_TIME_MAX_MS
\brief Maximum CIPSHUT AT command response time in milliseconds.
*/
#define SIM800_CIPSHUT_RESPONSE_TIME_MAX_MS              (65000)

/*!
\def found(str, check)
\brief Return true or false if check string is found in str string.
*/
#define found(str, check)                                (strstr(str, check))

/*!
\def send(data)
\brief write data in modem stream (Serial).
*/
#define send(data)                                       (modem->print(data))

/*!
\def printStatus(status, ok, error)
\brief Check if status is ok and print relative message.
*/
#define printStatus(status, ok, error)                   (status == SIM800_OK ? ok : error)

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
\def softwareSwitchOff()
\brief Send switchoff AT command.
*/
#define softwareSwitchOff()                              (sendCpowd())

/*!
\enum sim800_power_state_t
\brief Main loop finite state machine.
*/
typedef enum {
   SIM800_POWER_INIT,            //!< init task variables
   SIM800_POWER_SET_PIN_LOW,     //!< set sim800 poweron/poweroff pin low
   SIM800_POWER_SET_PIN_HIGH,    //!< set sim800 poweron/poweroff pin high
   SIM800_POWER_CHECK_STATUS,    //!< check if sim800 is on or is off
   SIM800_POWER_END,             //!< performs end operations and deactivate task
   SIM800_POWER_WAIT_STATE       //!< non-blocking waiting time
} sim800_power_state_t;

/*!
\enum sim800_setup_state_t
\brief Main loop finite state machine.
*/
typedef enum {
   SIM800_SETUP_INIT,                  //!< init task variables
   SIM800_SETUP_RESET,                 //!< reset sim800 to default state
   SIM800_SETUP_ECHO_MODE,             //!< disable sim800 echo mode
   SIM800_SETUP_GET_SIGNAL_QUALITY,    //!< get signal quality
   SIM800_SETUP_WAIT_NETWORK,          //!< wait for network availability
   SIM800_SETUP_END,                   //!< performs end operations and deactivate task
   SIM800_SETUP_WAIT_STATE             //!< non-blocking waiting time
} sim800_setup_state_t;

/*!
\enum sim800_connection_start_state_t
\brief Main loop finite state machine.
*/
typedef enum {
   SIM800_CONNECTION_START_INIT,                      //!< init task variables
   SIM800_CONNECTION_START_CHECK_GPRS,                //!< check if sim800 is attached to gprs
   SIM800_CONNECTION_START_ATTACH_GPRS,               //!< if not, attach it to gprs
   SIM800_CONNECTION_START_SINGLE_IP,                 //!< enable single ip mode
   SIM800_CONNECTION_START_TRANSPARENT_MODE,          //!< enable trasparent mode
   SIM800_CONNECTION_START_TRANSPARENT_MODE_CONFIG,   //!< configuring trasparent mode
   SIM800_CONNECTION_START_APN_USERNAME_PASSWORD,     //!< settting apn, username and password
   SIM800_CONNECTION_START_CONNECT,                   //!< starting up connection
   SIM800_CONNECTION_START_GET_IP,                    //!< get connection ip
   SIM800_CONNECTION_START_END,                       //!< performs end operations and deactivate task
   SIM800_CONNECTION_START_WAIT_STATE                 //!< non-blocking waiting time
} sim800_connection_start_state_t;

/*!
\enum sim800_connection_state_t
\brief Main loop finite state machine.
*/
typedef enum {
   SIM800_CONNECTION_INIT,             //!< init task variables
   SIM800_CONNECTION_OPEN,             //!< open udp or tcp socket
   SIM800_CONNECTION_CHECK_STATUS,     //!< check socket status
   SIM800_CONNECTION_END,              //!< performs end operations and deactivate task
   SIM800_CONNECTION_WAIT_STATE        //!< non-blocking waiting time
} sim800_connection_state_t;

/*!
\enum sim800_connection_stop_state_t
\brief Main loop finite state machine.
*/
typedef enum {
   SIM800_CONNECTION_STOP_INIT,                       //!< init task variables
   SIM800_CONNECTION_STOP_SWITCH_TO_COMMAND_MODE,     //!< exit transparent mode
   SIM800_CONNECTION_STOP_CLOSE,                      //!< close socket
   SIM800_CONNECTION_STOP_CLOSE_PDP,                  //!< close pdp context
   SIM800_CONNECTION_STOP_DETACH_GPRS,                //!< detach gprs
   SIM800_CONNECTION_STOP_END,                        //!< performs end operations and deactivate task
   SIM800_CONNECTION_STOP_WAIT_STATE                  //!< non-blocking waiting time
} sim800_connection_stop_state_t;

/*!
\enum sim800_exit_transparent_mode_state_t
\brief Main loop finite state machine.
*/
typedef enum {
   SIM800_EXIT_TRANSPARENT_MODE_INIT,                    //!< init task variables
   SIM800_EXIT_TRANSPARENT_MODE_SEND_ESCAPE_SEQUENCE,    //!< send escape sequence for exiting transparent mode
   SIM800_EXIT_TRANSPARENT_MODE_END,                     //!< performs end operations and deactivate task
   SIM800_EXIT_TRANSPARENT_MODE_WAIT_STATE               //!< non-blocking waiting time
} sim800_exit_transparent_mode_state_t;

/*!
\enum sim800_at_state_t
\brief Main loop finite state machine.
*/
typedef enum {
   SIM800_AT_INIT,         //!< init task variables
   SIM800_AT_SEND,         //!< send AT command
   SIM800_AT_RECEIVE,      //!< wait for AT response
   SIM800_AT_END,          //!< performs end operations and deactivate task
   SIM800_AT_WAIT_STATE    //!< non-blocking waiting time
} sim800_at_state_t;

/*!
\enum sim800_status_t
\brief Main loop finite state machine.
*/
typedef enum {
   SIM800_BUSY,      //!< busy: sim800 is doing something
   SIM800_OK,        //!< operation complete with success
   SIM800_ERROR      //!< operation abort due to error
} sim800_status_t;

/*!
\class SIM800
\brief SIM800 class.
*/
class SIM800 {
public:
   SIM800();

   /*!
   \fn bool isOn()
   \brief Check if sim800 is on or is off.
   \return true if it is on, false if is off.
   */
   bool isOn();

   /*!
   \fn bool isInitialized()
   \brief Check if sim800 is initialized.
   \return true if it is initialized, false otherwise.
   */
   bool isInitialized();

   /*!
   \fn bool isSetted()
   \brief Check if sim800 is setted.
   \return true if it is setted, false otherwise.
   */
   bool isSetted();

   /*!
   \fn bool isRegistered()
   \brief Check if sim800 is registered on network.
   \return true if it is registered, false otherwise.
   */
   bool isRegistered();

   /*!
   \fn bool isHttpInitialized()
   \brief Check if sim800 is connected.
   \return true if it is connected, false otherwise.
   */
   bool isHttpInitialized();

   /*!
   \fn void setSerial(HardwareSerial *serial)
   \brief Set serial port for sim800.
   \param[in] *serial pointer to serial stream.
   \return void.
   */
   void setSerial(HardwareSerial *serial);

   /*!
   \fn bool init(uint8_t _on_off_pin, uint8_t _reset_pin = 0xFF)
   \brief Init variables for sim800.
   \param[in] _on_off_pin on/off pin for module.
   \param[in] _reset_pin reset pin for module.
   \return true.
   */
   bool init(uint8_t _on_off_pin, uint8_t _reset_pin = 0xFF);

   /*!
   \fn sim800_status_t getGsn(char *imei)
   \brief Send GSN AT command for reading simcard IMEI.
   \param[out] *imei pointer to char buffer containing imei.
   \return sim800 status on each call.
   */
   sim800_status_t getGsn(char *imei);

   /*!
   \fn sim800_status_t getCreg(uint8_t *n, uint8_t *stat)
   \brief Send CREG AT command for reading network status.
   \param[out] *n pointer to variable containing n value.
   \param[out] *stat pointer to variable containing stat value.
   \return sim800 status on each call.
   */
   sim800_status_t getCreg(uint8_t *n, uint8_t *stat);

   /*!
   \fn sim800_status_t getCsq(uint8_t *rssi, uint8_t *ber)
   \brief Send CSQ AT command for reading signal quality.
   \param[out] *rssi pointer to variable containing rssi value.
   \param[out] *ber pointer to variable containing ber value.
   \return sim800 status on each call.
   */
   sim800_status_t getCsq(uint8_t *rssi, uint8_t *ber);

   /*!
   \fn sim800_status_t getCgatt(bool *is_attached)
   \brief Send CGATT AT command for check if gprs is attached.
   \param[out] *is_attached pointer to bool variable indicating if gprs is attach (true) or not (false).
   \return sim800 status on each call.
   */
   sim800_status_t getCgatt(bool *is_attached);

   /*!
   \fn sim800_status_t getCifsr(char *ip)
   \brief Send CIFSR AT command for reading IP.
   \param[out] *ip pointer to char buffer containing ip.
   \return sim800 status on each call.
   */
   sim800_status_t getCifsr(char *ip);

   /*!
   \fn sim800_status_t sendCpowd()
   \brief Send CPOWD AT command for power off module.
   \return sim800 status on each call.
   */
   sim800_status_t sendCpowd();

   /*!
   \fn sim800_status_t switchOn()
   \brief Power on module.
   \return sim800 status on each call.
   */
   sim800_status_t switchOn();

   /*!
   \fn sim800_status_t switchOff(uint8_t power_off_method = SIM800_POWER_OFF_BY_SWITCH)
   \brief Power off module.
   \param[in] power_off_method indicate what method you want to use for power off module (hardware switch or AT command). Default is hardware switch.
   \return sim800 status on each call.
   */
   sim800_status_t switchOff(uint8_t power_off_method = SIM800_POWER_OFF_BY_SWITCH);

   /*!
   \fn sim800_status_t sendAt()
   \brief Send "AT" AT command.
   \return sim800 status.
   */
   sim800_status_t sendAt();

   /*!
   \fn sim800_status_t initAutobaud()
   \brief Execute autobaud sequence.
   \return sim800 status on each call.
   */
   sim800_status_t initAutobaud();

   /*!
   \fn sim800_status_t setup()
   \brief Execute setup sequence.
   \return sim800 status on each call.
   */
   sim800_status_t setup();

   /*!
   \fn sim800_status_t startConnection(const char *apn, const char *username, const char *password)
   \brief Execute start connection sequence.
   \param[in] *apn apn for simcard network operation
   \param[in] *username username for simcard network operation
   \param[in] *password password for simcard network operation
   \return sim800 status on each call.
   */
   sim800_status_t startConnection(const char *apn, const char *username, const char *password);

   /*!
   \fn sim800_status_t connection(const char *tipo, const char *server, const int port)
   \brief Execute connection sequence.
   \param[in] *tipo indicate what type of connection you want to open. Supported type are "UDP" and "TCP".
   \param[in] *server IP address or hostname of server.
   \param[in] port connection port.
   \return sim800 status on each call.
   */
   sim800_status_t connection(const char *tipo, const char *server, const int port);

   /*!
   \fn sim800_status_t stopConnection()
   \brief Execute stop connection sequence.
   \return sim800 status on each call.
   */
   sim800_status_t stopConnection();

   /*!
   \fn sim800_status_t exitTransparentMode()
   \brief Execute exiting trasparent mode sequence.
   \return sim800 status on each call.
   */
   sim800_status_t exitTransparentMode();

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
   uint8_t receive(char *rx_buffer, const char *at_ok_string = AT_OK_STRING, const char *at_error_string = AT_ERROR_STRING);

   /*!
   \fn sendAtCommand(const char *command, char *buf, const char *at_ok_string = AT_OK_STRING, const char *at_error_string = AT_ERROR_STRING, uint32_t timeout_ms = SIM800_AT_DEFAULT_TIMEOUT_MS)
   \brief Write AT command to serial stream, managing timeout.
   \param[in] *command AT command to be send.
   \param[in,out] *buf pointer to readed and writed data buffer.
   \param[in] *at_ok_string success message.
   \param[in] *at_error_string error message.
   \param[in] timeout_ms timeout in milliseconds for receive a response.
   \return sim800 status on each call.
   */
   sim800_status_t sendAtCommand(const char *command, char *buf, const char *at_ok_string = AT_OK_STRING, const char *at_error_string = AT_ERROR_STRING, uint32_t timeout_ms = SIM800_AT_DEFAULT_TIMEOUT_MS);

   /*!
   \var state
   \brief SIM800 state.
   */
   uint8_t state;

protected:
   /*!
   \var *modem
   \brief pointer to modem serial stream.
   */
   HardwareSerial *modem = &Serial1;

private:
   /*!
   \fn sim800_status_t switchModem(bool is_switching_on)
   \brief Switch on to off or off to on state.
   \param[in] is_switching_on if true, switch module off to on, if false, switch module on to off.
   \return sim800 status on each call.
   */
   sim800_status_t switchModem(bool is_switching_on);

   /*!
   \var on_off_pin
   \brief pin for power on/off sim800.
   */
   uint8_t on_off_pin;

   /*!
   \var reset_pin
   \brief pin for resetting sim800.
   */
   uint8_t reset_pin;

   /*!
   \var sim800_power_state
   \brief sim800 power sequence state.
   */
   sim800_power_state_t sim800_power_state;

   /*!
   \var sim800_setup_state
   \brief sim800 setup sequence state.
   */
   sim800_setup_state_t sim800_setup_state;

   /*!
   \var sim800_connection_start_state
   \brief sim800 connection start sequence state.
   */
   sim800_connection_start_state_t sim800_connection_start_state;

   /*!
   \var sim800_connection_state
   \brief sim800 connection sequence state.
   */
   sim800_connection_state_t sim800_connection_state;

   /*!
   \var sim800_at_state
   \brief sim800 at sequence state.
   */
   sim800_at_state_t sim800_at_state;

   /*!
   \var sim800_connection_stop_state
   \brief sim800 connection stop sequence state.
   */
   sim800_connection_stop_state_t sim800_connection_stop_state;

   /*!
   \var sim800_exit_transparent_mode_state
   \brief sim800 exit transparent mode sequence state.
   */
   sim800_exit_transparent_mode_state_t sim800_exit_transparent_mode_state;
};

#endif
