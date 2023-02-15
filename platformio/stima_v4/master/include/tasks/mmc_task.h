/**
 ******************************************************************************
 * @file    mmc_task.h
 * @author  Moreno Gasperini <m.gasperini@digiteco.it>
 * @brief   mmc_task header file (SDMMC StimaV4)
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

#ifndef _MMC_TASK_H
#define _MMC_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "stima_utility.h"
#include "str.h"

#if (ENABLE_MMC)

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_master_hal.hpp"

#include <STM32RTC.h>
#include <STM32SD.h>

// Flash Access
#include "drivers/flash.h"

#define MMC_TASK_WAIT_DELAY_MS            (50)
#define MMC_TASK_WAIT_OPERATION_DELAY_MS  (1)

#define MMC_TASK_WAIT_REBOOT_MS           (2500)

#define MMC_TASK_GENERIC_RETRY_DELAY_MS   (5000)
#define MMC_TASK_GENERIC_RETRY            (3)

#define MMC_FW_BLOCK_SIZE                 (256)

#if (ENABLE_I2C1 || ENABLE_I2C2)
#include <Wire.h>
#include "drivers/eeprom.h"
#endif

#include "debug_F.h"

#define MMCCard_Present()     (!digitalRead(PIN_MMC1_DTC))

typedef enum
{
  MMC_STATE_CREATE,
  MMC_STATE_INIT,
  MMC_STATE_CHECK_SD,    
  MMC_STATE_WAITING_EVENT,
  MMC_UPLOAD_FIRMWARE_TO_FLASH,
  MMC_STATE_ERROR
} MmcState_t;

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *wireLock;
  cpp_freertos::BinarySemaphore *qspiLock;
  cpp_freertos::BinarySemaphore *rtcLock;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::Queue *dataRmapPutQueue;
  cpp_freertos::Queue *dataLogPutQueue;
  cpp_freertos::Queue *dataFilePutRequestQueue;
  cpp_freertos::Queue *dataFilePutResponseQueue;
  Flash *flash;
  EEprom *eeprom;
} MmcParam_t;

class MmcTask : public cpp_freertos::Thread {

public:
  MmcTask(const char *taskName, uint16_t stackSize, uint8_t priority, MmcParam_t MmcParam);

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

  void FileOpenSecurity(File *reqFile, char* file_name, uint8_t file_mode);

  STM32RTC &rtc = STM32RTC::getInstance();

  MmcState_t state;
  MmcParam_t param;

  // Local flashPointer
  uint64_t mmcFlashPtr;
  uint16_t mmcFlashBlock;
};

#endif
#endif