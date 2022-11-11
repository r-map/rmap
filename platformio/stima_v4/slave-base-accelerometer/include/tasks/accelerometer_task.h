/**@file accelerometer_task.h */

/*********************************************************************
Copyright (C) 2022  Moreno Gasperini <m.gasperini@digiteco.it>
authors:
Moreno Gasperini <m.gasperini@digiteco.it>

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

#ifndef _ACCELEROMETER_TASK_H
#define _ACCELEROMETER_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "STM32FreeRTOS.h"
#include "thread.hpp"
#include "ticks.hpp"
#include "drivers/module_slave_hal.hpp"

#if (ENABLE_I2C1 || ENABLE_I2C2)
#include <Wire.h>
#include "drivers/accelerometer.h"
#include "drivers/eeprom.h"
#endif

#include "debug_F.h"

typedef enum
{
  ACCELEROMETER_STATE_INIT,
  ACCELEROMETER_STATE_CHECK_OPERATION,
  ACCELEROMETER_STATE_LOAD_CONFIGURATION,
  ACCELEROMETER_STATE_SETUP_MODULE,
  ACCELEROMETER_STATE_READ,
  ACCELEROMETER_STATE_POWER_DOWN,
  ACCELEROMETER_STATE_SAVE_CONFIGURATION,
  ACCELEROMETER_STATE_WAIT_FOREVER  
} AccelerometerState_t;

typedef struct {
  accelerometer_t *configuration;
  BinarySemaphore *wireLock;
  BinarySemaphore *configurationLock;
  TwoWire *wire;
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
  AccelerometerParam_t AccelerometerParam;
  Accelerometer accelerometer;
  EEprom eeprom;
  float value_x;
  float value_y;
  float value_z;

  void PrintConfiguration(accelerometer_t *configuration, BinarySemaphore *lock);
  void LoadConfiguration(accelerometer_t *configuration, BinarySemaphore *lock);
  void SaveConfiguration(accelerometer_t *configuration, BinarySemaphore *lock, bool is_default);
  bool CheckModule(BinarySemaphore *lock);
  void SetupModule(accelerometer_t *configuration, BinarySemaphore *lock);
  void ReadModule(accelerometer_t *configuration, BinarySemaphore *lock);
  void PowerDownModule(accelerometer_t *configuration, BinarySemaphore *lock);

};

#endif
