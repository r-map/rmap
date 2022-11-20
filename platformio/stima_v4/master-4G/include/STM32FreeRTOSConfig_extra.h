/**@file STM32FreeRTOSConfig_extra.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>
Paolo patruno <p.patruno@iperbole.bologna.it>

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

#ifndef _FREERTOS_CONFIG_EXTRA_H
#define _FREERTOS_CONFIG_EXTRA_H

#define configUSE_CMSIS_RTOS_V2        1
#define configCHECK_FOR_STACK_OVERFLOW 1

#define _USE_FREERTOS_LOW_POWER
#ifdef _USE_FREERTOS_LOW_POWER
    #define _EXIT_SLEEP_FOR_DEBUGGING
    #define configUSE_TICKLESS_IDLE           1
    #define configEXPECTED_IDLE_TIME_BEFORE_SLEEP   100
    #define configPRE_SLEEP_PROCESSING( x ) xTaskSleepPrivate ( &x )
    #define configPOST_SLEEP_PROCESSING( x ) xTaskWakeUpPrivate ( x )
#endif

#endif
