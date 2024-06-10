/**@file debug_config.h */

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

#ifndef _DEBUG_CONFIG_H
#define _DEBUG_CONFIG_H

#if USE_DEBUG

/// @brief Stima trace level for debug
#define STIMA_TRACE_LEVEL              TRACE_LEVEL_INFO
/// @brief Ethernet task trace level for debug
#define ETHERNET_TASK_TRACE_LEVEL      TRACE_LEVEL_OFF
/// @brief Modem task trace level for debug
#define MODEM_TASK_TRACE_LEVEL         TRACE_LEVEL_VERBOSE
/// @brief NTP task trace level for debug
#define NTP_TASK_TRACE_LEVEL           TRACE_LEVEL_INFO
/// @brief MQTT task trace level for debug
#define MQTT_TASK_TRACE_LEVEL          TRACE_LEVEL_INFO
/// @brief HTTP task trace level for debug
#define HTTP_TASK_TRACE_LEVEL          TRACE_LEVEL_INFO
/// @brief Supervisor task trace level for debug
#define SUPERVISOR_TASK_TRACE_LEVEL    TRACE_LEVEL_INFO
/// @brief CAN task trace level for debug
#define CAN_TASK_TRACE_LEVEL           TRACE_LEVEL_INFO
/// @brief SD task trace level for debug
#define SD_TASK_TRACE_LEVEL            TRACE_LEVEL_INFO
/// @brief LCD task trace level for debug
#define LCD_TASK_TRACE_LEVEL           TRACE_LEVEL_INFO
/// @brief USB Serial task trace level for debug
#define USBSERIAL_TASK_TRACE_LEVEL     TRACE_LEVEL_INFO
/// @brief WhatchDog task trace level for debug
#define WDT_TASK_TRACE_LEVEL           TRACE_LEVEL_OFF
/// @brief SIM7600 trace level for debug
#define SIM7600_TRACE_LEVEL            TRACE_LEVEL_VERBOSE

#else

#define STIMA_TRACE_LEVEL              TRACE_LEVEL_OFF
#define ETHERNET_TASK_TRACE_LEVEL      TRACE_LEVEL_OFF
#define MODEM_TASK_TRACE_LEVEL         TRACE_LEVEL_OFF
#define NTP_TASK_TRACE_LEVEL           TRACE_LEVEL_OFF
#define MQTT_TASK_TRACE_LEVEL          TRACE_LEVEL_OFF
#define HTTP_TASK_TRACE_LEVEL          TRACE_LEVEL_OFF
#define SUPERVISOR_TASK_TRACE_LEVEL    TRACE_LEVEL_OFF
#define CAN_TASK_TRACE_LEVEL           TRACE_LEVEL_OFF
#define SD_TASK_TRACE_LEVEL            TRACE_LEVEL_OFF
#define SD_TASK_TRACE_LEVEL            TRACE_LEVEL_OFF
#define LCD_TASK_TRACE_LEVEL           TRACE_LEVEL_OFF
#define USBSERIAL_TASK_TRACE_LEVEL     TRACE_LEVEL_OFF
#define WDT_TASK_TRACE_LEVEL           TRACE_LEVEL_OFF
#define SIM7600_TRACE_LEVEL            TRACE_LEVEL_OFF

#endif

#endif
