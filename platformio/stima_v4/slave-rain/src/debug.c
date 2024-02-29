/**@file debug.cpp */

/*********************************************************************
<h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
authors:
Marco Baldinetti <marco.baldinetti@digiteco.it>

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
#include <stdarg.h>
#include "debug.h"

void print_debug(const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   vfprintf(stdout, fmt, args);
   va_end(args);
}

/**
 * @brief Display the contents of an array
 * @param[in] stream Pointer to a FILE object that identifies an output stream
 * @param[in] prepend String to prepend to the left of each line
 * @param[in] data Pointer to the data array
 * @param[in] length Number of bytes to display
 **/
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