/**
  ******************************************************************************
  * @file    lcd_task.hpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   LCD Task based u8gl library
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

#ifndef _LCD_TASK_H
#define _LCD_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "stima_utility.h"
#include "STM32FreeRTOS.h"
#include "thread.hpp"
#include "ticks.hpp"
#include "drivers/module_master_hal.hpp"
#include <U8g2lib.h>


#if (ENABLE_I2C1 || ENABLE_I2C2)
#include <Wire.h>
#include "drivers/eeprom.h"
#endif

#include "debug_F.h"

typedef enum
{
  LCD_STATE_INIT,
  LCD_STATE_CHECK_OPERATION,
  LCD_STATE_END
} LCDState_t;

typedef struct {
  configuration_t *configuration;
  BinarySemaphore *wireLock;
  TwoWire *wire;
} LCDParam_t;

class LCDTask : public cpp_freertos::Thread {

public:
  LCDTask(const char *taskName, uint16_t stackSize, uint8_t priority, LCDParam_t LCDParam);

protected:
  virtual void Run();

private:
  LCDState_t state;
  LCDParam_t LCDParam;
  U8G2_SH1108_128X160_F_FREERTOS_HW_I2C u8g2;
};

#endif
