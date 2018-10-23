/**@file mqtt_config.h */

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

#ifndef _MQTT_CONFIG_H
#define _MQTT_CONFIG_H

/*!
\def MQTT_ROOT_TOPIC_LENGTH
\brief Length in bytes for mqtt root topic.
*/
#define MQTT_ROOT_TOPIC_LENGTH         (50)

/*!
\def MQTT_MAINT_TOPIC_LENGTH
\brief Length in bytes for mqtt maint topic.
*/
#define MQTT_MAINT_TOPIC_LENGTH        (MQTT_ROOT_TOPIC_LENGTH)

/*!
\def MQTT_SUBSCRIBE_TOPIC_LENGTH
\brief Length in bytes for mqtt subscibe topic.
*/
#define MQTT_SUBSCRIBE_TOPIC_LENGTH    (50)

/*!
\def MQTT_SENSOR_TOPIC_LENGTH
\brief Length in bytes for mqtt sensor topic.
*/
#define MQTT_SENSOR_TOPIC_LENGTH       (30)

/*!
\def MQTT_CLIENT_ID_LENGTH
\brief Length in bytes for mqtt client id.
*/
#define MQTT_CLIENT_ID_LENGTH          (MQTT_ROOT_TOPIC_LENGTH)

/*!
\def MQTT_MESSAGE_LENGTH
\brief Length in bytes for mqtt message.
*/
#define MQTT_MESSAGE_LENGTH            (50)

/*!
\def MQTT_SERVER_LENGTH
\brief Length in bytes for mqtt server.
*/
#define MQTT_SERVER_LENGTH             (30)

/*!
\def MQTT_USERNAME_LENGTH
\brief Length in bytes for mqtt username.
*/
#define MQTT_USERNAME_LENGTH           (30)

/*!
\def MQTT_PASSWORD_LENGTH
\brief Length in bytes for mqtt password.
*/
#define MQTT_PASSWORD_LENGTH           (30)

/*!
\def MQTT_TIMEOUT_MS
\brief Timeout in milliseconds for mqtt stack.
*/
#define MQTT_TIMEOUT_MS                (6000)

/*!
\def MQTT_DEFAULT_SERVER
\brief Default MQTT server.
*/
#define MQTT_DEFAULT_SERVER            ("rmap.cc")

/*!
\def MQTT_DEFAULT_PORT
\brief Default MQTT server port.
*/
#define MQTT_DEFAULT_PORT              (1883)

/*!
\def MQTT_DEFAULT_ROOT_TOPIC
\brief Default MQTT root topic.
*/
#define MQTT_DEFAULT_ROOT_TOPIC        ("")

/*!
\def MQTT_DEFAULT_MAINT_TOPIC
\brief Default MQTT maint topic.
*/
#define MQTT_DEFAULT_MAINT_TOPIC       ("")

/*!
\def MQTT_DEFAULT_SUBSCRIBE_TOPIC
\brief Default MQTT subscibe topic.
*/
#define MQTT_DEFAULT_SUBSCRIBE_TOPIC   ("")

/*!
\def MQTT_DEFAULT_USERNAME
\brief Default MQTT username.
*/
#define MQTT_DEFAULT_USERNAME          ("")

/*!
\def MQTT_DEFAULT_PASSWORD
\brief Default MQTT password.
*/
#define MQTT_DEFAULT_PASSWORD          ("")

/*!
\def MQTT_STATUS_TOPIC
\brief Default MQTT status topic for printing on connect/disconnect message.
*/
#define MQTT_STATUS_TOPIC              ("254,0,0/265,0,-,-/B01213")

/*!
\def MQTT_ON_CONNECT_MESSAGE
\brief MQTT on connect message.
*/
#define MQTT_ON_CONNECT_MESSAGE        ("{\"v\":\"conn\"}")

/*!
\def MQTT_ON_DISCONNECT_MESSAGE
\brief MQTT on disconnect message.
*/
#define MQTT_ON_DISCONNECT_MESSAGE     ("{\"v\":\"disconn\"}")

/*!
\def MQTT_ON_ERROR_MESSAGE
\brief MQTT on error message.
*/
#define MQTT_ON_ERROR_MESSAGE          ("{\"v\":\"error01\"}")

#if (MQTT_ROOT_TOPIC_LENGTH + MQTT_SENSOR_TOPIC_LENGTH > 100)
#error MQTT root/sensor topic is too big!
#endif

#endif
