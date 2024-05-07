/**@file task_util.h */

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

#ifndef _TASK_UTIL_H
#define _TASK_UTIL_H

#undef OS_TASK_PRIORITY_IDLE
#undef OS_TASK_PRIORITY_NORMAL
#undef OS_TASK_PRIORITY_HIGH

#define OS_TASK_PRIORITY_00     (tskIDLE_PRIORITY)
#define OS_TASK_PRIORITY_01     (tskIDLE_PRIORITY + 1)
#define OS_TASK_PRIORITY_02     (tskIDLE_PRIORITY + 2)
#define OS_TASK_PRIORITY_03     (tskIDLE_PRIORITY + 3)
#define OS_TASK_PRIORITY_04     (tskIDLE_PRIORITY + 4)
#define OS_TASK_PRIORITY_05     (tskIDLE_PRIORITY + 5)
#define OS_TASK_PRIORITY_06     (tskIDLE_PRIORITY + 6)
#define OS_TASK_PRIORITY_07     (tskIDLE_PRIORITY + 7)
#define OS_TASK_PRIORITY_08     (tskIDLE_PRIORITY + 8)
#define OS_TASK_PRIORITY_09     (tskIDLE_PRIORITY + 9)
#define OS_TASK_PRIORITY_10     (tskIDLE_PRIORITY + 10)
#define OS_TASK_PRIORITY_11     (tskIDLE_PRIORITY + 11)
#define OS_TASK_PRIORITY_12     (tskIDLE_PRIORITY + 12)
#define OS_TASK_PRIORITY_13     (tskIDLE_PRIORITY + 13)

#define OS_TASK_PRIORITY_IDLE   (OS_TASK_PRIORITY_00)
#define OS_TASK_PRIORITY_NORMAL (OS_TASK_PRIORITY_02)
#define OS_TASK_PRIORITY_HIGH   (OS_TASK_PRIORITY_10)

#endif
