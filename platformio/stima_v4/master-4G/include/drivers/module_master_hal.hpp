/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    module_master_hal.hpp
  * @brief   module_master hal configuration
  ******************************************************************************
  * @attention
  *
  * This software is distributed under the terms of the MIT License.
  * Progetto RMAP - STIMA V4
  * Hardware Config, STIMAV4 MASTER Board - Rev.1.00
  * Copyright (C) 2022 Digiteco s.r.l.
  * Author: Gasperini Moreno <m.gasperini@digiteco.it>
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "config.h"
#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include "STM32FreeRTOSConfig_extra.h"
// #include "task.h"

// /* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MODULE_MASTER_HAL_H
#define __MODULE_MASTER_HAL_H

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
#endif

// // Automatic module MSP_Weak Init & DeInit
// #define _HW_MSP_AUTO_PRIVATE

// // INIT HW PRIVATE BOARD/ISTANCE CFG
// #define _HW_SETUP_GPIO_PRIVATE
// #define _HW_SETUP_CAN_PRIVATE
// #define _HW_SETUP_CRC_PRIVATE
// #define _HW_SETUP_I2C1_PRIVATE
// #define _HW_SETUP_I2C2_PRIVATE
// #define _HW_SETUP_LPTIM_PRIVATE
// #define _HW_SETUP_QSPI_PRIVATE
// #define _HW_SETUP_RNG_PRIVATE
// #define _HW_SETUP_RTC_PRIVATE
// #define _HW_SETUP_SD_PRIVATE
// #define _HW_SETUP_SPI_PRIVATE
// #define _HW_SETUP_TIM3_PRIVATE
// // #define _HW_SETUP_UART1_PRIVATE
// #define _HW_SETUP_UART2_PRIVATE
// #define _HW_SETUP_UART4_PRIVATE

// // MSP INIT AND DEINIT PRIVATE WEAK FUNCTION
// #ifndef _HW_MSP_AUTO_PRIVATE
//   // MSP MANUAL SELECT INIT AND DEINIT PRIVATE WEAK FUNCTION
//   #define _HW_MSP_CAN_PRIVATE
//   #define _HW_MSP_CRC_PRIVATE
//   #define _HW_MSP_I2C1_PRIVATE
//   #define _HW_MSP_I2C2_PRIVATE
//   #define _HW_MSP_LPTIM_PRIVATE
  // #define _HW_MSP_QSPI_PRIVATE
//   #define _HW_MSP_RNG_PRIVATE
//   #define _HW_MSP_RTC_PRIVATE
//   #define _HW_MSP_SD_PRIVATE
//   #define _HW_MSP_SPI_PRIVATE
//   #define _HW_MSP_TIM3_PRIVATE
//   #define _HW_MSP_UART1_PRIVATE
//   #define _HW_MSP_UART2_PRIVATE
//   #define _HW_MSP_UART4_PRIVATE
// #else
//   // MSP AUTOMATIC INIT AND DEINIT PRIVATE WEAK FUNCTION
//   #ifdef _HW_SETUP_CAN_PRIVATE
//     #define _HW_MSP_CAN_PRIVATE
//   #endif
//   #ifdef _HW_SETUP_CRC_PRIVATE
//     #define _HW_MSP_CRC_PRIVATE
//   #endif
//   #ifdef _HW_SETUP_I2C1_PRIVATE
//     #define _HW_MSP_I2C1_PRIVATE
//   #endif
//   #ifdef _HW_SETUP_I2C2_PRIVATE
//     #define _HW_MSP_I2C2_PRIVATE
//   #endif
//   #ifdef _HW_SETUP_LPTIM_PRIVATE
//     #define _HW_MSP_LPTIM_PRIVATE
//   #endif
  // #ifdef _HW_SETUP_QSPI_PRIVATE
  //   #define _HW_MSP_QSPI_PRIVATE
  // #endif
//   #ifdef _HW_SETUP_RNG_PRIVATE
//     #define _HW_MSP_RNG_PRIVATE
//   #endif
//   #ifdef _HW_SETUP_RTC_PRIVATE
//     #define _HW_MSP_RTC_PRIVATE
//   #endif
//   #ifdef _HW_SETUP_SD_PRIVATE
//     #define _HW_MSP_SD_PRIVATE
//   #endif
//   #ifdef _HW_SETUP_SPI_PRIVATE
//     #define _HW_MSP_SPI_PRIVATE
//   #endif
//   #ifdef _HW_SETUP_TIM3_PRIVATE
//     #define _HW_MSP_TIM3_PRIVATE
//   #endif
//   #ifdef _HW_SETUP_UART1_PRIVATE
//     #define _HW_MSP_UART1_PRIVATE
//   #endif
//   #ifdef _HW_SETUP_UART2_PRIVATE
//     #define _HW_MSP_UART2_PRIVATE
//   #endif
//   #ifdef _HW_SETUP_UART4_PRIVATE
//     #define _HW_MSP_UART4_PRIVATE
//   #endif
// #endif

// // ******************************************************************************

// // PIN NAMED STM32 ARDUINO GPIO_INIT

// SYSTEM
#define PIN_SYS_SWCLC   PA14
#define PIN_SYS_TRCSW   PB3
#define PIN_SYS_SWDIO   PA13
#define PIN_SYS_WKUP3   PE6

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
#define PIN_SPI_MISO    PB6

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
#define PIN_7600E_RI    PIN_UP27_PD2

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

// *****************************

// PIN NAMED STM32CUBE GPIO_INIT

#define ENCODER_A_Pin LL_GPIO_PIN_3
#define ENCODER_A_GPIO_Port GPIOE
#define ENCODER_B_Pin LL_GPIO_PIN_4
#define ENCODER_B_GPIO_Port GPIOE

// // ******************************************************************************

// // Data Istance and Prototype Function Extern "C"

// #ifdef _HW_SETUP_CAN_PRIVATE
// extern CAN_HandleTypeDef hcan1;
// #endif
// #ifdef _HW_SETUP_CRC_PRIVATE
// extern CRC_HandleTypeDef hcrc;
// #endif
// #ifdef _HW_SETUP_I2C1_PRIVATE
// extern I2C_HandleTypeDef hi2c1;
// #endif
// #ifdef _HW_SETUP_I2C2_PRIVATE
// extern I2C_HandleTypeDef hi2c2;
// #endif
// #ifdef _HW_SETUP_LPTIM_PRIVATE
// extern LPTIM_HandleTypeDef hlptim1;
// #endif
// #ifdef _HW_SETUP_QSPI_PRIVATE
// extern QSPI_HandleTypeDef hqspi;
// #endif
// #ifdef _HW_SETUP_RNG_PRIVATE
// extern RNG_HandleTypeDef hrng;
// #endif
// #ifdef _HW_SETUP_RTC_PRIVATE
// extern RTC_HandleTypeDef hrtc;
// #endif
// #ifdef _HW_SETUP_SD_PRIVATE
// extern SD_HandleTypeDef hsd1;
// #endif
// #ifdef _HW_SETUP_SPI_PRIVATE
// extern SPI_HandleTypeDef hspi1;
// #endif
// #ifdef _HW_SETUP_TIM3_PRIVATE
// extern TIM_HandleTypeDef htim3;
// #endif
// #ifdef _HW_SETUP_UART1_PRIVATE
// extern UART_HandleTypeDef huart1;
// #endif
// #ifdef _HW_SETUP_UART2_PRIVATE
// extern UART_HandleTypeDef huart2;
// #endif
// #ifdef _HW_SETUP_UART4_PRIVATE
// extern UART_HandleTypeDef huart4;
// #endif

#ifdef __cplusplus
extern "C" {
#endif

void SystemClock_Config(void);
void SetupSystemPeripheral(void);
// void HAL_MspInit(void);

// #ifdef _HW_SETUP_GPIO_PRIVATE
// void MX_GPIO_Init(void);
// #endif

// #ifdef _HW_SETUP_CAN_PRIVATE
// void MX_CAN1_Init(void);
// #endif
// #ifdef _HW_MSP_CAN_PRIVATE
// void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan);
// void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan);
// #endif

// #ifdef _HW_SETUP_CRC_PRIVATE
// void MX_CRC_Init(void);
// #endif
// #ifdef _HW_MSP_CRC_PRIVATE
// void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc);
// void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc);
// #endif

// #ifdef _HW_SETUP_I2C1_PRIVATE
// void MX_I2C1_Init(void);
// #endif
// #ifdef _HW_SETUP_I2C2_PRIVATE
// void MX_I2C2_Init(void);
// #endif
// #if defined(_HW_MSP_I2C1_PRIVATE) || defined(_HW_MSP_I2C2_PRIVATE)
// void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c);
// void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c);
// #endif

// #ifdef _HW_SETUP_LPTIM_PRIVATE
// void MX_LPTIM1_Init(void);
// #endif
// #ifdef _HW_MSP_LPTIM_PRIVATE
// void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef* hlptim);
// void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef* hlptim);
// #endif

#if (ENABLE_QSPI)
void MX_QUADSPI_Init(void);
void HAL_QSPI_MspInit(QSPI_HandleTypeDef* hqspi);
void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* hqspi);
#endif

// #ifdef _HW_SETUP_RNG_PRIVATE
// void MX_RNG_Init(void);
// #endif
// #ifdef _HW_MSP_RNG_PRIVATE
// void HAL_RNG_MspInit(RNG_HandleTypeDef* hrng);
// void HAL_RNG_MspDeInit(RNG_HandleTypeDef* hrng);
// #endif

// #ifdef _HW_SETUP_RTC_PRIVATE
// void MX_RTC_Init(void);
// #endif
// #ifdef _HW_MSP_RTC_PRIVATE
// void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc);
// void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc);
// #endif

// #ifdef _HW_SETUP_SD_PRIVATE
// void MX_SDMMC1_SD_Init(void);
// #endif
// #ifdef _HW_MSP_SD_PRIVATE
// void HAL_SD_MspInit(SD_HandleTypeDef* hsd);
// void HAL_SD_MspDeInit(SD_HandleTypeDef* hsd);
// #endif

// #ifdef _HW_SETUP_SPI_PRIVATE
// void MX_SPI1_Init(void);
// #endif
// #ifdef _HW_MSP_SPI_PRIVATE
// void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi);
// void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi);
// #endif

// #ifdef _HW_SETUP_TIM3_PRIVATE
// void MX_TIM3_Init(void);
// #endif
// #ifdef _HW_MSP_TIM3_PRIVATE
// void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef* htim_encoder);
// void HAL_TIM_Encoder_MspDeInit(TIM_HandleTypeDef* htim_encoder);
// #endif

// #ifdef _HW_SETUP_UART1_PRIVATE
// void MX_USART1_UART_Init(void);
// #endif
// #ifdef _HW_SETUP_UART2_PRIVATE
// void MX_USART2_UART_Init(void);
// #endif
// #ifdef _HW_SETUP_UART4_PRIVATE
// void MX_UART4_Init(void);
// #endif
// #if defined(_HW_SETUP_UART1_PRIVATE) || defined(_HW_SETUP_UART2_PRIVATE) || defined(_HW_SETUP_UART4_PRIVATE)
// void HAL_UART_MspInit(UART_HandleTypeDef* huart);
// void HAL_UART_MspDeInit(UART_HandleTypeDef* huart);
// #endif

#ifdef __cplusplus
}
#endif

#endif // __MODULE_MASTER_HAL_H