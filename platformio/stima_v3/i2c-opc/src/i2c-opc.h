/**@file i2c-opc.h */

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

#ifndef _I2C_OPC_H
#define _I2C_OPC_H

#include "i2c-opc-config.h"
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
#include <registers-opc.h>
#include <opcxx.h>
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
   uint8_t module_version;             //!< module version
   uint8_t module_type;                //!< module type
   uint8_t i2c_address;                //!< i2c address
   bool is_oneshot;                    //!< enable or disable oneshot mode
   bool is_continuous;                 //!< enable or disable continuous mode
   // uint8_t configuration_variables[OPCXX_CONFIGURATION_VARIABLES_LENGTH];
   // uint8_t configuration_variables_2[OPCXX_CONFIGURATION_VARIABLES_2_LENGTH];
} configuration_t;

#if (USE_SENSOR_OE3)
/*!
\struct value_t
\brief Value struct for storing sample, observation and minium, average and maximum measurement.
*/
typedef struct {
  float sample; //!< last sample
  // float med60;  //!< last observation
  float med;    //!< average values of observations
  // float max;    //!< maximum values of observations
  // float min;    //!< minium values of observations
  float sigma;  //!< standard deviation of observations
} temperature_value_t;

typedef struct {
  float sample; //!< last sample
  // float med60;  //!< last observation
  float med;    //!< average values of observations
  // float max;    //!< maximum values of observations
  // float min;    //!< minium values of observations
  float sigma;  //!< standard deviation of observations
} humidity_value_t;
#endif

typedef struct {
  float sample; //!< last sample
  // float med60;  //!< last observation
  float med;    //!< average values of observations
  // float max;    //!< maximum values of observations
  // float min;    //!< minium values of observations
  float sigma;  //!< standard deviation of observations
} pm_value_t;

typedef struct {
  uint16_t sample; //!< last sample
  // uint16_t med60;  //!< last observation
  uint16_t med;    //!< average values of observations
  // uint16_t max;    //!< maximum values of observations
  // uint16_t min;    //!< minium values of observations
  uint16_t sigma;  //!< standard deviation of observations
} bin_value_t;

/*!
\struct readable_data_t
\brief Readable data through i2c bus.
*/
typedef struct {
   uint8_t module_type;                 //!< module version
   uint8_t module_version;              //!< module type

   float pm_sample[OPCXX_PM_LENGTH];
   float pm_med[OPCXX_PM_LENGTH];
   float pm_sigma[OPCXX_PM_LENGTH];

   uint16_t bins_med[OPCN3_BINS_LENGTH];
   uint16_t bins_sigma[OPCN3_BINS_LENGTH];

   #if (USE_SENSOR_OE3)
   float temperature_sample;
   float humidity_sample;

   float temperature_med;
   float humidity_med;
   #endif

   pm_value_t pm1;                      //!< pm1 data for report
   pm_value_t pm25;                     //!< pm25 data for report
   pm_value_t pm10;                     //!< pm10 data for report
   bin_value_t bins[OPC_BINS_LENGTH];   //!< bins array data for report

   #if (USE_SENSOR_OE3)
   temperature_value_t temperature;
   humidity_value_t humidity;
   #endif
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

/*!
\struct sample_t
\brief Samples values for measured opc data
*/
typedef struct {
  float values;          //!< buffer containing the measured samples
  uint8_t count;         //!< number of good samples
  uint8_t error_count;   //!< number of bad samples
} sample_t;

/*!
\struct observation_t
\brief Observations values for opc
*/
typedef struct {
   float med[OBSERVATION_COUNT];        //!< buffer containing the mean values calculated on a one sample buffer respectively
   uint16_t count;                      //!< number of observations
   float *read_ptr;                     //!< reader pointer to buffer (read observations for calculate report value)
   float *write_ptr;                    //!< writer pointer to buffer (add new observation)
} float_observation_t;

typedef struct {
   uint16_t med[OBSERVATION_COUNT];        //!< buffer containing the mean values calculated on a one sample buffer respectively
   uint16_t count;                      //!< number of observations
   uint16_t *read_ptr;                     //!< reader pointer to buffer (read observations for calculate report value)
   uint16_t *write_ptr;                    //!< writer pointer to buffer (add new observation)
} uint16_observation_t;

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
\enum opc_state_t
\brief OPC setup and reading task finite state machine.
*/
typedef enum {
  OPC_INIT,
  OPC_SWITCH_ON,
  OPC_SEND_COMMAND_FAN_DAC,
  OPC_WAIT_RESULT_FAN_DAC,
  OPC_SEND_COMMAND_FAN_ON,
  OPC_WAIT_RESULT_FAN_ON,
  OPC_SEND_COMMAND_LASER_ON,
  OPC_WAIT_RESULT_LASER_ON,
  OPC_SEND_COMMAND_READ_HISTOGRAM,
  OPC_WAIT_RESULT_READ_HISTOGRAM,
  OPC_READ_HISTOGRAM,
  OPC_END,             //!< performs end operations and deactivate task
  OPC_WAIT_STATE       //!< non-blocking waiting time
} opc_state_t;

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
\var is_continuous
\brief Received command is in continuous mode.
*/
// bool is_read_configuration_variable;

/*!
\var opc_samples
\brief OPC samples: PM1, PM2.5, PM10, BINS[0-15].
*/
// sample_t opc_samples;
sample_t pm1_samples;
sample_t pm25_samples;
sample_t pm10_samples;
sample_t bins_samples[OPC_BINS_LENGTH];
#if (USE_SENSOR_OE3)
sample_t temperature_samples;
sample_t humidity_samples;
#endif

/*!
\var opc_observations
\brief OPC observations: PM1, PM2.5, PM10, BINS[0-15].
*/
float_observation_t pm1_observations;
float_observation_t pm25_observations;
float_observation_t pm10_observations;
uint16_observation_t bins_observations[OPC_BINS_LENGTH];
#if (USE_SENSOR_OE3)
float_observation_t temperature_observations;
float_observation_t humidity_observations;
#endif

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
volatile uint16_t timer_counter;

/*!
\var state
\brief Current main loop state.
*/
state_t state;

/*!
\var opc_state
\brief Current sensors reading task state.
*/
opc_state_t opc_state;

bool is_opc_setted;
bool is_opc_first_read;
uint8_t histogram_error_count;

/*!
\var opcn
\brief Alphasense OPC-N2 or OPC-N3 sensor
*/
#if (USE_SENSOR_OA2 || USE_SENSOR_OB2 || USE_SENSOR_OC2 || USE_SENSOR_OD2)
Opcn2 opcn(OPC_CHIP_SELECT, OPC_POWER_PIN, OPC_SPI_POWER_PIN, SENSORS_SAMPLE_TIME_MS / 1000);
#endif

#if (USE_SENSOR_OA3 || USE_SENSOR_OB3 || USE_SENSOR_OC3 || USE_SENSOR_OD3 || USE_SENSOR_OE3)
Opcn3 opcn(OPC_CHIP_SELECT, OPC_POWER_PIN, OPC_SPI_POWER_PIN, SENSORS_SAMPLE_TIME_MS / 1000);
#endif

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

template<typename sample_g, typename observation_g, typename values_v, typename value_v> void addSample(sample_g *sample, observation_g *observation, values_v *values, value_v value);

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

/*!
\fn void observations_processing(void)
\brief Main routine for processing the observations to calculate a value for report.
\return void.
*/
bool observations_processing(void);

/*!
\fn bool make_pm_observation_from_samples(bool is_force_processing, sample_t *sample, observation_t *observation)
\brief Processing the samples to calculate an observation when the number of the samples reaches the exact samples_count value.
\param is_force_processing if is true, force the calculation of the observation provided there is a minimum number of samples.
\param *sample input samples.
\param *observation output observation.
\return void.
*/
template<typename sample_g, typename observation_g> bool make_observation_from_samples(bool is_force_processing, sample_g *sample, observation_g *observation);


/*!
\fn bool make_value_from_samples_and_observations(sample_t *sample, observation_t *observation, volatile value_t *value)
\brief Processing the observations to calculate a value for report when the number of the observations reaches the minimum value of STATISTICAL_DATA_COUNT.
\param *sample input samples.
\param *observation input observation.
\param *value output value for report.
\return void.
*/
template<typename sample_g, typename observation_g, typename value_v, typename val_v> bool make_value_from_samples_and_observations(sample_g *sample, observation_g *observation, value_v *value);

/*********************************************************************
* TASKS
*********************************************************************/
/*!
\var is_event_opc_task
\brief Enable or disable the OPC task.
*/
volatile bool is_event_opc_task;

/*!
\fn void opc_task(void)
\brief Opc setup and reading Task.
Read data from OPC.
\return void.
*/
void opc_task(void);

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
