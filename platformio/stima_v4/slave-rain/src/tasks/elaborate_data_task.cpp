/**
  ******************************************************************************
  * @file    elaborate_data_task.cpp
  * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Elaborate data sensor to CAN source file
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

#define TRACE_LEVEL     ELABORATE_DATA_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   ELABORATE_TASK_ID

#include "tasks/elaborate_data_task.h"

using namespace cpp_freertos;

ElaborateDataTask::ElaborateDataTask(const char *taskName, uint16_t stackSize, uint8_t priority, ElaborateDataParam_t elaboradeDataParam) : Thread(taskName, stackSize, priority), param(elaboradeDataParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(ELABORATE_DATA_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  state = ELABORATE_DATA_INIT;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void ElaborateDataTask::TaskMonitorStack()
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
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void ElaborateDataTask::TaskWatchDog(uint32_t millis_standby)
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
void ElaborateDataTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
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

void ElaborateDataTask::Run() {
  // Queue for data
  elaborate_data_t edata;
  request_data_t request_data;
  // System message data queue structured
  system_message_t system_message;

  uint8_t edata_cmd; // command function to elaborate data queue

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  // Init scrolling TPR timings and buffer data
  // Maintenance not performed (automatic checked in rain_task)
  rain_elaborate.rain_scroll = 0;
  bufferReset<sample_t, uint16_t, rmapdata_t>(&rain_samples, SAMPLES_COUNT_MAX);
  uint32_t next_ms_buffer_check = millis() + SAMPLES_ACQUIRE_MS;
  uint32_t request_data_init_ms = millis();
  uint32_t sec_from_elaborate_start = 0;

  // Init real and last elaborate data response variables
  memset(&report, 0, sizeof(report_t));
  memset(&report_last, 0, sizeof(report_t));

  while (true) {

    // Elaborate scrolling XX required defined msec.
    if(millis() >= next_ms_buffer_check) {
      // Sec from starting (check valid RMAP data response timing when minimal data acquire is ready)
      sec_from_elaborate_start += SAMPLES_ACQUIRE_MS / 1000;
      next_ms_buffer_check += SAMPLES_ACQUIRE_MS;
      // ? over roll and security check timer area into reset check ms expected
      if (labs(millis() - next_ms_buffer_check) > SAMPLES_ACQUIRE_MS) {
        next_ms_buffer_check = millis() + SAMPLES_ACQUIRE_MS;
      }
      // Add current scrolling data to buffer
      addValue<sample_t, uint16_t, rmapdata_t>(&rain_samples, SAMPLES_COUNT_MAX, rain_elaborate.rain_scroll);
      // Perform reset on rain task. Security local new ungetted data rain.rain_scroll to 0
      rain_elaborate.rain_scroll = 0;
      edata_cmd = RAIN_SCROLL_RESET;
      param.rainQueue->Enqueue(&edata_cmd, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_RESET_TIP_MS));
    }

    // ********* SYSTEM QUEUE MESSAGE ***********
    // enqueud system message from caller task
    if (!param.systemMessageQueue->IsEmpty()) {
        // Read queue in test mode
        if (param.systemMessageQueue->Peek(&system_message, 0))
        {
            // Its request addressed into ALL TASK... -> no pull (only SUPERVISOR or external gestor)
            if(system_message.task_dest == ALL_TASK_ID)
            {
                // Pull && elaborate command, 
                if(system_message.command.do_sleep)
                {
                    // Enter sleep module OK and update WDT
                    TaskWatchDog(ELABORATE_TASK_SLEEP_DELAY_MS);
                    TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
                    Delay(Ticks::MsToTicks(ELABORATE_TASK_SLEEP_DELAY_MS));
                    TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
                }
            }
        }
    }

    // enqueud from rain sensors task (populate data)
    if (!param.elaborateDataQueue->IsEmpty()) {
      if (param.elaborateDataQueue->Peek(&edata, 0))
      {
        param.elaborateDataQueue->Dequeue(&edata);
        switch (edata.index)
        {
        case RAIN_TIPS_INDEX:
          TRACE_VERBOSE_F(F("Rain tips: %d\r\n"), edata.value);
          rain_elaborate.tips_count = (uint16_t)edata.value;
          break;

        case RAIN_RAIN_INDEX:
          TRACE_VERBOSE_F(F("Rain: %d\r\n"), edata.value);
          rain_elaborate.rain = (uint16_t)edata.value;
          break;

        case RAIN_FULL_INDEX:
          TRACE_VERBOSE_F(F("Rain full: %d\r\n"), edata.value);
          rain_elaborate.rain_full = (uint16_t)edata.value;
          break;

        case RAIN_SCROLL_INDEX:
          TRACE_VERBOSE_F(F("Rain scroll: %d\r\n"), edata.value);
          rain_elaborate.rain_scroll = (uint16_t)edata.value;
          break;
        }
      }
    }

    // enqueued from can task (get data, start command...)
    if (!param.requestDataQueue->IsEmpty()) {
      // Read request immediatly if queue is not empty
      if (param.requestDataQueue->Dequeue(&request_data))
      {
        // ? over roll and security check timer area into reset check ms expected
        // Report request is too fast (<REPORT_INVALID_ACQUIRE_MIN_MS) ... N.B. When are in Command retry!
        // Initialize value if command retry is also executed (before) but result data can be resetted
        // Need to save older value and retry value must are older value (only in retry command)
        // After timing retry command, value data report is ok and newver resetted value can be sended
        if (request_data.is_init) {
          // ? Request valid (last init request > REPORT_INVALID_ACQUIRE_MIN_MS)
          if (labs(millis() - request_data_init_ms) > REPORT_INVALID_ACQUIRE_MIN_MS) {
            // test if minimal data acquire are valid
            bool min_percentage_acquire = false;
            if (sec_from_elaborate_start < request_data.report_time_s) {
              if(((((float)sec_from_elaborate_start / (float)request_data.report_time_s)) * 100.0) < SAMPLE_ERROR_PERCENTAGE_MIN) {
                min_percentage_acquire = true;
              }
            }
            if(!min_percentage_acquire) {
              // Coerent value, generate report
              make_report(request_data.report_time_s, request_data.observation_time_s);
            } else {
              // Error timing not full, void report
              report.quality = RMAPDATA_MAX;
              report.rain = RMAPDATA_MAX;
              report.rain_full = RMAPDATA_MAX;
              report.rain_tpr_05m_avg = RMAPDATA_MAX;
              report.rain_tpr_60s_avg = RMAPDATA_MAX;
              report.tips_count = RMAPDATA_MAX;
            }
            // Saving data before reset index and scrolling value (Need for possible retry...)
            report_last = report;
            // Response with new value (resetted ore istant value. No retry command Here)
            param.reportDataQueue->Enqueue(&report);

            // Save timing for report request and with init index value for saving remote data
            request_data_init_ms = millis();

            // Forced reset index internal (internal index are updated only with queue on event from rain)
            // If no event occurs the queue would not fill and the value would remain the previous one
            rain_elaborate.tips_count = 0;
            rain_elaborate.tips_full = 0;
            rain_elaborate.rain = 0;
            rain_elaborate.rain_full = 0;

            // Perform reset on rain task (event = false) No Event of Rain. (Other incoming Request)
            edata_cmd = RAIN_TIPS_RESET;
            param.rainQueue->Enqueue(&edata_cmd, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_RESET_TIP_MS));

          } else {
            // Response with older value (Retry command Here without reinit command also called up)
            param.reportDataQueue->Enqueue(&report_last);
          }
        } else {
          // Response with new value (istant value. Unused retry command Here)
          make_report(request_data.report_time_s, request_data.observation_time_s);
          param.reportDataQueue->Enqueue(&report);
        }
      }
    }

    #if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
    #endif

    // Local TaskWatchDog update;
    TaskWatchDog(ELABORATE_TASK_WAIT_DELAY_MS);

    DelayUntil(Ticks::MsToTicks(ELABORATE_TASK_WAIT_DELAY_MS));
  }
}

/// @brief Check sensor Quality
/// @param  None
/// @return Quality of measure (0-100%)
uint8_t ElaborateDataTask::checkRain(void) {
  float quality = 100.0;

  #if (USE_CLOGGED_UP_CONTROL)
  // Checking signal CLOGGED_UP (on Event or Reset... remote calling)
  param.systemStatusLock->Take();
  #if (CLOGGED_EVENT_VALUE)
  param.system_status->events.is_clogged_up = digitalRead(CLOGGED_UP_PIN);
  #else
  param.system_status->events.is_clogged_up = !digitalRead(CLOGGED_UP_PIN);
  #endif
  param.systemStatusLock->Give();
  if(param.system_status->events.is_clogged_up) {
      TRACE_INFO_F(F("Sensor: read status of clogged up... [ ALERT ]\r\n"));
  }
  #endif

  if(param.system_status->events.is_clogged_up) {
    quality = 0.0;
  } else {
    if(param.system_status->events.is_tipping_error) {
      quality -= param.system_status->events.error_count; // 1 Error = -1%
      if(quality<0.0) quality = 0.0;
    }
    // Reduce 15% for error on one reed
    if(param.system_status->events.is_main_error) quality *= 0.85;
    if(param.system_status->events.is_redundant_error) quality *= 0.85;
    // Reduce 30% on error bubble_level (if accelerometr working correctly)
    if((!param.system_status->events.is_accelerometer_error) &&
       (param.system_status->events.is_bubble_level_error)) quality *= 0.7;
  }
  return (uint8_t) quality;
}

/// @brief Create an RMAP report value
/// @param report_time_s time of report
/// @param observation_time_s time to make an observation
void ElaborateDataTask::make_report(uint16_t report_time_s, uint8_t observation_time_s)
{
  #if (USE_MOBILE_TPR_60_S_AVG_MODE)
  uint16_t rain_buf_60s[SAMPLES_NEED_TPR_60_S] = {0};  // Scroll buffer for max on 60 sec
  #endif
  uint16_t rain_ist;              // Istant buffered data on 10 sec
  uint16_t rain_sum_60s = 0;      // current sum on 60 sec
  uint16_t rain_sum_60s_max = 0;  // max sum on mobile window of 60 sec.
  uint16_t rain_sum_05m = 0;      // current sum on 5 min
  uint16_t rain_sum_05m_max = 0;  // max sum on fixed window of 5 min.

  float valid_data_calc_perc;         // Shared calculate valid % of measure (Data Valid or not?)
  uint16_t n_sample = 0;              // Sample elaboration number... (incremented on calc development)
  bool is_05m_sample_valid = false;   // True if sample 05 min is valid (1 sample complete min)

  // Elaboration timings calculation (fix 5Min to TPR 5 Min, Fix 60 Sec to TPR 60 Sec)
  uint16_t report_sample_count = round((report_time_s * 1.0) / (SAMPLES_ACQUIRE_MS / 1000.0));
  uint16_t observation_sample_count = round((observation_time_s * 1.0) / (SAMPLES_ACQUIRE_MS / 1000.0));

  // Request to calculate is correct? Trace request
  if (report_time_s == 0)
  {
    // Request an direct sample value for istant measure
    TRACE_INFO_F(F("Elaborate: Requested an istant value\r\n"));
  }
  else
  {
    TRACE_INFO_F(F("Elaborate: Requested an report on %d seconds\r\n"), report_time_s);
    TRACE_DEBUG_F(F("-> %d samples counts need for report\r\n"), report_sample_count);
    TRACE_DEBUG_F(F("-> %d samples counts need for observation\r\n"), observation_sample_count);
    TRACE_DEBUG_F(F("-> %d available rain samples count\r\n"), rain_samples.count);
  }

  // Default value to RMAP Limit error value
  report.rain_tpr_60s_avg = RMAPDATA_MAX;
  report.rain_tpr_05m_avg = RMAPDATA_MAX;

  // Ptr for value sample
  bufferPtrResetBack<sample_t, uint16_t>(&rain_samples, SAMPLES_COUNT_MAX);

  // align all sensor's data to last common acquired sample
  uint16_t samples_count = rain_samples.count;

  // No need ... it's a simple istant or report request?
  for (uint16_t i = 0; i < samples_count; i++)
  {
    // Calculate rain rate on 60" mobile to report timing area
    // End of Sample in calculation (Completed with the request... Exit on Full Buffer ReadBack executed)
    if(n_sample >= report_sample_count) break;

    // Reading data ist from buffered value
    rain_ist = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&rain_samples, SAMPLES_COUNT_MAX);
    n_sample++;

    #if (USE_MOBILE_TPR_60_S_AVG_MODE)
    // Scrolling older value on timing observation value area
    for(uint8_t rr_idx = (SAMPLES_NEED_TPR_60_S - 1); rr_idx > 0; rr_idx--) {
      rain_buf_60s[rr_idx] = rain_buf_60s[rr_idx-1];
    }
    rain_buf_60s[0] = rain_ist;
    // Calculate SUM on timing obeservation in mobile mode
    rain_sum_60s = 0;
    for(uint8_t rr_idx = 0; rr_idx < SAMPLES_NEED_TPR_60_S; rr_idx++) {
      rain_sum_60s += rain_buf_60s[rr_idx];
    }
    if(rain_sum_60s > rain_sum_60s_max) rain_sum_60s_max = rain_sum_60s;
    #else
    // Populate buffer SUM on 60 sec - Fixed Calculate MAX on 60 SEC
    rain_sum_60s += rain_ist;
    if((n_sample % SAMPLES_NEED_TPR_60_S)==0) {
      // Calculate MAX of SUM and reinit for next timing check
      if(rain_sum_60s > rain_sum_60s_max) rain_sum_60s_max = rain_sum_60s;
      rain_sum_60s = 0;
    }
    #endif

    // Populate buffer SUM on 5 MIN - Fixed Calculate MAX on 5 MIN
    rain_sum_05m += rain_ist;
    if((n_sample % SAMPLES_NEED_TPR_05_M)==0) {
      is_05m_sample_valid = true;
      // Calculate MAX of SUM and reinit for next timing check
      if(rain_sum_05m > rain_sum_05m_max) rain_sum_05m_max = rain_sum_05m;
      rain_sum_05m = 0;
    }
  }
  // ***************************************************************************************************
  // ******* GENERATE REPORT RESPONSE WITH ALL DATA AVAIABLE AND VALID WITH EXPECETD OBSERVATION *******
  // ***************************************************************************************************

  report.tips_count = rain_elaborate.tips_count;
  report.rain = rain_elaborate.rain;
  report.rain_full = rain_elaborate.rain_full;
  report.quality = (rmapdata_t)checkRain();

  // rain TPR, elaboration final (if over number min sample)
  valid_data_calc_perc = (float)(n_sample) / (float)(report_sample_count) * 100.0;
  if (valid_data_calc_perc >= OBSERVATION_ERROR_PERCENTAGE_MIN)
  {
    // Use 10^4 rappresentation with 4 decimal
    report.rain_tpr_60s_avg = (rmapdata_t)(((float)rain_sum_60s_max * (float)RAIN_RATE_MULTIPLY) / 60.0);
    if(is_05m_sample_valid) {
      report.rain_tpr_05m_avg = (rmapdata_t)(((float)rain_sum_05m_max * (float)RAIN_RATE_MULTIPLY) / 300.0);
    }
  }
}

template <typename buffer_g, typename length_v, typename value_v>
value_v bufferRead(buffer_g *buffer, length_v length)
{
  value_v value = *buffer->read_ptr;

  if (buffer->read_ptr == buffer->values+length-1) {
    buffer->read_ptr = buffer->values;
  }
  else buffer->read_ptr++;

  return value;
}

template <typename buffer_g, typename length_v, typename value_v>
value_v bufferReadBack(buffer_g *buffer, length_v length)
{
  value_v value = *buffer->read_ptr;

  if (buffer->read_ptr == buffer->values) {
    buffer->read_ptr = buffer->values+length-1;
  }
  else buffer->read_ptr--;

  return value;
}

template <typename buffer_g, typename value_v>
void bufferWrite(buffer_g *buffer, value_v value)
{
  *buffer->write_ptr = value;
}

template <typename buffer_g>
void bufferPtrReset(buffer_g *buffer)
{
  buffer->read_ptr = buffer->values;
}

template <typename buffer_g, typename length_v>
void bufferPtrResetBack(buffer_g *buffer, length_v length)
{
  if (buffer->write_ptr == buffer->values)
  {
    buffer->read_ptr = buffer->values+length-1;
  }
  else buffer->read_ptr = buffer->write_ptr-1;
}

template <typename buffer_g, typename length_v>
void incrementBuffer(buffer_g *buffer, length_v length)
{
  if (buffer->count < length)
  {
    buffer->count++;
  }

  if (buffer->write_ptr+1 < buffer->values + length) {
    buffer->write_ptr++;
  } else buffer->write_ptr = buffer->values;
}

template <typename buffer_g, typename length_v, typename value_v>
void bufferReset(buffer_g *buffer, length_v length)
{
  memset(buffer->values, 0xFF, length * sizeof(value_v));
  buffer->count = 0;
  buffer->read_ptr = buffer->values;
  buffer->write_ptr = buffer->values;
}

template <typename buffer_g, typename length_v, typename value_v>
void addValue(buffer_g *buffer, length_v length, value_v value)
{
  *buffer->write_ptr = (value_v)value;
  incrementBuffer<buffer_g, length_v>(buffer, length);
}
