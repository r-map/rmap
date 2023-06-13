/**
 ******************************************************************************
 * @file    lcd_task.hpp
 * @author  Cristiano Souza Paz <c.souzapaz@digiteco.it>
 * @brief   LCD Task based u8gl library
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (C) 2022 Cristiano Souza Paz <c.souzapaz@digiteco.it>
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
#include "str.h"

#if (ENABLE_LCD)

#include <STM32RTC.h>
#include <time.h>

#include "STM32FreeRTOS.h"
#include "display_config.hpp"
#include "drivers/eeprom.h"
#include "drivers/module_master_hal.hpp"
#include "queue.hpp"
#include "thread.hpp"
#include "ticks.hpp"

#if (ENABLE_I2C1 || ENABLE_I2C2)
#include <U8g2lib.h>
#include <Wire.h>
#endif

#include "debug_F.h"

// Limit range for module sensor (LCD)
#define MAX_VALID_TEMPERATURE (100.0)
#define MIN_VALID_TEMPERATURE (-50.0)
#define MAX_VALID_HUMIDITY    (100.0)
#define MIN_VALID_HUMIDITY    (0.0)

#define LCD_TASK_PRINT_DELAY_MS (5000)
#define LCD_TASK_WAIT_DELAY_MS  (10)

typedef enum LCDState {
  LCD_STATE_CREATE,
  LCD_STATE_INIT,
  LCD_STATE_CHECK_OPERATION,
  LCD_STATE_STANDBY
} LCDState_t;

typedef enum LCDMasterCommands {
  MASTER_COMMAND_DOWNLOAD_CFG,
  MASTER_COMMAND_RESET_FLAGS,
  MASTER_COMMAND_UPDATE_STATION_SLUG,
  MASTER_COMMAND_UPDATE_MQTT_USERNAME,
  MASTER_COMMAND_UPDATE_GSM_APN,
  MASTER_COMMAND_UPDATE_GSM_NUMBER,
  MASTER_COMMAND_UPDATE_PSK_KEY,
  MASTER_COMMAND_FIRMWARE_UPGRADE,
  MASTER_COMMAND_EXIT  // Always the latest element
} stima4_master_commands_t;

typedef enum LCDSlaveCommands {
  SLAVE_COMMAND_MAINTENANCE,
  SLAVE_COMMAND_RESET_FLAGS,
  SLAVE_COMMAND_CALIBRATION_ACCELEROMETER,
  SLAVE_COMMAND_FIRMWARE_UPGRADE,
  SLAVE_COMMAND_EXIT  // Always the latest element
} stima4_slave_commands_t;

typedef enum LCDMenu {
  MAIN,
  CHANNEL,
  CONFIGURATION,
  UPDATE_STATION_SLUG,
  UPDATE_MQTT_USERNAME,
  UPDATE_GSM_APN,
  UPDATE_GSM_NUMBER,
  UPDATE_PSK_KEY
} stima4_menu_ui_t;

typedef union Encoder {
  struct Pin {
    bool a : 1;
    bool b : 1;
  } pin;
  uint8_t pin_val;

} encoder_t;

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  bootloader_t *boot_request;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::BinarySemaphore *rtcLock;
  cpp_freertos::BinarySemaphore *wireLock;
  cpp_freertos::Queue *systemMessageQueue;
  cpp_freertos::Queue *dataLogPutQueue;
  cpp_freertos::Queue *displayEventWakeUp;
  EEprom *eeprom;
  TwoWire *wire;
} LCDParam_t;

class LCDTask : public cpp_freertos::Thread {
 public:
  LCDTask(const char *taskName, uint16_t stackSize, uint8_t priority, LCDParam_t lcdParam);

 protected:
  virtual void Run();

 private:
#if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
#endif
  void TaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  bool ASCIIHexToDecimal(char** str, uint8_t *value_out);
  bool saveConfiguration(void);
  const char *get_master_command_name_from_enum(stima4_master_commands_t command);
  const char *get_slave_command_name_from_enum(stima4_slave_commands_t command);
  static void encoder_process(uint8_t new_value, uint8_t old_value);
  static void ISR_input_pression_pin_encoder(void);
  static void ISR_input_rotation_pin_encoder(void);
  void display_off(void);
  void display_on(void);
  void display_print_channel_interface(uint8_t module_type);
  void display_print_config_menu_interface(void);
  void display_print_default_interface(void);
  void display_print_main_interface(void);
  void display_print_update_gsm_apn_interface(void);
  void display_print_update_gsm_number_interface(void);
  void display_print_update_mqtt_username_interface(void);
  void display_print_update_psk_key_interface(void);
  void display_print_update_station_slug_interface(void);
  void display_setup(void);
  void elaborate_master_command(stima4_master_commands_t command);
  void elaborate_slave_command(stima4_slave_commands_t command);
  void switch_interface(void);

  // Default char list for user input
  char alphabet[ALPHABET_LENGTH] = {
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', '<', '>', '!'};

  // GSM number char list for user input
  char alphabet_gsm_number[ALPHABET_GSM_NUMBER_LENGTH] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '#', '*', '<', '>', '!'};

  // PSK KEY char list for user input
  char alphabet_psk_key[ALPHABET_PSK_KEY_LENGTH] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '<', '>', '!'};

  // It contains the new psk key in char format inserted from user
  char new_client_psk_key[2 * CLIENT_PSK_KEY_LENGTH + 1];

  // It contains the new gsm apn inserted from user
  char new_gsm_apn[GSM_APN_LENGTH] = {0};

  // It contains the new gsm number inserted from user
  char new_gsm_number[GSM_NUMBER_LENGTH] = {0};

  // It contains the new mqtt username of station inserted from user
  char new_mqtt_username[MQTT_USERNAME_LENGTH] = {0};

  // It contains the new slug of station inserted from user
  char new_station_slug[STATIONSLUG_LENGTH] = {0};

  // Indicates whether the display has printed the updates or not
  bool data_printed;

  // Indicates whether the display is off or not
  bool display_is_off;

  // Indicates if the pressure event has occurred or not
  inline static bool pression_event;

  // Indicates if the rotation event has occurred or not
  inline static bool rotation_event;

  // It contains the current logic state of the encoder
  inline static encoder_t encoder;

  // It contains the old logic state of the encoder
  inline static encoder_t encoder_old;

  // The time in milliseconds for debounce management
  inline static uint32_t debounce_millis;

  // The last time in milliseconds from any interactions with encoder for power display management
  inline static uint32_t last_display_timeout;

  // It contains the final result of the encoder state
  inline static uint8_t encoder_state;

  // Index used for read the data from array of slave boards
  int8_t channel;

  // Used for master configuration menu management of commands
  stima4_master_commands_t stima4_master_command;

  // Used for slave configuration menu management of commands
  stima4_slave_commands_t stima4_slave_command;

  // Current menu state
  stima4_menu_ui_t stima4_menu_ui;

  // Last menu state before configuration state
  stima4_menu_ui_t stima4_menu_ui_last;

  // Display instance
  U8G2_SH1108_128X160_F_FREERTOS_HW_I2C display;

  // Contains the stack size allocated by LCD task
  uint16_t stackSize;

  // It contains the current time in milliseconds
  uint32_t read_millis;

  // Number of configurated boards
  uint8_t board_count;

  // Indicates the position of command selector in configuration menu
  uint8_t command_selector_pos;

  // Contains the number of commands available for master board
  uint8_t commands_master_number;

  // Contains the number of commands available for each slave board
  uint8_t commands_slave_number;

  // Used to calculate the y-axis position of cursor to enter the new char of new station name
  uint8_t cursor_pos;

  // Contains the priority assigned to LCD task
  uint8_t priority;

  // Index used to determine the char selected from user in update name station interface
  uint8_t selected_char_index;

  char pin_bottom_left_encoder;
  char pin_bottom_right_encoder;
  char pin_top_left_encoder;

  // Static access for event quque
  inline static cpp_freertos::Queue *localDisplayEventWakeUp;

  STM32RTC &rtc = STM32RTC::getInstance();

  LCDState_t state;
  LCDParam_t param;
};

#endif
#endif