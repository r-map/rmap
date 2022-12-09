/**@file led_task.h */

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

#ifndef _LED_TASK_H
#define _LED_TASK_H

#include "hardware_config.h"
#include "Arduino.h"
#include "STM32FreeRTOS.h"
#include "thread.hpp"
#include "ticks.hpp"

typedef struct {
  uint8_t led;
  uint16_t onDelayMs;
  uint16_t offDelayMs;
} LedParam_t;

class LedTask : public cpp_freertos::Thread {

public:
  LedTask(const char *taskName, uint16_t stackSize, uint8_t priority, LedParam_t ledParam);

protected:
  virtual void Run();

private:
  char taskName[configMAX_TASK_NAME_LEN];
  uint16_t stackSize;
  uint8_t priority;
  LedParam_t LedParam;
};

#endif
