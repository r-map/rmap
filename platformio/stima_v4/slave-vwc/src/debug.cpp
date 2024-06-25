/**
  ******************************************************************************
  * @file    debug.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
  * @brief   debug for stimaV4 Slave source file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
  * <h2><center>All rights reserved.</center></h2>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  * 
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
  * <http://www.gnu.org/licenses/>.
  * 
  ******************************************************************************
*/

#include "debug.h"

/// @brief Put char to file output
/// @param c char to write
/// @param stream output mode file
/// @return success
int_t fputc(int_t c, FILE *stream)
{
   // Standard output or error output?
   if (stream == stdout || stream == stderr)
   {
      #ifndef DISABLE_SERIAL
      Serial.write(c);
      #endif
      return c;
   }
   // Unknown output?
   else
   {
      // If a writing error occurs, EOF is returned
      return EOF;
   }
}

/// @brief init serial monitor
/// @param baudrate speed monitor
void init_debug(uint32_t baudrate) {
  #ifndef DISABLE_SERIAL
  Serial.begin(baudrate);
  while (!Serial);
  #endif
}

/// @brief Print debug from ram
/// @param fmt pointer to class FlashStringHelper
/// @param any format output printf
void print_debug(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vfprintf(stdout, fmt, args);
  va_end(args);
  #ifndef DISABLE_SERIAL
  Serial.flush();
  #endif
}

/// @brief Display the contents of an array
/// @param prepend String to prepend to the left of each line
/// @param data Pointer to the data array
/// @param length Number of bytes to display
void print_debug_array(const char *prepend, const void *data, size_t length)
{
   for (uint8_t i = 0; i < length; i++)
   {
      // Beginning of a new line?
      if ((i % 16) == 0)
      {
         TRACE_PRINTF("%s", prepend);
      }
      // Display current data byte
      TRACE_PRINTF("%02" PRIX8 " ", *((const uint8_t *)data + i));
      // End of current line?
      if ((i % 16) == 15 || i == (length - 1))
      {
         TRACE_PRINTF("\r\n");
      }
   }
}

/// @brief Print debug from rom Flash
/// @param fmt pointer to class FlashStringHelper
/// @param any format output printf
void print_debug_F(const __FlashStringHelper *fmt, ...)
{
  osSuspendAllTasks();
  va_list args;
  va_start(args, fmt);
  vfprintf(stdout, (const char *)fmt, args);
  va_end(args);
  #ifndef DISABLE_SERIAL
  Serial.flush();
  #endif
  osResumeAllTasks();
}
