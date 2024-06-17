/**
 ******************************************************************************
 * @file    config.h
 * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
 * @author  Moreno Gasperini <m.gasperini@digiteco.it>
 * @brief   Configuration header file
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

#ifndef _CONFIG_H
#define _CONFIG_H

#include "sensors_config.h"
#include "stima_config.h"
#include "constantdata_config.h"
#include "gsm_config.h"
#include "ntp_config.h"
#include "http_config.h"
#include "mqtt_config.h"

/*********************************************************************
* MODULE
*********************************************************************/

/// @brief Module main version.
#define MODULE_MAIN_VERSION   (4)

/// @brief Module minor version.
#define MODULE_MINOR_VERSION  (6)

/// @brief Module minor version.
#define CONFIGURATION_VERSION (1)

/// @brief rmap protocol version
#define RMAP_PROCOTOL_VERSION (1)

/// @brief Type of module. It is defined in registers.h.
#define MODULE_TYPE (STIMA_MODULE_TYPE_MASTER_GSM)

/// @brief Enable control Error for Debug
#define DEBUG_MODE            (false)
#define ERROR_HANDLER_CB      (false)

/*********************************************************************
* HW DEVICES
*********************************************************************/

/// @brief Enable SPI1 interface
#define ENABLE_SPI1           (true)
#if defined(HAL_SD_MODULE_DISABLED) && defined(STIMAV4_MASTER_HW_VER_01_01)
/// @brief Enable SPI2 interface
#define ENABLE_SPI2           (true)
#endif
/// @brief Enable I2C1 interface
#define ENABLE_I2C1           (true)
/// @brief Enable I2C2 interface
#define ENABLE_I2C2           (true)
/// @brief Enable QSPI interface
#define ENABLE_QSPI           (true)
/// @brief Enable CAN interface
#define ENABLE_CAN            (true)
/// @brief Enable LCD interface
#define ENABLE_LCD            (true)
/// @brief Enable SD interface
#define ENABLE_SD             (true)
/// @brief Enable USB Serial interface
#define ENABLE_USBSERIAL      (true)
/// @brief Enable SIM7600E interface
#define ENABLE_SIM7600E       (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)

// Enable (Wdt Task and Module) and relative Function (Stack, Info ecc...)
/// @brief Enable WatchDog Task and Module
#define ENABLE_WDT            (true)
/// @brief WatchDog Hardware microseconds timeout
#define WDT_TIMEOUT_BASE_US   (8000000)
/// @brief Init WatchDog Task local milliseconds
#define WDT_STARTING_TASK_MS  (60000)
/// @brief Task milliseconds minimal check
#define WDT_CONTROLLER_MS     (2000)  
/// @brief Enable stack usage
#define ENABLE_STACK_USAGE    (true)
/// @brief Monitor Sub Position not used flag
#define UNUSED_SUB_POSITION   (0)          
/// @brief Monitor No Sleep / No Suspend
#define NORMAL_STATE          (0)  
/// @brief Sleep Task For Wdt or LowPower Check         
#define SLEEP_STATE           (1)   
/// @brief Suspend Task from Wdt        
#define SUSPEND_STATE         (2)           

/*********************************************************************
* Generic Semaphore Time acquire RTC
*********************************************************************/
/// @brief Enable RTC Interface
#define ENABLE_RTC            (true)
/// @brief Delay for RTC in milliseconds
#define RTC_WAIT_DELAY_MS     (100)
/// @brief Delay for RPC in milliseconds
#define RPC_WAIT_DELAY_MS     (500)
/// @brief Max Speed TASK Non Blocking Operation
#define TASK_WAIT_REALTIME_DELAY_MS    (1)  // 

/// @brief HW Diag PIN redefine
#define ENABLE_DIAG_PIN       (true)

/*********************************************************************
* Address EEProm for reserved bootloader flag param (and future used 2000 Bytes)
*********************************************************************/
/// @brief Starting EEPROM address
#define START_EEPROM_ADDRESS           (0)
/// @brief Size EEPROM reserved address. Must be > CONFIGURATION_EEPROM_END
#define SIZE_EEPROM_RESERVED           (1980)  
/// @brief Bootloader start address                       
#define BOOT_LOADER_STRUCT_ADDR        (START_EEPROM_ADDRESS)
/// @brief Bootloader struct size
#define BOOT_LOADER_STRUCT_SIZE        (sizeof(bootloader_t))
/// @brief Bootloader struct end address  
#define BOOT_LOADER_STRUCT_END         (START_EEPROM_ADDRESS + BOOT_LOADER_STRUCT_SIZE)

/*********************************************************************
* Private configuration board direct
*********************************************************************/
/// @brief Start Address EEPROM configuration
#define CONFIGURATION_EEPROM_ADDRESS   (20)
/// @brief Lenght EEPROM configuration. Must be < SIZE_EEPROM_RESERVED
#define CONFIGURATION_EEPROM_LENGHT    (sizeof(configuration_t))     
/// @brief End address EEPROM configuration  
#define CONFIGURATION_EEPROM_END       (CONFIGURATION_EEPROM_ADDRESS + CONFIGURATION_EEPROM_LENGHT)
/// @brief Start Standard UAVCAN Register
#define REGISTER_EEPROM_ADDRESS        (START_EEPROM_ADDRESS + SIZE_EEPROM_RESERVED)

/// @brief Monitor Debug Serial speed
#define SERIAL_DEBUG_BAUD_RATE         (115200)
/// @brief Monitor USB Serial speed
#define SERIAL_USB_BAUD_RATE           (115200)

/// @brief PPP0 interface name
#define PPP0_INTERFACE_NAME      ("ppp0")
/// @brief ETH0 interface name
#define ETH0_INTERFACE_NAME      ("eth0")

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
/// @brief INTERFACE name for STIMA_MODULE_TYPE_MASTER_GSM
#define INTERFACE_0_NAME         PPP0_INTERFACE_NAME
/// @brief INTERFACE index for STIMA_MODULE_TYPE_MASTER_GSM
#define INTERFACE_0_INDEX        (0)
/// @brief timeout in milliseconds for ppp0
#define PPP0_TIMEOUT_MS          (10000)
/// @brief default baud rate for ppp0
#define PPP0_BAUD_RATE_DEFAULT   (115200)
/// @brief max baud rate for ppp0
#define PPP0_BAUD_RATE_MAX       (921600)
/// @brief primary dns for ppp0
#define PPP0_PRIMARY_DNS         ("8.8.8.8")
/// @brief secondary dns for ppp0
#define PPP0_SECONDARY_DNS       ("8.8.4.4")

#elif (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_ETH)
/// @brief interface name for STIMA_MODULE_TYPE_MASTER_ETH
#define INTERFACE_0_NAME         ETH0_INTERFACE
/// @brief interface index for STIMA_MODULE_TYPE_MASTER_ETH
#define INTERFACE_0_INDEX        (0)
#endif

/// @brief Enable MQTT interface
#define USE_MQTT  (true)
/// @brief Enable NTP interface
#define USE_NTP   (true)
/// @brief Enable HTTP interface
#define USE_HTTP  (true)

#if (ENABLE_I2C1 || ENABLE_I2C2)
/// @brief Max length of I2C data
#define I2C_MAX_DATA_LENGTH (32)
/// @brief Max error count of I2C data
#define I2C_MAX_ERROR_COUNT (3)
#endif

#if (ENABLE_I2C1)
/// @brief I2C1 bus clock frequency in Hertz
#define I2C1_BUS_CLOCK_HZ (100000L)
#endif
#if (ENABLE_I2C2)
/// @brief I2C2 bus clock frequency in Hertz
#define I2C2_BUS_CLOCK_HZ (100000L)
#endif

#if (ENABLE_I2C1)
/// @brief I2C1 bus clock frequency in Hertz
#define I2C1_BUS_CLOCK_HZ (100000L)
#endif
#if (ENABLE_I2C2)
/// @brief I2C2 bus clock frequency in Hertz
#define I2C2_BUS_CLOCK_HZ (100000L)
#endif

/*********************************************************************
* Queue Lenght
*********************************************************************/
/// @brief Max usage on rapid configuration command MAX_BOARD message
#define SYSTEM_MESSAGE_QUEUE_LENGTH (BOARDS_COUNT_MAX + 2)
/// @brief Request data quete length
#define REQUEST_DATA_QUEUE_LENGTH   (1)
/// @brief Response data quete length
#define RESPONSE_DATA_QUEUE_LENGTH  (1)
/// @brief RMAP put data queue length. Fully Using only when data if on RAM with SD Fail (3 Record Data x Complex station)
#define RMAP_PUT_DATA_QUEUE_LENGTH  (12)
/// @brief RMAP get data queue length
#define RMAP_GET_DATA_QUEUE_LENGTH  (1)
/// @brief RMAP backup data queue length
#define RMAP_BKP_DATA_QUEUE_LENGTH  (6)
/// @brief File get data queue length
#define FILE_GET_DATA_QUEUE_LENGTH  (1)
/// @brief File put data queue length
#define FILE_PUT_DATA_QUEUE_LENGTH  (1)
/// @brief Log put data queue length
#define LOG_PUT_DATA_QUEUE_LENGTH   (8)
/// @brief Display event queue length
#define DISPLAY_EVENT_QUEUE_LENGTH  (1)

/*********************************************************************
* Queue Size block MAX
*********************************************************************/
/// @brief FIle get data block size. SET TO -> uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_
#define FILE_GET_DATA_BLOCK_SIZE    (256U) 
/// @brief FIle put data block size
#define FILE_PUT_DATA_BLOCK_SIZE    (256U)
/// @brief FIle get data block size, MAX LEN WIND = 144 Bytes
#define RMAP_DATA_MAX_ELEMENT_SIZE  (152U) 
/// @brief Log put data element size 
#define LOG_PUT_DATA_ELEMENT_SIZE   (128U)

/*********************************************************************
* Backup Older Format data Type RMAP File
*********************************************************************/
/// @brief RMAP backup data length topic size. NEED TO BE >= SENSOR_TOPIC FOR MQTT
#define RMAP_BACKUP_DATA_LEN_TOPIC_SIZE     (40U) 
/// @brief RMAP backup data length message size. NEED TO BE >= MESSAGE MQTT VALUE
#define RMAP_BACKUP_DATA_LEN_MESSAGE_SIZE   (110U)
/// @brief RMAP backup data max element size 
#define RMAP_BACKUP_DATA_MAX_ELEMENT_SIZE   (RMAP_BACKUP_DATA_LEN_TOPIC_SIZE + RMAP_BACKUP_DATA_LEN_MESSAGE_SIZE)

/// @brief Min time to wait after connection error before retry
#define MIN_INIBITH_CONNECT_RETRY_S (600) 

// Queue timeOut on FILE SD ACCESS
/// @brief Time out before error to R/W operartion with queue File data
#define FILE_IO_DATA_QUEUE_TIMEOUT  (2500)
/// @brief Time out before error to R/W operartion with queue File pointer
#define FILE_IO_PTR_QUEUE_TIMEOUT   (250)

/*********************************************************************
* Task system_status and queue ID message
*********************************************************************/
/// @brief All task ID. Send message to ALL Task
#define ALL_TASK_ID                 (99)    
/// @brief Supervisor task ID
#define SUPERVISOR_TASK_ID          (0)    
/// @brief LCD task ID
#define LCD_TASK_ID                 (1)
/// @brief CAN task ID
#define CAN_TASK_ID                 (2)
/// @brief Modem task ID
#define MODEM_TASK_ID               (3)
/// @brief NTP task ID
#define NTP_TASK_ID                 (4)
/// @brief HTTP task ID
#define HTTP_TASK_ID                (5)
/// @brief MQTT task ID
#define MQTT_TASK_ID                (6)
/// @brief SD task ID
#define SD_TASK_ID                  (7)
/// @brief USB Serial task ID
#define USBSERIAL_TASK_ID           (8)
/// @brief WhatchDog task ID
#define WDT_TASK_ID                 (9)
/// @brief Total Max Task for WDT Task Control
#define TOTAL_INFO_TASK             (WDT_TASK_ID + 1) 

/// @brief Chect connection time in second for try system reboot, without remote connection
#define WDT_CHECK_MQTT_CONN_SEC     (14400)

/// @brief Enable RPC admin method
#define USE_RPC_METHOD_ADMIN        (true)
/// @brief Enable RPC configure method
#define USE_RPC_METHOD_CONFIGURE    (true)
/// @brief Enable RPC update method
#define USE_RPC_METHOD_UPDATE       (true)
/// @brief Enable RPC reboot method
#define USE_RPC_METHOD_REBOOT       (true)
/// @brief Enable RPC recovery method
#define USE_RPC_METHOD_RECOVERY     (true)
/// @brief Enable RPC prepare method
#define USE_RPC_METHOD_PREPARE      (false)
/// @brief Enable RPC get json method
#define USE_RPC_METHOD_GETJSON      (false)
/// @brief Enable RPC prepand get method
#define USE_RPC_METHOD_PREPANDGET   (false)
/// @brief Enable RPC test method
#define USE_RPC_METHOD_TEST         (false)

/// @brief Enable RPC local reboot. Disable reboot for DEBUG
#define ENABLE_RPC_LOCAL_REBOOT     (true)

/// @brief Constantdata count.
#define USE_CONSTANTDATA_COUNT   (3)

/// @brief Time in seconds for report 
#define CONFIGURATION_DEFAULT_REPORT_S       (900)
/// @brief Time in seconds for observation 
#define CONFIGURATION_DEFAULT_OBSERVATION_S  (60)
/// @brief Time in seconds for update instant 
#define CONFIGURATION_DEFAULT_UPDATE_IST_S   (5)

/// @brief Default ntp server.
#define CONFIGURATION_DEFAULT_NTP_SERVER (NTP_DEFAULT_SERVER)

/// @brief Default station slug.
#define CONFIGURATION_DEFAULT_STATIONSLUG (DEFAULT_STATIONSLUG)

/// @brief Default board slug.
#define CONFIGURATION_DEFAULT_BOARDSLUG (DEFAULT_BOARDSLUG)

/// @brief Default board slug.
#define NAME_BSLUG_BOARD_PREFIX ("stimacan")

/// @brief Default data level.
#define CONFIGURATION_DEFAULT_DATA_LEVEL (DATA_LEVEL_REPORT)

/// @brief Default ident.
#define CONFIGURATION_DEFAULT_IDENT ("")

/// @brief Default network.
#define CONFIGURATION_DEFAULT_NETWORK (NETWORK_FIXED)

/// @brief Default latitude.
#define CONFIGURATION_DEFAULT_LATITUDE (4412345)

/// @brief Default longitude.
#define CONFIGURATION_DEFAULT_LONGITUDE (1112345)

#if (USE_MQTT)
/// @brief Default mqtt server port.
#define CONFIGURATION_DEFAULT_MQTT_PORT (MQTT_DEFAULT_PORT)

/// @brief Default mqtt server.
#define CONFIGURATION_DEFAULT_MQTT_SERVER (MQTT_DEFAULT_SERVER)

/// @brief Default mqtt root topic.
#define CONFIGURATION_DEFAULT_MQTT_ROOT_TOPIC (MQTT_DEFAULT_ROOT_TOPIC)

/// @brief Default mqtt maint topic.
#define CONFIGURATION_DEFAULT_MQTT_MAINT_TOPIC (MQTT_DEFAULT_MAINT_TOPIC)

/// @brief Default mqtt rpc topic.
#define CONFIGURATION_DEFAULT_MQTT_RPC_TOPIC (MQTT_DEFAULT_RPC_TOPIC)

/// @brief Default mqtt username.
#define CONFIGURATION_DEFAULT_MQTT_USERNAME (MQTT_DEFAULT_USERNAME)

/// @brief Default mqtt password.
#define CONFIGURATION_DEFAULT_MQTT_PASSWORD (MQTT_DEFAULT_PASSWORD)

#endif

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_ETH)

/// @brief Default DHCP status.
#define CONFIGURATION_DEFAULT_ETHERNET_DHCP_ENABLE (ETHERNET_DEFAULT_DHCP_ENABLE)

/// @brief Default mac address.
#define CONFIGURATION_DEFAULT_ETHERNET_MAC (ETHERNET_DEFAULT_MAC)

/// @brief Default ip address.
#define CONFIGURATION_DEFAULT_ETHERNET_IP (ETHERNET_DEFAULT_IP)

/// @brief Default netmask address.
#define CONFIGURATION_DEFAULT_ETHERNET_NETMASK (ETHERNET_DEFAULT_NETMASK)

/// @brief Default gateway address.
#define CONFIGURATION_DEFAULT_ETHERNET_GATEWAY (ETHERNET_DEFAULT_GATEWAY)

/// @brief Default primary dns address.
#define CONFIGURATION_DEFAULT_ETHERNET_PRIMARY_DNS (ETHERNET_DEFAULT_PRIMARY_DNS)
#endif

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)

/// @brief Default gsm apn.
#define CONFIGURATION_DEFAULT_GSM_APN (GSM_DEFAULT_APN)

/// @brief Default gsm number.
#define CONFIGURATION_DEFAULT_GSM_NUMBER (GSM_DEFAULT_NUMBER)

/// @brief Default gsm username.
#define CONFIGURATION_DEFAULT_GSM_USERNAME (GSM_DEFAULT_USERNAME)

/// @brief Default gsm password.
#define CONFIGURATION_DEFAULT_GSM_PASSWORD (GSM_DEFAULT_PASSWORD)

/// @brief Monitor flags monitor operation default.
#define CONFIGURATION_GSM_DEFAULT_MONITOR_FLAGS     (0x00)

/// @brief Default order list SIM7600 COMMAND AT+CNAOP.
#define CONFIGURATION_DEFAULT_GSM_NETWORK_ORDER     ("")

/// @brief Network configuration default prefered AS DEFAULT.
#define CONFIGURATION_GSM_DEFAULT_NETWORK           (0x00)

/// @brief Default gsm password.
#define CONFIGURATION_GSM_DEFAULT_REGISTRATION      (0x02)

#endif

/// @brief Seed length
#define SEED_LENGTH (32)

#endif