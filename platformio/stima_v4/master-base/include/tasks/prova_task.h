/**@file prova_task.h */

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

#ifndef _PROVA_TASK_H
#define _PROVA_TASK_H

#include "debug_config.h"
#include "STM32FreeRTOS.h"
#include "thread.hpp"
#include "ticks.hpp"
#include "debug.h"

typedef struct {
  int x;
} ProvaParam_t;

class ProvaTask : public cpp_freertos::Thread {

public:
  ProvaTask(const char *taskName, uint16_t stackSize, uint8_t priority, ProvaParam_t provaParam);

protected:
  virtual void Run();

private:
  char taskName[configMAX_TASK_NAME_LEN];
  uint16_t stackSize;
  uint8_t priority;
  ProvaParam_t ProvaParam;
};

#endif
