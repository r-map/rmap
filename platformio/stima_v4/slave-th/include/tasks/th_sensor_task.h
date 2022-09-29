/**@file th_sensor_task.h */

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

#ifndef _SENSOR_TASK_H
#define _SENSOR_TASK_H

#include "STM32FreeRTOS.h"
#include "thread.hpp"
#include "ticks.hpp"

#include "SensorDriver.h"
#include "debug.h"

typedef struct {
  uint8_t *sensors_count;
  sensor_configuration_t *sensors;
  uint32_t *acquisition_delay_ms;
} TemperatureHumidtySensorParam_t;

class TemperatureHumidtySensorTask : public cpp_freertos::Thread {
  typedef enum {
    INIT,
    SETUP,
    PREPARE,
    READ,
    END
  } State_t;

public:
  TemperatureHumidtySensorTask(const char *taskName, uint16_t stackSize, uint8_t priority, TemperatureHumidtySensorParam_t temperatureHumidtySensorParam);

protected:
  virtual void Run();


private:
  char taskName[configMAX_TASK_NAME_LEN];
  uint16_t stackSize;
  uint8_t priority;
  TemperatureHumidtySensorParam_t param;
  State_t state;
  SensorDriver *sensors[SENSORS_COUNT_MAX];
};

#endif
