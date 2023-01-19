/**@file prova_task.cpp */

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

#define TRACE_LEVEL PROVA_TASK_TRACE_LEVEL

#include "tasks/prova_task.h"
#include "drivers/flash.h"
#include "drivers/module_master_hal.hpp"

using namespace cpp_freertos;

Flash testFlash(&hqspi);
static uint8_t write[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static uint8_t read[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

ProvaTask::ProvaTask(const char *taskName, uint16_t stackSize, uint8_t priority, ProvaParam_t provaParam) : Thread(taskName, stackSize, priority), param(provaParam)
{
  Start();
};

void ProvaTask::Run() {
  bool first = true;
  bool msgOk = false;
  Delay(500);
  TRACE_INFO_F(F("Running TEST Flash..."));
  while (true) {
  //   if(first) {
  //     first = false;
  //     if (testFlash.BSP_QSPI_Init() != Flash::QSPI_OK)
  //       Error_Handler();
  //     Flash::QSPI_StatusTypeDef sts = testFlash.BSP_QSPI_GetStatus();
  //     if (sts) Error_Handler();
  //     if (testFlash.BSP_QSPI_Erase_Block(0))
  //       Error_Handler();
  //     sts = testFlash.BSP_QSPI_GetStatus();
  //     if (testFlash.BSP_QSPI_Write(write, 0, sizeof(uint8_t) * 10))
  //       Error_Handler();
  //     if (testFlash.BSP_QSPI_Read(read, 0, sizeof(uint8_t) * 10))
  //       Error_Handler();
  //     // Working in MemoryMapped Mode at 0x9000000
  //     if (testFlash.BSP_QSPI_EnableMemoryMappedMode())
  //       Error_Handler();
  //     msgOk = true;
  //     first = false;
  //   } else {
  //     if(msgOk) {
  //       msgOk = false;
  //       TRACE_INFO_F(F("TEST Flash OK!!!!"));
  //     }
  //     else
         //TRACE_INFO_F(F("Prova %s\r\n"), "TASK");
        TRACE_INFO_F(F("*"));
//     }
    DelayUntil(Ticks::MsToTicks(1000));
  }
}