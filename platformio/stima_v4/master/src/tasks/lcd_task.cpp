/**
 ******************************************************************************
 * @file    lcd_task.cpp
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
#define TRACE_LEVEL   LCD_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID LCD_TASK_ID

#include "tasks/lcd_task.h"

#if (ENABLE_LCD)

using namespace cpp_freertos;

LCDTask::LCDTask(const char* taskName, uint16_t stackSize, uint8_t priority, LCDParam_t LCDParam) : Thread(taskName, stackSize, priority), param(LCDParam) {
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(LCD_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  pin_bottom_left_encoder = PIN_ENCODER_A;
  pin_bottom_right_encoder = PIN_ENCODER_B;
  pin_top_left_encoder = PIN_ENCODER_INT;

  // **************************************************************************
  // ************************* TIMINGS SETUP **********************************
  // **************************************************************************

  debounce_millis = 0;
  last_display_timeout = millis();

  // **************************************************************************
  // ************************* ENCODER SETUP **********************************
  // **************************************************************************

  attachInterrupt(pin_bottom_left_encoder, ISR_input_rotation_pin_encoder, CHANGE);
  attachInterrupt(pin_bottom_right_encoder, ISR_input_rotation_pin_encoder, CHANGE);
  attachInterrupt(pin_top_left_encoder, ISR_input_pression_pin_encoder, CHANGE);

  // **************************************************************************
  // ************************* DISPLAY SETUP **********************************
  // **************************************************************************

  display = U8G2_SH1108_128X160_F_FREERTOS_HW_I2C(U8G2_R1, param.wire, param.wireLock);
  display_setup();

  // **************************************************************************
  // ************************* START TASK *************************************
  // **************************************************************************

  display_off();
  
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void LCDTask::TaskMonitorStack() {
  u_int16_t stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark(NULL);
  if ((stackUsage) && (stackUsage < param.system_status->tasks[LOCAL_TASK_ID].stack)) {
    param.systemStatusLock->Take();
    param.system_status->tasks[LOCAL_TASK_ID].stack = stackUsage;
    param.systemStatusLock->Give();
  }
}
#endif

/// @brief local watchDog and Sleep flag Task (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void LCDTask::TaskWatchDog(uint32_t millis_standby) {
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Update WDT Signal (Direct or Long function Timered)
  if (millis_standby) {
    // Check 1/2 Freq. controller ready to WDT only SET flag
    if ((millis_standby) < WDT_CONTROLLER_MS / 2) {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
    } else {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::timer;
      // Add security milimal Freq to check
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog_ms = millis_standby + WDT_CONTROLLER_MS;
    }
  } else {
    param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
  }
  param.systemStatusLock->Give();
}

/// @brief local suspend flag and positor running state Task (optional)
/// @param state_position Sw_Position (Local STATE)
/// @param state_subposition Sw_SubPosition (Optional Local SUB_STATE Position Monitor)
/// @param state_operation operative mode flag status for this task
void LCDTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation) {
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Signal Task sleep/disabled mode from request (Auto SET WDT on Resume)
  if ((param.system_status->tasks[LOCAL_TASK_ID].state == task_flag::suspended) &&
      (state_operation == task_flag::normal)) {
    param.system_status->tasks->watch_dog = wdt_flag::set;
  }
  param.system_status->tasks[LOCAL_TASK_ID].state = state_operation;
  param.system_status->tasks[LOCAL_TASK_ID].running_pos = state_position;
  param.system_status->tasks[LOCAL_TASK_ID].running_sub = state_subposition;
  param.systemStatusLock->Give();
}

void LCDTask::Run() {
// Start Running Monitor and First WDT normal state
#if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
#endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  display_off();

  while (true) {
// check if display is on and print every LCD_TASK_PRINT_DELAY_MS some variables in system status
#if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
#endif

    // One step base non blocking switch
    TaskWatchDog(LCD_TASK_WAIT_DELAY_MS);
    Delay(Ticks::MsToTicks(LCD_TASK_WAIT_DELAY_MS));
    TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

    // **************************************************************************
    // ************************* ROTATION HANDLER *******************************
    // **************************************************************************

    if (rotation_event) {
      // Reading pins from encoder
      encoder.pin.a = digitalRead(pin_bottom_left_encoder);
      encoder.pin.b = digitalRead(pin_bottom_right_encoder);

      if (encoder.pin_val != encoder_old.pin_val) {
        // Processing => DIRECTION: NONE, CLOCK WISE or COUNTER CLOCK WISE
        encoder_process(encoder.pin_val, encoder_old.pin_val);

        // Updating
        encoder_old.pin_val = encoder.pin_val;
        last_display_timeout = millis();
      }

      rotation_event = false;
    }

    // **************************************************************************
    // ************************* READ MILLISECONDS ******************************
    // **************************************************************************

    read_millis = millis();

    // **************************************************************************
    // ************************* DISPLAY OFF HANDLER ****************************
    // **************************************************************************

    if ((read_millis < last_display_timeout) || (read_millis - last_display_timeout) >= DISPLAY_OFF_TIMEOUT && !display_is_off) {
      display_off();
    }

    // **************************************************************************
    // ************************* DISPLAY ON HANDLER *****************************
    // **************************************************************************

    if (display_is_off && pression_event) {
      display_on();
    }

    // **************************************************************************
    // ************************* LCD HANDLER ************************************
    // **************************************************************************

    switch (state) {
      case LCD_STATE_INIT: {
        // **************************************************************************
        // ************************* VARIABLES INITIALIZATION ***********************
        // **************************************************************************

        board_count = 0;
        channel = -1;
        command_selector_pos = 0;
        data_printed = false;
        encoder_state = DIR_NONE;
        stima4_master_command = MASTER_COMMAND_SDCARD;
        stima4_menu_ui = MAIN;
        stima4_slave_command = SLAVE_COMMAND_MAINTENANCE;

        // Count the number of board configurated
        for (uint8_t i = 0; i < BOARDS_COUNT_MAX; i++) {
          if (param.system_status->data_slave[i].module_type != canardClass::Module_Type::undefined) {
            board_count++;
          }
        }

        // **************************************************************************
        // ************************* ENCODER INITIALIZATION *************************
        // **************************************************************************

        // Update the display with MAIN interface when display on after to press the button
        encoder.pin.a = digitalRead(pin_bottom_left_encoder);
        encoder.pin.b = digitalRead(pin_bottom_right_encoder);
        encoder_old.pin_val = encoder.pin_val;

        state = LCD_STATE_CHECK_OPERATION;
        break;
      }

      case LCD_STATE_CHECK_OPERATION: {
        // **************************************************************************
        // ************************* MENU HANDLER ***********************************
        // **************************************************************************

        if (!data_printed) {
          // Display default interface: title and decorations
          display_print_default_interface();

          switch (stima4_menu_ui) {
            case MAIN: {
              TRACE_INFO_F(F("LCD: MAIN\r\n"));

              // Display MAIN interface
              display_print_main_interface();

              break;
            }
            case CHANNEL: {
              TRACE_INFO_F(F("LCD: CHANNEL %d\r\n"), channel);

              // Display CHANNEL interface
              display_print_channel_interface(param.system_status->data_slave[channel].module_type);

              break;
            }
            case CONFIGURATION: {
              TRACE_INFO_F(F("LCD: CONFIGURATION\r\n"));

              // Calculate number of commands for master/each slave board
              if (stima4_menu_ui_last == MAIN) {
                commands_master_number = param.system_status->data_master.fw_upgradable == true ? 3 : 2;
              } else {
                commands_slave_number = param.system_status->data_slave[channel].fw_upgradable == true ? 3 : 2;
              }

              // Display CONFIGURATION MENU interface
              display_print_config_menu_interface();

              break;
            }
          }
          data_printed = true;
        }

        // Change the interface to print on display when there are the rotation or pression event on encoder
        if (data_printed) switch_interface();

        break;
      }

      case LCD_STATE_STANDBY: {
        break;
      }
    }
  }
}

/**
 * @brief Get the master command name from enumeration
 *
 * @param command master command enumeration
 * @return Command name in string format (const char*)
 */
const char* LCDTask::get_master_command_name_from_enum(stima4_master_commands_t command) {
  const char* command_name;
  switch (command) {
    case MASTER_COMMAND_SDCARD: {
      command_name = "Replacement SD card";
      break;
    }
    case MASTER_COMMAND_FIRMWARE_UPGRADE: {
      command_name = "Upgrade firmware";
      break;
    }
    case MASTER_COMMAND_EXIT: {
      command_name = "Exit";
      break;
    }
  }
  return command_name;
}

/**
 * @brief Get the slave command name from enumeration
 *
 * @param command slave command enumeration
 * @return Command name in string format (const char*)
 */
const char* LCDTask::get_slave_command_name_from_enum(stima4_slave_commands_t command) {
  const char* command_name;
  switch (command) {
    case SLAVE_COMMAND_MAINTENANCE: {
      command_name = "Maintenance";
      break;
    }
    case SLAVE_COMMAND_FIRMWARE_UPGRADE: {
      command_name = "Upgrade firmware";
      break;
    }
    case SLAVE_COMMAND_EXIT: {
      command_name = "Exit";
      break;
    }
  }
  return command_name;
}

/**
 * @brief Process the result of encoder rotation
 *
 * @param new_value new binary value of inputs encoder
 * @param old_value old binary value of inputs encoder
 */
void LCDTask::encoder_process(uint8_t new_value, uint8_t old_value) {
  switch (old_value) {
    case 0: {
      if (new_value == 1) {
        encoder_state = DIR_CLOCK_WISE;
      } else if (new_value == 2) {
        encoder_state = DIR_COUNTER_CLOCK_WISE;
      }
      break;
    }
    case 1: {
      if (new_value == 3) {
        encoder_state = DIR_CLOCK_WISE;
      } else if (new_value == 0) {
        encoder_state = DIR_COUNTER_CLOCK_WISE;
      }
      break;
    }
    case 2: {
      if (new_value == 0) {
        encoder_state = DIR_CLOCK_WISE;
      } else if (new_value == 3) {
        encoder_state = DIR_COUNTER_CLOCK_WISE;
      }
      break;
    }
    case 3: {
      if (new_value == 2) {
        encoder_state = DIR_CLOCK_WISE;
      } else if (new_value == 1) {
        encoder_state = DIR_COUNTER_CLOCK_WISE;
      }
      break;
    }
  }
}

/**
 * @brief ISR handler for encoder input that manage the button pression
 *
 */
void LCDTask::ISR_input_pression_pin_encoder() {
  // **************************************************************************
  // ************************* DEBOUNCE BUTTON HANDLER ************************
  // **************************************************************************

  if (millis() - debounce_millis >= DEBOUNCE_TIMEOUT) {
    // Processing
    pression_event = digitalRead(PIN_ENCODER_INT) == LOW ? true : false;
  }

  // Updating
  last_display_timeout = millis();
  debounce_millis = millis();
}

/**
 * @brief ISR handler for encoder inputs that manage the rotation
 *
 */
void LCDTask::ISR_input_rotation_pin_encoder() {
  // Processing
  rotation_event = true;
}

/**
 * @brief Put off display
 *
 */
void LCDTask::display_off() {
  TRACE_INFO_F(F("LCD: Display OFF\r\n"));

  param.systemStatusLock->Take();
  param.system_status->flags.display_on = false;
  param.systemStatusLock->Give();

  // Processing
  display.noDisplay();

  // Turn low phisicals pins
  digitalWrite(PIN_ENCODER_EN5, LOW);
  digitalWrite(PIN_DSP_POWER, LOW);

  // Updating
  display_is_off = true;
  state = LCD_STATE_STANDBY;
}

/**
 * @brief Put on display
 *
 */
void LCDTask::display_on() {
  TRACE_INFO_F(F("LCD: Display ON\r\n"));

  // Turn high phisicals pins
  digitalWrite(PIN_ENCODER_EN5, HIGH);
  digitalWrite(PIN_DSP_POWER, HIGH);

  // Processing
  display.display();

  // Updating
  display_is_off = false;
  last_display_timeout = millis();
  state = LCD_STATE_INIT;

  param.systemStatusLock->Take();
  param.system_status->flags.display_on = true;
  param.systemStatusLock->Give();
}

/**
 * @brief Rows with description, value and unity type of measurement
 *
 */
void LCDTask::display_print_channel_interface(uint8_t module_type) {
  char description[STIMA_LCD_DESCRIPTION_LENGTH];
  char unit_type[STIMA_LCD_UNIT_TYPE_LENGTH];
  char measure[STIMA_LCD_MEASURE_LENGTH];
  uint8_t decimals;
  bool bMeasValid_A = true;
  bool bMeasValid_B = true;

  // Take the informations to print
  getStimaLcdDescriptionByType(description, module_type);
  getStimaLcdUnitTypeByType(unit_type, module_type);
  getStimaLcdDecimalsByType(&decimals, module_type);

  // Process string format to print
  if (param.system_status->data_slave[channel].is_online) {
    float value_display = param.system_status->data_slave[channel].data_value_A;
    switch (param.system_status->data_slave[channel].module_type) {
      // Adjust UDM with comprensible value
      case canardClass::Module_Type::th:
        value_display = param.system_status->data_slave[channel].data_value_A - 27315;
        value_display /= 100;
        if (value_display < MIN_VALID_TEMPERATURE) bMeasValid_A = false;
        if (value_display > MAX_VALID_TEMPERATURE) bMeasValid_A = false;
        break;
      default:
        value_display = param.system_status->data_slave[channel].data_value_A;
        break;
    }
    dtostrf(value_display, 0, decimals, measure);
  } else {
    bMeasValid_A = false;
    bMeasValid_B = false;
  }
  if (bMeasValid_A) {
    snprintf(measure, sizeof(measure), "%s %s", measure, unit_type);
  } else {
    snprintf(measure, sizeof(measure), "--- %s", unit_type);
  }

  // Print description of measure
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE);
  display.print(description);

  // Print measurement information
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 2 * LINE_BREAK);
  display.setFont(u8g2_font_helvB12_tf);
  display.print(measure);

  // Print maintenance information if enabled
  if (param.system_status->data_slave[channel].maintenance_mode) {
    display.setFont(u8g2_font_helvR08_tf);
    display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 4 * LINE_BREAK);
    display.print(F("Maintenance mode"));
  }

  // Apply the updates to display
  display.sendBuffer();
  display.clearBuffer();
}

/**
 * @brief Rows with commands
 *
 */
void LCDTask::display_print_config_menu_interface() {
  // Index used for count printed rows
  uint8_t row_printed = 0;

  // Print a triangle to select the option
  display.drawTriangle(X_TEXT_FROM_RECT, Y_TOP_TRIANGLE + command_selector_pos * LINE_BREAK, X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 10 * command_selector_pos, X_PEAK_TRIANGLE, Y_PEAK_TRIANGLE + 10 * command_selector_pos);

  // Print command options
  if (stima4_menu_ui_last == MAIN) {
    for (uint8_t i = 0; i < (stima4_master_commands_t)MASTER_COMMAND_EXIT + 1; i++) {
      if (!param.system_status->data_master.fw_upgradable && (stima4_master_commands_t)i == MASTER_COMMAND_FIRMWARE_UPGRADE) continue;
      display.setCursor(X_TEXT_FROM_RECT_DESCRIPTION_COMMAND, Y_TEXT_FIRST_LINE + row_printed * LINE_BREAK);
      display.print(get_master_command_name_from_enum((stima4_master_commands_t)i));
      row_printed++;
    }
  } else {
    for (uint8_t i = 0; i < (stima4_slave_commands_t)SLAVE_COMMAND_EXIT + 1; i++) {
      if (!param.system_status->data_slave[channel].fw_upgradable && (stima4_slave_commands_t)i == SLAVE_COMMAND_FIRMWARE_UPGRADE) continue;
      display.setCursor(X_TEXT_FROM_RECT_DESCRIPTION_COMMAND, Y_TEXT_FIRST_LINE + row_printed * LINE_BREAK);
      display.print(get_slave_command_name_from_enum((stima4_slave_commands_t)i));
      row_printed++;
    }
  }

  // Apply the updates to display
  display.sendBuffer();
  display.clearBuffer();
}

/**
 * @brief Print default interface to always show
 *
 */
void LCDTask::display_print_default_interface() {
  display.drawFrame(X_RECT, Y_RECT, display.getWidth() - X_RECT_HEADER_MARGIN, display.getHeight() - Y_RECT_HEADER_MARGIN);
  display.drawHLine(X_RECT, Y_RECT_HEADER, display.getWidth() - X_RECT_HEADER_MARGIN);
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FROM_RECT);
  display.setFont(u8g2_font_helvR08_tf);
  display.print(F("Stima: Digiteco-Arpae"));
}

/**
 * @brief Print Main interface with general information about station
 *
 */
void LCDTask::display_print_main_interface() {
  char station[STATION_LCD_LENGTH];
  char firmware_version[FIRMWARE_VERSION_LCD_LENGTH];
  char dtIntest[18] = {0};

  // Get Station slug
  (void)snprintf(station, sizeof(station), "Station: %s", param.configuration->stationslug);
  // Get firmware version
  (void)snprintf(firmware_version, sizeof(firmware_version), "Firmware version: %d.%d", param.configuration->module_main_version, param.configuration->module_minor_version);
  // Get Date and Time
  if (param.rtcLock->Take(Ticks::MsToTicks(RTC_WAIT_DELAY_MS))) {
    sprintf(dtIntest, "%02d/%02d/%02d %02d:%02d:00",
            rtc.getDay(), rtc.getMonth(), rtc.getYear(), rtc.getHours(), rtc.getMinutes());
    param.rtcLock->Give();
  }

  // Print Date and Time
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE);
  display.print(dtIntest);

  // Print station name
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + LINE_BREAK);
  display.print(station);

  // Print firmware version
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 2 * LINE_BREAK);
  display.print(firmware_version);

  // Print SD card status
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 3 * LINE_BREAK);
  if (param.system_status->flags.sd_card_ready) {
    display.print("SD card status: OK");
  } else {
    display.print("SD card status: ERR");
  }

  // Apply the updates to display
  display.sendBuffer();
  display.clearBuffer();
}

/**
 * @brief Display setup handler
 *
 */
void LCDTask::display_setup() {
  display.begin();            // Initialize display
  display.enableUTF8Print();  // UTF8 support enabled
  display.clearBuffer();      // Clear the internal memory
}

/**
 * @brief Master command handler
 *
 * @param command type of elaboration based on master command
 */
void LCDTask::elaborate_master_command(stima4_master_commands_t command) {
  switch (command) {
    case MASTER_COMMAND_SDCARD: {
      break;
    }
    case MASTER_COMMAND_FIRMWARE_UPGRADE: {
      break;
    }
    case SLAVE_COMMAND_EXIT: {
      break;
    }
    default:
      break;
  }
}

/**
 * @brief Slave command handler
 *
 * @param command type of elaboration based on slave command
 */
void LCDTask::elaborate_slave_command(stima4_slave_commands_t command) {
  switch (command) {
    case SLAVE_COMMAND_MAINTENANCE: {
      param.systemStatusLock->Take();
      param.system_status->data_slave[channel].maintenance_mode = !param.system_status->data_slave[channel].maintenance_mode;
      param.systemStatusLock->Give();
      break;
    }
    case SLAVE_COMMAND_EXIT: {
      break;
    }
    default:
      break;
  }
}

/**
 * @brief Interface switch handler
 *
 */
void LCDTask::switch_interface() {
  // **************************************************************************
  // ************************* STATE HANDLER **********************************
  // **************************************************************************

  if (encoder_state) {
    // **************************************************************************
    // ************************* CLOCK WISE MANAGEMENT **************************
    // **************************************************************************

    if (encoder_state == DIR_CLOCK_WISE) {
      // **************************************************************************
      // ************************* CONFIG MENU MANAGEMENT *************************
      // **************************************************************************

      if (stima4_menu_ui == CONFIGURATION) {
        // **************************************************************************
        // ************************* MASTER CONFIG MENU *****************************
        // **************************************************************************

        if (stima4_menu_ui_last == MAIN) {
          command_selector_pos = stima4_master_command == MASTER_COMMAND_EXIT ? commands_master_number - 1 : command_selector_pos + 1;
          stima4_master_command = stima4_master_command == MASTER_COMMAND_EXIT ? MASTER_COMMAND_EXIT : (stima4_master_commands_t)(stima4_master_command + 1);
          if (!param.system_status->data_master.fw_upgradable && stima4_master_command == MASTER_COMMAND_FIRMWARE_UPGRADE) {
            stima4_master_command = (stima4_master_commands_t)(stima4_master_command + 1);
          }
        }
        // **************************************************************************
        // ************************* SLAVE CONFIG MENU ******************************
        // **************************************************************************

        else {
<<<<<<< Updated upstream
          command_selector_pos = stima4_slave_command == SLAVE_COMMAND_EXIT ? commands_slave_number - 1 : command_selector_pos + 1;
          stima4_slave_command = stima4_slave_command == SLAVE_COMMAND_EXIT ? SLAVE_COMMAND_EXIT : (stima4_slave_commands_t)(stima4_slave_command + 1);
          if (!param.system_status->data_slave[channel].fw_upgrade && stima4_slave_command == SLAVE_COMMAND_FIRMWARE_UPGRADE) {
=======
          command_selector_pos = stima4_slave_command == SLAVE_COMMAND_EXIT ? 0 : command_selector_pos + 1;
          stima4_slave_command = stima4_slave_command == SLAVE_COMMAND_EXIT ? SLAVE_COMMAND_MAINTENANCE : (stima4_slave_commands_t)(stima4_slave_command + 1);
          if (!param.system_status->data_slave[channel].fw_upgradable && stima4_slave_command == SLAVE_COMMAND_FIRMWARE_UPGRADE) {
>>>>>>> Stashed changes
            stima4_slave_command = (stima4_slave_commands_t)(stima4_slave_command + 1);
          }
        }

      }
      // **************************************************************************
      // ************************* CHANNELS MANAGEMENT ****************************
      // **************************************************************************

      else {
        channel = channel == -1 ? 0 : channel + 1;
        stima4_menu_ui = channel == board_count ? MAIN : CHANNEL;
        channel = channel == board_count ? -1 : channel;
      }
    }
    // **************************************************************************
    // ************************* COUNTER CLOCK WISE MANAGEMENT ******************
    // **************************************************************************

    else {
      // **************************************************************************
      // ************************* CONFIG MENU MANAGEMENT *************************
      // **************************************************************************

      if (stima4_menu_ui == CONFIGURATION) {
        // **************************************************************************
        // ************************* MASTER CONFIG MENU *****************************
        // **************************************************************************

        if (stima4_menu_ui_last == MAIN) {
          command_selector_pos = stima4_master_command == MASTER_COMMAND_SDCARD ? 0 : command_selector_pos - 1;
          stima4_master_command = stima4_master_command == MASTER_COMMAND_SDCARD ? MASTER_COMMAND_SDCARD : (stima4_master_commands_t)(stima4_master_command - 1);
          if (!param.system_status->data_master.fw_upgradable && stima4_master_command == MASTER_COMMAND_FIRMWARE_UPGRADE) {
            stima4_master_command = (stima4_master_commands_t)(stima4_master_command - 1);
          }
        }
        // **************************************************************************
        // ************************* SLAVE CONFIG MENU ******************************
        // **************************************************************************

        else {
          command_selector_pos = stima4_slave_command == SLAVE_COMMAND_MAINTENANCE ? 0 : command_selector_pos - 1;
          stima4_slave_command = stima4_slave_command == SLAVE_COMMAND_MAINTENANCE ? SLAVE_COMMAND_MAINTENANCE : (stima4_slave_commands_t)(stima4_slave_command - 1);
          if (!param.system_status->data_slave[channel].fw_upgradable && stima4_slave_command == SLAVE_COMMAND_FIRMWARE_UPGRADE) {
            stima4_slave_command = (stima4_slave_commands_t)(stima4_slave_command - 1);
          }
        }
      }
      // **************************************************************************
      // ************************* CHANNELS MANAGEMENT ****************************
      // **************************************************************************

      else {
        channel = channel == -1 ? board_count - 1 : channel - 1;
        stima4_menu_ui = channel == -1 ? MAIN : CHANNEL;
      }
    }
    data_printed = false;
  }

  // ************************************************************************
  // ************************* BUTTON HANDLER *******************************
  // ************************************************************************

  if (pression_event) {
    if (stima4_menu_ui != CONFIGURATION) {
      stima4_menu_ui_last = stima4_menu_ui;
      stima4_menu_ui = CONFIGURATION;
    } else {
      // ************************************************************************
      // ************************* ELABORATE COMMAND ****************************
      // ************************************************************************

      stima4_menu_ui == MAIN ? elaborate_master_command(stima4_master_command) : elaborate_slave_command(stima4_slave_command);

      command_selector_pos = 0;
      stima4_master_command = MASTER_COMMAND_SDCARD;
      stima4_slave_command = SLAVE_COMMAND_MAINTENANCE;
      stima4_menu_ui = stima4_menu_ui_last;
    }
    pression_event = false;
    data_printed = false;
  }

  encoder_state = DIR_NONE;
}

#endif
