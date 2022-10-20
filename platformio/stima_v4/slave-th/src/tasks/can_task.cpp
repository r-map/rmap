/**@file can_task.cpp */

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

#define TRACE_LEVEL CAN_TASK_TRACE_LEVEL

#include "tasks/can_task.h"

using namespace cpp_freertos;

CanTask::CanTask(const char *taskName, uint16_t stackSize, uint8_t priority, CanParam_t canParam) : Thread(taskName, stackSize, priority), param(canParam) {
  state = INIT;
  Start();
};

void CanTask::Run() {
  request_data_t request_data;
  report_t report;

  while (true) {
    TRACE_INFO(F("CAN TASK\r\n"));
    request_data.is_init = true;
    request_data.report_time_s = 900;
    request_data.observation_time_s = 90;
    param.requestDataQueue->Enqueue(&request_data, 0);
    if (param.reportDataQueue->Dequeue(&report, portMAX_DELAY)) {
      TRACE_INFO(F("--> CAN temperature report\t%d\t%d\t%d\t%d\t%d\t%d\r\n"), (int32_t) report.temperature.sample, (int32_t) report.temperature.ist, (int32_t) report.temperature.min, (int32_t) report.temperature.avg, (int32_t) report.temperature.max, (int32_t) report.temperature.quality);
      TRACE_INFO(F("--> CAN humidity report\t%d\t%d\t%d\t%d\t%d\t%d\r\n"), (int32_t) report.humidity.sample, (int32_t) report.humidity.ist, (int32_t) report.humidity.min, (int32_t) report.humidity.avg, (int32_t) report.humidity.max, (int32_t) report.humidity.quality);
    }
    DelayUntil(Ticks::MsToTicks(5000));
  }
}
