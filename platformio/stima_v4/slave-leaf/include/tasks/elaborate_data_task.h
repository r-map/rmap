/**
  ******************************************************************************
  * @file    elaborate_data_task.h
  * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Elaborate data sensor to CAN header file
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

#ifndef _ELABORATE_DATA_TASK_H
#define _ELABORATE_DATA_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "stima_utility.h"
#include "str.h"

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_slave_hal.hpp"

#include "debug_F.h"

using namespace cpp_freertos;

// Main TASK Switch Delay
#define ELABORATE_TASK_WAIT_DELAY_MS      (20)
#define ELABORATE_TASK_SLEEP_DELAY_MS     (850)

/// @brief struct array for module data
typedef struct {
  rmapdata_t values[SAMPLES_COUNT_MAX];   //!< samples buffer data values
  uint16_t count;                         //!< samples counter
  rmapdata_t *read_ptr;                   //!< reader pointer
  rmapdata_t *write_ptr;                  //!< writer pointer
} sample_t;

/// @brief struct array for maintenance
typedef struct {
  bool values[SAMPLES_COUNT_MAX];         //!< samples buffer maintenance values
  uint16_t count;                         //!< samples counter
  bool *read_ptr;                         //!< reader pointer
  bool *write_ptr;                        //!< writer pointer
} maintenance_t;

/// @brief struct local elaborate data parameter
typedef struct {
  configuration_t *configuration;                       //!< system configuration pointer struct
  system_status_t *system_status;                       //!< system status pointer struct
  cpp_freertos::BinarySemaphore *configurationLock;     //!< semaphore access to configuration
  cpp_freertos::BinarySemaphore *systemStatusLock;      //!< semaphore access to system status
  cpp_freertos::Queue *systemMessageQueue;              //!< Queue for system message
  cpp_freertos::Queue *elaborateDataQueue;              //!< Queue for elaborate data
  cpp_freertos::Queue *requestDataQueue;                //!< Queue for request data
  cpp_freertos::Queue *reportDataQueue;                 //!< Queue for report data
} ElaborateDataParam_t;

/// @brief ELABORATE DATA TASK cpp_freertos class
class ElaborateDataTask : public cpp_freertos::Thread {

  /// @brief Enum for state switch of running method
  typedef enum {
    ELABORATE_DATA_CREATE,
    ELABORATE_DATA_INIT,
    ELABORATE_DATA_RUN,
    ELABORATE_DATA_SUSPEND
  } State_t;

public:
  ElaborateDataTask(const char *taskName, uint16_t stackSize, uint8_t priority, ElaborateDataParam_t elaborateDataParam);

protected:
  virtual void Run();


private:

  #if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
  #endif
  void TaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  void make_report(bool is_init = true, uint16_t report_time_s = REPORTS_TIME_S, uint8_t observation_time_s = OBSERVATIONS_TIME_S);
  uint8_t checkLeaf(rmapdata_t main_leaf);

  State_t state;
  ElaborateDataParam_t param;

  sample_t leaf_samples;

  maintenance_t maintenance_samples;
  report_t report;
};

template<typename buffer_g, typename length_v, typename value_v> value_v bufferRead(buffer_g *buffer, length_v length);
template<typename buffer_g, typename length_v, typename value_v> value_v bufferReadBack(buffer_g *buffer, length_v length);
template<typename buffer_g, typename value_v> void bufferWrite(buffer_g *buffer, value_v value);
template<typename buffer_g> void bufferPtrReset(buffer_g *buffer);
template<typename buffer_g, typename length_v> void bufferPtrResetBack(buffer_g *buffer, length_v length);
template<typename buffer_g, typename length_v> void incrementBuffer(buffer_g *buffer, length_v length);
template<typename buffer_g, typename length_v, typename value_v> void bufferReset(buffer_g *buffer, length_v length);
template<typename buffer_g, typename length_v, typename value_v>void addValue(buffer_g *buffer, length_v length, value_v value);

#endif
