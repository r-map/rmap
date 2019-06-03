/**@file debug.cpp */

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

#include "debug.h"

/*!
\var serial_buffer_print[SERIAL_PRINTF_BUFFER_LENGTH]
\brief Serial data buffer for storing message before print.
*/
char serial_buffer_print[SERIAL_PRINTF_BUFFER_LENGTH];

/*!
\var serial_buffer_print_written_char
\brief Length in bytes of serial data buffer.
*/
int16_t serial_buffer_print_written_char;

char *serial_printf(bool is_add_hashtag, char *ptr, const char *fmt, ...) {
   va_list args;
   va_start (args, fmt);

   if (ptr == NULL) {
      ptr = serial_buffer_print;
   }

   if (is_add_hashtag) {
     ptr[0] = '#';
     serial_buffer_print_written_char = vsnprintf_P(ptr+1, SERIAL_PRINTF_BUFFER_LENGTH-1, fmt, args);
   }
   else {
     serial_buffer_print_written_char = vsnprintf_P(ptr, SERIAL_PRINTF_BUFFER_LENGTH-1, fmt, args);
   }

   serial_buffer_print_written_char = vsnprintf_P(ptr, SERIAL_PRINTF_BUFFER_LENGTH-1, fmt, args);
   va_end (args);
   return ptr;
}

char *serial_printf(bool is_add_hashtag, char *ptr, const __FlashStringHelper *fmt, ...) {
   va_list args;
   va_start (args, fmt);

   if (ptr == NULL) {
      ptr = serial_buffer_print;
   }

   if (is_add_hashtag) {
     ptr[0] = '#';
     serial_buffer_print_written_char = vsnprintf_P(ptr+1, SERIAL_PRINTF_BUFFER_LENGTH-1, (const char *) fmt, args);
   }
   else {
     serial_buffer_print_written_char = vsnprintf_P(ptr, SERIAL_PRINTF_BUFFER_LENGTH-1, (const char *) fmt, args);
   }

   va_end (args);
   return ptr;
}

char *serial_printf_array(bool is_add_hashtag, void *data, int16_t length, uint8_t type, const __FlashStringHelper *fmt, ...) {
  bool _is_add_hashtag = is_add_hashtag;
  char *serial_buffer_ptr = serial_buffer_print;

  if (length > SERIAL_PRINTF_BUFFER_LENGTH) {
    length = SERIAL_PRINTF_BUFFER_LENGTH;
  }

  // real data may > of length
  for (int i=0; i<length; i++) {
    if (i > 0) {
      _is_add_hashtag = false;
    }

    switch (type) {
      case INT8:
      serial_printf(_is_add_hashtag, serial_buffer_ptr, fmt, ((int8_t *)(data))[i]);
      break;
      case INT16:
      serial_printf(_is_add_hashtag, serial_buffer_ptr, fmt, ((int16_t *)(data))[i]);
      break;
      case INT32:
      serial_printf(_is_add_hashtag, serial_buffer_ptr, fmt, ((int32_t *)(data))[i]);
      break;
      case UINT8:
      serial_printf(_is_add_hashtag, serial_buffer_ptr, fmt, ((uint8_t *)(data))[i]);
      break;
      case UINT16:
      serial_printf(_is_add_hashtag, serial_buffer_ptr, fmt, ((uint16_t *)(data))[i]);
      break;
      case UINT32:
      serial_printf(_is_add_hashtag, serial_buffer_ptr, fmt, ((uint32_t *)(data))[i]);
      break;
      case FLOAT:
      serial_printf(_is_add_hashtag, serial_buffer_ptr, fmt, ((float *)(data))[i]);
      break;
    }
    serial_buffer_ptr += serial_buffer_print_written_char;
  }

  // delete last character
  *(serial_buffer_ptr-1) = 0;

  return serial_buffer_print;
}

/*!
\var lcd_buffer_print[LCD_PRINTF_BUFFER_LENGTH]
\brief Lcd data buffer for storing message before print.
*/
char lcd_buffer_print[LCD_PRINTF_BUFFER_LENGTH];

/*!
\var lcd_current_row
\brief Indicate the "free to print" lcd row.
*/
uint8_t lcd_current_row;

/*!
\var lcd_max_cols
\brief Number of lcd columns.
*/
uint8_t lcd_max_cols;

/*!
\var lcd_max_rows
\brief Number of lcd rows.
*/
uint8_t lcd_max_rows;

void lcd_begin (LiquidCrystal_I2C *lcd, uint8_t max_cols, uint8_t max_rows) {
   lcd_max_rows = max_rows;
   lcd_max_cols = max_cols;
   lcd_current_row = 0;
   lcd->begin(max_cols, max_rows);
   lcd->clear();
   lcd->backlight();
}

char *lcd_printf(LiquidCrystal_I2C *lcd, bool do_clear, bool go_to_next_line, char *ptr, const char *fmt, ...) {
   va_list args;
   va_start (args, fmt);

   if (ptr == NULL) {
      ptr = lcd_buffer_print;
   }

   if (do_clear) {
      lcd->clear();
      lcd->backlight();
      lcd_current_row = 0;
   }

   uint8_t count = vsnprintf(ptr, LCD_PRINTF_BUFFER_LENGTH, fmt, args);
   memset(ptr + count, ' ', lcd_max_cols - count - 1);

   if (go_to_next_line) {
      lcd->setCursor(0, lcd_current_row);

      if (++lcd_current_row == lcd_max_rows) {
        lcd_current_row = 0;
      }
   }

   va_end (args);
   return ptr;
}

char *lcd_printf(LiquidCrystal_I2C *lcd, bool do_clear, bool go_to_next_line, char *ptr, const __FlashStringHelper *fmt, ...) {
   va_list args;
   va_start (args, fmt);

   if (ptr == NULL) {
      ptr = lcd_buffer_print;
   }

   if (do_clear) {
      lcd->clear();
      lcd_current_row = 0;
   }

   uint8_t count = vsnprintf_P(ptr, LCD_PRINTF_BUFFER_LENGTH, (const char*) fmt, args);
   memset(ptr + count, ' ', lcd_max_cols - count - 1);

   if (go_to_next_line) {
      lcd->setCursor(0, lcd_current_row);

      if (++lcd_current_row == lcd_max_rows) {
        lcd_current_row = 0;
      }
   }

   va_end (args);
   return ptr;
}
