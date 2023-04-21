/**
  ******************************************************************************
  * @file    elaborate_data_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @author  Moreno Gasperini <m.baldinetti@digiteco.it>
  * @brief   elaborate_data_task source file (Elaborate acquire WindGill)
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

  bufferReset<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX);
  bufferReset<sample_t, uint16_t, rmapdata_t>(&wind_speed_samples, SAMPLES_COUNT_MAX);
  bufferReset<sample_t, uint16_t, rmapdata_t>(&wind_direction_samples, SAMPLES_COUNT_MAX);

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
        case WIND_SPEED_INDEX:
          addValue<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX, param.system_status->flags.is_maintenance);
          addValue<sample_t, uint16_t, rmapdata_t>(&wind_speed_samples, SAMPLES_COUNT_MAX, edata.value);
          TRACE_VERBOSE_F(F("Speed: %d\r\n"), edata.value);
          break;

        case WIND_DIRECTION_INDEX:
          addValue<sample_t, uint16_t, rmapdata_t>(&wind_direction_samples, SAMPLES_COUNT_MAX, edata.value);
          TRACE_VERBOSE_F(F("Direction: %d\r\n"), edata.value);
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
uint8_t ElaborateDataTask::checkWindQuality(float speed, float direction) {
  // Optional check quality data function
  float check_speed;
  float check_direction;
  uint8_t quality = 0;

  // Spike on Velocity > 10 m/sec on near acquire (1 samp / 1 sec.)
  check_speed = fabs(speed - speed_last);
  if(check_speed < 10) quality+= 50;
  else quality += check_speed - 50;
  // Check difference (90°) between 2 near acquire (1 samp / 1 sec.)
  check_direction = fabs(direction - direction_last);
  if((check_direction > 180.0)) check_direction = 360.0 - check_direction;
  if(check_direction < 90) quality+= 50;
  else quality += (0.55556 * (check_direction - 90.0));

  // Backup value
  speed_last = speed;
  direction_last = direction;

  return quality;
}

void ElaborateDataTask::getSDFromUV(float u, float v, float *speed, float *direction)
{
  *speed = sqrt(u * u + v * v);
  *direction = RAD_TO_DEG * atan2(-u, -v);
  *direction = round(*direction);
  *direction = *direction % 360;

  if (*speed == 0)
  {
    *direction = WIND_DIRECTION_MAX;
  }
}

void ElaborateDataTask::make_report(bool is_init, uint16_t report_time_s, uint8_t observation_time_s)
{
  // Generic and shared var
  bool measures_maintenance = false;  // Maintenance mode?
  float valid_data_calc_perc;         // Shared calculate valid % of measure (Data Valid or not?)
  bool is_observation = false;        // Is an observation (when calculate is requested)
  uint16_t n_sample = 0;              // Sample elaboration number... (incremented on calc development)

  float speed = 0;
  float direction = 0;

  uint16_t valid_count_a = 0;
  uint16_t error_count_a = 0;
  float error_a_per = 0;

  uint16_t valid_count_b = 0;
  uint16_t error_count_b = 0;
  float error_b_per = 0;

  uint16_t valid_count_a_o = 0;
  uint16_t error_count_a_o = 0;
  float error_a_o_per = 0;

  uint16_t valid_count_b_o = 0;
  uint16_t error_count_b_o = 0;
  float error_b_o_per = 0;

  uint16_t valid_count_speed = 0;
  uint16_t error_count_speed = 0;
  float error_count_speed_per = 0;

  float ua = 0;
  float va = 0;

  float ub = 0;
  float vb = 0;

  float ua_o = 0;
  float va_o = 0;

  float ub_o = 0;
  float vb_o = 0;

  float vavg10_speed = 0;
  float vavg10_direction = 0;

  float vavg_speed = 0;
  float vavg_direction = 0;

  float peak_gust_speed = -1.0;
  float peak_gust_direction = 0;

  float vavg_speed_o = -1.0;
  float vavg_direction_o = 0;

  float long_gust_speed = -1.0;
  float long_gust_direction = 0;

  float avg_speed = 0;

  float class_1 = 0;
  float class_2 = 0;
  float class_3 = 0;
  float class_4 = 0;
  float class_5 = 0;
  float class_6 = 0;

  float avg_quality = 0;

  // Elaboration timings calculation
  uint16_t report_sample_count = round((report_time_s * 1.0) / (param.configuration->sensor_acquisition_delay_ms / 1000.0));
  uint16_t wmo_report_sample_count = round((600 * 1.0) / (param.configuration->sensor_acquisition_delay_ms / 1000.0));
  uint16_t observation_sample_count = round((observation_time_s * 1.0) / (param.configuration->sensor_acquisition_delay_ms / 1000.0));
  uint16_t report_observations_count = 0;
  if(report_time_s && observation_sample_count) report_observations_count = report_sample_count / observation_sample_count;

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
    TRACE_DEBUG_F(F("-> %d samples counts need for WMO avg 10 min.\r\n"), wmo_report_sample_count);
    TRACE_DEBUG_F(F("-> %d samples counts need for observation\r\n"), observation_sample_count);
    TRACE_DEBUG_F(F("-> %d observation counts need for report\r\n"), report_observations_count);
    TRACE_DEBUG_F(F("-> %d available wind speed samples count\r\n"), wind_speed_samples.count);
    TRACE_DEBUG_F(F("-> %d available wind direction samples count\r\n"), wind_direction_samples.count);
  }

  // reset report buffer
  report.vavg10_speed = FLT_MAX;
  report.vavg10_direction = FLT_MAX;
  report.vavg_speed = FLT_MAX;
  report.vavg_direction = FLT_MAX;
  report.peak_gust_speed = FLT_MAX;
  report.long_gust_speed = FLT_MAX;
  report.avg_speed = FLT_MAX;
  report.class_1 = FLT_MAX;
  report.class_2 = FLT_MAX;
  report.class_3 = FLT_MAX;
  report.class_4 = FLT_MAX;
  report.class_5 = FLT_MAX;
  report.class_6 = FLT_MAX;
  report.quality = FLT_MAX;

  // Ptr for maintenance
  bufferPtrResetBack<maintenance_t, uint16_t>(&maintenance_samples, SAMPLES_COUNT_MAX);
  // Ptr for value sample
  bufferPtrResetBack<sample_t, uint16_t>(&wind_speed_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&wind_direction_samples, SAMPLES_COUNT_MAX);

  // align all sensor's data to last common acquired sample
  uint16_t samples_count = wind_speed_samples.count;
  if (wind_direction_samples.count < samples_count)
  {
    samples_count = wind_direction_samples.count;
  }

  // flush all data that is not aligned
  for (uint16_t i = samples_count; i < wind_speed_samples.count; i++)
  {
    bufferReadBack<maintenance_t, uint16_t, rmapdata_t>(&maintenance_samples, SAMPLES_COUNT_MAX);
    bufferReadBack<sample_t, uint16_t, rmapdata_t>(&wind_speed_samples, SAMPLES_COUNT_MAX);
  }
  for (uint16_t i = samples_count; i < wind_direction_samples.count; i++)
  {
    bufferReadBack<sample_t, uint16_t, rmapdata_t>(&wind_direction_samples, SAMPLES_COUNT_MAX);
  }

  // it's a simple istant or report request?
  if (report_time_s == 0) {
    // Make last data value to Get Istant show value
    speed = (float)bufferReadBack<sample_t, uint16_t, rmapdata_t>(&wind_speed_samples, SAMPLES_COUNT_MAX);
    if (!ISVALID_FLOAT(speed)) speed = 0;
    direction = (float)bufferReadBack<sample_t, uint16_t, rmapdata_t>(&wind_direction_samples, SAMPLES_COUNT_MAX);
    if (!ISVALID_FLOAT(direction)) direction = 0;
    // Used as sample for istant value (Only LCD for show value)
    report.vavg10_speed = speed;
    report.vavg10_direction = direction;
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
      is_observation = (n_sample % observation_sample_count) == 0;
      // Is Maintenance mode? (Excluding measure from elaboration value)
      // Maintenance is sytemic value for all measure (always pushed into module for excuding value with maintenance)
      measures_maintenance = bufferReadBack<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX);

      // ***************************************************************************************************
      // ************* GET SAMPLE VALUE DATA FROM AND CREATE OBSERVATION VALUES FOR TYPE SENSOR ************
      // ***************************************************************************************************

      speed = (float)bufferReadBack<sample_t, uint16_t, rmapdata_t>(&wind_speed_samples, SAMPLES_COUNT_MAX);
      speed /= WIND_CASTING_SPEED_MULT;
      if (speed < CALM_WIND_MAX_MS) speed = WIND_SPEED_MIN;

      direction = (float)bufferReadBack<sample_t, uint16_t, rmapdata_t>(&wind_direction_samples, SAMPLES_COUNT_MAX);
      direction /= WIND_CASTING_DIRECTION_MULT;
      if (speed < CALM_WIND_MAX_MS) direction = WIND_DIRECTION_MAX;

      // last sample
      if (i == 0)
      {
        TRACE_DEBUG_F(F("Elaborate: last sample [count, data (spd dir)] %u\t%u\t%.2f\t%.0f\t"), wind_speed_samples.count, wind_direction_samples.count, speed, direction);
      }

      // Calculate quality
      avg_quality += ((checkWindQuality(speed, direction) - avg_quality) / n_sample);

      // calc report on 10'
      if (i < wmo_report_sample_count)
      {
        if ((ISVALID_FLOAT(speed) && ISVALID_FLOAT(direction)) && !measures_maintenance)
        {
          valid_count_a++;
          ua += ((float)(-speed * sin(DEG_TO_RAD * direction)) - ua) / valid_count_a;
          va += ((float)(-speed * cos(DEG_TO_RAD * direction)) - va) / valid_count_a;
        }
        else
        {
          error_count_a++;
        }
      }

      // calc report
      if ((ISVALID_FLOAT(speed) && ISVALID_FLOAT(direction)) && !measures_maintenance)
      {
        valid_count_b++;

        ub += ((float)(-speed * sin(DEG_TO_RAD * direction)) - ub) / valid_count_b;
        vb += ((float)(-speed * cos(DEG_TO_RAD * direction)) - vb) / valid_count_b;

        if (speed >= peak_gust_speed)
        {
          peak_gust_speed = speed;
          peak_gust_direction = direction;
        }
      }
      else
      {
        error_count_b++;
      }

      if (ISVALID_FLOAT(speed) && !measures_maintenance)
      {
        valid_count_speed++;
        avg_speed += (speed - avg_speed) / valid_count_speed;

        if (speed < WIND_CLASS_1_MAX)
        {
          class_1++;
        }
        else if (speed < WIND_CLASS_2_MAX)
        {
          class_2++;
        }
        else if (speed < WIND_CLASS_3_MAX)
        {
          class_3++;
        }
        else if (speed < WIND_CLASS_4_MAX)
        {
          class_4++;
        }
        else if (speed < WIND_CLASS_5_MAX)
        {
          class_5++;
        }
        else
        {
          class_6++;
        }
      }
      else
      {
        error_count_speed++;
      }

      // ***************************************************************************************************
      // ************* ELABORATE OBSERVATION VALUES FOR TYPE SENSOR FOR PREPARE REPORT RESPONSE ************
      // ***************************************************************************************************
      if (is_observation)
      {
        // calc report on 10'
        if (i < wmo_report_sample_count)
        {
          error_a_per = (float)(error_count_a) / (float)(samples_count)*100.0;

          if (valid_count_a && (error_a_per <= SAMPLE_ERROR_PERCENTAGE_MAX))
          {
            valid_count_a_o++;

            ua_b += (ua - ua_b) / valid_count_a_o;
            va_b += (va - va_b) / valid_count_a_o;
          }
          else
          {
            error_count_a_o++;
          }
        }

        error_b_per = (float)(error_count_b) / (float)(samples_count)*100.0;

        if (valid_count_b && (error_b_per <= SAMPLE_ERROR_PERCENTAGE_MAX))
        {
          valid_count_b_o++;

          ub_b += (ub - ub_b) / valid_count_b_o;
          vb_b += (vb - vb_b) / valid_count_b_o;

          // raffica su 1 minuto (su una osservazione)
          getSDFromUV(ub, vb, &vavg_speed_o, &vavg_direction_o);
          if (vavg_speed_o >= long_gust_speed)
          {
            long_gust_speed = vavg_speed_o;
            long_gust_direction = vavg_direction_o;
          }
        }
        else
        {
          error_count_b_o++;
        }

        ua = 0;
        va = 0;
        ub = 0;
        vb = 0;
        vavg_speed_o = 0;
        vavg_direction_o = 0;
        valid_count_a = 0;
        error_count_a = 0;
        valid_count_b = 0;
        error_count_b = 0;
      }
    }

    error_a_o_per = (float)(error_count_o_a) / (float)(report_observations_count)*100.0;
    error_b_o_per = (float)(error_count_o_b) / (float)(report_observations_count)*100.0;
    error_count_speed_per = (float)(error_count_speed) / (float)(samples_count)*100.0;

    TRACE_DEBUG_F(F("-> %d samples error on wmo report (%d%%)\r\n"), error_count_a, (int32_t)error_a_per);
    TRACE_DEBUG_F(F("-> %d samples error on report (%d%%)\r\n"), error_count_b, (int32_t)error_b_per);
    TRACE_DEBUG_F(F("-> %d samples error on long gust (%d%%)\r\n"), error_count_o, (int32_t)error_c_per);
    TRACE_DEBUG_F(F("-> %d samples error on class (%d%%)\r\n"), error_count_speed, (int32_t)error_count_speed_per);

    if (!measures_maintenance && valid_count_a && (error_a_per >= SAMPLE_ERROR_PERCENTAGE_MIN))
    {
      getSDFromUV(ua_o, va_o, &vavg10_speed, &vavg10_direction);
      report.vavg10_speed = vavg10_speed;
      report.vavg10_direction = round(vavg10_direction);
      report.quality = avg_quality;
    }

    if (!measures_maintenance && valid_count_b && (error_b_per >= SAMPLE_ERROR_PERCENTAGE_MIN))
    {
      getSDFromUV(ub_o, vb_o, &vavg_speed_o, &vavg_direction_o);
      report.vavg_speed = vavg_speed;
      report.vavg_direction = round(vavg_direction);
      report.peak_gust_speed = peak_gust_speed;
      report.peak_gust_direction = round(peak_gust_direction);
    }

    if (!measures_maintenance && valid_count_o && (error_o_per >= SAMPLE_ERROR_PERCENTAGE_MIN))
    {
      report.long_gust_speed = long_gust_speed;
      report.long_gust_direction = round(long_gust_direction);
    }

    if (!measures_maintenance && error_count_speed && (error_count_speed_per >= SAMPLE_ERROR_PERCENTAGE_MIN))
    {
      class_1 = calcFrequencyPercent(class_1, valid_count_speed);
      class_2 = calcFrequencyPercent(class_2, valid_count_speed);
      class_3 = calcFrequencyPercent(class_3, valid_count_speed);
      class_4 = calcFrequencyPercent(class_4, valid_count_speed);
      class_5 = calcFrequencyPercent(class_5, valid_count_speed);
      class_6 = calcFrequencyPercent(class_6, valid_count_speed);

      report.avg_speed = avg_speed;
      report.quality = avg_quality;
      report.class_1 = round(class_1);
      report.class_2 = round(class_2);
      report.class_3 = round(class_3);
      report.class_4 = round(class_4);
      report.class_5 = round(class_5);
      report.class_6 = round(class_6);
    }
    // TRACE_DEBUG_F(F("%.3f\t%.3f\t%.2f\t%.0f\t%.3f\t%.3f\t%.2f\t%.0f\t%.2f\t%.2f\t%.0f\t%.2f\t%.0f\t%.0f\t%.0f\t%.0f\t%.0f\t%.0f\t%.0f\r\n"), ua, va, report.vavg10_speed, report.vavg10_direction, ub, vb, report.vavg_speed, report.vavg_direction, report.avg_speed, report.peak_gust_speed, report.peak_gust_direction, report.long_gust_speed, report.long_gust_direction, report.class_1, report.class_2, report.class_3, report.class_4, report.class_5, report.class_6);
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
