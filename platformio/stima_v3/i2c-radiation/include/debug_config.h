/**@file debug_config.h */

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

#ifndef _DEBUG_CONFIG_H
#define _DEBUG_CONFIG_H


/*!
\def SDCARD_LOGGING_FILE_NAME
\brief File name for logging on SD-Card.
*/
#define SDCARD_LOGGING_FILE_NAME                     ("radiation.log")

/*!
\def DISABLE_LOGGING
\brief fully disable logging
define the macro to fully disable logging, and reduce project size
*/
//#define DISABLE_LOGGING

/*!
\def ENABLE_SDCARD_LOGGING
\brief Enable logging on SDcard
define to 1 the macro to enable logging on a file on SDcard.
*/
#define ENABLE_SDCARD_LOGGING              (0)

/*!
\def LOG_LEVEL
\brief logging level at compile time
Available levels are:
LOG_LEVEL_SILENT
LOG_LEVEL_FATAL
LOG_LEVEL_ERROR
LOG_LEVEL_WARNING
LOG_LEVEL_NOTICE
LOG_LEVEL_TRACE
LOG_LEVEL_VERBOSE
*/
#define LOG_LEVEL   LOG_LEVEL_NOTICE


/*!
\def OK_STRING
\brief "OK" string message.
*/
#define OK_STRING                         ("OK")

/*!
\def ERROR_STRING
\brief "ERROR" string message.
*/
#define ERROR_STRING                      ("ERROR")

/*!
\def FAIL_STRING
\brief "FAIL" string message.
*/
#define FAIL_STRING                       ("FAIL")

/*!
\def YES_STRING
\brief "YES" string message.
*/
#define YES_STRING                        ("YES")

/*!
\def NO_STRING
\brief "NO" string message.
*/
#define NO_STRING                         ("NO")

/*!
\def ON_STRING
\brief "ON" string message.
*/
#define ON_STRING                         ("ON")

/*!
\def OFF_STRING
\brief "OFF" string message.
*/
#define OFF_STRING                        ("OFF")

/*!
\def SAVE_STRING
\brief "SAVE" string message.
*/
#define SAVE_STRING                       ("SAVE")

/*!
\def STIMA_LCD_TRACE_LEVEL
\brief Lcd trace level debug for stima sketch.
*/

#endif
