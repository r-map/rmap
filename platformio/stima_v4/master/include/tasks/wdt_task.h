/**
  ******************************************************************************
  * @file    wdt_task.h
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   wdt_task header file (Wdt && Logging Task for Module Slave)
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

#ifndef _WDT_TASK_H
#define _WDT_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "str.h"

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_master_hal.hpp"
#include "drivers/eeprom.h"

#include <STM32RTC.h>
#include <IWatchdog.h>

#include "debug_F.h"

// Main TASK Switch Delay
#define WDT_TASK_WAIT_DELAY_MS      (WDT_CONTROLLER_MS)

using namespace cpp_freertos;

typedef struct {
  system_status_t *system_status;
  cpp_freertos::Queue *dataLogPutQueue;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::BinarySemaphore *wireLock;
  cpp_freertos::BinarySemaphore *rtcLock;
  TwoWire *wire;
} WdtParam_t;

class WdtTask : public cpp_freertos::Thread {

public:
  WdtTask(const char *taskName, uint16_t stackSize, uint8_t priority, WdtParam_t wdtParam);

protected:
  virtual void Run();

private:
  WdtParam_t param;

  EEprom memEprom;
};

#endif
