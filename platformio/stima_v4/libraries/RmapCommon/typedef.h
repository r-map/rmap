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

#define ISVALID(v)              ((uint16_t) v != UINT16_MAX)

#define ISVALID_UINT32(v)       ((uint32_t) v != UINT32_MAX)
#define ISVALID_INT32(v)        ((int32_t) v != INT32_MAX)
#define ISVALID_UINT16(v)       ((uint16_t) v != UINT16_MAX)
#define ISVALID_INT16(v)        ((int16_t) v != INT16_MAX)
#define ISVALID_UINT8(v)        ((uint8_t) v != UINT8_MAX)
#define ISVALID_INT8(v)         ((int8_t) v != INT8_MAX)
#define ISVALID_FLOAT(v)        ((float) v != FLT_MAX)

typedef struct {
  char type[4];           //!< sensor type
  uint8_t i2c_address;    //!< i2c address of sensor
  bool is_redundant;      //!< if true it is used as redundant sensors in order to check one other main sensor
} sensor_configuration_t;

#endif
