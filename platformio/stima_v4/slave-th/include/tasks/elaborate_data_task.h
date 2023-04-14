/**
  ******************************************************************************
  * @file    elaborate_data_task.h
  * @author  Marco Baldinett <m.baldinetti@digiteco.it>
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Elaborate data sensor to CAN header file
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

typedef struct {
  rmapdata_t values[SAMPLES_COUNT_MAX];   //!< samples buffer data values
  uint16_t count;                         //!< samples counter
  rmapdata_t *read_ptr;                   //!< reader pointer
  rmapdata_t *write_ptr;                  //!< writer pointer
} sample_t;

typedef struct {
  bool values[SAMPLES_COUNT_MAX];         //!< samples buffer maintenance values
  uint16_t count;                         //!< samples counter
  bool *read_ptr;                         //!< reader pointer
  bool *write_ptr;                        //!< writer pointer
} maintenance_t;

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::Queue *systemMessageQueue;
  cpp_freertos::Queue *elaborateDataQueue;
  cpp_freertos::Queue *requestDataQueue;
  cpp_freertos::Queue *reportDataQueue;
} ElaborateDataParam_t;

class ElaborateDataTask : public cpp_freertos::Thread {
  typedef enum {
    ELABORATE_DATA_CREATE,
    ELABORATE_DATA_INIT,
    ELABORATE_DATA_RUN,
    ELABORATE_DATA_SUSPEND
  } State_t;

public:
  ElaborateDataTask(const char *taskName, uint16_t stackSize, uint8_t priority, ElaborateDataParam_t elaboradeDataParam);

protected:
  virtual void Run();


private:

  #if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
  #endif
  void TaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  void make_report(bool is_init = true, uint16_t report_time_s = REPORTS_TIME_S, uint8_t observation_time_s = OBSERVATIONS_TIME_S);
  uint8_t checkTemperature(rmapdata_t main_remperature, rmapdata_t redundant_remperature);
  uint8_t checkHumidity(rmapdata_t main_humidity, rmapdata_t redundant_humidity);

  State_t state;
  ElaborateDataParam_t param;

  sample_t temperature_main_samples;
  sample_t temperature_redundant_samples;
  sample_t humidity_main_samples;
  sample_t humidity_redundant_samples;
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
