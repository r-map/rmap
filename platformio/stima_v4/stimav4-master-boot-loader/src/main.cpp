/**
  ******************************************************************************
  * @file    main.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Bootloader Application for StimaV4 Master
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

  Descrizione Funzionamento del BOOT Loader:

  In avvio parte il bootLoader (questo applicativo) Se non necessario upload
  viene avviato l'applicativo standard APP all' indirizzo conosciuto con Jump

  Descrizione:
  All'avvio viene controllato un Bytes Flag della EEProm (settato dall'applicazione CAN)
  e un secondo flag che contiene le informazioni di firmware first boot OK

  Se il primo flag contiene un valore di aggiornamento allora viene controllato
  la presenza del file di firmware con il nome e versione e spazio consentito...
  Se OK continua, altrimenti esce aggiornando i flag di BOOT Loader annullando l'operazione

  se tutto OK, parte il backup del firmware corrente salvato in zona Flash conosciuta
  Se OK continua, altrimenti esce aggiornando i flag di BOOT Loader annullando l'operazione
  A questo punto viene resettato il flag di APP_EXECUTED_OK (applicazione run OK)
  che indica se il programma si è avviato correttamente. Questo flag unito ai precedenti
  comunica al bootloader in caso di riavvio anomalo che qualcosa è andato KO durante
  il flashing o che il programma caricato non ha effettuato un avvio corretto
  In questa condizione parte il rollback alla versione precedente salvata prima
  del caricamento del nuovo firmware

  Il boot finisce e parte l'applicativo come sopra jump + vectorTable setup...

  Dall'applicativo principale se l'avvio è corretto viene resettato il flag
  di first bootloader, altrimenti se tutto non parte correttamente e si riavvia
  il boot loader, boot loader troverà questo flag attivo ed effetuerà l'operazione di
  rollback del firmware precedente... (Descritto in precedenza)

*/

#include <Arduino.h>
#include <IWatchdog.h>
#include <strings.h>
// HW Configuration and module driver
#include "drivers/module_master_hal.h"
#include "drivers/flash.h"
#include "drivers/eeprom.h"
#include "drivers/STM32Flash.h"

// HW Class Variables access
Flash       memFlash;
EEprom      memEprom;

/// @brief Init qspi External Flash
/// @param  None
/// @return True if init OK -> Flash::QSPI_OK
bool qspiFlash_Init(void)
{
    // Init if required (DeInit after if required PowerDown Module)
    if(memFlash.BSP_QSPI_Init() != Flash::QSPI_OK) {
        return false;
    }
    // Check Status Flash OK
    if (memFlash.BSP_QSPI_GetStatus()) {
        return false;
    }
    return true;
}

/// @brief Scrive dati in append su Flash per scrittura sequenziale file backup firmware attuale
///        (Utilizzzato come backup Firmware corrente... RollBack se avvio non conforme)
/// @param block blocco corrente (0 = Starting Block... Init Flash ecc...)
/// @param buf blocco dati da scrivere in formato UAVCAN [256 Bytes]
/// @param count numero del blocco da scrivere [Blocco x Buffer 256 Bytes]
/// @return true se blocco OK, false per qualsiasi errore (abort upload...)
bool putBackupBlock(uint16_t block, void* buf, size_t count, uint32_t *flashPtr, uint16_t *flashBlock)
{
    #ifdef CHECK_FLASH_WRITE
    // check data (W->R) Verify Flash integrity OK    
    uint8_t check_data[256];
    #endif
    // Request New File Init Upload
    if(block==0) {
        // Start From PtrFlash Backup (acces direct)
        *flashPtr = FLASH_FW_BACKUP;
        // Get Block Current into Flash
        *flashBlock = *flashPtr / AT25SF641_BLOCK_SIZE;
        // Erase First Block Block (Block OF 4KBytes)
        if (memFlash.BSP_QSPI_Erase_Block(*flashBlock)) {
            #if USE_SERIAL_MESSAGE
            printf("FLASH: Erase block id:%d error\r\n", *flashBlock);
            #endif
            return false;
        } else {
            // Write into Flash
            #if USE_SERIAL_MESSAGE
            printf("FLASH: Erasing/Writing operation started...\r\n");
            #endif
        }
    }
    // Write Data Block if data request present
    if(count!=0) {
        // Starting Write at OFFSET Required... Erase here is Done
        if (memFlash.BSP_QSPI_Write((uint8_t*)buf, *flashPtr, count)) {
            #if USE_SERIAL_MESSAGE
            printf("FLASH: Write block at address:%d error\r\n", *flashPtr);
            #endif
            return false;
        }
        #ifdef CHECK_FLASH_WRITE
        memFlash.BSP_QSPI_Read(check_data, *flashPtr, count);
        if(memcmp(buf, check_data, count)!=0) {
            #if USE_SERIAL_MESSAGE
            printf("FLASH: verifying data check ERROR\r\n");
            #endif
            return false;
        }
        #endif
        *flashPtr += count;
        // Check if Next Page Addressed (For Erase Next Block)
        if((*flashPtr / AT25SF641_BLOCK_SIZE) != *flashBlock) {
            *flashBlock = *flashPtr / AT25SF641_BLOCK_SIZE;
            // Erase Next Block (Block OF 4KBytes)
            if (memFlash.BSP_QSPI_Erase_Block(*flashBlock)) {
                #if USE_SERIAL_MESSAGE
                printf("FLASH: Erase block id:%d error\r\n", *flashBlock);
                #endif
                return false;
            }
        }
    }
    return true;
}

/// @brief GetInfo per Firmware File archiviato su Flash (se esiste)
/// @param version version firmware
/// @param revision revision firmware
/// @param len lunghezza del file in bytes
/// @return true se il file esiste
bool getInfoFwFile(uint8_t *version, uint8_t *revision, uint64_t *len)
{
    uint8_t block[FLASH_FILE_SIZE_LEN];
    bool fileReady = false;

    // Init if required (DeInit after if required PowerDown Module)
    if(memFlash.BSP_QSPI_Init() != Flash::QSPI_OK) {
        return false;
    }
    // Check Status Flash OK
    if (memFlash.BSP_QSPI_GetStatus()) {
        return false;
    }

    // Read Name file, Version and Info
    memFlash.BSP_QSPI_Read(block, 0, FLASH_FILE_SIZE_LEN);
    #if CHECK_FULL_NAME_FW
    if(strncasecmp((char*)block, NODE_NAME, sizeof(NODE_NAME))) {
    #else
    if(strncasecmp((char*)block, NODE_NAME_PREFIX, sizeof(NODE_NAME_PREFIX))) {
    #endif
        uint8_t position = strlen((char*)block);
        *version = block[strlen((char*)block) - 11] - 48;
        *revision = block[strlen((char*)block) - 9] - 48;
        memFlash.BSP_QSPI_Read((uint8_t*)len, FLASH_SIZE_ADDR(0), FLASH_INFO_SIZE_U64);
        fileReady = true;
    }

    return fileReady;
}

/// @brief Legge un blocco file (memory) dalla Flash
/// @param address Indirizzo di lettura
/// @param block blocco dati
/// @param len lunghezza da leggere
/// @return true se lettura ok
bool getFileBlock(uint32_t address, uint8_t *block, uint16_t len)
{
    // Read Name file, Version and Info
    return(memFlash.BSP_QSPI_Read(block, address, len) == Flash::QSPI_OK);
}

// Setup Wire I2C Interface
void init_wire()
{
#if (ENABLE_I2C1)
  Wire.begin();
  Wire.setClock(I2C1_BUS_CLOCK_HZ);
#endif

#if (ENABLE_I2C2)
  Wire2.begin();
  Wire2.setClock(I2C2_BUS_CLOCK_HZ);
#endif
}

// *********************************************************************************************
//                                       SETUP AMBIENTE
// *********************************************************************************************
void setup(void) {

    // STARTUP PRIVATE BASIC HARDWARE CONFIG AND ISTANCE
    SetupSystemPeripheral();
    init_wire();
    delay(100);

    #if USE_SERIAL_MESSAGE
    Serial.begin(SERIAL_DEBUG_BAUD_RATE);
    // Wait for serial port to connect
    while (!Serial) {}
    printf("\r\nStart StimaV4 Bootloader Ver %d.%d Rev %d\r\n",
        VERSION_MAJOR, VERSION_MINOR, REVISION);
    #endif
}

// *************************************************************************************************
//                                          MAIN LOOP
// *************************************************************************************************
void loop(void)
{
    // Struct Boot Check EEPROM Fields
    bootloader_t boot_request;
    // External Flash Var
    uint32_t qspiWritePtr;
    uint32_t qspiReadPtr;
    uint16_t qspiFlashBlock;
    // RollBack Var
    uint8_t memBlock[256];
    uint8_t memBlockIndex = 0;
    uint16_t pageFlashIndex = 0;
    bool is_rollback = false;
    // Flashing Var
    uint32_t writeFlashPtr;
    uint32_t lenghtFlashPtr;
    bool end_flashing = false;    
    uint8_t eor_ctrl = 0;
    // Info Version Fw
    uint8_t version_fw, revision_fw;
    uint64_t tot_bytes;
    // Flashing procedure
    uint8_t status;
    uint64_t data;
    uint32_t cntr;
    // Reset event from WDT
    bool wdtResetEvent = false;

	// Control WDT Executing Reset
    if (IWatchdog.isReset(true)) {
        #if USE_SERIAL_MESSAGE
        printf("WatchDOG: signal received... something are wrong\r\n");
        #endif
        // Clear flag WDT
        IWatchdog.clearReset();
        wdtResetEvent = true;
    }

    // Init the watchdog timer with 10 seconds timeout (No need fast flashing operation)
    // Wdt Timescaler can modified from External Application to make freq. to fast
    IWatchdog.begin(10000000);

    // Setup EEprom and Flash memory Access
    memEprom=EEprom(&Wire2);

    // Reading structure EEProm Boot Request/Rollback procedure
    if(!memEprom.Read(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_request, sizeof(boot_request)))
    {
        #if USE_SERIAL_MESSAGE
        printf("Reading E2Prom external fail. Starting APP...\r\n");
        #endif
        // Run application...
        STM32Flash_JumpToApplication(APP_ADDRESS);
    }

    // Found Try to Start immediatly (if RESET Occurs before Starting APP)
    if(boot_request.app_forcing_start) {
        // Try one time without changing saving other flags
        boot_request.app_forcing_start = false;
        boot_request.rollback_executed = false;
        memEprom.Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_request, sizeof(boot_request));
        #if USE_SERIAL_MESSAGE
        printf("Found flag, Forcing Starting APP...\r\n");
        #endif
        // Run application...
        STM32Flash_JumpToApplication(APP_ADDRESS);
    }

    // Increment num of reset and WDT (if < MAX_VALUE UINT_8)
    if(boot_request.tot_reset != 0XFF) boot_request.tot_reset++;
    if((wdtResetEvent)&&(boot_request.wdt_reset != 0xFF)) boot_request.wdt_reset++;

    #if USE_SERIAL_MESSAGE
    printf("Number of Reboot: [ %d ] , WathcDog [ %d ]\r\n", boot_request.tot_reset, boot_request.wdt_reset);
    #endif

    // If recived WDT Reset event perform Field for check RollBack...
    // wdtResetEvent with check boot_request.app_executed_ok = false
    if((wdtResetEvent)&&(boot_request.app_executed_ok == true)) {
        // Event is External Flashing procedure...
        // Starting APP Directly
        // Save WDT and Reboot flag
        memEprom.Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_request, sizeof(boot_request));
        #if USE_SERIAL_MESSAGE
        printf("WatchDOG: signal not depending from flashing procedure. Starting APP...\r\n");
        #endif
        // Run application...
        STM32Flash_JumpToApplication(APP_ADDRESS);
    }

    // If need to flashing firmware ( or checking backup if reset)
    if(boot_request.request_upload || wdtResetEvent) {

        #if USE_SERIAL_MESSAGE
        if(wdtResetEvent)
            printf("WatchDOG: Starting APP failed check rollback...\r\n");
        else
            printf("FLAGS: found flag require flashing update...\r\n");
        #endif

        // Init Flash external object
        memFlash=Flash(&hqspi);
        if(qspiFlash_Init()) {
            #if USE_SERIAL_MESSAGE
            printf("Loading external flash module ok...\r\n");
            #endif
        } else {
            // Save to E2Prom the ErrorCode
            boot_request.backup_executed = false;
            boot_request.request_upload = false;
            boot_request.upload_executed = false;
            boot_request.upload_error = (uint8_t) eBootloaderErrorCodes::BL_EXT_FLASH_ERROR;
            memEprom.Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_request, sizeof(boot_request));
            #if USE_SERIAL_MESSAGE
            printf("Loading memory module error abort flashing...\r\n");
            #endif
            // Run application...
            STM32Flash_JumpToApplication(APP_ADDRESS);
        }

        // Loading Variables File Address, check is found
        if(!getInfoFwFile(&version_fw, &revision_fw, &tot_bytes)) {
            // Save to E2Prom the ErrorCode
            boot_request.backup_executed = false;
            boot_request.request_upload = false;
            boot_request.upload_executed = false;
            boot_request.upload_error = (uint8_t) eBootloaderErrorCodes::BL_FILE_NOT_FOUND;
            memEprom.Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_request, sizeof(boot_request));
            #if USE_SERIAL_MESSAGE
            printf("File flash not found, abort flashing...\r\n");
            #endif
            // Run application...
            STM32Flash_JumpToApplication(APP_ADDRESS);
        }

        // Check area fit for program
        if(STM32Flash_CheckSize(APP_ADDRESS, tot_bytes) != eBootloaderErrorCodes::BL_OK) {
            // Save to E2Prom the ErrorCode
            boot_request.backup_executed = false;
            boot_request.request_upload = false;
            boot_request.upload_executed = false;
            boot_request.upload_error = (uint8_t) eBootloaderErrorCodes::BL_SIZE_ERROR;
            memEprom.Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_request, sizeof(boot_request));
            #if USE_SERIAL_MESSAGE
            printf("File flash overflow size area programming, abort flashing...\r\n");
            #endif
            // Run application...
            STM32Flash_JumpToApplication(APP_ADDRESS);
        }

        // Condition program APP Not Started correctly...
        // Need to RollBack
        if((boot_request.app_executed_ok == false)&&
           (boot_request.backup_executed == true)&&
           (boot_request.upload_executed == true))
        {
            #if USE_SERIAL_MESSAGE
            printf("FLAGS: found flag APP Not started correctly, Rollback flashing...\r\n");
            #endif
            is_rollback = true;
        }
        else
        {
            #if USE_SERIAL_MESSAGE
            printf("Starting procedure flashing...\r\n");
            #endif
        }

        // Perform a backup flash actual program (prepare for rollBack...)
        if(!boot_request.backup_executed) {
            #if USE_SERIAL_MESSAGE
            printf("Backup firmware is required. Starting procedure...\r\n");
            #endif
            // Backup Flash actual Program for RollBack Function Flash Upload error or uncorrect
            // Need to work with WatchDog For securty control Running Firmware Uploaded
            for(u_int32_t memPtr = APP_ADDRESS; memPtr < APP_ROMEND; memPtr++) {
                memBlock[memBlockIndex++] = STM32Flash_ReadByte(memPtr);
                if(memBlockIndex == 0) {
                    putBackupBlock(pageFlashIndex++, memBlock, 0x100, &qspiWritePtr, &qspiFlashBlock);
                    IWatchdog.reload();
                }
            }
            if(memBlockIndex!=0) {
                putBackupBlock(pageFlashIndex++, memBlock, memBlockIndex, &qspiWritePtr, &qspiFlashBlock);
            }
            // Save to E2Prom
            boot_request.backup_executed = true;
            memEprom.Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_request, sizeof(boot_request));
            #if USE_SERIAL_MESSAGE
            printf("Backup firmware checked and completed...\r\n");
            #endif
        }

        // Reload WDT After backup
        IWatchdog.reload();

        // Start Flashing procedure (RollBack Or Flashing)

        // Check for flash write protection && Disable
        if(STM32Flash_GetProtectionStatus(APP_ADDRESS) & BL_PROTECTION_WRP)
        {
            #if USE_SERIAL_MESSAGE
            printf("Disable flash WR protection flag...\r\n");
            #endif
            STM32Flash_ConfigProtection(BL_PROTECTION_NONE, APP_ADDRESS);
        }

        // Init Bootloader and Flash
        STM32Flash_Init();

        // Erase Flash
        #if USE_SERIAL_MESSAGE
        printf("Erasing flash...\r\n");
        #endif
        if(STM32Flash_Erase(APP_ADDRESS) == eBootloaderErrorCodes::BL_OK) {
            #if USE_SERIAL_MESSAGE
            printf("Erasing flash done\r\n");
            #endif
        } else {
            #if USE_SERIAL_MESSAGE
            printf("Erasing flash error. Quit flashing...\r\n");
            #endif
            // Save to E2Prom
            boot_request.upload_error = (uint8_t)eBootloaderErrorCodes::BL_ERASE_ERROR;
            memEprom.Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_request, sizeof(boot_request));
            // Run application...
            STM32Flash_JumpToApplication(APP_ADDRESS);
        }

        IWatchdog.reload();

        // Programming Flash
        #if USE_SERIAL_MESSAGE
        printf("Starting programming flash...\r\n");
        #endif
        cntr = 0;
        STM32Flash_FlashBegin(&writeFlashPtr, APP_ADDRESS);

        // Set address start for load data from external Flash QSPI
        if(is_rollback) {
            // Fixed Address Param
            qspiReadPtr = FLASH_FW_BACKUP;
            lenghtFlashPtr = APP_ROMSIZE;
        } else {
            // Print loaded Variables File Address
            #if USE_SERIAL_MESSAGE
            printf("Upload firmware to Ver %d.%d, Total bytes %lu\r\n",
                version_fw, revision_fw, (uint32_t)tot_bytes);
            #endif
            // Address Var Source Read (After INFO_SIZE 1 PAGE OF External Flash QSPI)
            qspiReadPtr = FLASH_FW_POSITION + FLASH_INFO_SIZE_LEN;
            lenghtFlashPtr = tot_bytes;
        }

        // Reload WDT After erasing flash
        IWatchdog.reload();

        // *************************************************************
        //                       FLASHING MEMORY
        // *************************************************************
        tot_bytes = 0;
        do {
            // Read block data from External Flash
            if(getFileBlock(qspiReadPtr, (uint8_t*)&data, 8)) {
                tot_bytes += 8;
                qspiReadPtr += 8;
                if(tot_bytes >= lenghtFlashPtr) end_flashing = true;
                // EndOfRom Only RollBack... 0xFF REPETED 512 Times
                if(is_rollback) {
                    if(data == 0xFFFFFFFFFFFFFFFFULL) eor_ctrl++;
                    else eor_ctrl = 0;
                    if(eor_ctrl>=64u) end_flashing = true;
                }
                status = STM32Flash_FlashNext(data, &writeFlashPtr);
                IWatchdog.reload();
                if(status == BL_OK) {
                    cntr++;
                } else  {
                    #if USE_SERIAL_MESSAGE
                    printf("Programming error at: %lu byte\r\n", (cntr * 8));
                    #endif
                }
            }
        } while((status == BL_OK) && (end_flashing == false));
        // *************************************************************
        //                         END FLASHING
        // *************************************************************
        
        // Copy status Error BL_OK -> NO ERROR
        boot_request.upload_error = status;

        // End Programming
        STM32Flash_FlashEnd();
        
        #if USE_SERIAL_MESSAGE
        printf("Programming finished, flashed: %lu bytes.\r\n", (cntr * 8));
        #endif

        // Save to E2Prom Struct BOOTLoader
        if(is_rollback) {
            // Roll back disable request_upload
            boot_request.request_upload = false;
            boot_request.upload_executed = false;
            boot_request.rollback_executed = true;
            boot_request.app_executed_ok = true;
        } else {
            // Not modify Here request upload (only from APP)
            // boot_request.request_upload = false;
            boot_request.upload_executed = true;
            boot_request.rollback_executed = false;
            boot_request.app_executed_ok = false; // True from APP... IF Starting OK
            // Signal Flashing now (only to check on Reboot Try Start after flashing...)
            boot_request.app_forcing_start = true;
        }
        // Preformed down (as Starting APP... Normal Mode)
        memEprom.Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_request, sizeof(boot_request));
        STM32Flash_JumpToApplication(APP_ADDRESS);

    } else {
        
        // No required update...
        #if USE_SERIAL_MESSAGE
        printf("Normal operation, ");
        #endif

    }

    // Starting APP
    #if USE_SERIAL_MESSAGE
    printf("Starting application...\r\n");
    #endif

    // Save WDT and Reboot flag
    memEprom.Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_request, sizeof(boot_request));
    STM32Flash_JumpToApplication(APP_ADDRESS);
}