/**@file i2c-th.ino */

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
#define SERIAL_TRACE_LEVEL I2C_TH_SERIAL_TRACE_LEVEL

#include "i2c-th.h"

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
         if (is_event_sensors_reading) {
            sensors_reading_task();
            wdt_reset();
         }

         if (is_event_command_task) {
            command_task();
            wdt_reset();
         }

        // I2C Bus Check
        if (i2c_error >= I2C_MAX_ERROR_COUNT) {
          SERIAL_ERROR(F("Restart I2C BUS\r\n"));
          init_wire();
          wdt_reset();
        }

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
   memset((void *) &readable_data_write_ptr->humidity, UINT8_MAX, sizeof(value_t));
   memset((void *) &readable_data_write_ptr->temperature, UINT8_MAX, sizeof(value_t));

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
   is_event_sensors_reading = false;

   sensors_reading_state = SENSORS_READING_INIT;

   //! reset samples_count value
   samples_count = SENSORS_SAMPLE_COUNT_MIN;

   interrupts();
}

void init_pins() {
   pinMode(CONFIGURATION_RESET_PIN, INPUT_PULLUP);
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
   SERIAL_INFO(F("--> i2c temperature address: 0x%X (%d)\r\n"), configuration.i2c_temperature_address, configuration.i2c_temperature_address);
   SERIAL_INFO(F("--> i2c humidity address: 0x%X (%d)\r\n\r\n"), configuration.i2c_humidity_address, configuration.i2c_humidity_address);
}

void save_configuration(bool is_default) {
   if (is_default) {
      SERIAL_INFO(F("Save default configuration... [ %s ]\r\n"), OK_STRING);
      configuration.module_type = MODULE_TYPE;
      configuration.module_version = MODULE_VERSION;
      configuration.i2c_address = CONFIGURATION_DEFAULT_I2C_ADDRESS;
      configuration.is_oneshot = CONFIGURATION_DEFAULT_IS_ONESHOT;
      configuration.is_continuous = CONFIGURATION_DEFAULT_IS_CONTINUOUS;
      configuration.i2c_temperature_address = CONFIGURATION_DEFAULT_TEMPERATURE_ADDRESS;
      configuration.i2c_humidity_address = CONFIGURATION_DEFAULT_HUMIDITY_ADDRESS;

      #if (USE_SENSOR_ADT)
      configuration.i2c_temperature_address = 0x28;
      #endif

      #if (USE_SENSOR_HIH)
      configuration.i2c_humidity_address = 0x28;
      #endif

      #if (USE_SENSOR_HYT)
      configuration.i2c_temperature_address = HYT2X1_DEFAULT_ADDRESS;
      configuration.i2c_humidity_address = HYT2X1_DEFAULT_ADDRESS;
      #endif
   }
   else {
      SERIAL_INFO(F("Save configuration... [ %s ]\r\n"), OK_STRING);
      configuration.i2c_address = writable_data.i2c_address;
      configuration.is_oneshot = writable_data.is_oneshot;
      configuration.is_continuous = writable_data.is_continuous;
      configuration.i2c_temperature_address = writable_data.i2c_temperature_address;
      configuration.i2c_humidity_address = writable_data.i2c_humidity_address;
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
}

void init_sensors () {
   sensors_count = 0;

   SERIAL_INFO(F("Sensors...\r\n"));

   #if (USE_SENSOR_ADT)
   SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_ADT, configuration.i2c_temperature_address, 1, sensors, &sensors_count);
   SERIAL_INFO(F("--> %u: %s-%s: %s\t [ %s ]\r\n"), sensors_count, SENSOR_DRIVER_I2C, SENSOR_TYPE_ADT, "", sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
   #endif

   #if (USE_SENSOR_HIH)
   SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_HIH, configuration.i2c_humidity_address, 1, sensors, &sensors_count);
   SERIAL_INFO(F("--> %u: %s-%s: %s\t [ %s ]\r\n"), sensors_count, SENSOR_DRIVER_I2C, SENSOR_TYPE_HIH, "", sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
   #endif

   #if (USE_SENSOR_HYT)
   SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_HYT, configuration.i2c_temperature_address, 1, sensors, &sensors_count);
   SERIAL_INFO(F("--> %u: %s-%s: %s\t [ %s ]\r\n"), sensors_count, SENSOR_DRIVER_I2C, SENSOR_TYPE_HYT, "", sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
   #endif

   if (configuration.is_continuous) {
      SERIAL_INFO(F("--> acquiring %u~%u samples in %u minutes\r\n\r\n"), SENSORS_SAMPLE_COUNT_MIN, SENSORS_SAMPLE_COUNT_MAX, OBSERVATIONS_MINUTES);
      SERIAL_INFO(F("T-SMP\tT-IST\tT-MIN\tT-MED\tT-MAX\tH-SMP\tH-IST\tH-MIN\tH-MED\tH-MAX\tT-CNT\tH-CNT\r\n"));
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

   //! check if SENSORS_SAMPLE_TIME_MS ms have passed since last time. if true and if is in continuous mode and continuous start command It has been received, activate Sensor reading task
   if (executeTimerTaskEach(timer_counter, SENSORS_SAMPLE_TIME_MS, TIMER1_INTERRUPT_TIME_MS) && configuration.is_continuous && is_continuous && is_start) {
      if (!is_event_sensors_reading) {
         noInterrupts();
         is_event_sensors_reading = true;
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
  if (is_test_read) {
    switch(readable_data_address) {
      case I2C_TH_TEMPERATURE_MED60_ADDRESS:
        readable_data_address = I2C_TH_TEMPERATURE_SAMPLE_ADDRESS;
      break;

      case I2C_TH_HUMIDITY_MED60_ADDRESS:
        readable_data_address = I2C_TH_HUMIDITY_SAMPLE_ADDRESS;
      break;
    }
  }

   //! write readable_data_length bytes of data stored in readable_data_read_ptr (base) + readable_data_address (offset) on i2c bus
   Wire.write((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length);
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

      if (i2c_rx_data[0] == I2C_TH_ADDRESS_ADDRESS && rx_data_length == I2C_TH_ADDRESS_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_TH_ONESHOT_ADDRESS && rx_data_length == I2C_TH_ONESHOT_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_TH_CONTINUOUS_ADDRESS && rx_data_length == I2C_TH_CONTINUOUS_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_TH_TEMPERATURE_ADDRESS_ADDRESS && rx_data_length == I2C_TH_TEMPERATURE_ADDRESS_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_TH_HUMIDITY_ADDRESS_ADDRESS && rx_data_length == I2C_TH_HUMIDITY_ADDRESS_LENGTH) {
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

template<typename sample_g, typename observation_g, typename value_v> void addSample(sample_g *sample, observation_g *observation, value_v value) {
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
  SERIAL_DEBUG(F("%.0f\t \t \t \t \t%.0f\t \t \t \t \t%u\t%u\t%s\r\n"), temperature_samples.values, humidity_samples.values, temperature_samples.count, humidity_samples.count, is_force_processing ? "F" : "N");

  bool is_observations_processing = false;

  //! if true, a new temperature observation was calculated
  bool is_processing_temperature = make_observation_from_samples(is_force_processing, &temperature_samples, &temperature_observations);

  //! if true, a new humidity observation was calculated
  bool is_processing_humidity = make_observation_from_samples(is_force_processing, &humidity_samples, &humidity_observations);

  if (is_processing_temperature || is_processing_humidity) {
    #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_DEBUG)
    resetBackObservation(&temperature_observations, OBSERVATION_COUNT);
    resetBackObservation(&humidity_observations, OBSERVATION_COUNT);

    uint16_t temperature = readBackObservation<observation_t, uint16_t, uint16_t>(&temperature_observations, OBSERVATION_COUNT);
    uint16_t humidity = readBackObservation<observation_t, uint16_t, uint16_t>(&humidity_observations, OBSERVATION_COUNT);

    SERIAL_DEBUG(F("O----------------------------------------------------------------------------------------------\r\n"));
    SERIAL_DEBUG(F("\t%u\t \t \t \t \t%u\t \t \t \t%u/%u\t%u/%u\r\n"), temperature, humidity, temperature_samples.count, samples_count, humidity_samples.count, samples_count);
    SERIAL_DEBUG(F("O----------------------------------------------------------------------------------------------\r\n"));
    #endif

    //! assign new value for samples_count
    if (is_force_processing) {
      samples_count = SENSORS_SAMPLE_COUNT_MAX;
    }
    else samples_count = (samples_count == SENSORS_SAMPLE_COUNT_MAX ? SENSORS_SAMPLE_COUNT_MIN : SENSORS_SAMPLE_COUNT_MAX);

    is_observations_processing = observations_processing();
  }

  if (is_force_processing || is_processing_temperature || is_processing_humidity) {
    //! reset samples's buffer values
    reset_samples_buffer();
  }

  exchange_buffers();

  #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_INFO)
  if (is_observations_processing) {
    SERIAL_INFO(F("R----------------------------------------------------------------------------------------------\r\n"));
    if (isValid(readable_data_read_ptr->temperature.sample)) {
      SERIAL_INFO(F("%u\t"), readable_data_read_ptr->temperature.sample);
    }
    else SERIAL_INFO(F("-----\t"));

    if (isValid(readable_data_read_ptr->temperature.med60)) {
      SERIAL_INFO(F("%u\t"), readable_data_read_ptr->temperature.med60);
    }
    else SERIAL_INFO(F("-----\t"));

    if (isValid(readable_data_read_ptr->temperature.min)) {
      SERIAL_INFO(F("%u\t"), readable_data_read_ptr->temperature.min);
    }
    else SERIAL_INFO(F("-----\t"));

    if (isValid(readable_data_read_ptr->temperature.med)) {
      SERIAL_INFO(F("%u\t"), readable_data_read_ptr->temperature.med);
    }
    else SERIAL_INFO(F("-----\t"));

    if (isValid(readable_data_read_ptr->temperature.max)) {
      SERIAL_INFO(F("%u\t"), readable_data_read_ptr->temperature.max);
    }
    else SERIAL_INFO(F("-----\t"));

    if (isValid(readable_data_read_ptr->humidity.sample)) {
      SERIAL_INFO(F("%u\t"), readable_data_read_ptr->humidity.sample);
    }
    else SERIAL_INFO(F("-----\t"));

    if (isValid(readable_data_read_ptr->humidity.med60)) {
      SERIAL_INFO(F("%u\t"), readable_data_read_ptr->humidity.med60);
    }
    else SERIAL_INFO(F("-----\t"));

    if (isValid(readable_data_read_ptr->humidity.min)) {
      SERIAL_INFO(F("%u\t"), readable_data_read_ptr->humidity.min);
    }
    else SERIAL_INFO(F("-----\t"));

    if (isValid(readable_data_read_ptr->humidity.med)) {
      SERIAL_INFO(F("%u\t"), readable_data_read_ptr->humidity.med);
    }
    else SERIAL_INFO(F("-----\t"));

    if (isValid(readable_data_read_ptr->humidity.max)) {
      SERIAL_INFO(F("%u\r\n"), readable_data_read_ptr->humidity.max);
    }
    else SERIAL_INFO(F("-----\r\n"));
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

template<typename sample_g, typename observation_g, typename value_v, typename val_v> bool make_value_from_samples_and_observations(sample_g *sample, observation_g *observation, value_v *value) {
  //! reset value to default
  value->sample = UINT16_MAX;
  value->med60 = UINT16_MAX;
  value->med = UINT16_MAX;
  value->max = UINT16_MAX;
  value->min = UINT16_MAX;
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
  float mymin = UINT16_MAX;

  //! average value
  float med = 0;

  float sum = 0;
  float sum2 = 0;

  //! maximum value
  float mymax = 0;

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
        mymin = min(mymin, current);

        //! check and assing maximum value
        mymax = max(mymax, current);

        //! average calculation
        med += ((float) current - med) / (float) (count);

        //!  standard deviation
        sum2 += current * current;
        sum += current;

      } else error_count++;
    }

    //! calculate report value only if there are enough good observations
    if (error_count <= OBSERVATION_COUNT_TOLLERANCE) {
      //! assign last sample to report value
      value->sample = (val_v) sample->values;

      //! assign last observation to report value
      value->med60 = (val_v) current;

      //! average
      value->med = (val_v) med;

      //! assign maximum observation to report value
      value->max = (val_v) mymax;

      //! assign minimum observation to report value
      value->min = (val_v) mymin;

      //! calculate standard deviation: make sense for count >=2
      if (count > 1) {
        sigma = round(sqrt((sum2 - (sum * sum) / count) / count));
        //! assign standard deviation to standard deviation report value
        value->sigma = (val_v) sigma;
      }
    }
  }

  return is_processing;
}

//------------------------------------------------------------------------------
// I2C-TH
// STH: oneshot                   --> xxx.sample
// ITH: continuous istantaneous   --> xxx.med60
// MTH: continuous average        --> xxx.med
// NTH: continuous min            --> xxx.min
// XTH: continuous max            --> xxx.max
//------------------------------------------------------------------------------
bool observations_processing() {
  //! if true, a new temperature report was calculated
  bool is_processing_temperature = make_value_from_samples_and_observations<sample_t, observation_t, volatile value_t, float>(&temperature_samples, &temperature_observations, &readable_data_write_ptr->temperature);

  //! if true, a new humidty report was calculated
  bool is_processing_humidty = make_value_from_samples_and_observations<sample_t, observation_t, volatile value_t, float>(&humidity_samples, &humidity_observations, &readable_data_write_ptr->humidity);

  return is_processing_temperature || is_processing_humidty;
}

void sensors_reading_task () {
   static uint8_t i;
   static uint8_t retry;
   static sensors_reading_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;
   static int32_t values_readed_from_sensor[VALUES_TO_READ_FROM_SENSOR_COUNT];

   switch (sensors_reading_state) {
      case SENSORS_READING_INIT:
         //! reset internal state of all sensors to default
         for (i=0; i<sensors_count; i++) {
            sensors[i]->resetPrepared();
         }

         i = 0;
         retry = 0;
         state_after_wait = SENSORS_READING_INIT;
         sensors_reading_state = SENSORS_READING_PREPARE;
      break;

      case SENSORS_READING_PREPARE:
         //! prepare sensor and get delay for complete operation
         sensors[i]->prepare();
         delay_ms = sensors[i]->getDelay();
         start_time_ms = sensors[i]->getStartTime();

         //! if there is any delay, wait it. Otherwise go to next.
         if (delay_ms) {
            state_after_wait = SENSORS_READING_IS_PREPARED;
            sensors_reading_state = SENSORS_READING_WAIT_STATE;
         }
         else {
            sensors_reading_state = SENSORS_READING_IS_PREPARED;
         }
      break;

      case SENSORS_READING_IS_PREPARED:
        //! success
        if (sensors[i]->isPrepared()) {
          sensors_reading_state = SENSORS_READING_GET;
        }
        //! retry
        else if ((++retry) < SENSORS_RETRY_COUNT_MAX) {
          i2c_error++;
          delay_ms = sensors[i]->getDelay();
          start_time_ms = sensors[i]->getStartTime();
          state_after_wait = SENSORS_READING_PREPARE;
          sensors_reading_state = SENSORS_READING_WAIT_STATE;
        }
        //! fail
        else {
          sensors_reading_state = SENSORS_READING_GET;
        }
      break;

      case SENSORS_READING_GET:
         //! get VALUES_TO_READ_FROM_SENSOR_COUNT values from sensor and store it in values_readed_from_sensor.
         sensors[i]->get(values_readed_from_sensor, VALUES_TO_READ_FROM_SENSOR_COUNT);
         delay_ms = sensors[i]->getDelay();
         start_time_ms = sensors[i]->getStartTime();

         if (delay_ms) {
            state_after_wait = SENSORS_READING_IS_GETTED;
            sensors_reading_state = SENSORS_READING_WAIT_STATE;
         }
         else {
            sensors_reading_state = SENSORS_READING_IS_GETTED;
         }
      break;

      case SENSORS_READING_IS_GETTED:
        //! end of internal sensor state (finished read)
        if (sensors[i]->isEnd() && !sensors[i]->isReaded()) {
          //! success
          if (sensors[i]->isSuccess()) {
            sensors_reading_state = SENSORS_READING_READ;
          }
          //! retry
          else if ((++retry) < SENSORS_RETRY_COUNT_MAX) {
            i2c_error++;
            delay_ms = sensors[i]->getDelay();
            start_time_ms = sensors[i]->getStartTime();
            state_after_wait = SENSORS_READING_PREPARE;
            sensors_reading_state = SENSORS_READING_WAIT_STATE;
          }
          //! fail
          else {
            sensors_reading_state = SENSORS_READING_READ;
          }
        }
        //! process other internal sensor state
        else {
          sensors_reading_state = SENSORS_READING_GET;
        }
      break;

      case SENSORS_READING_READ:
        //! read sensor value
        #if (USE_SENSOR_ADT)
        if (strcmp(sensors[i]->getType(), SENSOR_TYPE_ADT) == 0) {
          addSample(&temperature_samples, &temperature_observations, values_readed_from_sensor[0]);
        }
        #endif

        #if (USE_SENSOR_HIH)
        if (strcmp(sensors[i]->getType(), SENSOR_TYPE_HIH) == 0) {
          addSample(&humidity_samples, &humidity_observations, values_readed_from_sensor[0]);
          addSample(&temperature_samples, &temperature_observations, values_readed_from_sensor[1]);
        }
        #endif

        #if (USE_SENSOR_HYT)
        if (strcmp(sensors[i]->getType(), SENSOR_TYPE_HYT) == 0) {
          addSample(&humidity_samples, &humidity_observations, values_readed_from_sensor[0]);
          addSample(&temperature_samples, &temperature_observations, values_readed_from_sensor[1]);
        }
        #endif

        sensors_reading_state = SENSORS_READING_NEXT;
      break;

      case SENSORS_READING_NEXT:
         //! go to next sensor
         if ((++i) < sensors_count) {
            retry = 0;
            sensors_reading_state = SENSORS_READING_PREPARE;
         }
         //! end (there are no other sensors to read)
         else {
            //! if it is in continuous mode, do samples processing
            if (configuration.is_continuous) {
              noInterrupts();
              samples_processing(false);
              interrupts();
            }

            #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_INFO)
            delay_ms = 10;
            start_time_ms = millis();
            state_after_wait = SENSORS_READING_END;
            sensors_reading_state = SENSORS_READING_WAIT_STATE;
            #else
            sensors_reading_state = SENSORS_READING_END;
            #endif
         }
      break;

      case SENSORS_READING_END:
        noInterrupts();
        is_event_sensors_reading = false;
        ready_tasks_count--;
        interrupts();
        sensors_reading_state = SENSORS_READING_INIT;
      break;

      case SENSORS_READING_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            sensors_reading_state = state_after_wait;
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
  temperature_samples.values = UINT16_MAX;
  temperature_samples.count = 0;
  temperature_samples.error_count = 0;

  humidity_samples.values = UINT16_MAX;
  humidity_samples.count = 0;
  humidity_samples.error_count = 0;
}

void reset_observations_buffer() {
  resetObservation(&temperature_observations, OBSERVATION_COUNT);
  resetObservation(&humidity_observations, OBSERVATION_COUNT);
}

void resetObservation (observation_t *buffer, uint16_t length) {
  memset((void *) buffer->med, UINT8_MAX, length * sizeof(uint16_t));
  buffer->count = 0;
  buffer->read_ptr = buffer->med;
  buffer->write_ptr = buffer->med;
}

void resetBackObservation (observation_t *buffer, uint16_t length) {
  if (buffer->write_ptr == buffer->med) {
    buffer->read_ptr = buffer->med+length-1;
  }
  else buffer->read_ptr = buffer->write_ptr-1;
}

void addObservation (observation_t *buffer, uint16_t length, uint16_t value) {
  *buffer->write_ptr = value;

  if (buffer->count < length) {
    buffer->count++;
  }

  if (buffer->write_ptr+1 < buffer->med + length) {
    buffer->write_ptr++;
  } else buffer->write_ptr = buffer->med;
}

uint16_t readBackObservation (observation_t *buffer, uint16_t length) {
  uint16_t value = *buffer->read_ptr;

  if (buffer->read_ptr == buffer->med) {
    buffer->read_ptr = buffer->med+length-1;
  }
  else buffer->read_ptr--;

  return value;
}

void command_task() {
   #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
   char buffer[30];
   #endif

   switch(i2c_rx_data[1]) {
      case I2C_TH_COMMAND_ONESHOT_START:
         #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
         strcpy(buffer, "ONESHOT START");
         #endif
         is_oneshot = true;
         is_continuous = false;
         is_start = true;
         is_stop = false;
         commands();
      break;

      case I2C_TH_COMMAND_ONESHOT_STOP:
         #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
         strcpy(buffer, "ONESHOT STOP");
         #endif
         is_oneshot = true;
         is_continuous = false;
         is_start = false;
         is_stop = true;
         commands();
      break;

      case I2C_TH_COMMAND_ONESHOT_START_STOP:
         #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
         strcpy(buffer, "ONESHOT START-STOP");
         #endif
         is_oneshot = true;
         is_continuous = false;
         is_start = true;
         is_stop = true;
         commands();
      break;

      case I2C_TH_COMMAND_CONTINUOUS_START:
         #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
         strcpy(buffer, "CONTINUOUS START");
         #endif
         is_oneshot = false;
         is_continuous = true;
         is_start = true;
         is_stop = false;
         commands();
      break;

      case I2C_TH_COMMAND_CONTINUOUS_STOP:
         #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
         strcpy(buffer, "CONTINUOUS STOP");
         #endif
         is_oneshot = false;
         is_continuous = true;
         is_start = false;
         is_stop = true;
         commands();
      break;

      case I2C_TH_COMMAND_CONTINUOUS_START_STOP:
        #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
        strcpy(buffer, "CONTINUOUS START-STOP");
        #endif
        is_oneshot = false;
        is_continuous = true;
        is_start = true;
        is_stop = true;
        commands();
      break;

      case I2C_TH_COMMAND_TEST_READ:
         #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
         strcpy(buffer, "TEST READ");
         #endif
         is_test_read = true;
         tests();
      break;

      case I2C_TH_COMMAND_SAVE:
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
  if (is_test_read) {
    noInterrupts();

    if (temperature_samples.count && humidity_samples.count) {
      if (isValid(temperature_samples.values) && isValid(humidity_samples.values)) {
        readable_data_write_ptr->temperature.sample = (uint16_t) temperature_samples.values;
        readable_data_write_ptr->humidity.sample = (uint16_t) humidity_samples.values;
        SERIAL_DEBUG(F("%.0f\t \t \t \t \t%.0f\t \t \t \t \t%u\t%u\t%s\r\n"), temperature_samples.values, humidity_samples.values, temperature_samples.count, humidity_samples.count, "T");
        exchange_buffers();
      }
    }

    interrupts();
  }
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
    TCNT1 = TIMER1_TCNT1_VALUE;
    samples_processing(true);
  }
  //! ONESHOT START
  else if (configuration.is_oneshot && is_oneshot && is_start && !is_stop) {
    reset_samples_buffer();

    if (!is_event_sensors_reading) {
      is_event_sensors_reading = true;
      ready_tasks_count++;
    }
  }
  //! ONESHOT STOP
  else if (configuration.is_oneshot && is_oneshot && !is_start && is_stop) {
    readable_data_write_ptr->temperature.sample = temperature_samples.values;
    readable_data_write_ptr->humidity.sample = humidity_samples.values;
    exchange_buffers();
  }
  //! ONESHOT START-STOP
  else if (configuration.is_oneshot && is_oneshot && is_start && is_stop) {
    readable_data_write_ptr->temperature.sample = temperature_samples.values;
    readable_data_write_ptr->humidity.sample = humidity_samples.values;
    exchange_buffers();
    reset_samples_buffer();

      if (!is_event_sensors_reading) {
         is_event_sensors_reading = true;
         ready_tasks_count++;
      }
   }

   interrupts();
}
