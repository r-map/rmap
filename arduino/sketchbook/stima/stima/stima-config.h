/**@file rmap-config.h */

/*********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>
Marco Baldinetti <m.baldinetti@digiteco.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#ifndef _RMAP_CONFIG_H
#define _RMAP_CONFIG_H

#include <stima_module.h>
#include <sensors_config.h>

/*********************************************************************
* MODULE
*********************************************************************/
/*!
\def MODULE_MAIN_VERSION
\brief Module main version.
*/
#define MODULE_MAIN_VERSION                           (3)

/*!
\def MODULE_MINOR_VERSION
\brief Module minor version.
*/
#define MODULE_MINOR_VERSION                          (2)

/*!
\def MODULE_TYPE
\brief Type of module. It is defined in registers.h.
*/
#define MODULE_TYPE                                   (STIMA_MODULE_TYPE_REPORT_GSM)

#if (MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE)
/*!
\def USE_MQTT
\brief MQTT support.
*/
#define USE_MQTT                                      (false)
#else
/*!
\def USE_MQTT
\brief MQTT support.
*/
#define USE_MQTT                                      (true)
#endif

/*!
\def USE_SDCARD
\brief SD-Card support.
*/
#define USE_SDCARD                                    (false | USE_MQTT)

#if (MODULE_TYPE != STIMA_MODULE_TYPE_PASSIVE)
/*!
\def USE_NTP
\brief NTP support.
*/
#define USE_NTP                                       (true)
#endif

/*!
\def USE_RTC
\brief RTC support.
*/
#define USE_RTC                                       (true)

/*!
\def USE_TIMER_1
\brief Timer 1 support instead of RTC.
*/
#define USE_TIMER_1                                   (USE_RTC == false ? true : false)

/*!
\def USE_RPC_METHOD_CONFIGURE
\brief RPC method for station configuration.
*/
#define USE_RPC_METHOD_CONFIGURE                      (true)

/*!
\def USE_RPC_METHOD_PREPARE
\brief RPC method for prepare sensors.
*/
#define USE_RPC_METHOD_PREPARE                        (false)

/*!
\def USE_RPC_METHOD_PREPANDGET
\brief RPC method for prepare and get data from sensors.
*/
#define USE_RPC_METHOD_PREPANDGET                     (false)

/*!
\def USE_RPC_METHOD_GETJSON
\brief RPC method for get sensor's data.
*/
#define USE_RPC_METHOD_GETJSON                        (false)

/*!
\def USE_RPC_METHOD_REBOOT
\brief RPC method for reboot station.
*/
#define USE_RPC_METHOD_REBOOT                         (false)

/*********************************************************************
* CONFIGURATION
*********************************************************************/
/*!
\def CONFIGURATION_DEFAULT_TH_ADDRESS
\brief Default i2c i2c-th address.
*/
#define CONFIGURATION_DEFAULT_TH_ADDRESS              (I2C_TH_DEFAULT_ADDRESS)

/*!
\def CONFIGURATION_DEFAULT_RAIN_ADDRESS
\brief Default i2c i2c-rain address.
*/
#define CONFIGURATION_DEFAULT_RAIN_ADDRESS            (I2C_RAIN_DEFAULT_ADDRESS)

/*!
\def CONFIGURATION_RESET_PIN
\brief Input pin for reset configuration at startup.
*/
#define CONFIGURATION_RESET_PIN                       (8)

/*!
\def CONFIGURATION_DEFAULT_NTP_SERVER
\brief Default ntp server.
*/
#define CONFIGURATION_DEFAULT_NTP_SERVER              (NTP_DEFAULT_SERVER)

#if (USE_MQTT)
/*!
\def CONFIGURATION_DEFAULT_MQTT_PORT
\brief Default mqtt server port.
*/
#define CONFIGURATION_DEFAULT_MQTT_PORT               (MQTT_DEFAULT_PORT)

/*!
\def CONFIGURATION_DEFAULT_MQTT_SERVER
\brief Default mqtt server.
*/
#define CONFIGURATION_DEFAULT_MQTT_SERVER             (MQTT_DEFAULT_SERVER)

/*!
\def CONFIGURATION_DEFAULT_MQTT_ROOT_TOPIC
\brief Default mqtt root topic.
*/
#define CONFIGURATION_DEFAULT_MQTT_ROOT_TOPIC         (MQTT_DEFAULT_ROOT_TOPIC)

/*!
\def CONFIGURATION_DEFAULT_MQTT_MAINT_TOPIC
\brief Default mqtt maint topic.
*/
#define CONFIGURATION_DEFAULT_MQTT_MAINT_TOPIC         (MQTT_DEFAULT_MAINT_TOPIC)

/*!
\def CONFIGURATION_DEFAULT_MQTT_SUBSCRIBE_TOPIC
\brief Default mqtt subscribe topic.
*/
#define CONFIGURATION_DEFAULT_MQTT_SUBSCRIBE_TOPIC    (MQTT_DEFAULT_SUBSCRIBE_TOPIC)

/*!
\def CONFIGURATION_DEFAULT_MQTT_USERNAME
\brief Default mqtt username.
*/
#define CONFIGURATION_DEFAULT_MQTT_USERNAME           (MQTT_DEFAULT_USERNAME)

/*!
\def CONFIGURATION_DEFAULT_MQTT_PASSWORD
\brief Default mqtt password.
*/
#define CONFIGURATION_DEFAULT_MQTT_PASSWORD           (MQTT_DEFAULT_PASSWORD)
#endif

#if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
/*!
\def CONFIGURATION_DEFAULT_ETHERNET_DHCP_ENABLE
\brief Default DHCP status.
*/
#define CONFIGURATION_DEFAULT_ETHERNET_DHCP_ENABLE    (ETHERNET_DEFAULT_DHCP_ENABLE)

/*!
\def CONFIGURATION_DEFAULT_ETHERNET_MAC
\brief Default mac address.
*/
#define CONFIGURATION_DEFAULT_ETHERNET_MAC            (ETHERNET_DEFAULT_MAC)

/*!
\def CONFIGURATION_DEFAULT_ETHERNET_IP
\brief Default ip address.
*/
#define CONFIGURATION_DEFAULT_ETHERNET_IP             (ETHERNET_DEFAULT_IP)

/*!
\def CONFIGURATION_DEFAULT_ETHERNET_NETMASK
\brief Default netmask address.
*/
#define CONFIGURATION_DEFAULT_ETHERNET_NETMASK        (ETHERNET_DEFAULT_NETMASK)

/*!
\def CONFIGURATION_DEFAULT_ETHERNET_GATEWAY
\brief Default gateway address.
*/
#define CONFIGURATION_DEFAULT_ETHERNET_GATEWAY        (ETHERNET_DEFAULT_GATEWAY)

/*!
\def CONFIGURATION_DEFAULT_ETHERNET_PRIMARY_DNS
\brief Default primary dns address.
*/
#define CONFIGURATION_DEFAULT_ETHERNET_PRIMARY_DNS    (ETHERNET_DEFAULT_PRIMARY_DNS)

#elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
/*!
\def CONFIGURATION_DEFAULT_GSM_APN
\brief Default gsm apn.
*/
#define CONFIGURATION_DEFAULT_GSM_APN                 (GSM_DEFAULT_APN)

/*!
\def CONFIGURATION_DEFAULT_GSM_USERNAME
\brief Default gsm username.
*/
#define CONFIGURATION_DEFAULT_GSM_USERNAME            (GSM_DEFAULT_USERNAME)

/*!
\def CONFIGURATION_DEFAULT_GSM_PASSWORD
\brief Default gsm password.
*/
#define CONFIGURATION_DEFAULT_GSM_PASSWORD            (GSM_DEFAULT_PASSWORD)

#endif

/*********************************************************************
* POWER DOWN
*********************************************************************/
#if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
/*!
\def USE_POWER_DOWN
\brief Enable or disable power down.
*/
#define USE_POWER_DOWN                                (false)

#elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE)
/*!
\def USE_POWER_DOWN
\brief Enable or disable power down.
*/
#define USE_POWER_DOWN                                (true)

#endif

/*!
\def DEBOUNCING_POWER_DOWN_TIME_MS
\brief Debounce power down ms.
*/
#define DEBOUNCING_POWER_DOWN_TIME_MS                 (10)

#if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
/*!
\def W5500_CHIP_SELECT_PIN
\brief Chip select pin for Wiznet W5500 ethernet module.
*/
#define W5500_CHIP_SELECT_PIN                         (10)

#elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
/*!
\def GSM_ON_OFF_PIN
\brief Chip select pin for SIMCom SIM800C/SIM800L gsm/gprs module.
*/
#define GSM_ON_OFF_PIN                                (SIM800_ON_OFF_PIN)

#if (USE_SIM_800L)
/*!
\def GSM_ON_OFF_PIN
\brief Reset pin for SIMCom SIM800L gsm/gprs module.
*/
#define GSM_RESET_PIN                                 (SIM800_RESET_PIN)
#endif

#endif

#if (USE_SDCARD)
/*!
\def SDCARD_CHIP_SELECT_PIN
\brief Chip select pin for SD-Card module.
*/
#define SDCARD_CHIP_SELECT_PIN                        (7)
#endif

/*********************************************************************
* WATCHDOG
*********************************************************************/
/*!
\def WDT_TIMER
\brief Watchdog timer for periodically check microprocessor block states.

Possible value for WDT_TIMER are:
WDTO_15MS, WDTO_30MS, WDTO_60MS, WDTO_120MS, WDTO_250MS, WDTO_500MS,
WDTO_1S, WDTO_2S, WDTO_4S, WDTO_8S
*/
#define WDT_TIMER                                     (WDTO_8S)

/*********************************************************************
* RTC
*********************************************************************/
/*!
\def RTC_FREQUENCY
\brief Real time clock frequency for generating interrupt for awaken the microprocessor and execute timed tasks.
*/
#define RTC_FREQUENCY                                 (PCF8563_CLKOUT_FREQUENCY_SECONDS)

/*!
\def RTC_INTERRUPT_PIN
\brief Interrupt pin for rtc.
*/
#define RTC_INTERRUPT_PIN                             (6)

/*********************************************************************
* TIMER1
*********************************************************************/
/*!
\def TIMER1_INTERRUPT_TIME_MS
\brief Value in milliseconds for generating timer1 interrupt.
*/
#define TIMER1_INTERRUPT_TIME_MS                      (1000)

/*!
\def TIMER1_OVERFLOW_TIME_MS
\brief Timer1 timer overflow with 1024 prescaler at 16 MHz.
*/
#define TIMER1_OVERFLOW_TIME_MS                       (4194)

/*!
\def TIMER1_TCNT1_VALUE
\brief Timer1 timer overflow with 1024 prescaler at 8 MHz.
*/
#define TIMER1_TCNT1_VALUE                            ((uint16_t)(0xFFFF - (float)(1.0 * 0xFFFF * TIMER1_INTERRUPT_TIME_MS / TIMER1_OVERFLOW_TIME_MS)))

/*********************************************************************
* SENSORS
*********************************************************************/
/*!
\def USE_SENSORS_COUNT
\brief Sensors count.
*/
#define USE_SENSORS_COUNT                             (USE_SENSOR_ITH + USE_SENSOR_MTH + USE_SENSOR_NTH + USE_SENSOR_XTH + USE_SENSOR_TBS + USE_SENSOR_TBR + USE_SENSOR_DW1 + USE_SENSOR_DEP + USE_SENSOR_ADT + USE_SENSOR_HIH + USE_SENSOR_HYT)

#if (USE_SENSORS_COUNT == 0)
#error No sensor used. Are you sure? If not, enable it in RmapConfig/sensors_config.h
#endif

/*********************************************************************
* TASKS
*********************************************************************/
/*!
\def SENSORS_RETRY_COUNT_MAX
\brief Maximum number of retry for sensors reading.
*/
#define SENSORS_RETRY_COUNT_MAX                       (3)

/*!
\def SENSORS_RETRY_DELAY_MS
\brief Waiting for reading between two attempts.
*/
#define SENSORS_RETRY_DELAY_MS                        (50)

/*!
\def DATA_PROCESSING_RETRY_COUNT_MAX
\brief Maximum number of retry for processing acquired data.
*/
#define DATA_PROCESSING_RETRY_COUNT_MAX               (2)

/*!
\def DATA_PROCESSING_RETRY_DELAY_MS
\brief Wait time between two attempts.
*/
#define DATA_PROCESSING_RETRY_DELAY_MS                (500)

/*!
\def DATA_SAVING_RETRY_COUNT_MAX
\brief Maximum number of retry for saving data on SD-Card.
*/
#define DATA_SAVING_RETRY_COUNT_MAX                   (2)

/*!
\def DATA_SAVING_DELAY_MS
\brief Wait time between two attempts.
*/
#define DATA_SAVING_DELAY_MS                          (100)

/*!
\def MQTT_RETRY_COUNT_MAX
\brief Maximum number of retry for doing mqtt operations.
*/
#define MQTT_RETRY_COUNT_MAX                          (3)

/*!
\def MQTT_DELAY_MS
\brief Wait time between two attempts.
*/
#define MQTT_DELAY_MS                                 (1000)

/*!
\def IP_STACK_TIMEOUT_MS
\brief IPStack timeout.
*/
#define IP_STACK_TIMEOUT_MS                           (MQTT_TIMEOUT_MS)

/*!
\def SUPERVISOR_CONNECTION_RETRY_COUNT_MAX
\brief Maximum number of retry for doing supervisor operations.
*/
#define SUPERVISOR_CONNECTION_RETRY_COUNT_MAX         (3)

/*!
\def SUPERVISOR_CONNECTION_TIMEOUT_MS
\brief Timeout for connecting.
*/
#define SUPERVISOR_CONNECTION_TIMEOUT_MS              (120000)

#if (USE_SDCARD)
/*!
\def SDCARD_MQTT_PTR_FILE_NAME
\brief Data file on SD-Card containing the pointer to the last mqtt transmitted data.
*/
#define SDCARD_MQTT_PTR_FILE_NAME                     ("mqtt_ptr.txt")
#endif

/*!
\def NTP_RETRY_COUNT_MAX
\brief Maximum number of retry for doing ntp operations.
*/
#define NTP_RETRY_COUNT_MAX                           (5)

/*!
\def NTP_RETRY_DELAY_MS
\brief Wait time between two attempts.
*/
#define NTP_RETRY_DELAY_MS                            (100)

/*!
\def NTP_TIME_FOR_RESYNC_S
\brief Maximum seconds for resync time over ntp.
*/
#define NTP_TIME_FOR_RESYNC_S                         (SECS_PER_WEEK)

/*!
\def LCD_TIME_FOR_REINITIALIZE_S
\brief Maximum seconds for reinitialize LCD.
*/
#define LCD_TIME_FOR_REINITIALIZE_S                   (SECS_PER_HOUR)

#if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
/*!
\def ETHERNET_RETRY_COUNT_MAX
\brief Maximum number of retry for doing ethernet operations.
*/
#define ETHERNET_RETRY_COUNT_MAX                      (5)

/*!
\def ETHERNET_RETRY_DELAY_MS
\brief Wait time between two attempts.
*/
#define ETHERNET_RETRY_DELAY_MS                       (ETHERNET_ATTEMPT_MS)

#elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
#endif

/*!
\def DATE_TIME_STRING_LENGTH
\brief Length of datetime string %04u-%02u-%02uT%02u:%02u:%02u
*/
#define DATE_TIME_STRING_LENGTH                       (25)

#endif
