/**
 ******************************************************************************
 * @file    lcd_task.cpp
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

#define TRACE_LEVEL LCD_TASK_TRACE_LEVEL

#include "tasks/lcd_task.h"

using namespace cpp_freertos;

LCDTask::LCDTask(const char *taskName, uint16_t stackSize, uint8_t priority, LCDParam_t LCDParam) : Thread(taskName, stackSize, priority), param(LCDParam)
{
  state = LCD_STATE_INIT;
  // Create LCD Access with Param Task
  u8g2 = U8G2_SH1108_128X160_F_FREERTOS_HW_I2C(U8G2_R1, param.wire, param.wireLock);
  Start();
};

void LCDTask::Run()
{
  // Loop Task
  while (true)
  {
    switch (state)
    {
    case LCD_STATE_INIT:
      TRACE_VERBOSE_F(F("LCD_STATE_INIT -> LCD_STATE_PRINT\r\n"));
      u8g2.begin();
      // Test Display Output
      u8g2.clearBuffer();					// clear the internal memory
      u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
      u8g2.drawStr(0,10,"Hello World!");	// write something to the internal memory
      u8g2.sendBuffer();					// transfer internal memory to the display
      state = LCD_STATE_PRINT;
      break;

    case LCD_STATE_PRINT:
      // check if display is on and print every LCD_TASK_PRINT_DELAY_MS some variables in system status
      Delay(Ticks::MsToTicks(LCD_TASK_PRINT_DELAY_MS));
      break;
    
    case LCD_STATE_END:
      // Display off...
      // Waiting for Resume...
      // TRACE_VERBOSE_F(F("LCD_STATE_END -> LCD_STATE_CHECK_OPERATION\r\n"));
      break;
    }
  }
}
