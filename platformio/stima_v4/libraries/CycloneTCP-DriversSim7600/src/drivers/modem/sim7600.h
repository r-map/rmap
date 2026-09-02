/**@file sim7600.h */

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
#define SIM7600_NAME                                        ("SIM7600")

/*!
\def FTS_PRECISION
\precision conversion NMEA GPS Fract real value coordinate
*/
#define FTS_PRECISION 6

/*!
\def SIM7600_AT_TX_CMD_DEBUG_PREFIX
\brief string prefix for printing AT command on debug interface
*/
#define SIM7600_AT_TX_CMD_DEBUG_PREFIX                      (">>")

/*!
\def SIM7600_AT_RX_CMD_DEBUG_PREFIX
\brief string prefix for printing AT command on debug interface
*/
#define SIM7600_AT_RX_CMD_DEBUG_PREFIX                      ("<<")

/*!
\def SIM7600_BUFFER_LENGTH
\brief Length of sim7600 buffer.
*/
#define SIM7600_BUFFER_LENGTH                               (150)

/*!
\def SIM7600_BUFFER_LENGTH
\brief Length of sim7600 buffer.
*/
#define SIM7600_BUFFER_CMD_LENGTH                           (70)

/*!
\def SIM7600_DEFAULT_BAUDRATE
\brief default baudrade.
*/
#define SIM7600_DEFAULT_BAUDRATE                            (115200)

/*!
\def AT_OK_STRING
\brief AT command: ok message.
*/
#define AT_OK_STRING                                        ("OK")

/*!
\def AT_ERROR_STRING
\brief AT command: error message.
*/
#define AT_ERROR_STRING                                     ("ERROR")

/*!
\def AT_PB_DONE_STRING
\brief AT command: pb done message.
*/
#define AT_PB_DONE_STRING                                   ("PB DONE")

/*!
\def AT_CONNECT_OK_STRING
\brief AT command: on connect message.
*/
#define AT_CONNECT_OK_STRING                                ("CONNECT")

/*!
\def AT_OK_STRING
\brief AT command: ok message.
*/
#define AT_NO_CARRIER_STRING                                ("NO CARRIER")

/*!
\def AT_OK_STRING
\brief AT command: ok message.
*/
#define AT_DISCONNETTING_STRING                             ("DISCONNECTED")

/*!
\def SIM7600_USE_ROAMING_REGISTER
\brief try connection with roaming register network response as network registered OK
*/
#define SIM7600_USE_ROAMING_REGISTER                        (false)

/*!
\def SIM7600_AT_DEFAULT_TIMEOUT_MS
\brief Default AT command response timeout in milliseconds.
*/
#define SIM7600_AT_DEFAULT_TIMEOUT_MS                       (10000)

/*!
\def SIM7600_AT_DEFAULT_TIMEOUT_MS
\brief Default AT command response timeout in milliseconds.
*/
#define SIM7600_AT_FASTCMD_TIMEOUT_MS                       (1500)

/*!
\def SIM7600_AT_DELAY_MS
\brief Waiting time in milliseconds between two AT command.
*/
#define SIM7600_AT_DELAY_MS                                 (10)

/*!
\def SIM7600_GENERIC_RETRY_COUNT_MAX
\brief Number of retry in case of error.
*/
#define SIM7600_GENERIC_RETRY_COUNT_MAX                     (3)

/*!
\def SIM7600_GENERIC_WAIT_DELAY_MS
\brief Waiting time in milliseconds between two retry in milliseconds.
*/
#define SIM7600_GENERIC_WAIT_DELAY_MS                       (3000)

/*!
\def SIM7600_COMMAND_MODE_WAIT_DELAY_MS
\brief Guard time after +++ before probing AT (escape sequence).
*/
#define SIM7600_COMMAND_MODE_WAIT_DELAY_MS                  (1000)

/*!
\def SIM7600_COMMAND_MODE_SWITCH_DELAY_MS
\brief Guard time when skipping +++ (PPP already closed) before AT/ATH.
*/
#define SIM7600_COMMAND_MODE_SWITCH_DELAY_MS                (2500)

/*!
\def SIM7600_GENERIC_STATE_DELAY_MS
\brief Waiting time in milliseconds between two machine state.
*/
#define SIM7600_GENERIC_STATE_DELAY_MS                      (10)

/*!
\def SIM7600_WAIT_FOR_NETWORK_DELAY_MS
\brief Waiting time in milliseconds network availability.
*/
#define SIM7600_WAIT_FOR_NETWORK_DELAY_MS                   (2500)

/*!
\def SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX_GSM GPRS EUTRAN
\brief Max number of retry for checking network availability.
*/
#define SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX_GSM        (40)
#define SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX_GPRS       (20)
#define SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX_EUTRAN     (5)
#define SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX_TEST       (80)
/*!
\def SIM7600_TEST_ANTENNA_REFRESH_COUNT_MAX
\brief After registration OK in LCD test mode: CSQ/CNSMOD refresh cycles for antenna positioning, then start PPP.
*/
#define SIM7600_TEST_ANTENNA_REFRESH_COUNT_MAX              (10)

/*!
\def SIM7600_WAIT_FOR_UART_RECONFIGURE_DELAY_MS
\brief Waiting time in milliseconds for getting uart reconfiguration.
*/
#define SIM7600_WAIT_FOR_UART_RECONFIGURE_DELAY_MS          (1000)

/*!
\def SIM7600_WAIT_FOR_GET_SIGNAL_QUALITY_DELAY_MS
\brief Waiting time in milliseconds for getting signal quality.
*/
#define SIM7600_WAIT_FOR_SETUP_SET_MODE_NETWORK_DELAY_MS    (4000)

/*!
\def SIM7600_WAIT_FOR_GET_SIGNAL_QUALITY_DELAY_MS
\brief Waiting time in milliseconds for getting signal quality.
*/
#define SIM7600_WAIT_FOR_GET_SIGNAL_QUALITY_DELAY_MS        (3000)

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
\def SIM7600_WAIT_FOR_POWER_OFF_DELAY_MS
\brief Waiting time in milliseconds for powering sim7600.
*/
#define SIM7600_WAIT_FOR_POWER_OFF_CPOF_DELAY_MS            (2500)

/*!
\def SIM7600_POWER_OFF_BY_SWITCH
\brief Execute sim7600 poweroff by pulling down relative pin.
*/
#define SIM7600_POWER_OFF_BY_SWITCH                         (0x01)

/*!
\def SIM7600_POWER_OFF_BY_AT_COMMAND
\brief Execute sim7600 poweroff by sending relative AT command.
*/
#define SIM7600_POWER_OFF_BY_AT_COMMAND                     (0x02)

/*!
\def RSSI_MIN
\brief Minimum value of RSSI (low signal).
*/
#define RSSI_MIN                                            (0)

/*!
\def RSSI_MAX
\brief Minimum value of RSSI (high signal).
*/
#define RSSI_MAX                                            (31)

/*!
\def RSSI_UNKNOWN
\brief Unknown value of RSSI.
*/
#define RSSI_UNKNOWN                                        (99)

/*!
\def CNSMOD_UNKNOWN
\brief Unknown value of CNSMOD.
*/
#define CNSMOD_UNKNOWN                                      (0)

/*!
\def CREG_N_UNKNOWN
\brief Unknown value of CREG N.
*/
#define CREG_N_UNKNOWN                                      (255)

/*!
\def CREG_STAT_UNKNOWN
\brief Unknown value of CREG STAT.
*/
#define CREG_STAT_UNKNOWN                                   (255)

/*!
\def BER_MIN
\brief Minimum value of BER.
*/
#define BER_MIN                                             (0)

/*!
\def BER_MAX
\brief Maximum value of BER.
*/
#define BER_MAX                                             (7)

/*!
\def BER_UNKNOWN
\brief Unknown value of BER.
*/
#define BER_UNKNOWN                                         (99)

/*!
\def found(str, check)
\brief Return true or false if check string is found in str string.
*/
#define found(str, check)                                   (strstr(str, check))

/*!
\def printStatus(status, ok, error)
\brief Check if status is ok and print relative message.
*/
#define printStatus(status, ok, error)                      (status == SIM7600_OK ? ok : error)

/*!
\enum sim7600_connection_network_mode_t
\Network mode connection network selection type (only primary band)
*/
typedef enum
{
   SIM7600_MODE_NETWORK_DEFAULT = 0,   ///< Use default assigned method form HW module
   SIM7600_MODE_NETWORK_AUTO = 2,      ///< Use method AUTO for network connection
   SIM7600_MODE_NETWORK_2G = 13,       ///< Use only method 2G for network connection
   SIM7600_MODE_NETWORK_2G_3G = 19,    ///< Use either method 2G/3G for network connection
   SIM7600_MODE_NETWORK_3G = 14,       ///< Use only method 3G for network connection
   SIM7600_MODE_NETWORK_4G = 38,       ///< Use only method 4G for network connection
   SIM7600_MODE_NETWORK_2G_4G = 51     ///< Use either method 2G/4G for network connection
} sim7600_connection_network_mode_t;

/*!
\enum sim7600_type_network_registration_t
\Network type registration abilitate and verify (+CREG +CGRER + CEREG).
\Mode for GSM, GPRS, EUTRAN familiy band registration network
*/
typedef enum
{
   SIM7600_REG_NETWORK_NONE,        ///< registration network not required
   SIM7600_REG_NETWORK_GSM,         ///< registration network GSM minimal required
   SIM7600_REG_NETWORK_GPRS,        ///< registration network GPRS minimal required
   SIM7600_REG_NETWORK_EUTRAN       ///< registration network EUTRAN minimal required
} sim7600_type_network_registration_t;

/*!
\enum sim7600_type_network_mode_t
\Network mode registration abilitate and verify (+CNSMOD).
\Mode for GSM, GPRS, EUTRAN familiy band registration network
*/
typedef enum
{
   SIM7600_TYPE_NETWORK_NO_SERVICE,
   SIM7600_TYPE_NETWORK_GSM,
   SIM7600_TYPE_NETWORK_GPRS,
   SIM7600_TYPE_NETWORK_EDGE,
   SIM7600_TYPE_NETWORK_WCDMA,
   SIM7600_TYPE_NETWORK_HSDPA,
   SIM7600_TYPE_NETWORK_HSUPA,
   SIM7600_TYPE_NETWORK_HSPA,
   SIM7600_TYPE_NETWORK_LTE,
   SIM7600_TYPE_NETWORK_TDS_CDMA,
   SIM7600_TYPE_NETWORK_TDS_HSDPA,
   SIM7600_TYPE_NETWORK_TDS_HSUPA,
   SIM7600_TYPE_NETWORK_TDS_HSPA,
   SIM7600_TYPE_NETWORK_CDMA,
   SIM7600_TYPE_NETWORK_EVDO,
   SIM7600_TYPE_NETWORK_CDMA_EVDO,
   SIM7600_TYPE_NETWORK_CDMA_LTE
} sim7600_type_network_mode_t;

/*!
\enum sim7600_power_state_t
\brief Main loop finite state machine.
*/
typedef enum
{
   SIM7600_POWER_INIT,           ///< init task variables
   SIM7600_POWER_ENABLE,         ///< set sim7600 poweron/poweroff pin low
   SIM7600_POWER_IMPULSE_UP,     ///< set sim7600 poweron/poweroff pin high
   SIM7600_POWER_IMPULSE_DOWN,   ///< set sim7600 poweron/poweroff pin high
   SIM7600_POWER_CHECK_STATUS,   ///< check if sim7600 is on or is off
   SIM7600_POWER_END             ///< performs end operations and deactivate task
} sim7600_power_state_t;

/*!
\enum sim7600_power_state_t
\brief Main loop finite state machine.
*/
typedef enum
{
   SIM7600_POWER_OFF_INIT,         ///< init task variables
   SIM7600_POWER_OFF_END           ///< performs end operations and deactivate task
} sim7600_power_off_state_t;

/*!
\def SIM7600_DISABLE_URC
\brief Disable unsolicited +CREG/+CGREG/+CEREG/+CNSMOD after setup.
\Queries AT+CNSMOD? / AT+CSQ / AT+CxREG? remain available (LCD / periodic poll).
\Override from application config.h via STIMA_MODEM_DISABLE_URC.
*/
#ifndef STIMA_MODEM_DISABLE_URC
#define STIMA_MODEM_DISABLE_URC                             (true)
#endif
#define SIM7600_DISABLE_URC                                 (STIMA_MODEM_DISABLE_URC)

/*!
\enum sim7600_setup_state_t
\brief Main loop finite state machine.
*/
typedef enum {
   SIM7600_SETUP_INIT,                    ///< init task variables
   SIM7600_SETUP_RESET,                   ///< reset sim7600 to default state
   SIM7600_SETUP_ECHO_MODE,               ///< disable sim7600 echo mode
   SIM7600_SETUP_DISABLE_URC,             ///< disable CREG/CGREG/CEREG/CNSMOD unsolicited reports
   SIM7600_SETUP_GET_SIGNAL_QUALITY,      ///< get signal quality (first attempt)
   SIM7600_SETUP_CHANGE_BAUD_RATE,        ///< switch baud from defualt to fast rs232 speed
   SIM7600_SETUP_SET_PHONE_FUNCTIONALITY, ///< set on off or partial functionally for modem
   SIM7600_SETUP_SET_MODE_NETWORK,        ///< set type network prefered
   SIM7600_SETUP_SET_PRIORITY_NETWORK,    ///< set priority list type network (AT+CNAOP)
   SIM7600_SETUP_ENABLE_NETWORK,          ///< enable type of network required
   SIM7600_SETUP_WAIT_NETWORK,            ///< wait for network availability
   SIM7600_SETUP_UPDATE_SIGNAL_QUALITY,   ///< update signal quality (update on registartion loop)
   SIM7600_SETUP_UPDATE_NETWORK_MODE,     ///< update network registration mode (update on registartion loop)
   SIM7600_SETUP_END,                     ///< performs end operations and deactivate task
   SIM7600_SETUP_WAIT_STATE               ///< non-blocking waiting time
} sim7600_setup_state_t;

/*!
\enum sim7600_connection_start_state_t
\brief Main loop finite state machine.
*/
typedef enum
{
   SIM7600_CONNECTION_START_INIT,      ///< init task variables
   SIM7600_CONNECTION_START_PDP,       ///< check if sim7600 is attached to gprs
   SIM7600_CONNECTION_START_PDP_AUTH,  ///< starting up connection
   SIM7600_CONNECTION_START_CONNECT,   ///< starting up connection
   SIM7600_CONNECTION_START_END        ///< performs end operations and deactivate task
} sim7600_connection_start_state_t;

/*!
\enum sim7600_connection_stop_state_t
\brief Main loop finite state machine.
*/
typedef enum
{
   SIM7600_CONNECTION_STOP_INIT,                       ///< init task variables
   SIM7600_CONNECTION_ENTER_COMMAND_MODE,              ///< send +++ escape (optional)
   SIM7600_CHECK_ENTER_COMMAND_MODE,                   ///< verify AT command mode
   SIM7600_CONNECTION_STOP_HANGUP,                     ///< ATH hangup
   SIM7600_CONNECTION_STOP_CLOSE_PDP,                  ///< close pdp context
   SIM7600_CONNECTION_STOP_END                         ///< performs end operations and deactivate task
} sim7600_connection_stop_state_t;

/*!
\enum sim7600_state_t
\brief sim7600 finite state machine.
*/
typedef enum
{
   SIM7600_STATE_NONE = 0b00000000,             ///< default state at power on
   SIM7600_STATE_ON = 0b00000001,               ///< module is on
   SIM7600_STATE_SETTED = 0b00000010,           ///< module is is setted
   SIM7600_STATE_CONNECTED = 0b00000100         ///< module is is connected
} sim7600_state_t;

/*!
\enum sim7600_at_state_t
\brief Main loop finite state machine.
*/
typedef enum
{
   SIM7600_AT_INIT,    ///< init task variables
   SIM7600_AT_SEND,    ///< send AT command
   SIM7600_AT_RECEIVE, ///< wait for AT response
   SIM7600_AT_END      ///< performs end operations and deactivate task
} sim7600_at_state_t;

/*!
\enum sim7600_status_t
\brief Main loop finite state machine.
*/
typedef enum {
   SIM7600_BUSY,      ///< busy: sim7600 is doing something
   SIM7600_OK,        ///< operation complete with success
   SIM7600_ERROR      ///< operation abort due to error
} sim7600_status_t;

/*!
\class SIM7600
\brief SIM7600 class.
*/
class SIM7600
{
public:
   SIM7600();
   
   SIM7600(NetInterface *_interface, uint32_t _low_baud_rate, uint32_t _high_baud_rate, uint8_t _enable_power_pin, uint8_t _power_pin, uint8_t _ring_indicator_pin);

   /*!
   \fn bool isOn()
   \brief Check if sim7600 is on or is off.
   \return true if it is on, false if is off.
   */
   bool isOn();

   /*!
   \fn bool isSetted()
   \brief Check if sim7600 is setted.
   \return true if it is setted, false otherwise.
   */
   bool isSetted();

   /*!
   \fn bool isConnected()
   \brief Check if sim7600 is connected on network.
   \return true if it is connecteded, false otherwise.
   */
   bool isConnected();

   /*!
   \fn void setInterface(NetInterface *_interface)
   \brief Set cyclonetcp interface for sim7600.
   \param[in] *interface pointer to interface.
   \return void.
   */
   void setInterface(NetInterface *_interface);

   /*!
   \fn void setPins(uint8_t _enable_power_pin, uint8_t _power_pin, uint8_t _ring_inicator_pin)
   \brief set pins for module.
   \param[in] _enable_power_pin on/off pin for module.
   \param[in] _power_pin enable power for module.
   \param[in] _ring_inicator_pin input ring indicator for module.
   */
   void setPins(uint8_t _enable_power_pin, uint8_t _power_pin, uint8_t _ring_inicator_pin);

   void initPins(bool _set_direction);
   /*!
   \fn void initPins(_set_direction)
   \brief Init module pins with direction if required
   */

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
   \fn uint8_t getRssi()
   \brief get sim7600 rssi of the active connection.
   */
   uint8_t getRssi();

   /*!
    \fn uint8_t getBer()
    \brief get sim7600 ber of the active connection.
    */
   uint8_t getBer();

   /*!
    \fn uint8_t getCregN()
    \brief get sim7600 creg n of the active connection.
    */
   uint8_t getCregN();

   /*!
    \fn uint8_t getCnsmod()
    \brief get sim7600 Cnsmod of the active connection.
    */
   uint8_t getCnsmod();

   /*!
    \fn uint8_t getCregStat()
    \brief get sim7600 creg stat of the active connection.
    */
   uint8_t getCregStat();

   /*!
    \fn uint8_t getCgregN()
    \brief get sim7600 cgreg n of the active connection.
    */
   uint8_t getCgregN();

   /*!
    \fn uint8_t getCgregStat()
    \brief get sim7600 cgreg stat of the active connection.
    */
   uint8_t getCgregStat();

   /*!
    \fn uint8_t getCeregN()
    \brief get sim7600 cereg n of the active connection.
    */
   uint8_t getCeregN();

   /*!
    \fn uint8_t getCeregStat()
    \brief get sim7600 cereg stat of the active connection.
    */
   uint8_t getCeregStat();

   /*!
   \fn sim7600_status_t sendAtCxreg()
   \brief Send CREG AT command for reading network status.
   \return sim7600 status on each call.
   */
   sim7600_status_t sendAtCxreg(uint8_t cxreg_mode = sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM);

   /*!
   \fn sim7600_status_t sendAtCsq(uint8_t *rssi, uint8_t *ber)
   \brief Send CSQ AT command for reading signal quality.
   \param[out] *rssi pointer to variable containing rssi value.
   \param[out] *ber pointer to variable containing ber value.
   \return sim7600 status on each call.
   */
   sim7600_status_t sendAtCsq();

   /**
    * @brief 
    * 
    * @return sim7600_status_t 
    */
   sim7600_status_t sendAtCnsmod();

   /**
    * @brief 
    * 
    * @return gps_info 
    */
   sim7600_status_t sendAtGpsInfo();

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
   \param[in] network_type type base network abilitation AUTO, 2G, 3G, 4G mode (AT+CNMP)
   \param[in] network_regver type of registration minimal metwork to get setup OK
   \param[in] *network_order CNAOP preferred access order (existing CFG field)
   \param[in] test_connection if only for check signal_status (not for standard connection)
   \param[out] update_flags setted when update quality signal, cnsmod, registration newer value available
   \return sim7600 status on each call.
   */
   sim7600_status_t setup(sim7600_connection_network_mode_t network_type = SIM7600_MODE_NETWORK_DEFAULT,
                           sim7600_type_network_registration_t network_regver = sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN,
                           char* network_order = "",
                           bool test_connection = false, bool *update_flags = NULL);
   /*!
   \fn sim7600_status_t connect(const char *apn, const char *number)
   \brief Execute start connection sequence.
   \param[in] *apn apn for simcard network operation
   \param[in] *number number for simcard network operation
   \return sim7600 status on each call.
   */
   sim7600_status_t connect(const char *apn, const char *number);

   /*!
   \fn sim7600_status_t connection(const char *tipo, const char *server, const int port)
   \brief Execute connection sequence.
   \param[in] *tipo indicate what type of connection you want to open. Supported type are "UDP" and "TCP".
   \param[in] *server IP address or hostname of server.
   \param[in] port connection port.
   \return sim7600 status on each call.
   */
   // sim7600_status_t connection(const char *tipo, const char *server, const int port);

   /*!
   \fn sim7600_status_t disconnect(bool bUseEscapeSeq)
   \brief Exit data mode and hangup.
   \param[in] bUseEscapeSeq true = send +++ then AT then ATH (CSD/data modem);
              false = after PPP close, wait then AT then ATH (no +++).
   \return sim7600 status on each call.
   */
   sim7600_status_t disconnect(bool bUseEscapeSeq = false);

   /*!
   \fn void cleanInput()
   \brief Clear PPP RX buffer before sending AT (discard URC/stale bytes).
   \return void.
   */
   void cleanInput();

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

private:
   /*!
   \var buffer_ext
   \brief Buffer for send AT command and receive response.
   */
   char buffer_ext[SIM7600_BUFFER_LENGTH];

   /*!
   \var buffer_ptr
   \brief Buffer pointer for check part of AT command and receive response.
   */
   char *buffer_ptr;

   /*!
   \var buffer_cmd
   \brief Buffer for send AT command and receive response.
   */
   char buffer_cmd[SIM7600_BUFFER_CMD_LENGTH];

   /*!
   \fn sim7600_status_t switchModem(bool is_switching_on)
   \brief Switch on to off or off to on state.
   \param[in] is_switching_on if true, switch module off to on, if false, switch module on to off.
   \return sim7600 status on each call.
   */
   sim7600_status_t switchModem(bool is_switching_on);

   /*!
   \fn floatToString(float num, char *tOutStr)
   \brief Convert float to string for local NMEA GPS Conversion
   \param[in] num float to be converted.
   \param[out] *tOutStr string float converted output.
   */
   void floatToString(float num, char *tOutStr);

   /*!
   \fn void Convert_NMEA_Decimal(char* tNmeaGPSInfo, char* tDecimalLat, char* tDecimalLon, char* tAltitude)
   \brief Convert from NMEA Format to standard es. GoogleMaps GPS Coordinate format.
   \param[in] tNmeaGPSInfo string containing NMEA Output string SIM7600 Module.
   \param[out] tDecimalLat string containing latitude GPS format.
   \param[out] tDecimalLon string containing longitudeGPS format.
   \param[out] tAltitude string containing altitude in meters.
   */
   void convertGpsNMEADecimal(char* tNmeaGPSInfo, char* tDecimalLat, char* tDecimalLon, char* tAltitude);

   /*!
   \var low_baud_rate
   \brief baud rate for serial port of sim7600.
   */
   uint32_t low_baud_rate;

   /*!
   \var high_baud_rate
   \brief baud rate for serial port of sim7600.
   */
   uint32_t high_baud_rate;

   /*!
   \var interface
   \brief cylocnetcp interface for sim7600.
   */
   NetInterface *interface;

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
      \var rssi
      \brief sim7600 rssi of the active connection.
   */
   uint8_t sim7600_rssi;

   /*!
      \var cnsmod
      \brief sim7600 cnsmod of the active connection.
   */
   uint8_t sim7600_cnsmod;

   /*!
      \var ber
      \brief sim7600 ber of the active connection.
   */
   uint8_t sim7600_ber;

   /*!
      \var creg_n
      \brief sim7600 creg_n of the active connection.
   */
   uint8_t sim7600_creg_n;

   /*!
      \var creg_stat
      \brief sim7600 creg_stat of the active connection.
   */
   uint8_t sim7600_creg_stat;

   /*!
      \var cgreg_n
      \brief sim7600 cgreg_n of the active connection.
   */
   uint8_t sim7600_cgreg_n;

   /*!
      \var cgreg_stat
      \brief sim7600 cgreg_stat of the active connection.
   */
   uint8_t sim7600_cgreg_stat;

   /*!
      \var cereg_n
      \brief sim7600 cereg_n of the active connection.
   */
   uint8_t sim7600_cereg_n;

   /*!
      \var cereg_stat
      \brief sim7600 cereg_stat of the active connection.
   */
   uint8_t sim7600_cereg_stat;

   /*!
      \var gps_info
      \brief sim7600 gps_info of the active session.
   */
   char sim7600_gps_lat_pos[11] = {0};
   char sim7600_gps_lon_pos[12] = {0};
   char sim7600_gps_alt[8] = {0};
   bool sim7600_is_gps_ready = false;

};

#endif
