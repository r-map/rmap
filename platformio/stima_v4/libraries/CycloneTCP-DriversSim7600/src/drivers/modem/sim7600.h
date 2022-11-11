// /**
//  * @file modem.h
//  * @brief Modem configuration
//  *
//  * @section License
//  *
//  * SPDX-License-Identifier: GPL-2.0-or-later
//  *
//  * Copyright (C) 2010-2022 Oryx Embedded SARL. All rights reserved.
//  *
//  * This program is free software; you can redistribute it and/or
//  * modify it under the terms of the GNU General Public License
//  * as published by the Free Software Foundation; either version 2
//  * of the License, or (at your option) any later version.
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program; if not, write to the Free Software Foundation,
//  * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//  *
//  * @author Oryx Embedded SARL (www.oryx-embedded.com)
//  * @version 2.1.4
//  **/

// #ifndef _MODEM_H
// #define _MODEM_H

// //Dependencies
// #include "debug_config.h"
// #include "core/net.h"
// #include "ppp/ppp.h"
// #include "debug.h"

// #define SIM7600_MODULE_NAME   ("SIM7600E")

// /*!
// \def SIM7600_BUFFER_LENGTH
// \brief Length of sim7600 buffer.
// */
// #define SIM7600_BUFFER_LENGTH (128)

// /*!
// \def SIM7600_SERIAL_PORT
// \brief Default sim7600 serial port.
// */
// // #define SIM7600_SERIAL_PORT (Serial2)

// /*!
// \def SIM7600_SERIAL_PORT_BAUD_RATE
// \brief Baud rate of sim7600 serial port.
// */
// // #define SIM7600_SERIAL_PORT_BAUD_RATE (115200)

// /*!
// \def AT_OK_STRING
// \brief AT command: ok message.
// */
// #define AT_OK_STRING (OK_STRING)

// /*!
// \def AT_ERROR_STRING
// \brief AT command: error message.
// */
// #define AT_ERROR_STRING (ERROR_STRING)

// /*!
// \def SIM7600_POWER_OFF_BY_SWITCH
// \brief Execute sim7600 poweroff by pulling down relative pin.
// */
// #define SIM7600_POWER_OFF_BY_SWITCH (0x01)

// /*!
// \def SIM7600_POWER_OFF_BY_AT_COMMAND
// \brief Execute sim7600 poweroff by sending relative AT command.
// */
// #define SIM7600_POWER_OFF_BY_AT_COMMAND (0x02)

// /*!
// \def SIM7600_POWER_ON_OFF_SWITCH_DELAY_MS
// \brief Waiting time in milliseconds for putting sim7600 on or off by pulling down on/off pin.
// */
// #define SIM7600_POWER_ON_OFF_SWITCH_DELAY_MS (500)

// /*!
// \def SIM7600_POWER_ON_OFF_DONE_DELAY_MS
// \brief Waiting time in milliseconds for sim7600 startup or shutdown sequence.
// */
// #define SIM7600_POWER_ON_OFF_DONE_DELAY_MS (2000)

// /*!
// \enum sim7600_state_t
// \brief module state
// */
// typedef enum
// {
//    SIM7600_STATE_NONE,
//    SIM7600_STATE_ON,
//    SIM7600_STATE_INITIALIZED,
//    SIM7600_STATE_SETTED,
//    SIM7600_STATE_REGISTERED,
//    SIM7600_STATE_CONNECTED
// } sim7600_state_t;

// /*!
// \enum sim7600_status_t
// \brief module status
// */
// typedef enum
// {
//    SIM7600_BUSY, //!< module is doing something
//    SIM7600_OK,   //!< operation complete with success
//    SIM7600_ERROR //!< operation abort due to error
// } sim7600_status_t;

// /*!
// \enum sim7600_power_state_t
// \brief Main loop finite state machine.
// */
// typedef enum
// {
//    SIM7600_POWER_INIT,         //!< init task variables
//    SIM7600_POWER_SET_PIN_LOW,  //!< set sim7600 poweron/poweroff pin low
//    SIM7600_POWER_SET_PIN_HIGH, //!< set sim7600 poweron/poweroff pin high
//    SIM7600_POWER_CHECK_STATUS, //!< check if sim7600 is on or is off
//    SIM7600_POWER_END,          //!< performs end operations and deactivate task
//    SIM7600_POWER_WAIT_STATE    //!< non-blocking waiting time
// } sim7600_power_state_t;

// sim7600_state_t sim7600_state;
// sim7600_power_state_t sim7600_power_state;

// /*!
// \def printStatus(status, ok, error)
// \brief Check if status is ok and print relative message.
// */
// #define printStatus(status, ok, error) (status == NO_ERROR ? ok : error)

// //C++ guard
// #ifdef __cplusplus
//     extern "C" {
// #endif

// //Modem related functions
// error_t modemSwitchOn();
// error_t modemSwitchOff(uint8_t power_off_method)
// error_t modemSwitch();
// error_t modemInit(NetInterface *interface);
// error_t modemConfigure(NetInterface *interface);
// error_t modemConnect(NetInterface *interface);
// error_t modemDisconnect(NetInterface *interface);

// error_t modemSendAtCommand(NetInterface *interface, const char_t *command, char_t *response, size_t size);

// //C++ guard
// #ifdef __cplusplus
// }
// #endif

// #endif
