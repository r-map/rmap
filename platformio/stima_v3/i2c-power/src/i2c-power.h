/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Paolo patruno <p.patruno@iperbole.bologna.it>
Marco Baldinetti <m.baldinetti@digiteco.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#ifndef _I2C_POWER_H
#define _I2C_POWER_H

#include "i2c-power-config.h"
#include <debug.h>
#include <i2c_config.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <i2c_utility.h>
#include <rmap_utility.h>
#include <sdcard_utility.h>
#if (USE_JSON)
#include <json_utility.h>
#endif
#include <eeprom_utility.h>
#include <Wire.h>
#include <TimeLib.h>
#include <typedef.h>
#include <ADS1115.h>
#include <registers-power.h>
#include <debug_config.h>
#include <SdFat.h>
#include <StreamUtils.h>
#include <ArduinoLog.h>

/*********************************************************************
* TYPEDEF
*********************************************************************/
/*!
\struct configuration_t
\brief EEPROM saved configuration.
*/
typedef struct {
   uint8_t module_main_version;        //!< module main version
   uint8_t module_configuration_version;   //!< module configuration version
   uint8_t module_type;                //!< module type
   uint8_t i2c_address;                //!< i2c address
   bool is_oneshot;                    //!< enable or disable oneshot mode
   uint16_t adc_voltage_max_panel;
   uint16_t adc_voltage_max_battery;
} configuration_t;

/*!
\struct report_t
\brief report data.
*/
typedef struct {
  int16_t sample_panel;
  int16_t sample_battery;
  int16_t avg_panel;
  int16_t avg_battery;
} data_t;

/*!
\struct readable_data_t
\brief Readable data through i2c bus.
*/
typedef struct {
  uint8_t module_type;                //!< module type
  uint8_t module_main_version;        //!< module main version
  uint8_t module_minor_version;       //!< module minor version
  data_t power;
} readable_data_t;

/*!
\struct writable_data_t
\brief Writable data through i2c bus.
*/
typedef struct {
   uint8_t i2c_address;                //!< i2c address
   bool is_oneshot;                    //!< enable or disable oneshot mode
   uint16_t adc_voltage_max_panel;
   uint16_t adc_voltage_max_battery;

} writable_data_t;

/*********************************************************************
* TYPEDEF for Finite State Machine
*********************************************************************/
/*!
\enum state_t
\brief Main loop finite state machine.
*/
typedef enum {
   INIT,                      //!< init tasks and sensors
   #if (USE_POWER_DOWN)
   ENTER_POWER_DOWN,          //!< if no task is running, activate power down
   #endif
   TASKS_EXECUTION,           //!< execute active tasks
   END                        //!< go to ENTER_POWER_DOWN or TASKS_EXECUTION
} state_t;

/*!
\enum power_state_t
\brief POWER setup and reading task finite state machine.
*/
typedef enum {
  POWER_INIT,
  POWER_READING_PANEL,
  POWER_READING_BATTERY,
  POWER_ELABORATE,
  POWER_END,             //!< performs end operations and deactivate task
  POWER_WAIT_STATE       //!< non-blocking waiting time
} power_state_t;

/*!
\enum power_hr_t
\brief POWER_HR setup and reading task finite state machine.
*/
typedef enum {
  POWER_HR_INIT,
  POWER_HR_READ,
  POWER_HR_EVALUATE,
  POWER_HR_PROCESS,
  POWER_HR_END,             //!< performs end operations and deactivate task
  POWER_HR_WAIT_STATE       //!< non-blocking waiting time
} power_hr_state_t;

/*********************************************************************
* GLOBAL VARIABLE
*********************************************************************/

#if (ENABLE_SDCARD_LOGGING)   
/*!
\var SD
\brief SD-Card structure.
*/
SdFat SD;

/*!
\var logFile
\brief File for logging on SD-Card.
*/
File logFile;

/*!
\var loggingStream
\brief stream for logging on Serial and  SD-Card together.
*/
WriteLoggingStream loggingStream(logFile,Serial);
#endif

/*!
\var configuration
\brief Configuration data.
*/
configuration_t configuration;

/*!
\var readable_data_1
\brief First readable i2c register.
*/
volatile readable_data_t readable_data_1;

/*!
\var readable_data_2
\brief Second readable i2c register.
*/
volatile readable_data_t readable_data_2;

/*!
\var readable_data_read_ptr
\brief Pointer for read data in i2c readable register.
*/
volatile readable_data_t *readable_data_read_ptr;

/*!
\var readable_data_write_ptr
\brief Pointer for write data in i2c readable register.
*/
volatile readable_data_t *readable_data_write_ptr;

/*!
\var readable_data_temp_ptr
\brief Temporary pointer for exchange read and write pointer for i2c readable register.
*/
volatile readable_data_t *readable_data_temp_ptr;

/*!
\var writable_data
\brief Writable i2c register.
*/
writable_data_t writable_data;

/*!
\var writable_data_ptr
\brief Pointer for read and write data in i2c writable register.
*/
writable_data_t *writable_data_ptr;

/*!
\var readable_data_address
\brief Address of readable i2c register.
*/
volatile uint8_t readable_data_address;

/*!
\var readable_data_length
\brief Number of bytes to read at readable_data_address.
*/
volatile uint8_t readable_data_length;

/*!
\var i2c_rx_data
\brief Buffer for i2c received data.
*/
volatile uint8_t i2c_rx_data[I2C_MAX_DATA_LENGTH];

/*!
\var i2c_error
\brief Number of i2c error.
*/
volatile uint8_t i2c_error;

/*!
\var ready_tasks_count
\brief Number of tasks ready to execute.
*/
volatile uint8_t ready_tasks_count;

/*!
\var awakened_event_occurred_time_ms
\brief System time (in millisecond) when the system has awakened from power down.
*/
uint32_t awakened_event_occurred_time_ms;

/*!
\var inside_transaction
\brief Status of command transaction.
*/
volatile bool inside_transaction;

/*!
\var transaction_time
\brief Timer counter variable for compute command transaction timeout.
*/
volatile uint16_t transaction_time;

/*!
\var lastcommand
\brief command to be executed.
*/
volatile uint8_t lastcommand;

/*!
\var is_start
\brief Execute start command.
*/
bool is_start;

/*!
\var is_stop
\brief Execute stop command.
*/
bool is_stop;

/*!
\var is_test_read
\brief Received command is in continuous mode.
*/
bool is_test_read;

/*!
\var is_oneshot
\brief Received command is in oneshot mode.
*/
bool is_oneshot;

/*!
\var is_continuous
\brief Received command is in continuous mode.
*/
bool is_continuous;

/*!
\var is_test
\brief Received command is in test mode.
*/
bool is_test;

int16_t sample_panel;
int16_t sample_battery;
int16_t average_panel;
int16_t average_battery;

ADS1115 adc1(ADC_I2C_ADDRESS);

/*!
\var samples_count_panel
\brief Number of samples to be acquired for make one panel observation.
*/
uint16_t samples_count_panel;

/*!
\var samples_error_count_panel
\brief Number of error while acquire samples for make one panel observation.
*/
uint16_t samples_error_count_panel;

/*!
\var samples_count_battery
\brief Number of samples to be acquired for make one battery observation.
*/
uint16_t samples_count_battery;

/*!
\var samples_error_count_battery
\brief Number of error while acquire samples for make one battery observation.
*/
uint16_t samples_error_count_battery;

/*!
\var timer_counter_ms
\brief Timer counter variable for execute timed task with time multiple of base Timer1 time.
*/
volatile uint16_t timer_counter_ms;
volatile uint16_t timer_counter_s;

/*!
\var state
\brief Current main loop state.
*/
state_t state;

/*!
\var power_state
\brief Current sensors reading task state.
*/
power_state_t power_state;

//power_hr_state_t power_hr_state;

/*********************************************************************
* FUNCTIONS
*********************************************************************/

/*!
\fn void init_logging(void)
\brief Init logging system.
\return void.
*/
void init_logging();

/*!
\fn void init_power_down(uint32_t *time_ms, uint32_t debouncing_ms)
\brief Enter power down mode.
\param time_ms pointer to a variable to save the last instant you entered power down.
\param debouncing_ms delay to power down.
\return void.
*/
void init_power_down(uint32_t *time_ms, uint32_t debouncing_ms);

/*!
\fn void init_wdt(uint8_t wdt_timer)
\brief Init watchdog.
\param wdt_timer a time value for init watchdog (WDTO_xxxx).
\return void.
*/
void init_wdt(uint8_t wdt_timer);

/*!
\fn void init_system(void)
\brief Init system.
\return void.
*/
void init_system(void);

/*!
\fn void init_buffers(void)
\brief Init buffers.
\return void.
*/
void init_buffers(void);

/*!
\fn void init_tasks(void)
\brief Init tasks variable and state.
\return void.
*/
void init_tasks(void);

/*!
\fn void init_pins(void)
\brief Init hardware pins.
\return void.
*/
void init_pins(void);

/*!
\fn void init_wire(void)
\brief Init wire (i2c) library and performs checks on the bus.
\return void.
*/
void init_wire(void);

/*!
\fn void init_adc(void)
\brief Init ADC converter.
\return void.
*/
void init_adc(void);

/*!
\fn void init_spi(void)
\brief Init SPI library.
\return void.
*/
void init_spi(void);

/*!
\fn void init_rtc(void)
\brief Init RTC module.
\return void.
*/
void init_rtc(void);

#if (USE_TIMER_1)
/*!
\fn void init_timer1(void)
\brief Init Timer1 module.
\return void.
*/
void init_timer1(void);

/*!
\fn void start_timer(void)
\brief Start Timer1 module.
\return void.
*/
void start_timer(void);

/*!
\fn void stop_timer(void)
\brief Stop Timer1 module.
\return void.
*/
void stop_timer(void);
#endif

/*!
\fn void init_sensors(void)
\brief Create and setup sensors.
\return void.
*/
void init_sensors(void);

/*!
\fn void print_configuration(void)
\brief Print current configuration.
\return void.
*/
void print_configuration(void);

/*!
\fn void load_configuration(void)
\brief Load configuration from EEPROM.
\return void.
*/
void load_configuration(void);

/*!
\fn void save_configuration(bool is_default)
\brief Save configuration to EEPROM.
\param is_default: if true save default configuration; if false save current configuration.
\return void.
*/
void save_configuration(bool);

/*!
\fn void commands(void)
\brief Performs specific operations based on the received command.
\return void.
*/
void commands(void);

/*!
\fn void tests(void)
\brief Performs specific operations based on the received command.
\return void.
*/
void tests(void);

/*!
\fn void reset_buffer(void)
\brief Reset sample and observations buffers to default value.
\return void.
*/

void reset_buffer(void);

/*!
\fn void exchange_buffers(void)
\brief Exchange reader and writer pointer to buffer.
\return void.
*/
void exchange_buffers(void);

/*!
\fn void make_report (bool init=false)
\brief Main routine for processing the samples to calculate an observation.
\return void.
*/
void make_report (bool init=false);


#if (USE_SENSOR_PWR)

uint16_t powerRead(uint8_t analog);
uint16_t powerMean(uint8_t analog, uint8_t count=3, uint8_t delay_ms=10);
uint16_t getPowerVoltage (uint8_t analog, uint16_t voltage_max);

#endif

/*********************************************************************
* TASKS
*********************************************************************/

/*!
\var is_event_power_task
\brief Enable or disable the POWER task.
*/
volatile bool is_event_power_task;

/*!
\fn void power_task(void)
\brief Setup and reading Task.
Read voltage from panel and battery.
\return void.
*/
void power_task(void);

/*!
\fn void power_task_hr(void)
\brief Setup and reading Task.
Read voltage from panel and battery
Higt resolution.
\return void.
*/
void power_task_hr(void);

/*!
\var is_event_command_task
\brief Enable or disable the Command task.
*/
volatile bool is_event_command_task;

/*!
\fn void command_task(void)
\brief Command Task.
Execute the command received on i2c bus by reading i2c received data buffer.
\return void.
*/
void command_task(void);

/*********************************************************************
* INTERRUPTS HANDLER
*********************************************************************/
/*!
\fn void i2c_request_interrupt_handler(void)
\brief I2C request interrupt handler.
\return void.
*/
void i2c_request_interrupt_handler(void);

/*!
\fn void i2c_receive_interrupt_handler(void)
\brief I2C receive interrupt handler.
\return void.
*/
void i2c_receive_interrupt_handler(void);

#endif
