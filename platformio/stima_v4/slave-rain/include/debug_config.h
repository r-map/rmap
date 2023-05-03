/**@file debug_config.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@digiteco.it>
authors:
Marco Baldinetti <marco.baldinetti@digiteco.it>

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

#ifndef _DEBUG_CONFIG_H
#define _DEBUG_CONFIG_H

#define STIMA_TRACE_LEVEL                 TRACE_LEVEL_INFO
#define SUPERVISOR_TASK_TRACE_LEVEL       TRACE_LEVEL_VERBOSE
#define CAN_TASK_TRACE_LEVEL              TRACE_LEVEL_OFF
#define ACCELEROMETER_TASK_TRACE_LEVEL    TRACE_LEVEL_INFO
#define WDT_TASK_TRACE_LEVEL              TRACE_LEVEL_OFF
#define RAIN_SENSOR_TASK_TRACE_LEVEL      TRACE_LEVEL_DEBUG
#define ELABORATE_DATA_TASK_TRACE_LEVEL   TRACE_LEVEL_VERBOSE

#endif
