/**
  ******************************************************************************
  * @file    th_sensor_task.h
  * @author  Marco Baldinett <m.baldinetti@digiteco.it>
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Driver Sensor acquire data, header file
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

#ifndef _TH_SENSOR_TASK_H
#define _TH_SENSOR_TASK_H

#include "config.h"
#include "debug_config.h"
#include "local_typedef.h"
#include "stima_utility.h"
#include "str.h"

#if ((MODULE_TYPE == STIMA_MODULE_TYPE_THR) || (MODULE_TYPE == STIMA_MODULE_TYPE_TH))

#define TH_TASK_POWER_ON_WAIT_DELAY_MS  (100)
#define TH_TASK_WAIT_DELAY_MS           (50)
#define TH_TASK_GENERIC_RETRY_DELAY_MS  (5000)
#define TH_TASK_GENERIC_RETRY           (3)
#define TH_TASK_LOW_POWER_ENABLED       (true)

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_slave_hal.hpp"

#if (ENABLE_I2C1 || ENABLE_I2C2)
#include <Wire.h>
#endif

#include "SensorDriver.h"
#include "debug_F.h"

using namespace cpp_freertos;

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  TwoWire *wire;
  cpp_freertos::BinarySemaphore *wireLock;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::Queue *systemMessageQueue;
  cpp_freertos::Queue *elaborateDataQueue;
} TemperatureHumidtySensorParam_t;

class TemperatureHumidtySensorTask : public cpp_freertos::Thread {
  typedef enum
  {
    SENSOR_STATE_CREATE,
    SENSOR_STATE_WAIT_CFG,
    SENSOR_STATE_INIT,
    SENSOR_STATE_SETUP,
    SENSOR_STATE_PREPARE,
    SENSOR_STATE_READ,
    SENSOR_STATE_END,
    SENSOR_STATE_CHECK_ERROR
  } State_t;

public:
  TemperatureHumidtySensorTask(const char *taskName, uint16_t stackSize, uint8_t priority, TemperatureHumidtySensorParam_t temperatureHumidtySensorParam);

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

  bool is_power_on;

  State_t state;
  TemperatureHumidtySensorParam_t param;
  SensorDriver *sensors[SENSORS_COUNT_MAX];
};

#endif
#endif