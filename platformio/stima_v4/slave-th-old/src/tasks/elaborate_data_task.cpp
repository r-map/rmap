/**@file elaborate_data_task.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>

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

#define TRACE_LEVEL ELABORATE_DATA_TASK_TRACE_LEVEL

#include "tasks/elaborate_data_task.h"

using namespace cpp_freertos;

ElaborateDataSensorTask::ElaborateDataSensorTask(const char *taskName, uint16_t stackSize, uint8_t priority, ElaboradeDataParam_t elaboradeDataParam) : Thread(taskName, stackSize, priority), param(elaboradeDataParam) {
  state = INIT;
  Start();
};

void ElaborateDataSensorTask::Run() {
  elaborate_data_t edata;

  bufferReset<sample_t, uint16_t, rmapdata_t>(&temperature_main_samples, SAMPLES_COUNT_MAX);
  bufferReset<sample_t, uint16_t, rmapdata_t>(&temperature_redundant_samples, SAMPLES_COUNT_MAX);
  bufferReset<sample_t, uint16_t, rmapdata_t>(&humidity_main_samples, SAMPLES_COUNT_MAX);
  bufferReset<sample_t, uint16_t, rmapdata_t>(&humidity_redundant_samples, SAMPLES_COUNT_MAX);

  bool is_1 = false;
  bool is_2 = false;
  bool is_3 = false;
  bool is_4 = false;

  while (true) {
    param.elaborataDataQueue->Dequeue(&edata, portMAX_DELAY);

    switch (edata.index) {
      case TEMPERATURE_MAIN_INDEX:
        TRACE_VERBOSE(F("Temperature [ %s ]: %d\r\n"), MAIN_STRING, edata.value);
        addValue<sample_t, uint16_t, rmapdata_t>(&temperature_main_samples, SAMPLES_COUNT_MAX, edata.value);
        is_1 = true;
      break;

      case TEMPERATURE_REDUNDANT_INDEX:
        TRACE_VERBOSE(F("Temperature [ %s ]: %d\r\n"), REDUNDANT_STRING, edata.value);
        addValue<sample_t, uint16_t, rmapdata_t>(&temperature_redundant_samples, SAMPLES_COUNT_MAX, edata.value);
        is_2 = true;
      break;

      case HUMIDITY_MAIN_INDEX:
        TRACE_VERBOSE(F("Humidity [ %s ]: %d\r\n"), MAIN_STRING, edata.value);
        addValue<sample_t, uint16_t, rmapdata_t>(&humidity_main_samples, SAMPLES_COUNT_MAX, edata.value);
        is_3 = true;
      break;

      case HUMIDITY_REDUNDANT_INDEX:
        TRACE_VERBOSE(F("Humidity [ %s ]: %d\r\n"), REDUNDANT_STRING, edata.value);
        addValue<sample_t, uint16_t, rmapdata_t>(&humidity_redundant_samples, SAMPLES_COUNT_MAX, edata.value);
        is_4 = true;
      break;
    }

    if (is_1 && is_2 && is_3 && is_4) {
      request_data_t request_data;
      is_1 = false;
      is_2 = false;
      is_3 = false;
      is_4 = false;

      if (param.requestDataQueue->Dequeue(&request_data, 0)) {
        make_report(request_data.is_init, request_data.report_time_s, request_data.observation_time_s);
        param.reportDataQueue->Enqueue(&report, 0);
      }
    }
  }
}

uint8_t ElaborateDataSensorTask::checkTemperature(rmapdata_t main_temperature, rmapdata_t redundant_temperature) {
  uint8_t quality = 0;

  float main = ((main_temperature - 27315.0) / 100.0);
  float redundant = ((redundant_temperature - 27315.0) / 100.0);

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

  return quality;
}

uint8_t ElaborateDataSensorTask::checkHumidity(rmapdata_t main_humidity, rmapdata_t redundant_humidity) {
  uint8_t quality = 0;

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

  return quality;
}

void ElaborateDataSensorTask::make_report (bool is_init, uint16_t report_time_s, uint8_t observation_time_s) {
  rmapdata_t main_temperature = 0;
  rmapdata_t redundant_temperature = 0;

  rmapdata_t main_humidity = 0;
  rmapdata_t redundant_humidity = 0;

  uint16_t valid_count_temperature = 0;
  uint16_t error_count_temperature = 0;
  float error_temperature_per = 0;

  uint16_t valid_count_humidity = 0;
  uint16_t error_count_humidity = 0;
  float error_humidity_per = 0;

  static uint16_t valid_count_temperature_o;
  static uint16_t error_count_temperature_o;
  float error_temperature_per_o = 0;

  static uint16_t valid_count_humidity_o;
  static uint16_t error_count_humidity_o;
  float error_humidity_per_o = 0;

  rmapdata_t avg_temperature = 0;
  rmapdata_t avg_temperature_quality = 0;

  rmapdata_t avg_humidity = 0;
  rmapdata_t avg_humidity_quality = 0;

  static rmapdata_t avg_temperature_o;
  static rmapdata_t min_temperature_o;
  static rmapdata_t max_temperature_o;
  static rmapdata_t avg_temperature_quality_o;

  static rmapdata_t avg_humidity_o;
  static rmapdata_t min_humidity_o;
  static rmapdata_t max_humidity_o;
  static rmapdata_t avg_humidity_quality_o;

  uint16_t report_sample_count = round((report_time_s * 1.0) / (*param.acquisition_delay_ms / 1000.0));
  uint16_t observation_sample_count = round((observation_time_s * 1.0) / (*param.acquisition_delay_ms / 1000.0));

  if (is_init) {
    valid_count_temperature_o = 0;
    error_count_temperature_o = 0;

    valid_count_humidity_o = 0;
    error_count_humidity_o = 0;

    avg_temperature_o = 0;
    min_temperature_o = RMAPDATA_MAX;
    max_temperature_o = RMAPDATA_MIN;
    avg_temperature_quality_o = 0;

    avg_humidity_o = 0;
    min_humidity_o = RMAPDATA_MAX;
    max_humidity_o = RMAPDATA_MIN;
    avg_humidity_quality_o = 0;
  }

  report.temperature.ist = RMAPDATA_MAX;
  report.temperature.min = RMAPDATA_MAX;
  report.temperature.avg = RMAPDATA_MAX;
  report.temperature.max = RMAPDATA_MAX;
  report.temperature.quality = RMAPDATA_MAX;

  report.humidity.ist = RMAPDATA_MAX;
  report.humidity.min = RMAPDATA_MAX;
  report.humidity.avg = RMAPDATA_MAX;
  report.humidity.max = RMAPDATA_MAX;
  report.humidity.quality = RMAPDATA_MAX;

  TRACE_INFO(F("Making report on %d seconds\r\n"), report_time_s);
  TRACE_DEBUG(F("-> %d samples counts need for report\r\n"), report_sample_count);
  TRACE_DEBUG(F("-> %d samples counts need for observation\r\n"), observation_sample_count);
  TRACE_DEBUG(F("-> %d observation counts need for report\r\n"), report_sample_count / observation_sample_count);
  TRACE_DEBUG(F("-> %d available temperature main samples count\r\n"), temperature_main_samples.count);
  TRACE_DEBUG(F("-> %d available temperature redundant samples count\r\n"), temperature_redundant_samples.count);
  TRACE_DEBUG(F("-> %d available humidity main samples count\r\n"), humidity_main_samples.count);
  TRACE_DEBUG(F("-> %d available humidity redundant samples count\r\n"), humidity_redundant_samples.count);

  bufferPtrResetBack<sample_t, uint16_t>(&temperature_main_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&humidity_main_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&temperature_redundant_samples, SAMPLES_COUNT_MAX);
  bufferPtrResetBack<sample_t, uint16_t>(&humidity_redundant_samples, SAMPLES_COUNT_MAX);

  // temperature samples
  for (uint16_t i=0; i<temperature_main_samples.count; i++) {
    main_temperature = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&temperature_main_samples, SAMPLES_COUNT_MAX);
    redundant_temperature = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&temperature_redundant_samples, SAMPLES_COUNT_MAX);

    // last sample
    if (i == 0) {
      report.temperature.sample = main_temperature;
    }

    avg_temperature_quality += (rmapdata_t) ((checkTemperature(main_temperature, redundant_temperature) - avg_temperature_quality) / (i+1));

    if (ISVALID_RMAPDATA(main_temperature)) {
      valid_count_temperature++;
      avg_temperature += (rmapdata_t) ((main_temperature - avg_temperature) / valid_count_temperature);
    }
    else {
      error_count_temperature++;
    }
  }

  error_temperature_per = (float)(error_count_temperature) / (float)(temperature_main_samples.count) * 100.0;
  TRACE_DEBUG(F("-> %d temperature samples error (%d%%)\r\n"), error_count_temperature, (int32_t) error_temperature_per);

  // humidity samples
  for (uint16_t i=0; i<humidity_main_samples.count; i++) {
    main_humidity = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&humidity_main_samples, SAMPLES_COUNT_MAX);
    redundant_humidity = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&humidity_redundant_samples, SAMPLES_COUNT_MAX);

    // last sample
    if (i == 0) {
      report.humidity.sample = main_humidity;
    }

    avg_humidity_quality += (rmapdata_t) ((checkHumidity(main_humidity, redundant_humidity) - avg_humidity_quality) / (i+1));

    if (ISVALID_RMAPDATA(main_humidity)) {
      valid_count_humidity++;
      avg_humidity += (rmapdata_t) ((main_humidity - avg_humidity) / valid_count_humidity);
    }
    else {
      error_count_humidity++;
    }
  }

  error_humidity_per = (float)(error_count_humidity) / (float)(humidity_main_samples.count) * 100.0;
  TRACE_DEBUG(F("-> %d humidity samples error (%d%%)\r\n"), error_count_humidity, (int32_t) error_humidity_per);

  // temperature
  if (temperature_main_samples.count >= observation_sample_count) {
    // sufficient number of valid samples
    if (valid_count_temperature && (error_temperature_per <= SAMPLE_ERROR_PERCENTAGE_MAX)) {
      valid_count_temperature_o++;

      avg_temperature_o += (rmapdata_t) ((avg_temperature - avg_temperature_o) / valid_count_temperature_o);

      avg_temperature_quality_o += (rmapdata_t) ((avg_temperature_quality - avg_temperature_quality_o) / (valid_count_temperature_o + error_count_temperature_o));

      if (avg_temperature <= min_temperature_o) {
        min_temperature_o = avg_temperature;
      }

      if (avg_temperature >= max_temperature_o) {
        max_temperature_o = avg_temperature;
      }
    }
    else {
      error_count_temperature_o++;
    }

    error_temperature_per_o = (float)(error_count_temperature_o) / (float)(observation_sample_count) * 100.0;
    TRACE_DEBUG(F("-> %d temperature observation error (%d%%)\r\n"), error_count_temperature_o, (int32_t) error_temperature_per_o);

    if (valid_count_temperature_o && (error_temperature_per_o <= OBSERVATION_ERROR_PERCENTAGE_MAX)) {
      report.temperature.ist = avg_temperature;
      report.temperature.min = min_temperature_o;
      report.temperature.avg = avg_temperature_o;
      report.temperature.max = max_temperature_o;
      report.temperature.quality = avg_temperature_quality_o;
    }
  }

  // humidity
  if (humidity_main_samples.count >= observation_sample_count) {
    // sufficient number of valid samples
    if (valid_count_humidity && (error_humidity_per <= SAMPLE_ERROR_PERCENTAGE_MAX)) {
      valid_count_humidity_o++;

      avg_humidity_o += (rmapdata_t) ((avg_humidity - avg_humidity_o) / valid_count_humidity_o);

      avg_humidity_quality_o += (rmapdata_t) ((avg_humidity_quality - avg_humidity_quality_o) / (valid_count_humidity_o + error_count_humidity_o));

      if (avg_humidity <= min_humidity_o) {
        min_humidity_o = avg_humidity;
      }

      if (avg_humidity >= max_humidity_o) {
        max_humidity_o = avg_humidity;
      }
    }
    else {
      error_count_humidity_o++;
    }

    error_humidity_per_o = (float)(error_count_humidity_o) / (float)(observation_sample_count) * 100.0;
    TRACE_DEBUG(F("-> %d humidity observation error (%d%%)\r\n"), error_count_humidity_o, (int32_t) error_humidity_per_o);

    if (valid_count_humidity_o && (error_humidity_per_o <= OBSERVATION_ERROR_PERCENTAGE_MAX)) {
      report.humidity.ist = avg_humidity;
      report.humidity.min = min_humidity_o;
      report.humidity.avg = avg_humidity_o;
      report.humidity.max = max_humidity_o;
      report.humidity.quality = avg_humidity_quality_o;
    }
  }

  TRACE_INFO(F("--> temperature report\t%d\t%d\t%d\t%d\t%d\t%d\r\n"), (int32_t) report.temperature.sample, (int32_t) report.temperature.ist, (int32_t) report.temperature.min, (int32_t) report.temperature.avg, (int32_t) report.temperature.max, (int32_t) report.temperature.quality);
  TRACE_INFO(F("--> humidity report\t%d\t%d\t%d\t%d\t%d\t%d\r\n"), (int32_t) report.humidity.sample, (int32_t) report.humidity.ist, (int32_t) report.humidity.min, (int32_t) report.humidity.avg, (int32_t) report.humidity.max, (int32_t) report.humidity.quality);
}

template<typename buffer_g, typename length_v, typename value_v> value_v bufferRead(buffer_g *buffer, length_v length) {
  value_v value = *buffer->read_ptr;

  if (buffer->read_ptr == buffer->values+length-1) {
    buffer->read_ptr = buffer->values;
  }
  else buffer->read_ptr++;

  return value;
}

template<typename buffer_g, typename length_v, typename value_v> value_v bufferReadBack(buffer_g *buffer, length_v length) {
  value_v value = *buffer->read_ptr;

  if (buffer->read_ptr == buffer->values) {
    buffer->read_ptr = buffer->values+length-1;
  }
  else buffer->read_ptr--;

  return value;
}

template<typename buffer_g, typename value_v> void bufferWrite(buffer_g *buffer, value_v value) {
  *buffer->write_ptr = value;
}

template<typename buffer_g> void bufferPtrReset(buffer_g *buffer) {
  buffer->read_ptr = buffer->values;
}

template<typename buffer_g, typename length_v> void bufferPtrResetBack(buffer_g *buffer, length_v length) {
  if (buffer->write_ptr == buffer->values) {
    buffer->read_ptr = buffer->values+length-1;
  }
  else buffer->read_ptr = buffer->write_ptr-1;
}

template<typename buffer_g, typename length_v> void incrementBuffer(buffer_g *buffer, length_v length) {
  if (buffer->count < length) {
    buffer->count++;
  }

  if (buffer->write_ptr+1 < buffer->values + length) {
    buffer->write_ptr++;
  } else buffer->write_ptr = buffer->values;
}

template<typename buffer_g, typename length_v, typename value_v> void bufferReset(buffer_g *buffer, length_v length) {
  memset(buffer->values, 0xFF, length * sizeof(value_v));
  buffer->count = 0;
  buffer->read_ptr = buffer->values;
  buffer->write_ptr = buffer->values;
}

template<typename buffer_g, typename length_v, typename value_v> void addValue(buffer_g *buffer, length_v length, value_v value) {
  *buffer->write_ptr = (value_v) value;
  incrementBuffer<buffer_g, length_v>(buffer, length);
}