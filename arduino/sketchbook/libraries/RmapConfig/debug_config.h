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
\def SERIAL_TRACE_LEVEL_OFF
\brief Debug level for disable debug on serial interface.
*/
#define SERIAL_TRACE_LEVEL_OFF            (0)

/*!
\def SERIAL_TRACE_LEVEL_ERROR
\brief Debug level for print error message on serial interface.
*/
#define SERIAL_TRACE_LEVEL_ERROR          (1)

/*!
\def SERIAL_TRACE_LEVEL_WARNING
\brief Debug level for print warning message on serial interface.
*/
#define SERIAL_TRACE_LEVEL_WARNING        (2)

/*!
\def SERIAL_TRACE_LEVEL_INFO
\brief Debug level for print informations message on serial interface.
*/
#define SERIAL_TRACE_LEVEL_INFO           (3)

/*!
\def SERIAL_TRACE_LEVEL_DEBUG
\brief Debug level for print verbose informations message on serial interface.
*/
#define SERIAL_TRACE_LEVEL_DEBUG          (4)

/*!
\def SERIAL_TRACE_LEVEL_TRACE
\brief Debug level for print detailed informations message on serial interface.
*/
#define SERIAL_TRACE_LEVEL_TRACE          (5)

/*!
\def LCD_TRACE_LEVEL_OFF
\brief Debug level for print error message on lcd interface.
*/
#define LCD_TRACE_LEVEL_OFF               (0)

/*!
\def LCD_TRACE_LEVEL_ERROR
\brief Debug level for print error message on lcd.
*/
#define LCD_TRACE_LEVEL_ERROR             (1)

/*!
\def LCD_TRACE_LEVEL_WARNING
\brief Debug level for print detailed informations message on serial interface.
*/
#define LCD_TRACE_LEVEL_WARNING           (2)

/*!
\def LCD_TRACE_LEVEL_INFO
\brief Debug level for print informations message on lcd.
*/
#define LCD_TRACE_LEVEL_INFO              (3)

/*!
\def LCD_TRACE_LEVEL_DEBUG
\brief Debug level for print verbose informations message on lcd.
*/
#define LCD_TRACE_LEVEL_DEBUG             (4)

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
\def SENSOR_DRIVER_SERIAL_TRACE_LEVEL
\brief Serial trace level debug for SensorDriver library.
*/
#define SENSOR_DRIVER_SERIAL_TRACE_LEVEL  (SERIAL_TRACE_LEVEL_OFF)

/*!
\def SIM800_SERIAL_TRACE_LEVEL
\brief Serial trace level debug for Sim800 library.
*/
#define SIM800_SERIAL_TRACE_LEVEL         (SERIAL_TRACE_LEVEL_INFO)

/*!
\def I2C_TH_SERIAL_TRACE_LEVEL
\brief Serial trace level debug for i2c-th sketch.
*/
#define I2C_TH_SERIAL_TRACE_LEVEL         (SERIAL_TRACE_LEVEL_INFO)

/*!
\def I2C_RAIN_SERIAL_TRACE_LEVEL
\brief Serial trace level debug for i2c-rain sketch.
*/
#define I2C_RAIN_SERIAL_TRACE_LEVEL       (SERIAL_TRACE_LEVEL_INFO)

/*!
\def RMAP_SERIAL_TRACE_LEVEL
\brief Serial trace level debug for rmap sketch.
*/
#define RMAP_SERIAL_TRACE_LEVEL           (SERIAL_TRACE_LEVEL_INFO)

/*!
\def RMAP_LCD_TRACE_LEVEL
\brief Lcd trace level debug for rmap sketch.
*/
#define RMAP_LCD_TRACE_LEVEL              (LCD_TRACE_LEVEL_INFO)

#endif
