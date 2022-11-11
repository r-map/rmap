/**@file config.h */

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

#ifndef _CONFIG_H
#define _CONFIG_H

#include "sensors_config.h"
#include "rmap_utility.h"

/*********************************************************************
* MODULE
*********************************************************************/
/*!
\def MODULE_MAIN_VERSION
\brief Module main version.
*/
#define MODULE_MAIN_VERSION   (4)

/*!
\def MODULE_MINOR_VERSION
\brief Module minor version.
*/
#define MODULE_MINOR_VERSION  (0)

/*!
\def MODULE_TYPE
\brief Type of module. It is defined in registers.h.
*/
#define MODULE_TYPE (STIMA_MODULE_TYPE_MASTER_GSM)

#define CONFIGURATION_EEPROM_ADDRESS (0)

#define ENABLE_I2C1 (true)
#define ENABLE_I2C2 (true)
#define ENABLE_QSPI (false)

// #define SAMPLES_COUNT_MAX                 (3600)
// #define SENSORS_ACQUISITION_DELAY_MS      (4000)
// #define OBSERVATRIONS_TIME_S              (60)
// #define REPORTS_TIME_S                    (900)
//
// #define ELABORATE_DATA_QUEUE_LENGTH       (4)
// #define REQUEST_DATA_QUEUE_LENGTH         (1)
// #define REPORT_DATA_QUEUE_LENGTH          (1)
//
// #define TEMPERATURE_MAIN_INDEX            (0)
// #define HUMIDITY_MAIN_INDEX               (1)
// #define TEMPERATURE_REDUNDANT_INDEX       (2)
// #define HUMIDITY_REDUNDANT_INDEX          (3)
// #define RAIN_INDEX                        (4)
//
// #define MAX_VALID_TEMPERATURE             (100.0)
// #define MIN_VALID_TEMPERATURE             (-50.0)
// #define MAX_VALID_HUMIDITY                (100.0)
// #define MIN_VALID_HUMIDITY                (0.0)
//
// #define SAMPLE_ERROR_PERCENTAGE_MAX       (50.0)
// #define OBSERVATION_ERROR_PERCENTAGE_MAX  (50.0)

#endif
