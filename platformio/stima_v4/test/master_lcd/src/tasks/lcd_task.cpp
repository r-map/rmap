/**
 ******************************************************************************
 * @file    lcd_task.cpp
 * @author  Cristiano Souza Paz <c.souzapaz@digiteco.it>
 * @brief   LCD Task based u8gl library
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (C) 2022 Cristiano Souza Paz <c.souzapaz@digiteco.it>
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
  // ************************* VARIABLES INITIALIZATION ***********************
  // **************************************************************************
  display_off();
  pression_event = false;
  rotation_event = false;

  // **************************************************************************
  // ************************* START TASK *************************************
  // **************************************************************************

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
        cursor_pos = 0;
        data_printed = false;
        encoder_state = DIR_NONE;
        selected_char_index = 0;
        stima4_master_command = MASTER_COMMAND_SDCARD;
        stima4_menu_ui = MAIN;
        stima4_slave_command = SLAVE_COMMAND_MAINTENANCE;

        // Count the number of board configurated
        for (uint8_t i = 0; i < BOARDS_COUNT_MAX; i++) {
          if (param.configuration->board_slave[i].module_type != Module_Type::undefined) {
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
              display_print_channel_interface(param.configuration->board_slave[channel].module_type);

              break;
            }

            case CONFIGURATION: {
              TRACE_INFO_F(F("LCD: CONFIGURATION\r\n"));

              // Calculate number of commands for master/each slave board
              if (stima4_menu_ui_last == MAIN) {
                commands_master_number = param.system_status->data_master.fw_upgradable == true ? (stima4_master_commands_t)MASTER_COMMAND_EXIT + 1 : (stima4_master_commands_t)MASTER_COMMAND_EXIT;
              } else {
                if (param.configuration->board_slave[channel].module_type == Module_Type::rain) {
                  commands_slave_number = param.system_status->data_slave[channel].fw_upgradable == true ? (stima4_slave_commands_t)SLAVE_COMMAND_EXIT + 1 : (stima4_slave_commands_t)SLAVE_COMMAND_EXIT;
                } else {
                  commands_slave_number = param.system_status->data_slave[channel].fw_upgradable == true ? (stima4_slave_commands_t)SLAVE_COMMAND_EXIT : (stima4_slave_commands_t)SLAVE_COMMAND_EXIT - 1;
                }
              }

              // Display CONFIGURATION MENU GENERAL interface
              display_print_config_menu_interface();

              break;
            }

            case UPDATE_NAME_STATION: {
              TRACE_INFO_F(F("LCD: UPDATE NAME STATION\r\n"));

              // Display CONFIGURATION MENU interface for UPDATE NAME STATION
              display_print_update_name_station_interface();

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
 * @brief Put off display
 *
 */
void LCDTask::display_off() {
  TRACE_INFO_F(F("LCD: Display OFF\r\n"));

  // Processing light of display
  display.noDisplay();

  // When the display turns on, it shows always the main interface
  display_print_default_interface();
  display_print_main_interface();

  // Turn low phisicals pins
  digitalWrite(PIN_ENCODER_EN5, LOW);
  digitalWrite(PIN_DSP_POWER, LOW);

  // Updating flags and states
  for (int i = 0; i < STATIONSLUG_LENGTH; i++) {
    new_station_name[i] = ' ';
  }
  cursor_pos = 0;
  display_is_off = true;
  param.systemStatusLock->Take();
  param.system_status->flags.display_on = false;
  param.systemStatusLock->Give();
  selected_char_index = 0;
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

  // Processing light of display
  display.display();

  // Updating flags and states
  display_is_off = false;
  last_display_timeout = millis();
  pression_event = false;
  rotation_event = false;
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
  bool bMeasValid_A = true, bMeasValid_B = true;
  bool printMeasB = false;
  char description_A[STIMA_LCD_DESCRIPTION_LENGTH], description_B[STIMA_LCD_DESCRIPTION_LENGTH];
  char measure_A[STIMA_LCD_MEASURE_LENGTH], measure_B[STIMA_LCD_MEASURE_LENGTH];
  char unit_type_A[STIMA_LCD_UNIT_TYPE_LENGTH], unit_type_B[STIMA_LCD_UNIT_TYPE_LENGTH];
  float value_display_A, value_display_B;
  uint8_t decimals_A, decimals_B;

  // Take the informations to print
  getStimaLcdDescriptionByType(description_A, description_B, module_type);
  getStimaLcdUnitTypeByType(unit_type_A, unit_type_B, module_type);
  getStimaLcdDecimalsByType(&decimals_A, &decimals_B, module_type);

  // Process string format to print
  if (param.system_status->data_slave[channel].is_online) {
    switch (param.system_status->data_slave[channel].module_type) {
      value_display_A = param.system_status->data_slave[channel].data_value_A;
      value_display_B = param.system_status->data_slave[channel].data_value_B;
      // Adjust UDM with comprensible value
      case Module_Type::th:
        printMeasB = true;
        value_display_A = param.system_status->data_slave[channel].data_value_A - 27315;
        value_display_A /= 100;
        value_display_B = param.system_status->data_slave[channel].data_value_B;
        if ((value_display_A < MIN_VALID_TEMPERATURE) || (value_display_A > MAX_VALID_TEMPERATURE)) bMeasValid_A = false;
        if ((value_display_B < MIN_VALID_HUMIDITY) || (value_display_B > MAX_VALID_HUMIDITY)) bMeasValid_B = false;
        break;
      default:
        value_display_A = param.system_status->data_slave[channel].data_value_A;
        value_display_B = param.system_status->data_slave[channel].data_value_B;
        break;
    }
    dtostrf(value_display_A, 0, decimals_A, measure_A);
    dtostrf(value_display_B, 0, decimals_B, measure_B);
  } else {
    bMeasValid_A = false;
    bMeasValid_B = false;
  }
  bMeasValid_A == true ? snprintf(measure_A, sizeof(measure_A), "%s %s", measure_A, unit_type_A) : snprintf(measure_A, sizeof(measure_A), "--- %s", unit_type_A);
  bMeasValid_B == true ? snprintf(measure_B, sizeof(measure_B), "%s %s", measure_B, unit_type_B) : snprintf(measure_B, sizeof(measure_B), "--- %s", unit_type_B);

  // Print description of measure
  display.setFont(u8g2_font_helvR08_tf);
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 0.5 * LINE_BREAK);
  display.print(description_A);

  // Print measurement information
  display.setFont(u8g2_font_helvR10_tf);
  display.setCursor(X_TEXT_FROM_RECT + 4 * STIMA_LCD_DESCRIPTION_LENGTH, Y_TEXT_FIRST_LINE + 0.5 * LINE_BREAK);
  display.print(measure_A);

  // Print the second measure for special cases
  if (printMeasB) {
    // Print description of measure
    display.setFont(u8g2_font_helvR08_tf);
    display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 2.5 * LINE_BREAK);
    display.print(description_B);

    // Print measurement information
    display.setCursor(X_TEXT_FROM_RECT + 4 * STIMA_LCD_DESCRIPTION_LENGTH, Y_TEXT_FIRST_LINE + 2.5 * LINE_BREAK);
    display.setFont(u8g2_font_helvR10_tf);
    display.print(measure_B);
  }

  // Print maintenance information if enabled
  if (param.system_status->data_slave[channel].maintenance_mode) {
    display.setFont(u8g2_font_open_iconic_embedded_2x_t);
    display.drawGlyph(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 7.75 * LINE_BREAK, U8G2_SYMBOL_MAINTENANCE);
    display.setFont(u8g2_font_helvR08_tf);
    display.setCursor(X_TEXT_SYSTEM_MESSAGE, Y_TEXT_FIRST_LINE + 7.5 * LINE_BREAK);
    display.print(F("Maintenance mode"));
  }

  // Print message of upgrading firmware when is running
  if (param.system_status->data_slave[channel].is_fw_upgrading) {
    display.setFont(u8g2_font_open_iconic_arrow_2x_t);
    display.drawGlyph(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 9.75 * LINE_BREAK, U8G2_SYMBOL_DOWNLOAD);
    display.setFont(u8g2_font_helvR08_tf);
    display.setCursor(X_TEXT_SYSTEM_MESSAGE, Y_TEXT_FIRST_LINE + 9.5 * LINE_BREAK);
    display.print(F("Firmware is upgrading..."));
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
      if ((!param.system_status->data_slave[channel].fw_upgradable && (stima4_slave_commands_t)i == SLAVE_COMMAND_FIRMWARE_UPGRADE) ||
          (param.configuration->board_slave[channel].module_type != Module_Type::rain && (stima4_slave_commands_t)i == SLAVE_COMMAND_CALIBRATION_ACCELEROMETER)) continue;
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
  display.setFont(u8g2_font_helvR08_tf);
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FROM_RECT);
  display.print(F("Stima: Digiteco-Arpae"));
}

/**
 * @brief Print Main interface with general information about station
 *
 */
void LCDTask::display_print_main_interface() {
  char dtIntest[18] = {0};
  char firmware_version[FIRMWARE_VERSION_LCD_LENGTH];
  char station[STATION_LCD_LENGTH];

  // Get Date and Time
  if (param.rtcLock->Take(Ticks::MsToTicks(RTC_WAIT_DELAY_MS))) {
    sprintf(dtIntest, "%02d/%02d/%02d %02d:%02d:00",
            rtc.getDay(), rtc.getMonth(), rtc.getYear(), rtc.getHours(), rtc.getMinutes());
    param.rtcLock->Give();
  }
  // Get Station name
  (void)snprintf(station, sizeof(station), "Station: %s", param.configuration->stationslug);
  // Get firmware version
  (void)snprintf(firmware_version, sizeof(firmware_version), "Firmware version: %d.%d", param.configuration->module_main_version, param.configuration->module_minor_version);

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
  param.system_status->flags.sd_card_ready == true ? display.print(F("SD card status: OK")) : display.print(F("SD card status: ERR"));

  // Print serial number
  display.drawFrame(X_RECT_SERIAL_NUMBER, Y_RECT_SERIAL_NUMBER, display.getWidth() - X_RECT_SERIAL_NUMBER_MARGIN, HEIGHT_RECT_SERIAL_NUMBER);
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 9.5 * LINE_BREAK);
  display.print(F("SN: "));

  for (int8_t id = 7; id >= 0; id--) {
    if ((uint8_t)((param.configuration->board_master.serial_number >> (8 * id)) & 0xFF) < 16) display.print(F("0"));
    display.print((uint8_t)((param.configuration->board_master.serial_number >> (8 * id)) & 0xFF), 16);
    if (id) display.print(F("-"));
  }

  // Apply the updates to display
  display.sendBuffer();
  display.clearBuffer();
}

/**
 * @brief Display the interface for update the name of station
 *
 */
void LCDTask::display_print_update_name_station_interface(void) {
  char buffer[STATIONSLUG_LENGTH];
  char status_message[20] = {0};

  // Get station name
  (void)snprintf(buffer, sizeof(buffer), "%s", new_station_name);

  // Print char selected
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE);
  display.print(F("Enter a name for the station "));
  // Print the buffer of station name
  display.setFont(u8g2_font_helvR10_tf);
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 4 * LINE_BREAK);
  display.print(buffer);

  switch (alphabet[selected_char_index]) {
    case '<': {
      strcpy(status_message, "Undo changes");
      display.setFont(u8g2_font_open_iconic_gui_2x_t);
      display.drawGlyph(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 9.75 * LINE_BREAK, U8G2_SYMBOL_UNDO);
      display.setCursor(X_TEXT_SYSTEM_MESSAGE, Y_TEXT_FIRST_LINE + 9.25 * LINE_BREAK);
      break;
    }
    case '>': {
      strcpy(status_message, "Apply changes");
      display.setFont(u8g2_font_open_iconic_gui_2x_t);
      display.drawGlyph(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 9.75 * LINE_BREAK, U8G2_SYMBOL_APPLY);
      display.setCursor(X_TEXT_SYSTEM_MESSAGE, Y_TEXT_FIRST_LINE + 9.25 * LINE_BREAK);
      break;
    }
    case '#': {
      strcpy(status_message, "Return to main");
      display.setFont(u8g2_font_open_iconic_arrow_2x_t);
      display.drawGlyph(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 9.75 * LINE_BREAK, U8G2_SYMBOL_EXIT);
      display.setCursor(X_TEXT_SYSTEM_MESSAGE, Y_TEXT_FIRST_LINE + 9.25 * LINE_BREAK);
      break;
    }
    default: {
      status_message[0] = alphabet[selected_char_index];
      display.drawFrame(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 8 * LINE_BREAK, 16, 16);
      display.setCursor(11, Y_TEXT_FIRST_LINE + 9.25 * LINE_BREAK);
      break;
    }
  }
  display.setFont(u8g2_font_helvR08_tf);
  display.print(status_message);

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
  system_message_t system_message = {0};

  switch (command) {
    case MASTER_COMMAND_SDCARD: {
      break;
    }
    case MASTER_COMMAND_UPDATE_NAME_STATION: {
      // Update the name of the station
      param.configurationLock->Take();
      strcpy(param.configuration->stationslug, new_station_name);
      param.configurationLock->Give();
      // Apply the updates to eeprom
      saveConfiguration();
      break;
    }
    case MASTER_COMMAND_FIRMWARE_UPGRADE: {
      // Set the queue to send
      system_message.task_dest = MMC_TASK_ID;
      system_message.command.do_update_fw = true;
      system_message.param = 0xFF;
      param.systemMessageQueue->Enqueue(&system_message, 0);
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
  system_message_t system_message = {0};

  switch (command) {
    case SLAVE_COMMAND_MAINTENANCE: {
      // Set the flag of maintenance
      param.systemStatusLock->Take();
      param.system_status->data_slave[channel].maintenance_mode = !param.system_status->data_slave[channel].maintenance_mode;
      param.systemStatusLock->Give();
      // Set the queue to send
      system_message.task_dest = CAN_TASK_ID;
      system_message.command.do_maint = true;
      system_message.param = channel;
      param.systemMessageQueue->Enqueue(&system_message, 0);
      break;
    }
    case SLAVE_COMMAND_CALIBRATION_ACCELEROMETER: {
      // Set the queue to send
      system_message.task_dest = CAN_TASK_ID;
      system_message.command.do_calib_acc = true;
      system_message.param = channel;
      param.systemMessageQueue->Enqueue(&system_message, 0);
      break;
    }
    case SLAVE_COMMAND_FIRMWARE_UPGRADE: {
      // Set the queue to send
      system_message.task_dest = CAN_TASK_ID;
      system_message.command.do_update_fw = true;
      system_message.param = channel;
      param.systemMessageQueue->Enqueue(&system_message, 0);
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
    case MASTER_COMMAND_UPDATE_NAME_STATION: {
      command_name = "Update station name";
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
    case SLAVE_COMMAND_CALIBRATION_ACCELEROMETER: {
      command_name = "Calibration";
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
 * @brief ISR handler for encoder input that manage the button pression
 *
 */
void LCDTask::ISR_input_pression_pin_encoder() {
  // **************************************************************************
  // ************************* DEBOUNCE BUTTON HANDLER ************************
  // **************************************************************************

  // Processing
  if (millis() - debounce_millis >= DEBOUNCE_TIMEOUT) {
    pression_event = digitalRead(PIN_ENCODER_INT) == LOW ? true : false;
  }

  // Updating flags and states
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

  // Updating flags and states
  last_display_timeout = millis();
}

/**
 * @brief Save new configuration to eeprom
 *
 */
bool LCDTask::saveConfiguration(void) {
  // Private param and Semaphore: param.configuration, param.configurationLock
  bool status = true;

  if (param.configurationLock->Take()) {
    // Write configuration to eeprom
    status = param.eeprom->Write(CONFIGURATION_EEPROM_ADDRESS, (uint8_t*)(param.configuration), sizeof(configuration_t));
    TRACE_INFO_F(F("LCD: Save configuration [ %s ]\r\n"), status ? OK_STRING : ERROR_STRING);
    param.configurationLock->Give();
  }

  return status;
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
      switch (stima4_menu_ui) {
          // ************************************************************************
          // ************************* CONFIG MENU MANAGEMENT ***********************
          // ************************************************************************

        case CONFIGURATION: {
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
            command_selector_pos = stima4_slave_command == SLAVE_COMMAND_EXIT ? commands_slave_number - 1 : command_selector_pos + 1;
            stima4_slave_command = stima4_slave_command == SLAVE_COMMAND_EXIT ? SLAVE_COMMAND_EXIT : (stima4_slave_commands_t)(stima4_slave_command + 1);
            if (!param.configuration->board_slave[channel].module_type != Module_Type::rain && stima4_slave_command == SLAVE_COMMAND_CALIBRATION_ACCELEROMETER) {
              stima4_slave_command = (stima4_slave_commands_t)(stima4_slave_command + 1);
            }
            if (!param.system_status->data_slave[channel].fw_upgradable && stima4_slave_command == SLAVE_COMMAND_FIRMWARE_UPGRADE) {
              stima4_slave_command = (stima4_slave_commands_t)(stima4_slave_command + 1);
            }
          }
          break;
        }

        case UPDATE_NAME_STATION: {
          selected_char_index = selected_char_index == ALPHABET_LENGTH - 1 ? 0 : selected_char_index + 1;
          break;
        }
          // **************************************************************************
          // ************************* CHANNELS MANAGEMENT ****************************
          // **************************************************************************

        default: {
          channel = channel == -1 ? 0 : channel + 1;
          stima4_menu_ui = channel == board_count ? MAIN : CHANNEL;
          channel = channel == board_count ? -1 : channel;
          break;
        }
      }
    }
    // **************************************************************************
    // ************************* COUNTER CLOCK WISE MANAGEMENT ******************
    // **************************************************************************

    else {
      switch (stima4_menu_ui) {
          // ************************************************************************
          // ************************* CONFIG MENU MANAGEMENT ***********************
          // ************************************************************************

        case CONFIGURATION: {
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
            command_selector_pos = stima4_slave_command == SLAVE_COMMAND_MAINTENANCE || command_selector_pos == 0 ? 0 : command_selector_pos - 1;
            stima4_slave_command = stima4_slave_command == SLAVE_COMMAND_MAINTENANCE ? SLAVE_COMMAND_MAINTENANCE : (stima4_slave_commands_t)(stima4_slave_command - 1);
            if (!param.configuration->board_slave[channel].module_type != Module_Type::rain && stima4_slave_command == SLAVE_COMMAND_CALIBRATION_ACCELEROMETER) {
              stima4_slave_command = (stima4_slave_commands_t)(stima4_slave_command - 1);
            }
            if (!param.system_status->data_slave[channel].fw_upgradable && stima4_slave_command == SLAVE_COMMAND_FIRMWARE_UPGRADE) {
              stima4_slave_command = (stima4_slave_commands_t)(stima4_slave_command - 1);
            }
          }
          break;
        }

        case UPDATE_NAME_STATION: {
          selected_char_index = selected_char_index == 0 ? ALPHABET_LENGTH - 1 : selected_char_index - 1;
          break;
        }
          // **************************************************************************
          // ************************* CHANNELS MANAGEMENT ****************************
          // **************************************************************************

        default: {
          channel = channel == -1 ? board_count - 1 : channel - 1;
          stima4_menu_ui = channel == -1 ? MAIN : CHANNEL;
          break;
        }
      }
    }

    // Updating flags and states
    data_printed = false;
  }

  // ************************************************************************
  // ************************* BUTTON HANDLER *******************************
  // ************************************************************************

  if (pression_event) {
    switch (stima4_menu_ui) {
        // ************************************************************************
        // ************************* CONFIG MENU MANAGEMENT ***********************
        // ************************************************************************

      case CONFIGURATION: {
        // ************************************************************************
        // ************************* ELABORATE COMMAND ****************************
        // ************************************************************************

        if (stima4_menu_ui_last == MAIN) {
          if (stima4_master_command == MASTER_COMMAND_UPDATE_NAME_STATION)
            stima4_menu_ui = UPDATE_NAME_STATION;
          else {
            elaborate_master_command(stima4_master_command);
            stima4_menu_ui = stima4_menu_ui_last;
          }
        } else {
          elaborate_slave_command(stima4_slave_command);
          stima4_menu_ui = stima4_menu_ui_last;
        }

        // Updating flags and states
        command_selector_pos = 0;
        stima4_master_command = MASTER_COMMAND_SDCARD;
        stima4_slave_command = SLAVE_COMMAND_MAINTENANCE;
        break;
      }

      case UPDATE_NAME_STATION: {
        // ************************************************************************
        // ************************* ELABORATE COMMAND ****************************
        // ************************************************************************

        switch (alphabet[selected_char_index]) {
          case '<': {
            cursor_pos = cursor_pos == 0 ? 0 : cursor_pos - 1;
            new_station_name[cursor_pos] = ' ';
            break;
          }
          case '>': {
            elaborate_master_command(MASTER_COMMAND_UPDATE_NAME_STATION);

            // Updating flags and states
            for (int i = 0; i < STATIONSLUG_LENGTH; i++) {
              new_station_name[i] = ' ';
            }
            cursor_pos = 0;
            selected_char_index = 0;
            stima4_menu_ui = stima4_menu_ui_last;
            break;
          }
          case '#': {
            // Updating flags and states
            for (int i = 0; i < STATIONSLUG_LENGTH; i++) {
              new_station_name[i] = ' ';
            }
            cursor_pos = 0;
            selected_char_index = 0;
            stima4_menu_ui = stima4_menu_ui_last;
            break;
          }
          default: {
            new_station_name[cursor_pos++] = alphabet[selected_char_index];
            if (cursor_pos == STATIONSLUG_LENGTH) {
              elaborate_master_command(MASTER_COMMAND_UPDATE_NAME_STATION);

              // Updating flags and states
              for (int i = 0; i < STATIONSLUG_LENGTH; i++) {
                new_station_name[i] = ' ';
              }
              cursor_pos = 0;
              selected_char_index = 0;
              stima4_menu_ui = stima4_menu_ui_last;
            }
            break;
          }
        }
        break;
      }

        // **************************************************************************
        // ************************* CHANNELS MANAGEMENT ****************************
        // **************************************************************************

      default: {
        stima4_menu_ui_last = stima4_menu_ui;
        stima4_menu_ui = CONFIGURATION;
        break;
      }
    }

    // Updating flags and states
    pression_event = false;
    data_printed = false;
  }

  // Updating flags and states
  encoder_state = DIR_NONE;
}

#endif
