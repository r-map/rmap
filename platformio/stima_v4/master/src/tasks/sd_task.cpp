/**
  ******************************************************************************
  * @file    sd_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   sd_task source file (SD SPI StimaV4)
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

#define TRACE_LEVEL     SD_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   SD_TASK_ID

#include "tasks/sd_task.h"
#include "date_time.h"

#if (ENABLE_SD)

using namespace cpp_freertos;

/// @brief Construct a new Sd Task:: Sd Task object
/// @param taskName name of the task
/// @param stackSize size of the stack
/// @param priority priority of the task
/// @param sdParam parameters for the task
SdTask::SdTask(const char *taskName, uint16_t stackSize, uint8_t priority, SdParam_t sdParam) : Thread(taskName, stackSize, priority), param(sdParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(SD_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  // Init val
  sdFlashPtr = 0;
  sdFlashBlock = 0;

  state = SD_STATE_INIT;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void SdTask::TaskMonitorStack()
{
  uint16_t stackUsage = (uint16_t)uxTaskGetStackHighWaterMark( NULL );
  if((stackUsage) && (stackUsage < param.system_status->tasks[LOCAL_TASK_ID].stack)) {
    param.systemStatusLock->Take();
    param.system_status->tasks[LOCAL_TASK_ID].stack = stackUsage;
    param.systemStatusLock->Give();
  }
}
#endif

/// @brief local watchDog and Sleep flag Task (optional)
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void SdTask::TaskWatchDog(uint32_t millis_standby)
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
void SdTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
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

/// @brief Return a STIMA's name file data for archive value starting by current block of data time in epoch style (uint32)
/// @param time IN uint32 epoch datetime (in format RMAP of data to archive).
/// @param dirPrefix IN directory prefix (add to filename to create complete path and fileName).
/// @param nameFile OUT Complete name of file with path.
void SdTask::namingFileData(uint32_t time, char *dirPrefix, char* nameFile)
{
  uint32_t dayno = time / SECS_DAY;
  int year = EPOCH_YR;
  uint8_t month = 0;

  while (dayno >= YEARSIZE(year)) {
    dayno -= YEARSIZE(year);
    year++;
  }
  while (dayno >= _ytab[LEAPYEAR(year)][month]) {
    dayno -= _ytab[LEAPYEAR(year)][month];
    month++;
  }

  sprintf(nameFile, "%s/%04d_%02d_%02d.dat", dirPrefix, year, ++month, ++dayno);
}

/// @brief If /data has more than SD_DATA_KEEP_FILES day files, remove the oldest .dat (not pointer, not open wr/rd).
/// @param protect_path_a Path or basename of a file that must not be deleted (write file, may be empty).
/// @param protect_path_b Path or basename of a file that must not be deleted (MQTT read file, may be empty). 
void SdTask::pruneOldestDataFile(const char *protect_path_a, const char *protect_path_b)
{
#if (SD_DATA_KEEP_FILES > 0)
  File dir = SD.open("/data");
  if (!dir || !dir.isDir()) {
    if (dir) dir.close();
    return;
  }

  const char *skip_a = protect_path_a;
  const char *skip_b = protect_path_b;
  if (skip_a && skip_a[0]) {
    const char *slash = strrchr(skip_a, '/');
    if (slash) skip_a = slash + 1;
  } else {
    skip_a = NULL;
  }
  if (skip_b && skip_b[0]) {
    const char *slash = strrchr(skip_b, '/');
    if (slash) skip_b = slash + 1;
  } else {
    skip_b = NULL;
  }

  uint16_t n_dat = 0;
  char oldest[DATA_FILENAME_LEN];
  oldest[0] = 0;
  char name[FILE_NAME_MAX_LENGHT];

  while (true) {
    TaskWatchDog(TASK_WAIT_REALTIME_DELAY_MS);
    File entry = dir.openNextFile();
    if (!entry) break;
    entry.getName(name, sizeof(name));
    entry.close();
    if (strcmp(name, "pointer.dat") == 0) continue;
    size_t nlen = strlen(name);
    if (nlen < 5) continue;
    if (strcmp(name + nlen - 4, ".dat") != 0) continue;
    n_dat++;
    if (skip_a && strcmp(name, skip_a) == 0) continue;
    if (skip_b && strcmp(name, skip_b) == 0) continue;
    if ((oldest[0] == 0) || (strcmp(name, oldest) < 0)) {
      strncpy(oldest, name, sizeof(oldest) - 1);
      oldest[sizeof(oldest) - 1] = 0;
    }
  }
  dir.close();

  if ((n_dat <= SD_DATA_KEEP_FILES) || (oldest[0] == 0)) return;

  char path[DATA_FILENAME_LEN];
  snprintf(path, sizeof(path), "/data/%s", oldest);
  TaskWatchDog(TASK_WAIT_REALTIME_DELAY_MS);
  if (SD.remove(path)) {
    TRACE_INFO_F(F("SD: prune %s (%u files, keep %u)\r\n"), path, (unsigned)n_dat, (unsigned)SD_DATA_KEEP_FILES);
  }
#endif
}

/// @brief Drop legacy /bkp from old FW (no longer used). Safe for remote FW upgrade without SD format.
void SdTask::dropLegacyBkpDir(void)
{
  if (!SD.exists("/bkp")) {
    return;
  }

  File dir = SD.open("/bkp");
  if (!dir) {
    return;
  }

  if (!dir.isDir()) {
    dir.close();
    if (SD.remove("/bkp")) {
      TRACE_INFO_F(F("SD: removed legacy file /bkp\r\n"));
    }
    return;
  }

  char name[FILE_NAME_MAX_LENGHT];
  char path[FILE_NAME_MAX_LENGHT];
  uint16_t n_removed = 0;

  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;
    entry.getName(name, sizeof(name));
    const bool is_subdir = entry.isDir();
    entry.close();
    if ((name[0] == '.') && ((name[1] == 0) || ((name[1] == '.') && (name[2] == 0)))) {
      continue;
    }
    snprintf(path, sizeof(path), "/bkp/%s", name);
    if (is_subdir) {
      // Old layout was flat; nested leftovers: leave for next boot after files cleared.
      continue;
    }
    if (SD.remove(path)) {
      n_removed++;
    }
    TaskWatchDog(TASK_WAIT_REALTIME_DELAY_MS);
    Delay(Ticks::MsToTicks(TASK_WAIT_REALTIME_DELAY_MS));
  }
  dir.close();

  if (SD.rmdir("/bkp")) {
    TRACE_INFO_F(F("SD: dropped legacy /bkp (%u files)\r\n"), (unsigned)n_removed);
  } else if (n_removed > 0) {
    TRACE_INFO_F(F("SD: /bkp cleanup partial (%u removed), retry next boot\r\n"), (unsigned)n_removed);
  }
}

/// @brief Next archive day file after after_basename (e.g. "2025_03_16.dat"), skipping gaps.
/// One directory scan with WDT kicks — does not exists() every day until now().
bool SdTask::findNextArchiveDat(const char *dir_prefix, const char *after_basename,
                                char *out_path, size_t out_path_len, uint32_t *out_day_epoch)
{
  if ((dir_prefix == NULL) || (after_basename == NULL) || (out_path == NULL) || (out_path_len < 8) ||
      (out_day_epoch == NULL)) {
    return false;
  }

  File dir = SD.open(dir_prefix);
  if (!dir || !dir.isDir()) {
    if (dir) dir.close();
    return false;
  }

  char name[FILE_NAME_MAX_LENGHT];
  char best[DATA_FILENAME_LEN];
  best[0] = 0;

  while (true) {
    TaskWatchDog(TASK_WAIT_REALTIME_DELAY_MS);
    File entry = dir.openNextFile();
    if (!entry) break;
    entry.getName(name, sizeof(name));
    entry.close();
    if (strcmp(name, "pointer.dat") == 0) continue;
    size_t nlen = strlen(name);
    if (nlen < 14) continue;
    if (strcmp(name + nlen - 4, ".dat") != 0) continue;
    if (strcmp(name, after_basename) <= 0) continue;
    if ((best[0] == 0) || (strcmp(name, best) < 0)) {
      strncpy(best, name, sizeof(best) - 1);
      best[sizeof(best) - 1] = 0;
    }
  }
  dir.close();

  if (best[0] == 0) {
    return false;
  }

  unsigned y = 0, mo = 0, d = 0;
  if (sscanf(best, "%u_%u_%u.dat", &y, &mo, &d) != 3) {
    return false;
  }
  DateTime day = {0};
  day.year = (uint16_t)y;
  day.month = (uint8_t)mo;
  day.day = (uint8_t)d;
  *out_day_epoch = (uint32_t)convertDateToUnixTime(&day);
  snprintf(out_path, out_path_len, "%s/%s", dir_prefix, best);
  return true;
}

/// @brief Scrive dati in append su Flash per scrittura sequenziale file data remoto
/// @param file_name nome del file UAVCAN
/// @param is_firmware true se il file +-è di tipo firmware
/// @param rewrite true se necessaria la riscrittura del file
/// @param buf blocco dati da scrivere in formato UAVCAN [256 Bytes]
/// @param count numero del blocco da scrivere in formato UAVCAN [Blocco x Buffer]
/// @return true if block saved OK, false on any error
bool SdTask::putFlashFile(const char* const file_name, const bool is_firmware, const bool rewrite, void* buf, size_t count)
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
                sdFlashPtr = FLASH_FW_POSITION;
            } else {
                // Standard File Data Upload
                sdFlashPtr = FLASH_FILE_POSITION;
            }
            // Get Block Current into Flash
            sdFlashBlock = sdFlashPtr / AT25SF641_BLOCK_SIZE;
            // Erase First Block Block (Block OF 4KBytes)
            TRACE_INFO_F(F("FLASH: Erase block: %d\r\n"), sdFlashBlock);
            if (param.flash->BSP_QSPI_Erase_Block(sdFlashBlock)) {
                param.qspiLock->Give();
                return false;
            }
            // Write Name File (Size at Eof...)
            uint8_t file_flash_name[FLASH_FILE_SIZE_LEN] = {0};
            memcpy(file_flash_name, file_name, strlen(file_name));
            param.flash->BSP_QSPI_Write(file_flash_name, sdFlashPtr, FLASH_FILE_SIZE_LEN);
            // Write into Flash
            TRACE_INFO_F(F("FLASH: Write [ %d ] bytes at addr: %d\r\n"), FLASH_FILE_SIZE_LEN, sdFlashPtr);
            #ifdef CHECK_FLASH_WRITE
            param.flash->BSP_QSPI_Read(check_data, sdFlashPtr, FLASH_FILE_SIZE_LEN);
            if(memcmp(file_flash_name, check_data, FLASH_FILE_SIZE_LEN)==0) {
                TRACE_INFO_F(F("FLASH: Reading check OK\r\n"));
            } else {
                TRACE_ERROR_F(F("FLASH: Reading check ERROR\r\n"));
                param.qspiLock->Give();
                return false;
            }
            #endif
            // Start Page...
            sdFlashPtr += FLASH_INFO_SIZE_LEN;
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
            TRACE_INFO_F(F("FLASH: Write [ %d ] bytes at addr: %d\r\n"), count, sdFlashPtr);
            // Starting Write at OFFSET Required... Erase here is Done
            param.flash->BSP_QSPI_Write((uint8_t*)buf, sdFlashPtr, count);
            #ifdef CHECK_FLASH_WRITE
            param.flash->BSP_QSPI_Read(check_data, sdFlashPtr, count);
            if(memcmp(buf, check_data, count)==0) {
                TRACE_INFO_F(F("FLASH: Reading check OK\r\n"));
            } else {
                TRACE_ERROR_F(F("FLASH: Reading check ERROR\r\n"));
                param.qspiLock->Give();
                return false;
            }
            #endif
            sdFlashPtr += count;
            // Check if Next Page Addressed (For Erase Next Block)
            if((sdFlashPtr / AT25SF641_BLOCK_SIZE) != sdFlashBlock) {
                sdFlashBlock = sdFlashPtr / AT25SF641_BLOCK_SIZE;
                // Erase First Block Block (Block OF 4KBytes)
                TRACE_INFO_F(F("FLASH: Erase block: %d\r\n"), sdFlashBlock);
                if (param.flash->BSP_QSPI_Erase_Block(sdFlashBlock)) {
                    param.qspiLock->Give();
                    return false;
                }
            }
        }
        // Eof if != 256 Bytes Write
        if(count!=0x100) {
            // Write Info File for Closing...
            // Size at 
            uint64_t lenghtFile = sdFlashPtr - FLASH_INFO_SIZE_LEN;
            if (is_firmware) {
                // Firmware Flash
                sdFlashPtr = FLASH_FW_POSITION;
            } else {
                // Standard File Data Upload
                sdFlashPtr = FLASH_FILE_POSITION;
            }
            param.flash->BSP_QSPI_Write((uint8_t*)&lenghtFile, FLASH_SIZE_ADDR(sdFlashPtr), FLASH_INFO_SIZE_U64);
            // Write into Flash
            TRACE_INFO_F(F("FLASH: Write [ %d ] bytes at addr: %d\r\n"), FLASH_INFO_SIZE_U64, sdFlashPtr);
            #ifdef CHECK_FLASH_WRITE
            param.flash->BSP_QSPI_Read(check_data, FLASH_SIZE_ADDR(sdFlashPtr), FLASH_INFO_SIZE_U64);
            if(memcmp(&lenghtFile, check_data, FLASH_INFO_SIZE_U64)==0) {
                TRACE_INFO_F(F("FLASH: Reading check OK\r\n"));
            } else {
                TRACE_INFO_F(F("FLASH: Reading check ERROR\r\n"));
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
bool SdTask::getFlashFwInfoFile(uint8_t *module_type, uint8_t *version, uint8_t *revision, uint64_t *len)
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

bool SdTask::sdCardBegin(bool volume)
{
  #if (SD_SPI_PORT_ID == 1)
  SdSpiConfig spiConfig(PIN_SPI1_SS, SD_SPI1_SS_USER_MODE, SPI_SPEED, &Spi1);
  #else
  SdSpiConfig spiConfig(PIN_SPI2_SS, DEDICATED_SPI, SPI_SPEED, &Spi2);
  #endif
  if (volume) {
    return SD.begin(spiConfig);
  }
  return SD.cardBegin(spiConfig);
}

void SdTask::sdCardEnd()
{
  SD.end();
}

void SdTask::sdCloseAllFiles(File &logFile, File &rmapWrFile, File &rmapRdFile,
                             File &putFile, File *getFile, uint8_t getFileCount,
                             File &dir, File &entry, File &tmpFile)
{
  if (logFile) logFile.close();
  if (rmapWrFile) rmapWrFile.close();
  if (rmapRdFile) rmapRdFile.close();
  if (putFile) putFile.close();
  if (dir) dir.close();
  if (entry) entry.close();
  if (tmpFile) tmpFile.close();
  if (getFile) {
    for (uint8_t i = 0; i < getFileCount; i++) {
      if (getFile[i]) getFile[i].close();
    }
  }
}

/// @brief RUN Task
void SdTask::Run() 
{
  // System message data queue structured
  system_message_t system_message;
  #ifdef PIN_SD_LED
  // Diagnostic LED
  bool bLedLevel;
  uint8_t led_counter;
  #endif
  // Security verify write flag
  bool bWriteErr;
  // Generic retry
  uint8_t retry;
  bool message_traced = false;
  bool sd_begin_pending = true;
  uint32_t last_sd_begin_attempt_ms = 0;
  uint32_t last_data_prune_ms = 0;
  bool is_getted_rtc;
  // Queue buffer for logging
  char logBuffer[LOG_PUT_DATA_ELEMENT_SIZE];
  char logIntest[23] = {0};
  // Data buffer for RMAP queue
  rmap_archive_data_t rmap_put_archive_data;
  rmap_get_request_t rmap_get_request;
  rmap_get_response_t rmap_get_response;
  // Name file for data append es. /data/2023_01_30.dat (RMAP File data are stored by Day)
  char rmap_file_name_wr[DATA_FILENAME_LEN] = {0};      // Name current Write File Data RMAP
  char rmap_file_name_rd[DATA_FILENAME_LEN] = {0};      // Name Current Read File Data RMAP (Get queue from MQTT/Supervisor Request)
  char rmap_file_name_check[DATA_FILENAME_LEN] = {0};   // Check control Name VAR (RMAP Data Day changed?)
  char rmap_file_name_rd_bkp[DATA_FILENAME_LEN] = {0};  // Name Backup Read File Data RMAP for Recovery End of Time "dte" param
  uint32_t rmap_pointer_seek;           // Seek Absolute Position Pointer Read in File RMAP Queue Out
  uint32_t rmap_pointer_datetime;       // Date Time Pointer Read in File RMAP Queue Out
  uint32_t rmap_pointer_seek_prv;       // Seek Absolute Position Pointer Read in File RMAP Queue Out (Previous Position before OK confirm data recived)
  uint32_t rmap_pointer_datetime_prv;   // Date Time Pointer Read in File RMAP Queue Out (Previous Position before OK confirm data recived)
  uint32_t rmap_pointer_seek_bkp;       // Bkp Seek Absolute Position Pointer Read in File RMAP Queue Out
  uint32_t rmap_pointer_datetime_bkp;   // Bkp Date Time Pointer Read in File RMAP Queue Out
  uint32_t rmap_pointer_datetime_end;   // End Date Time Pointer Read in File RMAP Queue Out (RPC Start/End)
  bool using_rmap_pointer_datetime_end; // ? Using now End Date Time Pointer Read (RPC Start/End)
  // Queue file put and get from external Task
  // Put From extern task to card ( Es. Receive firmware from http to SD )
  file_put_request_t file_put_request;
  file_put_response_t file_put_response;
  // Get from card to extern task ( Es. Transmit firmware from SD to CAN module )
  file_get_request_t file_get_request;
  file_get_response_t file_get_response;
  char remote_file_name[FILE_NAME_MAX_LENGHT];
  // Local Firmware check and update
  char data_block[SD_FW_BLOCK_SIZE];
  char stima_name[STIMA_MODULE_NAME_LENGTH];
  char local_file_name[FILE_NAME_MAX_LENGHT];
  char firmware_file_name[FILE_NAME_MAX_LENGHT];
  Module_Type module_type;
  uint8_t module_type_cast, fw_version, fw_revision;
  bool fw_found;
  bool fw_reload_struct = false;  // True if request reload structure firmware and send queue response
  bool fw_reinit_struct = false;  // True if request reinit structure firmware and send queue response (complete clean directory)
  bool sd_reinit_data = false;    // True if request reinit sd data and send queue response
  File rmapWrFile, rmapRdFile;    // File (RMAP Write Data Append and Read Data from External Task request)
  bool rmap_rd_file_open = false; // RD handle closed at EOF but File object may stay truthy
  File logFile, putFile;          // File Log and Firmware Write INTO SD (From Queue TASK Extern)
  File getFile[BOARDS_COUNT_MAX]; // File for remote boards Multi simultaneous file server Reading (For Queue Task Extern)
  File dir, entry, tmpFile;       // Only used for Temp(shared Open Close Single Operation) or Access directory List
  bool error_sd_card = false;     // Generic error Open/Other operation to SD require a new Synch Reset SD CARD
  bool is_real_time_task = false; // Request min delay (real time type for task) Need if queue file is opened direct to minimize external wait

  auto sdPrepareRemount = [&]() {
    sdCloseAllFiles(logFile, rmapWrFile, rmapRdFile, putFile, getFile, BOARDS_COUNT_MAX,
                    dir, entry, tmpFile);
    rmap_file_name_wr[0] = 0;
    rmap_file_name_rd[0] = 0;
    rmap_file_name_check[0] = 0;
    rmap_file_name_rd_bkp[0] = 0;
    rmap_rd_file_open = false;
    param.systemStatusLock->Take();
    param.system_status->flags.sd_card_ready = false;
    param.systemStatusLock->Give();
  };

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  // SD-CARD Setup PIN CS SD UPIN27 OR HW VER_01_01 INTERNAL SPI2 TO SD DIRECT
  #if (SD_SPI_PORT_ID == 1)
    pinMode(PIN_SPI1_SS, OUTPUT);
    digitalWrite(PIN_SPI1_SS, HIGH);
  #else
    pinMode(PIN_SPI2_SS, OUTPUT);
    digitalWrite(PIN_SPI2_SS, HIGH);
  #endif
  #ifdef PIN_SD_LED
  pinMode(PIN_SD_LED, OUTPUT);
  #endif

  while (true)
  {

    switch (state)
    {
    case SD_STATE_INIT:
      // Init signal LED Diagnostic to High Level
      #ifdef PIN_SD_LED
      bLedLevel = true;
      digitalWrite(PIN_SD_LED, bLedLevel);
      led_counter = 0;
      #endif
      param.systemStatusLock->Take();
      param.system_status->flags.sd_card_ready = false;
      param.systemStatusLock->Give();
      sd_begin_pending = true;
      // break unused
    case SD_STATE_INIT_SD:
      state = SD_STATE_INIT_SD;

      if (!param.systemMessageQueue->IsEmpty()) {
        system_message_t sd_cmd;
        param.systemMessageQueue->Peek(&sd_cmd);
        if ((sd_cmd.task_dest == LOCAL_TASK_ID) && sd_cmd.command.do_trunc_sd) {
          param.systemMessageQueue->Dequeue(&sd_cmd);
          if (sd_cmd.param == CMD_PARAM_REQUIRE_RESPONSE) sd_reinit_data = true;
          state = SD_STATE_TRUNCATE_DATA;
          break;
        }
      }

      if (!sd_begin_pending &&
          ((millis() - last_sd_begin_attempt_ms) < SD_TASK_NO_CARD_RETRY_DELAY_MS)) {
        #ifdef PIN_SD_LED
        led_counter++;
        if(led_counter > (500 / SD_TASK_WAIT_DELAY_MS)) {
          led_counter = 0;
          bLedLevel = !bLedLevel;
          digitalWrite(PIN_SD_LED, bLedLevel);
        }
        #endif
        break;
      }
      sd_begin_pending = false;
      last_sd_begin_attempt_ms = millis();

      sdCardEnd();
      if (sdCardBegin(true)) {
        TRACE_VERBOSE_F(F("SD Card slot ready -> SD_STATE_CHECK_STRUCTURE\r\n"));
        message_traced = false;
        state = SD_STATE_CHECK_STRUCTURE;
      } else {
        #ifdef PIN_SD_LED
        led_counter++;
        if(led_counter > (500 / SD_TASK_WAIT_DELAY_MS)) {
          led_counter = 0;
          bLedLevel = !bLedLevel;
          digitalWrite(PIN_SD_LED, bLedLevel);
        }
        #endif
        if(!message_traced) {
          param.systemStatusLock->Take();
          param.system_status->flags.sd_card_ready = false;
          param.systemStatusLock->Give();
          TRACE_VERBOSE_F(F("SD Card waiting to begin (retry every %u s)\r\n"),
            (unsigned)(SD_TASK_NO_CARD_RETRY_DELAY_MS / 1000U));
          message_traced = true;
        }
      }
      break;

    case SD_STATE_TRUNCATE_DATA:
      TRACE_INFO_F(F("SD: Erase/format SD Card requested\r\n"));
      sdPrepareRemount();
      sdCardEnd();

      if (!sdCardBegin(false)) {
        TRACE_ERROR_F(F("SD: Erase failed, card not responding\r\n"));
        if (sd_reinit_data) {
          sd_reinit_data = false;
          system_message_t system_message = {0};
          system_message.task_dest = ALL_TASK_ID;
          system_message.command.done_trunc_sd = true;
          param.systemMessageQueue->Enqueue(&system_message);
        }
        state = SD_STATE_INIT;
        break;
      }

      TaskWatchDog(SD_TASK_FORMAT_WDT_MS);
      #ifdef PIN_SD_LED
      digitalWrite(PIN_SD_LED, HIGH);
      #endif
      if (!SD.format()) {
        #ifdef PIN_SD_LED
        digitalWrite(PIN_SD_LED, LOW);
        #endif
        TRACE_ERROR_F(F("SD: Format failed\r\n"));
        if (sd_reinit_data) {
          sd_reinit_data = false;
          system_message_t system_message = {0};
          system_message.task_dest = ALL_TASK_ID;
          system_message.command.done_trunc_sd = true;
          param.systemMessageQueue->Enqueue(&system_message);
        }
        state = SD_STATE_INIT;
        break;
      }
      #ifdef PIN_SD_LED
      digitalWrite(PIN_SD_LED, LOW);
      #endif
      TRACE_INFO_F(F("SD: Format OK, remounting\r\n"));

      if (!sdCardBegin(true)) {
        TRACE_ERROR_F(F("SD: Remount after format failed\r\n"));
        if (sd_reinit_data) {
          sd_reinit_data = false;
          system_message_t system_message = {0};
          system_message.task_dest = ALL_TASK_ID;
          system_message.command.done_trunc_sd = true;
          param.systemMessageQueue->Enqueue(&system_message);
        }
        state = SD_STATE_INIT;
        break;
      }

      param.systemStatusLock->Take();
      for(uint8_t brd=0; brd<STIMA_MODULE_TYPE_MAX_AVAIABLE; brd++) {
        param.system_status->boards_update_avaiable[brd].module_type = Module_Type::undefined;
        param.system_status->boards_update_avaiable[brd].version = 0;
        param.system_status->boards_update_avaiable[brd].revision = 0;
      }
      param.system_status->data_master.fw_upgradable = false;
      for(uint8_t queueId=0; queueId<BOARDS_COUNT_MAX; queueId++) {
        param.system_status->data_slave[queueId].fw_upgradable = false;
      }
      param.systemStatusLock->Give();

      if(sd_reinit_data) {
        sd_reinit_data = false;
        system_message_t system_message = {0};
        system_message.task_dest = ALL_TASK_ID;
        system_message.command.done_trunc_sd = true;
        param.systemMessageQueue->Enqueue(&system_message);
      }

      TRACE_VERBOSE_F(F("SD_STATE_TRUNCATE_DATA -> SD_STATE_CHECK_STRUCTURE\r\n"));
      state = SD_STATE_CHECK_STRUCTURE;
      break;

    case SD_STATE_CHECK_STRUCTURE:
      // Waiting loading configuration complete before start application
      if (!param.system_status->configuration.is_loaded) {
          if(!message_traced) {
              TRACE_INFO_F(F("SD task: Waiting configuration before START\r\n"));
              message_traced = true;
          }
          break;
      }
      message_traced = false;

      // ***************************************************
      //    SD Check and create structure Directory Data
      // ***************************************************
      // LED Diag signal OFF (SD Ready and inizialized)
      #ifdef PIN_SD_LED
      bLedLevel = false;
      digitalWrite(PIN_SD_LED, bLedLevel);
      #endif
      // Optional Trace Type of CARD... and Size
      // Check or create directory Structure...
      // Create firmware directory
      if(!SD.exists("/firmware")) {
        if(!SD.mkdir("/firmware")) {
          state = SD_STATE_INIT;
          break;
        }
        TRACE_INFO_F(F("SD: created base structure dir firmware\r\n"));
      }
      dir = SD.open("/firmware");
      if(!dir.isDir()) {
        // firmware is a file (error on SD structure), restart this switch to create good structure
        SD.remove("/firmware");
        TRACE_INFO_F(F("SD: removed file firmware from structure\r\n"));
        break;
      }
      dir.close();

      // Create log directory
      if(!SD.exists("/log")) {
        if(!SD.mkdir("/log")) {
          state = SD_STATE_INIT;
          break;
        }
        TRACE_INFO_F(F("SD: created base structure dir log\r\n"));
      }
      dir = SD.open("/log");
      if(!dir.isDir()) {
        // Log is a file (error on SD structure), restart this switch to create good structure
        SD.remove("/log");
        TRACE_INFO_F(F("SD: removed file log from structure\r\n"));
        break;
      }
      dir.close();

      // Create data and pointer ditectory
      if(!SD.exists("/data")) {
        if(!SD.mkdir("/data")) {
          state = SD_STATE_INIT;
          break;
        }
        TRACE_INFO_F(F("SD: created base structure dir data\r\n"));
      }
      dir = SD.open("/data");
      if(!dir.isDir()) {
        // Data is a file (error on SD structure), restart this switch to create good structure
        SD.remove("/data");
        TRACE_INFO_F(F("SD: removed file data from structure\r\n"));
        break;
      }
      dir.close();

      // Old FW wrote /bkp; new FW does not. Drop on structure check so remote upgrade
      // frees dir slots / space without lab SD wipe. No-op if absent.
      dropLegacyBkpDir();

      // ***************************************************
      // SD Was Ready... for System Structure and Pointer OK
      // ***************************************************
      param.systemStatusLock->Take();
      param.system_status->flags.sd_card_ready = true;
      param.systemStatusLock->Give();

      TRACE_VERBOSE_F(F("SD_STATE_CHECK_STRUCTURE -> SD_STATE_CHECK_DATA_PTR\r\n"));

      state = SD_STATE_CHECK_DATA_PTR;
      break;

    case SD_STATE_CHECK_DATA_PTR:
      // **********************************************************************************
      // Open/Create File data pointer... and check if SD Starting OK (create if not exist)
      // **********************************************************************************
      if (SD.exists("/data/pointer.dat")) {
        tmpFile = SD.open("/data/pointer.dat", O_RDONLY);
        if(tmpFile) {
          // Open File High LED
          #ifdef PIN_SD_LED
          digitalWrite(PIN_SD_LED, HIGH);
          #endif
          tmpFile.read(&rmap_pointer_datetime, sizeof(rmap_pointer_datetime));
          tmpFile.read(&rmap_pointer_seek, sizeof(rmap_pointer_seek));
          // Debug message timestamp Ptr
          DateTime date;
          convertUnixTimeToDate(rmap_pointer_datetime, &date);
          TRACE_INFO_F(F("SD: load current data pointer at date/time %s\r\n"), formatDate(&date, NULL));
          rmap_pointer_datetime_bkp = rmap_pointer_datetime;
          rmap_pointer_datetime_end = rmap_pointer_datetime;
          rmap_pointer_seek_bkp = rmap_pointer_seek;
          // At satrtup dont use end ptr get data...
          using_rmap_pointer_datetime_end = false;
          tmpFile.close();
          // Close File Low LED
          #ifdef PIN_SD_LED
          digitalWrite(PIN_SD_LED, LOW);
          #endif
          // At First Get Data Set Sync Pointer position with loaded param
          namingFileData(rmap_pointer_datetime, "/data", rmap_file_name_rd);
          if (rmap_pointer_seek == UNKNOWN_POINTER_POSITION) {
            TRACE_INFO_F(F("SD: RMAP pointer seek UNKNOWN — defer open until reset_ptr\r\n"));
          } else if(SD.exists(rmap_file_name_rd)) {
            rmapRdFile = SD.open(rmap_file_name_rd, O_RDONLY);
            if(rmapRdFile) {
              rmap_rd_file_open = true;
              rmapRdFile.seek(rmap_pointer_seek);
              rmap_pointer_seek_bkp = rmap_pointer_seek;
              const int avail_boot = rmapRdFile.available();
              TRACE_INFO_F(F("SD: load current data position at [ %d ], bytes avaible to read [ %d ]\r\n"), rmap_pointer_seek, avail_boot);
              if (avail_boot == 0) {
                rmapRdFile.close();
                rmap_rd_file_open = false;
              }
            }
          } else {
            TRACE_INFO_F(F("SD: pointer day file missing, keep pointer (EOF)\r\n"));
          }
          // Arm upload if unread bytes remain or a newer day file exists past a gap
          {
            bool have_backlog = (rmap_rd_file_open && rmapRdFile && (rmapRdFile.available() > 0));
            if (!have_backlog && (rmap_pointer_seek != UNKNOWN_POINTER_POSITION)) {
              char after_base[DATA_FILENAME_LEN] = {0};
              namingFileData(rmap_pointer_datetime, "/data", rmap_file_name_check);
              const char *slash = strrchr(rmap_file_name_check, '/');
              strncpy(after_base, slash ? (slash + 1) : rmap_file_name_check, sizeof(after_base) - 1);
              char next_path[DATA_FILENAME_LEN];
              uint32_t next_ep = 0;
              if (findNextArchiveDat("/data", after_base, next_path, sizeof(next_path), &next_ep)) {
                have_backlog = true;
              }
            }
            if (have_backlog) {
              param.systemStatusLock->Take();
              param.system_status->flags.new_data_to_send = true;
              param.systemStatusLock->Give();
              TRACE_INFO_F(F("SD: backlog after pointer, arm new_data_to_send\r\n"));
            }
          }
        } else {
          // SD Pointer Error, general Open on first File...
          // Error. Send to system_state and retry OPEN INIT SD
          state = SD_STATE_INIT;
          break;
        }
      } else {
        tmpFile = SD.open("/data/pointer.dat", O_RDWR | O_CREAT);
        if(tmpFile) {
          TRACE_INFO_F(F("SD: create new data pointer at configured ()\r\n"));
          // Open File High LED
          #ifdef PIN_SD_LED
          digitalWrite(PIN_SD_LED, HIGH);
          #endif
          rmap_pointer_seek = UNKNOWN_POINTER_POSITION; // Unknown position
          rmap_pointer_datetime = param.system_status->datetime.ptr_time_for_sensors_get_value * param.configuration->report_s;  // Init to Data Next Epoch
          // System status enter in data not ready for SENT (no data present)
          param.systemStatusLock->Take();
          param.system_status->flags.new_data_to_send = false;
          param.systemStatusLock->Give();
          bWriteErr = false;
          bWriteErr |= !tmpFile.write(&rmap_pointer_datetime, sizeof(rmap_pointer_datetime));
          bWriteErr |= !tmpFile.write(&rmap_pointer_seek, sizeof(rmap_pointer_seek));
          tmpFile.close();
          // Close File Low LED
          #ifdef PIN_SD_LED
          digitalWrite(PIN_SD_LED, LOW);
          #endif
          if(bWriteErr) {
            // SD Pointer Error, general Openon first File...
            // Error. Send to system_stae and retry OPEN INIT SD
            state = SD_STATE_INIT;
            break;
          }
        } else {
          // SD Pointer Error, general Open on first File...
          // Pointer file not coerent, Remove and new creation starting
          SD.remove("/data/pointer.dat");
          SD.remove("/data");
          state = SD_STATE_INIT;
          break;
        }
      }

      // Init backup data
      rmap_pointer_seek_prv = rmap_pointer_seek;
      rmap_pointer_datetime_prv = rmap_pointer_datetime;

      TRACE_VERBOSE_F(F("SD_STATE_CHECK_DATA_PTR -> SD_STATE_CHECK_FIRMWARE\r\n"));

      state = SD_STATE_CHECK_FIRMWARE;
      break;

    case SD_STATE_CLEAN_FIRMWARE:
      // **********************************************************************************
      // Clean entire structure firware dir (destroy all file into before new synch server)
      // **********************************************************************************
      // Abort any in-progress HTTP→SD put session so redownload starts clean
      if (putFile) {
        putFile.close();
      }
      if (remote_file_name[0] != 0) {
        SD.remove(remote_file_name);
        remote_file_name[0] = 0;
      }
      TRACE_INFO_F(F("SD: clean /firmware (force full redownload)\r\n"));
      dir = SD.open("/firmware");
      if(!dir) {
        // ? Need to send response to sender (Only if required... from RPC, not from command LCD)
        // Normally on request from RPC Before calling -> system_message.(do_function_respond)
        // sd_reinit_data is true if must to respond queue to sender (RPC)
        if(fw_reinit_struct) {
          fw_reinit_struct = false;
          system_message_t system_message = {0};
          system_message.task_dest = ALL_TASK_ID;
          system_message.command.done_reinit_fw = true;
          param.systemMessageQueue->Enqueue(&system_message);
        }
        // Exit Error and Reset
        state = SD_STATE_INIT;
        break;
      }
      // Open File High LED
      #ifdef PIN_SD_LED
      digitalWrite(PIN_SD_LED, HIGH);
      #endif
      if(dir.isDir()) {
        // Delete all file into firmware directory (reinit...)
        while(true) {
          entry = dir.openNextFile();
          if(!entry) break;
          entry.getName(local_file_name, FILE_NAME_MAX_LENGHT);
          entry.close();
          strcpy(firmware_file_name, "/firmware/");
          strcat(firmware_file_name, local_file_name);
          SD.remove(firmware_file_name);
          // Long Operation perform non blocking TASK
          TaskWatchDog(TASK_WAIT_REALTIME_DELAY_MS);
          Delay(Ticks::MsToTicks(TASK_WAIT_REALTIME_DELAY_MS));
          #if (ENABLE_STACK_USAGE)
          TaskMonitorStack();
          #endif
        }
        dir.close();
      } else {
        // firmware is a file (error on SD structure)
        SD.remove("/firmware");
        // Recreate
        if(!SD.exists("/firmware")) {
          if(!SD.mkdir("/firmware")) {
            // ? Need to send response to sender (Not at startup -> fw_reinit_struct = false)
            // Normally on request from RPC Before calling -> system_message.(do_function_respond)
            // fw_reinit_struct is true if must to respond queue to sender (RPC)
            if(fw_reinit_struct) {
              fw_reinit_struct = false;
              system_message_t system_message = {0};
              system_message.task_dest = ALL_TASK_ID;
              system_message.command.done_reinit_fw = true;
              param.systemMessageQueue->Enqueue(&system_message);
            }
            state = SD_STATE_INIT;
            break;
          }
          TRACE_INFO_F(F("SD: created base structure dir firmware\r\n"));
        }
      }

      // ? Need to send response to sender (Not at startup -> fw_reinit_struct = false)
      // Normally on request from RPC Before calling -> system_message.(do_function_respond)
      // fw_reinit_struct is true if must to respond queue to sender (RPC)
      if(fw_reinit_struct) {
        fw_reinit_struct = false;
        system_message_t system_message = {0};
        system_message.task_dest = ALL_TASK_ID;
        system_message.command.done_reinit_fw = true;
        param.systemMessageQueue->Enqueue(&system_message);
      }

      // Force init array structure SD Card firmware present (RESETTED to initial void value, without Firmware files)
      param.systemStatusLock->Take();
      for(uint8_t brd=0; brd<STIMA_MODULE_TYPE_MAX_AVAIABLE; brd++) {
        param.system_status->boards_update_avaiable[brd].module_type = Module_Type::undefined;
        param.system_status->boards_update_avaiable[brd].version = 0;
        param.system_status->boards_update_avaiable[brd].revision = 0;
      }
      // Force init flags structure SD Card firmware ready (SETTED on get data connection Cyphal)
      param.system_status->data_master.fw_upgradable = false;
      for(uint8_t queueId=0; queueId<BOARDS_COUNT_MAX; queueId++) {
        param.system_status->data_slave[queueId].fw_upgradable = false;
      }
      param.systemStatusLock->Give();

      TRACE_VERBOSE_F(F("SD_STATE_CLEAN_FIRMWARE -> SD_STATE_WAITING_EVENT\r\n"));

      state = SD_STATE_WAITING_EVENT;
      break;

    case SD_STATE_CHECK_FIRMWARE:
      // **********************************************************************************
      // Check firmware file present Type, model and version from list file in firmware dir
      // Full system_status inform struct with firmware present on SD (Forced at startup)
      // **********************************************************************************
      dir = SD.open("/firmware");
      if(!dir) {
        // ? Need to send response to sender (Not at startup -> fw_reload_struct = false)
        // Normally on request from RPC Before calling -> system_message.(do_function_respond)
        // fw_reload_struct is true if must to respond queue to sender (RPC)
        if(fw_reload_struct) {
          fw_reload_struct = false;
          system_message_t system_message = {0};
          system_message.task_dest = ALL_TASK_ID;
          system_message.command.done_reload_fw = true;
          param.systemMessageQueue->Enqueue(&system_message);
        }
        // Exit Error and Reset
        state = SD_STATE_INIT;
        break;
      }

      // Security remove only local master flag on reload firmware file current struct
      // Master check version direct from firmware define value, slave only from CAN Get Data
      // Slave Flag removed with calling download_firmware. And slave module version reload
      // directly before next command update firmware
      param.systemStatusLock->Take();
      param.system_status->data_master.fw_upgradable = false;
      param.systemStatusLock->Give();

      while(true) {
        entry = dir.openNextFile();
        if(!entry) break;
        // Open File High LED
        #ifdef PIN_SD_LED
        digitalWrite(PIN_SD_LED, HIGH);
        #endif
        // Found firmware file?
        entry.getName(local_file_name, FILE_NAME_MAX_LENGHT);
        if(checkStimaFirmwareType(local_file_name, &module_type_cast, &fw_version, &fw_revision)) {
          module_type = static_cast<Module_Type>(module_type_cast);
          getStimaNameByType(stima_name, module_type);
          // Update info Fw File module array (system_status)
          TRACE_INFO_F(F("SD: found firmware type: %s Ver %u.%u\r\n"), stima_name, fw_version, fw_revision);
          // Check if already module file are detected and processed (first occurance or update if version major found)
          fw_found = false;
          for(uint8_t brd=0; brd<STIMA_MODULE_TYPE_MAX_AVAIABLE; brd++) {
            if(param.system_status->boards_update_avaiable[brd].module_type == module_type) {
              fw_found = true;
              break;
            }
          }
          // If Version> last file version or Version== and Revision >... Update module version/revision firmware avaiable struct
          for(uint8_t brd=0; brd<STIMA_MODULE_TYPE_MAX_AVAIABLE; brd++) {
            // First occurance or Found...            
            if(((!fw_found) && (param.system_status->boards_update_avaiable[brd].module_type == Module_Type::undefined)) ||
              (param.system_status->boards_update_avaiable[brd].module_type == module_type)) {
              if((fw_version > param.system_status->boards_update_avaiable[brd].version) ||
                ((fw_version == param.system_status->boards_update_avaiable[brd].version)&&(fw_revision > param.system_status->boards_update_avaiable[brd].revision))) {
                param.systemStatusLock->Take();
                param.system_status->boards_update_avaiable[brd].module_type = module_type;
                param.system_status->boards_update_avaiable[brd].version = fw_version;
                param.system_status->boards_update_avaiable[brd].revision = fw_revision;
                param.systemStatusLock->Give();
              }
              // ?Is this module ->Master? (check directly if fw upgrade is avaiable with last version file present)
              if(param.configuration->module_type == MODULE_TYPE) {
                if((fw_version > param.configuration->module_main_version) ||
                  ((fw_version == param.configuration->module_main_version) && (fw_revision > param.configuration->module_minor_version))) {
                  param.systemStatusLock->Take();
                  param.system_status->data_master.fw_upgradable = true;
                  param.systemStatusLock->Give();
                }
              }
              break;
            }
          }
        }
        entry.close();
      }
      dir.close();
      // Close File Low LED
      #ifdef PIN_SD_LED
      digitalWrite(PIN_SD_LED, LOW);
      #endif
      // **********************************************************************************

      // LCD / fupdate use data_slave[].fw_upgradable (not only boards_update_avaiable).
      // Do not mark UPGRADABLE while run is still 0.0 (CAN has not reported rev yet).
      param.systemStatusLock->Take();
      for (uint8_t queueId = 0; queueId < BOARDS_COUNT_MAX; queueId++) {
        const Module_Type mt = param.configuration->board_slave[queueId].module_type;
        if (mt == Module_Type::undefined) {
          continue;
        }
        bool upgradable = false;
        const uint8_t run_ver = param.system_status->data_slave[queueId].module_version;
        const uint8_t run_rev = param.system_status->data_slave[queueId].module_revision;
        for (uint8_t checkId = 0; checkId < STIMA_MODULE_TYPE_MAX_AVAIABLE; checkId++) {
          if (param.system_status->boards_update_avaiable[checkId].module_type != mt) {
            continue;
          }
          const uint8_t fv = param.system_status->boards_update_avaiable[checkId].version;
          const uint8_t fr = param.system_status->boards_update_avaiable[checkId].revision;
          if ((run_ver != 0) || (run_rev != 0)) {
            if ((fv > run_ver) || ((fv == run_ver) && (fr > run_rev))) {
              upgradable = true;
            }
            TRACE_INFO_F(F("SD: slave[%u] fw check run %u.%u vs SD %u.%u → %s\r\n"),
              (unsigned)queueId, (unsigned)run_ver, (unsigned)run_rev,
              (unsigned)fv, (unsigned)fr, upgradable ? "UPGRADABLE" : "up-to-date");
          } else {
            TRACE_INFO_F(F("SD: slave[%u] fw on SD %u.%u (run unknown — wait CAN)\r\n"),
              (unsigned)queueId, (unsigned)fv, (unsigned)fr);
          }
          break;
        }
        param.system_status->data_slave[queueId].fw_upgradable = upgradable;
      }
      param.systemStatusLock->Give();

      // ? Need to send response to sender (Not at startup -> fw_reload_struct = false)
      // Normally on request from RPC Before calling -> system_message.(do_function_respond)
      // fw_reload_struct is true if must to respond queue to sender (RPC)
      if(fw_reload_struct) {
        fw_reload_struct = false;
        system_message_t system_message = {0};
        system_message.task_dest = ALL_TASK_ID;
        system_message.command.done_reload_fw = true;
        param.systemMessageQueue->Enqueue(&system_message);
      }

      TRACE_VERBOSE_F(F("SD_STATE_CHECK_FIRMWARE -> SD_STATE_WAITING_EVENT\r\n"));

      state = SD_STATE_WAITING_EVENT;
      break;

    case SD_STATE_WAITING_EVENT:

      // Sliding window: skip while MQTT/SD read is in flight (queue timeout 2.5 s)
      if ((millis() - last_data_prune_ms) >= 2000) {
        if (param.dataRmapGetRequestQueue->IsEmpty()) {
          last_data_prune_ms = millis();
          pruneOldestDataFile(rmap_file_name_wr, rmap_file_name_rd);
        }
      }

      // ********* SYSTEM QUEUE MESSAGE ***********
      // *** If System SLEEP... SD Sleep WAIT *****
      // enqueud system message from caller task
      if (!param.systemMessageQueue->IsEmpty()) {
          // Read queue in test mode
          if (param.systemMessageQueue->Peek(&system_message, 0))
          {
              // Its request addressed into ALL TASK... -> no pull (only SUPERVISOR or exernal gestor)
              if(system_message.task_dest == ALL_TASK_ID)
              {
                  // Pull && elaborate command, 
                  if(system_message.command.do_sleep)
                  {
                      // Enter sleep module OK and update WDT
                      TaskWatchDog(SD_TASK_SLEEP_DELAY_MS);
                      TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
                      Delay(Ticks::MsToTicks(SD_TASK_SLEEP_DELAY_MS));
                      TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
                  }
              }
          }
      }

      // *********************************************************
      // Starting from LCD COMMAND or Remote RPC Request (->Queue)
      // *********************************************************
      if(!param.systemMessageQueue->IsEmpty()) {
        system_message_t system_message;
        param.systemMessageQueue->Peek(&system_message);
        if(system_message.task_dest == LOCAL_TASK_ID) {
          param.systemMessageQueue->Dequeue(&system_message);
          // Request direct Update local firmware (Master) from SD CARD
          if(system_message.command.do_trunc_sd) {
            // ?Need to send a reply to sender (Security clean data if command come from Remote RPC... Clear before sending data)
            if(system_message.param == CMD_PARAM_REQUIRE_RESPONSE) sd_reinit_data = true;
            state = SD_STATE_TRUNCATE_DATA;
            break;
          }
          // Request direct Update local firmware (Master) from SD CARD
          if((system_message.command.do_update_fw)&&(system_message.node_id == CMD_PARAM_MASTER_ADDRESS)) {
            retry = 0;
            state = SD_UPLOAD_FIRMWARE_TO_FLASH;
            break;
          }
          // Request to reload structure File Firmware (Check firwware are really updatable)
          // And Auto start CallBack Start Upload Firmware To CAN and Local...
          if(system_message.command.do_reload_fw) {
            retry = 0;
            // ?Need to send a reply to sender
            if(system_message.param == CMD_PARAM_REQUIRE_RESPONSE) fw_reload_struct = true;
            // Normally RPC Wait a response all firmware check and loaded structure before
            // Calling systemn update all with another system message to queue
            state = SD_STATE_CHECK_FIRMWARE;
            break;
          }
          // Request to clear structure File Firmware (Clean all file present to firmware archive)
          // And Auto start CallBack Start Upload Firmware To CAN and Local...
          if(system_message.command.do_reinit_fw) {
            retry = 0;
            // ?Need to send a reply to sender
            if(system_message.param == CMD_PARAM_REQUIRE_RESPONSE) fw_reinit_struct = true;
            // Normally RPC Wait a response all firmware check and reinit structure before
            // Calling systemn update all with another system message to queue
            state = SD_STATE_CLEAN_FIRMWARE;
            break;
          }
        }
      }

      // *********************************************************
      //             Perform LOG WRITE append message 
      // *********************************************************
      // If element get all element from the queue and Put to SD
      // Typical Put of Logging are Time controlled from TASK (If queue are free into reasonable time LOG is pushed)
      // Log queue element is reasonable sized to avoid problems
      // File are always opened if Append for fast Access Operation
      // File can be opened simultaneously also readonly mode by another function es.Read/Print/Send INFO LOG
      is_getted_rtc = false;
      while(!param.dataLogPutQueue->IsEmpty()) {
        // Exit while on Error
        if(error_sd_card) break;
        // Set Intest RTC LOG
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
        if(param.dataLogPutQueue->Dequeue(logBuffer)) {
          // Put to SD ( APPEND File Always Opened with Flush Data )
          if(!logFile) logFile = SD.open("/log/log.txt", O_RDWR | O_CREAT | O_AT_END);
          if(logFile) {
            // Open File High LED
            #ifdef PIN_SD_LED
            digitalWrite(PIN_SD_LED, HIGH);
            #endif
            bWriteErr = false;
            bWriteErr |= !logFile.print(logIntest);
            bWriteErr |= !logFile.write(logBuffer, strlen(logBuffer) < LOG_PUT_DATA_ELEMENT_SIZE ? strlen(logBuffer) : LOG_PUT_DATA_ELEMENT_SIZE);
            bWriteErr |= !logFile.println();
            logFile.flush();
            // Close File Low LED
            #ifdef PIN_SD_LED
            digitalWrite(PIN_SD_LED, LOW);
            #endif
            if(bWriteErr) error_sd_card = true;
          } else {
            // Generic open file Error
            error_sd_card = true;
          }
        }
      }
      // *********************************************************
      //             End OF perform LOG append message
      // *********************************************************

      // *********************************************************
      //       Perform RMAP Write Data append get message
      // *********************************************************
      // If element get all element from the queue and Put to SD
      // rmap_put_archive_data.date_time is epoch_style dateTime Archive record field
      // Check if data must be added into current day_file. If Day cahnged (nameing!=)
      // File data will be closed and reopened with the new name with control
      // File have to be opened and flushed for fast write access operation
      // Reading file ReadOnly can work simultaneously with opendFile.
      // Pointer Read from other TASK (Read Archive Data) work reading file when is WR/Open
      while(!param.dataRmapPutQueue->IsEmpty()) {
        // Exit while on Error
        if(error_sd_card) break;
        // Test message from queue ( Only if SD Trasnsaction complete, remove from Queue )
        if(param.dataRmapPutQueue->Peek(&rmap_put_archive_data)) {
          // Put to SD ( APPEND to File in Native Format. Check naming file )
          namingFileData(rmap_put_archive_data.date_time, "/data", rmap_file_name_check);
          // Day Name File Changed (Data is to save in New File?) or Not Open...
          if((strcmp(rmap_file_name_wr, rmap_file_name_check)) || (!rmapWrFile)) {
            // Save new file_name for next control
            strcpy(rmap_file_name_wr, rmap_file_name_check);
            // Not opened? Open... in append
            if(rmapWrFile) rmapWrFile.close();
            rmapWrFile = SD.open(rmap_file_name_wr, O_RDWR | O_CREAT | O_AT_END);
            if (rmapWrFile) {
              pruneOldestDataFile(rmap_file_name_wr, rmap_file_name_rd);
            }
            // Open File High LED
            #ifdef PIN_SD_LED
            digitalWrite(PIN_SD_LED, HIGH);
            #endif
          }
          // All correct... Write Block of data
          if(rmapWrFile) {
            bWriteErr = false;
            bWriteErr |= !rmapWrFile.write(&rmap_put_archive_data, sizeof(rmap_put_archive_data));
            if (!bWriteErr) {
              rmapWrFile.flush();
              // Now we can remove data from queue
              param.dataRmapPutQueue->Dequeue(&rmap_put_archive_data);
              // System status enter in data ready for SENT Not required
              // (new data present is set into CAN Receive data when queue become not empty)
            } else {
              // Generic open file Error
              error_sd_card = true;
            }
          } else {
            // Generic open file Error
            error_sd_card = true;
          }
        }
        // Close File Low LED
        #ifdef PIN_SD_LED
        digitalWrite(PIN_SD_LED, LOW);
        #endif
      }
      // *********************************************************
      //         End OF perform RMAP Write append message
      // *********************************************************

      // Reset pointer?
      if(rmap_pointer_seek == UNKNOWN_POINTER_POSITION) {
        memset(&rmap_get_request, 0, sizeof(rmap_get_request));
        rmap_get_request.command.do_reset_ptr = true;
        // Go to last data 
        rmap_get_request.param = (rmap_pointer_datetime / SECS_DAY) * SECS_DAY;
        param.dataRmapGetRequestQueue->Enqueue(&rmap_get_request, Ticks::MsToTicks(FILE_IO_PTR_QUEUE_TIMEOUT));
      }

      // *********************************************************
      //         Perform FILE (DATA RMAP) READ data block
      // *********************************************************
      // External request RMAP data block (Cypal DSDL Format)
      // Get Block and send with queue to REQUEST (MQTT Task) Get Data Update command
      // Set request with 2 options standard do_get_data (get next data avaiable if exist)
      // Option 2 do_synch_ptr, search file_name and pointer from request to now()
      // Each operation modify current pointer stored in a file for prepare standard do_get_data
      while(!param.dataRmapGetRequestQueue->IsEmpty()) {
        // Exit while on Error
        if(error_sd_card) break;
        // Try Get message from queue (Start, progress session download fron NETWORK TASK and push to SD CARD)
        // Send response -> system_reesponse generic mode to request
        // Request Pointer SET Modify rmap_file_name_rd and current Opened File for Reading Data from External QUEUE Request
        // Get Pointer, Get Data from File Opened. If Data Change File Day Archive (Data is Next Day From last request)
        // rmap_file_name_rd automatic close and reopen with New Day Archive. Data Are Opened in ReadOnlyMode
        // Resynch file are security made when New Data avaiable In Write File (system_status->new data avaiable)
        if(param.dataRmapGetRequestQueue->Dequeue(&rmap_get_request)) {
          // Locking data session (Get Request Operation)
          memset(&rmap_get_response, 0, sizeof(rmap_get_response));
          // ******************************************************************
          //           Request is set pointer to date/time?
          // ******************************************************************
          if((rmap_get_request.command.do_synch_ptr) || (rmap_get_request.command.do_reset_ptr)) {
            bool is_found = false;
            char rmap_file_name_new[DATA_FILENAME_LEN]; // Work with temp Name file (SET in Pointer only if all right)
            uint32_t dateTimeSearch = rmap_get_request.param;
            if (rmapRdFile) {
              rmapRdFile.close();
              rmap_rd_file_open = false;
            }
            rmap_pointer_datetime_bkp = rmap_pointer_datetime;
            rmap_pointer_seek_bkp = rmap_pointer_seek;
            strcpy(rmap_file_name_rd_bkp, rmap_file_name_rd);
            rmap_file_name_rd[0] = 0;
            // Trace INFO Queue Request SET Pointer TO->
            DateTime rmap_date_time_val;
            convertUnixTimeToDate(dateTimeSearch, &rmap_date_time_val);
            if(rmap_get_request.command.do_reset_ptr) {
              TRACE_INFO_F(F("Data RMAP reset pointer date/time at [ %s ]\r\n"), formatDate(&rmap_date_time_val, NULL));
            } else {
              TRACE_INFO_F(F("Data RMAP requested search pointer date/time at [ %s ]\r\n"), formatDate(&rmap_date_time_val, NULL));
            }
            // Check name File            
            namingFileData(dateTimeSearch, "/data", rmap_file_name_new);
            // If Exist, search pointer (correct position) into file
            // Search block dateTime to synch pointer requested
            if(SD.exists(rmap_file_name_new)) {
              // Request Name File EXIST
              // Found OK
              is_found = true;
              // Reset current dateTime control position
              uint32_t currReadDateTimeFile = 0;
              // Search pointer into file... (Open as a Temp File)
              tmpFile = SD.open(rmap_file_name_new, O_RDONLY);
              // Correctly opened..
              if(tmpFile) {
                // Open File High LED
                #ifdef PIN_SD_LED
                digitalWrite(PIN_SD_LED, HIGH);
                #endif
                // Search wile dateTime block into file are >= to requested dateTime block
                while(true) {
                  // Operation perform non blocking TASK
                  TaskWatchDog(TASK_WAIT_REALTIME_DELAY_MS);
                  Delay(Ticks::MsToTicks(TASK_WAIT_REALTIME_DELAY_MS));
                  // Save position before read dateTime (set back)
                  uint32_t peek_rmap_pointer = tmpFile.curPosition();
                  // No more data avaiable?... Not Found
                  if(!tmpFile.available()) {
                    // EOF Not found, but searching procedure are correct 
                    // Pointer requested is over last data. Set Value to CurrentPosition (DateTime to Request)
                    rmap_pointer_seek = peek_rmap_pointer;
                    rmap_pointer_datetime = dateTimeSearch;
                    // Procedure can go right (response... no more data avaiable)
                    break;
                  }
                  // Read block RMAP (to check dateTime)
                  int bytes_readed = tmpFile.read(&rmap_get_response.rmap_data, sizeof(rmap_get_response.rmap_data));
                  #if (ENABLE_STACK_USAGE)
                  TaskMonitorStack();
                  #endif
                  // Block read size is correct
                  if(bytes_readed == sizeof(rmap_get_response.rmap_data)) {
                    // Get dateTime of block
                    currReadDateTimeFile = rmap_get_response.rmap_data.date_time;
                    // Check if block DateTime is found
                    convertUnixTimeToDate(currReadDateTimeFile, &rmap_date_time_val);
                    if(!rmap_get_request.command.do_reset_ptr) {
                      TRACE_VERBOSE_F(F("Data RMAP current searching date/time (Readed) [ %s ]\r\n"), formatDate(&rmap_date_time_val, NULL));
                    }
                    if(currReadDateTimeFile >= dateTimeSearch) {
                      // Found first dateTime block compilant with initial position read (peek...)
                      rmap_pointer_seek = peek_rmap_pointer;
                      rmap_pointer_datetime = currReadDateTimeFile;
                      break;
                    }
                  } else {
                    // Error readed block not correctly dimensioned
                    // Generic open file Error
                    error_sd_card = true;
                    file_get_response.error_operation = true;
                    break;
                  }
                }
                tmpFile.close();
                // Close File Low LED
                #ifdef PIN_SD_LED
                digitalWrite(PIN_SD_LED, LOW);
                #endif
              } else {
                // Error opening file
                file_get_response.error_operation = true;
                // Generic open file Error
                error_sd_card = true;
              }
            } else {
              // New created pointer file and not existing file, search abort...
              if(!rmap_get_request.command.do_reset_ptr) {
                // Request Name File NOT EXIST (Search another file in date sequence)
                // Reading Current epoch to STOP Searching (No data avaiable in the future)
                // Stop on first data found over requested date pointer
                uint32_t currEpochLimitCheck;
                if (param.rtcLock->Take(Ticks::MsToTicks(RTC_WAIT_DELAY_MS))) {
                  currEpochLimitCheck = rtc.getEpoch();
                  param.rtcLock->Give();
                }
                char rmap_file_name_new[DATA_FILENAME_LEN]; // Work with temp Name file (SET in Pointer only if all right)
                dateTimeSearch = (dateTimeSearch / SECS_DAY) * SECS_DAY; // Around to DataeTime Hour 00:00:00
                while(true) {
                  // Operation perform non blocking TASK
                  TaskWatchDog(TASK_WAIT_REALTIME_DELAY_MS);
                  Delay(Ticks::MsToTicks(TASK_WAIT_REALTIME_DELAY_MS));
                  #if (ENABLE_STACK_USAGE)
                  TaskMonitorStack();
                  #endif
                  // Add time second day -> set Next Epoch Day
                  // If found, seek pointer are set to first block of data
                  // because the requested date is necessarily higher
                  dateTimeSearch += SECS_DAY;                
                  convertUnixTimeToDate(dateTimeSearch, &rmap_date_time_val);
                  TRACE_DEBUG_F(F("Data RMAP current searching date/time (Not readed) [ %s ]\r\n"), formatDate(&rmap_date_time_val, NULL));
                  namingFileData(dateTimeSearch, "/data", rmap_file_name_new);
                  // Exist?
                  if(SD.exists(rmap_file_name_new)) {
                    // FOUND FILE NEXT DATE
                    is_found = true;
                    // Real DateTime Pointer will be set on First GetData. DataPtr is setted to Day_00:00:00
                    rmap_pointer_datetime = dateTimeSearch;
                    rmap_pointer_seek = 0;
                    break;
                  } else {
                    // Exit when date_time is > now()
                    // No data found...
                    // No modify date_time pointer RMAP
                    if(dateTimeSearch >= currEpochLimitCheck) break;
                  }
                }
              } else {
                // Need to save resetted pointer
                rmap_get_request.command.do_save_ptr = true;
              }
            }
            // Found file and position correct?...
            if((!is_found)||(rmap_get_response.result.event_error)) {
              // Error procedure... or Not Found
              TRACE_VERBOSE_F(F("Data RMAP current searching date/time FOUND [ %s ]\r\n"), ERROR_STRING);
              // Need to save resetted pointer
              rmap_get_request.command.do_save_ptr = true;
              rmap_get_response.result.event_error = true;
            } else {
              // Responding data pointer Setted
              if(!rmap_get_request.command.do_reset_ptr) {
                TRACE_VERBOSE_F(F("Data RMAP current searching date/time FOUND [ %s ]\r\n"), OK_STRING);
              }
              rmap_get_response.result.done_synch = true;
            }
            // All OK?
            if(rmap_get_response.result.done_synch) {
              if(rmap_rd_file_open) {
                rmapRdFile.close();
                rmap_rd_file_open = false;
              }
              if (rmap_get_request.command.do_reset_ptr) {
                rmap_get_request.command.do_save_ptr = true;
              }
              param.systemStatusLock->Take();
              param.system_status->flags.new_data_to_send = true;
              param.systemStatusLock->Give();
            }
            // ***** Send response to request except internal request of reset *****
            if(!rmap_get_request.command.do_reset_ptr) {
              param.dataRmapGetResponseQueue->Enqueue(&rmap_get_response);
            }
          }
          // ******************************************************************
          //           Request is end pointer to date/time?
          // ******************************************************************
          else if(rmap_get_request.command.do_end_ptr) {
            using_rmap_pointer_datetime_end = true;
            rmap_pointer_datetime_end = rmap_get_request.param;
            // Trace INFO Queue Request SET END Pointer TO->
            DateTime rmap_date_time_val;
            convertUnixTimeToDate(rmap_pointer_datetime_end, &rmap_date_time_val);
            TRACE_INFO_F(F("Data RMAP requested end pointer date/time at [ %s ]\r\n"), formatDate(&rmap_date_time_val, NULL));
            rmap_get_response.result.done_synch = true;
            // ***** Send response to request *****
            param.dataRmapGetResponseQueue->Enqueue(&rmap_get_response);
          }
          // ******************************************************************
          // Request is previous pointer to date/time? (Something wrong sending)
          // ******************************************************************
          else if(rmap_get_request.command.do_previous_ptr) {
            // Restore previous position            
            rmap_pointer_seek = rmap_pointer_seek_prv;
            rmap_pointer_datetime = rmap_pointer_datetime_prv;
            namingFileData(rmap_pointer_datetime, "/data", rmap_file_name_check);
            // Day Name File Changed or Not, required reSynch PTR with previous position
            strcpy(rmap_file_name_rd, rmap_file_name_check);
            if(rmap_rd_file_open) {
              rmapRdFile.close();
              rmap_rd_file_open = false;
            }
            rmapRdFile = SD.open(rmap_file_name_rd, O_RDONLY);
            if (rmapRdFile) {
              rmap_rd_file_open = true;
              rmapRdFile.seek(rmap_pointer_seek);
            }
            // Open File High LED
            #ifdef PIN_SD_LED
            digitalWrite(PIN_SD_LED, HIGH);
            #endif
            // Auto save position (if reboot system send data ca be start from here...)
            rmap_get_request.command.do_save_ptr = true;
            TRACE_INFO_F(F("Data RMAP restore pointer to previous position\r\n"), NULL);
            rmap_get_response.result.done_synch = true;
            // ***** Send response to request *****
            param.dataRmapGetResponseQueue->Enqueue(&rmap_get_response);
          }
          // ******************************************************************
          // Request next avaiable data? ( N.B. Standard Request for GET DATA )
          // ******************************************************************
          else if(rmap_get_request.command.do_get_data) {
            namingFileData(rmap_pointer_datetime, "/data", rmap_file_name_check);
            // Day Name File Changed (Data is to save in New File? Or Not Realy Open...)
            if((strcmp(rmap_file_name_rd, rmap_file_name_check)) || (!rmap_rd_file_open)) {
              strcpy(rmap_file_name_rd, rmap_file_name_check);
              bool reSyncPtr;
              reSyncPtr = (!rmap_rd_file_open);
              if(rmap_rd_file_open) {
                rmapRdFile.close();
                rmap_rd_file_open = false;
              }
              rmapRdFile = SD.open(rmap_file_name_rd, O_RDONLY);
              if (rmapRdFile) rmap_rd_file_open = true;
              // Resync Position Before Last Data Read OtherWise Other File (SeekPosition = 0)
              if (reSyncPtr && rmap_rd_file_open) rmapRdFile.seek(rmap_pointer_seek);
              else rmap_pointer_seek = 0;
              // Open File High LED
              #ifdef PIN_SD_LED
              digitalWrite(PIN_SD_LED, HIGH);
              #endif
            }
            memset(&rmap_get_response, 0, sizeof(rmap_get_response));
            if(rmap_rd_file_open) {
              // Not avaiable, EOF...
              if(rmapRdFile.available()) {
                // Not read size correct block?... Error
                // Read File High LED
                #ifdef PIN_SD_LED
                digitalWrite(PIN_SD_LED, HIGH);
                #endif
                int bytes_readed = rmapRdFile.read(&rmap_get_response.rmap_data, sizeof(rmap_get_response.rmap_data));
                if(bytes_readed == sizeof(rmap_get_response.rmap_data)) {
                  // Backup previous pointer position (After read next seek position)
                  rmap_pointer_seek_prv = rmap_pointer_seek;
                  rmap_pointer_datetime_prv = rmap_pointer_datetime;
                  // CurPosition Check assert(bytes_readed+=sizeof(rmap_get_response.rmap_data))
                  rmap_pointer_seek = rmapRdFile.curPosition();
                  rmap_get_response.result.done_get_data = true;
                  // Set DateTime Local Pointer correct
                  rmap_pointer_datetime = rmap_get_response.rmap_data.date_time;
                  // Send an EOF with a block data if last block
                  if(!rmapRdFile.available()) {
                    // Check if another Day (Next) is present before sending End Of Data
                    // If Exist The Seek Pointer Have to be resetted to Init Value (First Data of New File)
                    namingFileData(rmap_pointer_datetime + SECS_DAY, "/data", rmap_file_name_check);
                    // Not Exist? End Of Data, Otherwise next request in New Day Direct open Day File without other operation
                    if(SD.exists(rmap_file_name_check)) {
                      // Reopen Operation can be Start Immediatly.
                      // Set SEEK Position to Start File and DateTime to hh:nn:ss at 0.0.0 Begin of Day
                      rmap_pointer_seek = 0;
                      rmap_pointer_datetime = ((rmap_pointer_datetime + SECS_DAY) / SECS_DAY) * SECS_DAY;
                      // Save new file_name for next control
                      strcpy(rmap_file_name_rd, rmap_file_name_check);
                      // Not opened? Open... in readonly
                      if(rmap_rd_file_open) {
                        rmapRdFile.close();
                        rmap_rd_file_open = false;
                      }
                      rmapRdFile = SD.open(rmap_file_name_rd, O_RDONLY);
                      if (rmapRdFile) rmap_rd_file_open = true;
                      // Not required Save pointer, data can be continued (rmap_pointer_seek is resetted to 0)
                      // Open File High LED
                      #ifdef PIN_SD_LED
                      digitalWrite(PIN_SD_LED, HIGH);
                      #endif
                    } else {
                      // Gap (missing day): jump to next existing YYYY_MM_DD.dat if any
                      char after_base[DATA_FILENAME_LEN] = {0};
                      const char *slash = strrchr(rmap_file_name_rd, '/');
                      strncpy(after_base, slash ? (slash + 1) : rmap_file_name_rd, sizeof(after_base) - 1);
                      uint32_t next_ep = 0;
                      if (findNextArchiveDat("/data", after_base, rmap_file_name_check,
                                             sizeof(rmap_file_name_check), &next_ep)) {
                        const char *to_base = strrchr(rmap_file_name_check, '/');
                        TRACE_INFO_F(F("SD: rmap gap jump %s -> %s\r\n"), after_base,
                                     to_base ? (to_base + 1) : rmap_file_name_check);
                        rmap_pointer_seek = 0;
                        rmap_pointer_datetime = next_ep;
                        strcpy(rmap_file_name_rd, rmap_file_name_check);
                        if (rmap_rd_file_open) {
                          rmapRdFile.close();
                          rmap_rd_file_open = false;
                        }
                        rmapRdFile = SD.open(rmap_file_name_rd, O_RDONLY);
                        if (rmapRdFile) rmap_rd_file_open = true;
                        #ifdef PIN_SD_LED
                        digitalWrite(PIN_SD_LED, HIGH);
                        #endif
                      } else {
                        rmap_get_response.result.end_of_data = true;
                        param.systemStatusLock->Take();
                        param.system_status->flags.new_data_to_send = false;
                        param.systemStatusLock->Give();
                      }
                    }
                  }
                } else {
                  // Error readed block not correctly dimensioned
                  rmap_get_response.result.event_error = true;
                  // Generic open file Error
                  error_sd_card = true;
                }
              } else {
                // Prefer next calendar day; on gap, jump to next existing archive .dat
                namingFileData(rmap_pointer_datetime + SECS_DAY, "/data", rmap_file_name_check);
                bool advanced = false;
                if (SD.exists(rmap_file_name_check)) {
                  rmap_pointer_datetime = ((rmap_pointer_datetime + SECS_DAY) / SECS_DAY) * SECS_DAY;
                  rmap_pointer_seek = 0;
                  strcpy(rmap_file_name_rd, rmap_file_name_check);
                  advanced = true;
                } else {
                  char after_base[DATA_FILENAME_LEN] = {0};
                  namingFileData(rmap_pointer_datetime, "/data", rmap_file_name_check);
                  const char *slash = strrchr(rmap_file_name_check, '/');
                  strncpy(after_base, slash ? (slash + 1) : rmap_file_name_check, sizeof(after_base) - 1);
                  uint32_t next_ep = 0;
                  if (findNextArchiveDat("/data", after_base, rmap_file_name_check,
                                         sizeof(rmap_file_name_check), &next_ep)) {
                    const char *to_base = strrchr(rmap_file_name_check, '/');
                    TRACE_INFO_F(F("SD: rmap gap jump %s -> %s\r\n"), after_base,
                                 to_base ? (to_base + 1) : rmap_file_name_check);
                    rmap_pointer_datetime = next_ep;
                    rmap_pointer_seek = 0;
                    strcpy(rmap_file_name_rd, rmap_file_name_check);
                    advanced = true;
                  }
                }
                if (advanced) {
                  if (rmap_rd_file_open) {
                    rmapRdFile.close();
                    rmap_rd_file_open = false;
                  }
                  rmapRdFile = SD.open(rmap_file_name_rd, O_RDONLY);
                  if (rmapRdFile) rmap_rd_file_open = true;
                  #ifdef PIN_SD_LED
                  digitalWrite(PIN_SD_LED, HIGH);
                  #endif
                } else {
                  rmap_get_response.result.end_of_data = true;
                  param.systemStatusLock->Take();
                  param.system_status->flags.new_data_to_send = false;
                  param.systemStatusLock->Give();
                }
              }
            } else {
              // Error on open file
              rmap_get_response.result.event_error = true;
              error_sd_card = true;
            }
            // Are currently function of SET POINTER START / END ?
            // Post in ALL Data complete search OK. IF No More DATA. AUtomatically End
            // Optional procedure in running (Set END Pointer)
            // DateTime is > DatEnd Request? Procedure Have to End and PointerData is
            // Restored with Backupped Pointer.
            // N.B. using_rmap_pointer_datetime_end? CAN Modify Response !!!
            if(using_rmap_pointer_datetime_end) {
              // End Of Data... -> End of Pointer End Data No more data avaiable
              // Probabiliy set request as Error > LastDateTime Avaiable
              if(rmap_get_response.result.end_of_data) {
                using_rmap_pointer_datetime_end = false;
                // Save Pointer automatically because END OF Data Received > Last Data Avaiable
                // Is Same of Data END in Standard MODE ( Save immediatly the pointer data )
                rmap_get_request.command.do_save_ptr = true;
                // No ned restore older pointer...
              } else {
                if(rmap_get_response.rmap_data.date_time > rmap_pointer_datetime_end) {
                  // Terminate END Control procedure, Return with backup at standard method
                  using_rmap_pointer_datetime_end = false;
                  // Simulate an End Of Data. Data END is reached up!!! End of procedure
                  rmap_get_response.result.end_of_data = true;
                  // Restore older Pointer Seek and File. Previous backup from RPC Recovery request.
                  rmap_pointer_datetime = rmap_pointer_datetime_bkp;
                  rmap_pointer_seek = rmap_pointer_seek_bkp;
                  strcpy(rmap_file_name_rd, rmap_file_name_rd_bkp);
                  // Need to close file to check next block if external write. No more data if not reclose/reopen File
                  if(rmap_rd_file_open) {
                    rmapRdFile.close();
                    rmap_rd_file_open = false;
                  }
                }
              }
            } else {
              // In standard Mode if END Of Data Receive, automatically Save Pointer DATA
              if(rmap_get_response.result.end_of_data) {
                rmap_get_request.command.do_save_ptr = true;
                // Need to close file to check next block if external write. No more data if not reclose/reopen File
                if(rmap_rd_file_open) {
                  rmapRdFile.close();
                  rmap_rd_file_open = false;
                }
              }
            }
            // ***** Send response to request *****
            param.dataRmapGetResponseQueue->Enqueue(&rmap_get_response);
            // Close File Low LED
            #ifdef PIN_SD_LED
            digitalWrite(PIN_SD_LED, LOW);
            #endif
          }
          // ******************************************************************
          //        Request is save current Seek and DateTime pointer
          // ******************************************************************
          // Non esclusive command (Not else_if) Save PTR Can Be executed all request
          // But for Fast Speed we can Call this function on End of Data Transmit
          // Or if call down and data cannot end process upload (From extern)
          if(rmap_get_request.command.do_save_ptr) {
            bool rmap_save_ok = false;
            // Rewrite Pointer Data File (Open only at startup for Set Position)
            tmpFile = SD.open("/data/pointer.dat", O_RDWR | O_CREAT);
            if(tmpFile) {
              uint32_t save_datetime = rmap_pointer_datetime;
              uint32_t save_seek = rmap_pointer_seek;
              // param==1: save pre-read position (_prv) = last fully published MQTT block
              if (rmap_get_request.param == 1u) {
                save_datetime = rmap_pointer_datetime_prv;
                save_seek = rmap_pointer_seek_prv;
              }
              if(save_seek == UNKNOWN_POINTER_POSITION) save_seek = 0;
              uint32_t now_ep = 0;
              if (param.rtcLock->Take(Ticks::MsToTicks(RTC_WAIT_DELAY_MS))) {
                now_ep = rtc.getEpoch();
                param.rtcLock->Give();
              }
              const uint32_t future_margin = (uint32_t)param.configuration->report_s * 2u;
              const bool future_ptr = param.system_status->connection.is_ntp_synchronized &&
                                      (now_ep > 0) &&
                                      (save_datetime > (now_ep + future_margin));
              if (future_ptr) {
                TRACE_INFO_F(F("SD: rmap skip save pointer in future (%lu > now %lu)\r\n"),
                             (unsigned long)save_datetime, (unsigned long)now_ep);
                rmap_pointer_datetime = rmap_pointer_datetime_bkp;
                rmap_pointer_seek = rmap_pointer_seek_bkp;
                tmpFile.close();
              } else {
                // Open File High LED
                #ifdef PIN_SD_LED
                digitalWrite(PIN_SD_LED, HIGH);
                #endif
                bWriteErr = false;
                bWriteErr |= !tmpFile.write(&save_datetime, sizeof(save_datetime));
                bWriteErr |= !tmpFile.write(&save_seek, sizeof(save_seek));
                tmpFile.close();
                // Close File Low LED
                #ifdef PIN_SD_LED
                digitalWrite(PIN_SD_LED, LOW);
                #endif
                if (!bWriteErr) {
                  rmap_pointer_datetime_bkp = save_datetime;
                  rmap_pointer_seek_bkp = save_seek;
                  rmap_pointer_datetime_prv = save_datetime;
                  rmap_pointer_seek_prv = save_seek;
                  if (rmap_get_request.param != 1u) {
                    rmap_pointer_datetime = save_datetime;
                    rmap_pointer_seek = save_seek;
                  }
                  DateTime ptrOk;
                  convertUnixTimeToDate(save_datetime, &ptrOk);
                  TRACE_INFO_F(F("SD: rmap save pointer OK at [ %s ] seek %lu (confirmed=%u)\r\n"),
                               formatDate(&ptrOk, NULL), (unsigned long)save_seek,
                               (unsigned)(rmap_get_request.param == 1u ? 1u : 0u));
                  rmap_save_ok = true;
                } else {
                  error_sd_card = true;
                  TRACE_ERROR_F(F("SD: rmap save pointer write failed\r\n"));
                }
              }
            } else {
              error_sd_card = true;
              TRACE_ERROR_F(F("SD: rmap save pointer open failed\r\n"));
            }
            if (rmap_get_request.command.do_save_ptr &&
                !rmap_get_request.command.do_get_data &&
                !rmap_get_request.command.do_synch_ptr &&
                !rmap_get_request.command.do_reset_ptr &&
                !rmap_get_request.command.do_previous_ptr &&
                !rmap_get_request.command.do_end_ptr) {
              memset(&rmap_get_response, 0, sizeof(rmap_get_response));
              rmap_get_response.result.done_synch = rmap_save_ok;
              param.dataRmapGetResponseQueue->Enqueue(&rmap_get_response);
            }
          }
        }
      }
      // *********************************************************
      //       END Perform FILE (DATA RMAP) READ data block
      // *********************************************************

      // *********************************************************
      //       Perform FILE (FIRMWARE) WRITE append message
      // *********************************************************
      // If element get all element from the queue and Put to SD N.B. If session running (Uploading...) Name File != NULL
      // If remote_file_name != NULL remote_file_name is not ready to system (Put firmware into module local and remote)
      // Any request for file (es. Cypal Request firmware are blocked)
      while(!param.dataFilePutRequestQueue->IsEmpty()) {
        // Exit while on Error
        if(error_sd_card) break;
        // Try Get message from queue (Start, progress session download fron NETWORK TASK and push to SD CARD)
        // Send response -> system_reesponse generic mode to request
        if(param.dataFilePutRequestQueue->Dequeue(&file_put_request)) {
          // Put the TASK in real_time mode to minimize external queue waiting and not blocking task (SD)
          is_real_time_task = true;
          // Put to SD ( CREATE / APPEND Firmware Block File session )
          if(file_put_request.block_type == file_block_type::file_name) {
            // Get File name set file name Upload (session current START)
            memset(remote_file_name, 0, sizeof(remote_file_name));
            strcpy(remote_file_name, "/firmware/");
            memcpy(remote_file_name + strlen(remote_file_name), file_put_request.block, file_put_request.block_lenght);
            // Create File in ReWrite Mode
            // Locking file session (uploading...)
            memset(&file_put_response, 0, sizeof(file_put_response));
            // Open Put File
            // Create/overwrite: without O_TRUNC a shorter re-download leaves old trailing bytes
            // → fw_upgradable true, flash fails, bootloader rollback (station survives, update fails)
            if (SD.exists(remote_file_name)) {
              SD.remove(remote_file_name);
            }
            putFile = SD.open(remote_file_name, O_RDWR | O_CREAT | O_TRUNC);
            // Open File High LED
            #ifdef PIN_SD_LED
            digitalWrite(PIN_SD_LED, HIGH);
            #endif
            if(!putFile)
              file_put_response.done_operation = false;
            else
              file_put_response.done_operation = true;
            // Send response to caller
            param.dataFilePutResponseQueue->Enqueue(&file_put_response);
          } else if(file_put_request.block_type == file_block_type::data_chunck) {
            memset(&file_put_response, 0, sizeof(file_put_response));
            if(putFile) {
              bWriteErr = false;
              bWriteErr |= !putFile.write(file_put_request.block, file_put_request.block_lenght);
              putFile.flush();
              // Bytes written is err or ok?
              if(bWriteErr) {
                file_put_response.error_operation = true;
                error_sd_card = true;
              }
              else
              {
                file_put_response.done_operation = true;
              }
            } else {
              file_put_response.error_operation = true;
            }
            // Send response to caller
            param.dataFilePutResponseQueue->Enqueue(&file_put_response);
          } else if(file_put_request.block_type == file_block_type::kill_file) {
            // Security Check remote_file_name is valid before continue, more exit from Stream are possible
            if(strlen(remote_file_name) > 0) {
              // Close file in append if again opened
              putFile.close();
              // Remove file name Upload (session current upload ERROR)
              SD.remove(remote_file_name);
              // Unlock session. File is ready for the system (without integrity control)
              memset(remote_file_name, 0, sizeof(remote_file_name));
              // Remove File Low LED
              #ifdef PIN_SD_LED
              digitalWrite(PIN_SD_LED, LOW);
              #endif
            }
            // Responding to queue command
            memset(&file_put_response, 0, sizeof(file_put_response));
            file_put_response.done_operation = true;
            param.dataFilePutResponseQueue->Enqueue(&file_put_response);
          } else if(file_put_request.block_type == file_block_type::end_of_file) {
            // Remove file name Upload (session current END)
            // Unlock session. File is ready for the system (without integrity control)
            memset(remote_file_name, 0, sizeof(remote_file_name));
            // Send response to caller ... OK done
            putFile.close();
            // Close File Low LED
            #ifdef PIN_SD_LED
            digitalWrite(PIN_SD_LED, LOW);
            #endif
            memset(&file_put_response, 0, sizeof(file_put_response));
            file_put_response.done_operation = true;
            param.dataFilePutResponseQueue->Enqueue(&file_put_response);
          } else if(file_put_request.block_type == file_block_type::ctrl_checksum) {
            // Remove file name Upload (session current END)
            // Unlock session. File is ready for the system
            // Need to control checksum (if any error file have to delete)
            memset(remote_file_name, 0, sizeof(remote_file_name));
            // Send response to caller ... OK done
            memset(&file_put_response, 0, sizeof(file_put_response));
            file_put_response.done_operation = true;
            param.dataFilePutResponseQueue->Enqueue(&file_put_response);
          }
        }
        // On Exit Low LED
        #ifdef PIN_SD_LED
        digitalWrite(PIN_SD_LED, LOW);
        #endif
      }
      // *********************************************************
      //        End OF FILE (FIRMWARE) WRITE append message
      // *********************************************************

      // *********************************************************
      //        Perform FILE (FIRMWARE) READ block message
      // *********************************************************
      // External request Firmware data block (Cypal...)
      // Get Block and send with queue to CAN and CAN Upload remote module... FW/File Update command
      // Flag Firmware (Receive from Network) toBe Completed before request to upload can start
      while(!param.dataFileGetRequestQueue->IsEmpty()) {
        // Exit while on Error
        if(error_sd_card) break;
        // Try Get message from queue (Start, progress session download fron NETWORK TASK and push to SD CARD)
        // Send response -> system_reesponse generic mode to request
        if(param.dataFileGetRequestQueue->Dequeue(&file_get_request)) {
          // Locking file session (uploading...)
          memset(&file_get_response, 0, sizeof(file_get_response));
          // Put to SD ( Start Read Firmware Block File session )
          if(file_get_request.block_id==0) {
            // Closing if file already used for other Session (preserve memoryLeak error)
            if(getFile[file_get_request.board_id]) getFile[file_get_request.board_id].close();
            // Get File name set file name Upload (session current START)
            memset(local_file_name, 0, sizeof(local_file_name));
            strcpy(local_file_name, "/firmware/");
            strcat(local_file_name + strlen(local_file_name), file_get_request.file_name);
            // Create File in ReWrite Mode
            // Open Get File (for reading)
            getFile[file_get_request.board_id] = SD.open(local_file_name, O_RDONLY);
            if(getFile[file_get_request.board_id]) {
              // Open File High LED
              #ifdef PIN_SD_LED
              digitalWrite(PIN_SD_LED, HIGH);
              #endif
              // Read the first block data (return number of bytes read)
              file_get_response.done_operation = true;
              uint32_t avaiables = getFile[file_get_request.board_id].available();
              if(avaiables > FILE_GET_DATA_BLOCK_SIZE) avaiables = FILE_GET_DATA_BLOCK_SIZE;
              file_get_response.block_lenght = getFile[file_get_request.board_id].readBytes(file_get_response.block, avaiables);
            } else
              file_get_response.done_operation = false;
            // Send response to caller
            // Closing automatic of file if block not completed (EOF)
            if(file_get_response.block_lenght != FILE_GET_DATA_BLOCK_SIZE) {
              // Rapid return, before closing file
              param.dataFileGetResponseQueue->Enqueue(&file_get_response);
              getFile[file_get_request.board_id].close();
              // Close File Low LED
              #ifdef PIN_SD_LED
              digitalWrite(PIN_SD_LED, LOW);
              #endif
              // Unlock file (clear name)
              memset(local_file_name, 0, sizeof(local_file_name));
            } else
              param.dataFileGetResponseQueue->Enqueue(&file_get_response);
          } else {
            // Set Seek Position or Reading Next Block (only if block_read_next not called)
            if(!file_get_request.block_read_next) 
              getFile[file_get_request.board_id].seek(FILE_GET_DATA_BLOCK_SIZE * file_get_request.block_id);
            // Read the first block data (return number of bytes read)
            file_get_response.done_operation = true;
            // Read File High LED
            #ifdef PIN_SD_LED
            digitalWrite(PIN_SD_LED, HIGH);
            #endif
            uint32_t avaiables = getFile[file_get_request.board_id].available();
            if(avaiables > FILE_GET_DATA_BLOCK_SIZE) avaiables = FILE_GET_DATA_BLOCK_SIZE;
            file_get_response.block_lenght = getFile[file_get_request.board_id].readBytes(file_get_response.block, avaiables);
            // Closing automatic of file if block not completed (EOF)
            if(file_get_response.block_lenght != FILE_GET_DATA_BLOCK_SIZE) {
              // Rapid return before closing file
              param.dataFileGetResponseQueue->Enqueue(&file_get_response);
              getFile[file_get_request.board_id].close();
              // Close File Low LED
              #ifdef PIN_SD_LED
              digitalWrite(PIN_SD_LED, LOW);
              #endif
              // Unlock file (clear name)
              memset(local_file_name, 0, sizeof(local_file_name));
            } else
              // Send response to caller
              param.dataFileGetResponseQueue->Enqueue(&file_get_response);
          }
        }
        // On Exit Low LED
        #ifdef PIN_SD_LED
        digitalWrite(PIN_SD_LED, LOW);
        #endif
      }
      // *********************************************************
      //        End OF FILE (FIRMWARE) WRITE append message
      // *********************************************************

      // ***********************************************************
      // Generic open or Other Operation file Error... Restart Synch
      // ***********************************************************
      if(error_sd_card) {
        error_sd_card = false;
        sdPrepareRemount();
        state = SD_STATE_INIT;
      }

      break;

    case SD_UPLOAD_FIRMWARE_TO_FLASH:

      // *********************************************************
      //           Perform Local Firmware FLASH Update
      // *********************************************************

      // Check firmware file present Type, model and version
      fw_found = false;
      // Name of module
      getStimaNameByType(stima_name, param.configuration->module_type);
      // Check list Firmware File
      dir = SD.open("/firmware");
      while(true) {
        entry = dir.openNextFile();
        if(!entry) break;
        // Open File High LED
        #ifdef PIN_SD_LED
        digitalWrite(PIN_SD_LED, HIGH);
        #endif
        // Found firmware file?
        entry.getName(local_file_name, FILE_NAME_MAX_LENGHT);
        if(!checkStimaFirmwareType(local_file_name, &module_type_cast, &fw_version, &fw_revision)) {
          entry.close();
          // Close File Low LED
          #ifdef PIN_SD_LED
          digitalWrite(PIN_SD_LED, LOW);
          #endif
        } else {
          module_type = static_cast<Module_Type>(module_type_cast);
          // Is this module ?
          if(module_type != param.configuration->module_type) {
            entry.close();
            // Close File Low LED
            #ifdef PIN_SD_LED
            digitalWrite(PIN_SD_LED, LOW);
            #endif
          } else {
            // if current version (last version into SD CARD?)
            bool is_last_firmware_on_sd = false;
            for(uint8_t brd=0; brd<STIMA_MODULE_TYPE_MAX_AVAIABLE; brd++) {
              if(param.system_status->boards_update_avaiable[brd].module_type == module_type) {
                if((fw_version == param.system_status->boards_update_avaiable[brd].version) &&
                   (fw_revision == param.system_status->boards_update_avaiable[brd].revision)) {
                  is_last_firmware_on_sd = true;
                  break;
                }
              }
            }
            // Is Last Firmware and Is Version i Major of Current Version?            
            if((is_last_firmware_on_sd) &&
              ((fw_version > param.configuration->module_main_version) ||
              ((fw_version == param.configuration->module_main_version) && (fw_revision > param.configuration->module_minor_version)))) {
              fw_found = true;
              // Get full name for local operation
              entry.getName(local_file_name, FILE_NAME_MAX_LENGHT);
              TRACE_INFO_F(F("SD: found firmware upgradable type: %s Ver %u.%u\r\n"), stima_name, fw_version, fw_revision);
              TRACE_INFO_F(F("SD: starting firmware upgrade...\r\n"));
              // Flag
              bool bFirstBlock = true;
              bool is_error = false;
              // Is file opened?
              if(entry) {
                int len_block;
                // Put firmware in correct Flash Location
                while(true) {
                  // *************  PREPARE FLASHING *************
                  // Read block data from file
                  // Read File High LED
                  #ifdef PIN_SD_LED
                  digitalWrite(PIN_SD_LED, HIGH);
                  #endif
                  len_block = entry.read(data_block, SD_FW_BLOCK_SIZE);
                  if(len_block < 0) {
                    is_error = true;
                    break;
                  }
                  // Signal to LCD updating fw to flash now from SD Card (Direct or downloaded from HTTP)
                  param.systemStatusLock->Take();
                  param.system_status->flags.fw_updating = true;
                  param.systemStatusLock->Give();
                  // Append block firmware file to flash (same of CAN FW Upgrade)
                  // EOF when block != SD_FW_BLOCK_SIZE (UAVCAN TYPE_LEN 256 BYTES)
                  if(!putFlashFile(local_file_name, true, bFirstBlock, data_block, len_block)) {
                    is_error = true;
                    break;
                  }
                  bFirstBlock = false;
                  if(len_block != SD_FW_BLOCK_SIZE) {
                    // EOF
                    break;
                  }
                  // WDT non blocking task (Delay basic operation)
                  TaskWatchDog(TASK_WAIT_REALTIME_DELAY_MS);
                  Delay(Ticks::MsToTicks(TASK_WAIT_REALTIME_DELAY_MS));
                }
                entry.close();
                // Close File Low LED
                #ifdef PIN_SD_LED
                digitalWrite(PIN_SD_LED, LOW);
                #endif
                // Nothing error, starting firmware upgrade
                if(!is_error) {
                  // Optional send SIGTerm to all task
                  // WDT non blocking task (Delay basic operation)
                  TRACE_INFO_F(F("SD: firmware upgrading complete waiting reboot for start flashing...\r\n"));
                  // Preparo la struttua per informare il Boot Loader
                  param.boot_request->app_executed_ok = false;
                  param.boot_request->backup_executed = false;
                  param.boot_request->app_forcing_start = false;
                  param.boot_request->rollback_executed = false;
                  param.boot_request->request_upload = true;
                  param.eeprom->Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) param.boot_request, sizeof(bootloader_t));
                  // Wait for reset
                  TaskWatchDog(SD_TASK_WAIT_REBOOT_MS);
                  Delay(Ticks::MsToTicks(SD_TASK_WAIT_REBOOT_MS));
                  NVIC_SystemReset();
                }
              }
              // Error opening file or procedure upload
              // is_error (open_file OK, procedure error)
              // !is_error (open_file error)
              if(++retry>SD_TASK_GENERIC_RETRY) {
                // Abort MAX Retry
                TRACE_INFO_F(F("SD: firmware upgrading error, Max retry reached up. Abort flashing!!!\r\n"));
                if(!is_error) {
                  // ReSynch SD Card... Error when opening file
                  TRACE_VERBOSE_F(F("SD_UPLOAD_FIRMWARE_TO_FLASH -> SD_STATE_ERROR\r\n"));
                  state = SD_STATE_ERROR;
                } else {
                  // Error procedure Flashing... Can Retry from extern
                  TRACE_VERBOSE_F(F("SD_UPLOAD_FIRMWARE_TO_FLASH -> SD_STATE_WAITING_EVENT\r\n"));
                  state = SD_STATE_WAITING_EVENT;
                }
              } else {
                // Error, next Retry
                TRACE_INFO_F(F("SD: firmware upgrading error waiting retry\r\n"));
                TaskWatchDog(SD_TASK_GENERIC_RETRY_DELAY_MS);
                Delay(Ticks::MsToTicks(SD_TASK_GENERIC_RETRY_DELAY_MS));
              }
            }
            else
            {
              entry.close();
            }
          }
        }
      }
      dir.close();
      // Firmware not Found
      if(!fw_found) {
        TRACE_INFO_F(F("SD: module firmware for module %s not found\r\n"), stima_name);
        TRACE_VERBOSE_F(F("SD_UPLOAD_FIRMWARE_TO_FLASH -> SD_STATE_WAITING_EVENT\r\n"));
        state = SD_STATE_WAITING_EVENT;
      }
      // On Exit Low LED
      #ifdef PIN_SD_LED
      digitalWrite(PIN_SD_LED, LOW);
      #endif
      break;

    case SD_STATE_ERROR:
      // Gest Error... Resynch SD
      TRACE_VERBOSE_F(F("SD_STATE_ERROR -> SD_STATE_INIT\r\n"));
      state = SD_STATE_INIT;
      break;
    }

    #if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
    #endif

    // One step base non blocking switch
    // WDT non blocking task (Delay basic operation)
    if(is_real_time_task) {
      // Perform an real_time task wait (switching task not blocking and restart immediatly)
      TaskWatchDog(TASK_WAIT_REALTIME_DELAY_MS);
      Delay(Ticks::MsToTicks(TASK_WAIT_REALTIME_DELAY_MS));
      // Put request real_time to false. Next queue request must set this var to true
      is_real_time_task = false;
    }
    else
    {
      // Standard waiting
      TaskWatchDog(SD_TASK_WAIT_DELAY_MS);
      Delay(Ticks::MsToTicks(SD_TASK_WAIT_DELAY_MS));
    }
  }
}

#endif
