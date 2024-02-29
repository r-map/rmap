/**
  ******************************************************************************
  * @file    freeRTOS_callback.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   CallBack Freertos and Handler base system function
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

#include "FreeRTOS.h"
#include "task.h"
#include "drivers/module_slave_hal.hpp"
#include "STM32LowPower.h"
#include <IWatchdog.h>

// /*******************************************************************************************
// ********************************************************************************************
//                 Freertos Param Config and CallBack __Weak Function Redefinition
// ********************************************************************************************
// *******************************************************************************************/

#ifdef _USE_FREERTOS_LOW_POWER

volatile bool isSleep = false;

/// @brief Prepara il sistema allo Sleep (OFF Circuirterie ed entrata in PowerDown)
/// @param xExpectedIdleTime Ticks RTOS (ms) attesi per la funzione di Sleep
extern "C" void xTaskSleepPrivate(TickType_t *xExpectedIdleTime) {
  #if (LOWPOWER_MODE==SLEEP_IDLE)
    LowPower.idle(*xExpectedIdleTime);
  #elif (LOWPOWER_MODE==SLEEP_LOWPOWER)
    LowPower.sleep(*xExpectedIdleTime - 10);
  #elif (LOWPOWER_MODE==SLEEP_STOP2)
    isSleep = true;
    LowPower.deepSleep(*xExpectedIdleTime - 10);
  #else
  *xExpectedIdleTime = 0;
  #endif
}

/// @brief Riattiva il sistema dopo lo Sleep (Riattivazione perifieriche, Clock ecc...)
/// @param xExpectedIdleTime Ticks RTOS (ms) effettivamente eseguiti dalla funzione di Sleep
extern "C" void xTaskWakeUpPrivate(TickType_t *xExpectedIdleTime) {
  isSleep = false;
}

// Remove Arduino OSSysTick for LPTIM(x) IRQ lptimTick.c Driver (AutoInc OsTick)
// Is Need to redefined weak void __attribute__((weak)) osSystickHandler(void)
// Note FROM Freertos_Config.h 
/*
 * IMPORTANT:
 * SysTick_Handler() from stm32duino core is calling weak osSystickHandler().
 * Both CMSIS-RTOSv2 and CMSIS-RTOS override osSystickHandler() 
 * which is calling xPortSysTickHandler(), defined in respective CortexM-x port
*/
#if ( configUSE_TICKLESS_IDLE == 2 )
extern "C" void osSystickHandler()
{
  // osSystickHandler CallBack UNUSED for LPTIM1 IRQ Set Increment of OsTickHadler
  // Optional User Code about osSystickHandler Private Here
  // ...
}
#endif

#endif

#if(DEBUG_MODE)
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
    #if (ENABLE_WDT)
    // Error signal WDT Refresh (For debugger only)
    IWatchdog.reload();
    #endif
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
#endif

#if ( configCHECK_FOR_STACK_OVERFLOW >= 1 )
	/**  Blink three short pulses if stack overflow is detected.
	Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected.
  \param[in] pxTask Task handle
  \param[in] pcTaskName Task name
  */
extern "C" void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
  // Ned to use Serial.print direct for prevent Malloc from RTOS (ISR Malloc ASSERT Error)
  Serial.print("Error stack overflow form task: ");
  Serial.print(pcTaskName);
  Serial.flush();
  #if(DEBUG_MODE)
  faultStimaV4(3);
  #else
  NVIC_SystemReset();
  #endif
}
#endif /* configCHECK_FOR_STACK_OVERFLOW >= 1 */

//------------------------------------------------------------------------------
// catch exceptions

// Generic Error_Handler
#if(ERROR_HANDLER_CB)
extern "C" void _Error_Handler(const char *msg, int val)
{
  /* User can add his own implementation to report the HAL error return state */
  // Ned to use Serial.print direct for prevent Malloc from RTOS (ISR Malloc ASSERT Error)
  Serial.print("Error handler: ");
  Serial.print(msg);
  Serial.print(", ");
  Serial.print(val);
  Serial.flush();
  #if(DEBUG_MODE)
  faultStimaV4(3);
  #else
  NVIC_SystemReset();
  #endif
}
#endif

/** Hard fault - blink four short flash every two seconds */
extern "C" void hard_fault_isr() {
  #if(DEBUG_MODE)
  faultStimaV4(4);
  #else
  NVIC_SystemReset();
  #endif
}
/** Hard fault - blink four short flash every two seconds */
extern "C" void HardFault_Handler() {
  #if(DEBUG_MODE)
  faultStimaV4(4);
  #else
  NVIC_SystemReset();
  #endif
}

/** Bus fault - blink five short flashes every two seconds */
extern "C" void bus_fault_isr() {
  #if(DEBUG_MODE)
  faultStimaV4(5);
  #else
  NVIC_SystemReset();
  #endif
}
/** Bus fault - blink five short flashes every two seconds */
extern "C" void BusFault_Handler() {
  #if(DEBUG_MODE)
  faultStimaV4(5);
  #else
  NVIC_SystemReset();
  #endif
}

/** Usage fault - blink six short flashes every two seconds */
extern "C" void usage_fault_isr() {
  #if(DEBUG_MODE)
  faultStimaV4(6);
  #else
  NVIC_SystemReset();
  #endif
}
/** Usage fault - blink six short flashes every two seconds */
extern "C" void UsageFault_Handler() {
  #if(DEBUG_MODE)
  faultStimaV4(6);
  #else
  NVIC_SystemReset();
  #endif
}

/** Usage fault - blink six short flashes every two seconds */
extern "C" void MemManage_fault_isr() {
  #if(DEBUG_MODE)
  faultStimaV4(7);
  #else
  NVIC_SystemReset();
  #endif
}
/** Usage fault - blink six short flashes every two seconds */
extern "C" void MemManage_Handler() {
  #if(DEBUG_MODE)
  faultStimaV4(7);
  #else
  NVIC_SystemReset();
  #endif
}
