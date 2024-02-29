/**
  ******************************************************************************
  * @file    Bootloader.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Bootloader STIMAV4 Master STM32 HAL_based_function() cpp source file
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

#include <Arduino.h>
#include "drivers/module_master_hal.h"
#include "drivers/STM32Flash.h"
#include <IWatchdog.h>

/// @brief  Initializes STM32Flash, and clear flags.
/// @param  None
void STM32Flash_Init(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_FLASH_CLK_ENABLE();

    /* Clear flash flags */
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
    HAL_FLASH_Lock();
}

/// @brief  Erases the user application area in flash (page size is auto calculate)
/// @param  address starting application
/// @return BL_OK ... BL_Error Type with specific error
uint8_t STM32Flash_Erase(uint32_t address)
{
    uint32_t NbrOfPages;
    uint32_t StartPages;
    uint32_t PageError = 0;
    FLASH_EraseInitTypeDef pEraseInit;
    HAL_StatusTypeDef status = HAL_OK;
    #if (USE_FLASH_BANK_2) && defined(FLASH_BANK_2)
    uint32_t NbrOfPages_bank_2 = 0;
    #endif

    HAL_FLASH_Unlock();

    // Get the number of pages to erase and Starting From
    NbrOfPages = (FLASH_BASE + FLASH_SIZE - address) / FLASH_PAGE_SIZE;
    StartPages = (address - FLASH_BASE) / FLASH_PAGE_SIZE;

    #if (USE_FLASH_BANK_2) && defined(FLASH_BANK_2)
    if(NbrOfPages > FLASH_PAGE_NBPERBANK) {
        NbrOfPages_bank_2 = NbrOfPages - FLASH_PAGE_NBPERBANK;
        NbrOfPages = FLASH_PAGE_NBPERBANK - StartPages;
    }
    #endif

    if(status == HAL_OK)
    {
        pEraseInit.Banks     = FLASH_BANK_1;
        pEraseInit.NbPages   = NbrOfPages;
        pEraseInit.Page      = StartPages;
        pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
        status               = HAL_FLASHEx_Erase(&pEraseInit, &PageError);
    }

    #if (USE_FLASH_BANK_2) && defined(FLASH_BANK_2)

    IWatchdog.reload();

    if((status == HAL_OK)&&(NbrOfPages_bank_2))
    {
        pEraseInit.Banks     = FLASH_BANK_2;
        pEraseInit.NbPages   = NbrOfPages_bank_2;
        pEraseInit.Page      = 0;
        pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
        status               = HAL_FLASHEx_Erase(&pEraseInit, &PageError);
    }

    #endif

    HAL_FLASH_Lock();

    return (status == HAL_OK) ? BL_OK : BL_ERASE_ERROR;
}

/// @brief  Begin flash programming, unlocks the flash and sets var for flashing
/// @param  flash_ptr var pointer address to flash in write
/// @param  address address start to flashing
void STM32Flash_FlashBegin(uint32_t *flash_ptr, uint32_t address)
{
    // Reset flash destination and unlock Flash access
    *flash_ptr = address;
    HAL_FLASH_Unlock();
}

/// @brief  Program QWORD data into flash with increments of the data pointer.
/// @param  data 64 bit data chunk to be written into flash 
/// @param  flash_ptr pointer to data writing
/// @return BL_OK ... BL_Error Type with specific error
uint8_t STM32Flash_FlashNext(uint64_t data, uint32_t *flash_ptr)
{
    if(!(*flash_ptr <= (FLASH_BASE + FLASH_SIZE - 8))) {
        HAL_FLASH_Lock();
        return BL_SIZE_ERROR;
    }

    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, *flash_ptr, data) == HAL_OK) {
        // Check Reading Written Data
        if(*(uint64_t*)(*flash_ptr) != data) {
            // Reading data != (Write Error)
            HAL_FLASH_Lock();
            return BL_WRITE_ERROR;
        }
        // Updating address
        *flash_ptr += 8;
    } else {
        // HAL Writing error
        HAL_FLASH_Lock();
        return BL_WRITE_ERROR;
    }

    return BL_OK;
}

/// @brief  Returns the protection status of flash bank
/// @param  address address start to flashing (chckeing area protection)
/// @return Flash protection status ::eFlashProtectionTypes
uint8_t STM32Flash_GetProtectionStatus(uint32_t address)
{
    FLASH_OBProgramInitTypeDef OBStruct = {0};
    uint8_t protection = BL_PROTECTION_NONE;

    HAL_FLASH_Unlock();

    // Bank 1
    OBStruct.PCROPConfig = FLASH_BANK_1;
    OBStruct.WRPArea = OB_WRPAREA_BANK1_AREAA;
    HAL_FLASHEx_OBGetConfig(&OBStruct);
    // Protection PCROP
    if(OBStruct.PCROPEndAddr > OBStruct.PCROPStartAddr) {
        if(OBStruct.PCROPStartAddr >= address) {
            protection |= BL_PROTECTION_PCROP;
        }
    }
    // Protection WR (A)
    if(OBStruct.WRPEndOffset > OBStruct.WRPStartOffset) {
        if((OBStruct.WRPStartOffset * FLASH_PAGE_SIZE + FLASH_BASE) >= address) {
            protection |= BL_PROTECTION_WRP;
        }
    }

    OBStruct.WRPArea = OB_WRPAREA_BANK1_AREAB;
    HAL_FLASHEx_OBGetConfig(&OBStruct);
    // Protection WR (B)
    if(OBStruct.WRPEndOffset > OBStruct.WRPStartOffset) {
        if((OBStruct.WRPStartOffset * FLASH_PAGE_SIZE + FLASH_BASE) >= address) {
            protection |= BL_PROTECTION_WRP;
        }
    }

    #if (USE_FLASH_BANK_2) && defined(FLASH_BANK_2)

    // Bank 2
    OBStruct.PCROPConfig = FLASH_BANK_2;
    OBStruct.WRPArea     = OB_WRPAREA_BANK2_AREAA;
    HAL_FLASHEx_OBGetConfig(&OBStruct);
    // Protection PCROP
    if(OBStruct.PCROPEndAddr > OBStruct.PCROPStartAddr) {
        if(OBStruct.PCROPStartAddr >= address) {
            protection |= BL_PROTECTION_PCROP;
        }
    }
    // Protection WR (A)
    if(OBStruct.WRPEndOffset > OBStruct.WRPStartOffset) {
        if((OBStruct.WRPStartOffset * FLASH_PAGE_SIZE + FLASH_BASE +
            FLASH_PAGE_SIZE * FLASH_PAGE_NBPERBANK) >= address) {
            protection |= BL_PROTECTION_WRP;
        }
    }

    OBStruct.WRPArea = OB_WRPAREA_BANK2_AREAB;
    HAL_FLASHEx_OBGetConfig(&OBStruct);
    // Protection WR (B)
    if(OBStruct.WRPEndOffset > OBStruct.WRPStartOffset) {
        if((OBStruct.WRPStartOffset * FLASH_PAGE_SIZE + FLASH_BASE +
            FLASH_PAGE_SIZE * FLASH_PAGE_NBPERBANK) >= address) {
            protection |= BL_PROTECTION_WRP;
        }
    }

    #endif

    // RDP
    if(OBStruct.RDPLevel != OB_RDP_LEVEL_0) {
        protection |= BL_PROTECTION_RDP;
    }

    HAL_FLASH_Lock();
    return protection;
}

/// @brief  Configures the wirte protection of bank flash
/// @param  protection type of protection required
/// @param  address address start to flashing (chckeing area protection)
/// @return BL_OK ... BL_Error Type with specific error
uint8_t STM32Flash_ConfigProtection(uint32_t protection, uint32_t address)
{
    FLASH_OBProgramInitTypeDef OBStruct = {0};
    bool statusCmd = false;

    statusCmd |= (HAL_FLASH_Unlock()!= HAL_OK);
    statusCmd |= (HAL_FLASH_OB_Unlock()!= HAL_OK);

    // Bank 1
    OBStruct.WRPArea    = OB_WRPAREA_BANK1_AREAA;
    OBStruct.OptionType = OPTIONBYTE_WRP;
    if(protection & BL_PROTECTION_WRP)
    {
        // Enable protection for application space
        OBStruct.WRPStartOffset = (address - FLASH_BASE) / FLASH_PAGE_SIZE;
        OBStruct.WRPEndOffset   = FLASH_PAGE_NBPERBANK - 1;
    }
    else
    {
        // Remove protection
        OBStruct.WRPStartOffset = 0xFF;
        OBStruct.WRPEndOffset   = 0x00;
    }
    statusCmd |= (HAL_FLASHEx_OBProgram(&OBStruct)!= HAL_OK);

    // Area B is not used
    OBStruct.WRPArea        = OB_WRPAREA_BANK1_AREAB;
    OBStruct.OptionType     = OPTIONBYTE_WRP;
    OBStruct.WRPStartOffset = 0xFF;
    OBStruct.WRPEndOffset   = 0x00;
    statusCmd |= (HAL_FLASHEx_OBProgram(&OBStruct)!= HAL_OK);

    #if (USE_FLASH_BANK_2) && defined(FLASH_BANK_2)

    // Bank 2
    OBStruct.WRPArea    = OB_WRPAREA_BANK2_AREAA;
    OBStruct.OptionType = OPTIONBYTE_WRP;
    if(protection & BL_PROTECTION_WRP)
    {
        // Enable protection for application space
        OBStruct.WRPStartOffset = 0x00;
        OBStruct.WRPEndOffset   = FLASH_PAGE_NBPERBANK - 1;
    }
    else
    {
        // Remove protection
        OBStruct.WRPStartOffset = 0xFF;
        OBStruct.WRPEndOffset   = 0x00;
    }
    statusCmd |= (HAL_FLASHEx_OBProgram(&OBStruct)!= HAL_OK);

    // Area B is not used
    OBStruct.WRPArea        = OB_WRPAREA_BANK2_AREAB;
    OBStruct.OptionType     = OPTIONBYTE_WRP;
    OBStruct.WRPStartOffset = 0xFF;
    OBStruct.WRPEndOffset   = 0x00;
    statusCmd |= (HAL_FLASHEx_OBProgram(&OBStruct)!= HAL_OK);
    #endif

    if(statusCmd == false)
    {
        // Loading Flash Option Bytes
        statusCmd |= (HAL_FLASH_OB_Launch()!= HAL_OK);
    }

    statusCmd |= (HAL_FLASH_OB_Lock()!= HAL_OK);
    statusCmd |= (HAL_FLASH_Lock()!= HAL_OK);

    return (statusCmd == false) ? BL_OK : BL_OBP_ERROR;
}

/// @brief  Checks size for new application if fits into flash.
/// @param  appsize size app
/// @param  address address start to flashing (chckeing size)
/// @return BL_OK ... BL_Error Type with specific error
uint8_t STM32Flash_CheckSize(uint32_t appsize, uint32_t address)
{
    return ((FLASH_BASE + FLASH_SIZE) >= (appsize + address)) ? BL_OK
                                                              : BL_SIZE_ERROR;
}

/// @brief Jump to the user application in flash with deInit Peripheral
/// @param address Address of application
void STM32Flash_JumpToApplication(uint32_t address)
{
    uint32_t JumpAddress = *(__IO uint32_t*)(address + 4);
    pFunction Jump       = (pFunction)JumpAddress;

    // Close && Flush Driver and Deinit App
    #if USE_SERIAL_MESSAGE
    Serial.flush();
    Serial.end();
    #endif
    Wire.end();
    HAL_QSPI_MspDeInit(&hqspi);
    HAL_RCC_DeInit();
    HAL_DeInit();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    #if(SET_VECTOR_TABLE)
    SCB->VTOR = address;
    #endif

    #if(SET_MSP)
    __set_MSP(*(__IO uint32_t*)address);
    #endif

    Jump();
}

/// @brief  Jump to MCU System Memory (ST Bootloader) in flash with deInit Peripheral
/// @param  None
void STM32Flash_JumpToSysMem(void)
{
    uint32_t JumpAddress = *(__IO uint32_t*)(SYSMEM_ADDRESS + 4);
    pFunction Jump       = (pFunction)JumpAddress;

    // Close && Flush Driver and Deinit App
    #if USE_SERIAL_MESSAGE
    Serial.flush();
    Serial.end();
    #endif
    Wire.end();
    HAL_QSPI_MspDeInit(&hqspi);
    HAL_RCC_DeInit();
    HAL_DeInit();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();

    #if(SET_MSP)
    __set_MSP(*(__IO uint32_t*)SYSMEM_ADDRESS);
    #endif

    Jump();
}
