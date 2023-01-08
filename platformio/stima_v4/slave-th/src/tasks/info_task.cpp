/**
  ******************************************************************************
  * @file    info_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   info_task source file (Info && Logging Task for Module Slave)
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (C) 2022  Moreno Gasperini <m.gasperini@digiteco.it>
  * All rights reserved.</center></h2>
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

#define TRACE_LEVEL INFO_TASK_TRACE_LEVEL

#include "tasks/info_task.h"
#include "drivers/module_slave_hal.hpp"
#include <STM32RTC.h>

using namespace cpp_freertos;

InfoTask::InfoTask(const char *taskName, uint16_t stackSize, uint8_t priority, InfoParam_t infoParam) : Thread(taskName, stackSize, priority), param(infoParam)
{
  Start();
};

void InfoTask::Run() {
  STM32RTC &rtc = STM32RTC::getInstance();

  TRACE_INFO_F(F("Starting INFO TASK..."));

  while (true) {

    TRACE_INFO_F(F("%s: "), Thread::GetName().c_str());
    // Trace DateTime
    TRACE_INFO_F(F("%02d/%02d/%02d "), rtc.getDay(), rtc.getMonth(), rtc.getYear());
    TRACE_INFO_F(F("%02d:%02d:%02d.%03d\r\n"), rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getSubSeconds());

    #ifdef LOG_STACK_USAGE
    TRACE_INFO_F(F("Stack Free:\r\n"));
    TRACE_INFO_F(F("Accelerometer : %d\r\n"), param.system_status->task.accelerometer_stack);
    TRACE_INFO_F(F("Can Bus       : %d\r\n"), param.system_status->task.can_stack);
    TRACE_INFO_F(F("Elaborate data: %d\r\n"), param.system_status->task.elaborate_data_stack);
    TRACE_INFO_F(F("Supervisor    : %d\r\n"), param.system_status->task.supervisor_stack);
    TRACE_INFO_F(F("Module sensor : %d\r\n"), param.system_status->task.th_sensor_stack);
    #endif
 
    DelayUntil(Ticks::MsToTicks(2500));
  }
}