/**@file th_sensor_task.cpp */

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

#define TRACE_LEVEL TH_SENSOR_TASK_TRACE_LEVEL

#include "tasks/th_sensor_task.h"

using namespace cpp_freertos;

TemperatureHumidtySensorTask::TemperatureHumidtySensorTask(const char *taskName, uint16_t stackSize, uint8_t priority, TemperatureHumidtySensorParam_t temperatureHumidtySensorParam) : Thread(taskName, stackSize, priority), param(temperatureHumidtySensorParam) {
  state = INIT;
  Start();
};

void TemperatureHumidtySensorTask::Run() {
  rmapdata_t values_readed_from_sensor[VALUES_TO_READ_FROM_SENSOR_COUNT];
  elaborate_data_t edata;
  uint32_t delay_ms;
  static bool is_test;
  bool is_temperature_redundant;
  bool is_humidity_redundant;

  while (true) {
    switch (state) {
      case INIT:

        TRACE_INFO(F("Initializing sensors...\r\n"));
        for (uint8_t i=0; i<*param.sensors_count; i++) {
          if (strlen(param.sensors[i].type) == 3) {
            SensorDriver::createSensor(SENSOR_DRIVER_I2C, param.sensors[i].type, param.sensors[i].i2c_address, 1, sensors, &Wire);
          }
        }
        state = SETUP;
      break;

      case SETUP:
        is_test = false;
        memset((void *) values_readed_from_sensor, RMAPDATA_MAX, (size_t) (VALUES_TO_READ_FROM_SENSOR_COUNT * sizeof(rmapdata_t)));

        for (uint8_t i=0; i<SensorDriver::getSensorsCount(); i++) {
          if (!sensors[i]->isSetted()) {
            param.wireLock->Take();
            sensors[i]->setup();
            param.wireLock->Give();
            TRACE_INFO(F("--> %u: %s-%s 0x%02X [ %s ]\t [ %s ]\r\n"), i+1, SENSOR_DRIVER_I2C, sensors[i]->getType(), sensors[i]->getAddress(), param.sensors[i].is_redundant ? REDUNDANT_STRING : MAIN_STRING, sensors[i]->isSetted() ? OK_STRING : FAIL_STRING);
          }
        }
        state = PREPARE;
      break;

      case PREPARE:
        delay_ms = 0;
        for (uint8_t i=0; i<SensorDriver::getSensorsCount(); i++) {
          sensors[i]->resetPrepared();
          param.wireLock->Take();
          sensors[i]->prepare(is_test);
          param.wireLock->Give();

          // wait the most slowest
          if (sensors[i]->getDelay() > delay_ms) {
            delay_ms = sensors[i]->getDelay();
          }
        }

        if (delay_ms) {
          DelayUntil(Ticks::MsToTicks(delay_ms));
        }
        state = READ;
      break;

      case READ:
        is_temperature_redundant = false;
        is_humidity_redundant = false;
        
        for (uint8_t i=0; i<SensorDriver::getSensorsCount(); i++) {
          do {
            param.wireLock->Take();
            sensors[i]->get(&values_readed_from_sensor[0], VALUES_TO_READ_FROM_SENSOR_COUNT, is_test);
            param.wireLock->Give();
            if (sensors[i]->getDelay()) {
              DelayUntil(Ticks::MsToTicks(sensors[i]->getDelay()));
            }
          }
          while (!sensors[i]->isEnd() && !sensors[i]->isReaded());

          if (false) {}

          #if (USE_SENSOR_ADT)
          else if (strcmp(sensors[i]->getType(), SENSOR_TYPE_ADT) == 0) {
            edata.value = values_readed_from_sensor[0];
            edata.index = param.sensors[i].is_redundant ? TEMPERATURE_REDUNDANT_INDEX : TEMPERATURE_MAIN_INDEX;
            param.elaborataDataQueue->Enqueue(&edata, 0);
            is_temperature_redundant = param.sensors[i].is_redundant;
          }
          #endif

          #if (USE_SENSOR_HIH)
          else if (strcmp(sensors[i]->getType(), SENSOR_TYPE_HIH) == 0) {
            edata.value = values_readed_from_sensor[0];
            edata.index = param.sensors[i].is_redundant ? HUMIDITY_REDUNDANT_INDEX : HUMIDITY_MAIN_INDEX;
            param.elaborataDataQueue->Enqueue(&edata, 0);
            is_humidity_redundant = param.sensors[i].is_redundant;
          }
          #endif

          #if (USE_SENSOR_HYT)
          else if (strcmp(sensors[i]->getType(), SENSOR_TYPE_HYT) == 0) {
            edata.value = values_readed_from_sensor[1];
            edata.index = param.sensors[i].is_redundant ? TEMPERATURE_REDUNDANT_INDEX : TEMPERATURE_MAIN_INDEX;
            param.elaborataDataQueue->Enqueue(&edata, 0);
            is_temperature_redundant = param.sensors[i].is_redundant;

            edata.value = values_readed_from_sensor[0];
            edata.index = param.sensors[i].is_redundant ? HUMIDITY_REDUNDANT_INDEX : HUMIDITY_MAIN_INDEX;
            param.elaborataDataQueue->Enqueue(&edata, 0);
            is_humidity_redundant = param.sensors[i].is_redundant;
          }
          #endif

          #if (USE_SENSOR_SHT)
          else if (strcmp(sensors[i]->getType(), SENSOR_TYPE_SHT) == 0) {
            edata.value = values_readed_from_sensor[1];
            edata.index = param.sensors[i].is_redundant ? TEMPERATURE_REDUNDANT_INDEX : TEMPERATURE_MAIN_INDEX;
            param.elaborataDataQueue->Enqueue(&edata, 0);
            is_temperature_redundant = param.sensors[i].is_redundant;

            edata.value = values_readed_from_sensor[0];
            edata.index = param.sensors[i].is_redundant ? HUMIDITY_REDUNDANT_INDEX : HUMIDITY_MAIN_INDEX;
            param.elaborataDataQueue->Enqueue(&edata, 0);
            is_humidity_redundant = param.sensors[i].is_redundant;
          }
          #endif

          #if (TRACE_LEVEL >= TRACE_LEVEL_VERBOSE)
          for (uint8_t k=0; k<VALUES_TO_READ_FROM_SENSOR_COUNT; k++) {
            TRACE_VERBOSE(F("%d\t"), values_readed_from_sensor[k]);
          }
          TRACE_VERBOSE(F("\r\n"));
          #endif
        }

        if (!is_temperature_redundant) {
          edata.value = RMAPDATA_MAX;
          edata.index = TEMPERATURE_REDUNDANT_INDEX;
          param.elaborataDataQueue->Enqueue(&edata, 0);
        }

        if (!is_humidity_redundant) {
          edata.value = RMAPDATA_MAX;
          edata.index = HUMIDITY_REDUNDANT_INDEX;
          param.elaborataDataQueue->Enqueue(&edata, 0);
        }

        state = END;
      break;

      case END:
        DelayUntil(Ticks::MsToTicks(*param.acquisition_delay_ms));
        state = SETUP;
      break;
    }
  }
}
