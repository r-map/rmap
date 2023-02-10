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

#define LCD_TASK_PRINT_DELAY_MS   (5000)
#define LCD_TASK_WAIT_DELAY_MS    (10)

#include "display_config.hpp"

#include "debug_config.h"
#include "local_typedef.h"
#include "str.h"
#include "stima_utility.h"
#include "drivers/module_master_hal.hpp"

#include "STM32FreeRTOS.h"
#include "thread.hpp"
#include "ticks.hpp"
#include "queue.hpp"

#if (ENABLE_I2C1 || ENABLE_I2C2)
#include <Wire.h>
#include <U8g2lib.h>
#endif

#include "debug_F.h"

// CHECK*********

typedef enum LCDState {
  LCD_STATE_CREATE,
  LCD_STATE_INIT,
  LCD_STATE_CHECK_OPERATION,
  LCD_STATE_STANDBY
} LCDState_t;

typedef enum Stimacommands {
  MAINTENANCE,
  EXIT  // Always the latest element
} stima4_commands_t;

typedef enum Stimamenu {
  MAIN,
  CHANNEL,
  CONFIGURATION
} stima4_menu_ui_t;

typedef union Encoder {
  struct Pin {
    bool a : 1;
    bool b : 1;
  } pin;
  uint8_t pin_val;

} encoder_t;

typedef struct Channel {
  bool maintenance_mode;
  float value;
  uint8_t module_type;
} channel_t;

typedef struct Data {
  channel_t channel[MAX_CHANNELS];
} data_t;

// ********************

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::Queue *systemRequestQueue;
  cpp_freertos::Queue *systemResponseQueue;
  cpp_freertos::BinarySemaphore *wireLock;
  TwoWire *wire;
} LCDParam_t;

class LCDTask : public cpp_freertos::Thread {

public:
  LCDTask(const char *taskName, uint16_t stackSize, uint8_t priority, LCDParam_t LCDParam);

protected:
  virtual void Run();

private:

  #if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
  #endif
  void TaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

const char* get_command_name_from_enum(stima4_commands_t command);
  static void encoder_process(uint8_t new_value, uint8_t old_value);
  static void ISR_input_pression_pin_encoder(void);
  static void ISR_input_rotation_pin_encoder(void);
  void display_off(void);
  void display_on(void);
  void display_print_channel_interface(uint8_t module_type);
  void display_print_config_menu_interface(void);
  void display_print_default_interface(void);
  void display_print_main_interface(void);
  void display_setup(void);
  void elaborate_command(stima4_commands_t command);
  void switch_interface(void);

bool data_printed;
  bool display_is_off;
  char taskName[configMAX_TASK_NAME_LEN];
  data_t data = {
    .channel = {{false, 0, STIMA_MODULE_TYPE_RAIN},
                {false, 100, STIMA_MODULE_TYPE_LEAF},
                {false, 2.3, STIMA_MODULE_TYPE_WIND},
                {false, 453, STIMA_MODULE_TYPE_SOLAR_RADIATION}}};
  inline static bool pression_event, rotation_event;
  inline static encoder_t encoder, encoder_old;
  inline static uint32_t debounce_millis;
  inline static uint32_t last_display_timeout;
  inline static uint8_t encoder_state;
  int8_t channel;
  stima4_commands_t stima4_command;
  stima4_menu_ui_t stima4_menu_ui, stima4_menu_ui_last;
  U8G2_SH1108_128X160_F_FREERTOS_HW_I2C display;
  uint16_t stackSize;
  uint32_t read_millis;
  uint8_t priority;

  char pin_bottom_left_encoder;
  char pin_bottom_right_encoder;
  char pin_top_left_encoder;
 
  LCDState_t state;
  LCDParam_t param;
};

#endif
