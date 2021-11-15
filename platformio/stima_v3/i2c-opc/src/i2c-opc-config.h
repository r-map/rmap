/**@file i2c-opc-config.h */

/*********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Paolo patruno <p.patruno@iperbole.bologna.it>
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

#ifndef _I2C_OPC_CONFIG_H
#define _I2C_OPC_CONFIG_H

#include <sensors_config.h>

/*********************************************************************
* MODULE
*********************************************************************/
/*!
\def MODULE_VERSION
\brief Module version.
*/
#define MODULE_VERSION                                (3)

/*!
\def MODULE_TYPE
\brief Type of module. It is defined in registers.h.
*/
#define MODULE_TYPE                                   (STIMA_MODULE_TYPE_OPC)

/*********************************************************************
* CONFIGURATION
*********************************************************************/
/*!
\def CONFIGURATION_DEFAULT_IS_ONESHOT
\brief Oneshot mode for default.
*/
#define CONFIGURATION_DEFAULT_IS_ONESHOT              (false)

/*!
\def CONFIGURATION_DEFAULT_IS_CONTINUOUS
\brief Continuous mode for default.
*/
#define CONFIGURATION_DEFAULT_IS_CONTINUOUS           (true)

/*!
\def CONFIGURATION_DEFAULT_I2C_ADDRESS
\brief Default i2c address.
*/
#define CONFIGURATION_DEFAULT_I2C_ADDRESS             (I2C_OPC_DEFAULT_ADDRESS)

/*!
\def CONFIGURATION_RESET_PIN
\brief Input pin for reset configuration at startup.
*/
#define CONFIGURATION_RESET_PIN                       (8)

/*!
\def OPC_POWER_PIN
\brief Output pin for power on and power off opc sensor.
*/
#define OPC_POWER_PIN                                 (9) // 8 per N2

/*!
\def OPC_POWER_PIN
\brief Output pin for power on and power off opc sensor.
*/
#define OPC_SPI_POWER_PIN                             (6)

/*!
\def OPC_CHIP_SELECT
\brief Output pin for spi chip select opc sensor.
*/
#define OPC_CHIP_SELECT                               (10)

/*!
\def SDCARD_CHIP_SELECT_PIN
\brief Chip select for SDcard SPI.
*/#define SDCARD_CHIP_SELECT_PIN 7

/*!
\def SPI_SPEED
\brief Clock speed for SPI and SDcard.
*/
#define SPI_SPEED SD_SCK_MHZ(4)

/*********************************************************************
* POWER DOWN
*********************************************************************/
/*!
\def USE_POWER_DOWN
\brief Enable or disable power down.
*/
#define USE_POWER_DOWN                              (true)

/*!
\def DEBOUNCING_POWER_DOWN_TIME_MS
\brief Debounce power down ms.
*/
#define DEBOUNCING_POWER_DOWN_TIME_MS               (10)

/*!
\def USE_TIMER_1
\brief Enable or disable timer1.
*/
#define USE_TIMER_1                                 (true)

/*!
\def USE_OPC_POWER_DOWN
\brief Enable or disable power down for opc sensor.
*/
#define USE_OPC_POWER_DOWN                          (false)

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
* OPC SENSORS
*********************************************************************/
// observations with processing every 1-10 minutes (minutes for processing sampling)
// report every 5-60 minutes (> OBSERVATIONS_MINUTES)

/*!
\def SENSORS_SAMPLE_TIME_MS
\brief Milliseconds for sampling sensors: 100 - 60000 [ms] must be integer multiple of TIMER1_INTERRUPT_TIME_MS !!!
*/
#define SENSORS_SAMPLE_TIME_MS                        (10000)

/*!
\def SENSORS_SAMPLE_COUNT_MIN
\brief Sample count minimum in OBSERVATIONS_MINUTES minutes.
*/
#define SENSORS_SAMPLE_COUNT_MIN                      ((uint8_t)(OBSERVATIONS_MINUTES * 60 / ((uint8_t)(SENSORS_SAMPLE_TIME_MS / 1000))))

#if ((OBSERVATIONS_MINUTES * 60) % (SENSORS_SAMPLE_TIME_MS / 1000) == 0)
/*!
\def SENSORS_SAMPLE_COUNT_MAX
\brief Sample count maximum in OBSERVATIONS_MINUTES minutes.
*/
#define SENSORS_SAMPLE_COUNT_MAX                      (SENSORS_SAMPLE_COUNT_MIN)
#else
/*!
\def SENSORS_SAMPLE_COUNT_MAX
\brief Sample count maximum in OBSERVATIONS_MINUTES minutes.
*/
#define SENSORS_SAMPLE_COUNT_MAX                      (SENSORS_SAMPLE_COUNT_MIN + 1)
#endif

/*!
\def SENSORS_SAMPLE_COUNT_TOLERANCE
\brief Maximum invalid sample count for generate a valid observations.
*/
#define SENSORS_SAMPLE_COUNT_TOLERANCE                (4)

/*!
\def USE_SENSORS_COUNT
\brief Sensors count.
*/
#define USE_SENSORS_COUNT                               (USE_SENSOR_OA2 + USE_SENSOR_OB2 + USE_SENSOR_OC2 + USE_SENSOR_OD2 + USE_SENSOR_OA3 + USE_SENSOR_OB3 + USE_SENSOR_OC3 + USE_SENSOR_OD3 + USE_SENSOR_OE3)

#if (USE_SENSORS_COUNT == 0)
#error No sensor used. Are you sure? If not, enable it in RmapConfig/sensors_config.h
#endif

/*********************************************************************
* TIMER1
*********************************************************************/
/*!
\def TIMER1_INTERRUPT_TIME_MS
\brief Value in milliseconds for generating timer1 interrupt: 100 - 8000 [ms].
*/
#define TIMER1_INTERRUPT_TIME_MS                      (2000)

/*!
\def TIMER1_OVERFLOW_TIME_MS
\brief Timer1 timer overflow with 1024 prescaler at 8 MHz.
\brief Timer1 timer overflow with 1024 prescaler at 16 MHz.
*/
#define TIMER1_OVERFLOW_TIME_MS                       (4194)

/*!
\def TIMER1_TCNT1_VALUE
\brief Timer1 timer overflow with 1024 prescaler at 8 MHz.
*/
#define TIMER1_TCNT1_VALUE                            ((uint16_t)(0xFFFF - (float)(1.0 * 0xFFFF * TIMER1_INTERRUPT_TIME_MS / TIMER1_OVERFLOW_TIME_MS)))

/*!
\def TIMER1_VALUE_MAX_MS
\brief Maximum timer1 counter value for timed tasks.
*/
#define TIMER1_VALUE_MAX_MS                           (60000)

/*********************************************************************
* TASKS
*********************************************************************/
/*!
\def OPC_RETRY_COUNT_MAX
\brief Maximum number of retry for opc reading.
*/
#define OPC_RETRY_COUNT_MAX                           (2)

#if (USE_SENSOR_OA2 || USE_SENSOR_OB2 || USE_SENSOR_OC2 || USE_SENSOR_OD2)
/*!
\def OPC_BINS_LENGTH
\brief OPC bin's buffer length.
*/
#define OPC_BINS_LENGTH                               (OPCN2_BINS_LENGTH)
#endif

#if (USE_SENSOR_OA3 || USE_SENSOR_OB3 || USE_SENSOR_OC3 || USE_SENSOR_OD3 || USE_SENSOR_OE3)
/*!
\def OPC_BINS_LENGTH
\brief OPC bin's buffer length.
*/
#define OPC_BINS_LENGTH                               (OPCN3_BINS_LENGTH)
#endif

#endif
