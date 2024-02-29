/**@file debug.h */

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

#ifndef _DEBUG_H
#define _DEBUG_H

//Dependencies
#include <stdio.h>
#include <stdint.h>
#include <Arduino.h>
#include "os_port.h"

#define OK_STRING             "OK"
#define NO_STRING             "NO"
#define YES_STRING            "YES"
#define FAIL_STRING           "FAIL"
#define ERROR_STRING          "ERROR"
#define REDUNDANT_STRING      "REDUNDANT"
#define MAIN_STRING           "MAIN"
#define ON_STRING             "ON"
#define OFF_STRING            "OFF"
#define SUSPEND_STRING        "x"
#define FLAG_STRING           "*"
#define SPACE_STRING          " "

//Trace level definitions
#define TRACE_LEVEL_OFF       0
#define TRACE_LEVEL_FATAL     1
#define TRACE_LEVEL_ERROR     2
#define TRACE_LEVEL_WARNING   3
#define TRACE_LEVEL_INFO      4
#define TRACE_LEVEL_DEBUG     5
#define TRACE_LEVEL_VERBOSE   6

//Default trace level
#ifndef TRACE_LEVEL
   #define TRACE_LEVEL TRACE_LEVEL_DEBUG
#endif

void print_debug(const char *fmt, ...);

//Trace output redirection
#ifndef TRACE_PRINTF
#define TRACE_PRINTF(...) osSuspendAllTasks(), print_debug(__VA_ARGS__), osResumeAllTasks()
#endif

#ifndef TRACE_ARRAY
#define TRACE_ARRAY(p, a, n) osSuspendAllTasks(), print_debug_array(p, a, n), osResumeAllTasks()
#endif

#ifndef TRACE_MPI
   #define TRACE_MPI(p, a) osSuspendAllTasks(), mpiDump(stdout, p, a), osResumeAllTasks()
#endif

//Debugging macros
#if (TRACE_LEVEL >= TRACE_LEVEL_FATAL)
   #define TRACE_FATAL(...) TRACE_PRINTF(__VA_ARGS__)
   #define TRACE_FATAL_ARRAY(p, a, n) TRACE_ARRAY(p, a, n)
   #define TRACE_FATAL_MPI(p, a) TRACE_MPI(p, a)
#else
   #define TRACE_FATAL(...)
   #define TRACE_FATAL_ARRAY(p, a, n)
   #define TRACE_FATAL_MPI(p, a)
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_ERROR)
   #define TRACE_ERROR(...) TRACE_PRINTF(__VA_ARGS__)
   #define TRACE_ERROR_ARRAY(p, a, n) TRACE_ARRAY(p, a, n)
   #define TRACE_ERROR_MPI(p, a) TRACE_MPI(p, a)
#else
   #define TRACE_ERROR(...)
   #define TRACE_ERROR_ARRAY(p, a, n)
   #define TRACE_ERROR_MPI(p, a)
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_WARNING)
   #define TRACE_WARNING(...) TRACE_PRINTF(__VA_ARGS__)
   #define TRACE_WARNING_ARRAY(p, a, n) TRACE_ARRAY(p, a, n)
   #define TRACE_WARNING_MPI(p, a) TRACE_MPI(p, a)
#else
   #define TRACE_WARNING(...)
   #define TRACE_WARNING_ARRAY(p, a, n)
   #define TRACE_WARNING_MPI(p, a)
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_INFO)
   #define TRACE_INFO(...) TRACE_PRINTF(__VA_ARGS__)
   #define TRACE_INFO_ARRAY(p, a, n) TRACE_ARRAY(p, a, n)
   #define TRACE_INFO_NET_BUFFER(p, b, o, n)
   #define TRACE_INFO_MPI(p, a) TRACE_MPI(p, a)
#else
   #define TRACE_INFO(...)
   #define TRACE_INFO_ARRAY(p, a, n)
   #define TRACE_INFO_NET_BUFFER(p, b, o, n)
   #define TRACE_INFO_MPI(p, a)
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
   #define TRACE_DEBUG(...) TRACE_PRINTF(__VA_ARGS__)
   #define TRACE_DEBUG_ARRAY(p, a, n) TRACE_ARRAY(p, a, n)
   #define TRACE_DEBUG_NET_BUFFER(p, b, o, n)
   #define TRACE_DEBUG_MPI(p, a) TRACE_MPI(p, a)
#else
   #define TRACE_DEBUG(...)
   #define TRACE_DEBUG_ARRAY(p, a, n)
   #define TRACE_DEBUG_NET_BUFFER(p, b, o, n)
   #define TRACE_DEBUG_MPI(p, a)
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_VERBOSE)
   #define TRACE_VERBOSE(...) TRACE_PRINTF(__VA_ARGS__)
   #define TRACE_VERBOSE_ARRAY(p, a, n) TRACE_ARRAY(p, a, n)
   #define TRACE_VERBOSE_NET_BUFFER(p, b, o, n)
   #define TRACE_VERBOSE_MPI(p, a) TRACE_MPI(p, a)
#else
   #define TRACE_VERBOSE(...)
   #define TRACE_VERBOSE_ARRAY(p, a, n)
   #define TRACE_VERBOSE_NET_BUFFER(p, b, o, n)
   #define TRACE_VERBOSE_MPI(p, a)
#endif

#define printError(error, ok_str, error_str) (error == NO_ERROR ? ok_str : error_str)

//C++ guard
#ifdef __cplusplus
    extern "C" {
#endif

//Debug related functions
void init_debug(uint32_t baudrate);

void print_debug_array(const char *prepend, const void *data, size_t length);

//Deprecated definitions
#define TRACE_LEVEL_NO_TRACE TRACE_LEVEL_OFF

// C++ guard
#ifdef __cplusplus
}
#endif

#endif
