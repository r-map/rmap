/**
 ******************************************************************************
 * @file    lcd_task.cpp
 * @author  Cristiano Souza Paz <c.souzapaz@digiteco.it>
 * @brief   LCD Task based u8gl library
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (C) 2022 Cristiano Souza Paz <c.souzapaz@digiteco.it></center></h2>
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

using namespace cpp_freertos;

#if (ENABLE_LCD)

/// @brief Construct a new LCD Task:: LCDTask object
/// @param taskName name of the task
/// @param stackSize size of the stack
/// @param priority priority of the task
/// @param lcdParam parameters for the task
LCDTask::LCDTask(const char *taskName, uint16_t stackSize, uint8_t priority, LCDParam_t lcdParam) : Thread(taskName, stackSize, priority), param(lcdParam) 
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(LCD_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  // Setting local static access parameter
  localDisplayEventWakeUp = param.displayEventWakeUp;

  pin_bottom_left_encoder = PIN_ENCODER_A;
  pin_bottom_right_encoder = PIN_ENCODER_B;
  pin_top_left_encoder = PIN_ENCODER_INT;

  // Timings setup
  debounce_millis = 0;
  last_display_timeout = millis();
  last_display_refresh = millis();

  // Encoder setup
  attachInterrupt(pin_bottom_left_encoder, ISR_input_rotation_pin_encoder, CHANGE);
  attachInterrupt(pin_bottom_right_encoder, ISR_input_rotation_pin_encoder, CHANGE);
  attachInterrupt(pin_top_left_encoder, ISR_input_pression_pin_encoder, CHANGE);

  // Display setup
  display = U8G2_SH1108_128X160_F_FREERTOS_HW_I2C(U8G2_R1, param.wire, param.wireLock);
  display_setup();

  // Init variables
  display_off();
  pression_event = false;

  // Start task
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void LCDTask::TaskMonitorStack() {
  uint16_t stackUsage = (uint16_t)uxTaskGetStackHighWaterMark(NULL);
  if ((stackUsage) && (stackUsage < param.system_status->tasks[LOCAL_TASK_ID].stack)) {
    param.systemStatusLock->Take();
    param.system_status->tasks[LOCAL_TASK_ID].stack = stackUsage;
    param.systemStatusLock->Give();
  }
}
#endif

/// @brief local watchDog and Sleep flag Task (optional)
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
    param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
  }
  param.system_status->tasks[LOCAL_TASK_ID].state = state_operation;
  param.system_status->tasks[LOCAL_TASK_ID].running_pos = state_position;
  param.system_status->tasks[LOCAL_TASK_ID].running_sub = state_subposition;
  param.systemStatusLock->Give();
}

/// @brief RUN Task
void LCDTask::Run() {
  bool event_wake_up;

// Start Running Monitor and First WDT normal state
#if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
#endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  while (true) {
// Check if display is on and print every LCD_TASK_PRINT_DELAY_MS some variables in system status
#if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
#endif

    // Security continuos flush event pression queue before suspend task
    if (!localDisplayEventWakeUp->IsEmpty()) {
      localDisplayEventWakeUp->Dequeue(&event_wake_up);
    }

    // One step base non blocking switch
    // Long time sleeping task on LCD Off mode
    if (display_is_off) {
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
      localDisplayEventWakeUp->Dequeue(&event_wake_up);
      TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
    } else {
      TaskWatchDog(LCD_TASK_WAIT_DELAY_MS);
      Delay(Ticks::MsToTicks(LCD_TASK_WAIT_DELAY_MS));
    }
    TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

    // **************************************************************************
    // ************************* READ MILLISECONDS ******************************
    // **************************************************************************

    read_millis = millis();

    // **************************************************************************
    // ************************* REFRESH LCD HANDLER ****************************
    // **************************************************************************

    if ((read_millis < last_display_refresh) || (read_millis - last_display_refresh) >= DISPLAY_REFRESH_TIMEOUT && !display_is_off) {
      data_printed = false;
    }

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
        // Waiting loading configuration complete before start application
        if (!param.system_status->configuration.is_loaded) {
          break;
        }

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
        stima4_master_command = MASTER_COMMAND_RESET_FLAGS;
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
        // ************************* REFRESH DISPLAY HANDLER ************************
        // **************************************************************************

        // Refresh display when a new data from current channel is available
        if (param.system_status->data_slave[channel].is_new_ist_data_ready) {
          param.systemStatusLock->Take();
          param.system_status->data_slave[channel].is_new_ist_data_ready = false;
          param.systemStatusLock->Give();
          data_printed = false;
        }

        // **************************************************************************
        // ***************************** MENU HANDLER *******************************
        // **************************************************************************

        if (!data_printed) {

          // Refresh millis page auto reload
          last_display_refresh = read_millis;

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

            case UPDATE_STATION_SLUG: {
              TRACE_INFO_F(F("LCD: UPDATE SLUG STATION\r\n"));

              // Display CONFIGURATION MENU interface for UPDATE_STATION_SLUG
              display_print_update_station_slug_interface();

              break;
            }

            case UPDATE_BOARD_SLUG: {
              TRACE_INFO_F(F("LCD: UPDATE SLUG BOARD\r\n"));

              // Display CONFIGURATION MENU interface for UPDATE_BOARD_SLUG
              display_print_update_board_slug_interface();

              break;
            }

            case UPDATE_MQTT_USERNAME: {
              TRACE_INFO_F(F("LCD: UPDATE MQTT USERNAME STATION\r\n"));

              // Display CONFIGURATION MENU interface for UPDATE_MQTT_USERNAME
              display_print_update_mqtt_username_interface();

              break;
            }

            case UPDATE_GSM_APN: {
              TRACE_INFO_F(F("LCD: UPDATE GSM APN\r\n"));

              // Display CONFIGURATION MENU interface for UPDATE_GSM_APN
              display_print_update_gsm_apn_interface();

              break;
            }

            #if (ENABLE_MENU_GSM_NUMBER)
            case UPDATE_GSM_NUMBER: {
              TRACE_INFO_F(F("LCD: UPDATE GSM NUMBER\r\n"));

              // Display CONFIGURATION MENU interface for UPDATE_GSM_NUMBER
              display_print_update_gsm_number_interface();

              break;
            }
            #endif

            case UPDATE_PSK_KEY: {
              TRACE_INFO_F(F("LCD: UPDATE PSK KEY\r\n"));

              // Display CONFIGURATION MENU interface for UPDATE_PSK_KEY
              display_print_update_psk_key_interface();

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

/// @brief Convert ASCII Hex 2 Format CHAR to uint8 value and increment string pointer to long string conversion (with error check)
/// @param str pointer to string (will be incremented if hex char are found and converted)
/// @param value_out pointer to data return value converted
/// @return true if error occurs. false if conversion is ready
bool LCDTask::ASCIIHexToDecimal(char** str, uint8_t* value_out) {
  bool is_error = false;

  if (isxdigit(**str)) {
    if (isdigit(**str)) {
      *value_out = **str - 48;
    } else {
      if (isupper(**str)) {
        *value_out = **str - 55;
      } else {
        *value_out = **str - 87;
      }
    }
    // Valid OK, Increment Char pointer
    *value_out <<= 4;
    (*str)++;
  } else
    is_error = true;

  if (!is_error && isxdigit(**str)) {
    if (isdigit(**str)) {
      *value_out += **str - 48;
    } else {
      if (isupper(**str)) {
        *value_out += **str - 55;
      } else {
        *value_out += **str - 87;
      }
    }
    // Valid OK, Increment Char pointer
    (*str)++;
  } else
    is_error = true;

  return is_error;
}

/// @brief Put off display
void LCDTask::display_off(void) {
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
    new_station_slug[i] = ' ';
  }
  cursor_pos = 0;
  display_is_off = true;
  param.systemStatusLock->Take();
  param.system_status->flags.display_on = false;
  param.systemStatusLock->Give();
  selected_char_index = 0;
  state = LCD_STATE_STANDBY;
}

/// @brief Put on display
void LCDTask::display_on(void) {
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
  state = LCD_STATE_INIT;
  param.systemStatusLock->Take();
  param.system_status->flags.display_on = true;
  param.systemStatusLock->Give();
}

/// @brief Rows with description, value and unity type of measurement
/// @param module_type The module type
void LCDTask::display_print_channel_interface(uint8_t module_type) {
  bool bMeasValid_A = true, bMeasValid_B = true, bMeasValid_C = true;
  bool printMeasB = false, printMeasC = false;
  char description_A[STIMA_LCD_DESCRIPTION_LENGTH], description_B[STIMA_LCD_DESCRIPTION_LENGTH], description_C[STIMA_LCD_DESCRIPTION_LENGTH];
  char measure_A[STIMA_LCD_MEASURE_LENGTH], measure_B[STIMA_LCD_MEASURE_LENGTH], measure_C[STIMA_LCD_MEASURE_LENGTH];
  char unit_type_A[STIMA_LCD_UNIT_TYPE_LENGTH], unit_type_B[STIMA_LCD_UNIT_TYPE_LENGTH], unit_type_C[STIMA_LCD_UNIT_TYPE_LENGTH];
  float value_display_A, value_display_B, value_display_C;
  uint8_t decimals_A, decimals_B, decimals_C;

  // Take the informations to print
  getStimaLcdDescriptionByType(description_A, description_B, description_C, module_type);
  getStimaLcdUnitTypeByType(unit_type_A, unit_type_B, unit_type_C, module_type);
  getStimaLcdDecimalsByType(&decimals_A, &decimals_B, &decimals_C, module_type);

  // Process string format to print
  if (param.system_status->data_slave[channel].is_online) {
    switch (param.system_status->data_slave[channel].module_type) {
      value_display_A = param.system_status->data_slave[channel].data_value[0];
      value_display_B = param.system_status->data_slave[channel].data_value[1];
      // Adjust UDM with comprensible value
      case Module_Type::th:
        printMeasB = true;
        value_display_A = param.system_status->data_slave[channel].data_value[0] - TEMPERATURE_OFFSET;
        value_display_A /= TEMPERATURE_SCALE;
        value_display_B = param.system_status->data_slave[channel].data_value[1];
        if ((value_display_A < MIN_VALID_TEMPERATURE) || (value_display_A > MAX_VALID_TEMPERATURE)) bMeasValid_A = false;
        if ((value_display_B < MIN_VALID_HUMIDITY) || (value_display_B > MAX_VALID_HUMIDITY)) bMeasValid_B = false;
        break;
      // Adjust UDM with comprensible value
      case Module_Type::rain:
        value_display_A = (float)param.system_status->data_slave[channel].data_value[0] / RAIN_GAUGE_SCALE;
        if ((value_display_A < MIN_VALID_RAIN) || (value_display_A > MAX_VALID_RAIN)) bMeasValid_A = false;
        break;
      // Adjust UDM with comprensible value
      case Module_Type::wind:
        printMeasB = true;
        value_display_A = (float)param.system_status->data_slave[channel].data_value[0] / WIND_SPEED_SCALE;
        value_display_B = (float)param.system_status->data_slave[channel].data_value[1];
        if ((value_display_A < MIN_VALID_WIND_SPEED) || (value_display_A > MAX_VALID_WIND_SPEED)) bMeasValid_A = false;
        if ((value_display_B < MIN_VALID_WIND_DIR) || (value_display_B > MAX_VALID_WIND_DIR)) bMeasValid_B = false;
        break;
      case Module_Type::radiation:
        value_display_A = param.system_status->data_slave[channel].data_value[0];
        if ((value_display_A < MIN_VALID_RADIATION) || (value_display_A > MAX_VALID_RADIATION)) bMeasValid_A = false;
        break;
      case Module_Type::level:
        value_display_A = param.system_status->data_slave[channel].data_value[0];
        if ((value_display_A < MIN_VALID_LEVEL) || (value_display_A > MAX_VALID_LEVEL)) bMeasValid_A = false;
        break;
      case Module_Type::power:
        printMeasB = true;
        printMeasC = true;
        value_display_A = (float)param.system_status->data_slave[channel].data_value[0];
        value_display_B = (float)param.system_status->data_slave[channel].data_value[1] / CURRENT_CHARGE_SCALE;
        value_display_C = (float)param.system_status->data_slave[channel].data_value[2];
        if ((value_display_A < MIN_VALID_POWER_CHG) || (value_display_A > MAX_VALID_POWER_CHG)) bMeasValid_A = false;
        if ((value_display_B < MIN_VALID_POWER_V) || (value_display_B > MAX_VALID_POWER_V)) bMeasValid_B = false;
        if ((value_display_C < MIN_VALID_POWER_I) || (value_display_C > MAX_VALID_POWER_I)) bMeasValid_C = false;
        break;
      case Module_Type::vwc:
        printMeasB = true;
        printMeasC = true;
        value_display_A = (float)param.system_status->data_slave[channel].data_value[0] / SOIL_MOISTURE_SCALE;
        value_display_B = (float)param.system_status->data_slave[channel].data_value[1] / SOIL_MOISTURE_SCALE;
        value_display_C = (float)param.system_status->data_slave[channel].data_value[2] / SOIL_MOISTURE_SCALE;
        if ((value_display_A < MIN_VALID_SOIL_MOISTURE) || (value_display_A > MAX_VALID_SOIL_MOISTURE)) bMeasValid_A = false;
        if ((value_display_B < MIN_VALID_SOIL_MOISTURE) || (value_display_B > MAX_VALID_SOIL_MOISTURE)) bMeasValid_B = false;
        if ((value_display_C < MIN_VALID_SOIL_MOISTURE) || (value_display_C > MAX_VALID_SOIL_MOISTURE)) bMeasValid_C = false;
        break;
      default:
        value_display_A = param.system_status->data_slave[channel].data_value[0];
        value_display_B = param.system_status->data_slave[channel].data_value[1];
        value_display_C = param.system_status->data_slave[channel].data_value[2];
        break;
    }
    dtostrf(value_display_A, 0, decimals_A, measure_A);
    if (printMeasB) {
      dtostrf(value_display_B, 0, decimals_B, measure_B);
    }
    if (printMeasC) {
      dtostrf(value_display_C, 0, decimals_C, measure_C);
    }
  } else {
    bMeasValid_A = false;
    bMeasValid_B = false;
    bMeasValid_C = false;
  }
  bMeasValid_A == true ? snprintf(measure_A, sizeof(measure_A), "%s %s", measure_A, unit_type_A) : snprintf(measure_A, sizeof(measure_A), "--- %s", unit_type_A);
  if (printMeasB) {
    bMeasValid_B == true ? snprintf(measure_B, sizeof(measure_B), "%s %s", measure_B, unit_type_B) : snprintf(measure_B, sizeof(measure_B), "--- %s", unit_type_B);
  }
  if (printMeasC) {
    bMeasValid_C == true ? snprintf(measure_C, sizeof(measure_C), "%s %s", measure_C, unit_type_C) : snprintf(measure_C, sizeof(measure_C), "--- %s", unit_type_C);
  }

#if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
#endif

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

  // Print the third measure for special cases
  if (printMeasC) {
    // Print description of measure
    display.setFont(u8g2_font_helvR08_tf);
    display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 4.5 * LINE_BREAK);
    display.print(description_C);

    // Print measurement information
    display.setCursor(X_TEXT_FROM_RECT + 4 * STIMA_LCD_DESCRIPTION_LENGTH, Y_TEXT_FIRST_LINE + 4.5 * LINE_BREAK);
    display.setFont(u8g2_font_helvR10_tf);
    display.print(measure_C);
  }

  // Print message of upgrading firmware when is running (prioritary)
  if (param.system_status->data_slave[channel].is_fw_upgrading) {
    display.setFont(u8g2_font_open_iconic_arrow_2x_t);
    display.drawGlyph(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 7.75 * LINE_BREAK, U8G2_SYMBOL_DOWNLOAD);
    display.setFont(u8g2_font_helvR08_tf);
    display.setCursor(X_TEXT_SYSTEM_MESSAGE, Y_TEXT_FIRST_LINE + 7.5 * LINE_BREAK);
    display.print(F("Firmware is upgrading..."));
  } else if (param.system_status->data_slave[channel].maintenance_mode) {
    // Print maintenance information if enabled
    display.setFont(u8g2_font_open_iconic_embedded_2x_t);
    display.drawGlyph(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 7.75 * LINE_BREAK, U8G2_SYMBOL_MAINTENANCE);
    display.setFont(u8g2_font_helvR08_tf);
    display.setCursor(X_TEXT_SYSTEM_MESSAGE, Y_TEXT_FIRST_LINE + 7.5 * LINE_BREAK);
    display.print(F("Maintenance mode"));
  } else {
    // Show Version and Revision actual for module on_line
    char firmware_version[FIRMWARE_VERSION_LCD_LENGTH];
    snprintf(firmware_version, sizeof(firmware_version), "Module version: %d.%d", param.system_status->data_slave[channel].module_version, param.system_status->data_slave[channel].module_revision);
    display.setFont(u8g2_font_helvR08_tf);
    display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 7.5 * LINE_BREAK);
    display.print(firmware_version);
  }

  // Apply the updates to display
  display.sendBuffer();
  display.clearBuffer();
}

/// @brief Show menu with commands list when press the button
void LCDTask::display_print_config_menu_interface(void) {
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

/// @brief Print default interface to always show. A simple rect with the header line
void LCDTask::display_print_default_interface(void) {
  display.drawFrame(X_RECT, Y_RECT, display.getWidth() - X_RECT_HEADER_MARGIN, display.getHeight() - Y_RECT_HEADER_MARGIN);
  display.drawHLine(X_RECT, Y_RECT_HEADER, display.getWidth() - X_RECT_HEADER_MARGIN);
  display.setFont(u8g2_font_helvR08_tf);
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FROM_RECT);
  display.print(F("Stima: Digiteco-Arpae"));
}

/// @brief Print Main interface with general information about station
void LCDTask::display_print_main_interface(void) {
  char buffer_errors[40] = {0};
  char msgOut[18] = {0};
  char errors[35] = {0};
  char firmware_version[FIRMWARE_VERSION_LCD_LENGTH];
  char station[STATION_LCD_LENGTH];
  bool is_error = false;
  uint8_t row_pos = 0;  // Incremental Row Position for info flag Message

  // Get Date and Time
  if (param.rtcLock->Take(Ticks::MsToTicks(RTC_WAIT_DELAY_MS))) {
    sprintf(msgOut, "%02d/%02d/%02d %02d:%02d:00",
            rtc.getDay(), rtc.getMonth(), rtc.getYear(), rtc.getHours(), rtc.getMinutes());
    param.rtcLock->Give();
  }

  // Get Station name
  snprintf(station, sizeof(station), "Slug: %s", param.configuration->stationslug);

  // Get firmware version
  snprintf(firmware_version, sizeof(firmware_version), "Firmware version: %d.%d", param.configuration->module_main_version, param.configuration->module_minor_version);

  // Print Date and Time
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE);
  display.print(msgOut);

  // Print station name
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + LINE_BREAK);
  display.print(station);

  // Print firmware version
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 2 * LINE_BREAK);
  display.print(firmware_version);

  // Print signal status
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 3 * LINE_BREAK);
  display.print(F("Signal status: "));
  display.drawFrame(X_TEXT_FROM_RECT + 65, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
  display.drawFrame(X_TEXT_FROM_RECT + 72, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
  display.drawFrame(X_TEXT_FROM_RECT + 79, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
  display.drawFrame(X_TEXT_FROM_RECT + 86, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
  display.drawFrame(X_TEXT_FROM_RECT + 93, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
  // With regular signal (>0)
  if (param.system_status->flags.gsm_rssi > 0) {
    if (param.system_status->flags.gsm_rssi <= 5) {
      display.drawBox(X_TEXT_FROM_RECT + 65, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
    } else if (param.system_status->flags.gsm_rssi <= 10) {
      display.drawBox(X_TEXT_FROM_RECT + 65, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
      display.drawBox(X_TEXT_FROM_RECT + 72, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
    } else if (param.system_status->flags.gsm_rssi <= 15) {
      display.drawBox(X_TEXT_FROM_RECT + 65, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
      display.drawBox(X_TEXT_FROM_RECT + 72, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
      display.drawBox(X_TEXT_FROM_RECT + 79, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
    } else if (param.system_status->modem.rssi <= 20) {
      display.drawBox(X_TEXT_FROM_RECT + 65, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
      display.drawBox(X_TEXT_FROM_RECT + 72, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
      display.drawBox(X_TEXT_FROM_RECT + 79, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
      display.drawBox(X_TEXT_FROM_RECT + 86, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
    } else {
      display.drawBox(X_TEXT_FROM_RECT + 65, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
      display.drawBox(X_TEXT_FROM_RECT + 72, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
      display.drawBox(X_TEXT_FROM_RECT + 79, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
      display.drawBox(X_TEXT_FROM_RECT + 86, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
      display.drawBox(X_TEXT_FROM_RECT + 93, Y_TEXT_FIRST_LINE + 2.4 * LINE_BREAK, 6, 6);
    }
  }

  // Print SD card status / connection Line
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 4 * LINE_BREAK);
  // Show connection state prioritary or SD Status Card
  if(param.system_status->flags.run_connection) {
    // Connection message running
    // Last message priority from Last to first (No message Disconnection, return to SD State)
    if(param.system_status->connection.is_mqtt_publishing_end) {
      display.print(F("Conn: mqtt publish OK"));
    }
    else if(param.system_status->connection.is_mqtt_publishing) {
      display.print(F("Conn: mqtt publishing..."));
    } 
    else if(param.system_status->connection.is_mqtt_connected) {
      display.print(F("Conn: mqtt connected OK"));
    }
    else if(param.system_status->connection.is_mqtt_connecting) {
      display.print(F("Conn: mqtt connection..."));
    }
    else if(param.system_status->connection.is_mqtt_connected) {
      display.print(F("Conn: mqtt connected OK"));
    }
    else if(param.system_status->connection.is_connected) {
      display.print(F("Conn: PPP connected OK"));
    }
    else if(param.system_status->connection.is_connecting) {
      display.print(F("Conn: ppp connection..."));
    }
  } else {
    // SD/Publish alternate Message without connection running
    if(!main_page_subinfo) {
      main_page_subinfo = 1;
      param.system_status->flags.sd_card_ready == true ? display.print(F("SD card status: OK")) : display.print(F("SD card status: ERR"));
    } else {
      main_page_subinfo = 0;
      sprintf(msgOut, "%07ld", param.system_status->connection.mqtt_data_published);
      display.print(F("Published ["));
      display.print(msgOut);
      display.print(F("] mqtt data"));
    }
  }

  // Print system status
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 5 * LINE_BREAK);
  display.print(F("System status: "));
  if (!param.system_status->flags.pnp_request &&
      !param.system_status->flags.fw_updating &&
      !param.system_status->flags.file_server_running &&
      !param.system_status->flags.ppp_error &&
      !param.system_status->flags.dns_error &&
      !param.system_status->flags.ntp_error &&
      !param.system_status->flags.mqtt_error &&
      !param.system_status->flags.http_error) {
    display.print(F(" OK"));
  } else {
    // Add type of diag message to buffer (Fw upgrade)
    if ((param.system_status->flags.fw_updating)&&(param.system_status->flags.file_server_running)) {
      strcat(errors, "Updating all firmware... ");
      // Remove Flag message (Always external SETTED continuos if DIAG message in running...)
      param.systemStatusLock->Take();
      param.system_status->flags.fw_updating = false;
      param.systemStatusLock->Give();
    } else {
      if (param.system_status->flags.fw_updating) {
        strcat(errors, "Updating master firmware... ");
        // Remove Flag message (Always external SETTED continuos if DIAG message in running...)
        param.systemStatusLock->Take();
        param.system_status->flags.fw_updating = false;
        param.systemStatusLock->Give();
      }
      if (param.system_status->flags.file_server_running) {
        strcat(errors, "Updating slave firmware... ");
        // No Remove Flag message (Used external SETTED continuos for operation...)
      }
    }
    // Plug and play
    if (param.system_status->flags.pnp_request) {
      strcat(errors, "pnp-");
      switch(param.system_status->flags.pnp_request) {
          case Module_Type::th:
              strcat(errors, "th ");
              break;
          case Module_Type::rain:
              strcat(errors, "rain ");
              break;
          case Module_Type::wind:
              strcat(errors, "wind ");
              break;
          case Module_Type::radiation:
              strcat(errors, "radiation ");
              break;
          case Module_Type::level:
              strcat(errors, "level ");
              break;
          case Module_Type::vwc:
              strcat(errors, "vwc ");
              break;
          case Module_Type::power:
              strcat(errors, "mppt ");
              break;
          default:
              strcat(errors, "unknown ");
              break;
      }
      // Remove Flag message (Always external SETTED continuos if DIAG message in running...)
      param.systemStatusLock->Take();
      param.system_status->flags.pnp_request = 0;
      param.systemStatusLock->Give();
    }
    // PPP Connection error, add type of error message to buffer
    if (param.system_status->flags.ppp_error) {
      is_error = true;
      // ppp error [rssi,ber XYZ] X=REGISTRATION [X]GSM [Y]GPRS [Z]EUTRAN (->9 NOT ATTEMPT)
      if(param.system_status->modem.creg_n > 9) param.system_status->modem.creg_n = 9;
      if(param.system_status->modem.cgreg_n > 9) param.system_status->modem.cgreg_n = 9;
      if(param.system_status->modem.cereg_n > 9) param.system_status->modem.cereg_n = 9;
      sprintf(msgOut, "ppp [%d,%d %d%d%d] ", param.system_status->modem.rssi, param.system_status->modem.ber,
        param.system_status->modem.creg_n, param.system_status->modem.cgreg_n, param.system_status->modem.cereg_n);
      strcat(errors, msgOut);
    }
    if (param.system_status->flags.dns_error) {
      is_error = true;
      strcat(errors, "dns ");
    }
    if (param.system_status->flags.ntp_error) {
      is_error = true;
      strcat(errors, "ntp ");
    }
    if (param.system_status->flags.mqtt_error) {
      is_error = true;
      strcat(errors, "mqtt ");
    }
    if (param.system_status->flags.http_error) {
      is_error = true;
      strcat(errors, "http");
    }
    // Dispaly Error or Diag Message
    if (is_error) {
      display.print(F("ERR"));
    } else {
      display.print(F("MSG"));
    }
    display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 6 * LINE_BREAK);
    snprintf(buffer_errors, sizeof(buffer_errors), "> %s", errors);
    display.print(buffer_errors);
  }

  // Security Remove flag config wait... Start success connection MQTT 
  if(param.system_status->flags.mqtt_wait_link) {    
    display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + (row_pos++ + 7.5) * LINE_BREAK);
    display.print(F("Waiting server connection..."));
  }

  // Print Wait configuration information
  if (param.system_status->flags.http_wait_cfg) {
    display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + (row_pos++ + 7.5) * LINE_BREAK);
    display.print(F("Waiting configuration..."));
  }

  // Print Wait download firmware information
  if (param.system_status->flags.http_wait_fw) {
    display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + (row_pos++ + 7.5) * LINE_BREAK);
    display.print(F("Waiting download firmware..."));
  }

  // Print serial number
  display.drawFrame(X_RECT_SERIAL_NUMBER, Y_RECT_SERIAL_NUMBER, display.getWidth() - X_RECT_SERIAL_NUMBER_MARGIN, HEIGHT_RECT_SERIAL_NUMBER);
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 9.5 * LINE_BREAK);
  display.print(F("SN: "));
  for (int8_t id = 7; id >= 0; id--) {
    if ((uint8_t)((param.configuration->board_master.serial_number >> (8 * id)) & 0xFF) < 16) display.print(F("0"));
    display.print((uint8_t)((param.configuration->board_master.serial_number >> (8 * id)) & 0xFF), 16);
    if (id) display.print(F("-"));
  }

#if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
#endif

  // Apply the updates to display
  display.sendBuffer();
  display.clearBuffer();
}

/// @brief Display the interface for update the board slug of station
void LCDTask::display_print_update_board_slug_interface(void) {
  char buffer[sizeof(new_board_slug)] = {0};
  char status_message[20] = {0};

  // Get parameter
  snprintf(buffer, sizeof(buffer), "%s", new_board_slug);

  // Print Title
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE);
  display.print(F("Enter board slug of the station"));

  // Print the buffer of parameter
  display.setFont(u8g2_font_helvR10_tf);
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 4 * LINE_BREAK);
  for (uint8_t i = 0; i < 16; i++) {
    display.print(buffer[i]);
  }
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 6 * LINE_BREAK);
  for (uint8_t i = 16; i < sizeof(new_board_slug); i++) {
    display.print(buffer[i]);
  }

  // Print char selected
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
    case '!': {
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

/// @brief Display the interface for update the GSM APN
void LCDTask::display_print_update_gsm_apn_interface(void) {
  char buffer[sizeof(new_gsm_apn)] = {0};
  char status_message[20] = {0};

  // Get parameter
  snprintf(buffer, sizeof(buffer), "%s", new_gsm_apn);

  // Print Title
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE);
  display.print(F("Enter GSM APN"));

  // Print the buffer of parameter
  display.setFont(u8g2_font_helvR10_tf);
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 4 * LINE_BREAK);
  for (uint8_t i = 0; i < 16; i++) {
    display.print(buffer[i]);
  }
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 6 * LINE_BREAK);
  for (uint8_t i = 16; i < sizeof(new_gsm_apn); i++) {
    display.print(buffer[i]);
  }

  // Print char selected
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
    case '!': {
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

/// @brief Display the interface for update the GSM NUMBER
#if (ENABLE_MENU_GSM_NUMBER)
void LCDTask::display_print_update_gsm_number_interface(void) {
  char buffer[sizeof(new_gsm_number)] = {0};
  char status_message[20] = {0};

  // Get parameter
  snprintf(buffer, sizeof(buffer), "%s", new_gsm_number);

  // Print Title
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE);
  display.print(F("Enter GSM number"));

  // Print the buffer of parameter
  display.setFont(u8g2_font_helvR10_tf);
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 4 * LINE_BREAK);
  for (uint8_t i = 0; i < 16; i++) {
    display.print(buffer[i]);
  }
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 6 * LINE_BREAK);
  for (uint8_t i = 16; i < sizeof(new_gsm_number); i++) {
    display.print(buffer[i]);
  }

  // Print char selected
  switch (alphabet_gsm_number[selected_char_index]) {
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
    case '!': {
      strcpy(status_message, "Return to main");
      display.setFont(u8g2_font_open_iconic_arrow_2x_t);
      display.drawGlyph(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 9.75 * LINE_BREAK, U8G2_SYMBOL_EXIT);
      display.setCursor(X_TEXT_SYSTEM_MESSAGE, Y_TEXT_FIRST_LINE + 9.25 * LINE_BREAK);
      break;
    }
    default: {
      status_message[0] = alphabet_gsm_number[selected_char_index];
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
#endif

/// @brief Display the interface for update the mqtt username of station
void LCDTask::display_print_update_mqtt_username_interface(void) {
  char buffer[sizeof(new_mqtt_username)] = {0};
  char status_message[20] = {0};

  // Get parameter
  snprintf(buffer, sizeof(buffer), "%s", new_mqtt_username);

  // Print title
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE);
  display.print(F("Enter mqtt username"));

  // Print the buffer of parameter
  display.setFont(u8g2_font_helvR10_tf);
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 4 * LINE_BREAK);
  for (uint8_t i = 0; i < 16; i++) {
    display.print(buffer[i]);
  }
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 6 * LINE_BREAK);
  for (uint8_t i = 16; i < sizeof(new_mqtt_username); i++) {
    display.print(buffer[i]);
  }

  // Print char selected
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
    case '!': {
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

/// @brief Display the interface for update the PSK KEY
void LCDTask::display_print_update_psk_key_interface(void) {
  char buffer[sizeof(new_client_psk_key)] = {0};
  char status_message[20] = {0};

  // Get parameter
  snprintf(buffer, sizeof(buffer), "%s", new_client_psk_key);

  // Print Title
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE);
  display.print(F("Enter PSK KEY"));

  // Print the buffer of parameter
  display.setFont(u8g2_font_helvR10_tf);
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 4 * LINE_BREAK);
  for (uint8_t i = 0; i < 16; i++) {
    display.print(buffer[i]);
  }
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 6 * LINE_BREAK);
  for (uint8_t i = 16; i < sizeof(new_client_psk_key); i++) {
    display.print(buffer[i]);
  }

  // Print char selected
  switch (alphabet_psk_key[selected_char_index]) {
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
    case '!': {
      strcpy(status_message, "Return to main");
      display.setFont(u8g2_font_open_iconic_arrow_2x_t);
      display.drawGlyph(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 9.75 * LINE_BREAK, U8G2_SYMBOL_EXIT);
      display.setCursor(X_TEXT_SYSTEM_MESSAGE, Y_TEXT_FIRST_LINE + 9.25 * LINE_BREAK);
      break;
    }
    default: {
      status_message[0] = alphabet_psk_key[selected_char_index];
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

/// @brief Display the interface for update the slug of station
void LCDTask::display_print_update_station_slug_interface(void) {
  char buffer[sizeof(new_station_slug)] = {0};
  char status_message[20] = {0};

  // Get parameter
  snprintf(buffer, sizeof(buffer), "%s", new_station_slug);

  // Print Title
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE);
  display.print(F("Enter slug of the station"));

  // Print the buffer of parameter
  display.setFont(u8g2_font_helvR10_tf);
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 4 * LINE_BREAK);
  for (uint8_t i = 0; i < 16; i++) {
    display.print(buffer[i]);
  }
  display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 6 * LINE_BREAK);
  for (uint8_t i = 16; i < sizeof(new_station_slug); i++) {
    display.print(buffer[i]);
  }

  // Print char selected
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
    case '!': {
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

/// @brief Display setup handler
void LCDTask::display_setup(void) {
  display.begin();            // Initialize display
  display.enableUTF8Print();  // UTF8 support enabled
  display.clearBuffer();      // Clear the internal memory
}

/// @brief Master command handler
/// @param command type of elaboration based on master command
void LCDTask::elaborate_master_command(stima4_master_commands_t command) {
  system_message_t system_message = {0};

  TRACE_INFO_F(F("LCD: Command to elaborate \"[ %s ]\"\r\n"), get_master_command_name_from_enum(command));

  switch (command) {
    case MASTER_COMMAND_RESET_FLAGS: {
      // Set the request on system status to reset flags
      param.systemStatusLock->Take();
      param.system_status->modem.connection_attempted = 0;
      param.system_status->modem.connection_completed = 0;
      param.system_status->modem.perc_modem_connection_valid = 100;
      param.system_status->connection.mqtt_data_exit_error = 0;
      param.systemStatusLock->Give();
      if(param.boot_request->tot_reset || param.boot_request->wdt_reset) {
        // Reset counter on new or restored firmware
        param.boot_request->tot_reset = 0;
        param.boot_request->wdt_reset = 0;
        // Save info bootloader block
        param.eeprom->Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*)param.boot_request, sizeof(bootloader_t));
      }
      break;
    }
    case MASTER_COMMAND_FORCE_CONNECTION: {
      // Set the request on system status to force connection request and synch NTP
      param.systemStatusLock->Take();
      param.system_status->command.do_ntp_synchronization = true;
      param.system_status->command.do_mqtt_connect = true;
      param.system_status->flags.mqtt_wait_link = true;
      param.systemStatusLock->Give();
      break;
    }
    case MASTER_COMMAND_DOWNLOAD_CFG: {
      // Set the request on system status to force connection request
      param.systemStatusLock->Take();
      param.system_status->command.do_http_configuration_update = true;
      param.system_status->flags.http_wait_cfg = true;
      param.systemStatusLock->Give();
      break;
    }
    case MASTER_COMMAND_DOWNLOAD_FW: {
      // Set the request on system status to force connection request
      param.systemStatusLock->Take();
      param.system_status->command.do_http_firmware_download = true;
      param.system_status->flags.http_wait_fw = true;
      param.systemStatusLock->Give();
      break;
    }
    case MASTER_COMMAND_TRUNCATE_DATA: {
      // Set the queue to send
      system_message.task_dest = SD_TASK_ID;
      system_message.command.do_trunc_sd = true;
      param.systemMessageQueue->Enqueue(&system_message, 0);
      break;
    }
    case MASTER_COMMAND_UPDATE_STATION_SLUG: {
      // Update the slug of the station
      param.configurationLock->Take();
      strcpy(param.configuration->stationslug, new_station_slug);
      param.configurationLock->Give();
      // Apply the updates to eeprom
      saveConfiguration();
      break;
    }
    #if(ENABLE_MENU_BOARD_SLUG)
    case MASTER_COMMAND_UPDATE_BOARD_SLUG: {
      // Update the board slug of the station
      param.configurationLock->Take();
      strcpy(param.configuration->board_master.boardslug, new_board_slug);
      param.configurationLock->Give();
      // Apply the updates to eeprom
      saveConfiguration();
      break;
    }
    #endif
    case MASTER_COMMAND_UPDATE_MQTT_USERNAME: {
      // Update the mqtt username of the station
      param.configurationLock->Take();
      strcpy(param.configuration->mqtt_username, new_mqtt_username);
      param.configurationLock->Give();
      // Apply the updates to eeprom
      saveConfiguration();
      break;
    }
    case MASTER_COMMAND_UPDATE_GSM_APN: {
      // Update the mqtt password of the station
      param.configurationLock->Take();
      strcpy(param.configuration->gsm_apn, new_gsm_apn);
      param.configurationLock->Give();
      // Apply the updates to eeprom
      saveConfiguration();
      break;
    }
    #if (ENABLE_MENU_GSM_NUMBER)
    case MASTER_COMMAND_UPDATE_GSM_NUMBER: {
      // Update the mqtt password of the station
      param.configurationLock->Take();
      strcpy(param.configuration->gsm_number, new_gsm_number);
      param.configurationLock->Give();
      // Apply the updates to eeprom
      saveConfiguration();
      break;
    }
    #endif
    case MASTER_COMMAND_UPDATE_PSK_KEY: {
      bool end_conversion = false;
      const char* ptr_read = new_client_psk_key;  // Point to PSK_KEY String NOT 0x-> but Direct
      uint8_t byte_pos = 0;
      uint8_t data_read;
      // Read all HexASCII (2Char for each Time) and Put into (serial_number) at power Byte byte_pos
      // Start from MSB to LSB. Terminate if All Byte expected was read or Error Char into Input String
      // Or Input String is terminated. Each character !" HEX_TIPE (0..9,A..F) terminate function
      // Hex string can be shorter than expected. Value are convert as UINT_64 MSB Left Formatted
      param.configurationLock->Take();
      // Reset PSK_KEY
      memset(param.configuration->client_psk_key, 0, CLIENT_PSK_KEY_LENGTH);
      while ((byte_pos != CLIENT_PSK_KEY_LENGTH) && !end_conversion) {
        end_conversion = ASCIIHexToDecimal((char**)&ptr_read, &data_read);
        param.configuration->client_psk_key[byte_pos++] = data_read;
      }
      param.configurationLock->Give();
      // Apply the updates to eeprom
      saveConfiguration();
      break;
    }
    case MASTER_COMMAND_FIRMWARE_UPGRADE: {
      // Set the queue to send
      system_message.task_dest = SD_TASK_ID;
      system_message.command.do_update_fw = true;
      system_message.node_id = CMD_PARAM_MASTER_ADDRESS;
      param.systemMessageQueue->Enqueue(&system_message, 0);
      break;
    }
    case MASTER_COMMAND_EXIT: {
      break;
    }
    default:
      break;
  }
}

/// @brief Slave command handler
/// @param command type of elaboration based on slave command
void LCDTask::elaborate_slave_command(stima4_slave_commands_t command) {
  system_message_t system_message = {0};

  TRACE_INFO_F(F("LCD: Command to elaborate \"[ %s ]\"\r\n"), get_slave_command_name_from_enum(command));

  switch (command) {
    case SLAVE_COMMAND_MAINTENANCE: {
      // Set the flag of maintenance (inverted from remote read or local save value)
      param.systemStatusLock->Take();
      param.system_status->data_slave[channel].maintenance_mode = !param.system_status->data_slave[channel].maintenance_mode;
      param.systemStatusLock->Give();
      // Set the queue to send
      system_message.task_dest = CAN_TASK_ID;
      if(param.system_status->data_slave[channel].maintenance_mode) {
        system_message.command.do_maint = true;
      } else {
        system_message.command.undo_maint = true;
      }
      system_message.node_id = channel;
      param.systemMessageQueue->Enqueue(&system_message, 0);
      break;
    }
    case SLAVE_COMMAND_RESET_FLAGS: {
      // Set the queue to send
      system_message.task_dest = CAN_TASK_ID;
      system_message.command.do_reset_flags = true;
      system_message.node_id = channel;
      param.systemMessageQueue->Enqueue(&system_message, 0);
      break;
    }
    case SLAVE_COMMAND_DO_FACTORY: {
      // Set the queue to send
      system_message.task_dest = CAN_TASK_ID;
      system_message.command.do_factory = true;
      system_message.node_id = channel;
      param.systemMessageQueue->Enqueue(&system_message, 0);
      break;
    }
    case SLAVE_COMMAND_CALIBRATION_ACCELEROMETER: {
      // Set the queue to send
      system_message.task_dest = CAN_TASK_ID;
      system_message.command.do_calib_acc = true;
      system_message.node_id = channel;
      param.systemMessageQueue->Enqueue(&system_message, 0);
      break;
    }
    case SLAVE_COMMAND_FIRMWARE_UPGRADE: {
      // Set the queue to send
      system_message.task_dest = CAN_TASK_ID;
      system_message.command.do_update_fw = true;
      system_message.node_id = channel;
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

/// @brief Process the result of encoder rotation
/// @param new_value new binary value of inputs encoder
/// @param old_value old binary value of inputs encoder
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

/// @brief Get the master command name from enumeration
/// @param command master command enumeration
/// @return Command name in string format (const char*)
const char* LCDTask::get_master_command_name_from_enum(stima4_master_commands_t command) {
  const char* command_name;
  switch (command) {
    case MASTER_COMMAND_RESET_FLAGS: {
      command_name = "Reset flags";
      break;
    }
    case MASTER_COMMAND_FORCE_CONNECTION: {
      command_name = "Start server connection";
      break;
    }
    case MASTER_COMMAND_DOWNLOAD_CFG: {
      command_name = "Download configuration";
      break;
    }
    case MASTER_COMMAND_DOWNLOAD_FW: {
      command_name = "Download firmware";
      break;
    }
    case MASTER_COMMAND_TRUNCATE_DATA: {
      command_name = "Init SD Card data";
      break;
    }
    case MASTER_COMMAND_UPDATE_STATION_SLUG: {
      command_name = "Update station slug";
      break;
    }
    #if(ENABLE_MENU_BOARD_SLUG)
    case MASTER_COMMAND_UPDATE_BOARD_SLUG: {
      command_name = "Update board slug";
      break;
    }
    #endif
    case MASTER_COMMAND_UPDATE_MQTT_USERNAME: {
      command_name = "Update mqtt username";
      break;
    }
    case MASTER_COMMAND_UPDATE_GSM_APN: {
      command_name = "Update GSM APN";
      break;
    }
    #if (ENABLE_MENU_GSM_NUMBER)
    case MASTER_COMMAND_UPDATE_GSM_NUMBER: {
      command_name = "Update GSM number";
      break;
    }
    #endif
    case MASTER_COMMAND_UPDATE_PSK_KEY: {
      command_name = "Update PSK key";
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

/// @brief Get the slave command name from enumeration
/// @param command slave command enumeration
/// @return Command name in string format (const char*)
const char* LCDTask::get_slave_command_name_from_enum(stima4_slave_commands_t command) {
  const char* command_name;
  switch (command) {
    case SLAVE_COMMAND_MAINTENANCE: {
      command_name = "Maintenance";
      break;
    }
    case SLAVE_COMMAND_RESET_FLAGS: {
      command_name = "Reset flags";
      break;
    }
    case SLAVE_COMMAND_DO_FACTORY: {
      command_name = "Do factory";
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

/// @brief ISR handler for encoder input that manage the button pression
void LCDTask::ISR_input_pression_pin_encoder(void) {
  // **************************************************************************
  // ************************* DEBOUNCE BUTTON HANDLER ************************
  // **************************************************************************
  bool wake_up_event;
  BaseType_t pxHigherPTW = true;

  // Processing
  if (millis() - debounce_millis >= DEBOUNCE_TIMEOUT) {
    pression_event = digitalRead(pin_top_left_encoder) == LOW ? true : false;
  }

  // Updating flags and states
  last_display_timeout = millis();
  debounce_millis = millis();

  // Enque from ISR WakeUP Event on pression
  if (pression_event) {
    localDisplayEventWakeUp->EnqueueFromISR(&wake_up_event, &pxHigherPTW);
  }
}

/// @brief ISR handler for encoder inputs that manage the rotation
void LCDTask::ISR_input_rotation_pin_encoder(void) {
  // Reading pins from encoder
  encoder.pin.a = digitalRead(pin_bottom_left_encoder);
  encoder.pin.b = digitalRead(pin_bottom_right_encoder);

  if (encoder.pin_val != encoder_old.pin_val) {
    // Processing => DIRECTION: NONE, CLOCK WISE or COUNTER CLOCK WISE
    encoder_process(encoder.pin_val, encoder_old.pin_val);
    // Updating
    encoder_old.pin_val = encoder.pin_val;
  }
  
  // Updating flags and states
  last_display_timeout = millis();
}

/// @brief Save new configuration to eeprom
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

/// @brief Interface switch handler
void LCDTask::switch_interface(void) {
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
            if (param.configuration->board_slave[channel].module_type != Module_Type::rain && stima4_slave_command == SLAVE_COMMAND_CALIBRATION_ACCELEROMETER) {
              stima4_slave_command = (stima4_slave_commands_t)(stima4_slave_command + 1);
            }
            if (!param.system_status->data_slave[channel].fw_upgradable && stima4_slave_command == SLAVE_COMMAND_FIRMWARE_UPGRADE) {
              stima4_slave_command = (stima4_slave_commands_t)(stima4_slave_command + 1);
            }
          }
          break;
        }

        case UPDATE_STATION_SLUG: {
          selected_char_index = selected_char_index == ALPHABET_LENGTH - 1 ? 0 : selected_char_index + 1;
          break;
        }

        case UPDATE_BOARD_SLUG: {
          selected_char_index = selected_char_index == ALPHABET_LENGTH - 1 ? 0 : selected_char_index + 1;
          break;
        }

        case UPDATE_MQTT_USERNAME: {
          selected_char_index = selected_char_index == ALPHABET_LENGTH - 1 ? 0 : selected_char_index + 1;
          break;
        }

        case UPDATE_GSM_APN: {
          selected_char_index = selected_char_index == ALPHABET_LENGTH - 1 ? 0 : selected_char_index + 1;
          break;
        }

        #if (ENABLE_MENU_GSM_NUMBER)
        case UPDATE_GSM_NUMBER: {
          selected_char_index = selected_char_index == ALPHABET_GSM_NUMBER_LENGTH - 1 ? 0 : selected_char_index + 1;
          break;
        }
        #endif

        case UPDATE_PSK_KEY: {
          selected_char_index = selected_char_index == ALPHABET_PSK_KEY_LENGTH - 1 ? 0 : selected_char_index + 1;
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
            command_selector_pos = stima4_master_command == MASTER_COMMAND_RESET_FLAGS ? 0 : command_selector_pos - 1;
            stima4_master_command = stima4_master_command == MASTER_COMMAND_RESET_FLAGS ? MASTER_COMMAND_RESET_FLAGS : (stima4_master_commands_t)(stima4_master_command - 1);
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
            if (param.configuration->board_slave[channel].module_type != Module_Type::rain && stima4_slave_command == SLAVE_COMMAND_CALIBRATION_ACCELEROMETER) {
              stima4_slave_command = (stima4_slave_commands_t)(stima4_slave_command - 1);
            }
            if (!param.system_status->data_slave[channel].fw_upgradable && stima4_slave_command == SLAVE_COMMAND_FIRMWARE_UPGRADE) {
              stima4_slave_command = (stima4_slave_commands_t)(stima4_slave_command - 1);
            }
          }
          break;
        }

        case UPDATE_STATION_SLUG: {
          selected_char_index = selected_char_index == 0 ? ALPHABET_LENGTH - 1 : selected_char_index - 1;
          break;
        }

        case UPDATE_BOARD_SLUG: {
          selected_char_index = selected_char_index == 0 ? ALPHABET_LENGTH - 1 : selected_char_index - 1;
          break;
        }

        case UPDATE_MQTT_USERNAME: {
          selected_char_index = selected_char_index == 0 ? ALPHABET_LENGTH - 1 : selected_char_index - 1;
          break;
        }

        case UPDATE_GSM_APN: {
          selected_char_index = selected_char_index == 0 ? ALPHABET_LENGTH - 1 : selected_char_index - 1;
          break;
        }
        #if (ENABLE_MENU_GSM_NUMBER)
        case UPDATE_GSM_NUMBER: {
          selected_char_index = selected_char_index == 0 ? ALPHABET_GSM_NUMBER_LENGTH - 1 : selected_char_index - 1;
          break;
        }
        #endif
        case UPDATE_PSK_KEY: {
          selected_char_index = selected_char_index == 0 ? ALPHABET_PSK_KEY_LENGTH - 1 : selected_char_index - 1;
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

        selected_char_index = 0;

        if (stima4_menu_ui_last == MAIN) {
          if (stima4_master_command == MASTER_COMMAND_UPDATE_STATION_SLUG) {
            // ************************************************************************
            // ************************* STATION SLUG INIT ****************************
            // ************************************************************************

            // Reset input buffer
            memset(new_station_slug, 0, sizeof(new_station_slug));
            // Set input buffer
            strcpy(new_station_slug, param.configuration->stationslug);
            // Cursor position to last character of parameter
            cursor_pos = strlen(param.configuration->stationslug);
            // Update current menu state
            stima4_menu_ui = UPDATE_STATION_SLUG;
          #if(ENABLE_MENU_BOARD_SLUG)
          } else if (stima4_master_command == MASTER_COMMAND_UPDATE_BOARD_SLUG) {
            // ************************************************************************
            // ************************* BOARD SLUG INIT ******************************
            // ************************************************************************

            // Reset input buffer
            memset(new_board_slug, 0, sizeof(new_board_slug));
            // Set input buffer
            strcpy(new_board_slug, param.configuration->board_master.boardslug);
            // Cursor position to last character of parameter
            cursor_pos = strlen(param.configuration->board_master.boardslug);
            // Update current menu state
            stima4_menu_ui = UPDATE_BOARD_SLUG;
          #endif
          } else if (stima4_master_command == MASTER_COMMAND_UPDATE_MQTT_USERNAME) {
            // ************************************************************************
            // ************************* MQTT USERNAME INIT ***************************
            // ************************************************************************

            // Reset input buffer
            memset(new_mqtt_username, 0, sizeof(new_mqtt_username));
            // Set input buffer
            strcpy(new_mqtt_username, param.configuration->mqtt_username);
            // Cursor position to last character of parameter
            cursor_pos = strlen(param.configuration->mqtt_username);
            // Update current menu state
            stima4_menu_ui = UPDATE_MQTT_USERNAME;
          } else if (stima4_master_command == MASTER_COMMAND_UPDATE_GSM_APN) {
            // ************************************************************************
            // *************************** GSM APN INIT *******************************
            // ************************************************************************

            // Reset input buffer
            memset(new_gsm_apn, 0, sizeof(new_gsm_apn));
            // Set input buffer
            strcpy(new_gsm_apn, param.configuration->gsm_apn);
            // Cursor position to last character of parameter
            cursor_pos = strlen(param.configuration->gsm_apn);
            // Update current menu state
            stima4_menu_ui = UPDATE_GSM_APN;
          #if (ENABLE_MENU_GSM_NUMBER)
          } else if (stima4_master_command == MASTER_COMMAND_UPDATE_GSM_NUMBER) {
            // ************************************************************************
            // ************************* GSM NUMBER INIT ******************************
            // ************************************************************************

            // Reset input buffer
            memset(new_gsm_number, 0, sizeof(new_gsm_number));
            // Set input buffer
            strcpy(new_gsm_number, param.configuration->gsm_number);
            // Cursor position to last character of parameter
            cursor_pos = strlen(param.configuration->gsm_number);
            // Update current menu state
            stima4_menu_ui = UPDATE_GSM_NUMBER;
          #endif
          } else if (stima4_master_command == MASTER_COMMAND_UPDATE_PSK_KEY) {
            // ************************************************************************
            // *************************** PSK KEY INIT *******************************
            // ************************************************************************

            // Reset input buffer
            memset(new_client_psk_key, 0, sizeof(new_client_psk_key));
            // Set input buffer
            for (int8_t id = 0; id < CLIENT_PSK_KEY_LENGTH; id++) {
              char tmp_data[2];
              sprintf(&new_client_psk_key[id * 2], "%02X", param.configuration->client_psk_key[id]);
            }
            // Cursor position to last character of parameter
            cursor_pos = (CLIENT_PSK_KEY_LENGTH * 2);
            // Update current menu state
            stima4_menu_ui = UPDATE_PSK_KEY;
          } else {
            // ************************************************************************
            // ************************* ELABORATE COMMAND ****************************
            // ************************************************************************

            elaborate_master_command(stima4_master_command);

            // Update current menu state
            stima4_menu_ui = stima4_menu_ui_last;
          }
        } else {
          elaborate_slave_command(stima4_slave_command);
          // Update current menu state
          stima4_menu_ui = stima4_menu_ui_last;
        }

        // Updating flags and states
        command_selector_pos = 0;
        stima4_master_command = MASTER_COMMAND_RESET_FLAGS;
        stima4_slave_command = SLAVE_COMMAND_MAINTENANCE;
        break;
      }

      case UPDATE_STATION_SLUG: {
        // ************************************************************************
        // ************************* ELABORATE COMMAND ****************************
        // ************************************************************************

        switch (alphabet[selected_char_index]) {
          case '<': {
            cursor_pos = cursor_pos == 0 ? 0 : cursor_pos - 1;
            new_station_slug[cursor_pos] = 0;
            break;
          }
          case '>': {
            elaborate_master_command(MASTER_COMMAND_UPDATE_STATION_SLUG);

            stima4_menu_ui = stima4_menu_ui_last;
            break;
          }
          case '!': {
            stima4_menu_ui = stima4_menu_ui_last;
            break;
          }
          default: {
            new_station_slug[cursor_pos++] = alphabet[selected_char_index];

            if (cursor_pos == STATIONSLUG_LENGTH - 1) {
              elaborate_master_command(MASTER_COMMAND_UPDATE_STATION_SLUG);

              stima4_menu_ui = stima4_menu_ui_last;
            }
            break;
          }
        }
        break;
      }

      #if(ENABLE_MENU_BOARD_SLUG)
      case UPDATE_BOARD_SLUG: {
        // ************************************************************************
        // ************************* ELABORATE COMMAND ****************************
        // ************************************************************************

        switch (alphabet[selected_char_index]) {
          case '<': {
            cursor_pos = cursor_pos == 0 ? 0 : cursor_pos - 1;
            new_board_slug[cursor_pos] = 0;
            break;
          }
          case '>': {
            elaborate_master_command(MASTER_COMMAND_UPDATE_BOARD_SLUG);

            stima4_menu_ui = stima4_menu_ui_last;
            break;
          }
          case '!': {
            stima4_menu_ui = stima4_menu_ui_last;
            break;
          }
          default: {
            new_board_slug[cursor_pos++] = alphabet[selected_char_index];

            if (cursor_pos == BOARDSLUG_LENGTH - 1) {
              elaborate_master_command(MASTER_COMMAND_UPDATE_BOARD_SLUG);

              stima4_menu_ui = stima4_menu_ui_last;
            }
            break;
          }
        }
        break;
      }
      #endif

      case UPDATE_MQTT_USERNAME: {
        // ************************************************************************
        // ************************* ELABORATE COMMAND ****************************
        // ************************************************************************

        switch (alphabet[selected_char_index]) {
          case '<': {
            cursor_pos = cursor_pos == 0 ? 0 : cursor_pos - 1;
            new_mqtt_username[cursor_pos] = 0;
            break;
          }
          case '>': {
            elaborate_master_command(MASTER_COMMAND_UPDATE_MQTT_USERNAME);

            stima4_menu_ui = stima4_menu_ui_last;
            break;
          }
          case '!': {
            stima4_menu_ui = stima4_menu_ui_last;
            break;
          }
          default: {
            new_mqtt_username[cursor_pos++] = alphabet[selected_char_index];

            if (cursor_pos == MQTT_USERNAME_LENGTH - 1) {
              elaborate_master_command(MASTER_COMMAND_UPDATE_MQTT_USERNAME);

              stima4_menu_ui = stima4_menu_ui_last;
            }
            break;
          }
        }
        break;
      }

      case UPDATE_GSM_APN: {
        // ************************************************************************
        // ************************* ELABORATE COMMAND ****************************
        // ************************************************************************

        switch (alphabet[selected_char_index]) {
          case '<': {
            cursor_pos = cursor_pos == 0 ? 0 : cursor_pos - 1;
            new_gsm_apn[cursor_pos] = 0;
            break;
          }
          case '>': {
            elaborate_master_command(MASTER_COMMAND_UPDATE_GSM_APN);

            stima4_menu_ui = stima4_menu_ui_last;
            break;
          }
          case '!': {
            stima4_menu_ui = stima4_menu_ui_last;
            break;
          }
          default: {
            new_gsm_apn[cursor_pos++] = alphabet[selected_char_index];

            if (cursor_pos == GSM_APN_LENGTH - 1) {
              elaborate_master_command(MASTER_COMMAND_UPDATE_GSM_APN);

              stima4_menu_ui = stima4_menu_ui_last;
            }
            break;
          }
        }
        break;
      }

      #if (ENABLE_MENU_GSM_NUMBER)
      case UPDATE_GSM_NUMBER: {
        // ************************************************************************
        // ************************* ELABORATE COMMAND ****************************
        // ************************************************************************

        switch (alphabet_gsm_number[selected_char_index]) {
          case '<': {
            cursor_pos = cursor_pos == 0 ? 0 : cursor_pos - 1;
            new_gsm_number[cursor_pos] = 0;
            break;
          }
          case '>': {
            elaborate_master_command(MASTER_COMMAND_UPDATE_GSM_NUMBER);

            stima4_menu_ui = stima4_menu_ui_last;
            break;
          }
          case '!': {
            stima4_menu_ui = stima4_menu_ui_last;
            break;
          }
          default: {
            new_gsm_number[cursor_pos++] = alphabet_gsm_number[selected_char_index];

            if (cursor_pos == GSM_NUMBER_LENGTH - 1) {
              elaborate_master_command(MASTER_COMMAND_UPDATE_GSM_NUMBER);

              stima4_menu_ui = stima4_menu_ui_last;
            }
            break;
          }
        }
        break;
      }
      #endif

      case UPDATE_PSK_KEY: {
        // ************************************************************************
        // ************************* ELABORATE COMMAND ****************************
        // ************************************************************************

        switch (alphabet_psk_key[selected_char_index]) {
          case '<': {
            cursor_pos = cursor_pos == 0 ? 0 : cursor_pos - 1;
            new_client_psk_key[cursor_pos] = 0;
            break;
          }
          case '>': {
            elaborate_master_command(MASTER_COMMAND_UPDATE_PSK_KEY);

            stima4_menu_ui = stima4_menu_ui_last;
            break;
          }
          case '!': {
            stima4_menu_ui = stima4_menu_ui_last;
            break;
          }
          default: {
            new_client_psk_key[cursor_pos++] = alphabet_psk_key[selected_char_index];

            if (cursor_pos == 2 * CLIENT_PSK_KEY_LENGTH) {
              elaborate_master_command(MASTER_COMMAND_UPDATE_PSK_KEY);

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
