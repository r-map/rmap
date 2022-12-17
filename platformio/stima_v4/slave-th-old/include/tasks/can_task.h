/**@file can_task.h */

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

#ifndef _CAN_TASK_H
#define _CAN_TASK_H

#include "config.h"

#include "STM32FreeRTOS.h"
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"

#include "report.h"
#include "debug.h"

// Classe Canard
#include "canardClass_th.hpp"
// Libcanard
#include "register.hpp"
#include <canard.h>
#include "bxcan.h"
// Namespace UAVCAN
#include <uavcan/node/Heartbeat_1_0.h>
#include <uavcan/node/GetInfo_1_0.h>
#include <uavcan/node/ExecuteCommand_1_1.h>
#include <uavcan/node/port/List_0_1.h>
#include <uavcan/_register/Access_1_0.h>
#include <uavcan/_register/List_1_0.h>
#include <uavcan/file/Read_1_1.h>
#include <uavcan/time/Synchronization_1_0.h>
#include <uavcan/pnp/NodeIDAllocationData_1_0.h>
// Namespace RMAP
#include <rmap/_module/TH_1_0.h>
#include <rmap/service/_module/TH_1_0.h>
// Standard Library
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
// Configurazione modulo, definizioni ed utility generiche
#include "module_config.hpp"

using namespace cpp_freertos;

typedef struct {
  Queue *requestDataQueue;
  Queue *reportDataQueue;
} CanParam_t;

class CanTask : public cpp_freertos::Thread {
  typedef enum {
    INIT,
    SETUP,
    STANDBY,
    SLEEP
  } State_t;

public:
  CanTask(const char *taskName, uint16_t stackSize, uint8_t priority, CanParam_t canParam);

protected:
  virtual void Run();

private:
  char taskName[configMAX_TASK_NAME_LEN];
  uint16_t stackSize;
  uint8_t priority;
  CanParam_t param;
  State_t state;
};

#endif
