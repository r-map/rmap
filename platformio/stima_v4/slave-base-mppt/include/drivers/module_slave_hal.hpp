/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    module_slave_hal.hpp
  * @brief   module_slave hal configuration
  ******************************************************************************
  * @attention
  *
  * This software is distributed under the terms of the MIT License.
  * Progetto RMAP - STIMA V4
  * Hardware Config, STIMAV4 SLAVE Board - Rev.1.00
  * Copyright (C) 2022 Digiteco s.r.l.
  * Author: Gasperini Moreno <m.gasperini@digiteco.it>
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include "STM32FreeRTOSConfig_extra.h"
// #include "task.h"

// /* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MODULE_SLAVE_HAL_H
#define __MODULE_SLAVE_HAL_H

#define ENABLE_I2C1           (true)
#define ENABLE_I2C2           (false)
#define ENABLE_QSPI           (true)

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

// INIT HW PRIVATE BOARD/ISTANCE CFG

// #define _HW_SETUP_GPIO_PRIVATE
// #define _HW_SETUP_CAN_PRIVATE
// #define _HW_SETUP_CRC_PRIVATE
// #define _HW_SETUP_LPTIM_PRIVATE
#define _HW_SETUP_QSPI_PRIVATE

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

// DIAG PIN (LED + BUTTON COME TEST NUCLEO)
// Commentare per escludere la funzionalità
#define HFLT_PIN  PIN_OUT1  // N.C. in Module_Power -> Output Signal Fault_Handler
#define LED1_PIN  PIN_OUT2  // LED 1 Nucleo Simulator
#define LED2_PIN  PIN_OUT3  // LED 2 Nucleo Simulator
#define USER_INP  PIN_IN2   // BTN_I Nucleo Simulator

// *****************************

// PIN NAMED STM32CUBE GPIO_INIT
#define STB_CAN_Pin       GPIO_PIN_15
#define STB_CAN_GPIO_Port GPIOB
#define EN_CAN_Pin        GPIO_PIN_12
#define EN_CAN_GPIO_Port  GPIOC

// ******************************************************************************

// Data Istance and Prototype Function Extern "C"

#ifdef _HW_SETUP_ADC_PRIVATE
extern ADC_HandleTypeDef hadc1;
#endif

#ifdef _HW_SETUP_CAN_PRIVATE
extern CAN_HandleTypeDef hcan1;
#endif

#ifdef _HW_SETUP_CRC_PRIVATE
extern CRC_HandleTypeDef hcrc;
#endif

#ifdef _HW_SETUP_LPTIM_PRIVATE
extern LPTIM_HandleTypeDef hlptim1;
#endif

#ifdef _HW_SETUP_QSPI_PRIVATE
extern QSPI_HandleTypeDef hqspi;
#endif

#ifdef _HW_SETUP_RTC_PRIVATE
extern RTC_HandleTypeDef hrtc;
#endif

#ifdef _HW_SETUP_SPI_PRIVATE
extern SPI_HandleTypeDef hspi1;
#endif

/* Private Hardware_Handler istance initialization ---------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

void SystemClock_Config(void);
void SetupSystemPeripheral(void);
void HAL_MspInit(void);

#ifdef _HW_SETUP_GPIO_PRIVATE
void MX_GPIO_Init(void);
#endif

#ifdef _HW_SETUP_CAN_PRIVATE
void MX_CAN1_Init(void);
#endif
#ifdef _HW_MSP_CAN_PRIVATE
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

#ifdef _HW_SETUP_RTC_PRIVATE
void MX_RTC_Init(void);
#endif
#ifdef _HW_MSP_RTC_PRIVATE
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc);
void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc);
#endif

#ifdef _HW_SETUP_SD_PRIVATE
void MX_SDMMC1_SD_Init(void);
#endif
#ifdef _HW_MSP_SD_PRIVATE
void HAL_SD_MspInit(SD_HandleTypeDef* hsd);
void HAL_SD_MspDeInit(SD_HandleTypeDef* hsd);
#endif

#ifdef _HW_SETUP_SPI_PRIVATE
void MX_SPI1_Init(void);
#endif
#ifdef _HW_MSP_SPI_PRIVATE
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi);
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi);
#endif

#ifdef _HW_SETUP_TIM3_PRIVATE
void MX_TIM3_Init(void);
#endif
#ifdef _HW_MSP_TIM3_PRIVATE
void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef* htim_encoder);
void HAL_TIM_Encoder_MspDeInit(TIM_HandleTypeDef* htim_encoder);
#endif

#ifdef __cplusplus
}
#endif

#endif // __MODULE_SLAVE_HAL_H