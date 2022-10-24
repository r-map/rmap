/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32_assert.h
  * @brief   STM32 assert file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MODULE_MASTER_HAL_H
#define __MODULE_MASTER_HAL_H

// INIT HW PRIVATE BOARD/ISTANCE CFG
#define _HW_SETUP_CAN_PRIVATE
#define _HW_SETUP_CRC_PRIVATE
#define _HW_SETUP_I2C_PRIVATE
#define _HW_SETUP_LPTIM_PRIVATE
#define _HW_SETUP_QSPI_PRIVATE
#define _HW_SETUP_RNG_PRIVATE
#define _HW_SETUP_RTC_PRIVATE
#define _HW_SETUP_SD_PRIVATE
#define _HW_SETUP_SPI_PRIVATE
#define _HW_SETUP_TIM3_PRIVATE
#define _HW_SETUP_UART_PRIVATE

// PRIVATE PIN
#define ENCODER_A_Pin LL_GPIO_PIN_3
#define ENCODER_A_GPIO_Port GPIOE
#define ENCODER_B_Pin LL_GPIO_PIN_4
#define ENCODER_B_GPIO_Port GPIOE

// Data Istance and Prototype Function Extern "C"

#ifdef _HW_SETUP_CAN_PRIVATE
extern CAN_HandleTypeDef hcan1;
#endif

#ifdef _HW_SETUP_CRC_PRIVATE
extern CRC_HandleTypeDef hcrc;
#endif

#ifdef _HW_SETUP_I2C_PRIVATE
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
#endif

#ifdef _HW_SETUP_LPTIM_PRIVATE
extern LPTIM_HandleTypeDef hlptim1;
extern LPTIM_HandleTypeDef hlptim2;
#endif

#ifdef _HW_SETUP_QSPI_PRIVATE
extern QSPI_HandleTypeDef hqspi;
#endif

#ifdef _HW_SETUP_RNG_PRIVATE
extern RNG_HandleTypeDef hrng;
#endif

#ifdef _HW_SETUP_RTC_PRIVATE
extern RTC_HandleTypeDef hrtc;
#endif

#ifdef _HW_SETUP_SD_PRIVATE
extern SD_HandleTypeDef hsd1;
#endif

#ifdef _HW_SETUP_SPI_PRIVATE
extern SPI_HandleTypeDef hspi1;
#endif

#ifdef _HW_SETUP_TIM3_PRIVATE
extern TIM_HandleTypeDef htim3;
#endif

#ifdef _HW_SETUP_UART_PRIVATE
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
#endif

#ifdef __cplusplus
 extern "C" {
#endif

void SystemClock_Config(void);
void MX_GPIO_Init(void);

#ifdef _HW_SETUP_CAN_PRIVATE
void MX_CAN1_Init(void);
#endif

#ifdef _HW_SETUP_CRC_PRIVATE
void MX_CRC_Init(void);
#endif

#ifdef _HW_SETUP_I2C_PRIVATE
void MX_I2C1_Init(void);
void MX_I2C2_Init(void);
#endif

#ifdef _HW_SETUP_LPTIM_PRIVATE
void MX_LPTIM1_Init(void);
void MX_LPTIM2_Init(void);
#endif

#ifdef _HW_SETUP_QSPI_PRIVATE
void MX_QUADSPI_Init(void);
#endif

#ifdef _HW_SETUP_RNG_PRIVATE
void MX_RNG_Init(void);
#endif

#ifdef _HW_SETUP_RTC_PRIVATE
void MX_RTC_Init(void);
#endif

#ifdef _HW_SETUP_SD_PRIVATE
void MX_SDMMC1_SD_Init(void);
#endif

#ifdef _HW_SETUP_SPI_PRIVATE
void MX_SPI1_Init(void);
#endif

#ifdef _HW_SETUP_TIM3_PRIVATE
void MX_TIM3_Init(void);
#endif

#ifdef _HW_SETUP_UART_PRIVATE
void MX_UART4_Init(void);
void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
#endif

#ifdef __cplusplus
 }
#endif

#endif // __MODULE_MASTER_HAL_H