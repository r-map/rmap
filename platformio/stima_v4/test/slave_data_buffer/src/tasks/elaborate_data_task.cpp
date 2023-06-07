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
  state = ELABORATE_DATA_INIT;
  Start();
};

void ElaborateDataTask::Run() {
  // Queue for data
  elaborate_data_t edata;
  request_data_t request_data;
  // System message data queue structured
  system_message_t system_message;

  bufferReset<sample_t, uint16_t, rmapdata_t>(&battery_charge_samples, SAMPLES_COUNT_MAX);
  bufferReset<sample_t, uint16_t, rmapdata_t>(&battery_voltage_samples, SAMPLES_COUNT_MAX);
  bufferReset<sample_t, uint16_t, rmapdata_t>(&battery_current_samples, SAMPLES_COUNT_MAX);
  bufferReset<sample_t, uint16_t, rmapdata_t>(&input_voltage_samples, SAMPLES_COUNT_MAX);
  bufferReset<sample_t, uint16_t, rmapdata_t>(&input_current_samples, SAMPLES_COUNT_MAX);
  bufferReset<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX);

  // Fill Buffer

  void *check_ptr;
  bool test_fail = false;
  bool maintenance_value = false;
  rmapdata_t rmap_value;

  // TEST 1 : FILL BUFFER FROM first data after reset to complete full buffer
  // Check Before and After Add Data Position of Buffer Write pointer Correct Adding TypeOf Template Data
  // maintenance(Bool) 1->Bytes for PTR, rmap_data_t for other (uint32) 4->Bytes
  // Last ADD rollBack PoiterWrite and Must GO To Values base address of Buffer (OK If Buffer Write Resetted)

  for(uint16_t iIdx=0; iIdx<SAMPLES_COUNT_MAX; iIdx++)
  {
    
    // Maintenance = False
    check_ptr = maintenance_samples.write_ptr;
    addValue<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX, maintenance_value);
    if ((check_ptr+sizeof(bool)) != maintenance_samples.write_ptr) {
      if(iIdx==SAMPLES_COUNT_MAX-1) {
        if(maintenance_samples.write_ptr != maintenance_samples.values) {
          test_fail = true;
        }
      } else {
        test_fail = true;
      }
    }
    if(maintenance_samples.values[iIdx]!=maintenance_value) {
      test_fail = true;
    }

    edata.value = 100;
    rmap_value = edata.value;
    check_ptr = battery_charge_samples.write_ptr;
    addValue<sample_t, uint16_t, rmapdata_t>(&battery_charge_samples, SAMPLES_COUNT_MAX, edata.value);
    if ((check_ptr+sizeof(rmapdata_t)) != battery_charge_samples.write_ptr) {
      if(iIdx==SAMPLES_COUNT_MAX-1) {
        if(battery_charge_samples.write_ptr != battery_charge_samples.values) {
          test_fail = true;
        }
      } else {
        test_fail = true;
      }
    }
    if(battery_charge_samples.values[iIdx]!=rmap_value) {
      test_fail = true;
    }

    edata.value = 12;
    check_ptr = battery_voltage_samples.write_ptr;
    addValue<sample_t, uint16_t, rmapdata_t>(&battery_voltage_samples, SAMPLES_COUNT_MAX, edata.value);
    if ((check_ptr+sizeof(rmapdata_t)) != battery_voltage_samples.write_ptr) {
      if(iIdx==SAMPLES_COUNT_MAX-1) {
        if(battery_voltage_samples.write_ptr != battery_voltage_samples.values) {
          test_fail = true;
        }
      } else {
        test_fail = true;
      }
    }

    edata.value = 1.5;
    check_ptr = battery_current_samples.write_ptr;
    addValue<sample_t, uint16_t, rmapdata_t>(&battery_current_samples, SAMPLES_COUNT_MAX, edata.value);
    if ((check_ptr+sizeof(rmapdata_t)) != battery_current_samples.write_ptr) {
      if(iIdx==SAMPLES_COUNT_MAX-1) {
        if(battery_current_samples.write_ptr != battery_current_samples.values) {
          test_fail = true;
        }
      } else {
        test_fail = true;
      }
    }

    edata.value = 13.5;
    check_ptr = input_voltage_samples.write_ptr;
    addValue<sample_t, uint16_t, rmapdata_t>(&input_voltage_samples, SAMPLES_COUNT_MAX, edata.value);
    if ((check_ptr+sizeof(rmapdata_t)) != input_voltage_samples.write_ptr) {
      if(iIdx==SAMPLES_COUNT_MAX-1) {
        if(input_voltage_samples.write_ptr != input_voltage_samples.values) {
          test_fail = true;
        }
      } else {
        test_fail = true;
      }
    }

    edata.value = 0.73;
    check_ptr = input_current_samples.write_ptr;
    addValue<sample_t, uint16_t, rmapdata_t>(&input_current_samples, SAMPLES_COUNT_MAX, edata.value);
    if ((check_ptr+sizeof(rmapdata_t)) != input_current_samples.write_ptr) {
      if(iIdx==SAMPLES_COUNT_MAX-1) {
        if(input_current_samples.write_ptr != input_current_samples.values) {
          test_fail = true;
        }
      } else {
        test_fail = true;
      }
    }
  }

  if(test_fail) {
    Serial.println("TEST 1: FAIL");
  } else
  {
    Serial.println("TEST 1: SUCCESS");
  }

  // Make Report
  make_report(true, 900, 60);
  test_fail = false;
  if(report.avg_battery_charge != 100) test_fail = true;

  if(test_fail) {
    Serial.println("TEST 2: FAIL");
  } else
  {
    Serial.println("TEST 2: SUCCESS");
  }

  // ADD OTHER 100 ( 1/2 900 sec x 4 Time(sec) acq) VALUE...
  // Mean value for first data (100... 125 Times + 50 100 Times) => 77
  for(uint16_t iIdx=0; iIdx<100; iIdx++)
  {
    // Maintenance = False
    addValue<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX, maintenance_value);

    edata.value = 50;
    rmap_value = edata.value;
    addValue<sample_t, uint16_t, rmapdata_t>(&battery_charge_samples, SAMPLES_COUNT_MAX, edata.value);

    edata.value = 12;
    rmap_value = edata.value;
    addValue<sample_t, uint16_t, rmapdata_t>(&battery_voltage_samples, SAMPLES_COUNT_MAX, edata.value);

    edata.value = 1.5;
    rmap_value = edata.value;
    addValue<sample_t, uint16_t, rmapdata_t>(&battery_current_samples, SAMPLES_COUNT_MAX, edata.value);

    edata.value = 13.5;
    rmap_value = edata.value;
    addValue<sample_t, uint16_t, rmapdata_t>(&input_voltage_samples, SAMPLES_COUNT_MAX, edata.value);

    edata.value = 0.73;
    rmap_value = edata.value;
    addValue<sample_t, uint16_t, rmapdata_t>(&input_current_samples, SAMPLES_COUNT_MAX, edata.value);
  }

  // Make Report
  make_report(true, 900, 60);
  test_fail = false;
  if(!((report.avg_battery_charge >= 77) && (report.avg_battery_charge <= 78))) test_fail = true;

  if(test_fail) {
    Serial.println("TEST 3: FAIL");
  } else
  {
    Serial.println("TEST 3: SUCCESS");
  }

  // Check value
  Serial.println("Result: ->");
  Serial.println(report.avg_battery_charge);
  Serial.println(report.avg_battery_current);
  Serial.println(report.avg_battery_voltage);
  Serial.println(report.avg_input_current);
  Serial.println(report.avg_input_voltage);

  // End Test

  while (true) {
    DelayUntil(Ticks::MsToTicks(ELABORATE_TASK_WAIT_DELAY_MS));
  }
}

uint8_t ElaborateDataTask::checkMppt(rmapdata_t main_mppt) {
  // Optional check quality data function
  uint8_t quality = 100;
  return quality;
}

void ElaborateDataTask::make_report (bool is_init, uint16_t report_time_s, uint8_t observation_time_s) {

  bool measures_maintenance = false;
  float valid_data_calc_perc;

  rmapdata_t battery_charge_s = 0;
  // rmapdata_t battery_voltage = 0;
  // rmapdata_t battery_current = 0;
  // rmapdata_t input_voltage = 0;
  // rmapdata_t input_current = 0;

  float avg_battery_charge_s = 0;
  float avg_battery_charge_o = 0;
  // rmapdata_t avg_battery_voltage = 0;
  // rmapdata_t avg_battery_current = 0;
  // rmapdata_t avg_input_voltage = 0;
  // rmapdata_t avg_input_current = 0;

  float avg_battery_charge_quality_s = 0;
  float avg_battery_charge_quality_o = 0;
  // rmapdata_t avg_battery_voltage_quality = 0;
  // rmapdata_t avg_battery_current_quality = 0;
  // rmapdata_t avg_input_voltage_quality = 0;
  // rmapdata_t avg_input_current_quality = 0;

  uint16_t valid_count_battery_charge_s = 0;
  uint16_t total_count_battery_charge_s = 0;
  uint16_t valid_count_battery_charge_o = 0;

  // uint16_t valid_count_battery_voltage = 0;
  // uint16_t error_count_battery_voltage = 0;
  // float error_battery_voltage_per = 0;

  // uint16_t valid_count_battery_current = 0;
  // uint16_t error_count_battery_current = 0;
  // float error_battery_current_per = 0;

  // uint16_t valid_count_input_voltage = 0;
  // uint16_t error_count_input_voltage = 0;
  // float error_input_voltage_per = 0;

  // uint16_t valid_count_input_current = 0;
  // uint16_t error_count_input_current = 0;
  // float error_input_current_per = 0;


  // static uint16_t valid_count_battery_voltage_o;
  // static uint16_t error_count_battery_voltage_o;
  // float error_battery_voltage_per_o = 0;

  // static uint16_t valid_count_battery_current_o;
  // static uint16_t error_count_battery_current_o;
  // float error_battery_current_per_o = 0;

  // static uint16_t valid_count_input_voltage_o;
  // static uint16_t error_count_input_voltage_o;
  // float error_input_voltage_per_o = 0;

  // static uint16_t valid_count_input_current_o;
  // static uint16_t error_count_input_current_o;
  // float error_input_current_per_o = 0;

  // static rmapdata_t avg_battery_voltage_o;
  // static rmapdata_t avg_battery_voltage_quality_o;

  // static rmapdata_t avg_battery_current_o;
  // static rmapdata_t avg_battery_current_quality_o;

  // static rmapdata_t avg_input_voltage_o;
  // static rmapdata_t avg_input_voltage_quality_o;

  // static rmapdata_t avg_input_current_o;
  // static rmapdata_t avg_input_current_quality_o;

  param.configuration->sensor_acquisition_delay_ms = 4000;

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

  bufferPtrResetBack<maintenance_t, uint16_t>(&maintenance_samples, SAMPLES_COUNT_MAX);

  bufferPtrResetBack<sample_t, uint16_t>(&battery_charge_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&battery_voltage_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&battery_current_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&input_voltage_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&input_current_samples, SAMPLES_COUNT_MAX);

  // align all sensor's data to last common acquired sample (miantenance is always first data add into queue)
  // ALign with last maintenance record pointer
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

  bool is_observation = false;
  uint16_t n_sample = 0;

  // it's a simple istant or report request?
  if (report_time_s == 0)
  {
    // Make last data value to Get Istant show value
    report.avg_battery_charge = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&battery_charge_samples, SAMPLES_COUNT_MAX);
    report.avg_battery_charge_quality = (rmapdata_t)checkMppt(battery_charge_s);
    // report.avg_battery_charge = RMAPDATA_MAX;
    // report.avg_battery_charge_quality = RMAPDATA_MAX;
    // report.avg_battery_voltage = RMAPDATA_MAX;
    // report.avg_battery_voltage_quality = RMAPDATA_MAX;
    // report.avg_battery_current = RMAPDATA_MAX;
    // report.avg_battery_current_quality = RMAPDATA_MAX;
    // report.avg_input_voltage = RMAPDATA_MAX;
    // report.avg_input_voltage_quality = RMAPDATA_MAX;
    // report.avg_input_current = RMAPDATA_MAX;
    // report.avg_input_current_quality = RMAPDATA_MAX;
  }
  else
  {
    // Make a report complete (try with all sample present)
    for (uint16_t i = 0; i < battery_charge_samples.count; i++)
    {

      n_sample++; // Elaborate sample number...

      // End of Sample in calculation (Completed with the request...)
      if(n_sample > report_sample_count) break;

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
      avg_battery_charge_quality_s += (float)(((float)checkMppt(battery_charge_s) - avg_battery_charge_quality_s) / total_count_battery_charge_s);
      if ((ISVALID_RMAPDATA(battery_charge_s)) && !measures_maintenance)
      {
        valid_count_battery_charge_s++;
        avg_battery_charge_s += (float)(((float)battery_charge_s - avg_battery_charge_s) / valid_count_battery_charge_s);
      }

      // battery_voltage = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&battery_voltage_samples, SAMPLES_COUNT_MAX);
      // if (!measures_maintenance)
      // {
      //   avg_battery_voltage_quality += (rmapdata_t)((checkMppt(battery_voltage) - avg_battery_voltage_quality) / (i + 1));

      //   if (ISVALID_RMAPDATA(battery_voltage))
      //   {
      //     valid_count_battery_voltage++;
      //     avg_battery_voltage += (rmapdata_t)((avg_battery_voltage - battery_voltage) / valid_count_battery_voltage);
      //   }
      //   else
      //   {
      //     error_count_battery_voltage++;
      //   }
      // }

      // battery_current = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&battery_current_samples, SAMPLES_COUNT_MAX);
      // if (!measures_maintenance)
      // {
      //   avg_battery_current_quality += (rmapdata_t)((checkMppt(battery_current) - avg_battery_current_quality) / (i + 1));

      //   if (ISVALID_RMAPDATA(battery_current))
      //   {
      //     valid_count_battery_current++;
      //     avg_battery_current += (rmapdata_t)((avg_battery_current - battery_current) / valid_count_battery_current);
      //   }
      //   else
      //   {
      //     error_count_battery_current++;
      //   }
      // }

      // input_voltage = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&input_voltage_samples, SAMPLES_COUNT_MAX);
      // if (!measures_maintenance)
      // {
      //   avg_input_voltage_quality += (rmapdata_t)((checkMppt(input_voltage) - avg_input_voltage_quality) / (i + 1));

      //   if (ISVALID_RMAPDATA(input_voltage))
      //   {
      //     valid_count_input_voltage++;
      //     avg_input_voltage += (rmapdata_t)((avg_input_voltage - input_voltage) / valid_count_input_voltage);
      //   }
      //   else
      //   {
      //     error_count_input_voltage++;
      //   }
      // }

      // input_current = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&input_current_samples, SAMPLES_COUNT_MAX);
      // if (!measures_maintenance)
      // {
      //   avg_input_current_quality += (rmapdata_t)((checkMppt(input_current) - avg_input_current_quality) / (i + 1));

      //   if (ISVALID_RMAPDATA(input_current))
      //   {
      //     valid_count_input_current++;
      //     avg_input_current += (rmapdata_t)((avg_input_current - input_current) / valid_count_input_current);
      //   }
      //   else
      //   {
      //     error_count_input_current++;
      //   }
      // }

      // ***************************************************************************************************
      // ************* ELABORATE OBSERVATION VALUES FOR TYPE SENSOR FOR PREPARE REPORT RESPONSE ************
      // ***************************************************************************************************
      if(is_observation) {

        //TRACE_DEBUG_F(F("-> %d battery charge error (%d%%)\r\n"), error_count_battery_charge, (int32_t)error_battery_charge_per);

        // sufficient number of valid samples
        valid_data_calc_perc = (float)(valid_count_battery_charge_s) / (float)(total_count_battery_charge_s) * 100.0;
        if (valid_data_calc_perc >= SAMPLE_ERROR_PERCENTAGE_MIN)
        {
          valid_count_battery_charge_o++;
          avg_battery_charge_o += (avg_battery_charge_s - avg_battery_charge_o) / valid_count_battery_charge_o;
          avg_battery_charge_quality_o += (avg_battery_charge_quality_s - avg_battery_charge_quality_o) / valid_count_battery_charge_o;
        }

        // error_battery_charge_per_o = (float)(error_count_battery_charge_o) / (float)(observation_sample_count)*100.0;
        // TRACE_DEBUG_F(F("-> %d battery charge observation error (%d%%)\r\n"), error_count_battery_charge_o, (int32_t)error_battery_charge_per_o);

        // Reset Buffer sample for calculate next observation
        avg_battery_charge_quality_s = 0;
        avg_battery_charge_s = 0;
        valid_count_battery_charge_s = 0;
        total_count_battery_charge_s = 0;

        // error_battery_voltage_per = (float)(error_count_battery_voltage) / (float)(battery_voltage_samples.count) * 100.0;
        // TRACE_DEBUG_F(F("-> %d battery voltage error (%d%%)\r\n"), error_count_battery_voltage, (int32_t)error_battery_voltage_per);

        // if (battery_voltage_samples.count >= observation_sample_count)
        // {
        //   // sufficient number of valid samples
        //   if (valid_count_battery_voltage && (error_battery_voltage_per <= SAMPLE_ERROR_PERCENTAGE_MAX))
        //   {
        //     valid_count_battery_voltage_o++;

        //     avg_battery_voltage_o += (rmapdata_t)((avg_battery_voltage - avg_battery_voltage_o) / valid_count_battery_voltage_o);
        //     avg_battery_voltage_quality_o += (rmapdata_t)((avg_battery_voltage_quality - avg_battery_voltage_quality_o) / (valid_count_battery_voltage_o + error_count_battery_voltage_o));
        //   }
        //   else
        //   {
        //     error_count_battery_voltage_o++;
        //   }

        //   error_battery_voltage_per_o = (float)(error_count_battery_voltage_o) / (float)(observation_sample_count)*100.0;
        //   TRACE_DEBUG_F(F("-> %d battery voltage observation error (%d%%)\r\n"), error_count_battery_voltage_o, (int32_t)error_battery_voltage_per_o);

        //   if (valid_count_battery_voltage_o && (error_battery_voltage_per_o <= OBSERVATION_ERROR_PERCENTAGE_MAX))
        //   {
        //     report.avg_battery_voltage = avg_battery_voltage_o;
        //     report.avg_battery_voltage_quality = avg_battery_voltage_quality_o;
        //   }
        // }

        // error_battery_current_per = (float)(error_count_battery_current) / (float)(battery_current_samples.count) * 100.0;
        // TRACE_DEBUG_F(F("-> %d battery current error (%d%%)\r\n"), error_count_battery_current, (int32_t)error_battery_current_per);

        // if (battery_current_samples.count >= observation_sample_count)
        // {
        //   // sufficient number of valid samples
        //   if (valid_count_battery_current && (error_battery_current_per <= SAMPLE_ERROR_PERCENTAGE_MAX))
        //   {
        //     valid_count_battery_current_o++;

        //     avg_battery_current_o += (rmapdata_t)((avg_battery_current - avg_battery_current_o) / valid_count_battery_current_o);
        //     avg_battery_current_quality_o += (rmapdata_t)((avg_battery_current_quality - avg_battery_current_quality_o) / (valid_count_battery_current_o + error_count_battery_current_o));
        //   }
        //   else
        //   {
        //     error_count_battery_current_o++;
        //   }

        //   error_battery_current_per_o = (float)(error_count_battery_current_o) / (float)(observation_sample_count)*100.0;
        //   TRACE_DEBUG_F(F("-> %d battery current observation error (%d%%)\r\n"), error_count_battery_current_o, (int32_t)error_battery_current_per_o);

        //   if (valid_count_battery_current_o && (error_battery_current_per_o <= OBSERVATION_ERROR_PERCENTAGE_MAX))
        //   {
        //     report.avg_battery_current = avg_battery_current_o;
        //     report.avg_battery_current_quality = avg_battery_current_quality_o;
        //   }
        // }

        // error_input_voltage_per = (float)(error_count_input_voltage) / (float)(input_voltage_samples.count) * 100.0;
        // TRACE_DEBUG_F(F("-> %d battery current error (%d%%)\r\n"), error_count_input_voltage, (int32_t)error_input_voltage_per);

        // if (input_voltage_samples.count >= observation_sample_count)
        // {
        //   // sufficient number of valid samples
        //   if (valid_count_input_voltage && (error_input_voltage_per <= SAMPLE_ERROR_PERCENTAGE_MAX))
        //   {
        //     valid_count_input_voltage_o++;

        //     avg_input_voltage_o += (rmapdata_t)((avg_input_voltage - avg_input_voltage_o) / valid_count_input_voltage_o);
        //     avg_input_voltage_quality_o += (rmapdata_t)((avg_input_voltage_quality - avg_input_voltage_quality_o) / (valid_count_input_voltage_o + error_count_input_voltage_o));
        //   }
        //   else
        //   {
        //     error_count_input_voltage_o++;
        //   }

        //   error_input_voltage_per_o = (float)(error_count_input_voltage_o) / (float)(observation_sample_count)*100.0;
        //   TRACE_DEBUG_F(F("-> %d input voltage observation error (%d%%)\r\n"), error_count_input_voltage_o, (int32_t)error_input_voltage_per_o);

        //   if (valid_count_input_voltage_o && (error_input_voltage_per_o <= OBSERVATION_ERROR_PERCENTAGE_MAX))
        //   {
        //     report.avg_input_voltage = avg_input_voltage_o;
        //     report.avg_input_voltage_quality = avg_input_voltage_quality_o;
        //   }
        // }

        // error_input_current_per = (float)(error_count_input_current) / (float)(input_current_samples.count) * 100.0;
        // TRACE_DEBUG_F(F("-> %d battery current error (%d%%)\r\n"), error_count_input_current, (int32_t)error_input_current_per);

        // if (input_current_samples.count >= observation_sample_count)
        // {
        //   // sufficient number of valid samples
        //   if (valid_count_input_current && (error_input_current_per <= SAMPLE_ERROR_PERCENTAGE_MAX))
        //   {
        //     valid_count_input_current_o++;

        //     avg_input_current_o += (rmapdata_t)((avg_input_current - avg_input_current_o) / valid_count_input_current_o);
        //     avg_input_current_quality_o += (rmapdata_t)((avg_input_current_quality - avg_input_current_quality_o) / (valid_count_input_current_o + error_count_input_current_o));
        //   }
        //   else
        //   {
        //     error_count_input_current_o++;
        //   }

        //   error_input_current_per_o = (float)(error_count_input_current_o) / (float)(observation_sample_count)*100.0;
        //   TRACE_DEBUG_F(F("-> %d input current observation error (%d%%)\r\n"), error_count_input_current_o, (int32_t)error_input_current_per_o);

        //   if (valid_count_input_current_o && (error_input_current_per_o <= OBSERVATION_ERROR_PERCENTAGE_MAX))
        //   {
        //     report.avg_input_current = avg_input_current_o;
        //     report.avg_input_current_quality = avg_input_current_quality_o;
        //   }
        // }
      }
    }

    // ***************************************************************************************************
    // ******* GENERATE REPORT RESPONSE WITH ALL DATA AVAIABLE AND VALID WITH EXPECETD OBSERVATION *******
    // ***************************************************************************************************
    valid_data_calc_perc = (float)(valid_count_battery_charge_o) / (float)(observation_sample_count) * 100.0;
    if (valid_data_calc_perc >= OBSERVATION_ERROR_PERCENTAGE_MIN)
    {
      report.avg_battery_charge = (rmapdata_t)avg_battery_charge_o;
      report.avg_battery_charge_quality = (rmapdata_t)avg_battery_charge_quality_o;
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