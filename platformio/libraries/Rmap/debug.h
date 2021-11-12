/**@file debug.h */

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

#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdarg.h>
#include <stdio.h>
#include <LiquidCrystal_I2C.h>

/*!
\def INT8
\brief Indicate a int8_t type.
*/
#define INT8    (1)

/*!
\def INT16
\brief Indicate a int16_t type.
*/
#define INT16   (2)

/*!
\def INT32
\brief Indicate a int32_t type.
*/
#define INT32   (3)

/*!
\def UINT8
\brief Indicate a uint8_t type.
*/
#define UINT8   (4)

/*!
\def UINT16
\brief Indicate a uint16_t type.
*/
#define UINT16  (5)

/*!
\def UINT32
\brief Indicate a uint32_t type.
*/
#define UINT32  (6)

/*!
\def FLOAT
\brief Indicate a float type.
*/
#define FLOAT   (7)

/*!
\def LCD_PRINTF_BUFFER_LENGTH
\brief Length in bytes for lcd data buffer.
*/
#define LCD_PRINTF_BUFFER_LENGTH             (20)

/*!
\fn void lcd_begin(LiquidCrystal_I2C *lcd, uint8_t max_cols, uint8_t max_rows)
\brief Initialize LCD library.
\param[in] *lcd pointer to lcd instance.
\param[in] max_cols number of lcd columns.
\param[in] max_rows number of lcd rows.
\return void.
*/
void lcd_begin(LiquidCrystal_I2C *lcd, uint8_t max_cols, uint8_t max_rows);

/*!
\fn char *lcd_printf(LiquidCrystal_I2C *lcd, bool do_clear, bool go_to_next_line, char *ptr, const __FlashStringHelper *fmt, ...)
\brief Print a message on lcd preformatting it with a user defined values and parameters.
\param[in] *lcd pointer to lcd instance.
\param[in] do_clear if true, clear lcd before printing message
\param[in] go_to_next_line if true, print message to next line
\param[in] *ptr message to print.
\param[in] *fmt typo progmem for vsnprintf function.
\return pointer to data buffer filled with message by vsnprintf function.
*/
char *lcd_printf(LiquidCrystal_I2C *lcd, bool do_clear, bool go_to_next_line, char *ptr, const __FlashStringHelper *fmt, ...);

/*!
\fn char *lcd_printf(LiquidCrystal_I2C *lcd, bool do_clear, bool go_to_next_line, char *ptr, const char *fmt, ...)
\brief Print a message on lcd preformatting it with a user defined values and parameters.
\param[in] *lcd pointer to lcd instance.
\param[in] do_clear if true, clear lcd before printing message
\param[in] go_to_next_line if true, print message to next line
\param[in] *ptr message to print.
\param[in] *fmt typo for vsnprintf function.
\return pointer to data buffer filled with message by vsnprintf function.
*/
char *lcd_printf(LiquidCrystal_I2C *lcd, bool do_clear, bool go_to_next_line, char *ptr, const char *fmt, ...);

// Debug output redirection
#if (LCD_TRACE_LEVEL > LCD_TRACE_LEVEL_OFF)

  #ifndef _LCD_PRINT
  /*!
  \def _LCD_PRINT(lcd, do_clear, ...)
  \brief Useful macro for print message on lcd port through lcd print function.
  */
   #define _LCD_PRINT(lcd, do_clear, go_to_next_line, ...) (((LiquidCrystal_I2C *) lcd)->print(lcd_printf(lcd, do_clear, go_to_next_line, NULL, __VA_ARGS__)))
  #endif

  #ifndef LCD_BEGIN
  /*!
  \def LCD_BEGIN(lcd, max_cols, max_rows)
  \brief Useful macro for initialize lcd library.
  */
   #define LCD_BEGIN(lcd, max_cols, max_rows) (lcd_begin(lcd, max_cols, max_rows))
  #endif
#else
  #ifndef LCD_BEGIN
  /*!
  \def LCD_BEGIN
  \brief Useful macro for initialize lcd library.
  */
   #define LCD_BEGIN(...)
  #endif
#endif

// Debugging macros
#if (LCD_TRACE_LEVEL >= LCD_TRACE_LEVEL_ERROR)
/*!
\def LCD_ERROR
\brief Useful macro for print error message on lcd port through lcd print macro.
*/
  #define LCD_ERROR(...) _LCD_PRINT(__VA_ARGS__)
#else
  #define LCD_ERROR(...)
#endif

#if (LCD_TRACE_LEVEL >= LCD_TRACE_LEVEL_WARNING)
/*!
\def LCD_WARNING
\brief Useful macro for print warning message on lcd port through lcd print macro.
*/
  #define LCD_WARNING(...) _LCD_PRINT( __VA_ARGS__)
#else
  #define LCD_WARNING(...)
#endif

#if (LCD_TRACE_LEVEL >= LCD_TRACE_LEVEL_INFO)
/*!
\def LCD_INFO
\brief Useful macro for print info message on lcd port through lcd print macro.
*/
  #define LCD_INFO(...) _LCD_PRINT(__VA_ARGS__)
#else
  #define LCD_INFO(...)
#endif

#if (LCD_TRACE_LEVEL >= LCD_TRACE_LEVEL_DEBUG)
/*!
\def LCD_DEBUG
\brief Useful macro for print verbose message on lcd port through lcd print macro.
*/
  #define LCD_DEBUG(...) _LCD_PRINT( __VA_ARGS__)
#else
  #define LCD_DEBUG(...)
#endif

#endif
