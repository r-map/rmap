/**
 * @file debug_config.h
 * @brief CycloneCRYPTO configuration file
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2022 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneCRYPTO Open.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.1.4
 **/

#ifndef _DEBUG_CONFIG_H
#define _DEBUG_CONFIG_H

#define SERIAL_STREAM  Serial

//Trace level for TCP/IP stack debugging
#define MEM_TRACE_LEVEL          TRACE_LEVEL_INFO
#define NIC_TRACE_LEVEL          TRACE_LEVEL_INFO
#define ETH_TRACE_LEVEL          TRACE_LEVEL_VERBOSE
#define LLDP_TRACE_LEVEL         TRACE_LEVEL_OFF
#define ARP_TRACE_LEVEL          TRACE_LEVEL_OFF
#define IP_TRACE_LEVEL           TRACE_LEVEL_OFF
#define IPV4_TRACE_LEVEL         TRACE_LEVEL_OFF
#define IPV6_TRACE_LEVEL         TRACE_LEVEL_OFF
#define ICMP_TRACE_LEVEL         TRACE_LEVEL_OFF
#define IGMP_TRACE_LEVEL         TRACE_LEVEL_OFF
#define ICMPV6_TRACE_LEVEL       TRACE_LEVEL_OFF
#define MLD_TRACE_LEVEL          TRACE_LEVEL_OFF
#define NDP_TRACE_LEVEL          TRACE_LEVEL_OFF
#define UDP_TRACE_LEVEL          TRACE_LEVEL_OFF
#define TCP_TRACE_LEVEL          TRACE_LEVEL_OFF
#define SOCKET_TRACE_LEVEL       TRACE_LEVEL_OFF
#define RAW_SOCKET_TRACE_LEVEL   TRACE_LEVEL_OFF
#define BSD_SOCKET_TRACE_LEVEL   TRACE_LEVEL_OFF
#define WEB_SOCKET_TRACE_LEVEL   TRACE_LEVEL_OFF
#define AUTO_IP_TRACE_LEVEL      TRACE_LEVEL_INFO
#define SLAAC_TRACE_LEVEL        TRACE_LEVEL_INFO
#define DHCP_TRACE_LEVEL         TRACE_LEVEL_VERBOSE
#define DHCPV6_TRACE_LEVEL       TRACE_LEVEL_VERBOSE
#define DNS_TRACE_LEVEL          TRACE_LEVEL_INFO
#define MDNS_TRACE_LEVEL         TRACE_LEVEL_OFF
#define NBNS_TRACE_LEVEL         TRACE_LEVEL_OFF
#define LLMNR_TRACE_LEVEL        TRACE_LEVEL_OFF
#define COAP_TRACE_LEVEL         TRACE_LEVEL_INFO
#define FTP_TRACE_LEVEL          TRACE_LEVEL_INFO
#define HTTP_TRACE_LEVEL         TRACE_LEVEL_DEBUG
#define MQTT_TRACE_LEVEL         TRACE_LEVEL_INFO
#define MQTT_SN_TRACE_LEVEL      TRACE_LEVEL_INFO
#define SMTP_TRACE_LEVEL         TRACE_LEVEL_INFO
#define SNMP_TRACE_LEVEL         TRACE_LEVEL_INFO
#define SNTP_TRACE_LEVEL         TRACE_LEVEL_INFO
#define TFTP_TRACE_LEVEL         TRACE_LEVEL_INFO
#define MODBUS_TRACE_LEVEL       TRACE_LEVEL_INFO

#endif