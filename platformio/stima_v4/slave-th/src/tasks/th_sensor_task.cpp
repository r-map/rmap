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
  int32_t values_readed_from_sensor[SENSORS_COUNT_MAX][VALUES_TO_READ_FROM_SENSOR_COUNT];
  uint32_t delay_ms;
  bool is_test;

  while (true) {
    switch (state) {
      case INIT:
        delay_ms = 0;
        is_test = false;

        TRACE_INFO(F("Initializing sensors...\r\n"));
        for (uint8_t i=0; i<*param.sensors_count; i++) {
          if (strlen(param.sensors[i].type) == 3) {
            SensorDriver::createSensor(SENSOR_DRIVER_I2C, param.sensors[i].type, param.sensors[i].i2c_address, 1, sensors, &Wire);
          }
        }
        state = SETUP;
      break;

      case SETUP:
        for (uint8_t i=0; i<SensorDriver::getSensorsCount(); i++) {
          if (!sensors[i]->isSetted()) {
            sensors[i]->setup();
            TRACE_INFO(F("--> %u: %s-%s 0x%02X [ %s ]\t [ %s ]\r\n"), i+1, SENSOR_DRIVER_I2C, sensors[i]->getType(), sensors[i]->getAddress(), param.sensors[i].is_redundant ? REDUNDANT_STRING : MAIN_STRING, sensors[i]->isSetted() ? OK_STRING : FAIL_STRING);
          }
        }
        state = PREPARE;
      break;

      case PREPARE:
        delay_ms = 0;
        for (uint8_t i=0; i<SensorDriver::getSensorsCount(); i++) {
          sensors[i]->resetPrepared();
          sensors[i]->prepare(is_test);

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
        for (uint8_t i=0; i<SensorDriver::getSensorsCount(); i++) {
          do {
            sensors[i]->get(&values_readed_from_sensor[i][0], VALUES_TO_READ_FROM_SENSOR_COUNT, is_test);
            if (sensors[i]->getDelay()) {
              DelayUntil(Ticks::MsToTicks(sensors[i]->getDelay()));
            }
          }
          while (!sensors[i]->isEnd() && !sensors[i]->isReaded());
        }
        state = END;
      break;

      case END:
        for (uint8_t i=0; i<SensorDriver::getSensorsCount(); i++) {
          for (uint8_t k=0; k<VALUES_TO_READ_FROM_SENSOR_COUNT; k++) {
            TRACE_INFO(F("%d\t"), values_readed_from_sensor[i][k]);
          }
          TRACE_INFO(F("\r\n"));
        }

        DelayUntil(Ticks::MsToTicks(*param.acquisition_delay_ms));
        state = SETUP;
      break;
    }
  }
}
