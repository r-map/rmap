/**@file typedef.h */

/*********************************************************************
<h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>

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

/*!
\def DRIVER_LENGTH
\brief Sensor driver's buffer length.
*/
#define DRIVER_LENGTH (5)

/*!
\def TYPE_LENGTH
\brief Sensor type's buffer length.
*/
#define TYPE_LENGTH (5)

typedef uint16_t rmapdata_t;

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
  rmapdata_t value;       ///< sensor acquired value
  uint8_t index;          ///< index of command
} elaborate_data_t;

typedef struct {
  bool is_init;
  uint16_t report_time_s;       //! time in seconds in order to calculate data (min, avg, max, quality)
  uint8_t observation_time_s;   //! time in seconds in order to calculate data (ist)
} request_data_t;

#endif
