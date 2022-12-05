/**
  ******************************************************************************
  * @file    module_slave_hal.hpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Interface STM32 hardware_hal STIMAV4 Header config
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

#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include "STM32FreeRTOSConfig_extra.h"

// /* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MODULE_SLAVE_HAL_H
#define __MODULE_SLAVE_HAL_H

#define USE_HAL_DRIVER        (true)

#define ENABLE_I2C1           (true)
#define ENABLE_I2C2           (false)
#define ENABLE_QSPI           (true)
#define ENABLE_CAN            (true)

// HW Diag PIN redefine
#define ENABLE_DIAG_PIN       (false)   // No DIAG Pin in MPPT

#if (ENABLE_I2C1 || ENABLE_I2C2)
#include <Wire.h>
#define I2C_MAX_DATA_LENGTH   (32)
#define I2C_MAX_ERROR_COUNT   (3)
#endif

#if (ENABLE_I2C1)
#define I2C1_BUS_CLOCK_HZ     (100000L)
#endif

#if (ENABLE_I2C2)
#define I2C2_BUS_CLOCK_HZ     (100000L)
extern TwoWire Wire2;
#endif

#if (ENABLE_QSPI)
extern QSPI_HandleTypeDef hqspi;
#define QSPI_NVIC_INT_PREMPT_PRIORITY 7
#endif

#if (ENABLE_CAN)
extern CAN_HandleTypeDef hcan1;
#define CAN_NVIC_INT_PREMPT_PRIORITY 8
#endif

// INIT HW PRIVATE BOARD/ISTANCE CFG

// #define _HW_SETUP_CRC_PRIVATE
// #define _HW_SETUP_LPTIM_PRIVATE
#define _HW_SETUP_QSPI_PRIVATE
// #define _HW_SETUP_RNG_PRIVATE

// ******************************************************************************

// PIN NAMED STM32 ARDUINO GPIO_INIT

// CAN BUS
#define PIN_CAN_RX1     PB12
#define PIN_CAN_TX1     PB13
#define PIN_CAN_EN      PC12
#define PIN_CAN_STB     PB15

#define PIN_I2C1_SDA    PB7
#define PIN_I2C1_SCL    PB6

#define PIN_SYS_JTMS    PA13

// PIN UART1 (Monitor Debugger)
#define PIN_USART1_TX   PA10
#define PIN_USART1_RX   PA9

#if (ENABLE_DIAG_PIN)
// DIAG PIN (LED + BUTTON COME TEST NUCLEO)
// Commentare per escludere la funzionalità
// #define HFLT_PIN  // N.C. in Module_Power -> Output Signal Fault_Handler
// #define LED1_PIN  // LED 1 Nucleo Simulator
// #define LED2_PIN  // LED 2 Nucleo Simulator
// #define USER_INP  // BTN_I Nucleo Simulator
#endif

// *****************************

// PIN NAMED STM32CUBE GPIO_INIT
#define STB_CAN_Pin       GPIO_PIN_15
#define STB_CAN_GPIO_Port GPIOB
#define EN_CAN_Pin        GPIO_PIN_12
#define EN_CAN_GPIO_Port  GPIOC

// ******************************************************************************

// Data Istance and Prototype Function Extern "C"

#ifdef _HW_SETUP_CRC_PRIVATE
extern CRC_HandleTypeDef hcrc;
#endif

#ifdef _HW_SETUP_LPTIM_PRIVATE
extern LPTIM_HandleTypeDef hlptim1;
#endif

/* Private Hardware_Handler istance initialization ---------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

void SystemClock_Config(void);
void SetupSystemPeripheral(void);
void HAL_MspInit(void);
void MX_GPIO_Init(void);

#if (ENABLE_CAN)
void MX_CAN1_Init(void);
void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan);
void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan);
#endif

#ifdef _HW_SETUP_CRC_PRIVATE
void MX_CRC_Init(void);
#endif
#ifdef _HW_MSP_CRC_PRIVATE
void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc);
void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc);
#endif

#ifdef _HW_SETUP_LPTIM_PRIVATE
void MX_LPTIM1_Init(void);
#endif
#ifdef _HW_MSP_LPTIM_PRIVATE
void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef* hlptim);
void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef* hlptim);
#endif

#if (ENABLE_QSPI)
void MX_QUADSPI_Init(void);
void HAL_QSPI_MspInit(QSPI_HandleTypeDef* hqspi);
void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* hqspi);
#endif

#ifdef _HW_SETUP_RNG_PRIVATE
void MX_RNG_Init(void);
#endif
#ifdef _HW_MSP_RNG_PRIVATE
void HAL_RNG_MspInit(RNG_HandleTypeDef* hrng);
void HAL_RNG_MspDeInit(RNG_HandleTypeDef* hrng);
#endif

#ifdef __cplusplus
}
#endif

#endif // __MODULE_SLAVE_HAL_H