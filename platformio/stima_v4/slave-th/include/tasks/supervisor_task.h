/**@file supervisor_task.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>

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

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_slave_hal.hpp"
#include "SensorDriver.h"

#define SUPERVISOR_TASK_WAIT_DELAY_MS           (10)
#define SUPERVISOR_TASK_GENERIC_RETRY_DELAY_MS  (5000)
#define SUPERVISOR_TASK_GENERIC_RETRY           (3)

#if (ENABLE_I2C1 || ENABLE_I2C2)
#include <Wire.h>
#include "drivers/eeprom.h"
#endif

#include "debug_F.h"

typedef enum
{
  SUPERVISOR_STATE_INIT,
  SUPERVISOR_STATE_CHECK_OPERATION,
  SUPERVISOR_STATE_LOAD_CONFIGURATION,
  SUPERVISOR_STATE_SAVE_CONFIGURATION,
  // SUPERVISOR_STATE_REQUEST_CONNECTION,
  // SUPERVISOR_STATE_CHECK_CONNECTION,
  // SUPERVISOR_STATE_CHECK_CONNECTION_TYPE,
  // SUPERVISOR_STATE_DO_NTP,
  // SUPERVISOR_STATE_DO_HTTP,
  // SUPERVISOR_STATE_DO_MQTT,
  // SUPERVISOR_STATE_REQUEST_DISCONNECTION,
  SUPERVISOR_STATE_END
} SupervisorState_t;

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *wireLock;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::Queue *systemRequestQueue;
  cpp_freertos::Queue *systemResponseQueue;
  TwoWire *wire;
} SupervisorParam_t;

class SupervisorTask : public cpp_freertos::Thread {

public:
  SupervisorTask(const char *taskName, uint16_t stackSize, uint8_t priority, SupervisorParam_t SupervisorParam);

protected:
  virtual void Run();

private:
  SupervisorState_t state;
  SupervisorParam_t param;
  EEprom eeprom;

  void printConfiguration(configuration_t *configuration, BinarySemaphore *lock);
  bool loadConfiguration(configuration_t *configuration, BinarySemaphore *lock);
  bool saveConfiguration(configuration_t *configuration, BinarySemaphore *lock, bool is_default);
};

#endif
