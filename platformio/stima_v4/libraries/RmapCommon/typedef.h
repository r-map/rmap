/**@file typedef.h */

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

#ifndef _TYPEDEF_H
#define _TYPEDEF_H

#include <stdint.h>
#include <float.h>
#include <limits>

typedef int32_t rmapdata_t;

#define RMAPDATA_MAX            (std::numeric_limits<rmapdata_t>::max())
#define RMAPDATA_MIN            (std::numeric_limits<rmapdata_t>::min())

#define ISVALID(v)              ((uint16_t) v != UINT16_MAX)

#define ISVALID_UINT32(v)       ((uint32_t) v != UINT32_MAX)
#define ISVALID_INT32(v)        ((int32_t) v != INT32_MAX)
#define ISVALID_UINT16(v)       ((uint16_t) v != UINT16_MAX)
#define ISVALID_INT16(v)        ((int16_t) v != INT16_MAX)
#define ISVALID_UINT8(v)        ((uint8_t) v != UINT8_MAX)
#define ISVALID_INT8(v)         ((int8_t) v != INT8_MAX)
#define ISVALID_FLOAT(v)        ((float) v != FLT_MAX)
#define ISVALID_RMAPDATA(v)     ((rmapdata_t) v != RMAPDATA_MAX)

typedef struct {
  char type[4];           //!< sensor type
  uint8_t i2c_address;    //!< i2c address of sensor
  bool is_redundant;      //!< if true it is used as redundant sensors in order to check one other main sensor
} sensor_configuration_t;

typedef struct {
  rmapdata_t value;       //!< sensor acquired value
  uint8_t index;          //!<
} elaborate_data_t;

typedef struct {
  bool is_init;
  uint16_t report_time_s;
  uint8_t observation_time_s;
} request_data_t;

/*!
\struct value_t
\brief Value struct for storing sample, observation and minium, average and maximum measurement.
*/
typedef struct {
  rmapdata_t sample;  //!< last sample
  rmapdata_t ist;     //!< last observation
  rmapdata_t min;     //!< average values of observations
  rmapdata_t avg;     //!< maximum values of observations
  rmapdata_t max;     //!< minium values of observations
  rmapdata_t quality; //!< quality of observations
} value_t;

#endif
