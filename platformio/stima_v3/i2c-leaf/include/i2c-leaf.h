/**@file i2c-leaf.h */

/*********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
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

#ifndef _I2C_LEAF_H
#define _I2C_LEAF_H

#include "i2c-leaf-config.h"
#include <debug.h>
#include <i2c_config.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <i2c_utility.h>
#include <rmap_utility.h>
#if (USE_JSON)
#include <json_utility.h>
#endif
#include <eeprom_utility.h>
#include <Wire.h>
#include <TimeLib.h>
#include <typedef.h>
#include <registers-leaf.h>

#define leafRead()                    (analogRead(LEAF_ANALOG_PIN))

/*********************************************************************
* TYPEDEF
*********************************************************************/
/*!
\struct configuration_t
\brief EEPROM saved configuration.
*/
typedef struct {
  uint8_t module_version;             //!< module version
  uint8_t module_type;                //!< module type
  uint8_t i2c_address;                //!< i2c address
  bool is_oneshot;                    //!< enable or disable oneshot mode
  bool is_continuous;                 //!< enable or disable continuous mode
  uint16_t leaf_calibration_min;
  uint16_t leaf_calibration_max;
} configuration_t;

/*!
\struct leaf_wetness_t
\brief Leaf wetness data.
*/
typedef struct {
  float timer;   //!< timer in seconds that indicates wet leaf
} leaf_wetness_t;

/*!
\struct readable_data_t
\brief Readable data through i2c bus.
*/
typedef struct {
  uint8_t module_type;                 //!< module version
  uint8_t module_version;              //!< module type
  leaf_wetness_t leaf_wetness;         //!< leaf wetness data for report
} readable_data_t;

/*!
\struct writable_data_t
\brief Writable data through i2c bus.
*/
typedef struct {
  uint8_t i2c_address;                //!< i2c address
  bool is_oneshot;                    //!< enable or disable oneshot mode
  bool is_continuous;                 //!< enable or disable continuous mode
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
\enum leaf_state_t
\brief LEAF reading task finite state machine.
*/
typedef enum {
  LEAF_READING_INIT,            //!< init task variables
  LEAF_READING_READ,
  LEAF_READING_END,             //!< performs end operations and deactivate task
  LEAF_READING_WAIT_STATE       //!< non-blocking waiting time
} leaf_reading_state_t;

/*********************************************************************
* GLOBAL VARIABLE
*********************************************************************/
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
\var leaf_wetness
\brief leaf_wetness data.
*/
leaf_wetness_t leaf_wetness;

bool is_leaf_init;
bool is_leaf_wet;

/*!
\var samples_count
\brief Number of samples to be acquired for make one observation.
*/
uint8_t samples_count;

/*!
\var do_buffers_reset
\brief Force a buffers reset.
*/
bool do_buffers_reset;

/*!
\var timer_counter
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
\var leaf_state
\brief Current sensors reading task state.
*/
leaf_reading_state_t leaf_reading_state;

/*********************************************************************
* FUNCTIONS
*********************************************************************/
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
\fn void reset_samples_buffer(void)
\brief Reset samples buffers to default value.
\return void.
*/
void reset_samples_buffer(void);
void reset_report_buffer(void);
void reset_buffer(void);

/*!
\fn void reset_observations_buffer(void)
\brief Reset observations buffers to default value.
\return void.
*/
// void reset_observations_buffer(void);

/*!
\fn void exchange_buffers(void)
\brief Exchange reader and writer pointer to buffer.
\return void.
*/
void exchange_buffers(void);

template<typename sample_g, typename observation_g, typename value_v> void addSample(sample_g *sample, observation_g *observation, value_v value);

template<typename observation_g, typename value_v> value_v readCurrentObservation(observation_g *buffer);
template<typename observation_g, typename value_v> void writeCurrentObservation(observation_g *buffer, value_v value);
template<typename observation_g, typename length_v> void resetObservation(observation_g *buffer, length_v length);
template<typename observation_g, typename length_v> void resetBackPmObservation(observation_g *buffer, length_v length);
template<typename observation_g, typename length_v> void incrementObservation(observation_g *buffer, length_v length);
template<typename observation_g, typename length_v, typename value_v> void addObservation(observation_g *buffer, length_v length, value_v value);
template<typename observation_g, typename length_v, typename value_v> value_v readBackObservation(observation_g *buffer, length_v length);


/*!
\fn void samples_processing(bool is_force_processing)
\brief Main routine for processing the samples to calculate an observation.
\param is_force_processing if is true, force the calculation of the observation provided there is a minimum number of samples.
\return void.
*/
void samples_processing(bool is_force_processing);

/*********************************************************************
* TASKS
*********************************************************************/
/*!
\var is_event_leaf_reading
\brief Enable or disable the Sensors reading task.
*/
volatile bool is_event_leaf_reading;

/*!
\fn void leaf_reading_task(void)
\brief Sensors reading Task.
Read data from sensors.
\return void.
*/
void leaf_reading_task(void);

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
