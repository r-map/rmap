/**
  ******************************************************************************
  * @file    flash.h
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   flash QSPI ETH452 AT256F161 header file
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

#ifndef _FLASH_H
#define _FLASH_H

/* Includes ------------------------------------------------------------------*/
#include <STM32FreeRTOS.h>
#include "ticks.hpp"
#include "thread.hpp"
#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "AT25SF161.h"

using namespace cpp_freertos;

#define FLASH_MEMORYMAP_BASE	0x90000000

// Security Check W->R Data
#define CHECK_FLASH_WRITE

#define FLASH_FW_POSITION   (0ul)
#define FLASH_FW_BACKUP     (262144ul)  // 000 - 256 KBytes For Program Flash
#define FLASH_FILE_POSITION (524288ul)  // 256 - 512 KBytes For Program Flash (Backup)
#define FLASH_FREE_ACCESS   (1048756ul) // 512 - 1024 KBytes For Extra File (... -> Free)
#define FLASH_BUFFER_SIZE   (256)
#define FLASH_INFO_SIZE_LEN (256)
#define FLASH_FILE_SIZE_LEN (128)
#define FLASH_SIZE_ADDR(X)  (X + FLASH_FILE_SIZE_LEN + 1)
#define FLASH_INFO_SIZE_U64 (8)

class Flash {

  public:

    /* QSPI Error codes */
    typedef enum
    {
      QSPI_KO_INIT				= -1,
      QSPI_OK       			= HAL_OK,
      QSPI_ERROR   				= HAL_ERROR,
      QSPI_BUSY     			= HAL_BUSY,
      QSPI_TIMEOUT				= HAL_TIMEOUT,
      QSPI_NOT_SUPPORTED,
      QSPI_SUSPENDED,
      QSPI_READY,
      QSPI_RESET
    } QSPI_StatusTypeDef;

    /* QSPI IT EventFlag */
    typedef union {
      struct {
        uint8_t flag_TE : 1;      /*!< CallBack Register_Flag TE */
        uint8_t flag_TC : 1;      /*!< CallBack Register_Flag TC */
        uint8_t flag_SM : 1;      /*!< CallBack Register_Flag SM */
        uint8_t match_TE : 1;     /*!< Enable Match Register_Flag TE */
        uint8_t match_TC : 1;     /*!< Enable Match Register_Flag TC */
        uint8_t match_SM : 1;     /*!< Enable Match Register_Flag SM */
      } flagBit;
      uint8_t flag;
    } QSPI_IT_EventFlag;

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

    Flash();
    Flash(QSPI_HandleTypeDef *hqspi);
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

protected:
private:

    QSPI_StatusTypeDef BSP_QSPI_Receive(uint8_t *pData, uint32_t Timeout);
    QSPI_StatusTypeDef BSP_QSPI_Transmit(uint8_t *pData, uint32_t Timeout);
    QSPI_StatusTypeDef BSP_QSPI_AutoPolling(QSPI_CommandTypeDef *cmd, QSPI_AutoPollingTypeDef *cfg, uint32_t Timeout);
    QSPI_StatusTypeDef BSP_QSPI_WaitingForEvent(uint32_t Timeout);
    QSPI_StatusTypeDef QSPI_SetDeepPowerDown(void);
    QSPI_StatusTypeDef QSPI_ExitDeepPowerDown(void);
    QSPI_StatusTypeDef QSPI_ResetMemory(void);
    QSPI_StatusTypeDef QSPI_DummyCyclesCfg(void);
    QSPI_StatusTypeDef QSPI_WriteEnableVolat(void);
    QSPI_StatusTypeDef QSPI_WriteEnable(void);
    QSPI_StatusTypeDef QSPI_WriteDisable(void);
    QSPI_StatusTypeDef QSPI_DisableContinuousMode(void);
    QSPI_StatusTypeDef QSPI_AutoPollingMemReady(uint32_t Timeout);

    QSPI_HandleTypeDef *_hqspi;
    inline static QSPI_Info _FlashInfo;
    inline static QSPI_IT_EventFlag *_evtFlag;
};

#endif /* __ETH452_QSPI_H */

