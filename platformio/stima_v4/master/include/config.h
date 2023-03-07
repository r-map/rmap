/**@file config.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>

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

// Enable control Error for Debug
#define DEBUG_MODE            (true)

// HW device
#define ENABLE_SPI1           (true)
#define ENABLE_I2C1           (true)
#define ENABLE_I2C2           (true)
#define ENABLE_QSPI           (true)
#define ENABLE_CAN            (true)
#define ENABLE_LCD            (true)
#define ENABLE_MMC            (false)
#define ENABLE_SD             (true)
#define ENABLE_USBSERIAL      (true)

#if (ENABLE_MMC) && (ENABLE_SD)
    #error Configuration error, you need to define only one method for TASK SD CARD: MMC or SD
#endif

#define ENABLE_SIM7600E       (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)

// Enable (Wdt Task and Module) and relative Function (Stack, Info ecc...)
#define ENABLE_WDT            (true)
#define WDT_TIMEOUT_BASE_US   (8000000)     // WatchDog HW us
#define WDT_STARTING_TASK_MS  (60000)       // Init WDT Task Local ms
#define WDT_CONTROLLER_MS     (2000)        // Task ms minimal check
#define ENABLE_STACK_USAGE    (true)
#define UNUSED_SUB_POSITION   (0)           // Monitor Sub Position Not Used Flag
#define NORMAL_STATE          (0)           // Monitor No Sleep / No Suspend
#define SLEEP_STATE           (1)           // Sleep Task For Wdt or LowPower Check
#define SUSPEND_STATE         (2)           // Suspend Task from WDT

// Generic Semaphore Time acquire RTC
#define ENABLE_RTC            (true)
#define RTC_WAIT_DELAY_MS     (100)
#define RPC_WAIT_DELAY_MS     (100)
#define TASK_WAIT_REALTIME_DELAY_MS    (1)  // Max Speed TASK Non Blocking Operation

// HW Diag PIN redefine
#define ENABLE_DIAG_PIN       (true)

// Address EEProm for reserved bootloader flag param (and future used 1Kb)
#define START_EEPROM_ADDRESS           (0)
#define SIZE_EEPROM_RESERVED           (1024)
#define BOOT_LOADER_STRUCT_ADDR        (START_EEPROM_ADDRESS)
#define BOOT_LOADER_STRUCT_SIZE        (sizeof(bootloader_t))
#define BOOT_LOADER_STRUCT_END         (START_EEPROM_ADDRESS + BOOT_LOADER_STRUCT_SIZE)
// Private configuration board direct
#define CONFIGURATION_EEPROM_ADDRESS   (20)
// Start Standard UAVCAN Register
#define REGISTER_EEPROM_ADDRESS        (START_EEPROM_ADDRESS + SIZE_EEPROM_RESERVED)

// Monitor Serial speed
#define SERIAL_DEBUG_BAUD_RATE         (115200)
#define SERIAL_USB_BAUD_RATE           (115200)

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
#define USE_HTTP  (true)

#if (ENABLE_I2C1 || ENABLE_I2C2)
#define I2C_MAX_DATA_LENGTH (32)
#define I2C_MAX_ERROR_COUNT (3)
#endif

#if (ENABLE_I2C1)
#define I2C1_BUS_CLOCK_HZ (100000L)
#endif
#if (ENABLE_I2C2)
#define I2C2_BUS_CLOCK_HZ (100000L)
#endif

#if (ENABLE_I2C1)
#define I2C1_BUS_CLOCK_HZ (100000L)
#endif
#if (ENABLE_I2C2)
#define I2C2_BUS_CLOCK_HZ (100000L)
#endif

// Queue Lenght
#define SYSTEM_MESSAGE_QUEUE_LENGTH (4)
#define REQUEST_DATA_QUEUE_LENGTH   (1)
#define RESPONSE_DATA_QUEUE_LENGTH  (1)
#define RMAP_PUT_DATA_QUEUE_LENGTH  (BOARDS_COUNT_MAX)
#define RMAP_GET_DATA_QUEUE_LENGTH  (1)
#define FILE_GET_DATA_QUEUE_LENGTH  (1)
#define FILE_PUT_DATA_QUEUE_LENGTH  (1)
#define LOG_PUT_DATA_QUEUE_LENGTH   (10)
// Queue Size block MAX
#define FILE_GET_DATA_BLOCK_SIZE    (256U)  // SET TO -> uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_
#define FILE_PUT_DATA_BLOCK_SIZE    (512U)
#define RMAP_DATA_MAX_ELEMENT_SIZE  (152U)  // MAX LEN TH = 144 Bytes
#define LOG_PUT_DATA_ELEMENT_SIZE   (128U)

// Queue timeOut on FILE SD/MMC ACCESS
#define FILE_IO_DATA_QUEUE_TIMEOUT  (2500)  // Time out before error to R/W operartion with queue File

// Task system_status and queue ID message
#define ALL_TASK_ID                 (99)      // Send message to ALL Task
#define SUPERVISOR_TASK_ID          (0)       // Send message to specific task..
#define LCD_TASK_ID                 (1)
#define CAN_TASK_ID                 (2)
#define MODEM_TASK_ID               (3)
#define NTP_TASK_ID                 (4)
#define HTTP_TASK_ID                (5)
#define MQTT_TASK_ID                (6)
#define MMC_TASK_ID                 (7)
#define SD_TASK_ID                  (7)
#define USBSERIAL_TASK_ID           (8)
#define WDT_TASK_ID                 (9)
#define TOTAL_INFO_TASK             (WDT_TASK_ID + 1) // Total Max Task for WDT Task Control

#define USE_RPC_METHOD_CONFIGURE    (true)
#define USE_RPC_METHOD_REBOOT       (true)
#define USE_RPC_METHOD_TEST         (true)
#define USE_RPC_METHOD_RECOVERY     (false)
#define USE_RPC_METHOD_PREPARE      (false)
#define USE_RPC_METHOD_GETJSON      (false)
#define USE_RPC_METHOD_PREPANDGET   (false)

/*!
\def USE_CONSTANTDATA_COUNT
\brief Constantdata count.
*/
#define USE_CONSTANTDATA_COUNT   (3)

#define CONFIGURATION_DEFAULT_REPORT_S       (900)
#define CONFIGURATION_DEFAULT_OBSERVATION_S  (60)
#define CONFIGURATION_DEFAULT_DISPLAY_IST_S  (10)

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

#define SEED_LENGTH (32)

#endif