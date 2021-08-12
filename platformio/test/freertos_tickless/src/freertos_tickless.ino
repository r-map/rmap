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
#include <STM32RTC.h>

STM32RTC& rtc = STM32RTC::getInstance();

void taskOne( void * parameter )
{
  while( true ){ 
    digitalToggle(PA8);
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}
void setup() {
  // Select RTC clock source: LSI_CLOCK, LSE_CLOCK or HSE_CLOCK.
  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin(); // initialize RTC NEEDED to start LPTIM1
 
  pinMode(PA8, OUTPUT);   
  xTaskCreate(taskOne,"TaskOne",200,NULL,1,NULL);
  vTaskStartScheduler(); // initialise and run the freeRTOS scheduler.
  //Execution should never return here.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

