/**@file ETH452_qspi.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
<http://www.gnu.org/licenses/>.
**********************************************************************/

#ifndef _ETH452_QSPI_H
#define _ETH452_QSPI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "os_port.h"
#include "stm32l4xx_hal.h"

/* Exported constants --------------------------------------------------------*/
/** @defgroup  Exported Constants
  * @{
  */
#define FLASH_MEMORYMAP_BASE	0x90000000

/* QSPI Error codes */
typedef enum
{
  QSPI_OK       			= HAL_OK,
  QSPI_ERROR   				= HAL_ERROR,
  QSPI_BUSY     			= HAL_BUSY,
	QSPI_TIMEOUT				= HAL_TIMEOUT,
	QSPI_NOT_SUPPORTED,
	QSPI_SUSPENDED,
	QSPI_READY,
	QSPI_RESET
} QSPI_StatusTypeDef;

/* QSPI Info */
typedef struct
{
  uint32_t FlashSize;          	/*!< Size of the flash */
  uint32_t EraseBlockSize;    	/*!< Size of blocks for the erase operation */
  uint32_t EraseBlockNumber;		/*!< Number of blocks for the erase operation */
  uint32_t ProgPageSize;       	/*!< Size of pages for the program operation */
  uint32_t ProgPagesNumber;    	/*!< Number of pages for the program operation */
	uint32_t StatusRegister;			/*!< Flash status register(byte 1 & byte 2) */
  __IO QSPI_StatusTypeDef State;   /*!< QSPI state  */
} QSPI_Info;

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup Exported Functions
  * @{
  */
QSPI_StatusTypeDef BSP_QSPI_Init(void);
QSPI_StatusTypeDef BSP_QSPI_DeInit(void);
QSPI_StatusTypeDef BSP_QSPI_Read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size);
QSPI_StatusTypeDef BSP_QSPI_Write(uint8_t *pData, uint32_t WriteAddr, uint32_t Size);
QSPI_StatusTypeDef BSP_QSPI_Erase_Block(uint32_t BlockAddress);
QSPI_StatusTypeDef BSP_QSPI_Erase_Sector(uint32_t Sector);
QSPI_StatusTypeDef BSP_QSPI_Erase_Chip(void);
QSPI_StatusTypeDef BSP_QSPI_ReadStatus(uint32_t *Reg);
QSPI_StatusTypeDef BSP_QSPI_WriteStatus(uint32_t Reg);
QSPI_StatusTypeDef BSP_QSPI_GetStatus(void);
QSPI_StatusTypeDef BSP_QSPI_GetInfo(QSPI_Info *pInfo);
QSPI_StatusTypeDef BSP_QSPI_EnableMemoryMappedMode(void);
QSPI_StatusTypeDef BSP_QSPI_DisableMemoryMappedMode(void);
uint8_t BSP_QSPI_SuspendErase(void);
uint8_t BSP_QSPI_ResumeErase(void);

#ifdef __cplusplus
}
#endif

#endif