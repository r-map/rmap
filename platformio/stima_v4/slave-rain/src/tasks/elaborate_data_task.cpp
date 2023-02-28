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

ElaborateDataTask::ElaborateDataTask(const char *taskName, uint16_t stackSize, uint8_t priority, ElaboradeDataParam_t elaboradeDataParam) : Thread(taskName, stackSize, priority), param(elaboradeDataParam)
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
  bufferReset<sample_t, uint16_t, rmapdata_t>(&rain_main_samples, SAMPLES_COUNT_MAX);
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
        case RAIN_MAIN_INDEX:
          TRACE_VERBOSE_F(F("Rain A [ %s ]: %d\r\n"), MAIN_STRING, edata.value);
          addValue<sample_t, uint16_t, rmapdata_t>(&rain_main_samples, SAMPLES_COUNT_MAX, edata.value);
          addValue<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX, param.system_status->flags.is_maintenance);
          break;

        case RAIN_REDUNDANT_INDEX:
          TRACE_VERBOSE_F(F("Rain B [ %s ]: %d\r\n"), REDUNDANT_STRING, edata.value);
          addValue<sample_t, uint16_t, rmapdata_t>(&rain_redundant_samples, SAMPLES_COUNT_MAX, edata.value);
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

uint8_t ElaborateDataTask::checkTemperature(rmapdata_t main_rain, rmapdata_t redundant_rain) {
  uint8_t quality = 0;

  #if (USE_REDUNDANT_SENSOR)

  float main = ((main_rain - 27315.0) / 100.0);
  float redundant = ((redundant_rain - 27315.0) / 100.0);

  if ((main > MAX_VALID_TEMPERATURE) || (main < MIN_VALID_TEMPERATURE)) {
    quality = 0;
  }
  else if (redundant != RMAPDATA_MAX) {
    if ((abs(main - redundant) <= 0.1)) {
      quality = 100;
    }
    else if ((abs(main - redundant) <= 0.2)) {
      quality = 95;
    }
    else if ((abs(main - redundant) <= 0.5)) {
      quality = 90;
    }
    else if ((abs(main - redundant) <= 0.6)) {
      quality = 80;
    }
    else if ((abs(main - redundant) <= 0.7)) {
      quality = 70;
    }
    else if ((abs(main - redundant) <= 0.8)) {
      quality = 60;
    }
    else if ((abs(main - redundant) <= 0.9)) {
      quality = 50;
    }
    else if ((abs(main - redundant) <= 1.0)) {
      quality = 40;
    }
    else if ((abs(main - redundant) <= 1.1)) {
      quality = 30;
    }
    else if ((abs(main - redundant) <= 1.5)) {
      quality = 20;
    }
    else if ((abs(main - redundant) <= 2.0)) {
      quality = 10;
    }
  }
  else {
    quality = 100;
  }

  #else
  quality = 100;
  #endif

  return quality;
}

uint8_t ElaborateDataTask::checkHumidity(rmapdata_t main_humidity, rmapdata_t redundant_humidity) {
  uint8_t quality = 0;

  #if (USE_REDUNDANT_SENSOR)

  if ((main_humidity > MAX_VALID_HUMIDITY) || (main_humidity < MIN_VALID_HUMIDITY)) {
    quality = 0;
  }
  else if (redundant_humidity != RMAPDATA_MAX) {
    if ((abs(main_humidity - redundant_humidity) <= 1)) {
      quality = 100;
    }
    else if ((abs(main_humidity - redundant_humidity) <= 2)) {
      quality = 95;
    }
    else if ((abs(main_humidity - redundant_humidity) <= 3)) {
      quality = 90;
    }
    else if ((abs(main_humidity - redundant_humidity) <= 4)) {
      quality = 80;
    }
    else if ((abs(main_humidity - redundant_humidity) <= 5)) {
      quality = 70;
    }
    else if ((abs(main_humidity - redundant_humidity) <= 6)) {
      quality = 60;
    }
    else if ((abs(main_humidity - redundant_humidity) <= 7)) {
      quality = 50;
    }
    else if ((abs(main_humidity - redundant_humidity) <= 8)) {
      quality = 40;
    }
    else if ((abs(main_humidity - redundant_humidity) <= 9)) {
      quality = 30;
    }
    else if ((abs(main_humidity - redundant_humidity) <= 10)) {
      quality = 20;
    }
    else if ((abs(main_humidity - redundant_humidity) <= 11)) {
      quality = 10;
    }
  }
  else {
    quality = 100;
  }

  #else
  quality = 100;
  #endif

  return quality;
}

void ElaborateDataTask::make_report (bool is_init, uint16_t report_time_s, uint8_t observation_time_s) {
  rmapdata_t main_rain = 0;
  rmapdata_t redundant_rain = 0;

  bool measures_maintenance = false;

  uint16_t valid_count_rain = 0;
  uint16_t error_count_rain = 0;
  float error_rain_per = 0;

  static uint16_t valid_count_rain_o;
  static uint16_t error_count_rain_o;
  float error_rain_per_o = 0;

  rmapdata_t avg_rain = 0;
  rmapdata_t avg_rain_quality = 0;

  static rmapdata_t avg_rain_o;
  static rmapdata_t min_rain_o;
  static rmapdata_t max_rain_o;
  static rmapdata_t avg_rain_quality_o;

  uint16_t report_sample_count = round((report_time_s * 1.0) / (param.configuration->sensor_acquisition_delay_ms / 1000.0));
  uint16_t observation_sample_count = round((observation_time_s * 1.0) / (param.configuration->sensor_acquisition_delay_ms / 1000.0));

  if (is_init) {
    valid_count_rain_o = 0;
    error_count_rain_o = 0;

    avg_rain_o = 0;
    min_rain_o = RMAPDATA_MAX;
    max_rain_o = RMAPDATA_MIN;
    avg_rain_quality_o = 0;

  }

  // TODO:_TH_RAIN_
  report.rain.ist = RMAPDATA_MAX;
  report.rain.sample = RMAPDATA_MAX;
  report.rain.quality = RMAPDATA_MAX;

  if (report_time_s && observation_time_s)
  {
    TRACE_INFO_F(F("Making report on %d seconds\r\n"), report_time_s);
    TRACE_DEBUG_F(F("-> %d samples counts need for report\r\n"), report_sample_count);
    TRACE_DEBUG_F(F("-> %d samples counts need for observation\r\n"), observation_sample_count);
    TRACE_DEBUG_F(F("-> %d observation counts need for report\r\n"), report_sample_count / observation_sample_count);
    TRACE_DEBUG_F(F("-> %d available rain main samples count\r\n"), rain_main_samples.count);
    TRACE_DEBUG_F(F("-> %d available rain redundant samples count\r\n"), rain_redundant_samples.count);
  }

  bufferPtrResetBack<sample_t, uint16_t>(&rain_main_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&rain_redundant_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<maintenance_t, uint16_t>(&maintenance_samples, SAMPLES_COUNT_MAX);

  // align all sensor's data to last common acquired sample
  uint16_t samples_count = rain_main_samples.count;

  if (rain_redundant_samples.count < samples_count)
  {
    samples_count = rain_redundant_samples.count;
  }

  // flush all data that is not aligned
  for (uint16_t i = samples_count; i < rain_main_samples.count; i++)
  {
    bufferReadBack<sample_t, uint16_t, rmapdata_t>(&rain_main_samples, SAMPLES_COUNT_MAX);
    bufferReadBack<maintenance_t, uint16_t, rmapdata_t>(&maintenance_samples, SAMPLES_COUNT_MAX);
  }

  for (uint16_t i = samples_count; i < rain_redundant_samples.count; i++)
  {
    bufferReadBack<sample_t, uint16_t, rmapdata_t>(&rain_redundant_samples, SAMPLES_COUNT_MAX);
  }

  // it's a report request
  if (report_time_s && observation_time_s)
  {
    for (uint16_t i = 0; i < rain_main_samples.count; i++)
    {
      main_rain = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&rain_main_samples, SAMPLES_COUNT_MAX);

      #if (USE_REDUNDANT_SENSOR)
      redundant_rain = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&rain_redundant_samples, SAMPLES_COUNT_MAX);
      #endif

      measures_maintenance = bufferReadBack<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX);

      // last sample
      if (i == 0)
      {
        report.rain.sample = main_rain;
      }

      // module in maintenance: ist, min, avg, max data it were not calculated
      if (!measures_maintenance)
      {
        avg_rain_quality += (rmapdata_t)((checkTemperature(main_rain, redundant_rain) - avg_rain_quality) / (i + 1));

        if (ISVALID_RMAPDATA(main_rain))
        {
          valid_count_rain++;
          avg_rain += (rmapdata_t)((main_rain - avg_rain) / valid_count_rain);
        }
        else
        {
          error_count_rain++;
        }
      }
    }

    error_rain_per = (float)(error_count_rain) / (float)(rain_main_samples.count) * 100.0;
    TRACE_DEBUG_F(F("-> %d temperature samples error (%d%%)\r\n"), error_count_rain, (int32_t)error_rain_per);

    // x MARCO
    // TODO: Verify Reset buffer maintenance se corretto qua ......
    // TODO: Verify soot e sopra ... for i..humidity samples.count o sensor_count allineato???
    // TODO: all'inizio c'e sempre un valore MIN a 0 di TP e UR se rihiesta è senza init
    // Non mi è chiaro a capire cosa è giusto chiedere dal master (init o no?)
    // Non capisco la differenza per avere il dato corrente o il dato complessivo da registrare
    // Io ho previsto 3 comandi 1 chè è solo il sample (x visualizzazione display == OK)
    // Gli altri due 1 per avere il dato corrente attuale e l'altro per il dato calcolato
    // alla fine con richiesta valore e reinizializzazione per nuovo calcolo...
    bufferPtrResetBack<maintenance_t, uint16_t>(&maintenance_samples, SAMPLES_COUNT_MAX);

    // temperature
    if (rain_main_samples.count >= observation_sample_count)
    {
      // sufficient number of valid samples
      if (valid_count_rain && (error_rain_per <= SAMPLE_ERROR_PERCENTAGE_MAX))
      {
        valid_count_rain_o++;

        avg_rain_o += (rmapdata_t)((avg_rain - avg_rain_o) / valid_count_rain_o);

        avg_rain_quality_o += (rmapdata_t)((avg_rain_quality - avg_rain_quality_o) / (valid_count_rain_o + error_count_rain_o));

        if (avg_rain <= min_rain_o)
        {
          min_rain_o = avg_rain;
        }

        if (avg_rain >= max_rain_o)
        {
          max_rain_o = avg_rain;
        }
      }
      else
      {
        error_count_rain_o++;
      }

      error_rain_per_o = (float)(error_count_rain_o) / (float)(observation_sample_count)*100.0;
      TRACE_DEBUG_F(F("-> %d temperature observation error (%d%%)\r\n"), error_count_rain_o, (int32_t)error_rain_per_o);

      if (valid_count_rain_o && (error_rain_per_o <= OBSERVATION_ERROR_PERCENTAGE_MAX))
      {
        report.rain.ist = avg_rain;
        report.rain.min = min_rain_o;
        report.rain.avg = avg_rain_o;
        report.rain.max = max_rain_o;
        report.rain.quality = avg_rain_quality_o;
      }
    }

    TRACE_INFO_F(F("--> rain report\t%d\t%d\t%d\t%d\t%d\t%d\r\n"), (int32_t)report.rain.sample, (int32_t)report.rain.ist, (int32_t)report.rain.min, (int32_t)report.rain.avg, (int32_t)report.rain.max, (int32_t)report.rain.quality);
  }
  // it's a sample request
  else
  {
    report.rain.sample = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&rain_main_samples, SAMPLES_COUNT_MAX);
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
