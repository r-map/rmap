/**
  ******************************************************************************
  * @file    accelerometer_task.h
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   accelerometer cpp_Freertos header file
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

#ifndef _ACCELEROMETER_TASK_H
#define _ACCELEROMETER_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "str.h"
#include "stima_utility.h"

#if (ENABLE_ACCELEROMETER)

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_slave_hal.hpp"

#if (ENABLE_I2C1 || ENABLE_I2C2)
#include <Wire.h>
#include "drivers/eeprom.h"
#include "drivers/accelerometer.h"
#endif

#include "debug_F.h"

using namespace cpp_freertos;

// Main TASK Switch Delay
#define ACELLEROMETER_TASK_WAIT_DELAY_MS    (10)
#define ACELLEROMETER_TASK_SLEEP_DELAY_MS   (1000)

typedef enum
{
  ACCELEROMETER_STATE_INIT,
  ACCELEROMETER_STATE_CHECK_OPERATION,
  ACCELEROMETER_STATE_LOAD_CONFIGURATION,
  ACCELEROMETER_STATE_SETUP_MODULE,
  ACCELEROMETER_STATE_READ,
  ACCELEROMETER_STATE_POWER_DOWN,
  ACCELEROMETER_STATE_SAVE_CONFIGURATION,
  ACCELEROMETER_STATE_WAIT_RESUME,
  ACCELEROMETER_STATE_HARDWARE_FAIL
} AccelerometerState_t;

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::Queue *systemMessageQueue;
  cpp_freertos::BinarySemaphore *wireLock;
  TwoWire *wire;
  accelerometer_t *accelerometer_configuration;
} AccelerometerParam_t;

class AccelerometerTask : public cpp_freertos::Thread {

public:
  AccelerometerTask(const char *taskName, uint16_t stackSize, uint8_t priority, AccelerometerParam_t AccelerometerParam);
  
protected:
  virtual void Run();

private:
  char taskName[configMAX_TASK_NAME_LEN];
  uint16_t stackSize;
  uint8_t priority;
  AccelerometerState_t state;
  AccelerometerParam_t param;
  Accelerometer accelerometer;
  EEprom eeprom;
  float value_x;
  float value_y;
  float value_z;

  void PrintConfiguration(accelerometer_t *configuration, BinarySemaphore *lock);
  bool LoadConfiguration(accelerometer_t *configuration, BinarySemaphore *lock);
  void SaveConfiguration(accelerometer_t *configuration, BinarySemaphore *lock, bool is_default);
  void Calibrate(accelerometer_t *configuration, BinarySemaphore *lock, bool is_default);
  bool CheckModule(BinarySemaphore *lock);
  void SetupModule(accelerometer_t *configuration, BinarySemaphore *lock);
  bool ReadModule(accelerometer_t *configuration, BinarySemaphore *lock);
  void PowerDownModule(accelerometer_t *configuration, BinarySemaphore *lock);

};

#endif
#endif