/**@file main.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
<http://www.gnu.org/licenses/>.
**********************************************************************/

#ifndef _MAIN_H
#define _MAIN_H

#define __STDC_LIMIT_MACROS

#include "config.h"
#include "task_config.h"

#if (HARDWARE_I2C == ENABLE)
#include <Wire.h>
#endif

#include <STM32FreeRTOS.h>
#include "thread.hpp"

#include "Typedef.h"
#include "bufferValue.h"

#include "os_port.h"
#include "net_config.h"
#include "cpu_endian.h"
#include "error.h"
#include "debug.h"

#include "tasks/led_task.h"
#include "tasks/th_sensor_task.h"

/*********************************************************************
* TYPEDEF
*********************************************************************/
typedef struct {
  uint8_t module_main_version;                        //!< module main version
  uint8_t module_minor_version;                       //!< module minor version
  uint8_t module_type;                                //!< module type
  uint8_t sensors_count;                              //!< number of configured sensors
  sensor_configuration_t sensors[SENSORS_COUNT_MAX];  //!< sensors configurations
  uint32_t sensor_acquisition_delay_ms;               //!< delay between 2 sensors acquisitions
} configuration_t;

typedef struct {
  float values[SAMPLES_COUNT_MAX];        //!< samples buffer
  uint16_t count;                         //!< samples counter
  float *read_ptr;                        //!< reader pointer
  float *write_ptr;                       //!< writer pointer
} sample_t;

typedef struct {
  float rain_tips;
  float rain;

  float sample_temperature;
  float sample_humidity;

  float ist_temperature;
  float ist_humidity;

  float min_temperature;
  float min_humidity;

  float avg_temperature;
  float avg_humidity;

  float max_temperature;
  float max_humidity;

  float quality_temperature;
  float quality_humidity;
} report_t;

// typedef struct {
//   uint8_t module_type;                //!< module type
//   uint8_t module_main_version;        //!< module main version
//   uint8_t module_minor_version;       //!< module minor version
//   report_t thr;
// } readable_data_t;
//
// typedef struct {
//   #if (USE_MODULE_THR || USE_MODULE_TH)
//   uint8_t i2c_temperature_address;    //!< i2c address of temperature sensor
//   uint8_t i2c_humidity_address;       //!< i2c address of humidity sensor
//   #endif
//
//   #if (USE_MODULE_THR || USE_MODULE_RAIN)
//   uint16_t tipping_bucket_time_ms;    //!< Tipping bucket time in milliseconds
//   uint8_t rain_for_tip;               //!< How much mm of rain for one tip of tipping bucket rain gauge
//   #endif
// } writable_data_t;

/*********************************************************************
* GLOBAL VARIABLE
*********************************************************************/
configuration_t configuration;

// volatile readable_data_t readable_data_1;
// volatile readable_data_t readable_data_2;
// volatile readable_data_t *readable_data_read_ptr;
// volatile readable_data_t *readable_data_write_ptr;
// volatile readable_data_t *readable_data_temp_ptr;
// writable_data_t writable_data;
// writable_data_t *writable_data_ptr;

/*!
\fn void print_configuration(void)
\brief Print current configuration.
\return void.
*/
void print_configuration(void);

/*!
\fn void load_configuration(void)
\brief Load configuration from EEPROM.
\return void.
*/
void load_configuration(void);

/*!
\fn void save_configuration(bool is_default)
\brief Save configuration to EEPROM.
\param is_default: if true save default configuration; if false save current configuration.
\return void.
*/
void save_configuration(bool);

void init_wire(void);
void init_tasks(void);
void init_sensors(void);

#endif
