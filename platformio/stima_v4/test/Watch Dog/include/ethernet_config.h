/**@file ethernet_config.h */

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

#ifndef _ETHERNET_CONFIG_H
#define _ETHERNET_CONFIG_H

/*!
\def ETHERNET_DEFAULT_DHCP_ENABLE
\brief Default option for enable or disable DHCP protocol with ethernet.
*/
#define ETHERNET_DEFAULT_DHCP_ENABLE            (true)

/*!
\def ETHERNET_DEFAULT_MAC
\brief Default mac address for ethernet device.
*/
#define ETHERNET_DEFAULT_MAC                    ("E2:21:B6:44:EB:29")

/*!
\def ETHERNET_DEFAULT_IP
\brief Default ip address for ethernet device.
*/
#define ETHERNET_DEFAULT_IP                     ("192.168.0.100")

/*!
\def ETHERNET_DEFAULT_NETMASK
\brief Default netmask for ethernet device.
*/
#define ETHERNET_DEFAULT_NETMASK                ("255.255.255.0")

/*!
\def ETHERNET_DEFAULT_GATEWAY
\brief Default gateway for ethernet device.
*/
#define ETHERNET_DEFAULT_GATEWAY                ("192.168.0.1")

/*!
\def ETHERNET_DEFAULT_PRIMARY_DNS
\brief Default primary dns for ethernet device.
*/
#define ETHERNET_DEFAULT_PRIMARY_DNS            ("192.168.0.1")

/*!
\def ETHERNET_DEFAULT_LOCAL_UDP_PORT
\brief Default local udp port for ethernet device.
*/
#define ETHERNET_DEFAULT_LOCAL_UDP_PORT         (8000)

/*!
\def ETHERNET_ATTEMPT_MS
\brief Set next ethernet library attempt delay in milliseconds after a failure.
*/
#define ETHERNET_ATTEMPT_MS                     (2000)

/*!
\def ETHERNET_RETRY_TIME_MS
\brief Set next ethernet task attempt delay in milliseconds after a failure.
*/
#define ETHERNET_RETRY_TIME_MS                  (4000)

/*!
\def ETHERNET_RETRY_COUNT
\brief Maximum number of retry for ethernet task.
*/
#define ETHERNET_RETRY_COUNT                    (3)

/*!
\def ETHERNET_MQTT_TIMEOUT_MS
\brief MQTT timeout in milliseconds for ethernet device.
*/
#define ETHERNET_MQTT_TIMEOUT_MS                (6000)

/*!
\def ETHERNET_MAC_LENGTH
\brief Length in bytes for mac address.
*/
#define ETHERNET_MAC_LENGTH                     (6)

/*!
\def ETHERNET_IP_LENGTH
\brief Length in bytes for ip address.
*/
#define ETHERNET_IP_LENGTH                      (4)

#endif
