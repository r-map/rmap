/**
  ******************************************************************************
  * @file    debug.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
  * @brief   debug and logger
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
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

#include "config.h"

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "semaphore.hpp"
#include "queue.hpp"

using namespace cpp_freertos;

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

void init_debug(uint32_t baudrate) {
  #ifndef DISABLE_SERIAL
  Serial.begin(baudrate);
  while (!Serial);
  #endif
}

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

void queue_debug_F(Queue *dataLogPutQueue, const __FlashStringHelper *fmt, ...)
{
  // Only if queue not Full... rapid check
  if(!dataLogPutQueue->IsFull()) {
    char logMessage[LOG_PUT_DATA_ELEMENT_SIZE];
    va_list args;
    va_start(args, fmt);
    sprintf(logMessage, (const char *)fmt, args);
    va_end(args);
    dataLogPutQueue->Enqueue(logMessage, 0);
  }
}
