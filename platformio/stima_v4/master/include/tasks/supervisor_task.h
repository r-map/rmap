/**
 ******************************************************************************
 * @file    supervisor_task.h
 * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
 * @author  Moreno Gasperini <m.gasperini@digiteco.it>
 * @brief   Supervisor check connection and config header file
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

#ifndef _SUPERVISOR_TASK_H
#define _SUPERVISOR_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "stima_utility.h"
#include "str.h"

#include <STM32RTC.h>

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_master_hal.hpp"

#define SUPERVISOR_TASK_WAIT_DELAY_MS           (10)
#define SUPERVISOR_TASK_GENERIC_RETRY_DELAY_MS  (5000)
#define SUPERVISOR_TASK_GENERIC_RETRY           (3)

#define SUPERVISOR_TASK_DEEP_POWER_DELAY_MS     (5000)
#define SUPERVISOR_TASK_SLEEP_DELAY_MS          (1000)

#define MIN_ATTEMPTED_CONNECTION_VALID          (5)
#define RANDOM_RUN_CONNECTION_SERVER_SEC        (60)

#if (ENABLE_I2C1 || ENABLE_I2C2)
#include <Wire.h>
#include "drivers/eeprom.h"
#endif

#include "register_class.hpp"

#include "debug_F.h"

/// @brief struct local elaborate data parameter
typedef struct {
  configuration_t *configuration;                     //!< system configuration pointer struct
  system_status_t *system_status;                     //!< system status pointer struct
  cpp_freertos::BinarySemaphore *configurationLock;   //!< Semaphore to configuration access
  cpp_freertos::BinarySemaphore *systemStatusLock;    //!< Semaphore to system status access
  cpp_freertos::BinarySemaphore *registerAccessLock;  //!< Semaphore to register Cyphal access
  cpp_freertos::Queue *systemMessageQueue;            //!< Queue for system message
  cpp_freertos::Queue *dataLogPutQueue;               //!< Queue for system logging put data
  cpp_freertos::Queue *connectionRequestQueue;        //!< Queue for connection Set request
  cpp_freertos::Queue *connectionResponseQueue;       //!< Queue for connection Get response
  EEprom *eeprom;                                     //!< Object EEprom C++ access
  EERegister *clRegister;                             //!< Object Register C++ access
} SupervisorParam_t;

/// @brief SUPERVISOR TASK cpp_freertos class
class SupervisorTask : public cpp_freertos::Thread {

  /// @brief Enum for state switch of running method
  typedef enum
  {
    SUPERVISOR_STATE_CREATE,
    SUPERVISOR_STATE_INIT,
    SUPERVISOR_STATE_LOAD_CONFIGURATION,
    SUPERVISOR_STATE_WAITING_EVENT,
    SUPERVISOR_STATE_CONNECTION_OPERATION,
    SUPERVISOR_STATE_SAVE_CONFIGURATION,
    SUPERVISOR_STATE_REQUEST_CONNECTION,
    SUPERVISOR_STATE_CHECK_CONNECTION,
    SUPERVISOR_STATE_DO_NTP,
    SUPERVISOR_STATE_DO_HTTP,
    SUPERVISOR_STATE_DO_MQTT,
    SUPERVISOR_STATE_REQUEST_DISCONNECTION,
    SUPERVISOR_STATE_CHECK_DISCONNECTION,
    SUPERVISOR_STATE_END
  } SupervisorState_t;

  /// @brief Enum for state switch of connection method
  typedef enum
  {
    CONNECTION_INIT,
    CONNECTION_CHECK,
    CONNECTION_CHECK_NTP,
    CONNECTION_CHECK_HTTP,
    CONNECTION_CHECK_MQTT,
    CONNECTION_END
  } SupervisorConnection_t;

public:
  SupervisorTask(const char *taskName, uint16_t stackSize, uint8_t priority, SupervisorParam_t supervisorParam);

protected:
  virtual void Run();

private:

  #if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
  #endif
  void TaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  void printConfiguration();
  bool loadConfiguration();
  bool saveConfiguration(bool is_default);

  STM32RTC &rtc = STM32RTC::getInstance();

  SupervisorState_t state;
  SupervisorParam_t param;
};

#endif
