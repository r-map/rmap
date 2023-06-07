/**@file main.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>

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

#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"
#include "Arduino.h"

// Setup Serial 2 Istance (Not use internal weak as GSM Local Driver)
HardwareSerial Serial2(PD_6, PD_5);

void setup() {
 
  // Initializing basic hardware's configuration, variables and function
  SetupSystemPeripheral();
  init_debug(SERIAL_DEBUG_BAUD_RATE);

  TRACE_INFO_F(F("Initialization HW Base done\r\n\r\n"));

  // ***************************************************************
  //           Setup parameter for Task and local Class
  // ***************************************************************

#if (ENABLE_USBSERIAL)
 // TASK SUPERVISOR PARAM CONFIG
  static UsbSerialParam_t usbSerialParam = {0};
#endif

  // *****************************************************************************
  // Startup Task, Supervisor as first for Loading parameter generic configuration
  // *****************************************************************************

#if (ENABLE_USBSERIAL)
  static UsbSerialTask usbSerial_task("UsbSerialTask", 1100, OS_TASK_PRIORITY_01, usbSerialParam);
#endif

  // Startup Schedulher
  Thread::StartScheduler();
}

// FreeRTOS idleHook callBack to loop
void loop() {
}