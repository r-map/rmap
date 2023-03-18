/**@file elaborate_data_task.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@digiteco.it>
authors:
Marco Baldinetti <marco.baldinetti@digiteco.it>

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
     param.system_status->tasks->watch_dog = wdt_flag::set;
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

  //TODO:_TH_RAIN_
  bufferReset<sample_t, uint16_t, rmapdata_t>(&mppt_main_samples, SAMPLES_COUNT_MAX);
  bufferReset<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX);

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  while (true) {

    // ********* SYSTEM QUEUE MESSAGE ***********
    // enqueud system message from caller task
    if (!param.systemMessageQueue->IsEmpty()) {
        // Read queue in test mode
        if (param.systemMessageQueue->Peek(&system_message, 0))
        {
            // Its request addressed into ALL TASK... -> no pull (only SUPERVISOR or exernal gestor)
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

    // enqueud from th sensors task (populate data)
    if (!param.elaborataDataQueue->IsEmpty()) {
      if (param.elaborataDataQueue->Peek(&edata, 0))
      {
        param.elaborataDataQueue->Dequeue(&edata, 0);
        switch (edata.index)
        {
        case POWER_MPPT_MAIN_INDEX:
          TRACE_VERBOSE_F(F("Rain A [ %s ]: %d\r\n"), MAIN_STRING, edata.value);
          addValue<sample_t, uint16_t, rmapdata_t>(&mppt_main_samples, SAMPLES_COUNT_MAX, edata.value);
          addValue<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX, param.system_status->flags.is_maintenance);
          break;

        }
      }
    }

    // enqueued from can task (get data, start command...)
    if (!param.elaborataDataQueue->IsEmpty()) {
      if (param.requestDataQueue->Peek(&request_data, 0))
      {
        // send request to elaborate task (all data is present verified on elaborate_task)
        param.requestDataQueue->Dequeue(&request_data, 0);
        make_report(request_data.is_init, request_data.report_time_s, request_data.observation_time_s);
        param.reportDataQueue->Enqueue(&report, 0);
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

uint8_t ElaborateDataTask::checkMppt(rmapdata_t main_mppt) {
  // Optional check quality data function
  uint8_t quality = 100;
  return quality;
}

void ElaborateDataTask::make_report (bool is_init, uint16_t report_time_s, uint8_t observation_time_s) {
  rmapdata_t battery_charge = 0;
  rmapdata_t battery_voltage = 0;
  rmapdata_t battery_current = 0;
  rmapdata_t input_voltage = 0;
  rmapdata_t input_current = 0;

  rmapdata_t avg_battery_charge = 0;
  rmapdata_t avg_battery_voltage = 0;
  rmapdata_t avg_battery_current = 0;
  rmapdata_t avg_input_voltage = 0;
  rmapdata_t avg_input_current = 0;

  bool measures_maintenance = false;

  uint16_t valid_count_battery_charge = 0;
  uint16_t error_count_battery_charge = 0;
  float error_battery_charge_per = 0;

  uint16_t valid_count_battery_voltage = 0;
  uint16_t error_count_battery_voltage = 0;
  float error_battery_voltage_per = 0;

  uint16_t valid_count_battery_current = 0;
  uint16_t error_count_battery_current = 0;
  float error_battery_current_per = 0;

  uint16_t valid_count_input_voltage = 0;
  uint16_t error_count_input_voltage = 0;
  float error_input_voltage_per = 0;

  uint16_t valid_count_input_current = 0;
  uint16_t error_count_input_current = 0;
  float error_input_current_per = 0;

  static uint16_t valid_count_battery_charge_o;
  static uint16_t error_count_battery_charge_o;
  float error_battery_charge_per = 0;

  static uint16_t valid_count_battery_voltage_o;
  static uint16_t error_count_battery_voltage_o;
  float error_battery_voltage_per = 0;

  static uint16_t valid_count_battery_current_o;
  static uint16_t error_count_battery_current_o;
  float error_battery_charge_per = 0;

  static uint16_t valid_count_input_voltage_o;
  static uint16_t error_count_input_voltage_o;
  float error_input_voltage_per = 0;

  static uint16_t valid_count_input_current_o;
  static uint16_t error_count_input_current_o;
  float error_input_current_per = 0;

  static rmapdata_t avg_battery_charge_o;
  static rmapdata_t avg_quality_battery_charge_o;

  static rmapdata_t avg_battery_voltage_o;
  static rmapdata_t avg_quality_battery_voltage_o;

  static rmapdata_t avg_battery_current_o;
  static rmapdata_t avg_quality_battery_current_o;

  static rmapdata_t avg_input_voltage_o;
  static rmapdata_t avg_quality_input_voltage_o;

  static rmapdata_t avg_input_current_o;
  static rmapdata_t avg_quality_input_current_o;

  uint16_t report_sample_count = round((report_time_s * 1.0) / (param.configuration->sensor_acquisition_delay_ms / 1000.0));
  uint16_t observation_sample_count = round((observation_time_s * 1.0) / (param.configuration->sensor_acquisition_delay_ms / 1000.0));

  if (is_init) {
    valid_count_o = 0;
    error_count_o = 0;

    avg_battery_charge_o = 0;
    avg_quality_battery_charge_o = 0;
    avg_battery_voltage_o = 0;
    avg_quality_battery_voltage_o = 0;
    avg_battery_current_o = 0;
    avg_quality_battery_current_o = 0;
    avg_input_voltage_o = 0;
    avg_quality_input_voltage_o = 0;
    avg_input_current_o = 0;
    avg_quality_input_current_o = 0;
  }

  report.avg_battery_charge_o = RMAPDATA_MAX;
  report.avg_battery_charge_quality_o = RMAPDATA_MAX;
  report.avg_battery_voltage_o = RMAPDATA_MAX;
  report.avg_battery_voltage_quality_o = RMAPDATA_MAX;
  report.avg_battery_current_o = RMAPDATA_MAX;
  report.avg_battery_current_quality_o = RMAPDATA_MAX;
  report.avg_input_voltage_o = RMAPDATA_MAX;
  report.avg_input_voltage_quality_o = RMAPDATA_MAX;
  report.avg_input_current_o = RMAPDATA_MAX;
  report.avg_input_current_quality_o = RMAPDATA_MAX;

  if (report_time_s && observation_time_s)
  {
    TRACE_INFO_F(F("Making report on %d seconds\r\n"), report_time_s);
    TRACE_DEBUG_F(F("-> %d samples counts need for report\r\n"), report_sample_count);
    TRACE_DEBUG_F(F("-> %d samples counts need for observation\r\n"), observation_sample_count);
    TRACE_DEBUG_F(F("-> %d observation counts need for report\r\n"), report_sample_count / observation_sample_count);
    TRACE_DEBUG_F(F("-> %d available battery charge samples count\r\n"), battery_charge_samples.count);
    TRACE_DEBUG_F(F("-> %d available battery voltage samples count\r\n"), battery_voltage_samples.count);
    TRACE_DEBUG_F(F("-> %d available battery current samples count\r\n"), battery_current_samples.count);
    TRACE_DEBUG_F(F("-> %d available input voltage samples count\r\n"), input_voltage_samples.count);
    TRACE_DEBUG_F(F("-> %d available input current samples count\r\n"), input_current_samples.count);
  }

  bufferPtrResetBack<sample_t, uint16_t>(&battery_charge_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&battery_voltage_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&battery_current_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&input_voltage_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&input_current_samples, SAMPLES_COUNT_MAX);

  bufferPtrResetBack<maintenance_t, uint16_t>(&maintenance_samples, SAMPLES_COUNT_MAX);

  // align all sensor's data to last common acquired sample
  uint16_t samples_count = battery_charge_samples.count;

  if (battery_voltage_samples.count < samples_count)
  {
    samples_count = battery_voltage_samples.count;
  }

  if (battery_current_samples.count < samples_count)
  {
    samples_count = battery_current_samples.count;
  }

  if (input_voltage_samples.count < samples_count)
  {
    samples_count = input_voltage_samples.count;
  }

  if (input_current_samples.count < samples_count)
  {
    samples_count = input_current_samples.count;
  }

  // flush all data that is not aligned
  for (uint16_t i = samples_count; i < battery_charge_samples.count; i++)
  {
    bufferReadBack<sample_t, uint16_t, rmapdata_t>(&battery_charge_samples, SAMPLES_COUNT_MAX);
    bufferReadBack<maintenance_t, uint16_t, rmapdata_t>(&maintenance_samples, SAMPLES_COUNT_MAX);
  }

  for (uint16_t i = samples_count; i < battery_voltage_samples.count; i++)
  {
    bufferReadBack<sample_t, uint16_t, rmapdata_t>(&battery_voltage_samples, SAMPLES_COUNT_MAX);
  }

  for (uint16_t i = samples_count; i < battery_current_samples.count; i++)
  {
    bufferReadBack<sample_t, uint16_t, rmapdata_t>(&battery_current_samples, SAMPLES_COUNT_MAX);
  }

  for (uint16_t i = samples_count; i < input_voltage_samples.count; i++)
  {
    bufferReadBack<sample_t, uint16_t, rmapdata_t>(&input_voltage_samples, SAMPLES_COUNT_MAX);
  }

  for (uint16_t i = samples_count; i < input_current_samples.count; i++)
  {
    bufferReadBack<sample_t, uint16_t, rmapdata_t>(&input_current_samples, SAMPLES_COUNT_MAX);
  }

  // it's a report request
  if (report_time_s && observation_time_s)
  {
    for (uint16_t i = 0; i < samples_count.count; i++)
    {
      battery_charge = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&battery_charge_samples, SAMPLES_COUNT_MAX);
      measures_maintenance = bufferReadBack<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX);

      if (!measures_maintenance)
      {
        avg_quality_battery_charge += (rmapdata_t)((checkMppt(battery_charge) - avg_quality_battery_charge) / (i + 1));

        if (ISVALID_RMAPDATA(battery_charge))
        {
          valid_count_battery_charge++;
          avg_battery_charge += (rmapdata_t)((avg_battery_charge - battery_charge) / valid_count_battery_charge);
        }
        else
        {
          error_count_battery_charge++;
        }
      }

      battery_voltage = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&battery_voltage_samples, SAMPLES_COUNT_MAX);
      measures_maintenance = bufferReadBack<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX);

      if (!measures_maintenance)
      {
        avg_quality_battery_voltage += (rmapdata_t)((checkMppt(battery_voltage) - avg_quality_battery_voltage) / (i + 1));

        if (ISVALID_RMAPDATA(battery_voltage))
        {
          valid_count_battery_voltage++;
          avg_battery_voltage += (rmapdata_t)((avg_battery_voltage - battery_voltage) / valid_count_battery_voltage);
        }
        else
        {
          error_count_battery_voltage++;
        }
      }

      battery_current = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&battery_current_samples, SAMPLES_COUNT_MAX);
      measures_maintenance = bufferReadBack<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX);

      if (!measures_maintenance)
      {
        avg_quality_battery_current += (rmapdata_t)((checkMppt(battery_current) - avg_quality_battery_current) / (i + 1));

        if (ISVALID_RMAPDATA(battery_current))
        {
          valid_count_battery_current++;
          avg_battery_current += (rmapdata_t)((avg_battery_current - battery_current) / valid_count_battery_current);
        }
        else
        {
          error_count_battery_current++;
        }
      }

      input_voltage = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&input_voltage_samples, SAMPLES_COUNT_MAX);
      measures_maintenance = bufferReadBack<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX);

      if (!measures_maintenance)
      {
        avg_quality_input_voltage += (rmapdata_t)((checkMppt(input_voltage) - avg_quality_input_voltage) / (i + 1));

        if (ISVALID_RMAPDATA(input_voltage))
        {
          valid_count_input_voltage++;
          avg_input_voltage += (rmapdata_t)((avg_input_voltage - input_voltage) / valid_count_input_voltage);
        }
        else
        {
          error_count_input_voltage++;
        }
      }

      input_current = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&input_current_samples, SAMPLES_COUNT_MAX);
      measures_maintenance = bufferReadBack<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX);

      if (!measures_maintenance)
      {
        avg_quality_input_current += (rmapdata_t)((checkMppt(input_current) - avg_quality_input_current) / (i + 1));

        if (ISVALID_RMAPDATA(input_current))
        {
          valid_count_input_current++;
          avg_input_current += (rmapdata_t)((avg_input_current - input_current) / valid_count_input_current);
        }
        else
        {
          error_count_input_current++;
        }
      }
    }

    error_battery_charge_per = (float)(error_count_battery_charge) / (float)(battery_charge_samples.count) * 100.0;
    TRACE_DEBUG_F(F("-> %d battery charge error (%d%%)\r\n"), error_count_battery_charge, (int32_t)error_battery_charge_per);

    bufferPtrResetBack<maintenance_t, uint16_t>(&maintenance_samples, SAMPLES_COUNT_MAX);

    if (battery_charge_samples.count >= observation_sample_count)
    {
      // sufficient number of valid samples
      if (valid_count_battery_charge && (error_battery_charge_per <= SAMPLE_ERROR_PERCENTAGE_MAX))
      {
        valid_count_battery_charge_o++;

        avg_battery_charge_o += (rmapdata_t)((avg_battery_charge - avg_battery_charge_o) / valid_count_battery_charge_o);
        avg_battery_charge_quality_o += (rmapdata_t)((avg_battery_charge_quality - avg_battery_charge_quality_o) / (valid_count_battery_charge_o + error_count_battery_charge_o));
      }
      else
      {
        error_count_battery_charge_o++;
      }

      error_battery_voltage_per_o = (float)(error_count_battery_charge_o) / (float)(observation_sample_count)*100.0;
      TRACE_DEBUG_F(F("-> %d solar radiation observation error (%d%%)\r\n"), error_count_battery_charge_o, (int32_t)error_mppt_per_o);

      if (valid_count_battery_charge_o && (error_battery_voltage_per_o <= OBSERVATION_ERROR_PERCENTAGE_MAX))
      {
        report.avg_battery_charge = avg_battery_charge_o;
        report.avg_battery_charge_quality = avg_battery_charge_quality_o;
      }
    }

    error_battery_voltage_per = (float)(error_count_battery_voltage) / (float)(battery_voltage_samples.count) * 100.0;
    TRACE_DEBUG_F(F("-> %d battery voltage error (%d%%)\r\n"), error_count_battery_voltage, (int32_t)error_battery_voltage_per);

    bufferPtrResetBack<maintenance_t, uint16_t>(&maintenance_samples, SAMPLES_COUNT_MAX);

    if (battery_voltage_samples.count >= observation_sample_count)
    {
      // sufficient number of valid samples
      if (valid_count_battery_voltage && (error_battery_voltage_per <= SAMPLE_ERROR_PERCENTAGE_MAX))
      {
        valid_count_battery_voltage_o++;

        avg_battery_voltage_o += (rmapdata_t)((avg_battery_voltage - avg_battery_voltage_o) / valid_count_battery_voltage_o);
        avg_battery_voltage_quality_o += (rmapdata_t)((avg_battery_voltage_quality - avg_battery_voltage_quality_o) / (valid_count_battery_voltage_o + error_count_battery_voltage_o));
      }
      else
      {
        error_count_battery_voltage_o++;
      }

      error_count_battery_voltage_per_o = (float)(error_count_battery_voltage_o) / (float)(observation_sample_count)*100.0;
      TRACE_DEBUG_F(F("-> %d solar radiation observation error (%d%%)\r\n"), error_count_battery_voltage_o, (int32_t)error_mppt_per_o);

      if (valid_count_battery_voltage_o && (error_count_battery_voltage_per_o <= OBSERVATION_ERROR_PERCENTAGE_MAX))
      {
        report.avg_battery_voltage = avg_battery_voltage_o;
        report.avg_battery_voltage_quality = avg_battery_voltage_quality_o;
      }
    }

    error_battery_current_per = (float)(error_count_battery_current) / (float)(battery_current_samples.count) * 100.0;
    TRACE_DEBUG_F(F("-> %d battery current error (%d%%)\r\n"), error_count_battery_current, (int32_t)error_battery_current_per);

    bufferPtrResetBack<maintenance_t, uint16_t>(&maintenance_samples, SAMPLES_COUNT_MAX);

    if (battery_current_samples.count >= observation_sample_count)
    {
      // sufficient number of valid samples
      if (valid_count_battery_current && (error_battery_current_per <= SAMPLE_ERROR_PERCENTAGE_MAX))
      {
        valid_count_battery_current_o++;

        avg_battery_current_o += (rmapdata_t)((avg_battery_current - avg_battery_current_o) / valid_count_battery_current_o);
        avg_battery_current_quality_o += (rmapdata_t)((avg_battery_current_quality - avg_battery_current_quality_o) / (valid_count_battery_current_o + error_count_battery_current_o));
      }
      else
      {
        error_count_battery_current_o++;
      }

      error_battery_current_per_o = (float)(error_count_battery_current_o) / (float)(observation_sample_count)*100.0;
      TRACE_DEBUG_F(F("-> %d solar radiation observation error (%d%%)\r\n"), error_count_battery_current_o, (int32_t)error_mppt_per_o);

      if (valid_count_battery_current_o && (error_battery_current_per_o <= OBSERVATION_ERROR_PERCENTAGE_MAX))
      {
        report.avg_battery_current = avg_battery_current_o;
        report.avg_battery_current_quality = avg_battery_current_quality_o;
      }
    }

    error_input_voltage_per = (float)(error_count_input_voltage) / (float)(input_voltage_samples.count) * 100.0;
    TRACE_DEBUG_F(F("-> %d battery current error (%d%%)\r\n"), error_count_input_voltage, (int32_t)error_input_voltage_per);

    bufferPtrResetBack<maintenance_t, uint16_t>(&maintenance_samples, SAMPLES_COUNT_MAX);

    if (input_voltage_samples.count >= observation_sample_count)
    {
      // sufficient number of valid samples
      if (valid_count_input_voltage && (error_input_voltage_per <= SAMPLE_ERROR_PERCENTAGE_MAX))
      {
        valid_count_input_voltage_o++;

        avg_input_voltage_o += (rmapdata_t)((avg_input_voltage - avg_input_voltage_o) / valid_count_input_voltage_o);
        avg_input_voltage_quality_o += (rmapdata_t)((avg_input_voltage_quality - avg_input_voltage_quality_o) / (valid_count_input_voltage_o + error_count_input_voltage_o));
      }
      else
      {
        error_count_input_voltage_o++;
      }

      error_input_voltage_per_o = (float)(error_count_input_voltage_o) / (float)(observation_sample_count)*100.0;
      TRACE_DEBUG_F(F("-> %d solar radiation observation error (%d%%)\r\n"), error_count_input_voltage_o, (int32_t)error_mppt_per_o);

      if (valid_count_input_voltage_o && (error_input_voltage_per_o <= OBSERVATION_ERROR_PERCENTAGE_MAX))
      {
        report.avg_input_voltage = avg_input_voltage_o;
        report.avg_input_voltage_quality = avg_input_voltage_quality_o;
      }
    }

    error_input_current_per = (float)(error_count_input_current) / (float)(input_current_samples.count) * 100.0;
    TRACE_DEBUG_F(F("-> %d battery current error (%d%%)\r\n"), error_count_input_current, (int32_t)error_input_current_per);

    bufferPtrResetBack<maintenance_t, uint16_t>(&maintenance_samples, SAMPLES_COUNT_MAX);

    if (input_current_samples.count >= observation_sample_count)
    {
      // sufficient number of valid samples
      if (valid_count_input_current && (error_input_current_per <= SAMPLE_ERROR_PERCENTAGE_MAX))
      {
        valid_count_input_current_o++;

        avg_input_current_o += (rmapdata_t)((avg_input_current - avg_input_current_o) / valid_count_input_current_o);
        avg_input_current_quality_o += (rmapdata_t)((avg_input_current_quality - avg_input_current_quality_o) / (valid_count_input_current_o + error_count_input_current_o));
      }
      else
      {
        error_count_input_current_o++;
      }

      error_input_current_per_o = (float)(error_count_input_current_o) / (float)(observation_sample_count)*100.0;
      TRACE_DEBUG_F(F("-> %d solar radiation observation error (%d%%)\r\n"), error_count_input_current_o, (int32_t)error_mppt_per_o);

      if (valid_count_input_current_o && (error_input_current_per_o <= OBSERVATION_ERROR_PERCENTAGE_MAX))
      {
        report.avg_input_current = avg_input_current_o;
        report.avg_input_current_quality = avg_input_current_quality_o;
      }
    }
  }
  // it's a sample request
  else
  {
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
