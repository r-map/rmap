/**@file debug_F.h */

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

#ifndef _DEBUG_F_H
#define _DEBUG_F_H

//Dependencies
#include "debug.h"

#include <STM32FreeRTOS.h>
#include "../../freertos-cpp/src/thread.hpp"
#include "../../freertos-cpp/src/semaphore.hpp"
#include "../../freertos-cpp/src/queue.hpp"

using namespace cpp_freertos;

void print_debug_F(const __FlashStringHelper *fmt, ...);
void queue_debug_F(Queue *dataLogPutQueue, const __FlashStringHelper *fmt, ...);

//Trace output redirection
#ifndef TRACE_PRINTF_F
#define TRACE_PRINTF_F(...) print_debug_F(__VA_ARGS__)
#endif

//Trace output redirection
#ifndef TRACE_LOG_F
#define TRACE_LOG_F(...) queue_debug_F(__VA_ARGS__)
#endif

//Debugging macros
#if (TRACE_LEVEL >= TRACE_LEVEL_FATAL)
   #define TRACE_FATAL_F(...) TRACE_PRINTF_F(__VA_ARGS__)
#else
   #define TRACE_FATAL_F(...)
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_ERROR)
   #define TRACE_ERROR_F(...) TRACE_PRINTF_F(__VA_ARGS__)
   #define LOG_ERROR_F(...) TRACE_LOG_F(__VA_ARGS__)
#else
   #define TRACE_ERROR_F(...)
   #define LOG_ERROR_F(...)
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_WARNING)
   #define TRACE_WARNING_F(...) TRACE_PRINTF_F(__VA_ARGS__)
   #define LOG_WARNING_F(...) TRACE_LOG_F(__VA_ARGS__)
#else
   #define TRACE_WARNING_F(...)
   #define LOG_WARNING_F(...)
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_INFO)
   #define TRACE_INFO_F(...) TRACE_PRINTF_F(__VA_ARGS__)
   #define LOG_INFO_F(...) TRACE_LOG_F(__VA_ARGS__)
#else
   #define TRACE_INFO_F(...)
   #define LOG_INFO_F(...)
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
   #define TRACE_DEBUG_F(...) TRACE_PRINTF_F(__VA_ARGS__)
   #define LOG_DEBUG_F(...) TRACE_LOG_F(__VA_ARGS__)
#else
   #define TRACE_DEBUG_F(...)
   #define LOG_DEBUG_F(...)
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_VERBOSE)
   #define TRACE_VERBOSE_F(...) TRACE_PRINTF_F(__VA_ARGS__)
   #define LOG_VERBOSE_F(...) TRACE_LOG_F(__VA_ARGS__)
#else
   #define TRACE_VERBOSE_F(...)
   #define LOG_VERBOSE_F(...)
#endif

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

// C++ guard
#ifdef __cplusplus
}
#endif

#endif
