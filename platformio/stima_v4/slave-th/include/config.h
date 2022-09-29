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

#include "hardware_config.h"
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
#if (USE_MODULE_THR)
#define MODULE_TYPE       (STIMA_MODULE_TYPE_THR)
#elif (USE_MODULE_TH)
#define MODULE_TYPE       (STIMA_MODULE_TYPE_TH)
#elif (USE_MODULE_RAIN)
#define MODULE_TYPE       (STIMA_MODULE_TYPE_RAIN)
#endif

#define SAMPLES_COUNT_MAX                 (3600)
#define ACQUISITION_DELAY_MS              (4000)

#endif
