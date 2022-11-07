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

void init_debug(uint32_t baudrate) {
  Serial4.begin(baudrate);
}

/**
 * @brief Display the contents of an array
 * @param[in] stream Pointer to a FILE object that identifies an output stream
 * @param[in] prepend String to prepend to the left of each line
 * @param[in] data Pointer to the data array
 * @param[in] length Number of bytes to display
 **/

void debugDisplayArray(Stream *stream, const char_t *prepend, const void *data, size_t length) {
  for (uint16_t i = 0; i < length; i++) {
    //Beginning of a new line?
    if ((i % 16) == 0) {
      stream->printf("%s", prepend);
      // fprintf(stream, "%s", prepend);
    }

    //Display current data byte
    stream->printf("%02" PRIX8 " ", *((uint8_t *) data + i));
    // fprintf(stream, "%02" PRIX8 " ", *((uint8_t *) data + i));

    //End of current line?
    if ((i % 16) == 15 || i == (length - 1)) {
      stream->printf("%s", prepend);
      // fprintf(stream, "\r\n");
    }
  }
}


/**
 * @brief Write character to stream
 * @param[in] c The character to be written
 * @param[in] stream Pointer to a FILE object that identifies an output stream
 * @return On success, the character written is returned. If a writing
 *   error occurs, EOF is returned
 **/

// int_t fputc(int_t c, FILE *stream) {
//    //Standard output or error output?
//    if(stream == stdout || stream == stderr) {
//       //Transmit data
//       Serial.print(c);
//       //On success, the character written is returned
//       return c;
//    }
//    //Unknown output?
//    else {
//       //If a writing error occurs, EOF is returned
//       return EOF;
//    }
// }
