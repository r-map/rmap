/**
 ******************************************************************************
 * @file    sd_task.h
 * @author  Moreno Gasperini <m.gasperini@digiteco.it>
 * @brief   sd_task header file (SD SPI StimaV4)
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

#ifndef _SD_TASK_H
#define _SD_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "stima_utility.h"
#include "str.h"

#if (ENABLE_SD)

// SD Param FS and Speed
#define SD_FAT_TYPE 3
#define SPI_SPEED SD_SCK_MHZ(8)

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_master_hal.hpp"

#if (ENABLE_SPI1 || ENABLE_SPI2)
#include <SPI.h>
#endif

// SD Fat
#include "SdFat.h"

#include <STM32RTC.h>

// Memory device Access
#include "drivers/flash.h"
#include "drivers/eeprom.h"

#define SD_TASK_WAIT_DELAY_MS            (50)

#define SD_TASK_SLEEP_DELAY_MS           (700)

#define SD_TASK_WAIT_REBOOT_MS           (250)

#define SD_TASK_GENERIC_RETRY_DELAY_MS   (5000)
#define SD_TASK_GENERIC_RETRY            (3)

#define SD_FW_BLOCK_SIZE                 (256)

#define FILE_NAME_MAX_LENGHT             (128)

#define DATA_FILENAME_LEN                (22)

#define UNKNOWN_POINTER_POSITION         (0xFFFFFFFFu)

#include "debug_F.h"

// TIME CONST and Define for Epoch function
#define	EPOCH_YR	              1970
#define	SECS_DAY	              86400ul
#define	LEAPYEAR(year)	        (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define	YEARSIZE(year)	        (LEAPYEAR(year) ? 366 : 365)
const int _ytab[2][12] = 
{
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

/// @brief struct local elaborate data parameter
typedef struct {
  configuration_t *configuration;                     //!< system configuration pointer struct
  system_status_t *system_status;                     //!< system status pointer struct
  bootloader_t *boot_request;                         //!< Boot struct pointer
  cpp_freertos::BinarySemaphore *qspiLock;            //!< Semaphore to QSPI Memory flash access
  cpp_freertos::BinarySemaphore *rtcLock;             //!< Semaphore to RTC Access
  cpp_freertos::BinarySemaphore *configurationLock;   //!< Semaphore to configuration access
  cpp_freertos::BinarySemaphore *systemStatusLock;    //!< Semaphore to system status access
  cpp_freertos::Queue *systemMessageQueue;            //!< Queue for system message
  cpp_freertos::Queue *dataRmapGetRequestQueue;       //!< Queue for access data RMAP Set Request
  cpp_freertos::Queue *dataRmapGetResponseQueue;      //!< Queue for access data RMAP Get Response
  cpp_freertos::Queue *dataRmapPutQueue;              //!< Queue for access data RMAP access Put Get Queue
  cpp_freertos::Queue *dataRmapPutBackupQueue;        //!< Queue for access data RMAP Put backup data
  cpp_freertos::Queue *dataLogPutQueue;               //!< Queue for system logging put data
  cpp_freertos::Queue *dataFilePutRequestQueue;       //!< Queue for Data Put File (firmware) Set request
  cpp_freertos::Queue *dataFilePutResponseQueue;      //!< Queue for Data Put File (firmware) Get response
  cpp_freertos::Queue *dataFileGetRequestQueue;       //!< Queue for Data Get File (firmware) Set request
  cpp_freertos::Queue *dataFileGetResponseQueue;      //!< Queue for Data Get File (firmware) Get response
  Flash *flash;                                       //!< Object Flash C++ access
  EEprom *eeprom;                                     //!< Object EEprom C++ access
} SdParam_t;

/// @brief SD TASK cpp_freertos class
class SdTask : public cpp_freertos::Thread {

  /// @brief Enum for state switch of running method
  typedef enum
  {
    SD_STATE_CREATE,
    SD_STATE_INIT,
    SD_STATE_INIT_SD,
    SD_STATE_TRUNCATE_DATA,
    SD_STATE_CHECK_STRUCTURE,
    SD_STATE_CHECK_DATA_PTR,
    SD_STATE_CHECK_FIRMWARE,
    SD_STATE_CLEAN_FIRMWARE,
    SD_STATE_WAITING_EVENT,
    SD_UPLOAD_FIRMWARE_TO_FLASH,
    SD_STATE_ERROR
  } SdState_t;

public:
  SdTask(const char *taskName, uint16_t stackSize, uint8_t priority, SdParam_t sdParam);

protected:
  virtual void Run();

private:

  #if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
  #endif
  void TaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  bool putFlashFile(const char* const file_name, const bool is_firmware, const bool rewrite, void* buf, size_t count);
  bool getFlashFwInfoFile(uint8_t *module_type, uint8_t *version, uint8_t *revision, uint64_t *len);

  void namingFileData(uint32_t time, char *dirPrefix, char* nameFile);

  SdState_t state;
  SdParam_t param;

  STM32RTC &rtc = STM32RTC::getInstance();

  SdFs SD; // SD Istance

  // Local flashPointer
  uint64_t sdFlashPtr;
  uint16_t sdFlashBlock;
};

#endif
#endif