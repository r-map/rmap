/**
 ******************************************************************************
 * @file    sd_task.h
 * @author  Moreno Gasperini <m.gasperini@digiteco.it>
 * @brief   sd_task header file (SD SPI StimaV4)
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

#ifndef _SD_TASK_H
#define _SD_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "stima_utility.h"
#include "str.h"

#if (ENABLE_SD)

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_master_hal.hpp"

#if (ENABLE_SPI1)
#include <SPI.h>
#endif

// SD Fat
#include "SdFat.h"

#include <STM32RTC.h>

// Memory device Access
#include "drivers/flash.h"
#include "drivers/eeprom.h"

#define SD_TASK_WAIT_DELAY_MS            (50)
#define SD_TASK_WAIT_OPERATION_DELAY_MS  (1)

#define SD_TASK_WAIT_REBOOT_MS           (2500)

#define SD_TASK_GENERIC_RETRY_DELAY_MS   (5000)
#define SD_TASK_GENERIC_RETRY            (3)

#define SD_FW_BLOCK_SIZE                 (256)

#define FILE_NAME_MAX_LENGHT             (128)

#include "debug_F.h"

#define errorExit(msg) errorHalt(F(msg))
#define initError(msg) initErrorHalt(F(msg))

typedef enum
{
  SD_STATE_CREATE,
  SD_STATE_INIT,
  SD_STATE_CHECK_SD,    
  SD_STATE_WAITING_EVENT,
  SD_UPLOAD_FIRMWARE_TO_FLASH,
  SD_STATE_ERROR
} SdState_t;

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
  cpp_freertos::Queue *dataFirmwarePutRequestQueue;
  cpp_freertos::Queue *dataFirmwarePutResponseQueue;
  Flash *flash;
  EEprom *eeprom;
} SdParam_t;

class SdTask : public cpp_freertos::Thread {

public:
  SdTask(const char *taskName, uint16_t stackSize, uint8_t priority, SdParam_t SdParam);

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

  SdState_t state;
  SdParam_t param;

  // Istance SD Card
  SdFat SD;

  STM32RTC &rtc = STM32RTC::getInstance();

  // Local flashPointer
  uint64_t sdFlashPtr;
  uint16_t sdFlashBlock;
};

#endif
#endif