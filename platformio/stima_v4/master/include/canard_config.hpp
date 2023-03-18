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
#define MAX_SUBSCRIPTION      30
#define HEAP_ARENA_SIZE       (1024 * 16)

// CAN SPEED RATE HZ
#define CAN_BIT_RATE 1000000ul
#define CAN_MTU_BASE 8

// Messaggi di lunghezza TimeOut Extra <> Standard
#define CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC 3500000UL
#define CANARD_READFILE_TRANSFER_ID_TIMEOUT_USEC     2500000UL

// Nodi Fissi per Moduli Master e Yakut
#define NODE_YAKUT_ID   127
#define NODE_MASTER_ID  100
// #define PORT_SERVICE_MASTER 100
// #define SUBJECTID_PUBLISH_MASTER 2100

// Nodo fisso per Modulo Slave (FIXED, NO READING FROM REGISTER)
#define USE_NODE_MASTER_ID_FIXED

// Utilizzo della m odalità full power per la rete UAVCAN (non permette sleep. Da usare Per test e/o debug)
// #define FORCE_FULL_POWER

// Utilizza metodo di sottoscrizione al publisher per acceso ai dati slave remoti
// Opzionale se non utilizzata per il popolamento di dati come ad. esempio display
// Sempre attiva invece sui nodi slave per accesso con tool esterni di debug (Yakut)
#define USE_SUB_PUBLISH_SLAVE_DATA

// Numero di nodi massimo da collegare al MASTER
#define MAX_NODE_CONNECT        BOARDS_COUNT_MAX

// SET Default value per risposte
#define GENERIC_STATE_UNDEFINED 0x0Fu
#define GENERIC_BVAL_UNDEFINED  0xFFu
#define GENERIC_BVAL_UNCOERENT  0xFEu

// Servizi di default
#define DEFAULT_PUBLISH_PORT_LIST true

// Time Publisher Servizi (secondi)
#define TIME_PUBLISH_HEARTBEAT      1
#define TIME_PUBLISH_PORT_LIST      20

// TimeOUT (millisecondi) OFF_LINE Deve essere > Degli altri TimeOUT
// Non succede nulla perchè gestito completamente con Reset TimeOut
// Ma si riesce ad identificare il TimeOut intervenuto per la sua gestione
#define NODE_OFFLINE_TIMEOUT_US  6000000
#define NODE_COMMAND_TIMEOUT_US  1250000
#define NODE_REGISTER_TIMEOUT_US 1500000
#define NODE_GETDATA_TIMEOUT_US  1500000
#define NODE_GETFILE_TIMEOUT_US  1750000
#define NODE_REQFILE_TIMEOUT_US  4000000
#define NODE_GETFILE_MAX_RETRY   3

// NODE REGISTER METODO IN SERVER MODE
#define NODE_REGISTER_WRITING    (true)
#define NODE_REGISTER_READING    (false)

// CODICI E STATUS AGGIORNAMENTO FIRMWARE REMOTI
#define CAN_FILE_NAME_SIZE_MAX          50

// Elaboration Sensor to index Register Class Uavcan (for sensor)
// Rain
#define SENSOR_METADATA_TBR             0
#define SENSOR_METADATA_RAIN_COUNT      1
// Th
#define SENSOR_METADATA_STH             0
#define SENSOR_METADATA_ITH             1
#define SENSOR_METADATA_MTH             2
#define SENSOR_METADATA_NTH             3
#define SENSOR_METADATA_XTH             4
#define SENSOR_METADATA_TH_COUNT        5
// Wind
#define SENSOR_METADATA_DWA             0
#define SENSOR_METADATA_DWB             1
#define SENSOR_METADATA_DWC             2
#define SENSOR_METADATA_DWD             3
#define SENSOR_METADATA_DWE             4
#define SENSOR_METADATA_DWF             5
#define SENSOR_METADATA_WIND_COUNT      6
// Power
#define SENSOR_METADATA_DEP             0
#define SENSOR_METADATA_POWER_COUNT     1
// Radiation
#define SENSOR_METADATA_DSA             0
#define SENSOR_METADATA_RADIATION_COUNT 1
// VWC
#define SENSOR_METADATA_VWC             0
#define SENSOR_METADATA_VWC_COUNT       1