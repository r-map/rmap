/**@file i2c-th-config.h */

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

#ifndef _I2C_TH_CONFIG_H
#define _I2C_TH_CONFIG_H

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
#define MODULE_TYPE                                   (STIMA_MODULE_TYPE_TH)

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
#define CONFIGURATION_DEFAULT_I2C_ADDRESS             (I2C_TH_DEFAULT_ADDRESS)

/*!
\def CONFIGURATION_DEFAULT_TEMPERATURE_ADDRESS
\brief Default i2c temperature address.
*/
#define CONFIGURATION_DEFAULT_TEMPERATURE_ADDRESS     (I2C_TH_TEMPERATURE_DEFAULT_ADDRESS)

/*!
\def CONFIGURATION_DEFAULT_HUMIDITY_ADDRESS
\brief Default i2c humidity address.
*/
#define CONFIGURATION_DEFAULT_HUMIDITY_ADDRESS        (I2C_TH_HUMIDITY_DEFAULT_ADDRESS)

/*!
\def CONFIGURATION_RESET_PIN
\brief Input pin for reset configuration at startup.
*/
#define CONFIGURATION_RESET_PIN                       (8)

/*********************************************************************
* POWER DOWN
*********************************************************************/
/*!
\def USE_POWER_DOWN
\brief Enable or disable power down.
*/
#define USE_POWER_DOWN                                (true)

/*!
\def DEBOUNCING_POWER_DOWN_TIME_MS
\brief Debounce power down ms.
*/
#define DEBOUNCING_POWER_DOWN_TIME_MS                 (10)

/*!
\def USE_TIMER_1
\brief Enable or disable timer1.
*/
#define USE_TIMER_1                                   (true)

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
* HUMIDITY AND TEMPERATURE SENSORS
*********************************************************************/
// observations with processing every 1-10 minutes (minutes for processing sampling)
// report every 5-60 minutes (> OBSERVATIONS_MINUTES)

/*!
\def SENSORS_SAMPLE_TIME_MS
\brief Milliseconds for sampling sensors: 2000 - 8000 [ms]
*/
#define SENSORS_SAMPLE_TIME_MS                        (4000)

/*!
\def SENSORS_SAMPLE_COUNT_MIN
\brief Sample count minimum in OBSERVATIONS_MINUTES minutes.
*/
#define SENSORS_SAMPLE_COUNT_MIN                      ((uint8_t)(OBSERVATIONS_MINUTES * 60 / ((uint8_t)(SENSORS_SAMPLE_TIME_MS / 1000))))

#if (OBSERVATIONS_MINUTES * 60 % SENSORS_SAMPLE_TIME_MS / 1000 == 0)
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
#define USE_SENSORS_COUNT                             (USE_SENSOR_ADT + USE_SENSOR_HIH + USE_SENSOR_HYT)

#if (USE_SENSORS_COUNT == 0)
#error No sensor used. Are you sure? If not, enable it in RmapConfig/sensors_config.h
#endif

/*********************************************************************
* TIMER1
*********************************************************************/
/*!
\def TIMER1_INTERRUPT_TIME_MS
\brief Value in milliseconds for generating timer1 interrupt.
*/
#define TIMER1_INTERRUPT_TIME_MS                      (4000)

/*!
\def TIMER1_OVERFLOW_TIME_MS
\brief Timer1 timer overflow with 1024 prescaler at 8 MHz.
*/
#define TIMER1_OVERFLOW_TIME_MS                       (8388)

/*!
\def TIMER1_TCNT1_VALUE
\brief Timer1 timer overflow with 1024 prescaler at 8 MHz.
*/
#define TIMER1_TCNT1_VALUE                            ((uint16_t)(0xFFFF - (float)(1.0 * 0xFFFF * TIMER1_INTERRUPT_TIME_MS / TIMER1_OVERFLOW_TIME_MS)))

/*!
\def TIMER1_VALUE_MAX_MS
\brief Maximum timer1 counter value for timed tasks.
*/
#define TIMER1_VALUE_MAX_MS                           (12000)

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

#endif
