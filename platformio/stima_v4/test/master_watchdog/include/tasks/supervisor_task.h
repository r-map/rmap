/**@file supervisor_task.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
<http://www.gnu.org/licenses/>.
**********************************************************************/

#ifndef _SUPERVISOR_TASK_H
#define _SUPERVISOR_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "stima_utility.h"
#include "str.h"

#include <STM32RTC.h>

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_master_hal.hpp"

#define SUPERVISOR_TASK_WAIT_DELAY_MS           (10)
#define SUPERVISOR_TASK_GENERIC_RETRY_DELAY_MS  (5000)
#define SUPERVISOR_TASK_GENERIC_RETRY           (3)

#define SUPERVISOR_TASK_NTP_SYNC_RETRY_DELAY_MS (5000)
#define SUPERVISOR_TASK_NTP_SYNC_RETRY          (3)

#if (ENABLE_I2C1 || ENABLE_I2C2)
#include <Wire.h>
#include "drivers/eeprom.h"
#endif

#include "register_class.hpp"

#include "debug_F.h"

typedef enum
{
  SUPERVISOR_STATE_CREATE,
  SUPERVISOR_STATE_INIT,
  SUPERVISOR_STATE_LOAD_CONFIGURATION,
  SUPERVISOR_STATE_WAITING_EVENT,
  SUPERVISOR_STATE_CONNECTION_OPERATION,
  SUPERVISOR_STATE_SAVE_CONFIGURATION,
  SUPERVISOR_STATE_REQUEST_CONNECTION,
  SUPERVISOR_STATE_CHECK_CONNECTION,
  SUPERVISOR_STATE_DO_NTP,
  SUPERVISOR_STATE_DO_HTTP,
  SUPERVISOR_STATE_DO_MQTT,
  SUPERVISOR_STATE_REQUEST_DISCONNECTION,
  SUPERVISOR_STATE_CHECK_DISCONNECTION,
  SUPERVISOR_STATE_END
} SupervisorState_t;

typedef enum
{
  CONNECTION_INIT,
  CONNECTION_CHECK,
  CONNECTION_CHECK_NTP,
  CONNECTION_CHECK_HTTP,
  CONNECTION_CHECK_MQTT,
  CONNECTION_END
} SupervisorConnection_t;

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::BinarySemaphore *registerAccessLock;
  cpp_freertos::Queue *systemMessageQueue;
  cpp_freertos::Queue *dataRmapGetRequestQueue;
  cpp_freertos::Queue *dataRmapGetResponseQueue;
  cpp_freertos::Queue *dataRmapPutQueue;
  cpp_freertos::Queue *connectionRequestQueue;
  cpp_freertos::Queue *connectionResponseQueue;
  cpp_freertos::Queue *dataFilePutRequestQueue;
  cpp_freertos::Queue *dataFilePutResponseQueue;
  EEprom *eeprom;
  EERegister *clRegister;
} SupervisorParam_t;

class SupervisorTask : public cpp_freertos::Thread {

public:
  SupervisorTask(const char *taskName, uint16_t stackSize, uint8_t priority, SupervisorParam_t SupervisorParam);

protected:
  virtual void Run();

private:

  #if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
  #endif
  void TaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  void printConfiguration();
  bool loadConfiguration();
  bool saveConfiguration(bool is_default);

  STM32RTC &rtc = STM32RTC::getInstance();

  SupervisorState_t state;
  SupervisorParam_t param;
};

#endif
