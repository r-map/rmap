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
  while (!Serial);
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
  osSuspendAllTasks();
  va_list args;
  va_start(args, fmt);
  vfprintf(stdout, (const char *)fmt, args);
  va_end(args);
  osResumeAllTasks();
}