/**@file i2c-wind-config.h */

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

#ifndef _I2C_WIND_CONFIG_H
#define _I2C_WIND_CONFIG_H

#include <sensors_config.h>

/*********************************************************************
* MODULE
*********************************************************************/
/*!
\def MODULE_VERSION
\brief Module version.
*/
#define MODULE_VERSION                                  (3)

/*!
\def MODULE_TYPE
\brief Type of module. It is defined in registers.h.
*/
#define MODULE_TYPE                                     (STIMA_MODULE_TYPE_WIND)

/*********************************************************************
* CONFIGURATION
*********************************************************************/
/*!
\def CONFIGURATION_DEFAULT_IS_ONESHOT
\brief Oneshot mode for default.
*/
#define CONFIGURATION_DEFAULT_IS_ONESHOT                (false)

/*!
\def CONFIGURATION_DEFAULT_IS_CONTINUOUS
\brief Continuous mode for default.
*/
#define CONFIGURATION_DEFAULT_IS_CONTINUOUS             (true)

/*!
\def CONFIGURATION_DEFAULT_I2C_ADDRESS
\brief Default i2c address.
*/
#define CONFIGURATION_DEFAULT_I2C_ADDRESS               (I2C_WIND_DEFAULT_ADDRESS)

/*!
\def CONFIGURATION_RESET_PIN
\brief Input pin for reset configuration at startup.
*/
#define CONFIGURATION_RESET_PIN                         (8)

#if (USE_SENSOR_DES || USE_SENSOR_DED)
/*!
\def WIND_POWER_PIN
\brief Output pin for power on and power off wind sensor.
*/
#define WIND_POWER_PIN                                  (5)

/*!
\def WIND_DIRECTION_ANALOG_PIN
\brief Input pin for reading wind direction sensor value.
*/
#define WIND_DIRECTION_ANALOG_PIN                       (A0)

/*!
\def WIND_SPEED_DIGITAL_PIN
\brief Input pin for reading wind speed sensor value.
*/
#define WIND_SPEED_DIGITAL_PIN                          (3)

#define ADC_VOLTAGE_MAX                                 (5000.0)
#define ADC_VOLTAGE_MIN                                 (0.0)

#define ADC_MAX                                         (1023.0)
#define ADC_MIN                                         (0.0)

#define WIND_DIRECTION_VOLTAGE_MAX                      (4500.0)
#define WIND_DIRECTION_VOLTAGE_MIN                      (500.0)

#define WIND_DIRECTION_ERROR_VOLTAGE_MAX                (50.0)
#define WIND_DIRECTION_ERROR_VOLTAGE_MIN                (50.0)

#define WIND_DIRECTION_MAX                              (360.0)
#define WIND_DIRECTION_MIN                              (0.0)

#define WIND_SPEED_HZ_MAX                               (250.0)
#define WIND_SPEED_HZ_MIN                               (0.0)
#define WIND_SPEED_HZ_TURN                              (1.0)

#define WIND_SPEED_MAX                                  (50.0)
#define WIND_SPEED_MIN                                  (0.0)

#define CONFIGURATION_DEFAULT_ADC_VOLTAGE_OFFSET_1      (0.0)
#define CONFIGURATION_DEFAULT_ADC_VOLTAGE_OFFSET_2      (1.0)
#define CONFIGURATION_DEFAULT_ADC_VOLTAGE_MIN           (WIND_DIRECTION_VOLTAGE_MIN)
#define CONFIGURATION_DEFAULT_ADC_VOLTAGE_MAX           (WIND_DIRECTION_VOLTAGE_MAX)

#endif

#if (USE_SENSOR_GWS)
#define WIND_POWER_PIN                                  (4)

#define WIND_DIRECTION_MAX                              (360.0)
#define WIND_DIRECTION_MIN                              (0.0)

#define WIND_SPEED_MAX                                  (60.0)
#define WIND_SPEED_MIN                                  (0.0)

#define GWS_SERIAL_BAUD                                 (9600)
#define GWS_SERIAL_TIMEOUT_MS                           (8)
#define GWS_ACQUISITION_COUNT_FOR_POWER_RESET           (100)

#define GWS_STX_INDEX                                   (0)
#define GWS_ETX_INDEX                                   (19)

#define GWS_WITHOUT_DIRECTION_OFFSET                    (3)

#define GWS_DIRECTION_INDEX                             (3)
#define GWS_DIRECTION_LENGTH                            (3)
#define GWS_SPEED_INDEX                                 (7)
#define GWS_SPEED_LENGTH                                (6)
#define GWS_CRC_INDEX                                   (20)
#define GWS_CRC_LENGTH                                  (2)
#define STX_VALUE                                       (2)
#define ETX_VALUE                                       (3)
#define CR_VALUE                                        (13)
#define LF_VALUE                                        (10)

#define UART_RX_BUFFER_LENGTH                           (40)
#endif

/*********************************************************************
* POWER DOWN
*********************************************************************/
/*!
\def USE_POWER_DOWN
\brief Enable or disable power down.
*/
#define USE_POWER_DOWN                                  (true)

/*!
\def DEBOUNCING_POWER_DOWN_TIME_MS
\brief Debounce power down ms.
*/
#define DEBOUNCING_POWER_DOWN_TIME_MS                   (10)

/*!
\def USE_TIMER_1
\brief Enable or disable timer1.
*/
#define USE_TIMER_1                                     (true)

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
#define WDT_TIMER                                       (WDTO_8S)

/*********************************************************************
* WIND SENSORS
*********************************************************************/
// observations with processing every 1-10 minutes (minutes for processing sampling)
// report every 5-60 minutes (> OBSERVATIONS_MINUTES)

#if (USE_SENSOR_DES || USE_SENSOR_DED)
/*!
\def SENSORS_SAMPLE_TIME_MS
\brief Milliseconds for sampling sensors: 100 - 60000 [ms] must be integer multiple of TIMER1_INTERRUPT_TIME_MS !!!
*/
#define SENSORS_ACQ_TIME_MS                             (2000)

#define SENSORS_SAMPLE_TIME_MS                          (SENSORS_ACQ_TIME_MS * 3)
#define SENSORS_WARMUP_TIME_MS                          (SENSORS_SAMPLE_TIME_MS / 2)
#endif

#if (USE_SENSOR_GWS)
#define SENSORS_SAMPLE_TIME_MS                          (4000)
#endif

/*!
\def OBSERVATION_SAMPLES_COUNT_MIN
\brief Sample count minimum in OBSERVATIONS_MINUTES minutes.
*/
#define OBSERVATION_SAMPLES_COUNT_MIN                   ((uint8_t)(OBSERVATIONS_MINUTES * 60 / ((uint8_t)(SENSORS_SAMPLE_TIME_MS / 1000))))

#if ((OBSERVATIONS_MINUTES * 60) % (SENSORS_SAMPLE_TIME_MS / 1000) == 0)
/*!
\def OBSERVATION_SAMPLES_COUNT_MAX
\brief Sample count maximum in OBSERVATIONS_MINUTES minutes.
*/
#define OBSERVATION_SAMPLES_COUNT_MAX                   (OBSERVATION_SAMPLES_COUNT_MIN)
#else
/*!
\def OBSERVATION_SAMPLES_COUNT_MAX
\brief Sample count maximum in OBSERVATIONS_MINUTES minutes.
*/
#define OBSERVATION_SAMPLES_COUNT_MAX                   (OBSERVATION_SAMPLES_COUNT_MIN + 1)
#endif

#define RMAP_REPORT_SAMPLE_VALID                        (true)

#define RMAP_REPORT_SAMPLES_COUNT                       (STATISTICAL_DATA_COUNT * OBSERVATIONS_MINUTES * OBSERVATION_SAMPLES_COUNT_MAX)
#define WMO_REPORT_SAMPLES_COUNT                        (10 * OBSERVATION_SAMPLES_COUNT_MAX)

/*!
\def OBSERVATION_SAMPLE_ERROR_MAX
\brief Maximum invalid sample count for generate a valid observations.
*/
#define OBSERVATION_SAMPLE_ERROR_MAX                    ((uint16_t)(round(OBSERVATION_SAMPLES_COUNT_MAX / 2)))
#define OBSERVATION_SAMPLE_VALID_MIN                    ((uint16_t)(OBSERVATION_SAMPLES_COUNT_MAX - OBSERVATION_SAMPLE_ERROR_MAX))

#define RMAP_REPORT_SAMPLE_ERROR_MAX                    ((uint16_t)(STATISTICAL_DATA_COUNT * OBSERVATION_SAMPLE_ERROR_MAX))
#define WMO_REPORT_SAMPLE_ERROR_MAX                     ((uint16_t)(10 * OBSERVATION_SAMPLE_ERROR_MAX))

#if (RMAP_REPORT_SAMPLE_VALID)
#define RMAP_REPORT_SAMPLE_VALID_MIN                    (OBSERVATION_SAMPLE_VALID_MIN)
#define WMO_REPORT_SAMPLE_VALID_MIN                     (OBSERVATION_SAMPLE_VALID_MIN)
#else
#define RMAP_REPORT_SAMPLE_VALID_MIN                    ((uint16_t)(STATISTICAL_DATA_COUNT * OBSERVATION_SAMPLE_VALID_MIN))
#define WMO_REPORT_SAMPLE_VALID_MIN                     ((uint16_t)(10 * OBSERVATION_SAMPLE_VALID_MIN))
#endif

#define RMAP_REPORT_ERROR_MAX                           ((uint16_t)(STATISTICAL_DATA_COUNT - 1))
#define RMAP_REPORT_VALID_MIN                           ((uint16_t)(STATISTICAL_DATA_COUNT - RMAP_REPORT_ERROR_MAX))

#define SAMPLES_COUNT                                   ((60000 / SENSORS_SAMPLE_TIME_MS * STATISTICAL_DATA_COUNT) + 10)

#define WIND_CLASS_1_MAX                                (1.0)
#define WIND_CLASS_2_MAX                                (2.0)
#define WIND_CLASS_3_MAX                                (4.0)
#define WIND_CLASS_4_MAX                                (7.0)
#define WIND_CLASS_5_MAX                                (10.0)

#if (USE_SENSOR_DES)
#define CALM_WIND_MAX_MS                                (0.3)
#endif

#if (USE_SENSOR_GWS)
#define CALM_WIND_MAX_MS                                (0.1)
#endif

/*!
\def USE_SENSORS_COUNT
\brief Sensors count.
*/
/*********************************************************************
* SENSORS
*********************************************************************/
/*!
\def USE_SENSORS_COUNT
\brief Sensors count.
*/
#define USE_SENSORS_COUNT                               (USE_SENSOR_DED + USE_SENSOR_DES + USE_SENSOR_GWS)

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
#if (USE_SENSOR_DES || USE_SENSOR_DED)
#define TIMER1_INTERRUPT_TIME_MS                        (SENSORS_ACQ_TIME_MS / 2)
#endif

#if (USE_SENSOR_GWS)
#define TIMER1_INTERRUPT_TIME_MS                        (SENSORS_SAMPLE_TIME_MS)
#endif

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
#define TIMER1_TCNT1_VALUE                              ((uint16_t)(0xFFFF - (float)(1.0 * 0xFFFF * TIMER1_INTERRUPT_TIME_MS / TIMER1_OVERFLOW_TIME_MS)))

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
\def WIND_SETUP_DELAY_MS
\brief Reading delay.
*/
#define WIND_SETUP_DELAY_MS                              (100)

/*!
\def WIND_POWER_STARTUP_DELAY_MS
\brief power on delay.
*/
#define WIND_POWER_ON_DELAY_MS                          (5000)

/*!
\def WIND_READ_DELAY_MS
\brief Reading delay.
*/
#define WIND_READ_DELAY_MS                              (2)

/*!
\def WIND_READ_DELAY_MS
\brief Reading delay.
*/
#define WIND_RETRY_DELAY_MS                             (2)

/*!
\def WIND_READ_COUNT
\brief number of read.
*/
#define WIND_READ_COUNT                                 (10)

/*!
\def WIND_READ_COUNT
\brief number of read.
*/
#define WIND_RETRY_MAX                                  (600)

#endif
