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
#include "STM32FreeRTOS.h"
#include "thread.hpp"
#include "ticks.hpp"
#include "drivers/eeprom.h"
#include "debug.h"
#include "Wire.h"

typedef enum
{
  SUPERVISOR_STATE_INIT,
  SUPERVISOR_STATE_CHECK_OPERATION,
  SUPERVISOR_STATE_LOAD_CONFIGURATION,
  SUPERVISOR_STATE_SAVE_CONFIGURATION,
  SUPERVISOR_STATE_END
} SupervisorState_t;

typedef struct {
  configuration_t *configuration;
  BinarySemaphore *wireLock;
  BinarySemaphore *configurationLock;
  TwoWire *wire;
} SupervisorParam_t;

class SupervisorTask : public cpp_freertos::Thread {

public:
  SupervisorTask(const char *taskName, uint16_t stackSize, uint8_t priority, SupervisorParam_t SupervisorParam);

protected:
  virtual void Run();

private:
  char taskName[configMAX_TASK_NAME_LEN];
  uint16_t stackSize;
  uint8_t priority;
  SupervisorState_t state;
  SupervisorParam_t SupervisorParam;
  EEprom eeprom;

  void PrintConfiguration(configuration_t *configuration, BinarySemaphore *lock);
  void LoadConfiguration(configuration_t *configuration, BinarySemaphore *lock);
  void SaveConfiguration(configuration_t *configuration, BinarySemaphore *lock, bool is_default);
};

#endif
