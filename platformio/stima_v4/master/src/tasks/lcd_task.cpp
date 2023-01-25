/**
 ******************************************************************************
 * @file    lcd_task.cpp
 * @author  Moreno Gasperini <m.gasperini@digiteco.it>
 * @brief   LCD Task based u8gl library
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

#define TRACE_LEVEL     LCD_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   LCD_TASK_ID

#include "tasks/lcd_task.h"

using namespace cpp_freertos;

LCDTask::LCDTask(const char *taskName, uint16_t stackSize, uint8_t priority, LCDParam_t LCDParam) : Thread(taskName, stackSize, priority), param(LCDParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(LCD_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  // Enable Encoder && Display
  digitalWrite(PIN_ENCODER_EN5, HIGH);
  digitalWrite(PIN_DSP_POWER, HIGH);

  state = LCD_STATE_INIT;
  // Create LCD Access with Param Task
  u8g2 = U8G2_SH1108_128X160_F_FREERTOS_HW_I2C(U8G2_R1, param.wire, param.wireLock);
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void LCDTask::TaskMonitorStack()
{
  u_int16_t stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark( NULL );
  if((stackUsage) && (stackUsage < param.system_status->tasks[LOCAL_TASK_ID].stack)) {
    param.systemStatusLock->Take();
    param.system_status->tasks[LOCAL_TASK_ID].stack = stackUsage;
    param.systemStatusLock->Give();
  }
}
#endif

/// @brief local watchDog and Sleep flag Task (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void LCDTask::TaskWatchDog(uint32_t millis_standby)
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
void LCDTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Signal Task sleep/disabled mode from request (Auto SET WDT on Resume)
  if((param.system_status->tasks[LOCAL_TASK_ID].state == task_flag::suspended)&&
     (state_operation==task_flag::normal))
     param.system_status->tasks->watch_dog = wdt_flag::set;
  param.system_status->tasks[LOCAL_TASK_ID].state = state_operation;
  param.system_status->tasks[LOCAL_TASK_ID].running_pos = state_position;
  param.system_status->tasks[LOCAL_TASK_ID].running_sub = state_subposition;
  param.systemStatusLock->Give();
}

void LCDTask::Run()
{

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  // Loop Task
  while (true)
  {
    switch (state)
    {
    case LCD_STATE_INIT:
      TRACE_VERBOSE_F(F("LCD_STATE_INIT -> LCD_STATE_PRINT\r\n"));
      u8g2.begin();
      // Test Display Output
      u8g2.clearBuffer();					// clear the internal memory
      u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
      u8g2.drawStr(0,10,"Hello World!");	// write something to the internal memory
      u8g2.sendBuffer();					// transfer internal memory to the display
      state = LCD_STATE_PRINT;
      break;

    case LCD_STATE_PRINT:
      // check if display is on and print every LCD_TASK_PRINT_DELAY_MS some variables in system status
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
      Suspend();
      break;
    
    case LCD_STATE_END:
      // Display off...
      // Waiting for Resume...
      // TRACE_VERBOSE_F(F("LCD_STATE_END -> LCD_STATE_CHECK_OPERATION\r\n"));
      break;
    }
  }
}
