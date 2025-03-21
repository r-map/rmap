/**
  ******************************************************************************
  * @file    elaborate_data_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @author  Moreno Gasperini <m.baldinetti@digiteco.it>
  * @brief   elaborate_data_task source file (Elaborate acquire WindGill)
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

/// @brief Construct the Elaborate Data Task::ElaborateDataTask object
/// @param taskName name of the task
/// @param stackSize size of the stack
/// @param priority priority of the task
/// @param elaborateDataParam parameters for the task
ElaborateDataTask::ElaborateDataTask(const char *taskName, uint16_t stackSize, uint8_t priority, ElaborateDataParam_t elaborateDataParam) : Thread(taskName, stackSize, priority), param(elaborateDataParam)
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

/// @brief RUN Task
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

  // Data Simulator
  #if defined(USE_SIMULATOR) && defined(INIT_SIMULATOR)
  int simulate_dir = 0;
  for(uint16_t iInit=0; iInit<900; iInit++) {
      addValue<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX, false);
      addValue<sample_t, uint16_t, rmapdata_t>(&wind_speed_samples, SAMPLES_COUNT_MAX, (rmapdata_t)200);
      simulate_dir++;
      simulate_dir%=360;
      addValue<sample_t, uint16_t, rmapdata_t>(&wind_direction_samples, SAMPLES_COUNT_MAX, (rmapdata_t)simulate_dir);
  }
  // Test diretto
  #ifdef VECT_MED_ON_360_SIMULATOR
  request_data.report_time_s = 360;
  #else
  request_data.report_time_s = 900;
  #endif
  request_data.observation_time_s = 60;
  request_data.is_init = true;
  make_report(request_data.is_init, request_data.report_time_s, request_data.observation_time_s);
  #endif

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
    if (!param.elaborateDataQueue->IsEmpty()) {
      if (param.elaborateDataQueue->Peek(&edata, 0))
      {
        param.elaborateDataQueue->Dequeue(&edata);
        switch (edata.index)
        {
        case WIND_SPEED_INDEX:
          // Data Simulator
          #ifdef USE_SIMULATOR
          edata.value = 200;
          #endif
          addValue<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX, param.system_status->flags.is_maintenance);
          addValue<sample_t, uint16_t, rmapdata_t>(&wind_speed_samples, SAMPLES_COUNT_MAX, edata.value);
          TRACE_VERBOSE_F(F("Speed: %d\r\n"), edata.value);
          break;

        case WIND_DIRECTION_INDEX:
          // Data Simulator
          #ifdef USE_SIMULATOR
          edata.value = simulate_dir;
          simulate_dir++;
          simulate_dir%=360;
          #endif
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
        param.requestDataQueue->Dequeue(&request_data);
        make_report(request_data.is_init, request_data.report_time_s, request_data.observation_time_s);
        param.reportDataQueue->Enqueue(&report);
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
/// @param speed wind speedreal value readed from sensor
/// @param direction wind direction real value readed from sensor
/// @return value uint_8 percent data quality value
uint8_t ElaborateDataTask::checkWindQuality(float speed, float direction) {
  // Optional check quality data function
  float check_speed;
  float check_direction;
  uint8_t quality = 0;

  // Spike on Velocity > 10 m/sec on near acquire (1 samp / 1 sec.)
  check_speed = fabs(speed - speed_last);
  if(check_speed < 10) quality += 50;
  else quality += check_speed - 50;
  // Check difference (90°) between 2 near acquire (1 samp / 1 sec.)
  check_direction = fabs(direction - direction_last);
  if((check_direction > 180.0)) check_direction = 360.0 - check_direction;
  if(check_direction < 90) quality += 50;
  else quality += (0.55556 * (check_direction - 90.0));

  // Limit control
  if(quality < 0) quality = 0;
  if(quality > 100) quality = 100;

  // Backup value
  speed_last = speed;
  direction_last = direction;

  return quality;
}

/// @brief Get speed and direction from vector array
/// @param u vector u
/// @param v vector v
/// @param speed return speed calculate
/// @param direction return direction calculate
void ElaborateDataTask::getSDFromUV(float u, float v, float *speed, float *direction)
{
  // Check limit for ATAN2 function near 0. Undefined result on 0
  if ((abs(u) < ATAN2_CHECK_LIMIT) && (abs(v) < ATAN2_CHECK_LIMIT)) {
    *speed = 0;
    *direction = 0;
  } else {
    *speed = sqrt(u * u + v * v);
    *direction = RAD_TO_DEG * atan2(-u, -v);
    int16_t tmp_dir = round(*direction);
    if(tmp_dir < 0) tmp_dir += 360;
    *direction = tmp_dir % 360;
    if(*speed < CALM_WIND_MAX_MED_VECT) {
      *speed = 0;
      *direction = 0;
    } else if (*direction == 0) {
      *direction = WIND_DIRECTION_MAX; // Translate 0 -> 360 (WIND_DIRECTION_MAX)
    }
  }
}

/// @brief Create a report from buffered sample
/// @param is_init Bool optional backup ptr_wr calc
/// @param report_time_s time of report
/// @param observation_time_s time to make an observation
void ElaborateDataTask::make_report(bool is_init, uint16_t report_time_s, uint8_t observation_time_s)
{
  // Generic and shared var
  bool measures_maintenance = false;  // Maintenance mode?
  bool is_observation = false;        // Is an observation (when calculate is requested)
  uint16_t n_sample = 0;              // Sample elaboration number... (incremented on calc development)

  bool is_valid_speed;
  bool is_valid_direction;

  float speed = 0;
  float direction = 0;

  uint16_t valid_count_a = 0;
  uint16_t total_count_a = 0;
  float valid_a_per = 0;

  uint16_t valid_count_b = 0;
  uint16_t total_count_b = 0;
  float valid_b_per = 0;

  uint16_t valid_count_b_o = 0;
  uint16_t total_count_b_o = 0;
  float valid_b_o_per = 0;

  uint16_t valid_count_speed = 0;
  uint16_t total_count_speed = 0;
  float valid_count_speed_per = 0;

  float ua = 0;
  float va = 0;

  float ub = 0;
  float vb = 0;

  float ub_o = 0;
  float vb_o = 0;

  float vavg10_speed = 0;
  float vavg10_direction = 0;

  float vavg_speed = 0;
  float vavg_direction = 0;

  float peak_gust_speed = -1;
  float peak_gust_direction = 0;

  float vavg_speed_o = 0;
  float vavg_direction_o = 0;

  float long_gust_speed = -1;
  float long_gust_direction = 0;

  float avg_speed = 0;

  uint16_t class_1 = 0;
  uint16_t class_2 = 0;
  uint16_t class_3 = 0;
  uint16_t class_4 = 0;
  uint16_t class_5 = 0;
  uint16_t class_6 = 0;

  float avg_quality = 0;
  uint16_t n_sample_quality = 0;

  // Request to init counter parameter (checking error)
  if(is_init) {
    param.systemStatusLock->Take();
    param.system_status->events.perc_rs232_error = 0;
    param.system_status->events.error_count = 0;
    param.system_status->events.measure_count = 0;
    param.systemStatusLock->Give();
  }

  // Elaboration timings calculation
  uint16_t report_sample_count = round((report_time_s * 1.0) / (param.configuration->sensor_acquisition_delay_ms / 1000.0));
  uint16_t wmo_report_sample_count = round((600 * 1.0) / (param.configuration->sensor_acquisition_delay_ms / 1000.0));
  uint16_t observation_sample_count = round((observation_time_s * 1.0) / (param.configuration->sensor_acquisition_delay_ms / 1000.0));
  uint16_t wmo_report_observations_count = wmo_report_sample_count / observation_sample_count;
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
    TRACE_DEBUG_F(F("-> %d WMO observation counts need for report\r\n"), wmo_report_observations_count);
    TRACE_DEBUG_F(F("-> %d available wind speed samples count\r\n"), wind_speed_samples.count);
    TRACE_DEBUG_F(F("-> %d available wind direction samples count\r\n"), wind_direction_samples.count);
  }

  // reset report buffer
  report.vavg10_speed = RMAPDATA_MAX;
  report.vavg10_direction = RMAPDATA_MAX;
  report.vavg_speed = RMAPDATA_MAX;
  report.vavg_direction = RMAPDATA_MAX;
  report.peak_gust_speed = RMAPDATA_MAX;
  report.long_gust_speed = RMAPDATA_MAX;
  report.avg_speed = RMAPDATA_MAX;
  report.class_1 = RMAPDATA_MAX;
  report.class_2 = RMAPDATA_MAX;
  report.class_3 = RMAPDATA_MAX;
  report.class_4 = RMAPDATA_MAX;
  report.class_5 = RMAPDATA_MAX;
  report.class_6 = RMAPDATA_MAX;
  report.quality = RMAPDATA_MAX;

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
    bufferReadBack<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX);
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
    if(speed >= RMAPDATA_MAX) speed = 0;
    direction = (float)bufferReadBack<sample_t, uint16_t, rmapdata_t>(&wind_direction_samples, SAMPLES_COUNT_MAX);
    if (direction >= RMAPDATA_MAX) direction = 0;
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

      is_valid_speed = false;
      is_valid_direction = false;

      // Casting value x10
      speed = (float)bufferReadBack<sample_t, uint16_t, rmapdata_t>(&wind_speed_samples, SAMPLES_COUNT_MAX);
      if (speed < RMAPDATA_MAX) {
        is_valid_speed = true;
        speed /= WIND_CASTING_SPEED_MULT_ACQUIRE;
      }
      if (speed < CALM_WIND_MAX_MS) speed = MIN_VALID_WIND_SPEED;

      // No casting value (real data)
      direction = (float)bufferReadBack<sample_t, uint16_t, rmapdata_t>(&wind_direction_samples, SAMPLES_COUNT_MAX);
      if(direction < RMAPDATA_MAX) is_valid_direction = true;
      if (speed < CALM_WIND_MAX_MS) direction = MIN_VALID_WIND_DIRECTION;

      // Calculate quality
      if(is_valid_speed && is_valid_direction) {
        n_sample_quality++;
        avg_quality += ((checkWindQuality(speed, direction) - avg_quality) / n_sample_quality);
      }

      // calc report on last 10'
      if (n_sample <= wmo_report_sample_count) {
        total_count_a++;
        if (is_valid_speed && is_valid_direction && !measures_maintenance) {
          valid_count_a++;
          ua += ((float)(-speed * sin(DEG_TO_RAD * direction)) - ua) / valid_count_a;
          va += ((float)(-speed * cos(DEG_TO_RAD * direction)) - va) / valid_count_a;
        }
      }

      // calc report
      total_count_b++;
      total_count_b_o++;
      if (is_valid_speed && is_valid_direction && !measures_maintenance) {
        // Total
        valid_count_b++;
        valid_count_b_o++;
        ub += ((float)(-speed * sin(DEG_TO_RAD * direction)) - ub) / valid_count_b;
        vb += ((float)(-speed * cos(DEG_TO_RAD * direction)) - vb) / valid_count_b;
        // On observation
        ub_o += ((float)(-speed * sin(DEG_TO_RAD * direction)) - ub_o) / valid_count_b_o;
        vb_o += ((float)(-speed * cos(DEG_TO_RAD * direction)) - vb_o) / valid_count_b_o;
      }

      #ifdef VECT_MED_ON_360_SIMULATOR
      // Test on avg vet on circle angle 0..359
      float vspd, vdir;
      getSDFromUV(ua, va, &vspd, &vdir);
      vspd *= 100;
      Serial.print((int)(vspd));
      Serial.print(" - ");
      Serial.println((int)(vdir));
      #endif

      total_count_speed++;
      if (is_valid_speed && is_valid_direction && !measures_maintenance) {

        valid_count_speed++;
        avg_speed += (speed - avg_speed) / valid_count_speed;

        if (speed > peak_gust_speed) {
          peak_gust_speed = speed;
          peak_gust_direction = direction;
          if (peak_gust_speed < CALM_WIND_MAX_MS) {
            peak_gust_speed = 0;
            peak_gust_direction = 0;
          } else {
            if (peak_gust_direction == 0) peak_gust_direction = WIND_DIRECTION_MAX; // Translate 0 -> 360 (WIND_DIRECTION_MAX)
          }
        }

        if (speed < WIND_CLASS_1_MAX) {
          class_1++;
        } else if (speed < WIND_CLASS_2_MAX) {
          class_2++;
        } else if (speed < WIND_CLASS_3_MAX) {
          class_3++;
        } else if (speed < WIND_CLASS_4_MAX) {
          class_4++;
        } else if (speed < WIND_CLASS_5_MAX) {
          class_5++;
        } else {
          class_6++;
        }
      }

      // ***************************************************************************************************
      // ************* ELABORATE OBSERVATION VALUES FOR TYPE SENSOR FOR PREPARE REPORT RESPONSE ************
      // ***************************************************************************************************
      if (is_observation) {
        valid_b_o_per = (float)(valid_count_b_o) / (float)(total_count_b_o) * 100.0;
        if (valid_b_o_per >= SAMPLE_ERROR_PERCENTAGE_MIN) {
          // raffica su una osservazione, 1 minuto
          getSDFromUV(ub_o, vb_o, &vavg_speed_o, &vavg_direction_o);
          if (vavg_speed_o > long_gust_speed)
          {
            long_gust_speed = vavg_speed_o;
            long_gust_direction = vavg_direction_o;
            if (long_gust_speed < CALM_WIND_MAX_MS) {
              long_gust_speed = 0;
              long_gust_direction = 0;
            } else {
              if (long_gust_direction == 0) long_gust_direction = WIND_DIRECTION_MAX; // Translate 0 -> 360 (WIND_DIRECTION_MAX)
            }
          }
        }
        // Reset index o
        ub_o = 0;
        vb_o = 0;
        valid_count_b_o = 0;
        total_count_b_o = 0;
      }
    }

    valid_a_per = (float)(valid_count_a) / (float)(wmo_report_sample_count) * 100.0;
    valid_b_per = (float)(valid_count_b) / (float)(report_sample_count) * 100.0;
    valid_count_speed_per = (float)(total_count_speed) / (float)(report_sample_count) * 100.0;

    TRACE_DEBUG_F(F("-> %d Wmo observation avaiable (%d%%)\r\n"), valid_count_a, (uint8_t)valid_a_per);
    TRACE_DEBUG_F(F("-> %d samples error on wmo report (%d%%)\r\n"), (total_count_a - valid_count_a), (uint8_t)(((float)total_count_a - (float)valid_count_a) / (float)total_count_a * 100.0));
    TRACE_DEBUG_F(F("-> %d Report observation avaiable\r\n"), valid_count_b_o);
    TRACE_DEBUG_F(F("-> %d samples error on report (%d%%)\r\n"), (total_count_b - valid_count_b), (uint8_t)(((float)total_count_b - (float)valid_count_b) / (float)total_count_b * 100.0));
    TRACE_DEBUG_F(F("-> %d Speed and class observation avaiable (%d%%)\r\n"), total_count_speed, (uint8_t)valid_count_speed_per);
    TRACE_DEBUG_F(F("-> %d samples error on speed and class (%d%%)\r\n"), (n_sample - total_count_speed), (uint8_t)(((float)n_sample - (float)total_count_speed) / (float)n_sample * 100.0));

    if (valid_a_per >= OBSERVATION_ERROR_PERCENTAGE_MIN) {
      getSDFromUV(ua, va, &vavg10_speed, &vavg10_direction);
      report.vavg10_speed = (rmapdata_t) (vavg10_speed * WIND_CASTING_SPEED_MULT);
      report.vavg10_direction = (rmapdata_t) round(vavg10_direction);
    }

    if (valid_b_per >= OBSERVATION_ERROR_PERCENTAGE_MIN) {
      getSDFromUV(ub, vb, &vavg_speed, &vavg_direction);
      report.vavg_speed = (rmapdata_t) (vavg_speed * WIND_CASTING_SPEED_MULT);
      report.vavg_direction = (rmapdata_t) round(vavg_direction);

      report.peak_gust_speed = (rmapdata_t) (peak_gust_speed * WIND_CASTING_SPEED_MULT);
      report.peak_gust_direction = (rmapdata_t) round(peak_gust_direction);

      report.long_gust_speed = (rmapdata_t) long_gust_speed * WIND_CASTING_SPEED_MULT;
      report.long_gust_direction = (rmapdata_t) round(long_gust_direction);
    }

    if (valid_count_speed_per >= SAMPLE_ERROR_PERCENTAGE_MIN) {
      report.avg_speed = (rmapdata_t) avg_speed * WIND_CASTING_SPEED_MULT;
      report.quality = (rmapdata_t) round(avg_quality);      
      report.class_1 = (rmapdata_t) round((float)class_1 / (float)valid_count_speed * 100.0);
      report.class_2 = (rmapdata_t) round((float)class_2 / (float)valid_count_speed * 100.0);
      report.class_3 = (rmapdata_t) round((float)class_3 / (float)valid_count_speed * 100.0);
      report.class_4 = (rmapdata_t) round((float)class_4 / (float)valid_count_speed * 100.0);
      report.class_5 = (rmapdata_t) round((float)class_5 / (float)valid_count_speed * 100.0);
      report.class_6 = (rmapdata_t) round((float)class_6 / (float)valid_count_speed * 100.0);
      // Check sum report round class (unlikely error)
      uint8_t perc_sum = report.class_1 + report.class_2 + report.class_3 + report.class_4 + report.class_5 + report.class_6;
      if (perc_sum != 100) {
        uint8_t max_val = report.class_1;
        uint8_t max_cls = 1;
        if (report.class_2 >= max_val) {
          max_val = report.class_2;
          max_cls = 2;
        }
        if (report.class_3 >= max_val) {
          max_val = report.class_3;
          max_cls = 3;
        }
        if (report.class_4 >= max_val) {
          max_val = report.class_4;
          max_cls = 4;
        }
        if (report.class_5 >= max_val) {
          max_val = report.class_5;
          max_cls = 5;
        }
        if (report.class_6 >= max_val) {
          max_val = report.class_6;
          max_cls = 6;
        }
        if(perc_sum > 100) {
          max_val = perc_sum - 100;
          switch(max_cls) {
            case 1: report.class_1 -= max_val; break;
            case 2: report.class_2 -= max_val; break;
            case 3: report.class_3 -= max_val; break;
            case 4: report.class_4 -= max_val; break;
            case 5: report.class_5 -= max_val; break;
            case 6: report.class_6 -= max_val; break;
          }
        }
        if(perc_sum < 100) {
          max_val = 100 - perc_sum;
          switch(max_cls) {
            case 1: report.class_1 += max_val; break;
            case 2: report.class_2 += max_val; break;
            case 3: report.class_3 += max_val; break;
            case 4: report.class_4 += max_val; break;
            case 5: report.class_5 += max_val; break;
            case 6: report.class_6 += max_val; break;
          }
        }
      }
    }

    TRACE_DEBUG_F(F("-> report.vavg10_speed (%d)\r\n"), (rmapdata_t) report.vavg10_speed);
    TRACE_DEBUG_F(F("-> report.vavg10_direction (%d)\r\n"), (rmapdata_t) report.vavg10_speed);
    TRACE_DEBUG_F(F("-> report.vavg_speed (%d)\r\n"), (rmapdata_t) report.vavg_speed);
    TRACE_DEBUG_F(F("-> report.vavg_direction (%d)\r\n"), (rmapdata_t) report.vavg_direction);
    TRACE_DEBUG_F(F("-> report.peak_gust_speed (%d)\r\n"), (rmapdata_t) report.peak_gust_speed);
    TRACE_DEBUG_F(F("-> report.peak_gust_direction (%d)\r\n"), (rmapdata_t) report.peak_gust_direction);
    TRACE_DEBUG_F(F("-> report.long_gust_speed (%d)\r\n"), (rmapdata_t) report.long_gust_speed);
    TRACE_DEBUG_F(F("-> report.long_gust_direction (%d)\r\n"), (rmapdata_t) report.long_gust_direction);
    TRACE_DEBUG_F(F("-> report.avg_speed (%d)\r\n"), (rmapdata_t) report.avg_speed);
    TRACE_DEBUG_F(F("-> report.class_1 (%d)\r\n"), (rmapdata_t) report.class_1);
    TRACE_DEBUG_F(F("-> report.class_2 (%d)\r\n"), (rmapdata_t) report.class_2);
    TRACE_DEBUG_F(F("-> report.class_3 (%d)\r\n"), (rmapdata_t) report.class_3);
    TRACE_DEBUG_F(F("-> report.class_4 (%d)\r\n"), (rmapdata_t) report.class_4);
    TRACE_DEBUG_F(F("-> report.class_5 (%d)\r\n"), (rmapdata_t) report.class_5);
    TRACE_DEBUG_F(F("-> report.class_6 (%d)\r\n"), (rmapdata_t) report.class_6);
    TRACE_DEBUG_F(F("-> report.quality (%d)\r\n"), (rmapdata_t) report.quality);

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
