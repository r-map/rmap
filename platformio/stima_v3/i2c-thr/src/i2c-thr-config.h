/**@file i2c-thr-config.h */

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

#ifndef _I2C_THR_CONFIG_H
#define _I2C_THR_CONFIG_H

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
#define MODULE_MINOR_VERSION                          (6)

/*!
\def MODULE_CONFIGURATION_VERSION
\brief Module version of compatibile configuration. If you change it, you have to reconfigure.
*/
#define MODULE_CONFIGURATION_VERSION                  (1)

/*!
\def MODULE_TYPE
\brief Type of module. It is defined in registers.h.
*/
#if (USE_MODULE_THR)
#define MODULE_TYPE                                   (STIMA_MODULE_TYPE_THR)
#elif (USE_MODULE_TH)
#define MODULE_TYPE                                   (STIMA_MODULE_TYPE_TH)
#elif (USE_MODULE_RAIN)
#define MODULE_TYPE                                   (STIMA_MODULE_TYPE_RAIN)
#endif

/*********************************************************************
* CONFIGURATION
*********************************************************************/
/*!
\def CONFIGURATION_DEFAULT_I2C_ADDRESS
\brief Default i2c address.
*/
#if (USE_MODULE_THR)
#define CONFIGURATION_DEFAULT_I2C_ADDRESS             (I2C_THR_DEFAULT_ADDRESS)

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

#elif (USE_MODULE_TH)
#define CONFIGURATION_DEFAULT_I2C_ADDRESS             (I2C_THR_DEFAULT_ADDRESS)

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

#elif (USE_MODULE_RAIN)
#define CONFIGURATION_DEFAULT_I2C_ADDRESS             (I2C_RAIN_DEFAULT_ADDRESS)

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

#endif

/*!
\def CONFIGURATION_DEFAULT_TEMPERATURE_ADDRESS
\brief Default i2c temperature address.
*/
#define CONFIGURATION_DEFAULT_TEMPERATURE_ADDRESS     (I2C_THR_TEMPERATURE_DEFAULT_ADDRESS)

/*!
\def CONFIGURATION_DEFAULT_HUMIDITY_ADDRESS
\brief Default i2c humidity address.
*/
#define CONFIGURATION_DEFAULT_HUMIDITY_ADDRESS        (I2C_THR_HUMIDITY_DEFAULT_ADDRESS)

/*!
\def CONFIGURATION_RESET_PIN
\brief Input pin for reset configuration at startup.
*/
#define CONFIGURATION_RESET_PIN                       (8)

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
* TIMER1
*********************************************************************/
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
\def TIMER1_INTERRUPT_TIME_MS
\brief Value in milliseconds for generating timer1 interrupt: from 100 to TIMER1_OVERFLOW_TIME_MS.
*/
#define TIMER1_INTERRUPT_TIME_MS                      (4000)

#if (TIMER1_INTERRUPT_TIME_MS > TIMER1_OVERFLOW_TIME_MS)
#error "TIMER1_INTERRUPT_TIME_MS must be <= TIMER1_OVERFLOW_TIME_MS"
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

/*********************************************************************
* TIPPING BUCKET RAIN GAUGE
*********************************************************************/
#if (USE_SENSOR_TBR)
/*!
\def TIPPING_BUCKET_PIN
\brief Interrupt pin for tipping bucket rain gauge.
*/
#define TIPPING_BUCKET_PIN                            (2)


/*!
\def CONFIGURATION_DEFAULT_TIPPING_BUCKET_TIME_MS
\brief Tipping bucket time in milliseconds.
*/
#define CONFIGURATION_DEFAULT_TIPPING_BUCKET_TIME_MS  (50)

/*!
\def CONFIGURATION_DEFAULT_RAIN_FOR_TIP
brief How much mm of rain for one tip of tipping bucket rain gauge.
*/
#define CONFIGURATION_DEFAULT_RAIN_FOR_TIP            (1)

#endif

/*!
\def SDCARD_CHIP_SELECT_PIN
\brief Chip select for SDcard SPI.
*/#define SDCARD_CHIP_SELECT_PIN                      (7)

/*!
\def SPI_SPEED
\brief Clock speed for SPI and SDcard.
*/
#define SPI_SPEED                                     (SD_SCK_MHZ(4))

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
// observations with processing every 1-10 minutes (minutes for processing sampling)
// report every 5-60 minutes (> OBSERVATIONS_MINUTES)

/*!
\def SENSORS_SAMPLE_TIME_MS
\brief Milliseconds for sampling sensors: 100 - 60000 [ms] must be integer multiple of TIMER1_INTERRUPT_TIME_MS !!!
*/
#define SENSORS_SAMPLE_TIME_MS                        (4000)

/*!
\def OBSERVATION_SAMPLES_COUNT_MIN
\brief Sample count minimum in OBSERVATIONS_MINUTES minutes.
*/
#define OBSERVATION_SAMPLES_COUNT_MIN                 ((uint8_t)(OBSERVATIONS_MINUTES * 60 / ((uint8_t)(SENSORS_SAMPLE_TIME_MS / 1000))))

#if ((OBSERVATIONS_MINUTES * 60) % (SENSORS_SAMPLE_TIME_MS / 1000) == 0)
/*!
\def OBSERVATION_SAMPLES_COUNT_MAX
\brief Sample count maximum in OBSERVATIONS_MINUTES minutes.
*/
#define OBSERVATION_SAMPLES_COUNT_MAX                 (OBSERVATION_SAMPLES_COUNT_MIN)
#else
/*!
\def OBSERVATION_SAMPLES_COUNT_MAX
\brief Sample count maximum in OBSERVATIONS_MINUTES minutes.
*/
#define OBSERVATION_SAMPLES_COUNT_MAX                 (OBSERVATION_SAMPLES_COUNT_MIN + 1)
#endif

#define RMAP_REPORT_SAMPLE_VALID                      (true)

#define RMAP_REPORT_SAMPLES_COUNT                     (STATISTICAL_DATA_COUNT * OBSERVATIONS_MINUTES * OBSERVATION_SAMPLES_COUNT_MAX)

/*!
\def OBSERVATION_SAMPLE_ERROR_MAX
\brief Maximum invalid sample count for generate a valid observations.
*/
#define OBSERVATION_SAMPLE_ERROR_MAX                  ((uint16_t)(round(OBSERVATION_SAMPLES_COUNT_MAX - 2)))
#define OBSERVATION_SAMPLE_VALID_MIN                  ((uint16_t)(OBSERVATION_SAMPLES_COUNT_MAX - OBSERVATION_SAMPLE_ERROR_MAX))

#define RMAP_REPORT_SAMPLE_ERROR_MAX                  ((uint16_t)(STATISTICAL_DATA_COUNT * OBSERVATION_SAMPLE_ERROR_MAX))

#if (RMAP_REPORT_SAMPLE_VALID)
#define RMAP_REPORT_SAMPLE_VALID_MIN                  (OBSERVATION_SAMPLE_VALID_MIN)
#else
#define RMAP_REPORT_SAMPLE_VALID_MIN                  ((uint16_t)(STATISTICAL_DATA_COUNT * OBSERVATION_SAMPLE_VALID_MIN))
#endif

#define RMAP_REPORT_ERROR_MAX                         ((uint16_t)(STATISTICAL_DATA_COUNT - 1))
#define RMAP_REPORT_VALID_MIN                         ((uint16_t)(STATISTICAL_DATA_COUNT - RMAP_REPORT_ERROR_MAX))

#define SAMPLES_COUNT                                 ((60000 / SENSORS_SAMPLE_TIME_MS * STATISTICAL_DATA_COUNT) + 10)

#define RMAP_REPORT_INTERVAL_S                        (STATISTICAL_DATA_COUNT * OBSERVATIONS_MINUTES * 60.0)

/*********************************************************************
* SENSORS
*********************************************************************/
/*!
\def USE_SENSORS_COUNT
\brief Sensors count.
*/
#define USE_SENSORS_COUNT                             (USE_SENSOR_ADT + USE_SENSOR_HIH + USE_SENSOR_HYT + USE_SENSOR_TBR)

#define USE_SENSOR_DRIVER_COUNT                       (USE_SENSOR_ADT + USE_SENSOR_HIH + USE_SENSOR_HYT)

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
#define SENSORS_RETRY_COUNT_MAX                       (2)
/*!
\def SENSORS_RETRY_DELAY_MS
\brief Waiting for reading between two attempts.
*/
#define SENSORS_RETRY_DELAY_MS                        (100)

#endif
