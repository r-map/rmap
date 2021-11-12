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
