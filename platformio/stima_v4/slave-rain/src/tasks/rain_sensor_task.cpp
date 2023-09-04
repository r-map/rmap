/**
  ******************************************************************************
  * @file    rain_sensor_task.cpp
  * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Rain sensor source file
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

#define TRACE_LEVEL     RAIN_SENSOR_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   SENSOR_TASK_ID

#include "tasks/rain_sensor_task.h"

#if (MODULE_TYPE == STIMA_MODULE_TYPE_RAIN)

using namespace cpp_freertos;

RainSensorTask::RainSensorTask(const char *taskName, uint16_t stackSize, uint8_t priority, RainSensorParam_t rainSensorParam) : Thread(taskName, stackSize, priority), param(rainSensorParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(SENSOR_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  localRainQueue = param.rainQueue;

  // Initialize ISR Event to Lock mode (Wait Init sensor to start)
  is_isr_event_running = true;

  TRACE_INFO_F(F("Initializing rain event sensor handler...\r\n"));
  pinMode(TIPPING_BUCKET_PIN, INPUT_PULLUP);
  attachInterrupt(TIPPING_BUCKET_PIN, ISR_tipping_bucket, TIPPING_EVENT_VALUE);
  #if (USE_TIPPING_BUCKET_REDUNDANT)
  pinMode(TIPPING_BUCKET_PIN_REDUNDANT, INPUT_PULLUP);
  attachInterrupt(TIPPING_BUCKET_PIN_REDUNDANT, ISR_tipping_bucket, TIPPING_EVENT_VALUE);
  #endif

  state = SENSOR_STATE_WAIT_CFG;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void RainSensorTask::TaskMonitorStack()
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
void RainSensorTask::TaskWatchDog(uint32_t millis_standby)
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
void RainSensorTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
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

void RainSensorTask::Run() {
  rmapdata_t values_readed_from_sensor[VALUES_TO_READ_FROM_SENSOR_COUNT];
  elaborate_data_t edata;

  // Request response for system queue Task controlled...
  system_message_t system_message;

  uint8_t flag_event;

  bool bMainError, bRedundantError, bTippingError;
  bool bEventMain, bEventRedundant;

  uint16_t error_count = 0;

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
        rain.tips_count = 0;
        rain.rain = 0;
        rain.tips_full = 0;
        rain.rain_full = 0;
        rain.tips_scroll = 0;
        rain.rain_scroll = 0;
        bMainError = false;
        bRedundantError = false;
        bTippingError = false;
        bEventMain = false;
        bEventRedundant = false;
        TRACE_VERBOSE_F(F("Sensor: WAIT -> INIT\r\n"));
        state = SENSOR_STATE_INIT;
      }
      else
      {
        // Local WatchDog update while config loaded
        TaskWatchDog(RAIN_TASK_WAIT_DELAY_MS);
        Delay(Ticks::MsToTicks(RAIN_TASK_WAIT_DELAY_MS));
      }
      // do something else with non-blocking wait ....
      break;

    case SENSOR_STATE_INIT:
      // Enter in suspended mode (wait queue from ISR Event...)
      // Reset is_isr_event_running. Now Event interrupt ISR are checked (main and/or redundant)
      is_isr_event_running = false;
      TaskWatchDog(RAIN_TASK_WAIT_DELAY_MS);
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
      // Waiting interrupt or External Reset (Suspend task)
      localRainQueue->Dequeue(&flag_event);
      // Is RESET RAIN? (param enqueued...)
      if(flag_event == RAIN_TIPS_RESET) {
        // Reset signal event (if error persistent, event error restored)
        bMainError = false;
        bRedundantError = false;
        bTippingError = false;
        error_count = 0;
        // Reset standard counter and exit
        rain.rain = 0;
        rain.rain_full = 0;
        rain.tips_count = 0;
        rain.tips_full = 0;
        memset(&rain, 0, sizeof(rain));
        break;
      }
      // Is RESET SCROLL? (false, is request Reset Counter value)
      if(flag_event == RAIN_SCROLL_RESET) {
        // Reset only scroll counter and exit
        rain.rain_scroll = 0;
        rain.tips_scroll = 0;
        break;
      }
      // ... and HERE is EVENT RAIN... check if OK!!!
      // ********************************************
      // Starting Event Rain Counter check operartion
      // ********************************************
      TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
      TRACE_INFO_F(F("Sensor: checking event...\r\n"));

      #if(USE_TIPPING_CONTROL)
      TaskWatchDog(param.configuration->sensors.tipping_bucket_time_ms / 2);
      Delay(Ticks::MsToTicks(param.configuration->sensors.tipping_bucket_time_ms / 2));
      state = SENSOR_STATE_CHECK_SPIKE;
      #else
      TaskWatchDog(param.configuration->sensors.tipping_bucket_time_ms / 2);
      Delay(Ticks::MsToTicks(param.configuration->sensors.tipping_bucket_time_ms / 2));
      state = SENSOR_STATE_READ;
      #endif
      break;

    case SENSOR_STATE_CHECK_SPIKE:
      // re-read pin status to filter spikes before increment rain tips
      // Read time now is tipping_bucket_time_ms / 2 -> from event start
      #if (!USE_TIPPING_BUCKET_REDUNDANT)
      // Standard control with only one PIN
      if (digitalRead(TIPPING_BUCKET_PIN) != TIPPING_EVENT_VALUE)
      {
        // Checking signal CLOGGED_UP (on Event or Reset... remote calling)
        TRACE_INFO_F(F("Sensor: Skip spike (to fast)\r\n"));
        error_count++;
        bTippingError = true;
        state = SENSOR_STATE_SPIKE;
        break;
      }
      #else
      // Read Pin standard and redundant (all reed is not event... Error. If one is event, continue)
      // Standard control with only one PIN
      bEventMain = (digitalRead(TIPPING_BUCKET_PIN) == TIPPING_EVENT_VALUE);
      bEventRedundant = (digitalRead(TIPPING_BUCKET_PIN_REDUNDANT) == TIPPING_EVENT_VALUE);
      // All sensor readin gone (Sensor are synch, event too fast)
      if ((!bEventMain)&&(!bEventRedundant))
      {
        TRACE_INFO_F(F("Sensor: Skip spike (to fast)\r\n"));
        error_count++;
        bTippingError = true;
        state = SENSOR_STATE_SPIKE;
        break;
      }
      // Here event Main and/or Redundat is Ok. Check if difference occurs
      if ((bEventMain)&&(!bEventRedundant))
      {
        // Error sensor redundant and signal to master
        bRedundantError = true;
        TRACE_INFO_F(F("Sensor: Error reading redundant tipping (no event)\r\n"));
      }
      if ((!bEventMain)&&(bEventRedundant))
      {
        // Error sensor redundant and signal to master
        bMainError = true;
        TRACE_INFO_F(F("Sensor: Error reading main tipping (no event)\r\n"));
      }
      #endif
      TRACE_VERBOSE_F(F("Sensor: checking end event...\r\n"));
      TaskWatchDog(param.configuration->sensors.tipping_bucket_time_ms);
      Delay(Ticks::MsToTicks(param.configuration->sensors.tipping_bucket_time_ms));
      state = SENSOR_STATE_READ;
      break;

    case SENSOR_STATE_READ:

      #if(USE_TIPPING_CONTROL)
      // re-read pin status to be inverted from event before increment rain tips
      // Read time now is tipping_bucket_time_ms * 1.5 -> from event start
      #if (!USE_TIPPING_BUCKET_REDUNDANT)
      // Standard control with only one PIN
      if (digitalRead(TIPPING_BUCKET_PIN) != TIPPING_EVENT_VALUE)
      #else
      // Read Pin standard and redundant (all reed must to be !event... Error if one is event)
      if ((digitalRead(TIPPING_BUCKET_PIN) != TIPPING_EVENT_VALUE) ||
          (digitalRead(TIPPING_BUCKET_PIN_REDUNDANT) != TIPPING_EVENT_VALUE))
      #endif
      {
        // Always add full data
        rain.tips_full++;
        rain.rain_full = rain.tips_full * param.configuration->sensors.rain_for_tip;
        // Add this Value only if system is not in maintenance mode
        if(!param.system_status->flags.is_maintenance) {
          rain.tips_count++;
          rain.tips_scroll++;
          rain.rain = rain.tips_count * param.configuration->sensors.rain_for_tip;
          rain.rain_scroll = rain.tips_scroll * param.configuration->sensors.rain_for_tip;
        }
        TRACE_INFO_F(F("Sensor: Rain tips (count, full)\t%d\t%d,\r\n"), rain.tips_count, rain.tips_full);
      }
      else
      {
        TRACE_INFO_F(F("Sensor: Skip spike (to late)\r\n"));
        error_count++;
        bTippingError = true;
        state = SENSOR_STATE_SPIKE;
        break;
      }
      #else
      // Always add full data
      rain.tips_full++;
      rain.rain_full = rain.tips_full * param.configuration->sensors.rain_for_tip;
      // Add this Value only if system is not in maintenance mode
      if(!param.system_status->flags.is_maintenance) {
        rain.tips_count++;
        rain.tips_scroll++;
        rain.rain = rain.tips_count * param.configuration->sensors.rain_for_tip;
        rain.rain_scroll = rain.tips_scroll * param.configuration->sensors.rain_for_tip;
      }
      TRACE_INFO_F(F("Sensor: Rain tips (count, full)\t%d\t%d,\r\n"), rain.tips_count, rain.tips_full);
      #endif

      edata.value = rain.tips_count;
      edata.index = RAIN_TIPS_INDEX;
      param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_PUSHDATA_MS));

      edata.value = rain.rain;
      edata.index = RAIN_RAIN_INDEX;
      param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_PUSHDATA_MS));

      edata.value = rain.rain_full;
      edata.index = RAIN_FULL_INDEX;
      param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_PUSHDATA_MS));

      edata.value = rain.rain_scroll;
      edata.index = RAIN_SCROLL_INDEX;
      param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_PUSHDATA_MS));

      #if (ENABLE_STACK_USAGE)
      TaskMonitorStack();
      #endif

      state = SENSOR_STATE_END;
      break;

    case SENSOR_STATE_END:
      // Inibith (Interrupt) next input data for event_end_time_ms and check error stall
      TaskWatchDog(param.configuration->sensors.event_end_time_ms);
      Delay(Ticks::MsToTicks(param.configuration->sensors.event_end_time_ms));

      // Saving Signal and exit event
      state = SENSOR_STATE_SAVE_SIGNAL;
      break;

    case SENSOR_STATE_SPIKE:
      // Standard control with only one PIN
      if (digitalRead(TIPPING_BUCKET_PIN) == TIPPING_EVENT_VALUE)
      {
        bMainError = true;
        TRACE_INFO_F(F("Sensor: Main tipping wrong timing or stalled tipping bucket\r\n"));
        // Signal an error
      }
      #if (USE_TIPPING_BUCKET_REDUNDANT)
      if (digitalRead(TIPPING_BUCKET_PIN) == TIPPING_EVENT_VALUE)
      {
        bRedundantError = true;
        TRACE_INFO_F(F("Sensor: Redundant tipping wrong timing or stalled tipping bucket\r\n"));
        // Signal an error
      }
      #endif
      // Continuate switch, (No Break here...)

    case SENSOR_STATE_SAVE_SIGNAL:

      // Checking signal Event and error sensor
      param.systemStatusLock->Take();
      param.system_status->events.is_main_error = bMainError;
      param.system_status->events.is_redundant_error = bRedundantError;
      param.system_status->events.is_tipping_error = bTippingError;
      param.system_status->events.error_count = error_count;
      param.systemStatusLock->Give();

      #if (ENABLE_STACK_USAGE)
      TaskMonitorStack();
      #endif

      state = SENSOR_STATE_INIT;
      break;
    }
  }
}

/// @brief ISR Waiting event (restore task)
void RainSensorTask::ISR_tipping_bucket() {
  // Event TIPPING_BUCKET_PIN start when interrupt has occurred (security check on reading state)
  uint8_t flags = RAIN_TIPS_INDEX;
  BaseType_t pxHigherPTW = true;
  // Event is also in execution... Exit, else Calling Queue from ISR
  if(!is_isr_event_running) {
    // Start an ISR Event
    is_isr_event_running = true;
    // enable Tipping bucket task queue
    localRainQueue->EnqueueFromISR(&flags, &pxHigherPTW);
  }
}

#endif