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

// Assert Locali (Enable / disable)
#define LOCAL_ASSERT    assert
// #define LOCAL_ASSERT    (void(0));

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
#define HEAP_ARENA_SIZE       (1024 * 16)

// CAN SPEED RATE HZ
#define CAN_BIT_RATE 1000000ul
#define CAN_MTU_BASE 8

// Messaggi di lunghezza TimeOut Extra <> Standard
#define CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC 3500000UL
#define CANARD_READFILE_TRANSFER_ID_TIMEOUT_USEC     2500000UL
#define CANARD_RMAPDATA_TRANSFER_ID_TIMEOUT_USEC     2500000UL

// Nodi Fissi per Moduli Master e Yakut
#define NODE_YAKUT_ID   127
#define NODE_MASTER_ID  100

// Parametri default per Modulo Slave (INIT_PARAMETER)
#define NODE_VALUE_UNSET  255 
#define NODE_SLAVE_ID     63
#define PORT_SERVICE_RMAP 53
#define SUBJECTID_PUBLISH_RMAP 100

// Maschera Check S.N. messaggio Hash Canard per PnP
#define HASH_SERNUMB_MASK   0x0000FFFFFFFFFF00u
#define HASH_EXCLUDING_BIT  16u

// (FIXED CONFIGURATION, NO READING FROM REGISTER)
#if (FIXED_CONFIGURATION)
#define USE_NODE_MASTER_ID_FIXED
#define USE_NODE_SLAVE_ID_FIXED
#define USE_PORT_SERVICE_RMAP_FIXED
#define USE_SUBJECTID_PUBLISH_RMAP_FIXED
#endif

// METADATA CONFIG AND DEFAULT VALUE
#define SENSOR_METADATA_DSA     0
#define SENSOR_METADATA_COUNT   1
// METADATA DEFAULT VALUE REGISTER INIT (P_IND, P1, P2 / TYPE_1, L1, TYPE_2, L2)
#define SENSOR_METADATA_LEVEL_1           65535
#define SENSOR_METADATA_LEVEL_2           65535
#define SENSOR_METADATA_LEVELTYPE_1       1
#define SENSOR_METADATA_LEVELTYPE_2       65535
#define SENSOR_METADATA_LEVEL_P1          0
#define SENSOR_METADATA_LEVEL_P_IND_DSA   0

// SET Default value per risposte
#define GENERIC_STATE_UNDEFINED 0x0Fu
#define GENERIC_BVAL_UNDEFINED  0xFFu

// Servizi Cypal attivi di default
#define DEFAULT_PUBLISH_PORT_LIST   true
#define DEFAULT_PUBLISH_MODULE_DATA false

// Time Publisher Servizi (secondi)
#define TIME_PUBLISH_MODULE_DATA    0.333
#define TIME_PUBLISH_PNP_REQUEST    4
#define TIME_PUBLISH_HEARTBEAT      1
#define TIME_PUBLISH_PORT_LIST      20

// TimeOUT (millisecondi)
#define MASTER_OFFLINE_TIMEOUT_US 6000000
#define MASTER_MAXSYNCRO_VALID_US 1250000
#define NODE_GETFILE_TIMEOUT_US   1750000
#define NODE_GETFILE_MAX_RETRY    3

// CODICI E STATUS AGGIORNAMENTO FILE REMOTI
#define CAN_FILE_NAME_SIZE_MAX 50
