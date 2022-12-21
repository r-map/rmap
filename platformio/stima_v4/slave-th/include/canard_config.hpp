/**
  ******************************************************************************
  * @file    canard_config.hpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Uavcan Canard Configuration file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (C) 2022  Moreno Gasperini <m.gasperini@digiteco.it>
  * All rights reserved.</center></h2>
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
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
  * <http://www.gnu.org/licenses/>.
  * 
  ******************************************************************************
*/

// Assert Locali
#define LOCAL_ASSERT    assert

#define KILO 1000L
#define MEGA ((int64_t)KILO * KILO)

// CODA, RIDONDANZA, TIMEDELAY TX & RX CANARD
#define CAN_REDUNDANCY_FACTOR 1
#define CAN_TX_QUEUE_CAPACITY 100
#define CAN_MAX_IFACE         1
#define CAN_RX_QUEUE_CAPACITY 100
#define IFACE_CAN_IDX         0
#define CAN_DELAY_US_SEND     0
#define MAX_SUBSCRIPTION      10

// CAN SPEED RATE HZ
#define CAN_BIT_RATE 1000000ul
#define CAN_MTU_BASE 8

// Messaggi di lunghezza TimeOut Extra <> Standard
#define CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC 3500000UL
#define CANARD_READFILE_TRANSFER_ID_TIMEOUT_USEC     2500000UL

// A compilazione per semplificazione setup nodo (TEST)
// #define INIT_REGISTER

// Nodo Fisso per Modulo Master
#define NODE_MASTER_ID 100

// Nodo fisso per Modulo Slave
#define NODE_SLAVE_ID 125

// SET Default value per risposte
#define GENERIC_STATE_UNDEFINED 0x0Fu
#define GENERIC_BVAL_UNDEFINED  0xFFu

// Servizi Cypal attivi di default
#define DEFAULT_PUBLISH_PORT_LIST   true
#define DEFAULT_PUBLISH_MODULE_DATA false

// Time Publisher Servizi (secondi)
#define TIME_PUBLISH_MODULE_DATA    0.333
#define TIME_PUBLISH_PNP_REQUEST    2
#define TIME_PUBLISH_HEARTBEAT      1
#define TIME_PUBLISH_PORT_LIST      20

// TimeOUT (millisecondi)
#define MASTER_OFFLINE_TIMEOUT_US 6000000
#define MASTER_MAXSYNCRO_VALID_US 1250000
#define NODE_GETFILE_TIMEOUT_US   1750000
#define NODE_GETFILE_MAX_RETRY    3

// CODICI E STATUS AGGIORNAMENTO FILE REMOTI
#define FILE_NAME_SIZE_MAX 50
