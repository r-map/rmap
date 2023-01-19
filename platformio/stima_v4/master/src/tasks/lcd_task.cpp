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

#define TRACE_LEVEL LCD_TASK_TRACE_LEVEL

#include "tasks/lcd_task.h"

using namespace cpp_freertos;

LCDTask::LCDTask(const char *taskName, uint16_t stackSize, uint8_t priority, LCDParam_t LCDParam) : Thread(taskName, stackSize, priority), param(LCDParam)
{
  // Start WDT controller and RunState Flags
  #if (ENABLE_WDT)
  WatchDog(param.system_status, param.systemStatusLock, WDT_TIMEOUT_BASE_US / 1000, false);
  RunState(param.system_status, param.systemStatusLock, RUNNING_START, false);
  #endif

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
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
void LCDTask::monitorStack(system_status_t *status, BinarySemaphore *lock)
{
  u_int16_t stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark( NULL );
  if((stackUsage) && (stackUsage < status->tasks[LCD_TASK_ID].stack)) {
    if(lock != NULL) lock->Take();
    status->tasks[LCD_TASK_ID].stack = stackUsage;
    if(lock != NULL) lock->Give();
  }
}
#endif

#if (ENABLE_WDT)
/// @brief local watchDog and Sleep flag Task (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void LCDTask::WatchDog(system_status_t *status, BinarySemaphore *lock, uint16_t millis_standby, bool is_sleep)
{
  // Local WatchDog update
  if(lock != NULL) lock->Take();
  // Signal Task sleep/disabled mode from request
  status->tasks[LCD_TASK_ID].is_sleep = is_sleep;
  if((millis_standby) < (WDT_TIMEOUT_BASE_US / 1000))
    status->tasks[LCD_TASK_ID].watch_dog = wdt_flag::set;
  else
    status->tasks[LCD_TASK_ID].watch_dog = wdt_flag::rest;
  if(lock != NULL) lock->Give();
}

/// @brief local suspend flag and positor running state Task (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param state_position Sw_Poition (STandard 1 RUNNING_START, 2 RUNNING_EXEC, XX User define SW POS fo Logging)
/// @param is_suspend TRUE if task enter suspending mode (Disable from WDT Controller)
void LCDTask::RunState(system_status_t *status, BinarySemaphore *lock, uint8_t state_position, bool is_suspend)
{
  // Local WatchDog update
  if(lock != NULL) lock->Take();
  // Signal Task sleep/disabled mode from request
  status->tasks[LCD_TASK_ID].is_suspend = is_suspend;
  status->tasks[LCD_TASK_ID].running_pos = state_position;
  if(lock != NULL) lock->Give();
}
#endif

void LCDTask::Run()
{

  // Starting Task and first WDT (if required and enabled. Time < than WDT_TIMEOUT_BASE_US)
  #if (ENABLE_WDT)
  RunState(param.system_status, param.systemStatusLock, RUNNING_EXEC, false);
  #endif
  #if (ENABLE_STACK_USAGE)
  monitorStack(param.system_status, param.systemStatusLock);
  #endif

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
      Delay(Ticks::MsToTicks(LCD_TASK_PRINT_DELAY_MS));
      break;
    
    case LCD_STATE_END:
      // Display off...
      // Waiting for Resume...
      // TRACE_VERBOSE_F(F("LCD_STATE_END -> LCD_STATE_CHECK_OPERATION\r\n"));
      break;
    }
  }
}
