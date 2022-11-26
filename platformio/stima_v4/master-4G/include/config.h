/**@file config.h */

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

#ifndef _CONFIG_H
#define _CONFIG_H

#include "sensors_config.h"
#include "stima_config.h"

/*********************************************************************
* MODULE
*********************************************************************/
/*!
\def MODULE_MAIN_VERSION
\brief Module main version.
*/
#define MODULE_MAIN_VERSION   (4)

/*!
\def MODULE_MINOR_VERSION
\brief Module minor version.
*/
#define MODULE_MINOR_VERSION  (0)

/*!
\def RMAP_PROCOTOL_VERSION
\brief rmap protocol version
*/
#define RMAP_PROCOTOL_VERSION (1)

/*!
\def MODULE_TYPE
\brief Type of module. It is defined in registers.h.
*/
#define MODULE_TYPE (STIMA_MODULE_TYPE_MASTER_GSM)

#define CONFIGURATION_EEPROM_ADDRESS (0)

#define SERIAL_DEBUG_BAUD_RATE   (115200)

#define ENABLE_I2C1              (true)
#define ENABLE_I2C2              (true)
#define ENABLE_QSPI              (false)
#define _HW_SETUP_GPIO_PRIVATE

#define PPP0_INTERFACE_NAME      ("ppp0")
#define ETH0_INTERFACE_NAME      ("eth0")

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
#define INTERFACE_0_NAME         PPP0_INTERFACE_NAME
#define INTERFACE_0_INDEX        (0)
#define PPP0_TIMEOUT_MS          (10000)
#define PPP0_BAUD_RATE_DEFAULT   (115200)
#define PPP0_BAUD_RATE_MAX       (921600)
#define PPP0_PRIMARY_DNS         ("8.8.8.8")
#define PPP0_SECONDARY_DNS       ("8.8.4.4")

#elif (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_ETH)
#define INTERFACE_0_NAME         ETH0_INTERFACE
#define INTERFACE_0_INDEX        (0)
#endif

#define USE_MQTT  (true)
#define USE_NTP   (true)

/*!
\def USE_CONSTANTDATA_COUNT
\brief Constantdata count.
*/
#define USE_CONSTANTDATA_COUNT   (3)

#define SYSTEM_REQUEST_QUEUE_LENGTH       (1)
#define SYSTEM_RESPONSE_QUEUE_LENGTH      (1)
#define SYSTEM_STATUS_QUEUE_LENGTH        (1)

#define CONFIGURATION_DEFAULT_REPORT_S       (900)
#define CONFIGURATION_DEFAULT_OBSERVATION_S  (60)

/*!
\def CONFIGURATION_DEFAULT_NTP_SERVER
\brief Default ntp server.
*/
#define CONFIGURATION_DEFAULT_NTP_SERVER (NTP_DEFAULT_SERVER)

/*!
\def CONFIGURATION_DEFAULT_STATIONSLUG
\brief Default station slug.
*/
#define CONFIGURATION_DEFAULT_STATIONSLUG (DEFAULT_STATIONSLUG)

/*!
\def CONFIGURATION_DEFAULT_boardSLUG
\brief Default board slug.
*/
#define CONFIGURATION_DEFAULT_BOARDSLUG (DEFAULT_BOARDSLUG)

/*!
\def CONFIGURATION_DEFAULT_DATA_LEVEL
\brief Default data level.
*/
#define CONFIGURATION_DEFAULT_DATA_LEVEL (DATA_LEVEL_REPORT)

/*!
\def CONFIGURATION_DEFAULT_IDENT
\brief Default ident.
*/
#define CONFIGURATION_DEFAULT_IDENT ("")

/*!
\def CONFIGURATION_DEFAULT_NETWORK
\brief Default network.
*/
#define CONFIGURATION_DEFAULT_NETWORK (NETWORK_TEST)

/*!
\def CONFIGURATION_DEFAULT_LATITUDE
\brief Default latitude.
*/
#define CONFIGURATION_DEFAULT_LATITUDE (4412345)

/*!
\def CONFIGURATION_DEFAULT_LONGITUDE
\brief Default longitude.
*/
#define CONFIGURATION_DEFAULT_LONGITUDE (1112345)

#if (USE_MQTT)
/*!
\def CONFIGURATION_DEFAULT_MQTT_PORT
\brief Default mqtt server port.
*/
#define CONFIGURATION_DEFAULT_MQTT_PORT (MQTT_DEFAULT_PORT)

/*!
\def CONFIGURATION_DEFAULT_MQTT_SERVER
\brief Default mqtt server.
*/
#define CONFIGURATION_DEFAULT_MQTT_SERVER (MQTT_DEFAULT_SERVER)

/*!
\def CONFIGURATION_DEFAULT_MQTT_ROOT_TOPIC
\brief Default mqtt root topic.
*/
#define CONFIGURATION_DEFAULT_MQTT_ROOT_TOPIC (MQTT_DEFAULT_ROOT_TOPIC)

/*!
\def CONFIGURATION_DEFAULT_MQTT_MAINT_TOPIC
\brief Default mqtt maint topic.
*/
#define CONFIGURATION_DEFAULT_MQTT_MAINT_TOPIC (MQTT_DEFAULT_MAINT_TOPIC)

/*!
\def CONFIGURATION_DEFAULT_MQTT_RPC_TOPIC
\brief Default mqtt rpc topic.
*/
#define CONFIGURATION_DEFAULT_MQTT_RPC_TOPIC (MQTT_DEFAULT_RPC_TOPIC)

/*!
\def CONFIGURATION_DEFAULT_MQTT_USERNAME
\brief Default mqtt username.
*/
#define CONFIGURATION_DEFAULT_MQTT_USERNAME (MQTT_DEFAULT_USERNAME)

/*!
\def CONFIGURATION_DEFAULT_MQTT_PASSWORD
\brief Default mqtt password.
*/
#define CONFIGURATION_DEFAULT_MQTT_PASSWORD (MQTT_DEFAULT_PASSWORD)

#endif

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_ETH)
/*!
\def CONFIGURATION_DEFAULT_ETHERNET_DHCP_ENABLE
\brief Default DHCP status.
*/
#define CONFIGURATION_DEFAULT_ETHERNET_DHCP_ENABLE (ETHERNET_DEFAULT_DHCP_ENABLE)

/*!
\def CONFIGURATION_DEFAULT_ETHERNET_MAC
\brief Default mac address.
*/
#define CONFIGURATION_DEFAULT_ETHERNET_MAC (ETHERNET_DEFAULT_MAC)

/*!
\def CONFIGURATION_DEFAULT_ETHERNET_IP
\brief Default ip address.
*/
#define CONFIGURATION_DEFAULT_ETHERNET_IP (ETHERNET_DEFAULT_IP)

/*!
\def CONFIGURATION_DEFAULT_ETHERNET_NETMASK
\brief Default netmask address.
*/
#define CONFIGURATION_DEFAULT_ETHERNET_NETMASK (ETHERNET_DEFAULT_NETMASK)

/*!
\def CONFIGURATION_DEFAULT_ETHERNET_GATEWAY
\brief Default gateway address.
*/
#define CONFIGURATION_DEFAULT_ETHERNET_GATEWAY (ETHERNET_DEFAULT_GATEWAY)

/*!
\def CONFIGURATION_DEFAULT_ETHERNET_PRIMARY_DNS
\brief Default primary dns address.
*/
#define CONFIGURATION_DEFAULT_ETHERNET_PRIMARY_DNS (ETHERNET_DEFAULT_PRIMARY_DNS)
#endif

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
/*!
\def CONFIGURATION_DEFAULT_GSM_APN
\brief Default gsm apn.
*/
#define CONFIGURATION_DEFAULT_GSM_APN (GSM_DEFAULT_APN)

/*!
\def CONFIGURATION_DEFAULT_GSM_NUMBER
\brief Default gsm number.
*/
#define CONFIGURATION_DEFAULT_GSM_NUMBER (GSM_DEFAULT_NUMBER)

/*!
\def CONFIGURATION_DEFAULT_GSM_USERNAME
\brief Default gsm username.
*/
#define CONFIGURATION_DEFAULT_GSM_USERNAME (GSM_DEFAULT_USERNAME)

/*!
\def CONFIGURATION_DEFAULT_GSM_PASSWORD
\brief Default gsm password.
*/
#define CONFIGURATION_DEFAULT_GSM_PASSWORD (GSM_DEFAULT_PASSWORD)
#endif

#endif
