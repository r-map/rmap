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
#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include "STM32FreeRTOSConfig_extra.h"
// #include "task.h"

// /* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MODULE_SLAVE_HAL_H
#define __MODULE_SLAVE_HAL_H

#define ENABLE_I2C1           (true)
#define ENABLE_I2C2           (false)

#if (ENABLE_I2C1)
#define I2C1_BUS_CLOCK_HZ     (100000L)
#endif

#if (ENABLE_I2C2)
#define I2C2_BUS_CLOCK_HZ     (100000L)
#endif

#if (ENABLE_I2C1 || ENABLE_I2C2)
#include <Wire.h>
#define I2C_MAX_DATA_LENGTH   (32)
#define I2C_MAX_ERROR_COUNT   (3)
#endif

// Automatic module MSP_Weak Init & DeInit
#define _HW_MSP_AUTO_PRIVATE

// INIT HW PRIVATE BOARD/ISTANCE CFG
// #define _HW_SETUP_GPIO_PRIVATE
// #define _HW_SETUP_ADC_PRIVATE
// #define _HW_SETUP_CAN_PRIVATE
// #define _HW_SETUP_CRC_PRIVATE
// #define _HW_SETUP_I2C1_PRIVATE
// #define _HW_SETUP_I2C2_PRIVATE
// #define _HW_SETUP_LPTIM_PRIVATE
#define _HW_SETUP_QSPI_PRIVATE
// #define _HW_SETUP_RTC_PRIVATE
// #define _HW_SETUP_SPI_PRIVATE
// #define _HW_SETUP_UART1_PRIVATE
// #define _HW_SETUP_UART2_PRIVATE

// MSP INIT AND DEINIT PRIVATE WEAK FUNCTION
#ifndef _HW_MSP_AUTO_PRIVATE
  // MSP MANUAL SELECT INIT AND DEINIT PRIVATE WEAK FUNCTION
  #define _HW_MSP_ADC_PRIVATE
  #define _HW_MSP_CAN_PRIVATE
  #define _HW_MSP_CRC_PRIVATE
  #define _HW_MSP_I2C1_PRIVATE
  #define _HW_MSP_I2C2_PRIVATE
  #define _HW_MSP_LPTIM_PRIVATE
  #define _HW_MSP_QSPI_PRIVATE
  #define _HW_MSP_RTC_PRIVATE
  #define _HW_MSP_SPI_PRIVATE
  #define _HW_MSP_UART1_PRIVATE
  #define _HW_MSP_UART2_PRIVATE
#else
  // MSP AUTOMATIC INIT AND DEINIT PRIVATE WEAK FUNCTION
  #ifdef _HW_SETUP_ADC_PRIVATE
    #define _HW_MSP_ADC_PRIVATE
  #endif
  #ifdef _HW_SETUP_CAN_PRIVATE
    #define _HW_MSP_CAN_PRIVATE
  #endif
  #ifdef _HW_SETUP_CRC_PRIVATE
    #define _HW_MSP_CRC_PRIVATE
  #endif
  #ifdef _HW_SETUP_I2C1_PRIVATE
    #define _HW_MSP_I2C1_PRIVATE
  #endif
  #ifdef _HW_SETUP_I2C2_PRIVATE
    #define _HW_MSP_I2C2_PRIVATE
  #endif
  #ifdef _HW_SETUP_LPTIM_PRIVATE
    #define _HW_MSP_LPTIM_PRIVATE
  #endif
  #ifdef _HW_SETUP_QSPI_PRIVATE
    #define _HW_MSP_QSPI_PRIVATE
  #endif
  #ifdef _HW_SETUP_RTC_PRIVATE
    #define _HW_MSP_RTC_PRIVATE
  #endif
  #ifdef _HW_SETUP_SPI_PRIVATE
    #define _HW_MSP_SPI_PRIVATE
  #endif
  #ifdef _HW_SETUP_UART1_PRIVATE
    #define _HW_MSP_UART1_PRIVATE
  #endif
  #ifdef _HW_SETUP_UART2_PRIVATE
    #define _HW_MSP_UART2_PRIVATE
  #endif
#endif

// ******************************************************************************

// PIN NAMED STM32 ARDUINO GPIO_INIT

// CAN BUS
#define PIN_CAN_RX1     PB12
#define PIN_CAN_TX1     PB13
#define PIN_CAN_EN      PC12
#define PIN_CAN_STB     PB15

#define PIN_DEN         PC4
#define PIN_DSEL0       PC5
#define PIN_DSEL1       PC6

// POWER SENSOR
#define PIN_PW0         PC7
#define PIN_PW1         PC8
#define PIN_PW2         PC9
#define PIN_PW3         PC10

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

// PIN FAULT
#define PIN_FAULT       PB4

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
#define PIN_ANALOG_09   PA4

// HARD_FAULT_SIGNAL
#define PIN_SIG_FAULT   PIN_PW3

// *****************************

// PIN NAMED STM32CUBE GPIO_INIT

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
#ifdef _HW_SETUP_I2C1_PRIVATE
extern I2C_HandleTypeDef hi2c1;
#endif
#ifdef _HW_SETUP_I2C2_PRIVATE
extern I2C_HandleTypeDef hi2c2;
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
#ifdef _HW_SETUP_UART1_PRIVATE
extern UART_HandleTypeDef huart1;
#endif
#ifdef _HW_SETUP_UART2_PRIVATE
extern UART_HandleTypeDef huart2;
#endif
/* Private Hardware_Handler istance initialization ---------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

void SystemClock_Config(void);
void SetupSystemPeripheral(void);
// void HAL_MspInit(void);

#ifdef _HW_SETUP_GPIO_PRIVATE
void MX_GPIO_Init(void);
#endif

#ifdef _HW_SETUP_ADC_PRIVATE
void MX_ADC1_Init(void);
#endif
#ifdef _HW_MSP_ADC_PRIVATE
void HAL_ADC_MspInit_Private(ADC_HandleTypeDef* hadc);
void HAL_ADC_MspDeInit_Private(ADC_HandleTypeDef* hadc);
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

#ifdef _HW_SETUP_I2C1_PRIVATE
void MX_I2C1_Init(void);
#endif
#ifdef _HW_SETUP_I2C2_PRIVATE
void MX_I2C2_Init(void);
#endif
#if defined(_HW_MSP_I2C1_PRIVATE) || defined(_HW_MSP_I2C2_PRIVATE)
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c);
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c);
#endif

#ifdef _HW_SETUP_LPTIM_PRIVATE
void MX_LPTIM1_Init(void);
#endif
#ifdef _HW_MSP_LPTIM_PRIVATE
void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef* hlptim);
void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef* hlptim);
#endif

#ifdef _HW_SETUP_QSPI_PRIVATE
void MX_QUADSPI_Init(void);
#endif
#ifdef _HW_MSP_QSPI_PRIVATE
void HAL_QSPI_MspInit(QSPI_HandleTypeDef* hqspi);
void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* hqspi);
#endif

#ifdef _HW_SETUP_RTC_PRIVATE
void MX_RTC_Init(void);
#endif
#ifdef _HW_MSP_RTC_PRIVATE
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc);
void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc);
#endif

#ifdef _HW_SETUP_SPI_PRIVATE
void MX_SPI1_Init(void);
#endif
#ifdef _HW_MSP_SPI_PRIVATE
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi);
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi);
#endif

#ifdef _HW_SETUP_UART1_PRIVATE
void MX_USART1_UART_Init(void);
#endif
#ifdef _HW_SETUP_UART2_PRIVATE
void MX_USART2_UART_Init(void);
#endif
#if defined(_HW_SETUP_UART1_PRIVATE) || defined(_HW_SETUP_UART2_PRIVATE)
void HAL_UART_MspInit(UART_HandleTypeDef* huart);
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart);
#endif

#ifdef _USE_FREERTOS_LOW_POWER
void xTaskSleepPrivate(TickType_t *xExpectedIdleTime);
void xTaskWakeUpPrivate(TickType_t xExpectedIdleTime);
#endif

#ifdef __cplusplus
}
#endif

#endif // __MODULE_MASTER_HAL_H