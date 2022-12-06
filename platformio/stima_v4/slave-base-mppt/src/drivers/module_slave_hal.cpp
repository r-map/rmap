/**
  ******************************************************************************
  * @file    module_slave_hal.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Interface STM32 hardware_hal STIMAV4
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

#include "drivers/module_slave_hal.hpp"
#include "stm32l4xx_ll_system.h"

#if (ENABLE_I2C2)
TwoWire Wire2 = TwoWire(PIN_I2C2_SDA, PIN_I2C2_SCL);
#endif

// Non utilizzo FreRTOS LOW_Power per il Debugging
#ifdef _USE_FREERTOS_LOW_POWER
#define _EXIT_SLEEP_FOR_DEBUGGING
#endif

/* Private Hardware_Handler istance initialization ---------------------------------------*/
#if (ENABLE_CAN)
CAN_HandleTypeDef hcan1;
#endif
#ifdef _HW_SETUP_CRC_PRIVATE
CRC_HandleTypeDef hcrc;
#endif
#ifdef _HW_SETUP_LPTIM_PRIVATE
LPTIM_HandleTypeDef hlptim1;
#endif
#if (ENABLE_QSPI)
QSPI_HandleTypeDef hqspi;
#endif
#ifdef _HW_SETUP_RNG_PRIVATE
RNG_HandleTypeDef hrng;
#endif

// ********************************************************************************
//  LOCAL REDEFINITION Weak PinMap ARDUINO for SETUP PIN Base or Alternate Function
// ********************************************************************************

#ifdef HAL_ADC_MODULE_ENABLED
const PinMap PinMap_ADC[] = {
  {NC,   NP,   0}
};
#endif

#ifdef HAL_DAC_MODULE_ENABLED
const PinMap PinMap_DAC[] = {
  {NC,   NP,   0}
};
#endif

#ifdef HAL_I2C_MODULE_ENABLED
const PinMap PinMap_I2C_SDA[] = {
  {PB_7,       I2C1, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF4_I2C1)},
  {NC,         NP,   0}
};
const PinMap PinMap_I2C_SCL[] = {
  {PB_6,       I2C1, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF4_I2C1)},
  {NC,         NP,   0}
};
#endif

#ifdef HAL_TIM_MODULE_ENABLED
const PinMap PinMap_TIM[] = {
  {NC,         NP,    0}
};
#endif

#ifdef HAL_UART_MODULE_ENABLED
const PinMap PinMap_UART_TX[] = {
  {PA_9,       USART1,  STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF7_USART1)},
  {NC,         NP,      0}
};
const PinMap PinMap_UART_RX[] = {
  {PA_10,      USART1,  STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF7_USART1)},
  {NC,         NP,      0}
};
const PinMap PinMap_UART_RTS[] = {
  {NC,         NP,      0}
};
const PinMap PinMap_UART_CTS[] = {
  {NC,         NP,      0}
};
#endif

#ifdef HAL_SPI_MODULE_ENABLED
const PinMap PinMap_SPI_MOSI[] = {
  {NC,        NP,   0}
};
const PinMap PinMap_SPI_MISO[] = {
  {NC,        NP,   0}
};
const PinMap PinMap_SPI_SCLK[] = {
  {NC,        NP,   0}
};
const PinMap PinMap_SPI_SSEL[] = {
  {NC,         NP,   0}
};
#endif

#ifdef HAL_CAN_MODULE_ENABLED
const PinMap PinMap_CAN_RD[] = {
  {PB_12, CAN1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_CAN1)},
  {NC,    NP,   0}
};
const PinMap PinMap_CAN_TD[] = {
  {PB_13, CAN1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_CAN1)},
  {NC,    NP,   0}
};
#endif

#ifdef HAL_QSPI_MODULE_ENABLED
const PinMap PinMap_QUADSPI_DATA0[] = {
  {PB_1, QUADSPI, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF10_QUADSPI)}, // QUADSPI_BK1_IO0
  {NC,   NP,      0}
};
const PinMap PinMap_QUADSPI_DATA1[] = {
  {PB_0, QUADSPI, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF10_QUADSPI)}, // QUADSPI_BK1_IO1
  {NC,   NP,      0}
};
const PinMap PinMap_QUADSPI_DATA2[] = {
  {PA_7, QUADSPI, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF10_QUADSPI)}, // QUADSPI_BK1_IO2
  {NC,   NP,      0}
};
const PinMap PinMap_QUADSPI_DATA3[] = {
  {PA_6, QUADSPI, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF10_QUADSPI)}, // QUADSPI_BK1_IO3
  {NC,   NP,      0}
};
const PinMap PinMap_QUADSPI_SCLK[] = {
  {PA_3,  QUADSPI, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF10_QUADSPI)}, // QUADSPI_CLK
  {NC,    NP,      0}
};
const PinMap PinMap_QUADSPI_SSEL[] = {
  {PB_11, QUADSPI, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF10_QUADSPI)}, // QUADSPI_BK1_NCS
  {NC,    NP,      0}
};
#endif

#if defined(HAL_PCD_MODULE_ENABLED) || defined(HAL_HCD_MODULE_ENABLED)
const PinMap PinMap_USB[] = {
  {NC,    NP,  0}
};
#endif

#ifdef HAL_SD_MODULE_ENABLED
const PinMap PinMap_SD[] = {
  {NC,    NP,     0}
};
#endif

/* Private Hardware_Handler istance initialization ---------------------------------------*/

/*******************************************************************************************
********************************************************************************************
                   System clock and private setup PIN (VBat/Chg, PLLSynch)
********************************************************************************************
*******************************************************************************************/
/**
  * @brief System Clock Configuration
  * @retval None
  */
extern "C" void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_CRSInitTypeDef RCC_CRSInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();

  /** Enable the SYSCFG APB clock
  */
  __HAL_RCC_CRS_CLK_ENABLE();

  /** Configures CRS
  */
  RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
  RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_LSE;
  RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
  RCC_CRSInitStruct.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000,32768);
  RCC_CRSInitStruct.ErrorLimitValue = 34;
  RCC_CRSInitStruct.HSI48CalibrationValue = 32;

  HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
}

/// @brief Startup PeripheralConfig Local Board
/// @param  none
void SetupSystemPeripheral(void) {

  FLASH_OBProgramInitTypeDef OBInit;		// flash option bytes copy

	/* get flash option bytes */
	OBInit.WRPArea = OB_WRPAREA_BANK1_AREAA;
	OBInit.PCROPConfig = FLASH_BANK_1;
	HAL_FLASHEx_OBGetConfig(&OBInit);

	/* config flash option bytes */
	if ((OBInit.USERConfig & OB_IWDG_STOP_RUN) == OB_IWDG_STOP_RUN) {
		OBInit.OptionType = OPTIONBYTE_USER;
		OBInit.USERType = OB_USER_IWDG_STOP | OB_USER_IWDG_STDBY;
		OBInit.USERConfig = OB_IWDG_STOP_FREEZE | OB_IWDG_STDBY_FREEZE;

		HAL_FLASH_Unlock();
		HAL_FLASH_OB_Unlock();
		HAL_FLASHEx_OBProgram(&OBInit);
		HAL_FLASH_OB_Launch();
		HAL_FLASH_OB_Lock();
		HAL_FLASH_Lock();
	}

	/** Enable cycle counter (for debug) **/
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  MX_GPIO_Init();

  #if (ENABLE_CAN)
  MX_CAN1_Init();
  #endif
  #ifdef _HW_SETUP_CRC_PRIVATE
  MX_CRC_Init();
  #endif
  #if (ENABLE_QSPI)
  MX_QUADSPI_Init();
  #endif
  #ifdef _HW_SETUP_LPTIM_PRIVATE
  MX_LPTIM1_Init();
  #endif
  #ifdef _HW_SETUP_RNG_PRIVATE
  MX_RNG_Init();
  #endif
  
  /* Abilito la carica del supercap */
	HAL_PWREx_EnableBatteryCharging(PWR_BATTERY_CHARGING_RESISTOR_5);

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */


/*******************************************************************************************
********************************************************************************************
                   System base Hardware Istance and private Initialization
********************************************************************************************
*******************************************************************************************/
#if (ENABLE_CAN)
/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
extern "C" void MX_CAN1_Init(void)
{
  CAN_FilterTypeDef CAN_FilterInitStruct;

  // Default Speed Base CAN Setup module 1Mhz
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 8;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_8TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK) {
    Error_Handler();
  }

  // CAN filter basic initialization
  CAN_FilterInitStruct.FilterIdHigh = 0x0000;
  CAN_FilterInitStruct.FilterIdLow = 0x0000;
  CAN_FilterInitStruct.FilterMaskIdHigh = 0x0000;
  CAN_FilterInitStruct.FilterMaskIdLow = 0x0000;
  CAN_FilterInitStruct.FilterFIFOAssignment = CAN_RX_FIFO0;
  CAN_FilterInitStruct.FilterBank = 0;
  CAN_FilterInitStruct.FilterMode = CAN_FILTERMODE_IDMASK;
  CAN_FilterInitStruct.FilterScale = CAN_FILTERSCALE_32BIT;
  CAN_FilterInitStruct.FilterActivation = ENABLE;

  // Check error initalization CAN filter
  if (HAL_CAN_ConfigFilter(&hcan1, &CAN_FilterInitStruct) != HAL_OK) {
    Error_Handler();
  }
}
#endif

#ifdef _HW_SETUP_CRC_PRIVATE
/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
extern "C" void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}
#endif

#ifdef _HW_SETUP_LPTIM_PRIVATE
/**
  * @brief LPTIM1 Initialization Function
  * @param None
  * @retval None
  */
extern "C" void MX_LPTIM1_Init(void)
{

  /* USER CODE BEGIN LPTIM1_Init 0 */

  /* USER CODE END LPTIM1_Init 0 */

  /* USER CODE BEGIN LPTIM1_Init 1 */

  /* USER CODE END LPTIM1_Init 1 */
  hlptim1.Instance = LPTIM1;
  hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
  hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
  hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
  if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPTIM1_Init 2 */

  /* USER CODE END LPTIM1_Init 2 */

}
#endif

#if (ENABLE_QSPI)
/**
  * @brief QUADSPI Initialization Function
  * @param None
  * @retval None
  */
extern "C" void MX_QUADSPI_Init(void)
{
  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /* USER CODE END QUADSPI_Init 1 */
  /* QUADSPI parameter configuration*/
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 1;
  hqspi.Init.FifoThreshold = 4;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
  hqspi.Init.FlashSize = 20;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */

  /* USER CODE END QUADSPI_Init 2 */
}
#endif

#ifdef _HW_SETUP_RNG_PRIVATE
/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */
extern "C" void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */

}
#endif

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
extern "C" void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pins : LTC_SMB_ALERT_Pin */
  GPIO_InitStruct.Pin = LTC_SMB_ALERT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : EN_CAN_Pin */
  GPIO_InitStruct.Pin = EN_CAN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : STB_CAN_Pin */
  GPIO_InitStruct.Pin = STB_CAN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, EN_CAN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, STB_CAN_Pin, GPIO_PIN_RESET);

}


/*******************************************************************************************
********************************************************************************************
                               PRIVATE HAL_MspInit_XXModule
********************************************************************************************
*******************************************************************************************/
/**
  * Initializes the Global MSP.
  */
extern "C" void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */
  PWR_PVDTypeDef sConfigPVD = {0};

  // Control debugger StopMode
	if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)	{
		LL_DBGMCU_EnableDBGStopMode();
	} else {
		LL_DBGMCU_DisableDBGStopMode();
	}

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();


  /** PVD Configuration
  */
  sConfigPVD.PVDLevel = PWR_PVDLEVEL_0;
  sConfigPVD.Mode = PWR_PVD_MODE_NORMAL;
  HAL_PWR_ConfigPVD(&sConfigPVD);

  /** Enable the PVD Output
  */
  HAL_PWR_EnablePVD();

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}
#if (ENABLE_CAN)
/**
* @brief CAN MSP Initialization
* This function configures the hardware resources used in this example
* @param hcan: CAN handle pointer
* @retval None
*/
extern "C" void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hcan->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */

    // GPIO Ports clock enable
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // CAN1 clock enable
    __HAL_RCC_CAN1_CLK_ENABLE();

    // Mapping GPIO for CAN
    /* Configure CAN pin: RX */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_CAN1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    /* Configure CAN pin: TX */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_CAN1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

/**
* @brief CAN MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hcan: CAN handle pointer
* @retval None
*/
extern "C" void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan)
{
  if(hcan->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN1 GPIO Configuration
    PB12     ------> CAN1_RX
    PB13     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13);

  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }

}
#endif

#ifdef _HW_MSP_CRC_PRIVATE
/**
* @brief CRC MSP Initialization
* This function configures the hardware resources used in this example
* @param hcrc: CRC handle pointer
* @retval None
*/
extern "C" void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc)
{
  if(hcrc->Instance==CRC)
  {
  /* USER CODE BEGIN CRC_MspInit 0 */

  /* USER CODE END CRC_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_CRC_CLK_ENABLE();
  /* USER CODE BEGIN CRC_MspInit 1 */

  /* USER CODE END CRC_MspInit 1 */
  }

}

/**
* @brief CRC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hcrc: CRC handle pointer
* @retval None
*/
extern "C" void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc)
{
  if(hcrc->Instance==CRC)
  {
  /* USER CODE BEGIN CRC_MspDeInit 0 */

  /* USER CODE END CRC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CRC_CLK_DISABLE();
  /* USER CODE BEGIN CRC_MspDeInit 1 */

  /* USER CODE END CRC_MspDeInit 1 */
  }

}
#endif

#ifdef _HW_MSP_LPTIM_PRIVATE
/**
* @brief LPTIM MSP Initialization
* This function configures the hardware resources used in this example
* @param hlptim: LPTIM handle pointer
* @retval None
*/
extern "C" void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef* hlptim)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(hlptim->Instance==LPTIM1)
  {
  /* USER CODE BEGIN LPTIM1_MspInit 0 */

  /* USER CODE END LPTIM1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
    PeriphClkInit.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_LPTIM1_CLK_ENABLE();
  /* USER CODE BEGIN LPTIM1_MspInit 1 */

  /* USER CODE END LPTIM1_MspInit 1 */
  }

}

/**
* @brief LPTIM MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hlptim: LPTIM handle pointer
* @retval None
*/
extern "C" void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef* hlptim)
{
  if(hlptim->Instance==LPTIM1)
  {
  /* USER CODE BEGIN LPTIM1_MspDeInit 0 */

  /* USER CODE END LPTIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LPTIM1_CLK_DISABLE();
  /* USER CODE BEGIN LPTIM1_MspDeInit 1 */

  /* USER CODE END LPTIM1_MspDeInit 1 */
  }

}
#endif

#if (ENABLE_QSPI)
/**
* @brief QSPI MSP Initialization
* This function configures the hardware resources used in this example
* @param hqspi: QSPI handle pointer
* @retval None
*/
extern "C" void HAL_QSPI_MspInit(QSPI_HandleTypeDef* hqspi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hqspi->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspInit 0 */

  /* USER CODE END QUADSPI_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_QSPI_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**QUADSPI GPIO Configuration
    PA3     ------> QUADSPI_CLK
    PA6     ------> QUADSPI_BK1_IO3
    PA7     ------> QUADSPI_BK1_IO2
    PB0     ------> QUADSPI_BK1_IO1
    PB1     ------> QUADSPI_BK1_IO0
    PB11     ------> QUADSPI_BK1_NCS
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* QUADSPI interrupt Init */
    HAL_NVIC_SetPriority(QUADSPI_IRQn, QSPI_NVIC_INT_PREMPT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(QUADSPI_IRQn);
  /* USER CODE BEGIN QUADSPI_MspInit 1 */

  /* USER CODE END QUADSPI_MspInit 1 */
  }

}

/**
* @brief QSPI MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hqspi: QSPI handle pointer
* @retval None
*/
void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* hqspi)
{
  if(hqspi->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspDeInit 0 */

  /* USER CODE END QUADSPI_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_QSPI_CLK_DISABLE();

    /**QUADSPI GPIO Configuration
    PA3     ------> QUADSPI_CLK
    PA6     ------> QUADSPI_BK1_IO3
    PA7     ------> QUADSPI_BK1_IO2
    PB0     ------> QUADSPI_BK1_IO1
    PB1     ------> QUADSPI_BK1_IO0
    PB11     ------> QUADSPI_BK1_NCS
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3|GPIO_PIN_6|GPIO_PIN_7);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_11);

    /* QUADSPI interrupt DeInit */
    HAL_NVIC_DisableIRQ(QUADSPI_IRQn);
  /* USER CODE BEGIN QUADSPI_MspDeInit 1 */

  /* USER CODE END QUADSPI_MspDeInit 1 */
  }

}
#endif

#ifdef _HW_MSP_RNG_PRIVATE
/**
* @brief RNG MSP Initialization
* This function configures the hardware resources used in this example
* @param hrng: RNG handle pointer
* @retval None
*/
extern "C" void HAL_RNG_MspInit(RNG_HandleTypeDef* hrng)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(hrng->Instance==RNG)
  {
  /* USER CODE BEGIN RNG_MspInit 0 */

  /* USER CODE END RNG_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RNG;
    PeriphClkInit.RngClockSelection = RCC_RNGCLKSOURCE_HSI48;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_RNG_CLK_ENABLE();
  /* USER CODE BEGIN RNG_MspInit 1 */

  /* USER CODE END RNG_MspInit 1 */
  }

}

/**
* @brief RNG MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hrng: RNG handle pointer
* @retval None
*/
extern "C" void HAL_RNG_MspDeInit(RNG_HandleTypeDef* hrng)
{
  if(hrng->Instance==RNG)
  {
  /* USER CODE BEGIN RNG_MspDeInit 0 */

  /* USER CODE END RNG_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RNG_CLK_DISABLE();
  /* USER CODE BEGIN RNG_MspDeInit 1 */

  /* USER CODE END RNG_MspDeInit 1 */
  }

}
#endif