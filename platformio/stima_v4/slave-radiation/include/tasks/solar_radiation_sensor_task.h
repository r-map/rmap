/**
  ******************************************************************************
  * @file    solar_radiation_task.cpp
  * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Module sensor header file
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

#ifndef _SOLAR_RADIATION_SENSOR_TASK_H
#define _SOLAR_RADIATION_SENSOR_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "stima_utility.h"
#include "str.h"

#if (MODULE_TYPE == STIMA_MODULE_TYPE_SOLAR_RADIATION)

#define SOLAR_RADIATION_TASK_LOW_POWER_ENABLED       (true)
#define SOLAR_RADIATION_TASK_SWITCH_POWER_ENABLED    (false)
#define SOLAR_RADIATION_TASK_WAIT_DELAY_MS           (50)
#define SOLAR_RADIATION_TASK_GENERIC_RETRY_DELAY_MS  (5000)
#define SOLAR_RADIATION_TASK_GENERIC_RETRY           (3)
#define WAIT_QUEUE_REQUEST_ELABDATA_MS               (50)

// POWER SENSOR SWITCHING OR FIXED MODE...
#if (!SOLAR_RADIATION_TASK_SWITCH_POWER_ENABLED)
// Timing to wakeUP only internal comparator ADC
#define SOLAR_RADIATION_TASK_POWER_ON_WAIT_DELAY_MS  (5)
#define SOLAR_RADIATION_FIXED_POWER_CHANEL_0         (true)
#define SOLAR_RADIATION_FIXED_POWER_CHANEL_1         (false)
#define SOLAR_RADIATION_FIXED_POWER_CHANEL_2         (false)
#define SOLAR_RADIATION_FIXED_POWER_CHANEL_3         (false)
#else
// Timing to wakeUP exernal sensor if automatic powered
#define SOLAR_RADIATION_TASK_POWER_ON_WAIT_DELAY_MS  (100)
#endif

/* Analog read resolution */
#define LL_ADC_RESOLUTION LL_ADC_RESOLUTION_12B
#define ADC_RANGE ADC_MAX
#define SAMPLES_REPETED_ADC 64

#include "stm32yyxx_ll_adc.h"

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_slave_hal.hpp"

#if (ENABLE_I2C1 || ENABLE_I2C2)
#include <Wire.h>
#endif

#include "debug_F.h"

using namespace cpp_freertos;

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::Queue *systemMessageQueue;
  cpp_freertos::Queue *elaborataDataQueue;
} SolarRadiationSensorParam_t;

class SolarRadiationSensorTask : public cpp_freertos::Thread {
  typedef enum
  {
    SENSOR_STATE_CREATE,
    SENSOR_STATE_WAIT_CFG,
    SENSOR_STATE_INIT,
    SENSOR_STATE_SET,
    SENSOR_STATE_EVALUATE,
    SENSOR_STATE_READ,
    SENSOR_STATE_END
  } State_t;

public:
  SolarRadiationSensorTask(const char *taskName, uint16_t stackSize, uint8_t priority, SolarRadiationSensorParam_t solarRadiationSensorParam);

protected:
  virtual void Run();

private:
  #if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
  #endif
  void TaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  void powerOn(uint8_t chanel_out);
  void powerOff();
  
  void resetADCData(uint8_t chanel_out);
  uint8_t addADCData(uint8_t chanel_out);
  float getADCData(uint8_t chanel_out);

  int32_t getVrefTemp(void);
  
  float getAdcCalibratedValue(float adc_value, float offset, float gain);
  float getAdcAnalogValue(float adc_value, Adc_Mode adc_type);
  float getSolarRadiation(float adc_value, float adc_voltage_min, float adc_voltage_max);

  // Global flag powered
  bool is_power_on;

  // Value of chanel ADC
  uint8_t adc_in_count[MAX_ADC_CHANELS];
  uint64_t adc_in[MAX_ADC_CHANELS];

  State_t state;
  SolarRadiationSensorParam_t param;
};

#endif
#endif