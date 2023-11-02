/**@file i2c-leaf-config.h */

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

#ifndef _I2C_LEAF_CONFIG_H
#define _I2C_LEAF_CONFIG_H

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
#define MODULE_TYPE                                   (STIMA_MODULE_TYPE_LEAF)

/*********************************************************************
* CONFIGURATION
*********************************************************************/
/*!
\def CONFIGURATION_DEFAULT_IS_ONESHOT
\brief Oneshot mode for default.
*/
#define CONFIGURATION_DEFAULT_IS_ONESHOT              (true)

/*!
\def CONFIGURATION_DEFAULT_IS_CONTINUOUS
\brief Continuous mode for default.
*/
#define CONFIGURATION_DEFAULT_IS_CONTINUOUS           (false)

/*!
\def CONFIGURATION_DEFAULT_I2C_ADDRESS
\brief Default i2c address.
*/
#define CONFIGURATION_DEFAULT_I2C_ADDRESS             (I2C_LEAF_DEFAULT_ADDRESS)

/*!
\def CONFIGURATION_RESET_PIN
\brief Input pin for reset configuration at startup.
*/
#define CONFIGURATION_RESET_PIN                       (8)

/*!
\def LEAF_POWER_PIN
\brief Output pin for power on and power off leaf sensor.
*/
#define LEAF_POWER_PIN                                (2)

/*!
\def LEAF_ANALOG_PIN
\brief Input pin for reading leaf sensor value.
*/
#define LEAF_ANALOG_PIN                               (A0)


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
\def USE_LEAF_POWER_DOWN
\brief Enable or disable power down for leaf sensor.
*/
#define USE_LEAF_POWER_DOWN                         (true)

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
* LEAF SENSORS
*********************************************************************/
// observations with processing every 1-10 minutes (minutes for processing sampling)
// report every 5-60 minutes (> OBSERVATIONS_MINUTES)

/*!
\def SENSORS_SAMPLE_TIME_MS
\brief Milliseconds for sampling sensors: 1000 - 60000 [ms] must be integer multiple of TIMER1_INTERRUPT_TIME_MS !!!
*/
#define SENSORS_SAMPLE_TIME_MS                        (8000)

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
#define SENSORS_SAMPLE_COUNT_TOLERANCE                (2)

/*!
\def USE_SENSORS_COUNT
\brief Sensors count.
*/
#define USE_SENSORS_COUNT                             (1)

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
\brief Timer1 timer overflow with 1024 prescaler at 8 or 16 MHz.
*/
#if (F_CPU == 8000000L)
#define TIMER1_OVERFLOW_TIME_MS                       (8388)
#elif (F_CPU == 16000000L)
#define TIMER1_OVERFLOW_TIME_MS                       (4194)
#endif

/*!
\def TIMER1_TCNT1_VALUE
\brief Timer1 timer overflow with 1024 prescaler at 8 MHz.
*/
#define TIMER1_TCNT1_VALUE                            ((uint16_t)(0xFFFF - (float)(1.0 * 0xFFFF * TIMER1_INTERRUPT_TIME_MS / TIMER1_OVERFLOW_TIME_MS)))

/*!
\def TIMER_COUNTER_VALUE_MAX_MS
\brief Maximum timer1 counter value for timed tasks.
*/
#define TIMER_COUNTER_VALUE_MAX_MS                    (SENSORS_SAMPLE_TIME_MS)
#define TIMER_COUNTER_VALUE_MAX_S                     (60)

/*********************************************************************
* TASKS
*********************************************************************/
/*!
\def LEAF_READ_DELAY_MS
\brief Reading delay.
*/
#define LEAF_READ_DELAY_MS                            (100)

/*!
\def LEAF_VALUES_READ_DELAY_MS
\brief Reading delay.
*/
#define LEAF_VALUES_READ_DELAY_MS                     (2)

/*!
\def LEAF_CALIBRATION_READ_COUNT
\brief Maximum number of retry for leaf reading.
*/
#define LEAF_CALIBRATION_READ_COUNT                   (100)

/*!
\def LEAF_RETRY_COUNT_MAX
\brief Maximum number of retry for leaf reading.
*/
#define LEAF_READ_COUNT                               (10)

/*!
\def LEAF_RETRY_COUNT_MAX
\brief Maximum number of retry for leaf reading.
*/
#define LEAF_CALIBRATION_OFFSET                       (5)

/*!
\def TRANSACTION_TIMEOUT_MS
\brief Timeout for command transaction.
*/
#define TRANSACTION_TIMEOUT_MS                       (12000)

#endif
