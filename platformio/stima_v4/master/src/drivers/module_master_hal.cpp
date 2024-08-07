/**
  ******************************************************************************
  * @file    module_master_hal.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Interface STM32 hardware_hal STIMAV4
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

#include "drivers/module_master_hal.hpp"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_cortex.h"

#if (ENABLE_I2C2)
TwoWire Wire2 = TwoWire(PIN_I2C2_SDA, PIN_I2C2_SCL);
#endif

#if (ENABLE_SPI2)
SPIClass Spi2(PIN_SPI2_MOSI, PIN_SPI2_MISO, PIN_SPI2_SCK);
#endif

// Non utilizzo FreRTOS LOW_Power per il Debugging
#ifdef _USE_FREERTOS_LOW_POWER
#define _EXIT_SLEEP_FOR_DEBUGGING
#endif

/* Private Hardware_Handler istance initialization ---------------------------------------*/
#if (ENABLE_CAN)
CAN_HandleTypeDef hcan1;
#endif

#if (ENABLE_QSPI)
QSPI_HandleTypeDef hqspi;
#endif

// ********************************************************************************
//  LOCAL REDEFINITION Weak PinMap ARDUINO for SETUP PIN Base or Alternate Function
// ********************************************************************************

// UART
#ifdef HAL_UART_MODULE_ENABLED
const PinMap PinMap_UART_TX[] = {
  {PA_0,       UART4,   STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF8_UART4)},
  {PB_6,       USART1,  STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF7_USART1)},
  {PD_5,       USART2,  STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF7_USART2)},
  {NC,         NP,      0}
};
const PinMap PinMap_UART_RX[] = {
  {PA_1,       UART4,   STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF8_UART4)},
  {PA_10,      USART1,  STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF7_USART1)},
  {PD_6,       USART2,  STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF7_USART2)},
  {NC,         NP,      0}
};
const PinMap PinMap_UART_RTS[] = {
  {PD_4,       USART2,  STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF7_USART2)},
  {NC,         NP,      0}
};
const PinMap PinMap_UART_CTS[] = {
  {PD_3,       USART2,  STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF7_USART2)},
  {NC,         NP,      0}
};
#endif

// I2C
#ifdef HAL_I2C_MODULE_ENABLED
const PinMap PinMap_I2C_SDA[] = {
  {PB_7,       I2C1, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF4_I2C1)},
  {PB_14,      I2C2, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF4_I2C2)},
  {NC,         NP,   0}
};
const PinMap PinMap_I2C_SCL[] = {
  {PB_8,       I2C1, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF4_I2C1)},
  {PB_10,      I2C2, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF4_I2C2)},
  {NC,         NP,   0}
};
#endif

// SPI
#ifdef HAL_SPI_MODULE_ENABLED
const PinMap PinMap_SPI_MOSI[] = {
  {PB_5,      SPI1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF5_SPI1)},
  #if defined(HAL_SD_MODULE_DISABLED) && defined(STIMAV4_MASTER_HW_VER_01_01)
  {PC_1,      SPI2, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF3_SPI2)},
  #endif
  {NC,        NP,   0}
};
const PinMap PinMap_SPI_MISO[] = {
  {PB_4,      SPI1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF5_SPI1)},
  #if defined(HAL_SD_MODULE_DISABLED) && defined(STIMAV4_MASTER_HW_VER_01_01)
  {PC_2,      SPI2, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF5_SPI2)},
  #endif
  {NC,        NP,   0}
};
const PinMap PinMap_SPI_SCLK[] = {
  {PA_5,      SPI1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF5_SPI1)},
  #if defined(HAL_SD_MODULE_DISABLED) && defined(STIMAV4_MASTER_HW_VER_01_01)
  {PB_13,     SPI2, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF5_SPI2)},
  #endif
  {NC,        NP,   0}
};
const PinMap PinMap_SPI_SSEL[] = {
  {PA_15,      SPI1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF5_SPI1)},
  #if defined(HAL_SD_MODULE_DISABLED) && defined(STIMAV4_MASTER_HW_VER_01_01)
  {PB_12,      SPI2, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF5_SPI2)},
  #endif
  {NC,         NP,   0}
};
#endif

// CAN
#ifdef HAL_CAN_MODULE_ENABLED
const PinMap PinMap_CAN_RD[] = {
  {PD_0,  CAN1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF9_CAN1)},
  {NC,    NP,   0}
};
const PinMap PinMap_CAN_TD[] = {
  {PD_1,  CAN1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF9_CAN1)},
  {NC,    NP,   0}
};
#endif

// USB
#if defined(HAL_PCD_MODULE_ENABLED) || defined(HAL_HCD_MODULE_ENABLED)
const PinMap PinMap_USB_OTG_FS[] = {
  // {PA_8,  USB_OTG_FS, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF10_OTG_FS)}, // USB_OTG_FS_SOF
  {PA_9,  USB_OTG_FS, STM_PIN_DATA(STM_MODE_INPUT, GPIO_NOPULL, GPIO_AF_NONE)}, // USB_OTG_FS_VBUS
  {PA_11, USB_OTG_FS, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF10_OTG_FS)}, // USB_OTG_FS_DM
  {PA_12, USB_OTG_FS, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF10_OTG_FS)}, // USB_OTG_FS_DP
  {NC,    NP,         0}
};
#endif

// QSPI FLASH
#ifdef HAL_QSPI_MODULE_ENABLED
const PinMap PinMap_QUADSPI_DATA0[] = {
  {PB_1,  QUADSPI, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF10_QUADSPI)}, // QUADSPI_BK1_IO0
  {NC,    NP,      0}
};
const PinMap PinMap_QUADSPI_DATA1[] = {
  {PB_0,  QUADSPI, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF10_QUADSPI)}, // QUADSPI_BK1_IO1
  {NC,    NP,      0}
};
const PinMap PinMap_QUADSPI_DATA2[] = {
  {PA_7,  QUADSPI, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF10_QUADSPI)}, // QUADSPI_BK1_IO2
  {NC,    NP,      0}
};
const PinMap PinMap_QUADSPI_DATA3[] = {
  {PA_6,  QUADSPI, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF10_QUADSPI)}, // QUADSPI_BK1_IO3
  {NC,    NP,      0}
};
const PinMap PinMap_QUADSPI_SCLK[] = {
  {PA_3,  QUADSPI, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF10_QUADSPI)}, // QUADSPI_CLK
  {NC,    NP,      0}
};
const PinMap PinMap_QUADSPI_SSEL[] = {
  {PA_2,  QUADSPI, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF10_QUADSPI)}, // QUADSPI_BK1_NCS
  {NC,    NP,      0}
};
#endif

// MMC1
#ifdef HAL_SD_MODULE_ENABLED
const PinMap PinMap_SD[] = {
  {PC_8,  SDMMC1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF12_SDMMC1)}, // SDMMC1_D0
  {PC_9,  SDMMC1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF12_SDMMC1)}, // SDMMC1_D1
  {PC_10, SDMMC1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF12_SDMMC1)}, // SDMMC1_D2
  {PC_11, SDMMC1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF12_SDMMC1)}, // SDMMC1_D3
  {PC_12, SDMMC1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF12_SDMMC1)}, // SDMMC1_CK
  {PD_2,  SDMMC1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF12_SDMMC1)}, // SDMMC1_CMD
  {NC,    NP,     0}
};
#endif

/* Private Hardware_Handler istance initialization ---------------------------------------*/

// ********************************************************************************************
// ********************************************************************************************
//                       System clock and peripheral base local SETUP
// ********************************************************************************************
// *******************************************************************************************/

/// @brief System Clock Configuration
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_LSE
                              |RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_LSE, RCC_MCODIV_1);

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

	// Enable cycle counter (for debug)
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  // Enable Handler Fault Error
  LL_HANDLER_EnableFault(LL_HANDLER_FAULT_USG);
  LL_HANDLER_EnableFault(LL_HANDLER_FAULT_BUS);
  LL_HANDLER_EnableFault(LL_HANDLER_FAULT_MEM);

  // Setup GPIO and Periph Hardware
  MX_GPIO_Init();

  #if (ENABLE_CAN)
  MX_CAN1_Init();
  #endif
  #if (ENABLE_QSPI)
  MX_QUADSPI_Init();
  #endif

	/* Abilito la carica del supercap */
	HAL_PWREx_EnableBatteryCharging(PWR_BATTERY_CHARGING_RESISTOR_5);

}

/// @brief Get Unique ID HW of CPU (SerialNumber Unique ID)
/// @param  ptrCpuId pointer to external 12 byte buffer required (CPU_ID)
void STM32L4GetCPUID(uint8_t *ptrCpuId) {
  for(uint8_t uid=0; uid<12; uid++) {
    ptrCpuId[uid++] = (uint8_t)(READ_REG(*((uint32_t *)(UID_BASE_ADDRESS + uid))));
  }
}

/// @brief Get StimaV4 Serial Number from UID Cpu and Module TYPE
/// @return Serial Number 64 BIT
uint64_t StimaV4GetSerialNumber(void) {
  volatile uint64_t serNumb = 0;
  uint8_t *ptrData = (uint8_t*)&serNumb;
  ptrData[0] = MODULE_TYPE;
  ptrData[1] = (uint8_t)(READ_REG(*((uint32_t *)(UID_BASE_ADDRESS))));
  ptrData[2] = (uint8_t)(READ_REG(*((uint32_t *)(UID_BASE_ADDRESS + 10))));
  ptrData[3] = (uint8_t)(READ_REG(*((uint32_t *)(UID_BASE_ADDRESS + 9))));
  ptrData[4] = (uint8_t)(READ_REG(*((uint32_t *)(UID_BASE_ADDRESS + 7))));
  ptrData[5] = (uint8_t)(READ_REG(*((uint32_t *)(UID_BASE_ADDRESS + 6))));
  ptrData[6] = (uint8_t)(READ_REG(*((uint32_t *)(UID_BASE_ADDRESS + 5))));
  ptrData[7] = (uint8_t)(READ_REG(*((uint32_t *)(UID_BASE_ADDRESS + 4))));
  return serNumb;
}

// ********************************************************************************************
// ********************************************************************************************
//                    System base Hardware Istance and private Initialization
// ********************************************************************************************
// *******************************************************************************************/
#if (ENABLE_CAN)
/// @brief CAN1 Initialization Function
void MX_CAN1_Init(void)
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

#if (ENABLE_QSPI)
/// @brief QUADSPI Initialization Function
void MX_QUADSPI_Init(void)
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
  hqspi.Init.FlashSize = 22;
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

/// @brief GPIO Initialization Function
void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();


  // ********** SETUP Port E *************

  // Set OUT Default Value
  HAL_GPIO_WritePin(GPIOE, ENCODER_Power_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, CAN_Enable_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, CAN_StanbdBy_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, BUZZER_Power_Pin, GPIO_PIN_RESET);

  // SETUP Encoder PIN Left Right PullUP
  GPIO_InitStruct.Pin = ENCODER_A_Pin|ENCODER_B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  // Setup Encoder Enter NoPull
  GPIO_InitStruct.Pin = ENCODER_Ent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* Output Can Enable/StandBy && Encoder Power ON && Buzzer */
  GPIO_InitStruct.Pin = ENCODER_Power_Pin|CAN_Enable_Pin|CAN_StanbdBy_Pin|BUZZER_Power_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* Unused PEx (esclude UPIN27) */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);


  // ********** SETUP Port C *************

  // Setup Encoder Enter NoPull
  GPIO_InitStruct.Pin = MMC1_Detect_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* Unused PCx (esclude UPIN27) */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);


  // ********** SETUP Port H *************
 
  /* Unused PHx (esclude UPIN27) */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);


  // ********** SETUP Port A *************
 
  /* Unused PAx (esclude UPIN27) */
  #if (!ENABLE_SIM7600E)
  GPIO_InitStruct.Pin = GSM_RingInd_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  #else
  /* Ring Indicator NoPull */
  GPIO_InitStruct.Pin = GSM_RingInd_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  #endif

  /* MCO */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // ********** SETUP Port B *************
 
  HAL_GPIO_WritePin(GPIOB, DISPLAY_Power_Pin, GPIO_PIN_RESET);

  /* Display enable */
  GPIO_InitStruct.Pin = DISPLAY_Power_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Unused PBx (esclude UPIN27) */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_9|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


  // ********** SETUP Port D *************

  // GSM_PowerEn_Pin Need to Set for initialization Chip
  HAL_GPIO_WritePin(GPIOD, GSM_PowerEn_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOD, GSM_PowerKey_Pin, GPIO_PIN_RESET);

  /* PowerKey && PowerGsm OUT */
  GPIO_InitStruct.Pin = GSM_PowerEn_Pin|GSM_PowerKey_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  #if (ENABLE_SIM7600E)
  /** USART2 GPIO Configuration to 7600E
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX
  */
  GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  #endif

  /* Unused PDx (esclude UPIN27) */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

// ********************************************************************************************
// ********************************************************************************************
//                                PRIVATE HAL_MspInit_XXModule
// ********************************************************************************************
// *******************************************************************************************/

/// @brief System Mase MSPInit
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
/// @brief CAN MSP Initialization. This function configures the hardware resources used in this example
/// @param hcan: CAN handle pointer
extern "C" void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hcan->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */

    // GPIO Ports clock enable
    __HAL_RCC_GPIOD_CLK_ENABLE();

    // CAN1 clock enable
    __HAL_RCC_CAN1_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PD0     ------> CAN1_RX
    PD1     ------> CAN1_TX
    */

    // Mapping GPIO for CAN
    /* Configure CAN pin: RX */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    /* Configure CAN pin: TX */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

/// @brief CAN MSP De-Initialization. This function freeze the hardware resources used in this example
/// @param hcan: CAN handle pointer
extern "C" void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan)
{
  if(hcan->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN1 GPIO Configuration
    PD0     ------> CAN1_RX
    PD1     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0|GPIO_PIN_1);

  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }

}
#endif

#if (ENABLE_QSPI)
/// @brief QSPI MSP Initialization. This function configures the hardware resources used in this example
/// @param hqspi: QSPI handle pointer
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
    PA2     ------> QUADSPI_BK1_NCS
    PA3     ------> QUADSPI_CLK
    PA6     ------> QUADSPI_BK1_IO3
    PA7     ------> QUADSPI_BK1_IO2
    PB0     ------> QUADSPI_BK1_IO1
    PB1     ------> QUADSPI_BK1_IO0
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN QUADSPI_MspInit 1 */

    /* QUADSPI interrupt Init */
    HAL_NVIC_SetPriority(QUADSPI_IRQn, QSPI_NVIC_INT_PREMPT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(QUADSPI_IRQn);

    /* USER CODE END QUADSPI_MspInit 1 */
  }

}

/// @brief QSPI MSP De-Initialization. This function freeze the hardware resources used in this example
/// @param hqspi: QSPI handle pointer
extern "C" void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* hqspi)
{
  if(hqspi->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspDeInit 0 */

  /* USER CODE END QUADSPI_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_QSPI_CLK_DISABLE();

    /**QUADSPI GPIO Configuration
    PA2     ------> QUADSPI_BK1_NCS
    PA3     ------> QUADSPI_CLK
    PA6     ------> QUADSPI_BK1_IO3
    PA7     ------> QUADSPI_BK1_IO2
    PB0     ------> QUADSPI_BK1_IO1
    PB1     ------> QUADSPI_BK1_IO0
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_6|GPIO_PIN_7);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0|GPIO_PIN_1);

    /* QUADSPI interrupt DeInit */
    HAL_NVIC_DisableIRQ(QUADSPI_IRQn);

    /* USER CODE BEGIN QUADSPI_MspDeInit 1 */

    /* USER CODE END QUADSPI_MspDeInit 1 */
  }

}
#endif

#if (ENABLE_SPI1)
/// @brief SPI MSP Initialization. This function configures the hardware resources used in this example
/// @param hspi: SPI handle pointer
extern "C" void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hspi->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PB4 (NJTRST)     ------> SPI1_MISO
    PB5     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI1_MspInit 1 */

  /* USER CODE END SPI1_MspInit 1 */
  }

}

/// @brief SPI MSP De-Initialization. This function freeze the hardware resources used in this example
/// @param hspi: SPI handle pointer
extern "C" void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
  if(hspi->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */

  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PB4 (NJTRST)     ------> SPI1_MISO
    PB5     ------> SPI1_MOSI
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_4|GPIO_PIN_5);

  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
  }

}
#endif