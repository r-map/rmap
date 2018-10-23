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
\def DEBUG_WAIT_FOR_SLEEP_MS
\brief If power down mode is enable, a necessary delay was needed after a debug message for correct print.
*/
#define DEBUG_WAIT_FOR_SLEEP_MS              (10)

/*!
\def SERIAL_PRINTF_BUFFER_LENGTH
\brief Length in bytes for serial data buffer.
*/
#define SERIAL_PRINTF_BUFFER_LENGTH          (256)

/*!
\def LCD_PRINTF_BUFFER_LENGTH
\brief Length in bytes for lcd data buffer.
*/
#define LCD_PRINTF_BUFFER_LENGTH             (20)

/*!
\fn char *serial_printf(char *ptr, const char *fmt, ...)
\brief Print a message over serial port preformatting it with a user defined values and parameters.
\param[in] *ptr message to print.
\param[in] *fmt typo for vsnprintf function.
\return pointer to data buffer filled with message by vsnprintf function.
*/
char *serial_printf(char *ptr, const char *fmt, ...);

/*!
\fn char *serial_printf(char *ptr, const __FlashStringHelper *fmt, ...)
\brief Print a message over serial port preformatting it with a user defined values and parameters.
\param[in] *ptr message to print.
\param[in] *fmt progmem for vsnprintf_P function.
\return pointer to data buffer filled with message by vsnprintf function.
*/
char *serial_printf(char *ptr, const __FlashStringHelper *fmt, ...);


/*!
\fn char *serial_printf_array(void *data, int16_t length, uint8_t type, const __FlashStringHelper *fmt, ...)
\brief Print array of data over serial port preformatting it with a user defined values and parameters.
\param[in] *data array of data to print.
\param[in] length length of array
\param[in] type type of array elements for correct printing (specified by INT8, INT16, INT32, UINT8, UINT16, UINT32 defines)
\param[in] *fmt typo for vsnprintf function.
\return pointer to data buffer filled with message by vsnprintf function.
*/
char *serial_printf_array(void *data, int16_t length, uint8_t type, const __FlashStringHelper *fmt, ...);

#ifndef SERIAL_TRACE_LEVEL
/*!
\def SERIAL_TRACE_LEVEL
\brief Serial trace level is disable for default.
*/
   #define SERIAL_TRACE_LEVEL SERIAL_TRACE_LEVEL_OFF
#endif

// Debug output redirection
#if (SERIAL_TRACE_LEVEL > SERIAL_TRACE_LEVEL_OFF)
  #ifndef _SERIAL_PRINT
  #define _SERIAL_PRINT(...) Serial.print(serial_printf(__VA_ARGS__))
  #endif
  #ifndef _SERIAL_PRINT_ARRAY
  #define _SERIAL_PRINT_ARRAY(...) Serial.print(serial_printf_array(__VA_ARGS__))
  #endif
  #ifndef SERIAL_BEGIN
  #define SERIAL_BEGIN(...) Serial.begin(__VA_ARGS__)
  #endif
#else
  #ifndef SERIAL_BEGIN
  /*!
  \def SERIAL_BEGIN
  \brief If serial debug was disabled, this macro do nothing.
  */
    #define SERIAL_BEGIN(...)
  #endif
#endif

// Debugging macros
#if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_ERROR)
/*!
\def SERIAL_ERROR
\brief Useful macro for print error message on serial port through serial print macro.
*/
  #define SERIAL_ERROR(...) _SERIAL_PRINT(NULL, __VA_ARGS__)

  /*!
  \def SERIAL_ERROR_ARRAY
  \brief Useful macro for print array error message on serial port through serial print macro.
  */
  #define SERIAL_ERROR_ARRAY(...) _SERIAL_PRINT_ARRAY(__VA_ARGS__)
#else
  #define SERIAL_ERROR(...)
  #define SERIAL_ERROR_ARRAY(...)
#endif

#if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_WARNING)
   /*!
   \def SERIAL_WARNING
   \brief Useful macro for print warning message on serial port through serial print macro.
   */
   #define SERIAL_WARNING(...) _SERIAL_PRINT(NULL, __VA_ARGS__)
   /*!
   \def SERIAL_WARNING_ARRAY
   \brief Useful macro for print warning array error message on serial port through serial print macro.
   */
   #define SERIAL_WARNING_ARRAY(...) _SERIAL_PRINT_ARRAY(__VA_ARGS__)
#else
  #define SERIAL_WARNING(...)
  #define SERIAL_WARNING_ARRAY(...)
#endif

#if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_INFO)
/*!
\def SERIAL_INFO
\brief Useful macro for print info message on serial port through serial print macro.
*/
  #define SERIAL_INFO(...) _SERIAL_PRINT(NULL, __VA_ARGS__)

  /*!
  \def SERIAL_INFO_ARRAY
  \brief Useful macro for print info array error message on serial port through serial print macro.
  */
  #define SERIAL_INFO_ARRAY(...) _SERIAL_PRINT_ARRAY(__VA_ARGS__)
#else
  #define SERIAL_INFO(...)
  #define SERIAL_INFO_ARRAY(...)
#endif

#if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_DEBUG)
/*!
\def SERIAL_DEBUG
\brief Useful macro for print verbose message on serial port through serial print macro.
*/
  #define SERIAL_DEBUG(...) _SERIAL_PRINT(NULL, __VA_ARGS__)

  /*!
  \def SERIAL_DEBUG_ARRAY
  \brief Useful macro for print verbose array error message on serial port through serial print macro.
  */
  #define SERIAL_DEBUG_ARRAY(...) _SERIAL_PRINT_ARRAY(__VA_ARGS__)
#else
  #define SERIAL_DEBUG(...)
  #define SERIAL_DEBUG_ARRAY(...)
#endif

#if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
/*!
\def SERIAL_TRACE
\brief Useful macro for print all verbose message on serial port through serial print macro.
*/
  #define SERIAL_TRACE(...) _SERIAL_PRINT(NULL, __VA_ARGS__)

  /*!
  \def SERIAL_TRACE_ARRAY
  \brief Useful macro for print all verbose array error message on serial port through serial print macro.
  */
  #define SERIAL_TRACE_ARRAY(...) _SERIAL_PRINT_ARRAY(__VA_ARGS__)
#else
  #define SERIAL_TRACE(...)
  #define SERIAL_TRACE_ARRAY(...)
#endif

// Default Debug level
#ifndef LCD_TRACE_LEVEL
/*!
\def LCD_TRACE_LEVEL
\brief Lcd trace level is disable for default.
*/
   #define LCD_TRACE_LEVEL LCD_TRACE_LEVEL_OFF
#endif

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
