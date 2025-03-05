/**
  ******************************************************************************
  * @file    soil_vwc_sensor_task.cpp
  * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Module sensor header file
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

#ifndef _SOIL_VWC_SENSOR_TASK_H
#define _SOIL_VWC_SENSOR_TASK_H

#include "config.h"
#include "debug_config.h"
#include "local_typedef.h"
#include "stima_utility.h"
#include "str.h"

#if (MODULE_TYPE == STIMA_MODULE_TYPE_VWC)

#define SOIL_VWC_TASK_LOW_POWER_ENABLED       (true)
#define SOIL_VWC_TASK_SWITCH_POWER_ENABLED    (true)
#define SOIL_VWC_TASK_WAIT_DELAY_MS           (50)
#define SOIL_VWC_TASK_GENERIC_RETRY_DELAY_MS  (5000)
#define SOIL_VWC_TASK_GENERIC_RETRY           (3)

// POWER SENSOR SWITCHING OR FIXED MODE...
#if (!SOIL_VWC_TASK_SWITCH_POWER_ENABLED)
// Timing to wakeUP only internal comparator ADC
#define SOIL_VWC_TASK_POWER_ON_WAIT_DELAY_MS  (5)
#define SOIL_VWC_FIXED_POWER_CHANEL_0         (false)
#define SOIL_VWC_FIXED_POWER_CHANEL_1         (false)
#define SOIL_VWC_FIXED_POWER_CHANEL_2         (false)
#define SOIL_VWC_FIXED_POWER_CHANEL_3         (false)
#else
// Timing to wakeUP exernal sensor if automatic powered
#define SOIL_VWC_TASK_POWER_ON_WAIT_DELAY_MS  (100)
#endif

/* Analog read resolution */
#define LL_ADC_RESOLUTION LL_ADC_RESOLUTION_12B
#define ADC_RANGE ADC_MAX
#define SAMPLES_REPETED_ADC 16

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

/// @brief struct local elaborate data parameter
typedef struct {
  configuration_t *configuration;                     //!< system configuration pointer struct
  system_status_t *system_status;                     //!< system status pointer struct
  cpp_freertos::BinarySemaphore *configurationLock;   //!< Semaphore to configuration access
  cpp_freertos::BinarySemaphore *systemStatusLock;    //!< Semaphore to system status access
  cpp_freertos::Queue *systemMessageQueue;            //!< Queue for system message
  cpp_freertos::Queue *elaborateDataQueue;            //!< Queue for elaborate data
} SoilVWCSensorParam_t;

/// @brief SENSOR TASK cpp_freertos class
class SoilVWCSensorTask : public cpp_freertos::Thread {
 
  /// @brief Enum for state switch of running method
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
  SoilVWCSensorTask(const char *taskName, uint16_t stackSize, uint8_t priority, SoilVWCSensorParam_t soilVWCSensorParam);

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
  
  static void resetADCData(uint8_t chanel_out);
  static uint8_t addADCData(uint8_t chanel_out);
  static float getADCData(uint8_t chanel_out, uint8_t *quality_data);

  int32_t getVrefTemp(void);
  
  float getAdcCalibratedValue(float adc_value, float offset, float gain);
  float getAdcAnalogValue(float adc_value, Adc_Mode adc_type);
  float getSoilVWC(float adc_value, float adc_voltage_min, float adc_voltage_max, bool *adc_overflow);

  // Global flag powered
  bool is_power_on[MAX_ADC_CHANELS];

  // Value of chanel ADC
  inline static uint8_t adc_in_count[MAX_ADC_CHANELS];
  inline static uint8_t adc_err_count[MAX_ADC_CHANELS];
  inline static uint64_t adc_in[MAX_ADC_CHANELS];

  State_t state;
  SoilVWCSensorParam_t param;
};

#endif
#endif