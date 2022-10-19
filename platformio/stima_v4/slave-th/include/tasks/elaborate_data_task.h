/**@file elaborate_data_task.h */

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

#ifndef _ELABORATE_DATA_TASK_H
#define _ELABORATE_DATA_TASK_H

#include "config.h"

#include "STM32FreeRTOS.h"
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"

#include "report.h"
#include "debug.h"

using namespace cpp_freertos;

typedef struct {
  rmapdata_t values[SAMPLES_COUNT_MAX];   //!< samples buffer
  uint16_t count;                         //!< samples counter
  rmapdata_t *read_ptr;                   //!< reader pointer
  rmapdata_t *write_ptr;                  //!< writer pointer
} sample_t;

typedef struct {
  uint32_t *acquisition_delay_ms;
  uint8_t *observation_time_s;
  Queue *elaborataDataQueue;
  Queue *requestDataQueue;
  Queue *reportDataQueue;
} ElaboradeDataParam_t;

class ElaborateDataSensorTask : public cpp_freertos::Thread {
  typedef enum {
    INIT,
    SETUP,
    PREPARE,
    READ,
    END
  } State_t;

public:
  ElaborateDataSensorTask(const char *taskName, uint16_t stackSize, uint8_t priority, ElaboradeDataParam_t elaboradeDataParam);

protected:
  virtual void Run();


private:
  char taskName[configMAX_TASK_NAME_LEN];
  uint16_t stackSize;
  uint8_t priority;
  ElaboradeDataParam_t param;
  State_t state;
  sample_t temperature_main_samples;
  sample_t temperature_redundant_samples;
  sample_t humidity_main_samples;
  sample_t humidity_redundant_samples;
  report_t report;

  void make_report(bool is_init = true, uint16_t report_time_s = REPORTS_TIME_S, uint8_t observation_time_s = OBSERVATRIONS_TIME_S);
  uint8_t checkTemperature(rmapdata_t main_remperature, rmapdata_t redundant_remperature);
  uint8_t checkHumidity(rmapdata_t main_humidity, rmapdata_t redundant_humidity);
};

template<typename buffer_g, typename length_v, typename value_v> value_v bufferRead(buffer_g *buffer, length_v length);
template<typename buffer_g, typename length_v, typename value_v> value_v bufferReadBack(buffer_g *buffer, length_v length);
template<typename buffer_g, typename value_v> void bufferWrite(buffer_g *buffer, value_v value);
template<typename buffer_g> void bufferPtrReset(buffer_g *buffer);
template<typename buffer_g, typename length_v> void bufferPtrResetBack(buffer_g *buffer, length_v length);
template<typename buffer_g, typename length_v> void incrementBuffer(buffer_g *buffer, length_v length);
template<typename buffer_g, typename length_v, typename value_v> void bufferReset(buffer_g *buffer, length_v length);
template<typename buffer_g, typename length_v, typename value_v> void addValue(buffer_g *buffer, length_v length, value_v value);

#endif
