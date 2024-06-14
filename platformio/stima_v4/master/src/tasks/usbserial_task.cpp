/**
  ******************************************************************************
  * @file    usbserial_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   usbserial_task source file (USB CDC StimaV4)
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
  * <h2><center>All rights reserved.</center></h2>
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

/// @brief Construct a new Usb Serial Task:: Usb Serial Task object
/// @param taskName name of the task
/// @param stackSize size of the stack
/// @param priority priority of the task
/// @param usbSerialParam parameters for the task
UsbSerialTask::UsbSerialTask(const char *taskName, uint16_t stackSize, uint8_t priority, UsbSerialParam_t usbSerialParam) : Thread(taskName, stackSize, priority), param(usbSerialParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(USBSERIAL_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  state = USBSERIAL_STATE_INIT;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void UsbSerialTask::TaskMonitorStack()
{
  uint16_t stackUsage = (uint16_t)uxTaskGetStackHighWaterMark( NULL );
  if((stackUsage) && (stackUsage < param.system_status->tasks[LOCAL_TASK_ID].stack)) {
    param.systemStatusLock->Take();
    param.system_status->tasks[LOCAL_TASK_ID].stack = stackUsage;
    param.systemStatusLock->Give();
  }
}
#endif

/// @brief local watchDog and Sleep flag Task (optional)
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void UsbSerialTask::TaskWatchDog(uint32_t millis_standby)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Update WDT Signal (Direct or Long function Timered)
  if(millis_standby)  
  {
    // Check 1/2 Freq. controller ready to WDT only SET flag
    if((millis_standby) < WDT_CONTROLLER_MS / 2) {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
    } else {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::timer;
      // Add security milimal Freq to check
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog_ms = millis_standby + WDT_CONTROLLER_MS;
    }
  }
  else
    param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
  param.systemStatusLock->Give();
}

/// @brief local suspend flag and positor running state Task (optional)
/// @param state_position Sw_Position (Local STATE)
/// @param state_subposition Sw_SubPosition (Optional Local SUB_STATE Position Monitor)
/// @param state_operation operative mode flag status for this task
void UsbSerialTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Signal Task sleep/disabled mode from request (Auto SET WDT on Resume)
  if((param.system_status->tasks[LOCAL_TASK_ID].state == task_flag::suspended)&&
     (state_operation==task_flag::normal))
     param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
  param.system_status->tasks[LOCAL_TASK_ID].state = state_operation;
  param.system_status->tasks[LOCAL_TASK_ID].running_pos = state_position;
  param.system_status->tasks[LOCAL_TASK_ID].running_sub = state_subposition;
  param.systemStatusLock->Give();
}

/// @brief RUN Task
void UsbSerialTask::Run()
{
  bool message_traced = false;
  bool task_usb_sleep = true;

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  while (true)
  {

    switch (state)
    {
    case USBSERIAL_STATE_INIT:
      // Check USB SERIAL Device or Resynch after Error
      SerialUSB.begin(SERIAL_USB_BAUD_RATE);
      if (!SerialUSB) {
        TRACE_VERBOSE_F(F("USB Serial device ready -> USBSERIAL_STATE_WAITING_EVENT\r\n"));
        state = USBSERIAL_STATE_WAITING_EVENT;
        message_traced = false;
      } else {
        // Only one TRACE message...
        if(!message_traced) {
          // USB Serial Was NOT Ready... for System
          TRACE_VERBOSE_F(F("USB Serial device waiting to begin\r\n"));
          message_traced = true;
        }
      }
      break;

    case USBSERIAL_STATE_WAITING_EVENT:
      // Check enter USB Serial RX to take RPC Semaphore (release on END event OK/ERR)
      if(SerialUSB.available()) {
        // On Event WakeUp
        task_usb_sleep = false;
        if(param.rpcLock->Take(Ticks::MsToTicks(RPC_WAIT_DELAY_MS))) {
          is_event_rpc = true;
          while(is_event_rpc) {
            // Security lock task_flag for External Local TASK RPC (Need for risk of WDT Reset)
            param.system_status->tasks[LOCAL_TASK_ID].state = task_flag::suspended;
            param.streamRpc->parseStream(&is_event_rpc, &SerialUSB, JRPC_DEFAULT_TIMEOUT_MS, RPC_TYPE_SERIAL);
            param.system_status->tasks[LOCAL_TASK_ID].state = task_flag::normal;
            param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
            // Non blocking task
            TaskWatchDog(TASK_WAIT_REALTIME_DELAY_MS);
            Delay(Ticks::MsToTicks(TASK_WAIT_REALTIME_DELAY_MS));
          }
          param.rpcLock->Give();
        }
      }
      break;

    case USBSERIAL_STATE_ERROR:
      // Gest Error... Resynch SD
      TRACE_VERBOSE_F(F("USBSERIAL_STATE_ERROR -> USBSERIAL_STATE_INIT\r\n"));
      state = USBSERIAL_STATE_INIT;
      break;
    }

    #if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
    #endif

    // One step base non blocking switch
    // time to pause depending to sleep mode (no sequence data event)
    if(task_usb_sleep) {
      TaskWatchDog(USBSERIAL_TASK_SLEEP_DELAY_MS);
      Delay(Ticks::MsToTicks(USBSERIAL_TASK_SLEEP_DELAY_MS));
    } else {
      // Mode Task small Delay but not sleep Mode (After event... if Waiting ather command)
      TaskWatchDog(USBSERIAL_TASK_WAIT_DELAY_MS);
      Delay(Ticks::MsToTicks(USBSERIAL_TASK_WAIT_DELAY_MS));
      // Next event can activate sleep mode
      task_usb_sleep = true;
    }

  }
}

#endif
