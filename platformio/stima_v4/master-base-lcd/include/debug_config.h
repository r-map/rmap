/**@file debug_config.h */

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

#ifndef _DEBUG_CONFIG_H
#define _DEBUG_CONFIG_H

#define MEM_TRACE_LEVEL           TRACE_LEVEL_OFF
#define NIC_TRACE_LEVEL           TRACE_LEVEL_OFF
#define ETH_TRACE_LEVEL           TRACE_LEVEL_OFF
#define LLDP_TRACE_LEVEL          TRACE_LEVEL_OFF
#define ARP_TRACE_LEVEL           TRACE_LEVEL_OFF
#define IP_TRACE_LEVEL            TRACE_LEVEL_OFF
#define IPV4_TRACE_LEVEL          TRACE_LEVEL_OFF
#define IPV6_TRACE_LEVEL          TRACE_LEVEL_OFF
#define ICMP_TRACE_LEVEL          TRACE_LEVEL_OFF
#define IGMP_TRACE_LEVEL          TRACE_LEVEL_OFF
#define ICMPV6_TRACE_LEVEL        TRACE_LEVEL_OFF
#define MLD_TRACE_LEVEL           TRACE_LEVEL_OFF
#define NDP_TRACE_LEVEL           TRACE_LEVEL_OFF
#define UDP_TRACE_LEVEL           TRACE_LEVEL_OFF
#define TCP_TRACE_LEVEL           TRACE_LEVEL_OFF
#define SOCKET_TRACE_LEVEL        TRACE_LEVEL_OFF
#define RAW_SOCKET_TRACE_LEVEL    TRACE_LEVEL_OFF
#define BSD_SOCKET_TRACE_LEVEL    TRACE_LEVEL_OFF
#define WEB_SOCKET_TRACE_LEVEL    TRACE_LEVEL_OFF
#define AUTO_IP_TRACE_LEVEL       TRACE_LEVEL_OFF
#define SLAAC_TRACE_LEVEL         TRACE_LEVEL_OFF
#define DHCP_TRACE_LEVEL          TRACE_LEVEL_OFF
#define DHCPV6_TRACE_LEVEL        TRACE_LEVEL_OFF
#define DNS_TRACE_LEVEL           TRACE_LEVEL_OFF
#define MDNS_TRACE_LEVEL          TRACE_LEVEL_OFF
#define NBNS_TRACE_LEVEL          TRACE_LEVEL_OFF
#define LLMNR_TRACE_LEVEL         TRACE_LEVEL_OFF
#define COAP_TRACE_LEVEL          TRACE_LEVEL_OFF
#define FTP_TRACE_LEVEL           TRACE_LEVEL_OFF
#define HTTP_TRACE_LEVEL          TRACE_LEVEL_OFF
#define MQTT_TRACE_LEVEL          TRACE_LEVEL_OFF
#define MQTT_SN_TRACE_LEVEL       TRACE_LEVEL_OFF
#define SMTP_TRACE_LEVEL          TRACE_LEVEL_OFF
#define SNMP_TRACE_LEVEL          TRACE_LEVEL_OFF
#define SNTP_TRACE_LEVEL          TRACE_LEVEL_OFF
#define TFTP_TRACE_LEVEL          TRACE_LEVEL_OFF
#define MODBUS_TRACE_LEVEL        TRACE_LEVEL_OFF
#define TLS_TRACE_LEVEL           TRACE_LEVEL_OFF
#define CRYPTO_TRACE_LEVEL        TRACE_LEVEL_OFF

#define STIMA_TRACE_LEVEL              TRACE_LEVEL_INFO
#define LED_TASK_TRACE_LEVEL           TRACE_LEVEL_OFF
#define ETHERNET_TASK_TRACE_LEVEL      TRACE_LEVEL_OFF
#define MQTT_TASK_TRACE_LEVEL          TRACE_LEVEL_OFF
#define SUPERVISOR_TASK_TRACE_LEVEL    TRACE_LEVEL_VERBOSE
#define PROVA_TASK_TRACE_LEVEL         TRACE_LEVEL_INFO
#define LCD_TASK_TRACE_LEVEL           TRACE_LEVEL_VERBOSE

#endif
