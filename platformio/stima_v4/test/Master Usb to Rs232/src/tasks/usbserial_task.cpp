/**
  ******************************************************************************
  * @file    usbserial_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   usbserial_task source file (USB CDC StimaV4)
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

#define TRACE_LEVEL     USBSERIAL_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   USBSERIAL_TASK_ID

#include "tasks/usbserial_task.h"

#if (ENABLE_USBSERIAL)

using namespace cpp_freertos;

UsbSerialTask::UsbSerialTask(const char *taskName, uint16_t stackSize, uint8_t priority, UsbSerialParam_t usbSerialParam) : Thread(taskName, stackSize, priority), param(usbSerialParam)
{
  Serial2.begin(SERIAL_DEBUG_BAUD_RATE);
  SerialUSB.begin(SERIAL_DEBUG_BAUD_RATE);
  
  state = USBSERIAL_STATE_INIT;
  Start();
};

void UsbSerialTask::Run()
{
  while (true)
  {
    // Check enter USB Serial RX to take RPC Semaphore (release on END event OK/ERR)
    if(SerialUSB.available()) {
      char chOut = SerialUSB.read();
      Serial.print("USB IN: ");
      Serial2.print(chOut);
      Serial.println((int)chOut, 16);
    }
    // Check enter USB Serial RX to take RPC Semaphore (release on END event OK/ERR)
    if(Serial2.available()) {
      char chIn = Serial2.read();
      Serial.print("RS232 IN: ");
      SerialUSB.print(chIn);
      Serial.println((int)chIn, 16);
    }
  }
  Delay(5);
}

#endif