
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

#ifndef _I2C_WIND_CONFIG_H
#define _I2C_WIND_CONFIG_H

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
#define GWS_SERIAL_TIMEOUT_MS                           (0)

#define UART_RX_BUFFER_LENGTH                           (40)
#endif

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
#define I2C_MAX_TIME             (180)

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

#if (USE_SENSOR_DES || USE_SENSOR_DED)
/*!
\def SENSORS_SAMPLE_TIME_MS<
\brief Milliseconds for sampling sensors: 100 - 60000 [ms] must be integer multiple of TIMER1_INTERRUPT_TIME_MS !!!
*/
#define SENSORS_ACQ_TIME_MS                             (2000)

#define SENSORS_SAMPLE_TIME_MS                          (SENSORS_ACQ_TIME_MS * 3)
#define SENSORS_WARMUP_TIME_MS                          (SENSORS_SAMPLE_TIME_MS / 2)
#endif

#if (USE_SENSOR_GWS)
  // for 644pa
  //#define SENSORS_SAMPLE_TIME_MS                          (2000)
  // for 1284p
  #define SENSORS_SAMPLE_TIME_MS                          (1000)
#endif

/*!
\def WMO_REPORT_SAMPLES_TIME
\brief Sample time for generate WMO standard wind (verctorial mean) (minutes).
*/
#define WMO_REPORT_SAMPLES_TIME                         (10)

/*!
\def WMO_REPORT_SAMPLES_COUNT
\brief Sample count for generate WMO standard wind (verctorial mean).
*/
#define WMO_REPORT_SAMPLES_COUNT                        (size_t)((WMO_REPORT_SAMPLES_TIME*1000Lu*60Lu)/SENSORS_SAMPLE_TIME_MS)

/*!
\def RMAP_REPORT_SAMPLE_ERROR_MAX_PERC
\brief Sample maximum error in percent for one observation.
*/
#define RMAP_REPORT_SAMPLE_ERROR_MAX_PERC               (10.)

/*!
\def LONG_GUST_SAMPLES_TIME
\brief Sample time for elaborate long gust (seconds).
*/
#define LONG_GUST_SAMPLES_TIME                         (60)


/*!
\def LONG_GUST_SAMPLES_COUNT
\brief Sample count for generate long gust.
*/
#define LONG_GUST_SAMPLES_COUNT                        (size_t)((LONG_GUST_SAMPLES_TIME*1000Lu)/SENSORS_SAMPLE_TIME_MS)


/*!
\def GWS_ERROR_COUNT_MAX
\brief Maximum error readeng GWS sensor before sensor reset and configuration.
*/
#define GWS_ERROR_COUNT_MAX                             (10)

// wind class definition
#define WIND_CLASS_1_MAX                                (1.0)
#define WIND_CLASS_2_MAX                                (2.0)
#define WIND_CLASS_3_MAX                                (4.0)
#define WIND_CLASS_4_MAX                                (7.0)
#define WIND_CLASS_5_MAX                                (10.0)

/*!
\def CALM_WIND_MAX_MS
\brief speed limit value for wind calm (m/s).
*/
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
\def WIND_POWER_RESPONSE_DELAY_MS
\brief windsonic poll mode delay for response (millisec).
*/
#define WIND_POWER_RESPONSE_DELAY_MS                    (150)

/*!
\def WIND_READ_DELAY_MS
\brief Reading delay.
*/
#define WIND_READ_DELAY_MS                              (2)

/*!
\def WIND_READ_COUNT
\brief number of read.
*/
#define WIND_READ_COUNT                                 (10)

/*!
\def TRANSACTION_TIMEOUT_MS
\brief Timeout for command transaction.
*/
#define TRANSACTION_TIMEOUT_MS                       (12000)


#endif
