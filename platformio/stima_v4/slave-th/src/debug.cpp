/**@file debug.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>

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

#include "debug.h"

int_t fputc(int_t c, FILE *stream)
{
   // Standard output or error output?
   if (stream == stdout || stream == stderr)
   {
      Serial.write(c);
      return c;
   }
   // Unknown output?
   else
   {
      // If a writing error occurs, EOF is returned
      return EOF;
   }
}

void init_debug(uint32_t baudrate) {
  Serial.begin(baudrate);
}

void print_debug(const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   vfprintf(stdout, fmt, args);
   va_end(args);
}

void print_debug_F(const __FlashStringHelper *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   vfprintf(stdout, (const char *)fmt, args);
   va_end(args);
}

/**
 * @brief Display the contents of an array
 * @param[in] stream Pointer to a FILE object that identifies an output stream
 * @param[in] prepend String to prepend to the left of each line
 * @param[in] data Pointer to the data array
 * @param[in] length Number of bytes to display
 **/
void print_debug_array_F(const char *prepend, const void *data, size_t length)
{
   for (uint8_t i = 0; i < length; i++)
   {
      // Beginning of a new line?
      if ((i % 16) == 0) {
         print_debug("%s", prepend);
      }
      // Display current data byte
      print_debug("%02" PRIX8 " ", *((uint8_t *)data + i));
      // End of current line?
      if ((i % 16) == 15 || i == (length - 1)) {
         print_debug("\r\n");
      }
   }
}