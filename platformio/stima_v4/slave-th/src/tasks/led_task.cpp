/**@file led_task.cpp */

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

#define TRACE_LEVEL LED_TASK_TRACE_LEVEL

#include "tasks/led_task.h"

using namespace cpp_freertos;

LedTask::LedTask(const char *taskName, uint16_t stackSize, uint8_t priority, LedParam_t ledParam) : Thread(taskName, stackSize, priority), LedParam(ledParam) {
  Start();
};

void LedTask::Run() {
  pinMode(LedParam.led, OUTPUT);
  while (true) {
    digitalWrite(LedParam.led, HIGH);
    DelayUntil(Ticks::MsToTicks(LedParam.onDelayMs));
    digitalWrite(LedParam.led, LOW);
    DelayUntil(Ticks::MsToTicks(LedParam.offDelayMs));
  }
}
