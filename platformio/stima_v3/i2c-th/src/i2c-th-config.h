
/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <m.baldinetti@digiteco.it>
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
\def MODULE_MAIN_VERSION
\brief Module main version.
*/
#define MODULE_MAIN_VERSION                           (3)

/*!
\def MODULE_MINOR_VERSION
\brief Module minor version.
*/
#define MODULE_MINOR_VERSION                          (20)

/*!
\def MODULE_CONFIGURATION_VERSION
\brief Module version of compatibile configuration. If you change it, you have to reconfigure.
*/
#define MODULE_CONFIGURATION_VERSION                  (2)

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
\def CONFIGURATION_DEFAULT_I2C_ADDRESS
\brief Default i2c address.
*/
#define CONFIGURATION_DEFAULT_I2C_ADDRESS             (I2C_TH_DEFAULT_ADDRESS)

/*!
\def CONFIGURATION_RESET_PIN
\brief Input pin for reset configuration at startup.
*/
#define CONFIGURATION_RESET_PIN                       (8)

/*!
\def TH_POWER_PIN
\brief Output pin for power on and power off th sensor.
*/
#define TH_POWER_PIN                                  (5)

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
* SENSORS
*********************************************************************/

/*!
\def SENSORS_ERROR_COUNT_MAX
\brief Max number of i2c error for a sensor before didable sensor and retry setup
*/
#define SENSOR_ERROR_COUNT_MAX                        (20)

/*********************************************************************
* HUMIDITY AND TEMPERATURE SENSORS
*********************************************************************/

/*!
\def OBSERVATIONS_MINUTES
\brief How much minutes for calculate an observations by processing sampling. Tipically 1-10 minutes.
*/
#define OBSERVATIONS_MINUTES                 (1)

  // observations with processing every 1-10 minutes (minutes for processing sampling)
// report every 5-60 minutes (> OBSERVATIONS_MINUTES)

/*!
\def SENSORS_SAMPLE_TIME_MS
\brief Milliseconds for sampling sensors: 2000 - 8000 [ms]
setting it to 3980 ms we gain 20 ms every sample, 300 ms every observation, 4500 every report (15minutes)
*/
#define SENSORS_SAMPLE_TIME_MS                    (4000)


#define SAMPLES_COUNT_MAX                         (((OBSERVATIONS_MINUTES * 60000UL) / SENSORS_SAMPLE_TIME_MS) + 10)

/*!
\def OBSERVATION_SAMPLES_COUNT
\brief Sample count  in OBSERVATIONS_MINUTES minutes.
*/
#define OBSERVATION_SAMPLES_COUNT                 (((OBSERVATIONS_MINUTES * 60000UL) / SENSORS_SAMPLE_TIME_MS ))


///*!
//\def OBSERVATION_COUNT
//\brief Observations buffer length.
//*/
//#define OBSERVATION_COUNT                    (STATISTICAL_DATA_COUNT * 25)


/*!
\def OBSERVATION_SAMPLE_ERROR_MAX
\brief Maximum invalid sample count for generate a valid observations.
*/
#define OBSERVATION_SAMPLE_ERROR_MAX                  ((uint16_t)(round(OBSERVATION_SAMPLES_COUNT/10.))+1)


#define RMAP_REPORT_ERROR_MAX                         ((uint16_t)(1))
#define RMAP_REPORT_VALID_MIN                         ((uint16_t)(2))



/*********************************************************************
* TIMER1
*********************************************************************/
/*!
\def TIMER1_INTERRUPT_TIME_MS
\brief Value in milliseconds for generating timer1 interrupt.
*/
#define TIMER1_INTERRUPT_TIME_MS                      (SENSORS_SAMPLE_TIME_MS)

/*!
\def TIMER1_TCNT1_VALUE
\brief Timer1 timer overflow with 1024 prescaler.
*/
#define TIMER1_TCNT1_VALUE                            (0xFFFFUL - (TIMER1_INTERRUPT_TIME_MS*1000UL/(1024 / (F_CPU/1000000)))+1)

/*!
\def TIMER1_VALUE_MAX_MS
\brief Maximum timer1 counter value for timed tasks.
*/
#define TIMER1_VALUE_MAX_MS                           (TIMER1_INTERRUPT_TIME_MS * 3)

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
\def TRANSACTION_TIMEOUT_MS
\brief Timeout for command transaction.
*/
#define TRANSACTION_TIMEOUT_MS                       (5000)


#endif
