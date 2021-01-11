/****************************************************************************
Copyright (C) 2021  Paolo Paruno <p.patruno@iperbole.bologna.it>
authors: Paolo Patruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Freeg Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

#include "STM32FreeRTOS.h"

void taskOne( void * parameter )
{
  while( true ){ 
    digitalToggle(LED_BUILTIN);    
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);   
 
  xTaskCreate(
	      taskOne,          /* Task function. */
	      "TaskOne",        /* String with name of task. */
	      200,              /* Stack size in bytes. */
	      NULL,             /* Parameter passed as input of the task */
	      1,                /* Priority of the task. */
	      NULL);            /* Task handle. */

  vTaskStartScheduler(); // initialise and run the freeRTOS scheduler. Execution should never return here.
  
}

void loop()
{
  // Empty. Things are done in Tasks.
}

