/**
 ******************************************************************************
 * @file    lcd_task.cpp
 * @author  Cristiano Souza Paz <c.souzapaz@digiteco.it>
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
#define TRACE_LEVEL     LCD_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   LCD_TASK_ID

#include "tasks/lcd_task.h"

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

    state = LCD_STATE_INIT;

    Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void LCDTask::TaskMonitorStack()
{
  u_int16_t stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark( NULL );
  if((stackUsage) && (stackUsage < param.system_status->tasks[LOCAL_TASK_ID].stack)) {
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
void LCDTask::TaskWatchDog(uint32_t millis_standby)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Update WDT Signal (Direct or Long function Timered)
  if(millis_standby)  
  {
    // Check 1/2 Freq. controller ready to WDT only SET flag
    if((millis_standby) < WDT_CONTROLLER_MS / 2) {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
    } else {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::timer;
      // Add security milimal Freq to check
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog_ms = millis_standby + WDT_CONTROLLER_MS;
    }
  }
  else
    param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
  param.systemStatusLock->Give();
}

/// @brief local suspend flag and positor running state Task (optional)
/// @param state_position Sw_Position (Local STATE)
/// @param state_subposition Sw_SubPosition (Optional Local SUB_STATE Position Monitor)
/// @param state_operation operative mode flag status for this task
void LCDTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Signal Task sleep/disabled mode from request (Auto SET WDT on Resume)
  if((param.system_status->tasks[LOCAL_TASK_ID].state == task_flag::suspended)&&
     (state_operation==task_flag::normal))
     param.system_status->tasks->watch_dog = wdt_flag::set;
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

                channel = -1;
                data_printed = false;
                display_is_off = false;
                encoder_state = DIR_NONE;
                stima4_command = MAINTENANCE;
                stima4_menu_ui = MAIN;

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
                            TRACE_INFO_F(F("MAIN\r\n"));

                            // Display MAIN interface
                            display_print_main_interface();

                            break;
                        }
                        case CHANNEL: {
                            TRACE_INFO_F(F("CHANNEL: %d\r\n"), channel);

                            // Display CHANNEL interface
                            display_print_channel_interface(data.channel[channel].module_type);

                            break;
                        }
                        case CONFIGURATION: {
                            TRACE_INFO_F(F("CONFIGURATION: %s\r\n"), get_command_name_from_enum(stima4_command));

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
 * @brief Get the command name from enumeration
 *
 * @param command command enumeration
 * @return Command name in string format (const char*)
 */
const char* LCDTask::get_command_name_from_enum(stima4_commands_t command) {
    const char* command_name;
    switch (command) {
        case MAINTENANCE: {
            command_name = "MAINTENANCE";
            break;
        }
        case EXIT: {
            command_name = "EXIT";
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
    TRACE_INFO_F(F("Display OFF\r\n"));

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
    TRACE_INFO_F(F("Display ON\r\n"));

    // Turn high phisicals pins
    digitalWrite(PIN_ENCODER_EN5, HIGH);
    digitalWrite(PIN_DSP_POWER, HIGH);

    // Processing
    display.display();

    // Updating
    display_is_off = false;
    last_display_timeout = millis();
    state = LCD_STATE_INIT;
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
    
    // Take the informations to print
    getStimaLcdDescriptionByType(description, module_type);
    getStimaLcdUnitTypeByType(unit_type, module_type);
    getStimaLcdDecimalsByType(&decimals, module_type);

    // Process string format to print
    dtostrf(data.channel[channel].value, 0, decimals, measure);
    (void)snprintf(measure, sizeof(measure), "%s %s", measure, unit_type);

    // Print description of measure
    display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE);
    display.print(description);
    
    // Print measurement information
    display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 2 * LINE_BREAK);
    display.setFont(u8g2_font_helvB12_tf);
    display.print(measure);

    // Print maintenance information if enabled
    if (data.channel[channel].maintenance_mode) {
        display.setFont(u8g2_font_helvR08_tf);
        display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 4 * LINE_BREAK);
        display.print("Maintenance mode");
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
    display.drawTriangle(X_TEXT_FROM_RECT, Y_TOP_TRIANGLE + stima4_command * LINE_BREAK, X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 10 * stima4_command, X_PEAK_TRIANGLE, Y_PEAK_TRIANGLE + 10 * stima4_command);
    for (uint8_t i = 0; i < (stima4_commands_t)EXIT + 1; i++) {
        display.setCursor(X_TEXT_FROM_RECT_DESCRIPTION_COMMAND, Y_TEXT_FIRST_LINE + i * LINE_BREAK);
        display.print(get_command_name_from_enum((stima4_commands_t)i));
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
    display.print("Stima: Digiteco-Arpae");
}

/**
 * @brief Print Main interface with general information about station
 *
 */
void LCDTask::display_print_main_interface() {
    char station[STATION_LCD_LENGTH];
    char firmware_version[FIRMWARE_VERSION_LCD_LENGTH];

    // Process strings format to print
    (void)snprintf(station, sizeof(station), "Station: %s", param.configuration->stationslug);
    (void)snprintf(firmware_version, sizeof(firmware_version), "Firmware version: %d.%d", param.configuration->module_main_version, param.configuration->module_minor_version);

    // Print data and time
    display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE);
    display.print("08/02/23 15:00:00");

    // Print station name 
    display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + LINE_BREAK);
    display.print(station);

    // Print firmware version
    display.setCursor(X_TEXT_FROM_RECT, Y_TEXT_FIRST_LINE + 2 * LINE_BREAK);
    display.print(firmware_version);

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
 * @brief Command handler
 *
 * @param command type of elaboration based on command
 */
void LCDTask::elaborate_command(stima4_commands_t command) {
    switch (command) {
        case MAINTENANCE: {
            data.channel[channel].maintenance_mode = !data.channel[channel].maintenance_mode;
            break;
        }
        case EXIT: {
            break;
        }
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
        if (encoder_state == DIR_CLOCK_WISE) {
            if (stima4_menu_ui == CONFIGURATION) {
                stima4_command = stima4_command == EXIT ? MAINTENANCE : (stima4_commands_t)(stima4_command + 1);
            } else {
                channel = channel == -1 ? 0 : channel + 1;
                stima4_menu_ui = channel == MAX_CHANNELS ? MAIN : CHANNEL;
                channel = channel == MAX_CHANNELS ? -1 : channel;
            }
        } else {
            if (stima4_menu_ui == CONFIGURATION) {
                stima4_command = stima4_command == MAINTENANCE ? EXIT : (stima4_commands_t)(stima4_command - 1);
            } else {
                channel = channel == -1 ? MAX_CHANNELS - 1 : channel - 1;
                stima4_menu_ui = channel == -1 ? MAIN : CHANNEL;
            }
        }
        data_printed = false;
    }

    // ************************************************************************
    // ************************* BUTTON HANDLER *******************************
    // ************************************************************************

    if (pression_event) {
        if (stima4_menu_ui == CHANNEL) {
            stima4_menu_ui_last = stima4_menu_ui;
            stima4_menu_ui = CONFIGURATION;
        } else if (stima4_menu_ui == CONFIGURATION) {
            // ************************* SEND COMMAND *******************************
            elaborate_command(stima4_command);
            // **********************************************************************
            stima4_command = MAINTENANCE;
            stima4_menu_ui = stima4_menu_ui_last;
        }
        pression_event = false;
        data_printed = false;
    }

    encoder_state = DIR_NONE;
}
