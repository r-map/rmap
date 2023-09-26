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

#ifndef _I2C_POWER_CONFIG_H
#define _I2C_POWER_CONFIG_H

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
#define MODULE_MINOR_VERSION                          (11)

/*!
\def MODULE_CONFIGURATION_VERSION
\brief Module version of compatibile configuration. If you change it, you have to reconfigure.
*/
#define MODULE_CONFIGURATION_VERSION                  (1)

/*!
\def MODULE_TYPE
\brief Type of module. It is defined in stima_module.h.
*/
#define MODULE_TYPE                                   (STIMA_MODULE_TYPE_POWER)

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
#define CONFIGURATION_DEFAULT_I2C_ADDRESS             (I2C_POWER_DEFAULT_ADDRESS)

/*!
\def CONFIGURATION_RESET_PIN
\brief Input pin for reset configuration at startup.
*/
#define CONFIGURATION_RESET_PIN                       (8)


/*!
\def POWER_ADC_CHANNEL_INPUT_PANEL
\brief Input channel for panel
*/
#define POWER_ADC_CHANNEL_INPUT_PANEL                         (0)

/*!
\def POWER_ADC_CHANNEL_INPUT_BATTERY
\brief Input channel for battery
*/
#define POWER_ADC_CHANNEL_INPUT_BATTERY                       (2)

/*
 ADS1115 Address Selection

The ADS111x have one address pin, ADDR, that configures the I2C
address of the device. This pin can be connected to GND, VDD, SDA, or
SCL, allowing for four different addresses to be selected with one
pin, as shown in Table 4. The state of address pin ADDR is sampled
continuously. Use the GND, VDD and SCL addresses first. If SDA is used
as the device address, hold the SDA line low for at least 100 ns after
the SCL line goes low to make sure the device decodes the address
correctly during I2C communication.
*/
#define ADC_GND_I2C_ADDRESS                             (0x48)  // GND
#define ADC_VDD_I2C_ADDRESS                             (0x49)  // VDD
#define ADC_SCL_I2C_ADDRESS                             (0x4B)  // SCL
#define ADC_SDA_I2C_ADDRESS                             (0x4A)  // SDA

#define ADC_I2C_ADDRESS                               ADC_VDD_I2C_ADDRESS

// max value in mV to be applied before partitor to get max ADC input (2.048 V)
#define CONFIGURATION_DEFAULT_ADC_VOLTAGE_MAX_PANEL     (30000)
#define CONFIGURATION_DEFAULT_ADC_VOLTAGE_MAX_BATTERY   (15000)

///*!
//\def POWER_ANALOG_PIN1
//\brief Input pin 1 for reading panel value.
//*/
//#define POWER_ANALOG_PIN1                    (A0)
//
///*!
//\def POWER_ANALOG_PIN2
//\brief Input pin 2 for reading battery value.
//*/
//#define POWER_ANALOG_PIN2                    (A1)

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
* POWER SENSORS
*********************************************************************/
// observations with processing every 1-10 seconds (minutes for processing sampling)
// report every 5-60 minutes (> OBSERVATIONS_MINUTES)

/*!
\def SENSORS_SAMPLE_TIME_MS
\brief Milliseconds for sampling sensors: 100 - 60000 [ms] must be integer multiple of TIMER1_INTERRUPT_TIME_MS !!!
*/
#define SENSORS_SAMPLE_TIME_MS                        (10000)


/*!
\def RMAP_REPORT_SAMPLE_ERROR_MAX_PERC
\brief Sample maximum error in percent for one observation.
*/
#define RMAP_REPORT_SAMPLE_ERROR_MAX_PERC             (10.)



/*!
\def RMAP_REPORT_SAMPLE_MIN_TIME
\brief Sample minimun time for elaborate one observation (seconds).
*/
#define RMAP_REPORT_SAMPLE_MIN_TIME                   (60)


/*********************************************************************
* SENSORS
*********************************************************************/
/*!
\def USE_SENSORS_COUNT
\brief Sensors count.
*/
#define USE_SENSORS_COUNT                             (USE_SENSOR_PWR)

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
\def TRANSACTION_TIMEOUT_MS
\brief Timeout for command transaction.
*/
#define TRANSACTION_TIMEOUT_MS                       (12000)

#endif
