/**@file i2c-opc.ino */

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

#include <debug_config.h>

/*!
\def SERIAL_TRACE_LEVEL
\brief Serial debug level for this sketch.
*/
#define SERIAL_TRACE_LEVEL I2C_OPC_SERIAL_TRACE_LEVEL

#include "i2c-opc.h"

/*!
\fn void setup()
\brief Arduino setup function.
*  Init watchdog, hardware, debug, buffer and load configuration stored in EEPROM.
\return void.
*/
void setup() {
   init_wdt(WDT_TIMER);
   SERIAL_BEGIN(115200);
   init_pins();
   load_configuration();
   init_buffers();
   init_wire();
   init_spi();
   init_rtc();
   #if (USE_TIMER_1)
   init_timer1();
   #endif
   init_system();
   wdt_reset();
}

/*!
\fn void loop()
\brief Arduino loop function.
*  First, initialize tasks and sensors, then execute the tasks and activates the power down if no task is running.
\return void.
*/
void loop() {
   switch (state) {
      case INIT:
         init_tasks();
         init_sensors();
         wdt_reset();
         state = TASKS_EXECUTION;
      break;

      #if (USE_POWER_DOWN)
      case ENTER_POWER_DOWN:
         //! enter in power down mode only if DEBOUNCING_POWER_DOWN_TIME_MS milliseconds have passed since last time (awakened_event_occurred_time_ms)
         init_power_down(&awakened_event_occurred_time_ms, DEBOUNCING_POWER_DOWN_TIME_MS);
         state = TASKS_EXECUTION;
      break;
      #endif

      case TASKS_EXECUTION:
        // I2C Bus Check
        if (i2c_error >= I2C_MAX_ERROR_COUNT) {
          SERIAL_ERROR(F("Restart I2C BUS\r\n"));
          init_wire();
          wdt_reset();
        }

        if (is_event_opc_task) {
          opc_task();
          wdt_reset();
        }

        if (is_event_command_task) {
          command_task();
          wdt_reset();
        }

        // if (digitalRead(CONFIGURATION_RESET_PIN) == LOW) {
        //   save_configuration(CONFIGURATION_CURRENT);
        // }

        if (ready_tasks_count == 0) {
          wdt_reset();
          state = END;
        }
      break;

      case END:
         #if (USE_POWER_DOWN)
         state = ENTER_POWER_DOWN;
         #else
         state = TASKS_EXECUTION;
         #endif
      break;
   }
}

void init_power_down(uint32_t *time_ms, uint32_t debouncing_ms) {
	if (millis() - *time_ms > debouncing_ms) {
		*time_ms = millis();

		power_adc_disable();
		power_spi_disable();
		power_timer0_disable();
    #if (USE_TIMER_1 == false)
    power_timer1_disable();
    #endif
		power_timer2_disable();

		noInterrupts ();
		sleep_enable();

		interrupts ();

		sleep_cpu();
		sleep_disable();

		power_adc_enable();
		power_spi_enable();
		power_timer0_enable();
    #if (USE_TIMER_1 == false)
    power_timer1_enable();
    #endif
		power_timer2_enable();
	}
}

void init_wdt(uint8_t wdt_timer) {
   wdt_disable();
   wdt_reset();
   wdt_enable(wdt_timer);
}

void init_buffers() {
  readable_data_read_ptr = &readable_data_1;
  readable_data_write_ptr = &readable_data_2;
  writable_data_ptr = &writable_data;

  readable_data_write_ptr->module_type = MODULE_TYPE;
  readable_data_write_ptr->module_version = MODULE_VERSION;

  volatile pm_value_t *ptr;
  for (uint8_t i = 0; i < OPCXX_PM_LENGTH; i++) {
    switch (i) {
      case 0: ptr = &readable_data_write_ptr->pm1;
      break;
      case 1: ptr = &readable_data_write_ptr->pm25;
      break;
      case 2: ptr = &readable_data_write_ptr->pm10;
      break;
    }
    ptr->sample = UINT16_MAX;
    // ptr->med60 = UINT16_MAX;
    ptr->med = UINT16_MAX;
    // ptr->max = UINT16_MAX;
    // ptr->min = UINT16_MAX;
    ptr->sigma = UINT16_MAX;
  }

  memset((void *) &readable_data_write_ptr->bins, UINT8_MAX, sizeof(bin_value_t) * OPC_BINS_LENGTH);

  #if (USE_SENSOR_OE3)
  readable_data_write_ptr->temperature.med = UINT16_MAX;
  readable_data_write_ptr->humidity.med = UINT16_MAX;
  #endif

  for (uint8_t i = 0; i < OPCXX_PM_LENGTH; i++) {
    readable_data_write_ptr->pm_sample[i] = UINT16_MAX;
    readable_data_write_ptr->pm_med[i] = UINT16_MAX;
    readable_data_write_ptr->pm_sigma[i] = UINT16_MAX;
  }

  for (uint8_t i = 0; i < OPCN3_BINS_LENGTH; i++) {
    readable_data_write_ptr->bins_med[i] = UINT16_MAX;
    readable_data_write_ptr->bins_sigma[i] = UINT16_MAX;
  }

  #if (USE_SENSOR_OE3)
  readable_data_write_ptr->temperature_sample = UINT16_MAX;
  readable_data_write_ptr->temperature_med = UINT16_MAX;
  readable_data_write_ptr->humidity_sample = UINT16_MAX;
  readable_data_write_ptr->humidity_med = UINT16_MAX;
  #endif

  //! copy readable_data_2 in readable_data_1
  memcpy((void *) readable_data_read_ptr, (const void*) readable_data_write_ptr, sizeof(readable_data_t));

  reset_samples_buffer();
  reset_observations_buffer();
}

void init_tasks() {
   noInterrupts();

   //! no tasks ready
   ready_tasks_count = 0;

   is_event_command_task = false;
   is_event_opc_task = false;

   opc_state = OPC_INIT;

   is_opc_setted = false;
   is_opc_first_read = true;
   histogram_error_count = 0;

   //! reset samples_count value
   samples_count = SENSORS_SAMPLE_COUNT_MIN;

   // is_read_configuration_variable = true;

   interrupts();
}

void init_pins() {
   pinMode(CONFIGURATION_RESET_PIN, INPUT_PULLUP);
   opcn.initPins();
}

void init_wire() {
  i2c_error = 0;
  Wire.end();
  Wire.begin(configuration.i2c_address);
  Wire.setClock(I2C_BUS_CLOCK);
  Wire.onRequest(i2c_request_interrupt_handler);
  Wire.onReceive(i2c_receive_interrupt_handler);
}

void init_spi() {
  SPI.begin();
}

void init_rtc() {
}

#if (USE_TIMER_1)
void init_timer1() {
   start_timer();
}

void start_timer() {
   TCCR1A = 0x00;                //!< Normal timer operation
   TCCR1B = 0x05;                //!< 1:1024 prescaler
   TCNT1 = TIMER1_TCNT1_VALUE;   //!< Pre-load timer counter register
   TIFR1 |= (1 << TOV1);         //!< Clear interrupt overflow flag register
   TIMSK1 |= (1 << TOIE1);       //!< Enable overflow interrupt
}

void stop_timer() {
   TCCR1B = 0x00;                //!< Stop
   TIMSK1 &= ~(1 << TOIE1);      //!< Disable overflow interrupt
   TIFR1 |= (1 << TOV1);         //!< Clear interrupt overflow flag register
   TCNT1 = TIMER1_TCNT1_VALUE;   //!< Pre-load timer counter register
}
#endif

void init_system() {
   #if (USE_POWER_DOWN)
   set_sleep_mode(SLEEP_MODE_IDLE);
   awakened_event_occurred_time_ms = millis();
   #endif

   //! main loop state
   state = INIT;
}

void print_configuration() {
   char stima_name[20];
   getStimaNameByType(stima_name, configuration.module_type);
   SERIAL_INFO(F("--> type: %s\r\n"), stima_name);
   SERIAL_INFO(F("--> version: %d\r\n"), configuration.module_version);
   SERIAL_INFO(F("--> i2c address: 0x%X (%d)\r\n"), configuration.i2c_address, configuration.i2c_address);
   SERIAL_INFO(F("--> oneshot: %s\r\n"), configuration.is_oneshot ? ON_STRING : OFF_STRING);
   SERIAL_INFO(F("--> continuous: %s\r\n"), configuration.is_continuous ? ON_STRING : OFF_STRING);
   // SERIAL_INFO(F("--> configuration variables: "));
   // for (uint16_t i = 0; i < OPCXX_CONFIGURATION_VARIABLES_LENGTH; i++) {
   //   SERIAL_INFO_CLEAN(F("%X"), configuration.configuration_variables[i]);
   // }
   // SERIAL_INFO_CLEAN(F("\r\n"));
   // SERIAL_INFO(F("--> configuration variables 2: "));
   // for (uint16_t i = 0; i < OPCXX_CONFIGURATION_VARIABLES_2_LENGTH; i++) {
   //   SERIAL_INFO_CLEAN(F("%X"), configuration.configuration_variables_2[i]);
   // }
   // SERIAL_INFO_CLEAN(F("\r\n"));
}

void save_configuration(bool is_default) {
  if (is_default) {
    SERIAL_INFO(F("Save default configuration... [ %s ]\r\n"), OK_STRING);
    configuration.module_type = MODULE_TYPE;
    configuration.module_version = MODULE_VERSION;
    configuration.i2c_address = CONFIGURATION_DEFAULT_I2C_ADDRESS;
    configuration.is_oneshot = CONFIGURATION_DEFAULT_IS_ONESHOT;
    configuration.is_continuous = CONFIGURATION_DEFAULT_IS_CONTINUOUS;
    // memset(configuration.configuration_variables, 0, OPCXX_CONFIGURATION_VARIABLES_LENGTH);
    // memset(configuration.configuration_variables_2, 0, OPCXX_CONFIGURATION_VARIABLES_2_LENGTH);
  }
  else {
    SERIAL_INFO(F("Save configuration... [ %s ]\r\n"), OK_STRING);
    configuration.i2c_address = writable_data.i2c_address;
    configuration.is_oneshot = writable_data.is_oneshot;
    configuration.is_continuous = writable_data.is_continuous;
  }

  //! write configuration to eeprom
  ee_write(&configuration, CONFIGURATION_EEPROM_ADDRESS, sizeof(configuration));

  print_configuration();
}

void load_configuration() {
  //! read configuration from eeprom
  ee_read(&configuration, CONFIGURATION_EEPROM_ADDRESS, sizeof(configuration));

  if (configuration.module_type != MODULE_TYPE || configuration.module_version != MODULE_VERSION || digitalRead(CONFIGURATION_RESET_PIN) == LOW) {
    save_configuration(CONFIGURATION_DEFAULT);
  }
  else {
    SERIAL_INFO(F("Load configuration... [ %s ]\r\n"), OK_STRING);
    print_configuration();
  }

  writable_data.i2c_address = configuration.i2c_address;
  writable_data.is_oneshot = configuration.is_oneshot;
  writable_data.is_continuous = configuration.is_continuous;
}

void init_sensors () {
  opcn.init();

  if (configuration.is_continuous) {
     SERIAL_INFO(F("\r\n--> acquiring %u~%u samples in %u minutes (max %u samples error)\r\n\r\n"), SENSORS_SAMPLE_COUNT_MIN, SENSORS_SAMPLE_COUNT_MAX, OBSERVATIONS_MINUTES, SENSORS_SAMPLE_COUNT_TOLERANCE);
  }
}

/*!
\fn ISR(TIMER1_OVF_vect)
\brief Timer1 overflow interrupts routine.
\return void.
*/
ISR(TIMER1_OVF_vect) {
   //! Pre-load timer counter register
   TCNT1 = TIMER1_TCNT1_VALUE;

   //! increment timer_counter by TIMER1_INTERRUPT_TIME_MS
   timer_counter += TIMER1_INTERRUPT_TIME_MS;

   //! check if SENSORS_SAMPLE_TIME_MS ms have passed since last time. if true and if is in continuous mode and continuous start command It has been received, activate Sensor OPC task
   if (executeTimerTaskEach(timer_counter, SENSORS_SAMPLE_TIME_MS, TIMER1_INTERRUPT_TIME_MS) && configuration.is_continuous && is_continuous && is_start) {
     if (!is_event_opc_task) {
       noInterrupts();
       is_event_opc_task = true;
       ready_tasks_count++;
       interrupts();
     }
   }

   //! reset timer_counter if it has become larger than TIMER1_VALUE_MAX_MS
   if (timer_counter >= TIMER1_VALUE_MAX_MS) {
      timer_counter = 0;
   }
}

void i2c_request_interrupt_handler() {
  if (readable_data_length) {
    //! write readable_data_length bytes of data stored in readable_data_read_ptr (base) + readable_data_address (offset) on i2c bus
    Wire.write((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length);
    //! write crc8
    Wire.write(crc8((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length));
  } else {
    Wire.write(UINT16_MAX);
  }
}

void i2c_receive_interrupt_handler(int rx_data_length) {
  bool is_i2c_data_ok = false;

  readable_data_length = 0;

  // read rx_data_length bytes of data from i2c bus
  for (uint8_t i = 0; i < rx_data_length; i++) {
    i2c_rx_data[i] = Wire.read();
  }

  //! check crc: ok
  if (i2c_rx_data[rx_data_length - 1] == crc8((uint8_t *)i2c_rx_data, rx_data_length - 1)) {
    rx_data_length--;

    // it is a registers read?
    if (rx_data_length == 2 && is_readable_register(i2c_rx_data[0])) {
      // offset in readable_data_read_ptr buffer
      readable_data_address = i2c_rx_data[0];

      // length (in bytes) of data to be read in readable_data_read_ptr
      readable_data_length = i2c_rx_data[1];
    }
    // it is a command?
    else if (rx_data_length == 2 && is_command(i2c_rx_data[0])) {
      noInterrupts();
      // enable Command task
      if (!is_event_command_task) {
        is_event_command_task = true;
        ready_tasks_count++;
      }
      interrupts();
    }
    // it is a registers write?
    else if (is_writable_register(i2c_rx_data[0])) {
      rx_data_length -= 2;

      if (i2c_rx_data[0] == I2C_OPC_ADDRESS_ADDRESS && rx_data_length == I2C_OPC_ADDRESS_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_OPC_ONESHOT_ADDRESS && rx_data_length == I2C_OPC_ONESHOT_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_OPC_CONTINUOUS_ADDRESS && rx_data_length == I2C_OPC_CONTINUOUS_LENGTH) {
        is_i2c_data_ok = true;
      }

      if (is_i2c_data_ok) {
        for (uint8_t i = 0; i < rx_data_length; i++) {
          // write rx_data_length bytes in writable_data_ptr (base) at (i2c_rx_data[i] - I2C_WRITE_REGISTER_START_ADDRESS) (position in buffer)
          ((uint8_t *)writable_data_ptr)[i2c_rx_data[0] - I2C_WRITE_REGISTER_START_ADDRESS + i] = i2c_rx_data[i + 2];
        }
      }
    }
  } else {
    readable_data_length = 0;
    i2c_error++;
  }
}

template<typename sample_g, typename observation_g, typename values_v, typename value_v> void addSample(sample_g *sample, observation_g *observation, values_v *values, value_v value) {
  values->sample = (value_v) value;

  if (isValid(value)) {
    sample->values = value;
    sample->count++;
    float med = (float) readCurrentObservation<observation_g, value_v>(observation);
    med += ((float) sample->values - med) / (float) (sample->count);
    writeCurrentObservation(observation, (value_v) med);
  }
  else {
    sample->values = UINT16_MAX;
    sample->error_count++;
  }
}

template<typename observation_g, typename value_v> value_v readCurrentObservation(observation_g *buffer) {
  return *buffer->write_ptr;
}

template<typename observation_g, typename value_v> void writeCurrentObservation(observation_g *buffer, value_v value) {
  *buffer->write_ptr = value;
}

template<typename observation_g, typename length_v> void resetObservation(observation_g *buffer, length_v length) {
  for (length_v i = 0; i < OBSERVATION_COUNT; i++) {
    buffer->med[i] = UINT16_MAX;
  }

  buffer->count = 0;
  buffer->read_ptr = buffer->med;
  buffer->write_ptr = buffer->med;
}

template<typename observation_g, typename length_v> void resetBackObservation(observation_g *buffer, length_v length) {
  if (buffer->write_ptr == buffer->med) {
    buffer->read_ptr = buffer->med+length-1;
  }
  else buffer->read_ptr = buffer->write_ptr-1;
}

template<typename observation_g, typename length_v> void incrementObservation(observation_g *buffer, length_v length) {
  if (buffer->count < length) {
    buffer->count++;
  }

  if (buffer->write_ptr+1 < buffer->med + length) {
    buffer->write_ptr++;
  } else buffer->write_ptr = buffer->med;
}

template<typename observation_g, typename length_v, typename value_v> void addObservation(observation_g *buffer, length_v length, value_v value) {
  *buffer->write_ptr = value;
  incrementObservation(buffer, length);
}

template<typename observation_g, typename length_v, typename value_v> value_v readBackObservation(observation_g *buffer, length_v length) {
  value_v value = *buffer->read_ptr;

  if (buffer->read_ptr == buffer->med) {
    buffer->read_ptr = buffer->med+length-1;
  }
  else buffer->read_ptr--;

  return value;
}

void samples_processing(bool is_force_processing) {
  SERIAL_DEBUG(F("%u/%u\t%.1f %.1f %.1f\t"), pm1_samples.count + pm1_samples.error_count, samples_count, pm1_samples.values, pm25_samples.values, pm10_samples.values);

  #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_DEBUG)
  for (uint8_t index = 0; index < OPC_BINS_LENGTH; index++) {
    SERIAL_DEBUG_CLEAN(F("%.0f "), bins_samples[index].values);
  }
  #endif

  #if (USE_SENSOR_OE3)
  SERIAL_DEBUG_CLEAN(F("\t%.1f %.0f"), temperature_samples.values, humidity_samples.values);
  #endif

  SERIAL_DEBUG_CLEAN(F("\t\t%s\r\n"), is_force_processing ? "F" : "N");

  bool is_processing = false;
  bool is_observations_processing = false;

  //! if true, a new bins and pm observation was calculated
  is_processing |= make_observation_from_samples(is_force_processing, &pm1_samples, &pm1_observations);
  is_processing |= make_observation_from_samples(is_force_processing, &pm25_samples, &pm25_observations);
  is_processing |= make_observation_from_samples(is_force_processing, &pm10_samples, &pm10_observations);

  for (uint8_t i = 0; i < OPC_BINS_LENGTH; i++) {
    is_processing |= make_observation_from_samples(is_force_processing, &bins_samples[i], &bins_observations[i]);
  }

  #if (USE_SENSOR_OE3)
  is_processing |= make_observation_from_samples(is_force_processing, &temperature_samples, &temperature_observations);
  is_processing |= make_observation_from_samples(is_force_processing, &humidity_samples, &humidity_observations);
  #endif

  if (is_processing) {
    #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_DEBUG)
    resetBackObservation(&pm1_observations, OBSERVATION_COUNT);
    resetBackObservation(&pm25_observations, OBSERVATION_COUNT);
    resetBackObservation(&pm10_observations, OBSERVATION_COUNT);

    for (uint8_t i = 0; i < OPC_BINS_LENGTH; i++) {
      resetBackObservation(&bins_observations[i], OBSERVATION_COUNT);
    }

    #if (USE_SENSOR_OE3)
    resetBackObservation(&temperature_observations, OBSERVATION_COUNT);
    resetBackObservation(&humidity_observations, OBSERVATION_COUNT);
    #endif

    float pm1 = readBackObservation<float_observation_t, uint16_t, float>(&pm1_observations, OBSERVATION_COUNT);
    float pm25 = readBackObservation<float_observation_t, uint16_t, float>(&pm25_observations, OBSERVATION_COUNT);
    float pm10 = readBackObservation<float_observation_t, uint16_t, float>(&pm10_observations, OBSERVATION_COUNT);

    SERIAL_DEBUG(F("O----------------------------------------------------------------------------------------------\r\n"));
    SERIAL_DEBUG(F("%u/%u\t%.1f %.1f %.1f\t"), pm1_samples.count, samples_count, pm1, pm25, pm10);
    for (uint8_t i = 0; i < OPC_BINS_LENGTH; i++) {
      uint16_t bin = readBackObservation<uint16_observation_t, uint16_t, uint16_t>(&bins_observations[i], OBSERVATION_COUNT);
      SERIAL_DEBUG_CLEAN(F("%u "), bin);
    }

    #if (USE_SENSOR_OE3)
    float temperature = readBackObservation<float_observation_t, uint16_t, float>(&temperature_observations, OBSERVATION_COUNT);
    float humidity = readBackObservation<float_observation_t, uint16_t, float>(&humidity_observations, OBSERVATION_COUNT);

    SERIAL_DEBUG_CLEAN(F("\t%.1f %.0f"), temperature, humidity);
    #endif

    SERIAL_DEBUG_CLEAN(F("\r\n"));
    SERIAL_DEBUG(F("O----------------------------------------------------------------------------------------------\r\n"));
    #endif

    //! assign new value for samples_count
    if (is_force_processing) {
      samples_count = SENSORS_SAMPLE_COUNT_MAX;
    }
    else samples_count = (samples_count == SENSORS_SAMPLE_COUNT_MAX ? SENSORS_SAMPLE_COUNT_MIN : SENSORS_SAMPLE_COUNT_MAX);

    is_observations_processing = observations_processing();
  }

  readable_data_write_ptr->pm_sample[0] = readable_data_write_ptr->pm1.sample;
  readable_data_write_ptr->pm_med[0] = readable_data_write_ptr->pm1.med;
  readable_data_write_ptr->pm_sigma[0] = readable_data_write_ptr->pm1.sigma;
  readable_data_write_ptr->pm_sample[1] = readable_data_write_ptr->pm25.sample;
  readable_data_write_ptr->pm_med[1] = readable_data_write_ptr->pm25.med;
  readable_data_write_ptr->pm_sigma[1] = readable_data_write_ptr->pm25.sigma;
  readable_data_write_ptr->pm_sample[2] = readable_data_write_ptr->pm10.sample;
  readable_data_write_ptr->pm_med[2] = readable_data_write_ptr->pm10.med;
  readable_data_write_ptr->pm_sigma[2] = readable_data_write_ptr->pm10.sigma;

  for (uint8_t i = 0; i < OPC_BINS_LENGTH; i++) {
    readable_data_write_ptr->bins_med[i] = readable_data_write_ptr->bins[i].med;
    readable_data_write_ptr->bins_sigma[i] = readable_data_write_ptr->bins[i].sigma;
  }

  #if (USE_SENSOR_OE3)
  readable_data_write_ptr->temperature_sample = readable_data_write_ptr->temperature.sample;
  readable_data_write_ptr->temperature_med = readable_data_write_ptr->temperature.med;

  readable_data_write_ptr->humidity_sample = readable_data_write_ptr->humidity.sample;
  readable_data_write_ptr->humidity_med = readable_data_write_ptr->humidity.med;
  #endif

  if (is_processing || is_force_processing) {
    //! reset samples's buffer values
    reset_samples_buffer();
    exchange_buffers();
  }

  #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_INFO)
  if (is_observations_processing) {
    SERIAL_INFO(F("R----------------------------------------------------------------------------------------------\r\n"));
    SERIAL_INFO(F("\t"));

    if (isValid(readable_data_read_ptr->pm_med[0])) {
      SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm_med[0]);
    }
    else SERIAL_INFO_CLEAN(F("\t"));

    if (isValid(readable_data_read_ptr->pm_sigma[0])) {
      SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm_sigma[0]);
    }
    else SERIAL_INFO_CLEAN(F("\t"));

    if (isValid(readable_data_read_ptr->pm_med[1])) {
      SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm_med[1]);
    }
    else SERIAL_INFO_CLEAN(F("\t"));

    if (isValid(readable_data_read_ptr->pm_sigma[1])) {
      SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm_sigma[1]);
    }
    else SERIAL_INFO_CLEAN(F("\t"));

    if (isValid(readable_data_read_ptr->pm_med[2])) {
      SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm_med[2]);
    }
    else SERIAL_INFO_CLEAN(F("\t"));

    if (isValid(readable_data_read_ptr->pm_sigma[2])) {
      SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm_sigma[2]);
    }
    else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm1.sample)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm1.sample);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm1.med60)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm1.med60);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm1.min)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm1.min);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm1.med)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm1.med);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm1.sigma)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm1.sigma);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm1.max)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm1.max);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm25.sample)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm25.sample);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm25.med60)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm25.med60);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm25.min)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm25.min);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm25.med)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm25.med);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm25.sigma)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm25.sigma);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm25.max)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm25.max);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm10.sample)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm10.sample);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm10.med60)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm10.med60);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm10.min)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm10.min);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm10.med)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->pm10.med);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm10.sigma)) {
    //   SERIAL_INFO_CLEAN(F("%.1f\t"), readable_data_read_ptr->pm10.sigma);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->pm10.max)) {
    //   SERIAL_INFO_CLEAN(F("%.1f\t"), readable_data_read_ptr->pm10.max);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));

    for (uint8_t i = 0; i < OPC_BINS_LENGTH; i++) {
      if (isValid(readable_data_read_ptr->bins_med[i])) {
        SERIAL_INFO_CLEAN(F("%u "), readable_data_read_ptr->bins_med[i]);
      }
      else SERIAL_INFO_CLEAN(F("\t"));

      // if (isValid(readable_data_read_ptr->bins[i].sample)) {
      //   SERIAL_INFO_CLEAN(F("%u "), readable_data_read_ptr->bins[i].sample);
      // }
      // else SERIAL_INFO_CLEAN(F("\t"));

      // if (isValid(readable_data_read_ptr->bins[i].med60)) {
      //   SERIAL_INFO_CLEAN(F("%u "), readable_data_read_ptr->bins[i].med60);
      // }
      // else SERIAL_INFO_CLEAN(F("\t"));

      // if (isValid(readable_data_read_ptr->bins[i].min)) {
      //   SERIAL_INFO_CLEAN(F("%u "), readable_data_read_ptr->bins[i].min);
      // }
      // else SERIAL_INFO_CLEAN(F("\t"));

      // if (isValid(readable_data_read_ptr->bins[i].med)) {
      //   SERIAL_INFO_CLEAN(F("%u "), readable_data_read_ptr->bins[i].med);
      // }
      // else SERIAL_INFO_CLEAN(F("\t"));

      // if (isValid(readable_data_read_ptr->bins[i].sigma)) {
      //   SERIAL_INFO_CLEAN(F("%u "), readable_data_read_ptr->bins[i].sigma);
      // }
      // else SERIAL_INFO_CLEAN(F("\t"));

      // if (isValid(readable_data_read_ptr->bins[i].max)) {
      //   SERIAL_INFO_CLEAN(F("%u "), readable_data_read_ptr->bins[i].max);
      // }
      // else SERIAL_INFO_CLEAN(F("\t"));
    }

    #if (USE_SENSOR_OE3)
    if (isValid(readable_data_read_ptr->temperature_med)) {
      SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->temperature_med);
    }
    else SERIAL_INFO_CLEAN(F("\t"));

    if (isValid(readable_data_read_ptr->humidity_med)) {
      SERIAL_INFO_CLEAN(F("%.0f "), readable_data_read_ptr->humidity_med);
    }
    else SERIAL_INFO_CLEAN(F("\t"));

    // if (isValid(readable_data_read_ptr->temperature.med)) {
    //   SERIAL_INFO_CLEAN(F("%.1f "), readable_data_read_ptr->temperature.med);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));
    //
    // if (isValid(readable_data_read_ptr->humidity.med)) {
    //   SERIAL_INFO_CLEAN(F("%.0f "), readable_data_read_ptr->humidity.med);
    // }
    // else SERIAL_INFO_CLEAN(F("\t"));
    #endif

    SERIAL_INFO_CLEAN(F("\r\n"));
    SERIAL_INFO(F("R----------------------------------------------------------------------------------------------\r\n"));
  }
  #endif
}

template<typename sample_g, typename observation_g> bool make_observation_from_samples(bool is_force_processing, sample_g *sample, observation_g *observation) {
  //! when true, indicates that sufficient samples have been acquired for the calculation of an observation
  bool is_processing_end;

  //! force processing when total samples count (sample->count + sample->error_count >= (SENSORS_SAMPLE_COUNT_MAX - SENSORS_SAMPLE_COUNT_TOLERANCE))
  if (is_force_processing && samples_count == SENSORS_SAMPLE_COUNT_MAX) {
    is_processing_end = (sample->count + sample->error_count >= (SENSORS_SAMPLE_COUNT_MAX - SENSORS_SAMPLE_COUNT_TOLERANCE));
  }
  //! force processing when total samples count (sample->count + sample->error_count >= (SENSORS_SAMPLE_COUNT_MIN - SENSORS_SAMPLE_COUNT_TOLERANCE))
  else if (is_force_processing && samples_count == SENSORS_SAMPLE_COUNT_MIN) {
    is_processing_end = (sample->count + sample->error_count >= (SENSORS_SAMPLE_COUNT_MIN - SENSORS_SAMPLE_COUNT_TOLERANCE));
  }
  //! normal behavior: processing when correct sample count ((sample->count + sample->error_count) == samples_count)
  else is_processing_end = ((sample->count + sample->error_count) == samples_count);

  if (is_processing_end) {
    if (sample->error_count > SENSORS_SAMPLE_COUNT_TOLERANCE) {
      //! add missing observation to observations buffer
      addObservation(observation, OBSERVATION_COUNT, UINT16_MAX);
    }
    else {
      //! increment ptr for set new calculted observation to observations buffer
      incrementObservation(observation, OBSERVATION_COUNT);
    }
  }

  return is_processing_end;
}

//------------------------------------------------------------------------------
// I2C-OPC
// MOP: continuous PM1, PM2.5, PM10 average and standard deviation
// OPB: continuous BINS average
// OPB: continuous BINS standard deviation
//------------------------------------------------------------------------------
bool observations_processing() {
  bool is_processing = false;

  //! if true, a new pm1 data report was calculated
  is_processing |= make_value_from_samples_and_observations<sample_t, float_observation_t, volatile pm_value_t, float>(&pm1_samples, &pm1_observations, &readable_data_write_ptr->pm1);

  //! if true, a new pm2.5 data report was calculated
  is_processing |= make_value_from_samples_and_observations<sample_t, float_observation_t, volatile pm_value_t, float>(&pm25_samples, &pm25_observations, &readable_data_write_ptr->pm25);

  //! if true, a new pm10 data report was calculated
  is_processing |= make_value_from_samples_and_observations<sample_t, float_observation_t, volatile pm_value_t, float>(&pm10_samples, &pm10_observations, &readable_data_write_ptr->pm10);

  //! if true, a new bins data report was calculated
  for (uint8_t i = 0; i < OPC_BINS_LENGTH; i++) {
    is_processing |= make_value_from_samples_and_observations<sample_t, uint16_observation_t, volatile bin_value_t, uint16_t>(&bins_samples[i], &bins_observations[i], &readable_data_write_ptr->bins[i]);
  }

  #if (USE_SENSOR_OE3)
  //! if true, a new temperature report was calculated
  is_processing |= make_value_from_samples_and_observations<sample_t, float_observation_t, volatile temperature_value_t, float>(&temperature_samples, &temperature_observations, &readable_data_write_ptr->temperature);

  //! if true, a new humidty report was calculated
  is_processing |= make_value_from_samples_and_observations<sample_t, float_observation_t, volatile humidity_value_t, float>(&humidity_samples, &humidity_observations, &readable_data_write_ptr->humidity);
  #endif

  return is_processing;
}

template<typename sample_g, typename observation_g, typename value_v, typename val_v> bool make_value_from_samples_and_observations(sample_g *sample, observation_g *observation, value_v *value) {
  //! assign last sample to report value
  // value->sample = (val_v) sample->values;
  if (isValid(sample->values)) {
    value->sample = (val_v) sample->values;
  }

  //! reset value to default
  // value->med60 = UINT16_MAX;
  value->med = UINT16_MAX;
  // value->max = UINT16_MAX;
  // value->min = UINT16_MAX;
  value->sigma = UINT16_MAX;

  //! if true, you can calculate the value for report (there are at least STATISTICAL_DATA_COUNT observations)
  bool is_processing = (observation->count >= STATISTICAL_DATA_COUNT);

  //! good observation counter
  uint16_t count = 0;

  //! error observation counter
  uint16_t error_count = 0;

  //! current observation's value
  val_v current = UINT16_MAX;

  //! minimum value
  // float min = UINT16_MAX;

  //! average value
  float med = 0;

  float sum = 0;
  float sum2 = 0;

  //! maximum value
  // float max = 0;

  //! standard deviation value
  float sigma = 0;

  if (is_processing) {
    resetBackObservation(observation, OBSERVATION_COUNT);

    //! loop backwards in last STATISTICAL_DATA_COUNT observations array
    for (uint16_t i = 0; i < STATISTICAL_DATA_COUNT; i++) {
      current = readBackObservation<observation_g, uint16_t, val_v>(observation, OBSERVATION_COUNT);

      //! if it is a good observation, calculate sum, minimum and maximum value. Otherwise increment error counter.
      if (isValid(current)) {
        count++;

        //! check and assing minimum value
        // min = min(min, current);

        //! check and assing maximum value
        // max = max(max, current);

        //! average calculation
        med += ((float) current - med) / ((float) count);

        //!  standard deviation
        sum2 += ((float) current) * ((float) current);
        sum += (float) current;

      } else error_count++;
    }

    //! calculate report value only if there are enough good observations
    if (error_count <= OBSERVATION_COUNT_TOLLERANCE) {
      //! assign last observation to report value
      // value->med60 = (val_v) current;

      //! average
      value->med = (val_v) med;

      //! assign maximum observation to report value
      // value->max = (val_v) max;

      //! assign minimum observation to report value
      // value->min = (val_v) min;

      //! calculate standard deviation: make sense for count >=2
      if (count > 1) {
        sigma = sqrt((sum2 - (sum * sum) / (float) (count)) / (float) (count));
        //! assign standard deviation to standard deviation report value
        value->sigma = (val_v) sigma;
      }
    }
  }

  return is_processing;
}

void opc_task () {
  static uint8_t i;
  static uint8_t retry;
  static bool is_error;
  static opc_state_t state_after_wait;
  static uint32_t delay_ms;
  static uint32_t start_time_ms;
  opcxx_state_t opcxx_status;
  static float sampling_period;

  switch (opc_state) {
    case OPC_INIT:
      i = 0;
      retry = 0;
      is_error = false;
      state_after_wait = OPC_INIT;

      if (is_opc_setted) {
        opc_state = OPC_SEND_COMMAND_READ_HISTOGRAM;
        SERIAL_TRACE(F("OPC_INIT --> OPC_SEND_COMMAND_READ_HISTOGRAM\r\n"));
      }
      else {
        opc_state = OPC_SWITCH_ON;
        SERIAL_TRACE(F("OPC_INIT --> OPC_SWITCH_ON\r\n"));
      }
      break;

    case OPC_SWITCH_ON:
      if (opcn.isOff()) {
        opcn.switchOn();
        start_time_ms = millis();
        delay_ms = OPCXX_SWITCH_ON_DELAY_MS;
        opc_state = OPC_WAIT_STATE;
        state_after_wait = OPC_SEND_COMMAND_FAN_DAC;
        SERIAL_TRACE(F("OPC_SWITCH_ON -->> OPC_SEND_COMMAND_FAN_DAC\r\n"));
      }
      else {
        opc_state = OPC_SEND_COMMAND_FAN_DAC;
        SERIAL_TRACE(F("OPC_SWITCH_ON --> OPC_SEND_COMMAND_FAN_DAC\r\n"));
      }
      break;

    case OPC_SEND_COMMAND_FAN_DAC:
      opcn.setFanDacCmd(OPCXX_FAN_DAC_MAX);
      opc_state = OPC_WAIT_RESULT_FAN_DAC;
      SERIAL_TRACE(F("OPC_SEND_COMMAND_FAN_DAC --> OPC_WAIT_RESULT_FAN_DAC\r\n"));
      break;

    case OPC_WAIT_RESULT_FAN_DAC:
      opcxx_status = opcn.setFanDacRst();

      //! success
      if (opcxx_status == OPCXX_OK) {
        retry = 0;
        is_error = false;
        start_time_ms = millis();
        delay_ms = OPCXX_GENERIC_OPERATION_DELAY_MS;
        opc_state = OPC_WAIT_STATE;
        state_after_wait = OPC_SEND_COMMAND_FAN_ON;
        SERIAL_TRACE(F("OPC_WAIT_RESULT_FAN_DAC -->> OPC_SEND_COMMAND_FAN_ON\r\n"));
      }
      //! retry
      else if (opcxx_status == OPCXX_ERROR && (retry++) < OPC_RETRY_COUNT_MAX) {
        start_time_ms = millis();
        delay_ms = OPCXX_RETRY_DELAY_MS;
        opc_state = OPC_WAIT_STATE;
        state_after_wait = OPC_SEND_COMMAND_FAN_DAC;
        SERIAL_TRACE(F("OPC_WAIT_RESULT_FAN_DAC -->> OPC_SEND_COMMAND_FAN_DAC\r\n"));
      }
      //! fail
      else if (opcxx_status == OPCXX_ERROR || retry >= OPC_RETRY_COUNT_MAX) {
        retry = 0;
        is_error = true;
        opc_state = OPC_END;
        SERIAL_TRACE(F("OPC_WAIT_RESULT_FAN_DAC --> OPC_END\r\n"));
      }
      break;

    case OPC_SEND_COMMAND_FAN_ON:
      opcn.fanOnCmd();
      opc_state = OPC_WAIT_RESULT_FAN_ON;
      SERIAL_TRACE(F("OPC_SEND_COMMAND_FAN_ON --> OPC_WAIT_RESULT_FAN_ON\r\n"));
      break;

    case OPC_WAIT_RESULT_FAN_ON:
      opcxx_status = opcn.fanOnOffRst(true);

      //! success
      if (opcxx_status == OPCXX_OK) {
        retry = 0;
        is_error = false;
        start_time_ms = millis();
        delay_ms = OPCXX_FAN_ON_DELAY_MS;
        opc_state = OPC_WAIT_STATE;
        state_after_wait = OPC_SEND_COMMAND_LASER_ON;
        SERIAL_TRACE(F("OPC_WAIT_RESULT_FAN_ON -->> OPC_SEND_COMMAND_LASER_ON\r\n"));
      }
      //! retry
      else if (opcxx_status == OPCXX_ERROR && (retry++) < OPC_RETRY_COUNT_MAX) {
        start_time_ms = millis();
        delay_ms = OPCXX_RETRY_DELAY_MS;
        opc_state = OPC_WAIT_STATE;
        state_after_wait = OPC_SEND_COMMAND_FAN_DAC;
        SERIAL_TRACE(F("OPC_WAIT_RESULT_FAN_ON -->> OPC_SEND_COMMAND_FAN_ON\r\n"));
      }
      //! fail
      else if (opcxx_status == OPCXX_ERROR || retry >= OPC_RETRY_COUNT_MAX) {
        retry = 0;
        is_error = true;
        opc_state = OPC_END;
        SERIAL_TRACE(F("OPC_WAIT_RESULT_FAN_ON --> OPC_END\r\n"));
      }
      break;

    case OPC_SEND_COMMAND_LASER_ON:
      opcn.laserOnCmd();
      opc_state = OPC_WAIT_RESULT_LASER_ON;
      SERIAL_TRACE(F("OPC_SEND_COMMAND_LASER_ON --> OPC_WAIT_RESULT_LASER_ON\r\n"));
      break;

    case OPC_WAIT_RESULT_LASER_ON:
      opcxx_status = opcn.laserOnOffRst(true);

      //! success
      if (opcxx_status == OPCXX_OK) {
        is_opc_setted = true;
        retry = 0;
        is_error = false;
        start_time_ms = millis();
        delay_ms = OPCXX_LASER_ON_DELAY_MS;
        opc_state = OPC_WAIT_STATE;
        state_after_wait = OPC_END;
        SERIAL_TRACE(F("OPC_WAIT_RESULT_LASER_ON -->> OPC_END\r\n"));
      }
      //! retry
      else if (opcxx_status == OPCXX_ERROR && (retry++) < OPC_RETRY_COUNT_MAX) {
        start_time_ms = millis();
        delay_ms = OPCXX_RETRY_DELAY_MS;
        opc_state = OPC_WAIT_STATE;
        state_after_wait = OPC_SEND_COMMAND_LASER_ON;
        SERIAL_TRACE(F("OPC_WAIT_RESULT_LASER_ON -->> OPC_SEND_COMMAND_LASER_ON\r\n"));
      }
      //! fail
      else if (opcxx_status == OPCXX_ERROR || retry >= OPC_RETRY_COUNT_MAX) {
        retry = 0;
        is_error = true;
        opc_state = OPC_END;
        SERIAL_TRACE(F("OPC_WAIT_RESULT_LASER_ON --> OPC_END\r\n"));
      }
      break;

    case OPC_SEND_COMMAND_READ_HISTOGRAM:
      opcn.readHistogramCmd();
      opc_state = OPC_WAIT_RESULT_READ_HISTOGRAM;
      SERIAL_TRACE(F("OPC_SEND_COMMAND_READ_HISTOGRAM --> OPC_WAIT_RESULT_READ_HISTOGRAM\r\n"));
      break;

    case OPC_WAIT_RESULT_READ_HISTOGRAM:
      opcxx_status = opcn.readHistogramRst();

      //! success
      if (opcxx_status == OPCXX_OK) {
        retry = 0;
        is_error = false;
        opc_state = OPC_READ_HISTOGRAM;
        SERIAL_TRACE(F("OPC_WAIT_RESULT_READ_HISTOGRAM -->> OPC_READ_HISTOGRAM\r\n"));
      }
      else if (opcxx_status == OPCXX_ERROR || opcxx_status == OPCXX_ERROR_RESULT) {
        opcn.resetHistogram();
        retry = 0;
        histogram_error_count++;

        if (is_opc_first_read) {
          is_opc_first_read = false;
          opc_state = OPC_END;
          SERIAL_TRACE(F("OPC_WAIT_RESULT_READ_HISTOGRAM -->> OPC_END\r\n"));
        }
        else {
          opc_state = OPC_READ_HISTOGRAM;
          SERIAL_TRACE(F("OPC_WAIT_RESULT_READ_HISTOGRAM -->> OPC_READ_HISTOGRAM\r\n"));
        }
      }
      break;

    case OPC_READ_HISTOGRAM:
      sampling_period = opcn.getSamplingPeriod();
      addSample<sample_t, float_observation_t, volatile pm_value_t, float>(&pm1_samples, &pm1_observations, &readable_data_write_ptr->pm1, opcn.getPm1());
      addSample<sample_t, float_observation_t, volatile pm_value_t, float>(&pm25_samples, &pm25_observations, &readable_data_write_ptr->pm25, opcn.getPm25());
      addSample<sample_t, float_observation_t, volatile pm_value_t, float>(&pm10_samples, &pm10_observations, &readable_data_write_ptr->pm10, opcn.getPm10());

      for (uint8_t index = 0; index < OPC_BINS_LENGTH; index++) {
        addSample<sample_t, uint16_observation_t, volatile bin_value_t, uint16_t>(&bins_samples[index], &bins_observations[index], &readable_data_write_ptr->bins[index], opcn.getBinNormalizedAtIndex(index));
      }

      #if (USE_SENSOR_OE3)
      addSample<sample_t, float_observation_t, volatile temperature_value_t, float>(&temperature_samples, &temperature_observations, &readable_data_write_ptr->temperature, opcn.getTemperature());
      addSample<sample_t, float_observation_t, volatile humidity_value_t, float>(&humidity_samples, &humidity_observations, &readable_data_write_ptr->humidity, opcn.getHumidity());
      #endif

      if (configuration.is_continuous) {
        noInterrupts();
        samples_processing(false);
        interrupts();
      }
      opc_state = OPC_END;
      SERIAL_TRACE(F("OPC_READ_HISTOGRAM -->> OPC_END\r\n"));
      break;

    case OPC_END:
      if (is_error) {
        opcn.switchOff();
        is_opc_setted = false;
      }

      if (histogram_error_count >= 4) {
        histogram_error_count = 0;
        opcn.switchOff();
        is_opc_setted = false;
      }

      noInterrupts();
      is_event_opc_task = false;
      ready_tasks_count--;
      interrupts();
      opc_state = OPC_INIT;
      SERIAL_TRACE(F("OPC_END --> OPC_INIT\r\n"));
      break;

    case OPC_WAIT_STATE:
      if (millis() - start_time_ms > delay_ms) {
        opc_state = state_after_wait;
      }
      break;
  }
}

void exchange_buffers() {
  readable_data_temp_ptr = readable_data_write_ptr;
  readable_data_write_ptr = readable_data_read_ptr;
  readable_data_read_ptr = readable_data_temp_ptr;
}

void reset_samples_buffer() {
  pm1_samples.values = UINT16_MAX;
  pm1_samples.count = 0;
  pm1_samples.error_count = 0;

  pm25_samples.values = UINT16_MAX;
  pm25_samples.count = 0;
  pm25_samples.error_count = 0;

  pm10_samples.values = UINT16_MAX;
  pm10_samples.count = 0;
  pm10_samples.error_count = 0;

  for (uint8_t i = 0; i < OPC_BINS_LENGTH; i++) {
    bins_samples[i].values = UINT16_MAX;
    bins_samples[i].count = 0;
    bins_samples[i].error_count = 0;
  }

  #if (USE_SENSOR_OE3)
  temperature_samples.values = UINT16_MAX;
  temperature_samples.count = 0;
  temperature_samples.error_count = 0;

  humidity_samples.values = UINT16_MAX;
  humidity_samples.count = 0;
  humidity_samples.error_count = 0;
  #endif
}

void reset_observations_buffer() {
  resetObservation(&pm1_observations, OBSERVATION_COUNT);
  resetObservation(&pm25_observations, OBSERVATION_COUNT);
  resetObservation(&pm10_observations, OBSERVATION_COUNT);

  for (uint8_t i = 0; i < OPC_BINS_LENGTH; i++) {
    resetObservation(&bins_observations[i], OBSERVATION_COUNT);
  }

  #if (USE_SENSOR_OE3)
  resetObservation(&temperature_observations, OBSERVATION_COUNT);
  resetObservation(&humidity_observations, OBSERVATION_COUNT);
  #endif
}

void command_task() {
   #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
   char buffer[30];
   #endif

   switch(i2c_rx_data[1]) {
      case I2C_OPC_COMMAND_ONESHOT_START:
         #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
         strcpy(buffer, "ONESHOT START");
         #endif
         is_oneshot = true;
         is_continuous = false;
         is_start = true;
         is_stop = false;
         commands();
      break;

      case I2C_OPC_COMMAND_ONESHOT_STOP:
         #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
         strcpy(buffer, "ONESHOT STOP");
         #endif
         is_oneshot = true;
         is_continuous = false;
         is_start = false;
         is_stop = true;
         commands();
      break;

      case I2C_OPC_COMMAND_ONESHOT_START_STOP:
         #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
         strcpy(buffer, "ONESHOT START-STOP");
         #endif
         is_oneshot = true;
         is_continuous = false;
         is_start = true;
         is_stop = true;
         commands();
      break;

      case I2C_OPC_COMMAND_CONTINUOUS_START:
         #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
         strcpy(buffer, "CONTINUOUS START");
         #endif
         is_oneshot = false;
         is_continuous = true;
         is_start = true;
         is_stop = false;
         commands();
      break;

      case I2C_OPC_COMMAND_CONTINUOUS_STOP:
         #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
         strcpy(buffer, "CONTINUOUS STOP");
         #endif
         is_oneshot = false;
         is_continuous = true;
         is_start = false;
         is_stop = true;
         commands();
      break;

      case I2C_OPC_COMMAND_CONTINUOUS_START_STOP:
         #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
         strcpy(buffer, "CONTINUOUS START-STOP");
         #endif
         is_oneshot = false;
         is_continuous = true;
         is_start = true;
         is_stop = true;
         commands();
      break;

      case I2C_OPC_COMMAND_SAVE:
         is_oneshot = false;
         is_continuous = false;
         is_start = false;
         is_stop = false;
         SERIAL_TRACE(F("Execute command [ SAVE ]\r\n"));
         save_configuration(CONFIGURATION_CURRENT);
         init_wire();
      break;
   }

   #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
   if (configuration.is_oneshot == is_oneshot || configuration.is_continuous == is_continuous) {
      SERIAL_TRACE(F("Execute [ %s ]\r\n"), buffer);
   }
   else if (configuration.is_oneshot == is_continuous || configuration.is_continuous == is_oneshot) {
      SERIAL_TRACE(F("Ignore [ %s ]\r\n"), buffer);
   }
   #endif

   noInterrupts();
   is_event_command_task = false;
   ready_tasks_count--;
   interrupts();
}

void tests() {
}

void commands() {
  noInterrupts();

  //! CONTINUOUS START
  if (configuration.is_continuous && is_continuous && is_start && !is_stop) {
    reset_samples_buffer();
    reset_observations_buffer();
  }
  //! CONTINUOUS STOP
  else if (configuration.is_continuous && is_continuous && !is_start && is_stop) {
    samples_processing(true);
  }
  //! CONTINUOUS START-STOP
  else if (configuration.is_continuous && is_continuous && is_start && is_stop) {
    // TCNT1 = TIMER1_TCNT1_VALUE;
    samples_processing(true);
  }
  //! ONESHOT START
  else if (configuration.is_oneshot && is_oneshot && is_start && !is_stop) {
     // reset_samples_buffer();

     // if (!is_event_opc_task) {
     //   is_event_opc_task = true;
     //   ready_tasks_count++;
     // }
   }
   //! ONESHOT STOP
   else if (configuration.is_oneshot && is_oneshot && !is_start && is_stop) {
     // for (uint8_t i = 0; i < OPC_BINS_LENGTH; i++) {
     //   readable_data_write_ptr->bins[i].sample = bins_samples[i].values;
     // }
     //
     // readable_data_write_ptr->pm1.sample = pm1_samples.values;
     // readable_data_write_ptr->pm25.sample = pm25_samples.values;
     // readable_data_write_ptr->pm10.sample = pm10_samples.values;
     // exchange_buffers();
   }
   //! ONESHOT START-STOP
   else if (configuration.is_oneshot && is_oneshot && is_start && is_stop) {
     // for (uint8_t i = 0; i < OPC_BINS_LENGTH; i++) {
     //   readable_data_write_ptr->bins[i].sample = bins_samples[i].values;
     // }
     //
     // readable_data_write_ptr->pm1.sample = pm1_samples.values;
     // readable_data_write_ptr->pm25.sample = pm25_samples.values;
     // readable_data_write_ptr->pm10.sample = pm10_samples.values;
     //
     // exchange_buffers();
     // reset_samples_buffer();

     // if (!is_event_opc_task) {
     //   is_event_opc_task = true;
     //   ready_tasks_count++;
     // }
   }

   interrupts();
}
