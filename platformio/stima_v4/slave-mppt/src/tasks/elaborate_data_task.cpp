/**
  ******************************************************************************
  * @file    elaborate_data_task.cpp
  * @author  Marco Baldinetti<m.baldinetti@digiteco.it>
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Elaborate data sensor to CAN source file
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

  bufferReset<sample_t, uint16_t, rmapdata_t>(&input_current_samples, SAMPLES_COUNT_MAX);
  bufferReset<sample_t, uint16_t, rmapdata_t>(&battery_charge_samples, SAMPLES_COUNT_MAX);
  bufferReset<sample_t, uint16_t, rmapdata_t>(&battery_voltage_samples, SAMPLES_COUNT_MAX);
  bufferReset<sample_t, uint16_t, rmapdata_t>(&battery_current_samples, SAMPLES_COUNT_MAX);
  bufferReset<sample_t, uint16_t, rmapdata_t>(&input_voltage_samples, SAMPLES_COUNT_MAX);

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
        case POWER_BATTERY_CHARGE_INDEX:
          TRACE_VERBOSE_F(F("Battery charge: %d\r\n"), edata.value);
          addValue<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX, param.system_status->flags.is_maintenance);
          addValue<sample_t, uint16_t, rmapdata_t>(&battery_charge_samples, SAMPLES_COUNT_MAX, edata.value);
          break;
        case POWER_BATTERY_VOLTAGE_INDEX:
          TRACE_VERBOSE_F(F("Battery voltage: %d\r\n"), edata.value);
          addValue<sample_t, uint16_t, rmapdata_t>(&battery_voltage_samples, SAMPLES_COUNT_MAX, edata.value);
          break;
        case POWER_BATTERY_CURRENT_INDEX:
          TRACE_VERBOSE_F(F("Battery current: %d\r\n"), edata.value);
          addValue<sample_t, uint16_t, rmapdata_t>(&battery_current_samples, SAMPLES_COUNT_MAX, edata.value);
          break;
        case POWER_INPUT_VOLTAGE_INDEX:
          TRACE_VERBOSE_F(F("Input voltage: %d\r\n"), edata.value);
          addValue<sample_t, uint16_t, rmapdata_t>(&input_voltage_samples, SAMPLES_COUNT_MAX, edata.value);
          break;
        case POWER_INPUT_CURRENT_INDEX:
          TRACE_VERBOSE_F(F("Input current: %d\r\n"), edata.value);
          addValue<sample_t, uint16_t, rmapdata_t>(&input_current_samples, SAMPLES_COUNT_MAX, edata.value);
          break;
        }
      }
    }

    // enqueued from can task (get data, start command...)
    if (!param.requestDataQueue->IsEmpty()) {
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

/// @brief Check data in and perform calculate of Optional Quality value
/// @param data_in real value readed from sensor
/// @return value uint_8 percent data quality value
uint8_t ElaborateDataTask::checkMppt_P_Chg(rmapdata_t data_in)
{
  // Optional check quality data function
  uint8_t quality = 100;
  return quality;
}

/// @brief Check data in and perform calculate of Optional Quality value
/// @param data_in real value readed from sensor
/// @return value uint_8 percent data quality value
uint8_t ElaborateDataTask::checkMppt_V_Bat(rmapdata_t data_in)
{
  // Optional check quality data function
  uint8_t quality = 100;
  return quality;
}

/// @brief Check data in and perform calculate of Optional Quality value
/// @param data_in real value readed from sensor
/// @return value uint_8 percent data quality value
uint8_t ElaborateDataTask::checkMppt_I_Bat(rmapdata_t data_in)
{
  // Optional check quality data function
  uint8_t quality = 100;
  return quality;
}

/// @brief Check data in and perform calculate of Optional Quality value
/// @param data_in real value readed from sensor
/// @return value uint_8 percent data quality value
uint8_t ElaborateDataTask::checkMppt_V_In(rmapdata_t data_in)
{
  // Optional check quality data function
  uint8_t quality = 100;
  return quality;
}

/// @brief Check data in and perform calculate of Optional Quality value
/// @param data_in real value readed from sensor
/// @return value uint_8 percent data quality value
uint8_t ElaborateDataTask::checkMppt_I_In(rmapdata_t data_in)
{
  // Optional check quality data function
  uint8_t quality = 100;
  return quality;
}

/// @brief Create a report from buffered sample
/// @param is_init Bool optional backup ptr_wr calc
/// @param report_time_s Report time for the calc
/// @param observation_time_s Observation time for the calc
void ElaborateDataTask::make_report (bool is_init, uint16_t report_time_s, uint8_t observation_time_s)
{
  // Generic and shared var
  bool measures_maintenance = false;  // Maintenance mode?
  float valid_data_calc_perc;         // Shared calculate valid % of measure (Data Valid or not?)
  bool is_observation = false;        // Is an observation (when calculate is requested)
  uint16_t n_sample = 0;              // Sample elaboration number... (incremented on calc development)

  // Elaborate suffix description: _s(sample) _o(observation) _t(total) [Optional for debug]

  // Elaboration battery_charge
  rmapdata_t battery_charge_s = 0;
  float avg_battery_charge_s = 0;
  float avg_battery_charge_o = 0;
  float avg_battery_charge_quality_s = 0;
  float avg_battery_charge_quality_o = 0;
  #if TRACE_LEVEL >= TRACE_LEVEL_DEBUG
  uint16_t valid_count_battery_charge_t = 0;
  #endif
  uint16_t valid_count_battery_charge_s = 0;
  uint16_t total_count_battery_charge_s = 0;
  uint16_t valid_count_battery_charge_o = 0;

  // Elaboration battery_voltage
  rmapdata_t battery_voltage_s = 0;
  float avg_battery_voltage_s = 0;
  float avg_battery_voltage_o = 0;
  float avg_battery_voltage_quality_s = 0;
  float avg_battery_voltage_quality_o = 0;
  #if TRACE_LEVEL >= TRACE_LEVEL_DEBUG
  uint16_t valid_count_battery_voltage_t = 0;
  #endif
  uint16_t valid_count_battery_voltage_s = 0;
  uint16_t total_count_battery_voltage_s = 0;
  uint16_t valid_count_battery_voltage_o = 0;
  
  // Elaboration battery_current
  rmapdata_t battery_current_s = 0;
  float avg_battery_current_s = 0;
  float avg_battery_current_o = 0;
  float avg_battery_current_quality_s = 0;
  float avg_battery_current_quality_o = 0;
  #if TRACE_LEVEL >= TRACE_LEVEL_DEBUG
  uint16_t valid_count_battery_current_t = 0;
  #endif
  uint16_t valid_count_battery_current_s = 0;
  uint16_t total_count_battery_current_s = 0;
  uint16_t valid_count_battery_current_o = 0;

  // Elaboration input_voltage
  rmapdata_t input_voltage_s = 0;
  float avg_input_voltage_s = 0;
  float avg_input_voltage_o = 0;
  float avg_input_voltage_quality_s = 0;
  float avg_input_voltage_quality_o = 0;
  #if TRACE_LEVEL >= TRACE_LEVEL_DEBUG
  uint16_t valid_count_input_voltage_t = 0;
  #endif
  uint16_t valid_count_input_voltage_s = 0;
  uint16_t total_count_input_voltage_s = 0;
  uint16_t valid_count_input_voltage_o = 0;

  // Elaboration input_current
  rmapdata_t input_current_s = 0;
  float avg_input_current_s = 0;
  float avg_input_current_o = 0;
  float avg_input_current_quality_s = 0;
  float avg_input_current_quality_o = 0;
  #if TRACE_LEVEL >= TRACE_LEVEL_DEBUG
  uint16_t valid_count_input_current_t = 0;
  #endif
  uint16_t valid_count_input_current_s = 0;
  uint16_t total_count_input_current_s = 0;
  uint16_t valid_count_input_current_o = 0;

  // Elaboration timings calculation
  uint16_t report_sample_count = round((report_time_s * 1.0) / (param.configuration->sensor_acquisition_delay_ms / 1000.0));
  uint16_t observation_sample_count = round((observation_time_s * 1.0) / (param.configuration->sensor_acquisition_delay_ms / 1000.0));
  uint16_t sample_for_observation = 0;
  if(report_time_s && observation_sample_count) sample_for_observation = report_sample_count / observation_sample_count;

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
    TRACE_DEBUG_F(F("-> %d observation counts need for report\r\n"), sample_for_observation);
    TRACE_DEBUG_F(F("-> %d available battery charge samples count\r\n"), battery_charge_samples.count);
    TRACE_DEBUG_F(F("-> %d available battery voltage samples count\r\n"), battery_voltage_samples.count);
    TRACE_DEBUG_F(F("-> %d available battery current samples count\r\n"), battery_current_samples.count);
    TRACE_DEBUG_F(F("-> %d available input voltage samples count\r\n"), input_voltage_samples.count);
    TRACE_DEBUG_F(F("-> %d available input current samples count\r\n"), input_current_samples.count);
  }

  // Default value to RMAP Limit error value
  report.avg_battery_charge = RMAPDATA_MAX;
  report.avg_battery_charge_quality = RMAPDATA_MAX;
  report.avg_battery_voltage = RMAPDATA_MAX;
  report.avg_battery_voltage_quality = RMAPDATA_MAX;
  report.avg_battery_current = RMAPDATA_MAX;
  report.avg_battery_current_quality = RMAPDATA_MAX;
  report.avg_input_voltage = RMAPDATA_MAX;
  report.avg_input_voltage_quality = RMAPDATA_MAX;
  report.avg_input_current = RMAPDATA_MAX;
  report.avg_input_current_quality = RMAPDATA_MAX;

  // Ptr for maintenance
  bufferPtrResetBack<maintenance_t, uint16_t>(&maintenance_samples, SAMPLES_COUNT_MAX);
  // Ptr for value sample
  bufferPtrResetBack<sample_t, uint16_t>(&battery_charge_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&battery_voltage_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&battery_current_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&input_voltage_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&input_current_samples, SAMPLES_COUNT_MAX);

  // Align all sensor's data to last common acquired sample (miantenance is always first data add into queue)
  // Align with last maintenance record pointer
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
    bufferReadBack<maintenance_t, uint16_t, rmapdata_t>(&maintenance_samples, SAMPLES_COUNT_MAX);
    bufferReadBack<sample_t, uint16_t, rmapdata_t>(&battery_charge_samples, SAMPLES_COUNT_MAX);
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

  // it's a simple istant or report request?
  if (report_time_s == 0)
  {
    // Make last data value to Get Istant show value
    report.avg_battery_charge = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&battery_charge_samples, SAMPLES_COUNT_MAX);
    report.avg_battery_voltage = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&battery_voltage_samples, SAMPLES_COUNT_MAX);
    report.avg_battery_current = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&battery_current_samples, SAMPLES_COUNT_MAX);
    report.avg_input_voltage = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&input_voltage_samples, SAMPLES_COUNT_MAX);
    report.avg_input_current = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&input_current_samples, SAMPLES_COUNT_MAX);
  }
  else
  {
    // Make a report complete (try with all sample present aligned)
    for (uint16_t i = 0; i < samples_count; i++)
    {
      // End of Sample in calculation (Completed with the request... Exit on Full Buffer ReadBack executed)
      if(n_sample >= report_sample_count) break;
      // Get base operation for any record...
      n_sample++; // Elaborate next sample... (Initzialize with 0, Sample 1 is first. Exit UP if buffer completed)
      // Check if is an observation
      is_observation = (n_sample % sample_for_observation) == 0;
      // Is Maintenance mode? (Excluding measure from elaboration value)
      // Maintenance is sytemic value for all measure (always pushed into module for excuding value with maintenance)
      measures_maintenance = bufferReadBack<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX);

      // ***************************************************************************************************
      // ************* GET SAMPLE VALUE DATA FROM AND CREATE OBSERVATION VALUES FOR TYPE SENSOR ************
      // ***************************************************************************************************
      
      battery_charge_s = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&battery_charge_samples, SAMPLES_COUNT_MAX);
      total_count_battery_charge_s++;
      avg_battery_charge_quality_s += (float)(((float)checkMppt_P_Chg(battery_charge_s) - avg_battery_charge_quality_s) / total_count_battery_charge_s);
      if ((ISVALID_RMAPDATA(battery_charge_s)) && !measures_maintenance)
      {
        valid_count_battery_charge_s++;
        #if TRACE_LEVEL >= TRACE_LEVEL_DEBUG
        valid_count_battery_charge_t++;
        #endif
        avg_battery_charge_s += (float)(((float)battery_charge_s - avg_battery_charge_s) / valid_count_battery_charge_s);
      }

      battery_voltage_s = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&battery_voltage_samples, SAMPLES_COUNT_MAX);
      total_count_battery_voltage_s++;
      avg_battery_voltage_quality_s += (float)(((float)checkMppt_V_Bat(battery_voltage_s) - avg_battery_voltage_quality_s) / total_count_battery_voltage_s);
      if ((ISVALID_RMAPDATA(battery_voltage_s)) && !measures_maintenance)
      {
        valid_count_battery_voltage_s++;
        #if TRACE_LEVEL >= TRACE_LEVEL_DEBUG
        valid_count_battery_voltage_t++;
        #endif
        avg_battery_voltage_s += (float)(((float)battery_voltage_s - avg_battery_voltage_s) / valid_count_battery_voltage_s);
      }

      battery_current_s = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&battery_current_samples, SAMPLES_COUNT_MAX);
      total_count_battery_current_s++;
      avg_battery_current_quality_s += (float)(((float)checkMppt_I_Bat(battery_current_s) - avg_battery_current_quality_s) / total_count_battery_current_s);
      if ((ISVALID_RMAPDATA(battery_current_s)) && !measures_maintenance)
      {
        valid_count_battery_current_s++;
        #if TRACE_LEVEL >= TRACE_LEVEL_DEBUG
        valid_count_battery_current_t++;
        #endif
        avg_battery_current_s += (float)(((float)battery_current_s - avg_battery_current_s) / valid_count_battery_current_s);
      }

      input_voltage_s = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&input_voltage_samples, SAMPLES_COUNT_MAX);
      total_count_input_voltage_s++;
      avg_input_voltage_quality_s += (float)(((float)checkMppt_V_In(input_voltage_s) - avg_input_voltage_quality_s) / total_count_input_voltage_s);
      if ((ISVALID_RMAPDATA(input_voltage_s)) && !measures_maintenance)
      {
        valid_count_input_voltage_s++;
        #if TRACE_LEVEL >= TRACE_LEVEL_DEBUG
        valid_count_input_voltage_t++;
        #endif
        avg_input_voltage_s += (float)(((float)input_voltage_s - avg_input_voltage_s) / valid_count_input_voltage_s);
      }

      input_current_s = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&input_current_samples, SAMPLES_COUNT_MAX);
      total_count_input_current_s++;
      avg_input_current_quality_s += (float)(((float)checkMppt_I_In(input_current_s) - avg_input_current_quality_s) / total_count_input_current_s);
      if ((ISVALID_RMAPDATA(input_current_s)) && !measures_maintenance)
      {
        valid_count_input_current_s++;
        #if TRACE_LEVEL >= TRACE_LEVEL_DEBUG
        valid_count_input_current_t++;
        #endif
        avg_input_current_s += (float)(((float)input_current_s - avg_input_current_s) / valid_count_input_current_s);
      }

      // ***************************************************************************************************
      // ************* ELABORATE OBSERVATION VALUES FOR TYPE SENSOR FOR PREPARE REPORT RESPONSE ************
      // ***************************************************************************************************
      if(is_observation) {

        // battery_charge, sufficient number of valid samples?
        valid_data_calc_perc = (float)(valid_count_battery_charge_s) / (float)(total_count_battery_charge_s) * 100.0;
        if (valid_data_calc_perc >= SAMPLE_ERROR_PERCENTAGE_MIN)
        {
          valid_count_battery_charge_o++;
          avg_battery_charge_o += (avg_battery_charge_s - avg_battery_charge_o) / valid_count_battery_charge_o;
          avg_battery_charge_quality_o += (avg_battery_charge_quality_s - avg_battery_charge_quality_o) / valid_count_battery_charge_o;
        }
        // Reset Buffer sample for calculate next observation
        avg_battery_charge_quality_s = 0;
        avg_battery_charge_s = 0;
        valid_count_battery_charge_s = 0;
        total_count_battery_charge_s = 0;

        // battery_voltage, sufficient number of valid samples?
        valid_data_calc_perc = (float)(valid_count_battery_voltage_s) / (float)(total_count_battery_voltage_s) * 100.0;
        if (valid_data_calc_perc >= SAMPLE_ERROR_PERCENTAGE_MIN)
        {
          valid_count_battery_voltage_o++;
          avg_battery_voltage_o += (avg_battery_voltage_s - avg_battery_voltage_o) / valid_count_battery_voltage_o;
          avg_battery_voltage_quality_o += (avg_battery_voltage_quality_s - avg_battery_voltage_quality_o) / valid_count_battery_voltage_o;
        }
        // Reset Buffer sample for calculate next observation
        avg_battery_voltage_quality_s = 0;
        avg_battery_voltage_s = 0;
        valid_count_battery_voltage_s = 0;
        total_count_battery_voltage_s = 0;

        // battery_current, sufficient number of valid samples?
        valid_data_calc_perc = (float)(valid_count_battery_current_s) / (float)(total_count_battery_current_s) * 100.0;
        if (valid_data_calc_perc >= SAMPLE_ERROR_PERCENTAGE_MIN)
        {
          valid_count_battery_current_o++;
          avg_battery_current_o += (avg_battery_current_s - avg_battery_current_o) / valid_count_battery_current_o;
          avg_battery_current_quality_o += (avg_battery_current_quality_s - avg_battery_current_quality_o) / valid_count_battery_current_o;
        }
        // Reset Buffer sample for calculate next observation
        avg_battery_current_quality_s = 0;
        avg_battery_current_s = 0;
        valid_count_battery_current_s = 0;
        total_count_battery_current_s = 0;

        // input_voltage, sufficient number of valid samples?
        valid_data_calc_perc = (float)(valid_count_input_voltage_s) / (float)(total_count_input_voltage_s) * 100.0;
        if (valid_data_calc_perc >= SAMPLE_ERROR_PERCENTAGE_MIN)
        {
          valid_count_input_voltage_o++;
          avg_input_voltage_o += (avg_input_voltage_s - avg_input_voltage_o) / valid_count_input_voltage_o;
          avg_input_voltage_quality_o += (avg_input_voltage_quality_s - avg_input_voltage_quality_o) / valid_count_input_voltage_o;
        }
        // Reset Buffer sample for calculate next observation
        avg_input_voltage_quality_s = 0;
        avg_input_voltage_s = 0;
        valid_count_input_voltage_s = 0;
        total_count_input_voltage_s = 0;

        // input_current, sufficient number of valid samples?
        valid_data_calc_perc = (float)(valid_count_input_current_s) / (float)(total_count_input_current_s) * 100.0;
        if (valid_data_calc_perc >= SAMPLE_ERROR_PERCENTAGE_MIN)
        {
          valid_count_input_current_o++;
          avg_input_current_o += (avg_input_current_s - avg_input_current_o) / valid_count_input_current_o;
          avg_input_current_quality_o += (avg_input_current_quality_s - avg_input_current_quality_o) / valid_count_input_current_o;
        }
        // Reset Buffer sample for calculate next observation
        avg_input_current_quality_s = 0;
        avg_input_current_s = 0;
        valid_count_input_current_s = 0;
        total_count_input_current_s = 0;

      }
    }

    // ***************************************************************************************************
    // ******* GENERATE REPORT RESPONSE WITH ALL DATA AVAIABLE AND VALID WITH EXPECETD OBSERVATION *******
    // ***************************************************************************************************

    // battery_charge, elaboration final
    valid_data_calc_perc = (float)(valid_count_battery_charge_o) / (float)(observation_sample_count) * 100.0;
    TRACE_DEBUG_F(F("-> %d battery charge sample error (%d%%)\r\n"), (n_sample - valid_count_battery_charge_t), (uint8_t)(((float)n_sample - (float)valid_count_battery_charge_t)/(float)n_sample * 100.0));
    TRACE_DEBUG_F(F("-> %d battery charge observation avaiable (%d%%)\r\n"), valid_count_battery_charge_o, (uint8_t)valid_data_calc_perc);
    if (valid_data_calc_perc >= OBSERVATION_ERROR_PERCENTAGE_MIN)
    {
      report.avg_battery_charge = (rmapdata_t)avg_battery_charge_o;
      report.avg_battery_charge_quality = (rmapdata_t)avg_battery_charge_quality_o;
    }

    // battery_voltage, elaboration final
    valid_data_calc_perc = (float)(valid_count_battery_voltage_o) / (float)(observation_sample_count) * 100.0;
    TRACE_DEBUG_F(F("-> %d battery voltage error (%d%%)\r\n"), (n_sample - valid_count_battery_voltage_t), (uint8_t)(((float)n_sample - (float)valid_count_battery_voltage_t)/(float)n_sample * 100.0));
    TRACE_DEBUG_F(F("-> %d battery voltage observation avaiable (%d%%)\r\n"), valid_count_battery_voltage_o, (uint8_t)valid_data_calc_perc);
    if (valid_data_calc_perc >= OBSERVATION_ERROR_PERCENTAGE_MIN)
    {
      report.avg_battery_voltage = (rmapdata_t)avg_battery_voltage_o;
      report.avg_battery_voltage_quality = (rmapdata_t)avg_battery_voltage_quality_o;
    }

    // battery_current, elaboration final
    valid_data_calc_perc = (float)(valid_count_battery_current_o) / (float)(observation_sample_count) * 100.0;
    TRACE_DEBUG_F(F("-> %d battery current error (%d%%)\r\n"), (n_sample - valid_count_battery_current_t), (uint8_t)(((float)n_sample - (float)valid_count_battery_current_t)/(float)n_sample * 100.0));
    TRACE_DEBUG_F(F("-> %d battery current observation avaiable (%d%%)\r\n"), valid_count_battery_current_o, (uint8_t)valid_data_calc_perc);
    if (valid_data_calc_perc >= OBSERVATION_ERROR_PERCENTAGE_MIN)
    {
      report.avg_battery_current = (rmapdata_t)avg_battery_current_o;
      report.avg_battery_current_quality = (rmapdata_t)avg_battery_current_quality_o;
    }

    // input_voltage, elaboration final
    valid_data_calc_perc = (float)(valid_count_input_voltage_o) / (float)(observation_sample_count) * 100.0;
    TRACE_DEBUG_F(F("-> %d input voltage error (%d%%)\r\n"), (n_sample - valid_count_input_voltage_t), (uint8_t)(((float)n_sample - (float)valid_count_input_voltage_t)/(float)n_sample * 100.0));
    TRACE_DEBUG_F(F("-> %d input voltage observation avaiable (%d%%)\r\n"), valid_count_input_voltage_o, (uint8_t)valid_data_calc_perc);
    if (valid_data_calc_perc >= OBSERVATION_ERROR_PERCENTAGE_MIN)
    {
      report.avg_input_voltage = (rmapdata_t)avg_input_voltage_o;
      report.avg_input_voltage_quality = (rmapdata_t)avg_input_voltage_quality_o;
    }

    // input_current, elaboration final
    valid_data_calc_perc = (float)(valid_count_input_current_o) / (float)(observation_sample_count) * 100.0;
    TRACE_DEBUG_F(F("-> %d input current error (%d%%)\r\n"), (n_sample - valid_count_input_current_t), (uint8_t)(((float)n_sample - (float)valid_count_input_current_t)/(float)n_sample * 100.0));
    TRACE_DEBUG_F(F("-> %d input current observation avaiable (%d%%)\r\n"), valid_count_input_current_o, (uint8_t)valid_data_calc_perc);
    if (valid_data_calc_perc >= OBSERVATION_ERROR_PERCENTAGE_MIN)
    {
      report.avg_input_current = (rmapdata_t)avg_input_current_o;
      report.avg_input_current_quality = (rmapdata_t)avg_input_current_quality_o;
    }
  }
  // Trace report final (Istant or Elaborate)
  TRACE_INFO_F(F("--> battery charge\t%d\t%d\r\n"), (int32_t)report.avg_battery_charge, (int32_t)report.avg_battery_charge_quality);
  TRACE_INFO_F(F("--> battery voltage\t%d\t%d\r\n"), (int32_t)report.avg_battery_voltage, (int32_t)report.avg_battery_voltage_quality);
  TRACE_INFO_F(F("--> battery current\t%d\t%d\r\n"), (int32_t)report.avg_battery_current, (int32_t)report.avg_battery_current_quality);
  TRACE_INFO_F(F("--> input voltage\t%d\t%d\r\n"), (int32_t)report.avg_input_voltage, (int32_t)report.avg_input_voltage_quality);
  TRACE_INFO_F(F("--> input current\t%d\t%d\r\n"), (int32_t)report.avg_input_current, (int32_t)report.avg_input_current_quality);
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