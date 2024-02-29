/**
  ******************************************************************************
  * @file    module_slave_hal.hpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Interface STM32 hardware_hal STIMAV4 Header config
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

#include "config.h"
#include <Arduino.h>
#include <STM32FreeRTOS.h>

// /* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _MODULE_SLAVE_HAL_H
#define _MODULE_SLAVE_HAL_H

// CPUID STM32
#define UID_BASE_ADDRESS       (0x1FFF7590UL)

#if (ENABLE_I2C1 || ENABLE_I2C2)
#include <Wire.h>
#endif

#if (ENABLE_I2C2)
extern TwoWire Wire2;
#endif

#if (ENABLE_SERIAL2)
extern HardwareSerial Serial2;
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

// ******************************************************************************

// PIN NAMED STM32 ARDUINO GPIO_INIT

// CAN BUS
#define PIN_CAN_RX1     PB12
#define PIN_CAN_TX1     PB13
#define PIN_CAN_EN      PC12
#define PIN_CAN_STB     PB15

// POWER OUTPUT SENSOR OR DIGITAL OUT Protection & Controller
#define PIN_IN_ICTRL    PA4     // Power Controller Output Consumation
#define PIN_OUT_DEN     PC4     // Enable Output
#define PIN_OUT_DSEL0   PC5     // MUX Chanel 0 Power Consumation Controller
#define PIN_OUT_DSEL1   PC6     // MUX Chanel 1 Power Consumation Controller
#define PIN_OUT0        PC7     // Digital OUT 0
#define PIN_OUT1        PC8     // Digital OUT 1
#define PIN_OUT2        PC9     // Digital OUT 2
#define PIN_OUT3        PC10    // Digital OUT 3

// INPUT DIGITAL
#define PIN_IN0         PB2
#define PIN_IN1         PA8
#define PIN_IN2         PB8

// I2C
#define PIN_I2C2_EN     PC11
#define PIN_I2C2_SDA    PB14
#define PIN_I2C2_SCL    PB10

#define PIN_I2C1_SDA    PB7
#define PIN_I2C1_SCL    PB6

// PIN ENABLE SUPPLY
#define PIN_EN_SPLY     PD2
#define PIN_EN_5VS      PB5
#define PIN_EN_5VA      PB9
#define PIN_FAULT_SPLY  PB4

// PIN SPI
#define PIN_SPI_SCK     PA5
#define PIN_SPI_MOSI    PA12
#define PIN_SPI_MISO    PA11

#define PIN_SYS_JTMS    PA13

// PIN UART1 (Monitor Debugger)
#define PIN_USART1_TX   PA10
#define PIN_USART1_RX   PA9

// PIN UART2
#define PIN_USART2_TX   PA2
#define PIN_USART2_RX   PA15
#define PIN_USART2_CTS  PA0
#define PIN_USART2_RTS  PA1

// PIN ANALOG
#define PIN_ANALOG_01   PC0
#define PIN_ANALOG_02   PC1
#define PIN_ANALOG_03   PC2
#define PIN_ANALOG_04   PC3
#define PIN_ANALOG_09   PA4     // Power Controller Consumation

#if (ENABLE_DIAG_PIN)
// DIAG PIN (LED + BUTTON COME TEST NUCLEO)
// Commentare per escludere la funzionalità
#define HFLT_PIN  PIN_OUT1  // N.C. in Module_Power -> Output Signal Fault_Handler
#define LED1_PIN  PIN_OUT2  // LED 1 Nucleo Simulator
#define LED2_PIN  PIN_OUT3  // LED 2 Nucleo Simulator
#define USER_INP  PIN_IN2   // BTN_I Nucleo Simulator
#endif

// ******************************************************************************

// PIN NAMED STM32CUBE FOR GPIO_INIT

#define DEN_Pin           GPIO_PIN_4
#define DEN_GPIO_Port     GPIOC
#define DSEL0_Pin         GPIO_PIN_5
#define DSEL0_GPIO_Port   GPIOC
#define IN0_Pin           GPIO_PIN_2
#define IN0_GPIO_Port     GPIOB
#define STB_CAN_Pin       GPIO_PIN_15
#define STB_CAN_GPIO_Port GPIOB
#define DSEL1_Pin         GPIO_PIN_6
#define DSEL1_GPIO_Port   GPIOC
#define PW0_Pin           GPIO_PIN_7
#define PW0_GPIO_Port     GPIOC
#define PW1_Pin           GPIO_PIN_8
#define PW1_GPIO_Port     GPIOC
#define PW2_Pin           GPIO_PIN_9
#define PW2_GPIO_Port     GPIOC
#define IN1_Pin           GPIO_PIN_8
#define IN1_GPIO_Port     GPIOA
#define PW3_Pin           GPIO_PIN_10
#define PW3_GPIO_Port     GPIOC
#define I2C2_EN_Pin       GPIO_PIN_11
#define I2C2_EN_GPIO_Port GPIOC
#define EN_CAN_Pin        GPIO_PIN_12
#define EN_CAN_GPIO_Port  GPIOC
#define EN_SPLY_Pin       GPIO_PIN_2
#define EN_SPLY_GPIO_Port GPIOD
#define FAULT_Pin         GPIO_PIN_4
#define FAULT_GPIO_Port   GPIOB
#define EN_5VS_Pin        GPIO_PIN_5
#define EN_5VS_GPIO_Port  GPIOB
#define IN2_Pin           GPIO_PIN_8
#define IN2_GPIO_Port     GPIOB
#define EN_5VA_Pin        GPIO_PIN_9
#define EN_5VA_GPIO_Port  GPIOB

// ******************************************************************************

/* Private Hardware_Handler istance initialization ---------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

// Basic Function Hardware
void SystemClock_Config(void);
void SetupSystemPeripheral(void);
void HAL_MspInit(void);
void MX_GPIO_Init(void);

void STM32L4GetCPUID(uint8_t *ptrCpuId);
uint64_t StimaV4GetSerialNumber(void);

#if (ENABLE_CAN)
void MX_CAN1_Init(void);
void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan);
void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan);
#endif

#if (ENABLE_QSPI)
void MX_QUADSPI_Init(void);
void HAL_QSPI_MspInit(QSPI_HandleTypeDef* hqspi);
void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* hqspi);
#endif

#ifdef __cplusplus
}
#endif

#endif // __MODULE_SLAVE_HAL_H