/**
  ******************************************************************************
  * @file    supervisor_task.h
  * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Supervisor module header file
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


#ifndef _SUPERVISOR_TASK_H
#define _SUPERVISOR_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "stima_utility.h"
#include "str.h"

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_slave_hal.hpp"

// Register EEprom
#include "register_class.hpp"

// Main TASK Switch Delay
#define SUPERVISOR_TASK_WAIT_DELAY_MS     (20)
#define SUPERVISOR_TASK_SLEEP_DELAY_MS    (850)

#define SUPERVISOR_TASK_GENERIC_RETRY_DELAY_MS  (5000)
#define SUPERVISOR_TASK_GENERIC_RETRY           (3)

#include "debug_F.h"

typedef enum
{
  SUPERVISOR_STATE_CREATE,
  SUPERVISOR_STATE_INIT,
  SUPERVISOR_STATE_CHECK_OPERATION,
  SUPERVISOR_STATE_END
} SupervisorState_t;

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::BinarySemaphore *registerAccessLock;
  cpp_freertos::Queue *systemMessageQueue;
  EERegister *clRegister;
  bool is_initialization_request;
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
  void loadConfiguration();
  void saveConfiguration(bool is_default);

  SupervisorState_t state;
  SupervisorParam_t param;
};

#endif
