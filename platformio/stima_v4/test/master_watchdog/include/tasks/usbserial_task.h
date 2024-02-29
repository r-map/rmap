/**
 ******************************************************************************
 * @file    usbserial_task.h
 * @author  Moreno Gasperini <m.gasperini@digiteco.it>
 * @brief   usbserial_task header file (USB SERIAL CDC StimaV4)
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

#ifndef _USBSERIAL_TASK_H
#define _USBSERIAL_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "stima_utility.h"
#include "str.h"

#if (ENABLE_USBSERIAL)

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_master_hal.hpp"

#include <STM32RTC.h>
#include <arduinoJsonRPC.h>

// Memory device Access
#include "drivers/flash.h"
#include "drivers/eeprom.h"

#define USBSERIAL_TASK_WAIT_DELAY_MS            (50)

#define USBSERIAL_TASK_WAIT_REBOOT_MS           (2500)

#define USBSERIAL_TASK_GENERIC_RETRY_DELAY_MS   (5000)
#define USBSERIAL_TASK_GENERIC_RETRY            (3)

#include "debug_F.h"

typedef enum
{
  USBSERIAL_STATE_CREATE,
  USBSERIAL_STATE_INIT,
  USBSERIAL_STATE_WAITING_EVENT,
  USBSERIAL_STATE_ERROR
} UsbSerialState_t;

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *qspiLock;
  cpp_freertos::BinarySemaphore *rtcLock;
  cpp_freertos::BinarySemaphore *rpcLock;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::Queue *systemMessageQueue;
  Flash *flash;
  EEprom *eeprom;
  JsonRPC *streamRpc;
} UsbSerialParam_t;

class UsbSerialTask : public cpp_freertos::Thread {

public:
  UsbSerialTask(const char *taskName, uint16_t stackSize, uint8_t priority, UsbSerialParam_t UsbSerialParam);

protected:
  virtual void Run();

private:

  #if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
  #endif
  void TaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  UsbSerialState_t state;
  UsbSerialParam_t param;

  STM32RTC &rtc = STM32RTC::getInstance();

  bool is_event_rpc;
};

#endif
#endif