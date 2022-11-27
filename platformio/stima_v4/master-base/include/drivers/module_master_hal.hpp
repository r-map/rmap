/**
  ******************************************************************************
  * @file    module_master_hal.hpp
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
#ifndef __MODULE_MASTER_HAL_H
#define __MODULE_MASTER_HAL_H

#define USE_HAL_DRIVER        (true)

// HW device
#define ENABLE_I2C1           (true)
#define ENABLE_I2C2           (true)
#define ENABLE_QSPI           (true)
#define ENABLE_CAN            (false)
#define ENABLE_SIM7600E       (true)

// HW Diag PIN redefine
#define ENABLE_DIAG_PIN       (true)

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
// #define _HW_MSP_AUTO_PRIVATE

// INIT HW PRIVATE BOARD/ISTANCE CFG

// #define _HW_SETUP_CAN_PRIVATE
// #define _HW_SETUP_CRC_PRIVATE
// #define _HW_SETUP_LPTIM_PRIVATE
// #define _HW_SETUP_RNG_PRIVATE
// #define _HW_SETUP_SD_PRIVATE
// #define _HW_SETUP_UART2_PRIVATE

#if (ENABLE_SIM7600E)
  // SIM7600 Using local driver UART2
  #undef _HW_SETUP_UART2_PRIVATE
#endif
// ******************************************************************************

// PIN NAMED STM32 ARDUINO GPIO_INIT

// SYSTEM
#define PIN_SYS_SWCLC   PA14
#define PIN_SYS_TRCSW   PB3
#define PIN_SYS_SWDIO   PA13

// CAN BUS
#define PIN_CAN_RX1     PD0
#define PIN_CAN_TX1     PD1
#define PIN_CAN_EN      PE14
#define PIN_CAN_STB     PE15

// SDCARD (SD_DETECT_PIN -> HAL_STM32)
#define PIN_MMC1_DTC    PC7
#define PIN_MMC1_D0     PC8
#define PIN_MMC1_D1     PC9
#define PIN_MMC1_D2     PC10
#define PIN_MMC1_D3     PC11
#define PIN_MMC1_CLK    PC12
#define PIN_MMC1_CMD    PD2

// BUZZER
#define PIN_BUZZER      PE9

// ENCODER
#define PIN_ENCODER_A   PE3
#define PIN_ENCODER_B   PE4
#define PIN_ENCODER_EN5 PE5
#define PIN_ENCODER_INT PE6

// SPI1
#define PIN_SPI_SCK     PA5
#define PIN_SPI_MOSI    PB5
#define PIN_SPI_MISO    PB4

// I2C1 Esterna (Upin 27 A4/A5)
#define PIN_I2C1_SDA    PB7
#define PIN_I2C1_SCL    PB8

// I2C2 Interna (Dsp, Eeprom)
#define PIN_I2C2_SDA    PB14
#define PIN_I2C2_SCL    PB10

// UPIN27
#define PIN_UP27_PC1    PC1
#define PIN_UP27_PC3    PC3
#define PIN_UP27_PC4    PC4
#define PIN_UP27_PD10   PA15
#define PIN_UP27_PD0    PIN_UART2_RX
#define PIN_UP27_PD1    PIN_UART2_TX
// #define PIN_UP27_PD2    PIN_UART1_RX
// #define PIN_UP27_PD3    PIN_UART1_TX
#define PIN_UP27_PD2    PA10
#define PIN_UP27_PD3    PB6
#define PIN_UP27_PD4    PD11
#define PIN_UP27_PD5    PD12
#define PIN_UP27_PD6    PD13
#define PIN_UP27_PD8    PD14
#define PIN_UP27_PD9    PD15

// SIM7600E SU UPIN 27
#define PIN_GSM_PW_KEY  PIN_UP27_PD5
#define PIN_GSM_EN_POW  PIN_UP27_PD4
#define PIN_GSM_RX0     PIN_UP27_PD0    // PIN_UART2_RX
#define PIN_GSM_TX0     PIN_UP27_PD1    // PIN_UART2_TX
#define PIN_GSM_RI      PIN_UP27_PD2

// CLOCK
#define PIN_SWDIO       PA13
#define PIN_SWCLK       PA14

// UART2
#define PIN_UART2_TX    PD5
#define PIN_UART2_RX    PD6
#define PIN_UART2_CTS   PD3
#define PIN_UART2_RTS   PD4

// // UART1
// #define PIN_UART1_TX    PB6
// #define PIN_UART1_RX    PA10

// UART4
#define PIN_UART4_TX    PA0
#define PIN_UART4_RX    PA1

// USB
#define PIN_USB_DM      PA11
#define PIN_USB_DP      PA12

// QSPI FLASH
#define PIN_QSPIF_SCK   PA3
#define PIN_QSPIF_CS    PA2
#define PIN_QSPIF_IO_0  PB1
#define PIN_QSPIF_IO_1  PB0
#define PIN_QSPIF_IO_2  PA7
#define PIN_QSPIF_IO_3  PA6

// RCC
#define PIN_RCC_MCO     PA8
#define PIN_RCC_OSC_IN  PC14
#define PIN_RCC_OSC_OUT PC15

// POWER
#define PIN_USB_POWER   PA9
#define PIN_DSP_POWER   PB11

#if (ENABLE_DIAG_PIN)
// DIAG PIN (LED + BUTTON COME TEST NUCLEO)
// Commentare per escludere la funzionalità
#define HFLT_PIN  PIN_BUZZER        // N.C. in Module_Power -> Output Signal Fault_Handler
// #define LED1_PIN  PIN_OUT2          // LED 1 Nucleo Simulator
// #define LED2_PIN  PIN_OUT3          // LED 2 Nucleo Simulator
#define USER_INP  PIN_ENCODER_INT   // BTN_I Nucleo Simulator
#endif

// *****************************

// PIN NAMED STM32CUBE GPIO_INIT

#define DISPLAY_Power_Pin       GPIO_PIN_11
#define DISPLAY_Power_GPIO_Port GPIOB

#define ENCODER_A_Pin     GPIO_PIN_3
#define ENCODER_B_Pin     GPIO_PIN_4
#define ENCODER_Power_Pin GPIO_PIN_5
#define ENCODER_Ent_Pin   GPIO_PIN_6
#define BUZZER_Power_Pin  GPIO_PIN_9
#define CAN_Enable_Pin    GPIO_PIN_14
#define CAN_StanbdBy_Pin  GPIO_PIN_15
#define ENCODER_GPIO_Port GPIOE
#define BUZZER_GPIO_Port  GPIOE
#define CAN_GPIO_Port     GPIOE

#define MMC1_Detect_Pin   GPIO_PIN_7
#define MMC1_GPIO_Port    GPIOC

#define GSM_RingInd_Pin   GPIO_PIN_2
#define GSM_PowerEn_Pin   GPIO_PIN_4
#define GSM_PowerKey_Pin  GPIO_PIN_5
#define GSM_GPIO_Port     GPIOD

// ******************************************************************************

// Data Istance and Prototype Function Extern "C"

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
extern CRC_HandleTypeDef hcrc;
#endif
#ifdef _HW_SETUP_LPTIM_PRIVATE
extern LPTIM_HandleTypeDef hlptim1;
#endif
#ifdef _HW_SETUP_RNG_PRIVATE
extern RNG_HandleTypeDef hrng;
#endif
#ifdef _HW_SETUP_SD_PRIVATE
extern SD_HandleTypeDef hsd1;
#endif
#ifdef _HW_SETUP_UART2_PRIVATE
extern UART_HandleTypeDef huart2;
#endif

#ifdef __cplusplus
extern "C" {
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

#ifdef _HW_SETUP_SD_PRIVATE
void MX_SDMMC1_SD_Init(void);
#endif
#ifdef _HW_MSP_SD_PRIVATE
void HAL_SD_MspInit(SD_HandleTypeDef* hsd);
void HAL_SD_MspDeInit(SD_HandleTypeDef* hsd);
#endif

#ifdef _HW_SETUP_UART2_PRIVATE
void MX_USART2_UART_Init(void);
#endif
#if defined(_HW_SETUP_UART2_PRIVATE)
void HAL_UART_MspInit(UART_HandleTypeDef* huart);
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart);
#endif

#ifdef __cplusplus
}
#endif

#endif // __MODULE_MASTER_HAL_H