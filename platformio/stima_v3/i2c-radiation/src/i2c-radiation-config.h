/**@file i2c-radiation-config.h */

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

#ifndef _I2C_SOLAR_RADIATION_CONFIG_H
#define _I2C_SOLAR_RADIATION_CONFIG_H

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
#define MODULE_TYPE                                   (STIMA_MODULE_TYPE_SOLAR_RADIATION)

/*********************************************************************
* CONFIGURATION
*********************************************************************/

/*!
\def CONFIGURATION_DEFAULT_IS_ONESHOT
\brief Oneshot mode for default.
*/
#define CONFIGURATION_DEFAULT_ONESHOT              (false)

/*!
\def CONFIGURATION_DEFAULT_I2C_ADDRESS
\brief Default i2c address.
*/
#define CONFIGURATION_DEFAULT_I2C_ADDRESS             (I2C_SOLAR_RADIATION_DEFAULT_ADDRESS)

/*!
\def CONFIGURATION_RESET_PIN
\brief Input pin for reset configuration at startup.
*/
#define CONFIGURATION_RESET_PIN                       (8)

#define ADC_0                                         (0)
#define ADC_1                                         (1)
#define ADC_2                                         (2)

#define ADC_1_I2C_ADDRESS                             (0x48)  // GND
#define ADC_2_I2C_ADDRESS                             (0x49)  // VDD
#define ADC_3_I2C_ADDRESS                             (0x4B)  // SCL
#define ADC_4_I2C_ADDRESS                             (0x4A)  // SDA

#define CONFIGURATION_DEFAULT_ADC_VOLTAGE_OFFSET_1    (0.0)
#define CONFIGURATION_DEFAULT_ADC_VOLTAGE_OFFSET_2    (1.0)
#define CONFIGURATION_DEFAULT_ADC_VOLTAGE_MIN         (SOLAR_RADIATION_VOLTAGE_MIN)
#define CONFIGURATION_DEFAULT_ADC_VOLTAGE_MAX         (SOLAR_RADIATION_VOLTAGE_MAX)

#define ACQUISITION_COUNT_FOR_POWER_RESET             (100)

#if (USE_SENSOR_DSR)
/*!
\def SOLAR_RADIATION_POWER_PIN
\brief Output pin for power on and power off radiation sensor.
*/
#define SOLAR_RADIATION_POWER_PIN                     (5)

/*!
\def SOLAR_RADIATION_ANALOG_PIN
\brief Input pin for reading radiation direction sensor value.
*/
#define SOLAR_RADIATION_ANALOG_PIN                    (A0)

#define ADC_VOLTAGE_MAX                               (5000.0)
#define ADC_VOLTAGE_MIN                               (0.0)
#define ADC_VOLTAGE_OFFSET                            (-20.0)

#define ADC_MAX                                       (1023.0)
#define ADC_MIN                                       (0.0)

#define SOLAR_RADIATION_VOLTAGE_MAX                   (5000.0)
#define SOLAR_RADIATION_VOLTAGE_MIN                   (1000.0)

#define SOLAR_RADIATION_ERROR_VOLTAGE_MAX             (SOLAR_RADIATION_VOLTAGE_MAX + 50.0)
#define SOLAR_RADIATION_ERROR_VOLTAGE_MIN             (SOLAR_RADIATION_VOLTAGE_MIN - 50.0)

#define SOLAR_RADIATION_MAX                           (1600.0)
#define SOLAR_RADIATION_MIN                           (0.0)

#define SOLAR_RADIATION_ERROR_MAX                     (1600.0)
#define SOLAR_RADIATION_ERROR_MIN                     (8.0)

#endif

#if (USE_SENSOR_VSR)
/*!
\def SOLAR_RADIATION_POWER_PIN
\brief Output pin for power on and power off radiation sensor.
*/
#define SOLAR_RADIATION_POWER_PIN                     (5)

/*!
\def SOLAR_RADIATION_ANALOG_PIN
\brief Input pin for reading radiation direction sensor value.
*/
#define ADC_VOLTAGE_MAX                               (6144.0)
#define ADC_VOLTAGE_MIN                               (-6144.0)
#define ADC_VOLTAGE_OFFSET                            (0.0)

#define ADC_MAX                                       (32767)
#define ADC_MIN                                       (-32767)

#define SOLAR_RADIATION_VOLTAGE_MAX                   (5000.0)
#define SOLAR_RADIATION_VOLTAGE_MIN                   (0.0)

#define SOLAR_RADIATION_ERROR_VOLTAGE_MAX             (SOLAR_RADIATION_VOLTAGE_MAX + 50.0)
#define SOLAR_RADIATION_ERROR_VOLTAGE_MIN             (SOLAR_RADIATION_VOLTAGE_MIN - 10.0)

#define SOLAR_RADIATION_MAX                           (2000.0)
#define SOLAR_RADIATION_MIN                           (0.0)

#define SOLAR_RADIATION_ERROR_MAX                     (2000.0)
#define SOLAR_RADIATION_ERROR_MIN                     (1.0)


#define SOLAR_RADIATION_ADC_INDEX                     (ADC_0)
#define SOLAR_RADIATION_ADC_CHANNEL_INPUT             (1)

#endif

#define ADC_COUNT                                     (1)

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
* RADIATION SENSORS
*********************************************************************/
// observations with processing every 1-10 minutes (minutes for processing sampling)
// report every 5-60 minutes (> OBSERVATIONS_MINUTES)

/*!
\def SENSORS_SAMPLE_TIME_MS
\brief Milliseconds for sampling sensors: 100 - 60000 [ms] must be integer multiple of TIMER1_INTERRUPT_TIME_MS !!!
*/
#define SENSORS_SAMPLE_TIME_MS                        (4000)


/*!
\def OBSERVATION_SAMPLES_COUNT_MAX
\brief Sample count maximum in OBSERVATIONS_MINUTES minutes.
*/
#define OBSERVATION_SAMPLES_COUNT_MAX                 (100)

#define RMAP_REPORT_SAMPLE_VALID                      (true)

#define RMAP_REPORT_SAMPLES_COUNT                     (STATISTICAL_DATA_COUNT * OBSERVATIONS_MINUTES * OBSERVATION_SAMPLES_COUNT_MAX)

/*!
\def OBSERVATION_SAMPLE_ERROR_MAX
\brief Maximum invalid sample count for generate a valid observations.
*/
#define OBSERVATION_SAMPLE_ERROR_MAX                  ((uint16_t)(round(OBSERVATION_SAMPLES_COUNT_MAX / 2)))
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


/*********************************************************************
* SENSORS
*********************************************************************/
/*!
\def USE_SENSORS_COUNT
\brief Sensors count.
*/
#define USE_SENSORS_COUNT                             (USE_SENSOR_DSR + USE_SENSOR_VSR)

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
#define TIMER1_INTERRUPT_TIME_MS                      (SENSORS_SAMPLE_TIME_MS)

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
\brief Timer1 timer overflow with 1024 prescaler at 16 MHz.
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
\def SOLAR_RADIATION_READ_DELAY_MS
\brief Reading delay.
*/
#define SOLAR_RADIATION_READ_DELAY_MS                 (500)

/*!
\def SOLAR_RADIATION_VALUES_READ_DELAY_MS
\brief Reading delay.
*/
#define SOLAR_RADIATION_VALUES_READ_DELAY_MS          (10)

/*!
\def SOLAR_RADIATION_READ_COUNT
\brief number of read.
*/
#define SOLAR_RADIATION_READ_COUNT                    (20)

/*!
\def TRANSACTION_TIMEOUT_MS
\brief Timeout for command transaction.
*/
#define TRANSACTION_TIMEOUT_MS                       (12000)

#endif
