/**
  ******************************************************************************
  * @file    mppt_sensor_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Mppt controller source file
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

#define TRACE_LEVEL     MPPT_SENSOR_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   SENSOR_TASK_ID

#include "tasks/mppt_sensor_task.h"

#if (MODULE_TYPE == STIMA_MODULE_TYPE_POWER_MPPT)

using namespace cpp_freertos;

MpptSensorTask::MpptSensorTask(const char *taskName, uint16_t stackSize, uint8_t priority, MpptSensorParam_t mpptSensorParam) : Thread(taskName, stackSize, priority), param(mpptSensorParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(SENSOR_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  state = SENSOR_STATE_WAIT_CFG;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void MpptSensorTask::TaskMonitorStack()
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
void MpptSensorTask::TaskWatchDog(uint32_t millis_standby)
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
void MpptSensorTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
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

void MpptSensorTask::Run() {
  rmapdata_t values_readed_from_sensor[VALUES_TO_READ_FROM_SENSOR_COUNT];
  elaborate_data_t edata;
  uint32_t delay_ms;
  static bool is_test;
  // Request response for system queue Task controlled...
  system_message_t system_message;
  
  uint8_t error_count;

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  while (true)
  {

    switch (state)
    {
    case SENSOR_STATE_WAIT_CFG:
      // check if configuration is done loaded
      if (param.system_status->flags.is_cfg_loaded)
      {
        TRACE_VERBOSE_F(F("WAIT -> INIT\r\n"));
        state = SENSOR_STATE_INIT;
      }
      // other
      else
      {
        // Local WatchDog update;
        TaskWatchDog(MPPT_TASK_WAIT_DELAY_MS);
        Delay(Ticks::MsToTicks(MPPT_TASK_WAIT_DELAY_MS));
      }
      // do something else with non-blocking wait ....
      break;

    case SENSOR_STATE_INIT:
      TRACE_INFO_F(F("Initializing sensors...\r\n"));
      state = SENSOR_STATE_READ;
      break;

      case SENSOR_STATE_READ:

        // Read Data from LTC_4015
        edata.value = param.mpptIC->get_P_CHG();
        edata.index = POWER_BATTERY_CHARGE_INDEX;
        param.elaborataDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));

        edata.value = param.mpptIC->get_V_BAT() * POWER_BATTERY_VOLTAGE_MULT;
        edata.index = POWER_BATTERY_VOLTAGE_INDEX;
        param.elaborataDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));

        edata.value = param.mpptIC->get_I_BAT() * POWER_INPUT_CURRENT_MULT;
        edata.index = POWER_BATTERY_CURRENT_INDEX;
        param.elaborataDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));

        edata.value = param.mpptIC->get_V_IN() * POWER_INPUT_VOLTAGE_MULT;
        edata.index = POWER_INPUT_VOLTAGE_INDEX;
        param.elaborataDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));

        edata.value = param.mpptIC->get_I_IN() * POWER_INPUT_CURRENT_MULT;
        edata.index = POWER_INPUT_CURRENT_INDEX;
        param.elaborataDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));

        state = SENSOR_STATE_END;
        break;

      case SENSOR_STATE_END:

        #if (ENABLE_STACK_USAGE)
        TaskMonitorStack();
        #endif

        // Local TaskWatchDog update and Sleep Activate before Next Read
        TaskWatchDog(param.configuration->sensor_acquisition_delay_ms);
        TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
        DelayUntil(Ticks::MsToTicks(param.configuration->sensor_acquisition_delay_ms));
        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

        state = SENSOR_STATE_READ;
        break;
    }
  }
}

#endif