/**
  ******************************************************************************
  * @file    canard_config.hpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Uavcan Canard Configuration file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
  * <h2><center>All rights reserved.</center></h2>
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

/// @brief Assert Locali (Enable / disable)
#define LOCAL_ASSERT    assert
// #define LOCAL_ASSERT    (void(0));

/// @brief kilo unit
#define KILO 1000L
/// @brief mega unit
#define MEGA ((int64_t)KILO * KILO)

// CODA, RIDONDANZA, TIMEDELAY TX & RX CANARD
/// @brief CAN redundancy factor
#define CAN_REDUNDANCY_FACTOR 1
/// @brief  CAN TX queue capacity factor
#define CAN_TX_QUEUE_CAPACITY 100
/// @brief CAN max iface
#define CAN_MAX_IFACE         1
/// @brief CAN Rx queue capacity factor
#define CAN_RX_QUEUE_CAPACITY 100
/// @brief Iface CAN index
#define IFACE_CAN_IDX         0
/// @brief CAN delay send in microseconds 
#define CAN_DELAY_US_SEND     0
/// @brief Max subscriptions to CAN
#define MAX_SUBSCRIPTION      30
/// @brief Heap arena size
#define HEAP_ARENA_SIZE       (1024 * 16)

/// @brief CAN bit rate in Hertz
#define CAN_BIT_RATE 1000000ul
/// @brief CAN MTU base
#define CAN_MTU_BASE 8

/// @brief Messaggi di lunghezza TimeOut Extra <> Standard
#define CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC 3500000UL
#define CANARD_READFILE_TRANSFER_ID_TIMEOUT_USEC     2500000UL
#define CANARD_RMAPDATA_TRANSFER_ID_TIMEOUT_USEC     2500000UL

/// @brief Nodo Fisso per Modulo Yakut
#define NODE_YAKUT_ID     127
/// @brief Nodo Fisso per Modulo Master
#define NODE_MASTER_ID    100

/// @brief Parametri default per Modulo Master (INIT_PARAMETER)
#define PORT_RMAP_TH         50
#define PORT_RMAP_RAIN       51
#define PORT_RMAP_WIND       52
#define PORT_RMAP_RADIATION  53
#define PORT_RMAP_MPPT       54
#define PORT_RMAP_VWC        55
#define PORT_RMAP_LEVEL      56
#define PORT_RMAP_LEAF       57
#define PORT_RMAP_MASTER     100

/// @brief Parametri default per Modulo Slave (INIT_PARAMETER)
#define NODE_VALUE_UNSET  255
#define PORT_SERVICE_RMAP (PORT_RMAP_MASTER)
#define SUBJECTID_PUBLISH_RMAP (PORT_RMAP_MASTER)


/// @brief Maschera Check S.N. messaggio Hash Canard per PnP
#define HASH_SERNUMB_MASK   0x0000FFFFFFFFFF00u
#define HASH_EXCLUDING_BIT  16u

/// @brief Nodo fisso per Modulo Slave (FIXED, NO READING FROM REGISTER)
#define USE_NODE_MASTER_ID_FIXED

/// @brief Utilizzo del flag di configurazione fissi (check moduli CAN connection and data)
#if (FIXED_CONFIGURATION)
#define USE_MODULE_FIXED_TH
#define USE_MODULE_FIXED_RAIN
#define USE_MODULE_FIXED_WIND
#define USE_MODULE_FIXED_RADIATION
#define USE_MODULE_FIXED_LEAF
#define USE_MODULE_FIXED_VWC
#define USE_MODULE_FIXED_LEVEL
#define USE_MODULE_FIXED_POWER
#endif

/// @brief Utilizzo della modalità full power per la rete UAVCAN (non permette sleep. Da usare Per test e/o debug)
// #define FORCE_FULL_POWER

/// @brief Metodo di sottoscrizione al publisher per acceso ai dati slave remoti
/// Opzionale se non utilizzata per il popolamento di dati come ad. esempio display
/// Sempre attiva invece sui nodi slave per accesso con tool esterni di debug (Yakut)
#define USE_SUB_PUBLISH_SLAVE_DATA
// #define SUBSCRIBE_PUBLISH_SLAVE_DATA

/// @brief Numero di nodi massimo da collegare al MASTER
#define MAX_NODE_CONNECT        BOARDS_COUNT_MAX

/// @brief SET Default value per risposte
#define GENERIC_STATE_UNDEFINED 0x0Fu
#define GENERIC_BVAL_UNDEFINED  0xFFu
#define GENERIC_BVAL_UNCOERENT  0xFEu

/// @brief Servizi di default
#define DEFAULT_PUBLISH_PORT_LIST true

/// @brief Time Publisher Servizi (secondi)
#define TIME_PUBLISH_HEARTBEAT      1
#define TIME_PUBLISH_PORT_LIST      20

/// @brief TimeOUT (millisecondi) OFF_LINE Deve essere > Degli altri TimeOUT
/// Non succede nulla perchè gestito completamente con Reset TimeOut
/// Ma si riesce ad identificare il TimeOut intervenuto per la sua gestione
#define NODE_OFFLINE_TIMEOUT_US  6000000
#define NODE_COMMAND_TIMEOUT_US  1250000
#define NODE_REGISTER_TIMEOUT_US 1500000
#define NODE_GETDATA_TIMEOUT_US  2500000
#define NODE_GETFILE_TIMEOUT_US  1750000
#define NODE_REQFILE_TIMEOUT_US  4000000
#define NODE_REGISTER_MAX_RETRY  5
#define NODE_GETDATA_MAX_RETRY   5
#define NODE_GETFILE_MAX_RETRY   3

/// @brief NODE REGISTER METODO IN SERVER MODE
#define NODE_REGISTER_WRITING    (true)
#define NODE_REGISTER_READING    (false)

/// @brief CODICI E STATUS AGGIORNAMENTO FIRMWARE REMOTI
#define CAN_FILE_NAME_SIZE_MAX          50

/// @brief Elaboration Sensor to index Register Class Uavcan (for sensor)
// Rain
#define SENSOR_METADATA_TBR             0
#define SENSOR_METADATA_TPR             1
#define SENSOR_METADATA_RAIN_COUNT      2
// Th
#define SENSOR_METADATA_ITH             0
#define SENSOR_METADATA_MTH             1
#define SENSOR_METADATA_NTH             2
#define SENSOR_METADATA_XTH             3
#define SENSOR_METADATA_TH_COUNT        4
// Wind
#define SENSOR_METADATA_DWA             0
#define SENSOR_METADATA_DWB             1
#define SENSOR_METADATA_DWC             2
#define SENSOR_METADATA_DWD             3
#define SENSOR_METADATA_DWE             4
#define SENSOR_METADATA_DWF             5
#define SENSOR_METADATA_WIND_COUNT      6
// Power
#define SENSOR_METADATA_MPP             0
#define SENSOR_METADATA_POWER_COUNT     1
// Radiation
#define SENSOR_METADATA_DSA             0
#define SENSOR_METADATA_RADIATION_COUNT 1
// Leaf
#define SENSOR_METADATA_BFT             0
#define SENSOR_METADATA_LEAF_COUNT      1
// VWC
#define SENSOR_METADATA_VWC1            0
#define SENSOR_METADATA_VWC2            1
#define SENSOR_METADATA_VWC3            2
#define SENSOR_METADATA_VWC_COUNT       3
// LEVEL
#define SENSOR_METADATA_LVM             0
#define SENSOR_METADATA_LEVEL_COUNT     1