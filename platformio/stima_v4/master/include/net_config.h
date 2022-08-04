/**
 * @file net_config.h
 * @brief CycloneTCP configuration file
 *
 * @section License
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

#include "debug_config.h"

#define APP_ETHERNET_TICK_EVENT_HANDLER_MS    (100)
#define APP_MQTT_ATTEMPT_DELAY_MS             (1000)
#define APP_MQTT_TIMEOUT_MS                   (60000)
#define APP_MQTT_KEEP_ALIVE_S                 (30)
#define APP_MQTT_SERVER_NAME_LENGTH           (40)
#define APP_MQTT_CLIENT_IDENTIFIER_LENGTH     (40)
#define APP_MQTT_USERNAME_LENGTH              (40)
#define APP_MQTT_PASSWORD_LENGTH              (40)
#define APP_MQTT_WILL_TOPIC_LENGTH            (100)
#define APP_MQTT_WILL_MSG_LENGTH              (100)

#define APP_PSK_IDENTITY_LENGTH               (40)

// Ethernet interface configuration
#define APP_IF_NAME "eth0"
#define APP_HOST_NAME "STIMA"
#define APP_MAC_ADDR "9C-49-D0-29-1D-FE"

#define APP_USE_DHCP_CLIENT ENABLED
#define APP_IPV4_HOST_ADDR "192.168.88.50"
#define APP_IPV4_SUBNET_MASK "255.255.255.0"
#define APP_IPV4_DEFAULT_GATEWAY "192.168.88.1"
#define APP_IPV4_PRIMARY_DNS "8.8.8.8"
#define APP_IPV4_SECONDARY_DNS "8.8.4.4"

#define APP_USE_SLAAC DISABLED
#define APP_IPV6_LINK_LOCAL_ADDR "fe80::755"
#define APP_IPV6_PREFIX "2001:db8::"
#define APP_IPV6_PREFIX_LENGTH 64
#define APP_IPV6_GLOBAL_ADDR "2001:db8::755"
#define APP_IPV6_ROUTER "fe80::1"
#define APP_IPV6_PRIMARY_DNS "2001:4860:4860::8888"
#define APP_IPV6_SECONDARY_DNS "2001:4860:4860::8844"

// Application configuration
#define APP_MQTT_SERVER_NAME "test.rmap.cc"
#define APP_MQTT_SERVER_PORT 8885

#define APP_CLIENT_PSK_IDENTITY "userv4/stimav4/stima4"
#define APP_SET_CIPHER_SUITES ENABLED
#define APP_SET_SERVER_NAME ENABLED

//Number of network adapters
#define NET_INTERFACE_COUNT 1

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
#define IGMP_HOST_SUPPORT ENABLED

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
#define MLD_SUPPORT ENABLED

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

//LLMNR responder support
#define LLMNR_RESPONDER_SUPPORT ENABLED

//WebSocket support
#define WEB_SOCKET_SUPPORT DISABLED
//Support for WebSocket connections over TLS
#define WEB_SOCKET_TLS_SUPPORT DISABLED

//MQTT client support
#define MQTT_CLIENT_SUPPORT ENABLED
//MQTT over TLS
#define MQTT_CLIENT_TLS_SUPPORT ENABLED
//MQTT over WebSocket
#define MQTT_CLIENT_WS_SUPPORT DISABLED

#endif
