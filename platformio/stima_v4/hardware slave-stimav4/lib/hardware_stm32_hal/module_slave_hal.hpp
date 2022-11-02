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
#ifndef __MODULE_SLAVE_HAL_H
#define __MODULE_SLAVE_HAL_H

// INIT HW PRIVATE BOARD/ISTANCE CFG

#define _HW_SETUP_GPIO_PRIVATE
#define _HW_SETUP_ADC_PRIVATE
#define _HW_SETUP_CAN_PRIVATE
#define _HW_SETUP_CRC_PRIVATE
#define _HW_SETUP_I2C_PRIVATE
#define _HW_SETUP_LPTIM_PRIVATE
#define _HW_SETUP_QSPI_PRIVATE
#define _HW_SETUP_RTC_PRIVATE
#define _HW_SETUP_SPI_PRIVATE
#define _HW_SETUP_UART_PRIVATE

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

#ifdef _HW_SETUP_I2C_PRIVATE
extern I2C_HandleTypeDef hi2c1;
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

#ifdef _HW_SETUP_UART_PRIVATE
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
#endif
/* Private Hardware_Handler istance initialization ---------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

void SystemClock_Config(void);
void HAL_MspInit(void);

#ifdef _HW_SETUP_GPIO_PRIVATE
void MX_GPIO_Init(void);
#endif

#ifdef _HW_SETUP_ADC_PRIVATE
void MX_ADC1_Init(void);
void HAL_ADC_MspInit_Private(ADC_HandleTypeDef* hadc);
void HAL_ADC_MspDeInit_Private(ADC_HandleTypeDef* hadc);
#endif

#ifdef _HW_SETUP_CAN_PRIVATE
void MX_CAN1_Init(void);
void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan);
void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan);
#endif

#ifdef _HW_SETUP_CRC_PRIVATE
void MX_CRC_Init(void);
void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc);
void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc);
#endif

#ifdef _HW_SETUP_I2C_PRIVATE
void MX_I2C1_Init(void);
void MX_I2C2_Init(void);
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c);
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c);
#endif

#ifdef _HW_SETUP_LPTIM_PRIVATE
void MX_LPTIM1_Init(void);
void MX_LPTIM2_Init(void);
void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef* hlptim);
void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef* hlptim);
#endif

#ifdef _HW_SETUP_QSPI_PRIVATE
void MX_QUADSPI_Init(void);
void HAL_QSPI_MspInit(QSPI_HandleTypeDef* hqspi);
void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* hqspi);
#endif

#ifdef _HW_SETUP_RTC_PRIVATE
void MX_RTC_Init(void);
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc);
void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc);
#endif

#ifdef _HW_SETUP_SPI_PRIVATE
void MX_SPI1_Init(void);
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi);
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi);
#endif

#ifdef _HW_SETUP_UART_PRIVATE
void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
void HAL_UART_MspInit(UART_HandleTypeDef* huart);
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart);
#endif

#ifdef __cplusplus
}
#endif

#endif // __MODULE_SLAVE_HAL_H