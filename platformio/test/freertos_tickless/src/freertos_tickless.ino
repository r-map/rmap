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
#include "stm32l4xx_hal_lptim.h"


LPTIM_HandleTypeDef hlptim1;
//RTC_HandleTypeDef hrtc;

static void MX_LPTIM1_Init(void)
{
  hlptim1.Instance = LPTIM1;
  hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
  hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
  hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
  if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
  {
    Error_Handler();
  }
}

/*
static void MX_RTC_Init(void)
{
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
}
*/

void taskOne( void * parameter )
{
  while( true ){ 
    digitalToggle(LED_BUILTIN);
    //delay(1000);
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}
void setup() {

  MX_LPTIM1_Init();
  //MX_RTC_Init();
  
  pinMode(LED_BUILTIN, OUTPUT);   
  xTaskCreate(taskOne,"TaskOne",200,NULL,1,NULL);
  vTaskStartScheduler(); // initialise and run the freeRTOS scheduler.
  //Execution should never return here.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

