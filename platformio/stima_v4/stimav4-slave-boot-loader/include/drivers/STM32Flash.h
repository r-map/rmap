/**
  ******************************************************************************
  * @file    Bootloader.h
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Bootloader STIMAV4 Slave STM32 HAL_based_function() header file
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

#ifndef __STM32Flash_H
#define __STM32Flash_H

// HAL_Function Include
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"

// ***********************************************************************
//                   BOOTLOADER CONFIGURATION FLAGS
// ***********************************************************************

// Start address of application space in flash
// N.B. Enter Here Application start define value (used as address in Main.c)
#define APP_ADDRESS (uint32_t)0x0800B000

// Calculate Address applicaton limit
#define APP_ROMEND  (uint32_t)(FLASH_BASE + FLASH_SIZE)
#define APP_ROMSIZE (APP_ROMEND - APP_ADDRESS)

// Address of System Memory (ST Bootloader BOOT0 Pin)
#define SYSMEM_ADDRESS (uint32_t)0x1FFF0000

// Remove for disable Serial and relative message from BOOTLOADER (size flash)
// Reduce about 9Kb of flash and fit bootlader into first two sector 32 Kb
// less than 28 Kb. Note: It's need to fit Boot in pages aligned of 2 Kb
#define USE_SERIAL_MESSAGE      (1)

// Enable full check name module for confirm correct firmware file
// Disable for use only check prefix as "stimav4." or "stimav4.module_th"
// Disable for create one file application for slave, otherwise each
// module need to have a single bootloader application.
// Disable check can modify module from one type to other.
// Control name is also performed in Application standard
#define CHECK_FULL_NAME_FW      (0)

// Enable write protection after performing in-app-programming
#define USE_WRITE_PROTECTION    (0)

// Automatically set vector table location before launching application
#define SET_VECTOR_TABLE        (1)

// Automatically set MSP before launching application
#define SET_MSP                 (0)

// Using dual bank flash
#define USE_FLASH_BANK_2        (0)

// ***********************************************************************
//                  Private enumerations and Types
// ***********************************************************************
// Size of application in DWORD (32bits or 4bytes)
#define APP_SIZE (uint32_t)(((END_ADDRESS - APP_ADDRESS) + 3) / 4)

// Number of pages per bank in flash
#define FLASH_PAGE_NBPERBANK (256)

// MCU RAM information (to check whether flash contains valid application)
#define RAM_BASE SRAM1_BASE       // Start address of RAM
#define RAM_SIZE SRAM1_SIZE_MAX   // RAM size in bytes

// Address EEProm for reserved bootloader flag param (and future used param)
#define START_EEPROM_ADDRESS           (0)
#define BOOT_LOADER_STRUCT_ADDR        (START_EEPROM_ADDRESS)
#define BOOT_LOADER_STRUCT_SIZE        (sizeof(bootloader_t))
#define BOOT_LOADER_STRUCT_END         (START_EEPROM_ADDRESS + BOOT_LOADER_STRUCT_SIZE)

// ***********************************************************************
//                  Private enumerations and Types
// ***********************************************************************
// Bootloader error codes
enum eBootloaderErrorCodes
{
    BL_OK = 0,
    BL_NO_APP,
    BL_SIZE_ERROR,
    BL_CHKS_ERROR,
    BL_ERASE_ERROR,
    BL_WRITE_ERROR,
    BL_OBP_ERROR,
    BL_EXT_FLASH_ERROR,
    BL_FILE_NOT_FOUND
};

// Flash Protection Types
enum eFlashProtectionTypes
{
    BL_PROTECTION_NONE  = 0,
    BL_PROTECTION_WRP   = 0x1,
    BL_PROTECTION_RDP   = 0x2,
    BL_PROTECTION_PCROP = 0x4
};

// Function pointer definition typedef
typedef void (*pFunction)(void);

// Backup && Upload Firmware TypeDef
typedef struct
{
  bool request_upload;
  bool backup_executed;
  bool upload_executed;
  bool rollback_executed;
  bool app_executed_ok;
  bool app_forcing_start;
  uint8_t upload_error;
  uint8_t tot_reset;
  uint8_t wdt_reset;
} bootloader_t;

// ***********************************************************************
//                           MACRO DEFINITION
// ***********************************************************************

// Read poiter to data from flash at address (X)
#define STM32Flash_ReadByte(X)  (uint8_t)((*(uint8_t*)X))
// Ending flashing
#define STM32Flash_FlashEnd()   HAL_FLASH_Lock()

// ***********************************************************************
//                          FUNCTION DEFINITION
// ***********************************************************************

void STM32Flash_Init(void);

uint8_t STM32Flash_Erase(uint32_t address);
void    STM32Flash_FlashBegin(uint32_t *flash_ptr, uint32_t address);
uint8_t STM32Flash_FlashNext(uint64_t data, uint32_t *flash_ptr);


uint8_t STM32Flash_GetProtectionStatus(uint32_t address);
uint8_t STM32Flash_ConfigProtection(uint32_t protection, uint32_t address);
uint8_t STM32Flash_CheckSize(uint32_t appsize, uint32_t address);

void STM32Flash_JumpToApplication(uint32_t address);
void STM32Flash_JumpToSysMem(void);

#endif /* __STM32Flash_H */

/************************ (C) COPYRIGHT Digiteco s.r.l. *****END OF FILE****/