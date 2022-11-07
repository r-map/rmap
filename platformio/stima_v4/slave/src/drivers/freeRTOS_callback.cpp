// /**
//   ******************************************************************************
//   * @file    freeRTOS_callback.cpp
//   * @brief   freeRTOS_callback hal configuration
//   ******************************************************************************
//   * @attention
//   *
//   * This software is distributed under the terms of the MIT License.
//   * Progetto RMAP - STIMA V4
//   * Hardware Config, STIMAV4 MASTER Board - Rev.1.00
//   * Copyright (C) 2022 Digiteco s.r.l.
//   * Author: Gasperini Moreno <m.gasperini@digiteco.it>
//   *
//   ******************************************************************************
// **/

// #include <Arduino.h>
// #include "FreeRTOS.h"
// #include "task.h"
#include "drivers/module_slave_hal.hpp"

// /*******************************************************************************************
// ********************************************************************************************
//                 Freertos Param Config and CallBack __Weak Function Redefinition
// ********************************************************************************************
// *******************************************************************************************/

// Local prototype Static
static void delayMS(uint32_t millis);
static void signalFaultStimaV4(int n);

// Implementation

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
static void signalFaultStimaV4(int n) {
  #ifdef PIN_SIG_FAULT
  __disable_irq();
  pinMode(PIN_SIG_FAULT, OUTPUT);
  for (;;) {
    int i;
    for (i = 0; i < n; i++) {
      digitalWrite(PIN_SIG_FAULT, 1);
      delayMS(300);
      digitalWrite(PIN_SIG_FAULT, 0);
      delayMS(300);
    }
    delayMS(2000);
  }
#else
  while(1);
#endif // PIN_BUZZER
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
  signalFaultStimaV4(3);
}
#endif /* configCHECK_FOR_STACK_OVERFLOW >= 1 */

// Exception Internal
volatile uint32_t exceptionInt = 0;

//------------------------------------------------------------------------------
// catch exceptions
/** Hard fault - blink four short flash every two seconds */
extern "C" void hard_fault_isr() {
  //printf("Hard fault isr\n");
  signalFaultStimaV4(4);
}
/** Hard fault - blink four short flash every two seconds */
extern "C" void HardFault_Handler() {
  exceptionInt++;
  signalFaultStimaV4(4);
  NVIC_SystemReset();
}

/** Bus fault - blink five short flashes every two seconds */
extern "C" void bus_fault_isr() {
  signalFaultStimaV4(5);
}
/** Bus fault - blink five short flashes every two seconds */
extern "C" void BusFault_Handler() {
  signalFaultStimaV4(5);
}

/** Usage fault - blink six short flashes every two seconds */
extern "C" void usage_fault_isr() {
  signalFaultStimaV4(6);
}
/** Usage fault - blink six short flashes every two seconds */
extern "C" void UsageFault_Handler() {
  signalFaultStimaV4(6);
}

// #endif