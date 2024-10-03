/**
  ******************************************************************************
  * @file    accelerometer_task.h
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   accelerometer cpp_Freertos header file
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
#include "drivers/accelerometer.h"

#include <STM32RTC.h>

// Register EEprom
#include "register_class.hpp"

#include "debug_F.h"

using namespace cpp_freertos;

// Main TASK Switch Delay
#define ACCELEROMETER_TASK_WAIT_DELAY_MS    (20)
#define ACCELEROMETER_TASK_SLEEP_DELAY_MS   (850)

#define BUBBLE_ANGLE_ERROR                  (0.04)
#define BUBBLE_ANGLE_MIRROR                 (0.75)

#define BUBBLE_TIMINGS_VERIFY               (14400)

/// @brief struct local elaborate data parameter
typedef struct {
  configuration_t *configuration;                     //!< system configuration pointer struct
  system_status_t *system_status;                     //!< system status pointer struct
  TwoWire *wire;                                      //!< Local Wire access for sensor accelerometer
  cpp_freertos::BinarySemaphore *systemStatusLock;    //!< Semaphore to system status access
  cpp_freertos::BinarySemaphore *registerAccessLock;  //!< Semaphore to register Cyphal access
  cpp_freertos::BinarySemaphore *wireLock;            //!< Semaphore to Wire access for sensor accelerometer
  cpp_freertos::Queue *systemMessageQueue;            //!< Queue for system message
  EERegister *clRegister;                             //!< Object Register C++ access
} AccelerometerParam_t;

/// @brief ACCELEROMETER TASK cpp_freertos class
class AccelerometerTask : public cpp_freertos::Thread {

  /// @brief Enum for state switch of running method
  typedef enum
  {
    ACCELEROMETER_STATE_CREATE,
    ACCELEROMETER_STATE_INIT,
    ACCELEROMETER_STATE_CHECK_HARDWARE,
    ACCELEROMETER_STATE_LOAD_CONFIGURATION,
    ACCELEROMETER_STATE_SETUP_MODULE,
    ACCELEROMETER_STATE_CHECK_OPERATION,
    ACCELEROMETER_STATE_WAIT_RESUME,
    ACCELEROMETER_STATE_HARDWARE_FAIL
  } AccelerometerState_t;

public:
  AccelerometerTask(const char *taskName, uint16_t stackSize, uint8_t priority, AccelerometerParam_t AccelerometerParam);
  
protected:
  virtual void Run();

private:

  #if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
  #endif
  void TaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  void printConfiguration(void);
  void loadConfiguration(void);
  void saveConfiguration(bool is_default);
  void calibrate(bool is_default, bool save_register);
  bool checkModule(void);
  void setupModule(void);
  bool readModule(void);
  void powerDownModule(void);

  // Rtc check event access
  STM32RTC &rtc = STM32RTC::getInstance();

  AccelerometerState_t state;
  AccelerometerParam_t param;
  Accelerometer accelerometer;
  accelerometer_t accelerometer_configuration;
    
  // Value data
  float value_x;
  float value_y;
  float value_z;
};

#endif
#endif
