/**
  ******************************************************************************
  * @file    Flash.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   flash QSPI ETH452 AT256F641 source file
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
#include "drivers/flash.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup
  * @{
  */

/** @defgroup QSPI
  * @{
  */

/* QSPI IRQ CallBack for QSPI Class ---------------------------------------------------*/

/// @brief This function handles QUADSPI global interrupt.
extern "C" void QUADSPI_IRQHandler(void)
{
  /* USER CODE BEGIN QUADSPI_IRQn 0 */

  /* USER CODE END QUADSPI_IRQn 0 */
  HAL_QSPI_IRQHandler(&hqspi);
  /* USER CODE BEGIN QUADSPI_IRQn 1 */

  /* USER CODE END QUADSPI_IRQn 1 */
}

// Flash Object Pointer IRQ_CallBACK Function
Flash::QSPI_IT_EventFlag QSPI_IRQ_Flag;

/// @brief  Transfer Error callback.
/// @param  hqspi : QSPI handle
/// @retval None
extern "C" void HAL_QSPI_ErrorCallback(QSPI_HandleTypeDef *hqspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hqspi);
  QSPI_IRQ_Flag.flagBit.flag_TE = 1;  
}

/// @brief  Command completed callback.
/// @param  hqspi : QSPI handle
/// @retval None
extern "C" void HAL_QSPI_CmdCpltCallback(QSPI_HandleTypeDef *hqspi)
{
  UNUSED(hqspi);
  QSPI_IRQ_Flag.flagBit.flag_TC = 1;  
}

/// @brief  Rx Transfer completed callback.
/// @param  hqspi : QSPI handle
/// @retval None
extern "C" void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
  UNUSED(hqspi);
  QSPI_IRQ_Flag.flagBit.flag_TC = 1;  
}

/// @brief  Tx Transfer completed callback.
/// @param  hqspi : QSPI handle
/// @retval None
extern "C" void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hqspi);
  QSPI_IRQ_Flag.flagBit.flag_TC = 1;  
}

/// @brief  Status Match callback.
/// @param  hqspi : QSPI handle
/// @retval None
extern "C" void HAL_QSPI_StatusMatchCallback(QSPI_HandleTypeDef *hqspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hqspi);
  QSPI_IRQ_Flag.flagBit.flag_SM = 1;
}

/* Constructor Class ---------------------------------------------------------*/

/** @defgroup Constructor Class
  * @{
  */

Flash::Flash()
{
}

Flash::Flash(QSPI_HandleTypeDef *hqspi)
{
	_hqspi = hqspi;
  _evtFlag = &QSPI_IRQ_Flag;
}

/* Implementation functions ---------------------------------------------------------*/

/// @brief  Initializes the QSPI interface.
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_Init(void)
{
	/* Check if QSPI is initialized */
	if (_FlashInfo.State == QSPI_READY)
	  return QSPI_OK;

  _hqspi->Instance = QUADSPI;
	_FlashInfo.State = QSPI_ERROR;

	/* Call the DeInit function to reset the driver */
  if (HAL_QSPI_DeInit(_hqspi) != HAL_OK)
    return QSPI_ERROR;

  /* QSPI initialization (done in MX_QUADSPI_Init() here rewrite only FlashSize) */
	_hqspi->Init.FlashSize = POSITION_VAL(AT25SF641_FLASH_SIZE) - 1;

  if (HAL_QSPI_Init(_hqspi) != HAL_OK)
    return QSPI_ERROR;

  /* Start Flash Get State */
	_FlashInfo.State = QSPI_KO_INIT;

	/* Exit from deep power down */
	if (QSPI_ExitDeepPowerDown())
    return QSPI_ERROR;

	/* Read flash status */
	if (BSP_QSPI_ReadStatus(&_FlashInfo.StatusRegister) != QSPI_OK)
    return QSPI_ERROR;

	/* Flash busy? */
	if ((_FlashInfo.StatusRegister & AT25SF641_SR_BUSY))
    return QSPI_BUSY;

	/* Device Quad mode enable */
	if ((_FlashInfo.StatusRegister & AT25SF641_SR_QE) != AT25SF641_SR_QE) {
		uint32_t Flash_SR = _FlashInfo.StatusRegister | AT25SF641_SR_QE;
		if (BSP_QSPI_WriteStatus(Flash_SR) != QSPI_OK)
			return QSPI_ERROR;
	}

	/* Read again flash status */
	if (BSP_QSPI_ReadStatus(&_FlashInfo.StatusRegister) != QSPI_OK)
    return QSPI_ERROR;

	/* QSPI memory reset */
  if (QSPI_ResetMemory() != QSPI_OK)
    return QSPI_NOT_SUPPORTED;

  /* Dummy cycles configuration on QSPI memory side */
  if (QSPI_DummyCyclesCfg() != QSPI_OK)
    return QSPI_NOT_SUPPORTED;

  /* Get Info State and Set Ready */
	BSP_QSPI_GetInfo(&_FlashInfo);

	_FlashInfo.State = QSPI_READY;
  return QSPI_OK;
}

/// @brief  De-Initializes the QSPI interface.
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_DeInit(void)
{
	HAL_StatusTypeDef sts;
	QSPI_StatusTypeDef qsts;

	if (_FlashInfo.State == 0 || _FlashInfo.State == QSPI_RESET)
	  return QSPI_OK;

	_hqspi->Instance = QUADSPI;

	/* Disable memory mapped mode */
	if (BSP_QSPI_DisableMemoryMappedMode() != QSPI_OK)
		return QSPI_ERROR;

  /* WakeUp */
	if ((qsts = QSPI_SetDeepPowerDown()) != QSPI_OK)
		return qsts;
	HAL_Delay(1);

	/* Call the DeInit function to reset the driver */
	if ((sts = HAL_QSPI_DeInit(_hqspi)) != HAL_OK)
		return (QSPI_StatusTypeDef) sts;

	/* Init QSPI status flag */
  _evtFlag->flag = 0;

	_FlashInfo.State = QSPI_RESET;

  return QSPI_OK;
}

/// @brief  Reads an amount of data from the QSPI memory.
/// @param  pData: Pointer to data to be read
/// @param  ReadAddr: Read start address
/// @param  Size: Size of data to read
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_Read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
	HAL_StatusTypeDef sts;
	QSPI_CommandTypeDef sCommand = {0};

	/* Disable memory mapped mode */
	if (BSP_QSPI_DisableMemoryMappedMode() != QSPI_OK)
		return QSPI_ERROR;

  /* Initialize the read command */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = QUAD_INOUT_FAST_READ_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
  sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
  sCommand.Address           = ReadAddr;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
	sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	sCommand.AlternateBytes		 = 0;
  sCommand.DataMode          = QSPI_DATA_4_LINES;
  sCommand.DummyCycles       = AT25SF641_DUMMY_CYCLES_READ_QUAD;
  sCommand.NbData            = Size;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
	if ((sts = HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK)
		return (QSPI_StatusTypeDef) sts;

  /* Reception of the data */
	return BSP_QSPI_Receive(pData, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
}

/// @brief  Writes an amount of data to the QSPI memory.
/// @param  pData: Pointer to data to be written
/// @param  WriteAddr: Write start address
/// @param  Size: Size of data to write
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_Write(uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
	HAL_StatusTypeDef sts;
	QSPI_StatusTypeDef qsts;
	QSPI_CommandTypeDef sCommand = {0};
  uint32_t end_addr, current_size, current_addr;

  /* Calculation of the size between the write address and the end of the page */
  current_size = AT25SF641_PAGE_SIZE - (WriteAddr % AT25SF641_PAGE_SIZE);

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > Size)
    current_size = Size;

  /* Initialize the adress variables */
  current_addr = WriteAddr;
  end_addr = WriteAddr + Size;

	/* Disable memory mapped mode */
	if ((qsts = BSP_QSPI_DisableMemoryMappedMode()) != QSPI_OK)
		return qsts;

  /* Initialize the program command */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = PAGE_PROG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
  sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_1_LINE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Perform the write page by page */
  do {
    sCommand.Address = current_addr;
    sCommand.NbData  = current_size;

    /* Enable write operations */
    if ((qsts = QSPI_WriteEnable()) != QSPI_OK)
      return qsts;

    /* Configure the command */
    if ((sts = HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK)
      return (QSPI_StatusTypeDef) sts;

    /* Transmission of the data */
    if ((qsts = BSP_QSPI_Transmit(pData, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)) != QSPI_OK)
      return qsts;

    /* Configure automatic polling mode to wait for end of program */
    if ((qsts = QSPI_AutoPollingMemReady(HAL_QSPI_TIMEOUT_DEFAULT_VALUE)) != QSPI_OK)
      return qsts;

    /* Update the address and size variables for next page programming */
    current_addr += current_size;
    pData += current_size;
    current_size = ((current_addr + AT25SF641_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : AT25SF641_PAGE_SIZE;
  } while (current_addr < end_addr);

  return QSPI_OK;
}

/// @brief  Erases the specified block of the QSPI memory.
/// @param  BlockAddress: Block address to erase
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_Erase_Block(uint32_t BlockAddress)
{
	QSPI_CommandTypeDef sCommand = {0};

	/* Disable memory mapped mode */
	if (BSP_QSPI_DisableMemoryMappedMode() != QSPI_OK)
		return QSPI_ERROR;

  /* Initialize the erase command */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = BLOCK_ERASE_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
  sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
  sCommand.Address           = (BlockAddress * AT25SF641_BLOCK_SIZE);
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  if (QSPI_WriteEnable() != QSPI_OK)
    return QSPI_ERROR;

  /* Send the command */
	if (HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

  /* Configure automatic polling mode to wait for end of erase */
  return QSPI_AutoPollingMemReady(AT25SF641_BLOCK_ERASE_MAX_TIME);
}

/// @brief  Erases the specified sector of the QSPI memory.
/// @param  Sector: Sector address to erase (0 to 255)
/// @retval QSPI memory status
/// @note This function is non blocking meaning that sector erase
///       operation is started but not completed when the function
///       returns. Application has to call BSP_QSPI_GetStatus()
///       to know when the device is available again (i.e. erase operation
///       completed).
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_Erase_Sector(uint32_t Sector)
{
	QSPI_CommandTypeDef sCommand = {0};

  if (Sector >= (uint32_t)(AT25SF641_FLASH_SIZE / AT25SF641_SECTOR_SIZE))
    return QSPI_ERROR;

	/* Disable memory mapped mode */
	if (BSP_QSPI_DisableMemoryMappedMode() != QSPI_OK)
		return QSPI_ERROR;

  /* Initialize the erase command */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = SECTOR_ERASE_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
  sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
  sCommand.Address           = (Sector * AT25SF641_SECTOR_SIZE);
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  if (QSPI_WriteEnable() != QSPI_OK)
    return QSPI_ERROR;

  /* Send the command */
	if (HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

  /* Configure automatic polling mode to wait for end of erase */
  return QSPI_AutoPollingMemReady(AT25SF641_SECTOR_ERASE_MAX_TIME);
}

// @brief  Erases the entire QSPI memory.
// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_Erase_Chip(void)
{
	QSPI_CommandTypeDef sCommand = {0};

	/* Disable memory mapped mode */
	if (BSP_QSPI_DisableMemoryMappedMode() != QSPI_OK)
		return QSPI_ERROR;

  /* Initialize the erase command */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = BULK_ERASE_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  if (QSPI_WriteEnable() != QSPI_OK)
    return QSPI_ERROR;

  /* Send the command */
	if (HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return QSPI_ERROR;

  /* Configure automatic polling mode to wait for end of erase */
  return QSPI_AutoPollingMemReady(AT25SF641_BULK_ERASE_MAX_TIME);
}

/// @brief  Reads current full status registers (byte1 and 2) of the QSPI memory.
/// @param  Reg: destination variable address
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_ReadStatus(uint32_t *Reg)
{
	HAL_StatusTypeDef sts;
	QSPI_CommandTypeDef sCommand = {0};
  uint8_t reg1, reg2;

  /* Initialize the read status register byte 1 command */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = READ_STATUS1_REG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_1_LINE;
  sCommand.DummyCycles       = 0;
  sCommand.NbData            = 1;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if ((sts = HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK)
    return (QSPI_StatusTypeDef) sts;

  /* Reception of the data byte 1*/
  if ((sts = HAL_QSPI_Receive(_hqspi, &reg1, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK)
    return (QSPI_StatusTypeDef) sts;

  /* Initialize the read status register byte 2 command */
  sCommand.Instruction = READ_STATUS2_REG_CMD;

  /* Configure the command */
  if ((sts = HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK)
    return (QSPI_StatusTypeDef) sts;

  /* Reception of the data byte 2*/
  if ((sts = HAL_QSPI_Receive(_hqspi, &reg2, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK)
    return (QSPI_StatusTypeDef) sts;

	*Reg = (reg2 << 8) + reg1;
  return QSPI_OK;
}
 
/// @brief  Write status registers of the QSPI memory.
/// @param  Reg: source variable
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_WriteStatus(uint32_t Reg)
{
	HAL_StatusTypeDef sts;
	QSPI_StatusTypeDef qsts;
	QSPI_CommandTypeDef sCommand = {0};
  uint8_t reg[2];
	reg[0] = Reg & 0xFF;
	reg[1] = Reg >> 8;

	/* Enable Volatile Status Register mode */
	if ((qsts = QSPI_WriteEnableVolat())!= QSPI_OK)
    return qsts;
#if 0
  /* Enable write operations (NON CI VUOLE!) */
  if ((qsts = QSPI_WriteEnable()) != QSPI_OK)
    return qsts;
#endif

  /* Initialize the write status register command */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = WRITE_STATUS1_REG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_1_LINE;
  sCommand.DummyCycles       = 0;
  sCommand.NbData            = 1;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
	if ((sts = HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK)
		return (QSPI_StatusTypeDef) sts;

	/* Transmission of the status 1 data */
	if ((qsts = BSP_QSPI_Transmit(&reg[0], HAL_QSPI_TIMEOUT_DEFAULT_VALUE)) != QSPI_OK)
    return qsts;

	if ((qsts = QSPI_AutoPollingMemReady(HAL_QSPI_TIMEOUT_DEFAULT_VALUE)) != QSPI_OK)
		return qsts;

	/* Enable Volatile Status Register mode */
	if ((qsts = QSPI_WriteEnableVolat())!= QSPI_OK)
    return qsts;

  /* Initialize the write status register byte 2 command */
  sCommand.Instruction = WRITE_STATUS2_REG_CMD;

  /* Configure the command */
  if ((sts = HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK)
    return (QSPI_StatusTypeDef) sts;

	/* Transmission of the status 2 data */
	if ((qsts = BSP_QSPI_Transmit(&reg[1], HAL_QSPI_TIMEOUT_DEFAULT_VALUE)) != QSPI_OK)
    return qsts;

	return QSPI_AutoPollingMemReady(HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
}

/// @brief  Reads current status (RDY/BUSY, WEL and SUS bits) of the QSPI memory.
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_GetStatus(void)
{
  uint32_t sts;

	if (BSP_QSPI_ReadStatus(&sts) != QSPI_OK)
    return QSPI_ERROR;

	if ((sts & AT25SF641_FS_ERSUS) == AT25SF641_FS_ERSUS)
    return QSPI_SUSPENDED;
	else if ((sts & AT25SF641_SR_BUSY) == AT25SF641_SR_BUSY)
    return QSPI_BUSY;
  else
    return QSPI_OK;
}

/// @brief  Return the configuration of the QSPI memory.
/// @param  pInfo: pointer on the configuration structure
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_GetInfo(QSPI_Info *pInfo)
{
  /* Configure the structure with the memory configuration */
  pInfo->FlashSize          = AT25SF641_FLASH_SIZE;
  pInfo->EraseBlockSize    = AT25SF641_BLOCK_SIZE;
  pInfo->EraseBlockNumber = (AT25SF641_FLASH_SIZE / AT25SF641_BLOCK_SIZE);
  pInfo->ProgPageSize       = AT25SF641_PAGE_SIZE;
  pInfo->ProgPagesNumber    = (AT25SF641_FLASH_SIZE / AT25SF641_PAGE_SIZE);

  return QSPI_OK;
}

/// @brief  Configure the QSPI in memory-mapped mode
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_EnableMemoryMappedMode(void)
{
	QSPI_CommandTypeDef sCommand = {0};
	QSPI_MemoryMappedTypeDef sMemMappedCfg = {0};

	/* Is already in memory mapped mode? */
	if (_hqspi->State == HAL_QSPI_STATE_BUSY_MEM_MAPPED)
		return QSPI_OK;

  /* Configure the command for the read instruction */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = QUAD_INOUT_FAST_READ_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
  sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
	sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	sCommand.AlternateBytes		 = 0x20;

  sCommand.DataMode          = QSPI_DATA_4_LINES;
  sCommand.DummyCycles       = AT25SF641_DUMMY_CYCLES_READ_QUAD;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;

  /* Configure the memory mapped mode */
  sMemMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;

	return (QSPI_StatusTypeDef) HAL_QSPI_MemoryMapped(_hqspi, &sCommand, &sMemMappedCfg);
}

/// @brief  Disable the QSPI memory-mapped mode
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_DisableMemoryMappedMode(void)
{
	if (__HAL_QSPI_GET_FLAG(_hqspi, QSPI_FLAG_BUSY) == SET) {
		if (HAL_QSPI_Abort(_hqspi) != HAL_OK)
			return QSPI_ERROR;
		if (QSPI_DisableContinuousMode() != QSPI_OK)
			return QSPI_ERROR;
	}
  return QSPI_OK;
}

/// @brief  This function suspends an ongoing erase command.
/// @retval QSPI memory status
uint8_t Flash::BSP_QSPI_SuspendErase(void)
{
	QSPI_CommandTypeDef sCommand = {0};

  /* Check whether the device is busy (erase operation is
  in progress).
  */
  if (BSP_QSPI_GetStatus() == QSPI_BUSY) {
    /* Initialize the erase command */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = PROG_ERASE_SUSPEND_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Enable write operations */
    if (QSPI_WriteEnable() != QSPI_OK)
      return QSPI_ERROR;

    /* Send the command */
    if (HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
      return QSPI_ERROR;

    if (BSP_QSPI_GetStatus() == QSPI_SUSPENDED)
      return QSPI_OK;

    return QSPI_ERROR;
  }

  return QSPI_OK;
}

/// @brief  This function resumes a paused erase command.
/// @retval QSPI memory status
uint8_t Flash::BSP_QSPI_ResumeErase(void)
{
	QSPI_CommandTypeDef sCommand = {0};

  /* Check whether the device is in suspended state */
  if (BSP_QSPI_GetStatus() == QSPI_SUSPENDED) {
    /* Initialize the erase command */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = PROG_ERASE_RESUME_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Enable write operations */
    if (QSPI_WriteEnable() != QSPI_OK)
      return QSPI_ERROR;

    /* Send the command */
    if (HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
      return QSPI_ERROR;

    /*
    When this command is executed, the status register write in progress bit is set to 1, and
    the flag status register program erase controller bit is set to 0. This command is ignored
    if the device is not in a suspended state.
    */

    if (BSP_QSPI_GetStatus() == QSPI_BUSY)
      return QSPI_OK;

    return QSPI_ERROR;
  }

  return QSPI_OK;
}

/// @brief Receive an amount of data in interrupt mode.
/// @param hqspi : QSPI handle
/// @param pData : pointer to data buffer
/// @param Timeout : Timeout duration
/// @note   This function is used only in Indirect Read Mode
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_Receive(uint8_t *pData, uint32_t Timeout)
{
	/* Try Match QSPI status flag */
  _evtFlag->flag = 0;
  _evtFlag->flagBit.match_TE = 1;
  _evtFlag->flagBit.match_TC = 1;
  if (HAL_QSPI_Receive_IT(_hqspi, pData) != HAL_OK)
    return QSPI_ERROR;
  return BSP_QSPI_WaitingForEvent(Timeout);
}

/// @brief Transmit an amount of data in interrupt mode.
/// @param hqspi : QSPI handle
/// @param pData : pointer to data buffer
/// @param Timeout : Timeout duration
/// @note   This function is used only in Indirect Write Mode
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_Transmit(uint8_t *pData, uint32_t Timeout)
{
	/* Try Match QSPI status flag */
  _evtFlag->flag = 0;
  _evtFlag->flagBit.match_TE = 1;
  _evtFlag->flagBit.match_TC = 1;
  if (HAL_QSPI_Transmit_IT(_hqspi, pData) != HAL_OK)
    return QSPI_ERROR;
  return BSP_QSPI_WaitingForEvent(Timeout);
}

/// @brief  Configure the QSPI Automatic Polling Mode in interrupt mode.
/// @param  hqspi : QSPI handle
/// @param  cmd : structure that contains the command configuration information.
/// @param  cfg : structure that contains the polling configuration information.
/// @param  Timeout : Timeout duration
/// @note   This function is used only in Automatic Polling Mode
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_AutoPolling(QSPI_CommandTypeDef *cmd, QSPI_AutoPollingTypeDef *cfg, uint32_t Timeout)
{
	/* Try Match QSPI status flag */
  _evtFlag->flag = 0;
  _evtFlag->flagBit.match_TE = 1;
  _evtFlag->flagBit.match_SM = 1;
	if (HAL_QSPI_AutoPolling_IT(_hqspi, cmd, cfg) != HAL_OK)
    return QSPI_ERROR;
  return BSP_QSPI_WaitingForEvent(Timeout);
}

/// @brief Waiting event for QSPI Module
/// @param evtIT EventIT flag waiting test
/// @param Timeout Timeout for operation
/// @return QSPI polling status flag
Flash::QSPI_StatusTypeDef Flash::BSP_QSPI_WaitingForEvent(uint32_t Timeout) {
  bool bSetOFlag = false;
  bool bTimeOut = false;
  bool bMatchField = false;
  Timeout += millis();
  if( Timeout < millis()) bSetOFlag = true;
  while((!bTimeOut)&&(!bMatchField)) {
    // Switch Task
    taskYIELD();
    // Test Flag
    if(_evtFlag->flagBit.match_SM && _evtFlag->flagBit.flag_SM) bMatchField = true;
    if(_evtFlag->flagBit.match_TC && _evtFlag->flagBit.flag_TC) bMatchField = true;
    if(_evtFlag->flagBit.match_TE && _evtFlag->flagBit.flag_TE) bMatchField = true;
    // Check Timeout
    if(bSetOFlag) {
      if(millis() < Timeout) bSetOFlag = false;
    } else {
      if(millis() > Timeout) bTimeOut = true;
    }
  }
  if(bTimeOut) return QSPI_TIMEOUT;
  if(_evtFlag->flagBit.match_SM && _evtFlag->flagBit.flag_SM) return QSPI_OK;
  if(_evtFlag->flagBit.match_TC && _evtFlag->flagBit.flag_TC) return QSPI_OK;
  return QSPI_ERROR;
}

/// @brief  This function enter in deep power down the QSPI memory.
/// @param  hqspi: QSPI handle
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::QSPI_SetDeepPowerDown(void)
{
	QSPI_CommandTypeDef sCommand = {0};

  /* Enable write operations */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = DEEP_POWER_DOWN_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  return (QSPI_StatusTypeDef) HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
}

/// @brief  This function exit from deep power down the QSPI memory.
/// @param  hqspi: QSPI handle
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::QSPI_ExitDeepPowerDown(void)
{
	QSPI_CommandTypeDef sCommand = {0};

  /* Enable write operations */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = RESUME_FROM_DEEP_PWD_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  return (QSPI_StatusTypeDef) HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
}

/// @brief  This function reset the QSPI memory.
/// @param  hqspi: QSPI handle
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::QSPI_ResetMemory(void)
{
#ifdef __AT25SF641_H
	return QSPI_OK;
#else
	QSPI_CommandTypeDef sCommand = {0};

  /* Initialize the reset enable command */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = RESET_ENABLE_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return QSPI_ERROR;

  /* Send the reset memory command */
  sCommand.Instruction = RESET_MEMORY_CMD;
  if (HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return QSPI_ERROR;

  /* Configure automatic polling mode to wait the memory is ready */
  if (QSPI_AutoPollingMemReady(HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
    return QSPI_ERROR;

  return QSPI_OK;
#endif
}

/// @brief  This function configure the dummy cycles on memory side.
/// @param  hqspi: QSPI handle
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::QSPI_DummyCyclesCfg(void)
{
#ifdef __AT25SF641_H
	return QSPI_OK;
#else

	QSPI_CommandTypeDef sCommand = {0};
  uint8_t reg;

  /* Initialize the read volatile configuration register command */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = READ_VOL_CFG_REG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_1_LINE;
  sCommand.DummyCycles       = 0;
  sCommand.NbData            = 1;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return QSPI_ERROR;

  /* Reception of the data */
  if (HAL_QSPI_Receive(_hqspi, &reg, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return QSPI_ERROR;

  /* Enable write operations */
  if (QSPI_WriteEnable() != QSPI_OK)
    return QSPI_ERROR;

  /* Update volatile configuration register (with new dummy cycles) */
  sCommand.Instruction = WRITE_VOL_CFG_REG_CMD;
  MODIFY_REG(reg, AT25SF641_VCR_NB_DUMMY, (AT25SF641_DUMMY_CYCLES_READ_QUAD << POSITION_VAL(AT25SF641_VCR_NB_DUMMY)));

  /* Configure the write volatile configuration register command */
  if (HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return QSPI_ERROR;

  /* Transmission of the data */
  if (HAL_QSPI_Transmit(_hqspi, &reg, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return QSPI_ERROR;

  return QSPI_OK;
#endif
}

/// @brief  This function enable the write for Volatile Status Register only.
/// @param  hqspi: QSPI handle
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::QSPI_WriteEnableVolat(void)
{
	QSPI_CommandTypeDef sCommand = {0};

  /* Enable write volatile operations */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = WRITE_EN_VOLAT_STATUS_REG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  return (QSPI_StatusTypeDef) HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
}

/// @brief  This function set Write Enable Latch bit and wait it is effective.
/// @param  hqspi: QSPI handle
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::QSPI_WriteEnable(void)
{
	HAL_StatusTypeDef sts;
	QSPI_CommandTypeDef sCommand = {0};
	QSPI_AutoPollingTypeDef sConfig = {0};

  /* Enable write operations */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = WRITE_ENABLE_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if ((sts = HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK)
    return (QSPI_StatusTypeDef) sts;

  /* Configure automatic polling mode to wait for write enabling */
  sConfig.Match           = AT25SF641_SR_WEL;
  sConfig.Mask            = AT25SF641_SR_WEL;
  sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
  sConfig.StatusBytesSize = 1;
  sConfig.Interval        = 0x10;
  sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  sCommand.Instruction    = READ_STATUS1_REG_CMD;
  sCommand.DataMode       = QSPI_DATA_1_LINE;

	return BSP_QSPI_AutoPolling(&sCommand, &sConfig, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
}

/// @brief  This function clear Write Enable Latch bit and wait it is effective.
/// @param  hqspi: QSPI handle
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::QSPI_WriteDisable(void)
{
	HAL_StatusTypeDef sts;
	QSPI_CommandTypeDef sCommand = {0};
	QSPI_AutoPollingTypeDef sConfig = {0};

  /* Disable write operations */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = WRITE_DISABLE_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if ((sts = HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK)
    return (QSPI_StatusTypeDef) sts;

  /* Configure automatic polling mode to wait for write disabling */
  sConfig.Match           = RESET;
  sConfig.Mask            = AT25SF641_SR_WEL;
  sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
  sConfig.StatusBytesSize = 1;
  sConfig.Interval        = 0x10;
  sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  sCommand.Instruction    = READ_STATUS1_REG_CMD;
  sCommand.DataMode       = QSPI_DATA_1_LINE;

	return BSP_QSPI_AutoPolling(&sCommand, &sConfig, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
}

/// @brief  This function disable Continuous Read Mode Reset - Quad.
/// @param  hqspi: QSPI handle
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::QSPI_DisableContinuousMode(void)
{
	QSPI_CommandTypeDef sCommand = {0};

  /* Enable write volatile operations */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = QUAD_CONTINUOUS_READ_MODE_RESET;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	return (QSPI_StatusTypeDef) HAL_QSPI_Command(_hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
}

/// @brief  This function read the SR of the memory and wait the EOP.
/// @param  hqspi: QSPI handle
/// @param  Timeout: Timeout for auto-polling
/// @retval QSPI memory status
Flash::QSPI_StatusTypeDef Flash::QSPI_AutoPollingMemReady(uint32_t Timeout)
{
	QSPI_CommandTypeDef sCommand = {0};
	QSPI_AutoPollingTypeDef sConfig = {0};

  /* Configure automatic polling mode to wait for memory ready */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = READ_STATUS1_REG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_1_LINE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  sConfig.Match           = RESET;
  sConfig.Mask            = AT25SF641_SR_BUSY;
  sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
  sConfig.StatusBytesSize = 1;
  sConfig.Interval        = 0x10;
  sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

	return BSP_QSPI_AutoPolling(&sCommand, &sConfig, Timeout);
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */



