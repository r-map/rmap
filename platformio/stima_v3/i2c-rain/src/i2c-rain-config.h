/**@file i2c-rain-config.h */

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

#ifndef _I2C_RAIN_CONFIG_H
#define _I2C_RAIN_CONFIG_H

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
#define MODULE_MINOR_VERSION                          (7)

/*!
\def MODULE_CONFIGURATION_VERSION
\brief Module version of compatibile configuration. If you change it, you have to reconfigure.
*/
#define MODULE_CONFIGURATION_VERSION                  (2)

/*!
\def MODULE_TYPE
\brief Type of module. It is defined in registers.h.
*/
#define MODULE_TYPE                             (STIMA_MODULE_TYPE_RAIN)

/*********************************************************************
* TIPPING BUCKET RAIN GAUGE
*********************************************************************/
/*!
\def TIPPING_BUCKET_PIN
\brief Interrupt pin for tipping bucket rain gauge.
*/
#define TIPPING_BUCKET_PIN                      (2)


/*********************************************************************
* CONFIGURATION
*********************************************************************/

/*!
\def CONFIGURATION_DEFAULT_TIPPING_BUCKET_TIME_MS
\brief Tipping bucket time in milliseconds.
*/
#define CONFIGURATION_DEFAULT_TIPPING_BUCKET_TIME_MS                  (50)

/*!
\def CONFIGURATION_DEFAULT_RAIN_FOR_TIP
brief How much mm of rain for one tip of tipping bucket rain gauge.
*/
#define CONFIGURATION_DEFAULT_RAIN_FOR_TIP                            (1)

/*!
\def CONFIGURATION_DEFAULT_IS_ONESHOT
\brief Oneshot mode for default.
*/
#define CONFIGURATION_DEFAULT_IS_ONESHOT        (true)

/*!
\def CONFIGURATION_DEFAULT_IS_CONTINUOUS
\brief Continuous mode for default.
*/
#define CONFIGURATION_DEFAULT_IS_CONTINUOUS     (false)

/*!
\def CONFIGURATION_DEFAULT_I2C_ADDRESS
\brief Default i2c address.
*/
#define CONFIGURATION_DEFAULT_I2C_ADDRESS       (I2C_RAIN_DEFAULT_ADDRESS)

/*!
\def CONFIGURATION_RESET_PIN
\brief Input pin for reset configuration at startup.
*/
#define CONFIGURATION_RESET_PIN                 (8)

/*!
\def SDCARD_CHIP_SELECT_PIN
\brief Chip select for SDcard SPI.
*/#define SDCARD_CHIP_SELECT_PIN 7

/*!
\def SPI_SPEED
\brief Clock speed for SPI and SDcard.
*/
#define SPI_SPEED SD_SCK_MHZ(4)

/*!
\def I2C_MAX_TIME
\brief Max i2c time in seconds before i2c restart.
*/
#define I2C_MAX_TIME             (12)

/*********************************************************************
* POWER DOWN
*********************************************************************/
/*!
\def USE_POWER_DOWN
\brief Enable or disable power down.
*/
#define USE_POWER_DOWN                          (true)

/*!
\def DEBOUNCING_POWER_DOWN_TIME_MS
\brief Debounce power down ms.
*/
#define DEBOUNCING_POWER_DOWN_TIME_MS           (10)

/*!
\def USE_TIMER_1
\brief Enable or disable timer1.
*/
#define USE_TIMER_1                             (true)

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
#define WDT_TIMER                               (WDTO_4S)

/*********************************************************************
* TIMER1
*********************************************************************/
/*!
\def TIMER1_INTERRUPT_TIME_MS
\brief Value in milliseconds for generating timer1 interrupt: 100 - 8000 [ms].
*/
#define TIMER1_INTERRUPT_TIME_MS                      (4000)

/*!
\def TIMER1_TCNT1_VALUE
\brief Timer1 timer overflow with 1024 prescaler.
*/
#define TIMER1_TCNT1_VALUE                            (0xFFFFUL - (TIMER1_INTERRUPT_TIME_MS*1000UL/(1024 / (F_CPU/1000000)))+1)

#endif
