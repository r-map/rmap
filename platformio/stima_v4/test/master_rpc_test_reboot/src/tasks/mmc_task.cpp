/**
  ******************************************************************************
  * @file    mmc_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   mmc_task source file (SDMMC StimaV4)
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

#define TRACE_LEVEL     MMC_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   MMC_TASK_ID

#include "tasks/mmc_task.h"

#if (ENABLE_MMC)

using namespace cpp_freertos;

MmcTask::MmcTask(const char *taskName, uint16_t stackSize, uint8_t priority, MmcParam_t mmcParam) : Thread(taskName, stackSize, priority), param(mmcParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(MMC_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  // Init val
  mmcFlashPtr = 0;
  mmcFlashBlock = 0;

  state = MMC_STATE_INIT;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void MmcTask::TaskMonitorStack()
{
  u_int16_t stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark( NULL );
  if((stackUsage) && (stackUsage < param.system_status->tasks[LOCAL_TASK_ID].stack)) {
    param.systemStatusLock->Take();
    param.system_status->tasks[LOCAL_TASK_ID].stack = stackUsage;
    param.systemStatusLock->Give();
  }
}
#endif

/// @brief local watchDog and Sleep flag Task (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void MmcTask::TaskWatchDog(uint32_t millis_standby)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Update WDT Signal (Direct or Long function Timered)
  if(millis_standby)  
  {
    // Check 1/2 Freq. controller ready to WDT only SET flag
    if((millis_standby) < WDT_CONTROLLER_MS / 2) {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
    } else {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::timer;
      // Add security milimal Freq to check
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog_ms = millis_standby + WDT_CONTROLLER_MS;
    }
  }
  else
    param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
  param.systemStatusLock->Give();
}

/// @brief local suspend flag and positor running state Task (optional)
/// @param state_position Sw_Position (Local STATE)
/// @param state_subposition Sw_SubPosition (Optional Local SUB_STATE Position Monitor)
/// @param state_operation operative mode flag status for this task
void MmcTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Signal Task sleep/disabled mode from request (Auto SET WDT on Resume)
  if((param.system_status->tasks[LOCAL_TASK_ID].state == task_flag::suspended)&&
     (state_operation==task_flag::normal))
     param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
  param.system_status->tasks[LOCAL_TASK_ID].state = state_operation;
  param.system_status->tasks[LOCAL_TASK_ID].running_pos = state_position;
  param.system_status->tasks[LOCAL_TASK_ID].running_sub = state_subposition;
  param.systemStatusLock->Give();
}

/// @brief Scrive dati in append su Flash per scrittura sequenziale file data remoto
/// @param file_name nome del file UAVCAN
/// @param is_firmware true se il file +-è di tipo firmware
/// @param rewrite true se necessaria la riscrittura del file
/// @param buf blocco dati da scrivere in formato UAVCAN [256 Bytes]
/// @param count numero del blocco da scrivere in formato UAVCAN [Blocco x Buffer]
/// @return true if block saved OK, false on any error
bool MmcTask::putFlashFile(const char* const file_name, const bool is_firmware, const bool rewrite, void* buf, size_t count)
{
    #ifdef CHECK_FLASH_WRITE
    // check data (W->R) Verify Flash integrity OK    
    uint8_t check_data[FLASH_BUFFER_SIZE];
    #endif
    // Request New File Init Upload
    if(rewrite) {
        // Qspi Security Semaphore
        if(param.qspiLock->Take(Ticks::MsToTicks(FLASH_SEMAPHORE_MAX_WAITING_TIME_MS))) {
            // Init if required (DeInit after if required PowerDown Module)
            if(param.flash->BSP_QSPI_Init() != Flash::QSPI_OK) {
                param.qspiLock->Give();
                return false;
            }
            // Check Status Flash OK
            Flash::QSPI_StatusTypeDef sts = param.flash->BSP_QSPI_GetStatus();
            if (sts) {
                param.qspiLock->Give();
                return false;
            }
            // Start From PtrFlash 0x100 (Reserve 256 Bytes For InfoFile)
            if (is_firmware) {
                // Firmware Flash
                mmcFlashPtr = FLASH_FW_POSITION;
            } else {
                // Standard File Data Upload
                mmcFlashPtr = FLASH_FILE_POSITION;
            }
            // Get Block Current into Flash
            mmcFlashBlock = mmcFlashPtr / AT25SF641_BLOCK_SIZE;
            // Erase First Block Block (Block OF 4KBytes)
            TRACE_INFO_F(F("FLASH: Erase block: %d\n\r"), mmcFlashBlock);
            if (param.flash->BSP_QSPI_Erase_Block(mmcFlashBlock)) {
                param.qspiLock->Give();
                return false;
            }
            // Write Name File (Size at Eof...)
            uint8_t file_flash_name[FLASH_FILE_SIZE_LEN] = {0};
            memcpy(file_flash_name, file_name, strlen(file_name));
            param.flash->BSP_QSPI_Write(file_flash_name, mmcFlashPtr, FLASH_FILE_SIZE_LEN);
            // Write into Flash
            TRACE_INFO_F(F("FLASH: Write [ %d ] bytes at addr: %d\n\r"), FLASH_FILE_SIZE_LEN, mmcFlashPtr);
            #ifdef CHECK_FLASH_WRITE
            param.flash->BSP_QSPI_Read(check_data, mmcFlashPtr, FLASH_FILE_SIZE_LEN);
            if(memcmp(file_flash_name, check_data, FLASH_FILE_SIZE_LEN)==0) {
                TRACE_INFO_F(F("FLASH: Reading check OK\n\r"));
            } else {
                TRACE_ERROR_F(F("FLASH: Reading check ERROR\n\r"));
                param.qspiLock->Give();
                return false;
            }
            #endif
            // Start Page...
            mmcFlashPtr += FLASH_INFO_SIZE_LEN;
            param.qspiLock->Give();
        }
    }
    // Write Data Block
    // Qspi Security Semaphore
    if(param.qspiLock->Take(Ticks::MsToTicks(FLASH_SEMAPHORE_MAX_WAITING_TIME_MS))) {
        // 0 = Is UavCan Signal EOF for Last Block Exact Len 256 Bytes...
        // If Value Count is 0 no need to Write Flash Data (Only close Fule Info)
        if(count!=0) {
            // Write into Flash
            TRACE_INFO_F(F("FLASH: Write [ %d ] bytes at addr: %d\n\r"), count, mmcFlashPtr);
            // Starting Write at OFFSET Required... Erase here is Done
            param.flash->BSP_QSPI_Write((uint8_t*)buf, mmcFlashPtr, count);
            #ifdef CHECK_FLASH_WRITE
            param.flash->BSP_QSPI_Read(check_data, mmcFlashPtr, count);
            if(memcmp(buf, check_data, count)==0) {
                TRACE_INFO_F(F("FLASH: Reading check OK\n\r"));
            } else {
                TRACE_ERROR_F(F("FLASH: Reading check ERROR\n\r"));
                param.qspiLock->Give();
                return false;
            }
            #endif
            mmcFlashPtr += count;
            // Check if Next Page Addressed (For Erase Next Block)
            if((mmcFlashPtr / AT25SF641_BLOCK_SIZE) != mmcFlashBlock) {
                mmcFlashBlock = mmcFlashPtr / AT25SF641_BLOCK_SIZE;
                // Erase First Block Block (Block OF 4KBytes)
                TRACE_INFO_F(F("FLASH: Erase block: %d\n\r"), mmcFlashBlock);
                if (param.flash->BSP_QSPI_Erase_Block(mmcFlashBlock)) {
                    param.qspiLock->Give();
                    return false;
                }
            }
        }
        // Eof if != 256 Bytes Write
        if(count!=0x100) {
            // Write Info File for Closing...
            // Size at 
            uint64_t lenghtFile = mmcFlashPtr - FLASH_INFO_SIZE_LEN;
            if (is_firmware) {
                // Firmware Flash
                mmcFlashPtr = FLASH_FW_POSITION;
            } else {
                // Standard File Data Upload
                mmcFlashPtr = FLASH_FILE_POSITION;
            }
            param.flash->BSP_QSPI_Write((uint8_t*)&lenghtFile, FLASH_SIZE_ADDR(mmcFlashPtr), FLASH_INFO_SIZE_U64);
            // Write into Flash
            TRACE_INFO_F(F("FLASH: Write [ %d ] bytes at addr: %d\n\r"), FLASH_INFO_SIZE_U64, mmcFlashPtr);
            #ifdef CHECK_FLASH_WRITE
            param.flash->BSP_QSPI_Read(check_data, FLASH_SIZE_ADDR(mmcFlashPtr), FLASH_INFO_SIZE_U64);
            if(memcmp(&lenghtFile, check_data, FLASH_INFO_SIZE_U64)==0) {
                TRACE_INFO_F(F("FLASH: Reading check OK\n\r"));
            } else {
                TRACE_INFO_F(F("FLASH: Reading check ERROR\n\r"));
            }
            #endif
        }
        param.qspiLock->Give();
    }
    return true;
}

/// @brief GetInfo for Firmware File on Flash
/// @param module_type type module of firmware
/// @param version version firmware
/// @param revision revision firmware
/// @param len length of file in bytes
/// @return true if exixst
bool MmcTask::getFlashFwInfoFile(uint8_t *module_type, uint8_t *version, uint8_t *revision, uint64_t *len)
{
    uint8_t block[FLASH_FILE_SIZE_LEN];
    bool fileReady = false;

    // Qspi Security Semaphore
    if(param.qspiLock->Take(Ticks::MsToTicks(FLASH_SEMAPHORE_MAX_WAITING_TIME_MS))) {
        // Init if required (DeInit after if required PowerDown Module)
        if(param.flash->BSP_QSPI_Init() != Flash::QSPI_OK) {
            param.qspiLock->Give();
            return false;
        }
        // Check Status Flash OK
        if (param.flash->BSP_QSPI_GetStatus()) {
            param.qspiLock->Give();
            return false;
        }

        // Read Name file, Version and Info
        param.flash->BSP_QSPI_Read(block, 0, FLASH_FILE_SIZE_LEN);
        char stima_name[STIMA_MODULE_NAME_LENGTH] = {0};
        getStimaNameByType(stima_name, MODULE_TYPE);
        if(checkStimaFirmwareType((char*)block, module_type, version, revision)) {
            param.flash->BSP_QSPI_Read((uint8_t*)len, FLASH_SIZE_ADDR(0), FLASH_INFO_SIZE_U64);
            fileReady = true;
        }
        param.qspiLock->Give();
    }
    return fileReady;
}

/// @brief File Open with check SD Status (Various ISUUE... Resolving Resolution in progress STM32 Staff )
///        on STM32 CubeMX. STM32Duino SD not Not currently managed Inserted by digiteco according to the
///        various ISUEE some solutions that limit the problem
/// @param reqFile File object
/// @param file_name nameFile
/// @param file_mode file_mode open (WR/RD ecc..)
void MmcTask::FileOpenSecurity(File *reqFile, char* file_name, uint8_t file_mode) {
  *reqFile = SD.open(file_name, file_mode);
  if(!(*reqFile)) {
    // Disable the MMC1 peripheral
    SDMMC1->POWER = 0;
    RCC->APB2ENR &= ~RCC_APB2ENR_SDMMC1EN;
    // Waiting before RESET HW Complete
    Delay(200);
    SD.end();
    while(!SD.begin(PIN_MMC1_DTC)) {
      // Disable the MMC1 peripheral
      SDMMC1->POWER = 0;
      RCC->APB2ENR &= ~RCC_APB2ENR_SDMMC1EN;
      Delay(200);
      SD.end();
    }
    *reqFile = SD.open(file_name, file_mode);
  }
}

void MmcTask::Run()
{
  // Generic retry
  uint8_t retry;
  bool message_traced = false;
  bool is_getted_rtc;
  // Queue buffer
  char queueBuffer[RMAP_PUT_DATA_ELEMENT_SIZE > LOG_PUT_DATA_ELEMENT_SIZE ? RMAP_PUT_DATA_ELEMENT_SIZE : LOG_PUT_DATA_ELEMENT_SIZE] = {0};
  char logIntest[23] = {0};
  // Queue file put from external Task
  file_put_request_t data_file_queue;
  file_put_response_t system_response;
  char remote_file_name[FILE_NAME_MAX_LENGHT];
  // Local Firmware check and update
  char data_block[MMC_FW_BLOCK_SIZE];
  char stima_name[STIMA_MODULE_NAME_LENGTH];
  char local_file_name[FILE_NAME_MAX_LENGHT];
  uint16_t idxList;
  uint8_t module_type, fw_version, fw_revision;
  bool fw_found;
  File logFile;
  File putFile;

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  /************************************************************
   *                     SD USAGE - NOTE
   * *********************************************************
   * SD.remove("test.txt"); -> Delay (1ms) before NextOperation
   * SD.error... Delay (500ms) Next -> SD.begin(); Resync State
   * File.OpenNext (ARDUINO Leak Memory problem)
   *    -> Create ListIndex (WORK FINE WITH ONLY FILE LIST)
  *************************************************************/

  while (true)
  {

    switch (state)
    {
    case MMC_STATE_INIT:

      // Check SD or Resynch after Error
      if (MMCCard_Present()) {
        TRACE_VERBOSE_F(F("MMC Card slot ready -> MMC_STATE_CHECK_SD\r\n"));
        state = MMC_STATE_CHECK_SD;
        message_traced = false;
      } else {
        // Only one TRACE message... SD Not present
        if(!message_traced) {
          // SD Was NOT Ready... for System
          param.systemStatusLock->Take();
          param.system_status->flags.sd_card_ready = false;
          param.systemStatusLock->Give();
          TRACE_VERBOSE_F(F("MMC Card slot empty, MMC function disabled\r\n"));
          message_traced = true;
        }
      }
      break;

    case MMC_STATE_CHECK_SD:
      // Setup SD Card
      if (SD.begin()) {
        // SD Was Ready... for System
        param.systemStatusLock->Take();
        param.system_status->flags.sd_card_ready = true;
        param.systemStatusLock->Give();
        TRACE_VERBOSE_F(F("MMC Card init complete -> MMC_STATE_WAITING_EVENT\r\n"));
        // Optional Trace Type of CARD... and Size
        // Check or create directory Structure...
        if(!SD.exists("firmware")) SD.mkdir("firmware");
        if(!SD.exists("log")) SD.mkdir("log");
        if(!SD.exists("data")) SD.mkdir("data");

        #if (TRACE_LEVEL > TRACE_LEVEL_OFF)
        // Trace Firmware List present on SD Card.
        // Charged in SD or getted from remote HTTP
        // Check firmware file present Type, model and version
        idxList = 0;
        // Check list Firmware File (Added Function to SD Class)
        while(SD.listIndex("/firmware", idxList++, local_file_name)) {
          // Found firmware file?
          if(checkStimaFirmwareType(local_file_name, &module_type, &fw_version, &fw_revision)) {
            getStimaNameByType(stima_name, module_type);
            TRACE_INFO_F(F("MMC: found firmware type: %s Ver %u.%u\r\n"), stima_name, fw_version, fw_revision);
          }
        }
        #endif

        // Push firmware to Flash
        state = MMC_STATE_WAITING_EVENT;
      } else {
        // Waiting to Start MMC (Only one trace and modify system_status do mmc not ready)
        if(!message_traced) {
          message_traced = true;
          // SD Was NOT Ready... for System
          param.systemStatusLock->Take();
          param.system_status->flags.sd_card_ready = false;
          param.systemStatusLock->Give();
          TRACE_VERBOSE_F(F("MMC Card waiting to begin...\r\n"));
          message_traced = true;
        }          
      }
      break;

    case MMC_STATE_WAITING_EVENT:

      // If System SLEEP...  Long WAIT
      // Else check all queue input
      // Go to response/action

      // TEST FW UPGRADE
      // Starting from LCD or Remote RPC Request
      if(0)
      {
        retry = 0;
        state = MMC_UPLOAD_FIRMWARE_TO_FLASH;
        break;
      }

      // *********************************************************
      //             Perform LOG WRITE append message
      // *********************************************************
      // If element get all element from the queue and Put to MMC
      // Typical Put of Logging are Time controlled from TASK (If queue are free into reasonable time LOG is pushed)
      // Log queue element is reasonable sized to avoid problems
      is_getted_rtc = false;
      while(!param.dataLogPutQueue->IsEmpty()) {
        if(!is_getted_rtc) {
          // Get date time to Intest string to PUT (for this message session)
          is_getted_rtc = true;
          if(param.rtcLock->Take(Ticks::MsToTicks(RTC_WAIT_DELAY_MS))) {
            sprintf(logIntest, "%02d/%02d/%02d %02d:%02d:%02d.%03d ",
              rtc.getDay(), rtc.getMonth(), rtc.getYear(), rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getSubSeconds());
            param.rtcLock->Give();
          }
        }
        // Get message from queue
        if(param.dataLogPutQueue->Dequeue(queueBuffer)) {
          // Put to MMC ( APPEND File )
          FileOpenSecurity(&logFile, "log/log.txt", FA_OPEN_APPEND | FA_WRITE);
          logFile.print(logIntest);
          logFile.write(queueBuffer, strlen(queueBuffer) < LOG_PUT_DATA_ELEMENT_SIZE ? strlen(queueBuffer) : LOG_PUT_DATA_ELEMENT_SIZE);
          logFile.println();
          logFile.close();
        }
      }
      // *********************************************************
      //             End OF perform LOG append message
      // *********************************************************

      // *********************************************************
      //           Perform RMAP Data append get message
      // *********************************************************
      // If element get all element from the queue and Put to MMC
      while(!param.dataRmapPutQueue->IsEmpty()) {
        // Get message from queue
        if(param.dataLogPutQueue->Dequeue(queueBuffer)) {
          // TODO:
          // Put to MMC ( APPEND to File in Format ready to SEND Mqtt Format )
        }
      }
      // *********************************************************
      //            End OF perform RMAP append message
      // *********************************************************

      // *********************************************************
      //       Perform FILE (FIRMWARE) WRITE append message
      // *********************************************************
      // If element get all element from the queue and Put to MMC N.B. If session running (Uploading...) Name File != NULL
      // If remote_file_name != NULL remote_file_name is not ready to system (Put firmware into module local and remote)
      // Any request for file (es. Cypal Request firmware are blocked)
      while(!param.dataFilePutRequestQueue->IsEmpty()) {
        // Try Get message from queue (Start, progress session download fron NETWORK TASK and push to SD CARD)
        // Send response -> system_reesponse generic mode to request
        if(param.dataFilePutRequestQueue->Dequeue(&data_file_queue)) {
          // Put to MMC ( CREATE / APPEND Firmware Block File session )
          if(data_file_queue.block_type == file_block_type::file_name) {
            // Get File name set file name Upload (session current START)
            memset(remote_file_name, 0, sizeof(remote_file_name));
            strcpy(remote_file_name, "/firmware/");
            memcpy(remote_file_name + strlen(remote_file_name), data_file_queue.block, data_file_queue.block_lenght);
            // Create File in ReWrite Mode
            // Locking file session (uploading...)
            memset(&system_response, 0, sizeof(system_response));
            // Open Put File
            FileOpenSecurity(&putFile, remote_file_name, FA_WRITE | FA_CREATE_ALWAYS);
            system_response.done_operation = true;
            // Send response to caller
            param.dataFilePutResponseQueue->Enqueue(&system_response, 0);
          } else if(data_file_queue.block_type == file_block_type::data_chunck) {
            memset(&system_response, 0, sizeof(system_response));
            if(putFile) {
              putFile.write(data_file_queue.block, data_file_queue.block_lenght);
              system_response.done_operation = true;
            } else {
              system_response.error_operation = true;
            }
            // Send response to caller
            param.dataFilePutResponseQueue->Enqueue(&system_response, 0);
          } else if(data_file_queue.block_type == file_block_type::end_of_file) {
            // Remove file name Upload (session current END)
            // Unlock session. File is ready for the system (without integrity control)
            memset(remote_file_name, 0, sizeof(remote_file_name));
            // Send response to caller ... OK done
            putFile.close();
            memset(&system_response, 0, sizeof(system_response));
            system_response.done_operation = true;
            param.dataFilePutResponseQueue->Enqueue(&system_response, 0);
          } else if(data_file_queue.block_type == file_block_type::ctrl_checksum) {
            // Remove file name Upload (session current END)
            // Unlock session. File is ready for the system
            // Need to control checksum (if any error file have to delete)
            memset(remote_file_name, 0, sizeof(remote_file_name));
            // TODO: checksum... other control file
            // Send response to caller ... OK done
            memset(&system_response, 0, sizeof(system_response));
            system_response.done_operation = true;
            param.dataFilePutResponseQueue->Enqueue(&system_response, 0);
          }
        }
      }
      // *********************************************************
      //        End OF FILE (FIRMWARE) WRITE append message
      // *********************************************************

      break;
    
    case MMC_UPLOAD_FIRMWARE_TO_FLASH:

      // *********************************************************
      //           Perform Local Firmware FLASH Update
      // *********************************************************

      // N.B TODO: Need to Inform Other TASK Priority MAX to Upload Firmware To Flash
      // All operation are to suspend, MMC Task End to Responding at Standard Queue request

      // Check firmware file present Type, model and version
      fw_found = false;
      // Name of module
      getStimaNameByType(stima_name, param.configuration->module_type);
      // Check list Firmware File
      idxList = 0;
      while(SD.listIndex("/firmware", idxList++, local_file_name)) {
        // Found firmware file?
        if(checkStimaFirmwareType(local_file_name, &module_type, &fw_version, &fw_revision)) {
          // Is this module ?
          if(module_type == param.configuration->module_type) {
            fw_found = true;
            if((fw_version > param.configuration->module_main_version) ||
              ((fw_version == param.configuration->module_main_version) && (fw_revision > param.configuration->module_minor_version)))
            {
              TRACE_INFO_F(F("MMC: found firmware upgradable type: %s Ver %u.%u\r\n"), stima_name, fw_version, fw_revision);
              TRACE_INFO_F(F("MMC: starting firmware upgrade...\n\r"));
              // SetName File
              memset(data_block, 0, MMC_FW_BLOCK_SIZE);
              strcpy(data_block,"/firmware/");
              strcat(data_block, local_file_name);
              // Open file SD
              putFile = SD.open(data_block);
              // Flag
              bool bFirstBlock = true;
              bool is_error = false;
              // Is file opened?
              if(putFile) {
                int len_block;
                // Put firmware in correct Flash Location
                while(1) {
                  // *************  PREPARE FLASHING *************
                  // Read block data from file
                  len_block = putFile.read(data_block, MMC_FW_BLOCK_SIZE);
                  if(len_block < 0) {
                    is_error = true;
                    break;
                  }
                  // Append block firmware file to flash (same of CAN FW Upgrade)
                  // EOF when block != MMC_FW_BLOCK_SIZE (UAVCAN TYPE_LEN 256 BYTES)
                  if(!putFlashFile(local_file_name, true, bFirstBlock, data_block, len_block)) {
                    is_error = true;
                    break;
                  }
                  bFirstBlock = false;
                  if(len_block != MMC_FW_BLOCK_SIZE) {
                    // EOF
                    break;
                  }
                  // WDT non blocking task (Delay basic operation)
                  TaskWatchDog(MMC_TASK_WAIT_OPERATION_DELAY_MS);
                  Delay(Ticks::MsToTicks(MMC_TASK_WAIT_OPERATION_DELAY_MS));
                }
                putFile.close();
                // Nothing error, starting firmware upgrade
                if(!is_error) {
                  // Remove from SD (NextCheck is from HTTP Connection and VersioneRevision Verify)
                  SD.remove(local_file_name);
                  // Optional send SIGTerm to all task
                  // WDT non blocking task (Delay basic operation)
                  TRACE_INFO_F(F("MMC: firmware upgrading complete waiting reboot for start flashing...\n\r"));
                  TaskWatchDog(MMC_TASK_WAIT_REBOOT_MS);
                  Delay(Ticks::MsToTicks(MMC_TASK_WAIT_REBOOT_MS));
                  NVIC_SystemReset();
                }
              }
              // Error opening file or procedure upload
              // is_error (open_file OK, procedure error)
              // !is_error (open_file error)
              if(++retry>MMC_TASK_GENERIC_RETRY) {
                // Abort MAX Retry
                TRACE_INFO_F(F("MMC: firmware upgrading error, Max retry reached up. Abort flashing!!!\n\r"));
                if(!is_error) {
                  // ReSynch SD Card... Error when opening file
                  TRACE_VERBOSE_F(F("MMC_UPLOAD_FIRMWARE_TO_FLASH -> MMC_STATE_ERROR\r\n"));
                  state = MMC_STATE_ERROR;
                } else {
                  // Error procedure Flashing... Can Retry from extern
                  TRACE_VERBOSE_F(F("MMC_UPLOAD_FIRMWARE_TO_FLASH -> MMC_STATE_WAITING_EVENT\r\n"));
                  state = MMC_STATE_WAITING_EVENT;
                }
              } else {
                // Error, next Retry
                TRACE_INFO_F(F("MMC: firmware upgrading error waiting retry\n\r"));
                TaskWatchDog(MMC_TASK_GENERIC_RETRY_DELAY_MS);
                Delay(Ticks::MsToTicks(MMC_TASK_GENERIC_RETRY_DELAY_MS));
              }
            }
            else
            {
              // Remove from SD (NextCheck is from HTTP Connection and VersioneRevision Verify)
              SD.remove(local_file_name);
              TRACE_INFO_F(F("MMC: found firmware obsolete: %s Ver %u.%u\r\n"), stima_name, fw_version, fw_revision);
              TRACE_INFO_F(F("MMC: firmware upgrade abort!!!\n\r"));
              TRACE_VERBOSE_F(F("MMC_UPLOAD_FIRMWARE_TO_FLASH -> MMC_STATE_WAITING_EVENT\r\n"));
              state = MMC_STATE_WAITING_EVENT;
            }
            break;
          }
        }
      }
      // Firmware not Found
      if(!fw_found) {
        TRACE_INFO_F(F("MMC: module firmware for module %s not found\r\n"), stima_name);
        TRACE_VERBOSE_F(F("MMC_UPLOAD_FIRMWARE_TO_FLASH -> MMC_STATE_WAITING_EVENT\r\n"));
        state = MMC_STATE_WAITING_EVENT;
      }
      break;

    case MMC_STATE_ERROR:
      // Gest Error... Resynch SD
      TRACE_VERBOSE_F(F("MMC_STATE_ERROR -> MMC_STATE_INIT\r\n"));
      state = MMC_STATE_INIT;
      break;
    }

    #if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
    #endif

    // One step base non blocking switch
    TaskWatchDog(MMC_TASK_WAIT_DELAY_MS);
    Delay(Ticks::MsToTicks(MMC_TASK_WAIT_DELAY_MS));

  }
}

#endif