/**@file rmap_utility.h */

/*********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>
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

#ifndef _RMAP_UTILITY_H
#define _RMAP_UTILITY_H

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "stima_module.h"
#include <float.h>

#define ISVALID(v)      ((uint16_t) v != UINT16_MAX)

#define ISVALID_UINT32(v)      ( v != UINT32_MAX)
#define ISVALID_INT32(v)      ( v != INT32_MAX)
#define ISVALID_UINT16(v)      ( v != UINT16_MAX)
#define ISVALID_INT16(v)      ( v != INT16_MAX)
#define ISVALID_UINT8(v)      ( v != UINT8_MAX)
#define ISVALID_INT8(v)      ( v != INT8_MAX)
#define ISVALID_FLOAT(v)      ( v != FLT_MAX)

/*!
\fn void getStimaNameByType(char *name, uint8_t type)
\brief Return a STIMA's name starting from a module type stored in configuration.
\param[out] *name STIMA's name.
\param[in] *type module type stored in configuration.
\return void.
*/
void getStimaNameByType(char *name, uint8_t type);

/*!
fn bool executeTimerTaskEach(uint16_t counter, uint16_t desidered, uint16_t offset)
\param[in] uint16_t counter counter ms.
\param[in] uint16_t desidered desidered ms for return a true value.
\param[in] uint16_t offset offset ms for increment counter.
\brief Return true or false if current == desidered calculated in each offset.
*/
bool executeTimerTaskEach(uint16_t counter, uint16_t desidered, uint16_t offset);

#endif
