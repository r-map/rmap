/**
  ******************************************************************************
  * @file    freeRTOS_callback.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   CallBack Freertos and Handler base system function
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (C) 2022  Moreno Gasperini <m.gasperini@digiteco.it>
  * All rights reserved.</center></h2>
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
#include "FreeRTOS.h"
#include "task.h"
#include "drivers/module_master_hal.hpp"

// /*******************************************************************************************
// ********************************************************************************************
//                 Freertos Param Config and CallBack __Weak Function Redefinition
// ********************************************************************************************
// *******************************************************************************************/

#ifdef _USE_FREERTOS_LOW_POWER

/// @brief Prepara il sistema allo Sleep (OFF Circuirterie ed entrata in PowerDown)
/// @param xExpectedIdleTime Ticks RTOS (ms) attesi per la funzione di Sleep
extern "C" void xTaskSleepPrivate(TickType_t *xExpectedIdleTime) {
   #ifdef _EXIT_SLEEP_FOR_DEBUGGING
   // Imposto 0 al tempo di attesa Idle e comunica a FreeRTOS
   // Di non entrare nello stato Sleep
   *xExpectedIdleTime = 0;
   #endif
}

/// @brief Riattiva il sistema dopo lo Sleep (Riattivazione perifieriche, Clock ecc...)
/// @param xExpectedIdleTime Ticks RTOS (ms) effettivamente eseguiti dalla funzione di Sleep
extern "C" void xTaskWakeUpPrivate(TickType_t xExpectedIdleTime) {
}

#endif

//------------------------------------------------------------------------------
/// @brief LocalMS Delay Handler_XXX
/// @param millis Millisecondi di Wait
static void delayMS(uint32_t millis) {
  uint32_t iterations = millis * (F_CPU/7000);
  uint32_t i;
  for(i = 0; i < iterations; ++i) {
    __asm__("nop\n\t");
  }
}

/// @brief Buzzer Redefiniton Segnalazione LedBlink con Buzzer
/// @param n Numero di iterazioni per segnalazione
static void faultStimaV4(int n) {
  #ifdef HFLT_PIN
  __disable_irq();
  pinMode(HFLT_PIN, OUTPUT);
  for (;;) {
    int i;
    for (i = 0; i < n; i++) {
      digitalWrite(HFLT_PIN, 1);
      delayMS(300);
      digitalWrite(HFLT_PIN, 0);
      delayMS(300);
    }
    delayMS(2000);
  }
#else
  while(1);
#endif // HFLT_PIN
}
//------------------------------------------------------------------------------

#if ( configCHECK_FOR_STACK_OVERFLOW >= 1 )
	/**  Blink three short pulses if stack overflow is detected.
	Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected.
  \param[in] pxTask Task handle
  \param[in] pcTaskName Task name
  */
extern "C" void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
  (void) pcTaskName;
  (void) pxTask;
  faultStimaV4(3);
}
#endif /* configCHECK_FOR_STACK_OVERFLOW >= 1 */

// Exception Internal
volatile uint32_t exceptionInt = 0;

//------------------------------------------------------------------------------
// catch exceptions
/** Hard fault - blink four short flash every two seconds */
extern "C" void hard_fault_isr() {
  //printf("Hard fault isr\n");
  faultStimaV4(4);
}
/** Hard fault - blink four short flash every two seconds */
extern "C" void HardFault_Handler() {
  exceptionInt++;
  faultStimaV4(4);
  NVIC_SystemReset();
}

/** Bus fault - blink five short flashes every two seconds */
extern "C" void bus_fault_isr() {
  faultStimaV4(5);
}
/** Bus fault - blink five short flashes every two seconds */
extern "C" void BusFault_Handler() {
  faultStimaV4(5);
}

/** Usage fault - blink six short flashes every two seconds */
extern "C" void usage_fault_isr() {
  faultStimaV4(6);
}
/** Usage fault - blink six short flashes every two seconds */
extern "C" void UsageFault_Handler() {
  faultStimaV4(6);
}

// #endif