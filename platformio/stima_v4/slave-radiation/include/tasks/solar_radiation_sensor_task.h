/**@file solar_radiation_sensor_task.h */

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

#ifndef _SOLAR_RADIATION_SENSOR_TASK_H
#define _SOLAR_RADIATION_SENSOR_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "stima_utility.h"
#include "str.h"

#if (MODULE_TYPE == STIMA_MODULE_TYPE_SOLAR_RADIATION)

#define SOLAR_RADIATION_TASK_POWER_ON_WAIT_DELAY_MS  (100)
#define SOLAR_RADIATION_TASK_WAIT_DELAY_MS           (50)
#define SOLAR_RADIATION_TASK_GENERIC_RETRY_DELAY_MS  (5000)
#define SOLAR_RADIATION_TASK_GENERIC_RETRY           (3)
#define SOLAR_RADIATION_TASK_LOW_POWER_ENABLED       (true)
#define SOLAR_RADIATION_TASK_ERROR_FOR_POWER_OFF     (SENSORS_COUNT_MAX * 3 * 2)

#define WAIT_QUEUE_REQUEST_ELABDATA_MS  (50)

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_slave_hal.hpp"

#if (ENABLE_I2C1 || ENABLE_I2C2)
#include <Wire.h>
#endif

// TODO:_TH_RAIN_
// #include "SensorDriver.h"
#include "debug_F.h"

using namespace cpp_freertos;

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::Queue *systemMessageQueue;
  cpp_freertos::Queue *elaborataDataQueue;
} SolarRadiationSensorParam_t;

class SolarRadiationSensorTask : public cpp_freertos::Thread {
  typedef enum
  {
    SENSOR_STATE_CREATE,
    SENSOR_STATE_WAIT_CFG,
    SENSOR_STATE_INIT,
    SENSOR_STATE_SET,
    SENSOR_STATE_EVALUATE,
    SENSOR_STATE_READ,
    SENSOR_STATE_END
  } State_t;

public:
  SolarRadiationSensorTask(const char *taskName, uint16_t stackSize, uint8_t priority, SolarRadiationSensorParam_t solarRadiationSensorParam);

protected:
  virtual void Run();

private:
  #if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
  #endif
  void TaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  void powerOn();
  void powerOff();
  float getAdcCalibratedValue(float adc_value, float offset, float gain);
  float getAdcAnalogValue(float adc_value, float min, float max);
  float getSolarRadiation(float adc_value, float adc_voltage_min, float adc_voltage_max);

  bool is_power_on;

  State_t state;
  SolarRadiationSensorParam_t param;
  //TODO:_TH_RAIN_
  //SensorDriver *sensors[SENSORS_COUNT_MAX];
};

#endif
#endif