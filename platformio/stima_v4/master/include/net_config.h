/**
 * @file net_config.h
 * @brief CycloneTCP configuration file
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2022 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneTCP Open.
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

#ifndef _NET_CONFIG_H
#define _NET_CONFIG_H

#define MEM_TRACE_LEVEL          TRACE_LEVEL_OFF
#define NIC_TRACE_LEVEL          TRACE_LEVEL_OFF
#define ETH_TRACE_LEVEL          TRACE_LEVEL_OFF
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
#define AUTO_IP_TRACE_LEVEL      TRACE_LEVEL_OFF
#define SLAAC_TRACE_LEVEL        TRACE_LEVEL_OFF
#define DHCP_TRACE_LEVEL         TRACE_LEVEL_OFF
#define DHCPV6_TRACE_LEVEL       TRACE_LEVEL_OFF
#define DNS_TRACE_LEVEL          TRACE_LEVEL_OFF
#define MDNS_TRACE_LEVEL         TRACE_LEVEL_OFF
#define NBNS_TRACE_LEVEL         TRACE_LEVEL_OFF
#define LLMNR_TRACE_LEVEL        TRACE_LEVEL_OFF
#define COAP_TRACE_LEVEL         TRACE_LEVEL_OFF
#define FTP_TRACE_LEVEL          TRACE_LEVEL_OFF
#define HTTP_TRACE_LEVEL         TRACE_LEVEL_OFF
#define MQTT_TRACE_LEVEL         TRACE_LEVEL_OFF
#define MQTT_SN_TRACE_LEVEL      TRACE_LEVEL_OFF
#define SMTP_TRACE_LEVEL         TRACE_LEVEL_OFF
#define SNMP_TRACE_LEVEL         TRACE_LEVEL_OFF
#define SNTP_TRACE_LEVEL         TRACE_LEVEL_OFF
#define TFTP_TRACE_LEVEL         TRACE_LEVEL_OFF
#define MODBUS_TRACE_LEVEL       TRACE_LEVEL_OFF
#define PPP_TRACE_LEVEL          TRACE_LEVEL_OFF

//Number of network adapters
#define NET_INTERFACE_COUNT 1

//Ethernet support
#define ETH_SUPPORT DISABLED
//Size of the MAC address filter
#define MAC_ADDR_FILTER_SIZE 12

//IPv4 support
#define IPV4_SUPPORT ENABLED
//Size of the IPv4 multicast filter
#define IPV4_MULTICAST_FILTER_SIZE 4

//IPv4 fragmentation support
#define IPV4_FRAG_SUPPORT ENABLED
//Maximum number of fragmented packets the host will accept
//and hold in the reassembly queue simultaneously
#define IPV4_MAX_FRAG_DATAGRAMS 4
//Maximum datagram size the host will accept when reassembling fragments
#define IPV4_MAX_FRAG_DATAGRAM_SIZE 8192

//Size of ARP cache
#define ARP_CACHE_SIZE 8
//Maximum number of packets waiting for address resolution to complete
#define ARP_MAX_PENDING_PACKETS 2

//IGMP host support
#define IGMP_HOST_SUPPORT DISABLED
//DHCP client support
#define DHCP_CLIENT_SUPPORT DISABLED

//IPv6 support
#define IPV6_SUPPORT DISABLED
//Size of the IPv6 multicast filter
#define IPV6_MULTICAST_FILTER_SIZE 8

//IPv6 fragmentation support
#define IPV6_FRAG_SUPPORT ENABLED
//Maximum number of fragmented packets the host will accept
//and hold in the reassembly queue simultaneously
#define IPV6_MAX_FRAG_DATAGRAMS 4
//Maximum datagram size the host will accept when reassembling fragments
#define IPV6_MAX_FRAG_DATAGRAM_SIZE 8192

//MLD support
#define MLD_SUPPORT DISABLED

//Neighbor cache size
#define NDP_NEIGHBOR_CACHE_SIZE 8
//Destination cache size
#define NDP_DEST_CACHE_SIZE 8
//Maximum number of packets waiting for address resolution to complete
#define NDP_MAX_PENDING_PACKETS 2

//TCP support
#define TCP_SUPPORT ENABLED
//Default buffer size for transmission
#define TCP_DEFAULT_TX_BUFFER_SIZE (1430*2)
//Default buffer size for reception
#define TCP_DEFAULT_RX_BUFFER_SIZE (1430*2)
//Default SYN queue size for listening sockets
#define TCP_DEFAULT_SYN_QUEUE_SIZE 4
//Maximum number of retransmissions
#define TCP_MAX_RETRIES 5
//Selective acknowledgment support
#define TCP_SACK_SUPPORT DISABLED

//UDP support
#define UDP_SUPPORT ENABLED
//Receive queue depth for connectionless sockets
#define UDP_RX_QUEUE_SIZE 4

//Raw socket support
#define RAW_SOCKET_SUPPORT DISABLED
//Receive queue depth for raw sockets
#define RAW_SOCKET_RX_QUEUE_SIZE 4

//Number of sockets that can be opened simultaneously
#define SOCKET_MAX_COUNT 10

#define DNS_CLIENT_SUPPORT ENABLED
#define DNS_CLIENT_INIT_TIMEOUT (10000)
#define DNS_CLIENT_MAX_TIMEOUT (10000)

//NBNS client support
#define NBNS_CLIENT_SUPPORT DISABLED
//NBNS responder support
#define NBNS_RESPONDER_SUPPORT DISABLED

//HTTP client support
#define HTTP_CLIENT_SUPPORT ENABLED
//HTTP over TLS
#define HTTP_CLIENT_TLS_SUPPORT ENABLED
//HTTP timeout
#define HTTP_CLIENT_TIMEOUT_MS (10000)

// SNTP timeout
#define SNTP_CLIENT_TIMEOUT_MS (10000)

//PPP support
#define PPP_SUPPORT ENABLED
//PAP authentication support
#define PAP_SUPPORT DISABLED
//CHAP authentication support
#define CHAP_SUPPORT DISABLED

// LLMNR responder support
#define LLMNR_RESPONDER_SUPPORT ENABLED

// WebSocket support
#define WEB_SOCKET_SUPPORT DISABLED
// Support for WebSocket connections over TLS
#define WEB_SOCKET_TLS_SUPPORT DISABLED

// MQTT client support
#define MQTT_CLIENT_SUPPORT ENABLED
// MQTT over TLS
#define MQTT_CLIENT_TLS_SUPPORT ENABLED
// MQTT over WebSocket
#define MQTT_CLIENT_WS_SUPPORT DISABLED

#endif
