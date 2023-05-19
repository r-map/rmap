/**
  ******************************************************************************
  * @file    wind_sensor_task.h
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @author  Moreno Gasperini <m.baldinetti@digiteco.it>
  * @brief   wind_sensor_task header file (Module sensor task acquire WindGill)
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

#ifndef _WIND_SENSOR_TASK_H
#define _WIND_SENSOR_TASK_H

#include "config.h"
#include "debug_config.h"
#include "local_typedef.h"
#include "stima_utility.h"
#include "str.h"

#if (MODULE_TYPE == STIMA_MODULE_TYPE_WIND)

#define WIND_TASK_POWER_ON_WAIT_DELAY_MS  (100)
#define WIND_TASK_WAIT_DELAY_MS           (50)
#define WIND_TASK_GENERIC_RETRY_DELAY_MS  (5000)
#define WIND_TASK_GENERIC_RETRY           (3)
#define WIND_TASK_LOW_POWER_ENABLED       (true)
#define WIND_TASK_ERROR_FOR_POWER_OFF     (5)

#define UART_RX_BUFFER_LENGTH             (24)

#define WINDSONIC_POLLED_MODE             (true)

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include <STM32LowPower.h>
#include "drivers/module_slave_hal.hpp"

#if (ENABLE_I2C1 || ENABLE_I2C2)
#include <Wire.h>
#endif

#include "debug_F.h"

// Local Serial for WindSonic
#define SerialWindSonic   Serial2

using namespace cpp_freertos;

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::Queue *systemMessageQueue;
  cpp_freertos::Queue *elaborataDataQueue;
} WindSensorParam_t;

class WindSensorTask : public cpp_freertos::Thread {
  typedef enum
  {
    SENSOR_STATE_CREATE,
    SENSOR_STATE_WAIT_CFG,
    SENSOR_STATE_INIT,
    #if (WINDSONIC_POLLED_MODE)
    SENSOR_STATE_SETUP,
    SENSOR_STATE_REQUEST,
    #endif
    SENSOR_STATE_WAIT_DATA,
    SENSOR_STATE_READING,
    SENSOR_STATE_ELABORATE,
    SENSOR_STATE_END
  } State_t;

public:
  WindSensorTask(const char *taskName, uint16_t stackSize, uint8_t priority, WindSensorParam_t windSensorParam);

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
  bool isWindOn();
  bool isWindOff();
  void serialReset();
  
  uint8_t windsonicInterpreter(float *speed, float *direction);

  bool is_power_on;
  bool is_error;
  float speed;
  float direction;

  State_t state;
  WindSensorParam_t param;
  uint8_t uart_rx_buffer_ptr;
  uint8_t uart_rx_buffer[UART_RX_BUFFER_LENGTH];
  bool is_wind_on;
};

#endif
#endif