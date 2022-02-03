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

#include "i2c-th.h"

/*!
\fn void setup()
\brief Arduino setup function.
*  Init watchdog, hardware, debug, buffer and load configuration stored in EEPROM.
\return void.
*/
void setup() {
   init_wdt(WDT_TIMER);
   Serial.begin(115200);
   init_pins();
   init_spi();
   init_logging();
   load_configuration();
   init_buffers();
   init_wire();
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
         #if (ENABLE_SDCARD_LOGGING)   
	 logFile.flush();
	 #endif
	 Serial.flush();	
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
          LOGE(F("Restart I2C BUS"));
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


void logPrefix(Print* _logOutput) {
  char m[12];
  sprintf(m, "%10lu ", millis());
  _logOutput->print("#");
  _logOutput->print(m);
  _logOutput->print(": ");
}

void logSuffix(Print* _logOutput) {
  _logOutput->print('\n');
  _logOutput->flush();  // we use this to flush every log message
}

void init_logging(){
  
#if (ENABLE_SDCARD_LOGGING)      
  Serial.println("\nInitializing SD card..." );
  
  if (!SD.begin(SDCARD_CHIP_SELECT_PIN,SPI_SPEED)){
    Serial.println   (F("initialization failed. Things to check:"));
    Serial.println   (F("* is a card inserted?"));
    Serial.println   (F("* is your wiring correct?"));
    Serial.println   (F("* did you change the chipSelect pin to match your shield or module?"));
  } else {
    Serial.println   (F("Wiring is correct and a card is present."));
    Serial.print     (F("The FAT type of the volume: "));
    Serial.println   (SD.vol()->fatType());

    // remove firmware to do not redo update the next reboot
    if (sdcard_remove_firmware(&SD, MODULE_MAIN_VERSION, MODULE_MINOR_VERSION)){
      LOGN(F("removed firmware version %d.%d from SD"),MODULE_MAIN_VERSION, MODULE_MINOR_VERSION);
    }
  }
  
  logFile= SD.open(SDCARD_LOGGING_FILE_NAME, O_RDWR | O_CREAT | O_APPEND);
  if (logFile) {
    logFile.seekEnd(0);
    Log.begin(LOG_LEVEL, &loggingStream);
  } else {
    Log.begin(LOG_LEVEL, &Serial);
  }
#else
  Log.begin(LOG_LEVEL, &Serial);
#endif
  
  Log.setPrefix(logPrefix);
  Log.setSuffix(logSuffix);
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
   readable_data_write_ptr->module_main_version = MODULE_MAIN_VERSION;
   readable_data_write_ptr->module_minor_version = MODULE_CONFIGURATION_VERSION;
   memset((void *) &readable_data_write_ptr->humidity, UINT8_MAX, sizeof(value_t));
   memset((void *) &readable_data_write_ptr->temperature, UINT8_MAX, sizeof(value_t));

   //! copy readable_data_2 in readable_data_1
   memcpy((void *) readable_data_read_ptr, (const void*) readable_data_write_ptr, sizeof(readable_data_t));

   reset_samples_buffer();
   reset_observations_buffer();

   readable_data_address=0xFF;
   readable_data_length=0;
}

void init_tasks() {
   noInterrupts();

   //! no tasks ready
   ready_tasks_count = 0;

   is_event_command_task = false;
   is_event_sensors_reading = false;
   is_test_read = false;

   sensors_reading_state = SENSORS_READING_INIT;

   //! reset samples_count value
   samples_count = SENSORS_SAMPLE_COUNT_MIN;

   interrupts();
}

void init_pins() {
   pinMode(CONFIGURATION_RESET_PIN, INPUT_PULLUP);
   pinMode(SDCARD_CHIP_SELECT_PIN, OUTPUT);
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
#if (ENABLE_SDCARD_LOGGING)      
  SPI.begin();
#endif
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
   LOGN(F("--> type: %s"), stima_name);
   LOGN(F("--> version: %d.%d"), MODULE_MAIN_VERSION, MODULE_MINOR_VERSION);   
   LOGN(F("--> configuration version: %d.%d"), configuration.module_main_version, configuration.module_configuration_version);
   LOGN(F("--> i2c address: %X (%d)"), configuration.i2c_address, configuration.i2c_address);
   LOGN(F("--> oneshot: %s"), configuration.is_oneshot ? ON_STRING : OFF_STRING);
   LOGN(F("--> continuous: %s"), configuration.is_continuous ? ON_STRING : OFF_STRING);
   LOGN(F("--> i2c temperature address: %X (%d)"), configuration.i2c_temperature_address, configuration.i2c_temperature_address);
   LOGN(F("--> i2c humidity address: %X (%d)"), configuration.i2c_humidity_address, configuration.i2c_humidity_address);
}

void save_configuration(bool is_default) {
   if (is_default) {
      LOGN(F("Save default configuration... [ %s ]"), OK_STRING);
      configuration.module_type = MODULE_TYPE;
      configuration.module_main_version = MODULE_MAIN_VERSION;
      configuration.module_configuration_version = MODULE_CONFIGURATION_VERSION;
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
      LOGN(F("Save configuration... [ %s ]"), OK_STRING);
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

   if (configuration.module_type != MODULE_TYPE || configuration.module_main_version != MODULE_MAIN_VERSION || configuration.module_configuration_version != MODULE_CONFIGURATION_VERSION || digitalRead(CONFIGURATION_RESET_PIN) == LOW) {
      save_configuration(CONFIGURATION_DEFAULT);
   }
   else {
      LOGN(F("Load configuration... [ %s ]"), OK_STRING);
      print_configuration();
   }

   writable_data.i2c_address = configuration.i2c_address;
   writable_data.is_oneshot = configuration.is_oneshot;
}

void init_sensors () {
   sensors_count = 0;

   LOGN(F("Sensors..."));

   #if (USE_SENSOR_ADT)
   SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_ADT, configuration.i2c_temperature_address, 1, sensors, &sensors_count);
   LOGN(F("--> %d: %s-%s: %s\t [ %s ]"), sensors_count, SENSOR_DRIVER_I2C, SENSOR_TYPE_ADT, "", sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
   #endif

   #if (USE_SENSOR_HIH)
   SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_HIH, configuration.i2c_humidity_address, 1, sensors, &sensors_count);
   LOGN(F("--> %d: %s-%s: %s\t [ %s ]"), sensors_count, SENSOR_DRIVER_I2C, SENSOR_TYPE_HIH, "", sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
   #endif

   #if (USE_SENSOR_HYT)
   SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, SENSOR_TYPE_HYT, configuration.i2c_temperature_address, 1, sensors, &sensors_count);
   LOGN(F("--> %d: %s-%s: %s\t [ %s ]"), sensors_count, SENSOR_DRIVER_I2C, SENSOR_TYPE_HYT, "", sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
   #endif

   if (configuration.is_continuous) {
      LOGN(F("--> acquiring %d~%d samples in %d minutes"), SENSORS_SAMPLE_COUNT_MIN, SENSORS_SAMPLE_COUNT_MAX, OBSERVATIONS_MINUTES);
      //LOGN(F("T-SMP\tT-IST\tT-MIN\tT-MED\tT-MAX\tH-SMP\tH-IST\tH-MIN\tH-MED\tH-MAX\tT-CNT\tH-CNT"));
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
  Wire.write(crc8((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length));
  //LOGV("request_interrupt_handler: %d-%d crc:%d",readable_data_address,readable_data_length,crc8((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length));
  
  readable_data_address=0xFF;
  readable_data_length=0;
}

void i2c_receive_interrupt_handler(int rx_data_length) {
  bool is_i2c_data_ok = false;

  // read rx_data_length bytes of data from i2c bus
  for (uint8_t i = 0; i < rx_data_length; i++) {
    i2c_rx_data[i] = Wire.read();
  }

  if (rx_data_length < 2) {
    // no payload and CRC as for scan I2c bus
    //LOGN(F("No CRC: size %d"),rx_data_length);
  } else   
  //! check crc: ok
  if (i2c_rx_data[rx_data_length - 1] == crc8((uint8_t *)i2c_rx_data, rx_data_length - 1)) {
    rx_data_length--;

    // it is a registers read?
    if (rx_data_length == 2 && is_readable_register(i2c_rx_data[0])) {
      // offset in readable_data_read_ptr buffer
      readable_data_address = i2c_rx_data[0];

      // length (in bytes) of data to be read in readable_data_read_ptr
      readable_data_length = i2c_rx_data[1];
      //LOGV(F("set readable_data: %d-%d"),readable_data_address,readable_data_length);
    }
    // it is a command?
    else if (rx_data_length == 2 && is_command(i2c_rx_data[0])) {
      //noInterrupts();
      // enable Command task
      if (!is_event_command_task) {
        is_event_command_task = true;
        ready_tasks_count++;
      }
      //interrupts();
    }
    // it is a registers write?
    else if (is_writable_register(i2c_rx_data[0])) {
      rx_data_length -= 1;

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
          ((uint8_t *)writable_data_ptr)[i2c_rx_data[0] - I2C_WRITE_REGISTER_START_ADDRESS + i] = i2c_rx_data[i + 1];
        }
      }
    }
  } else {
    readable_data_address=0xFF;
    readable_data_length = 0;
    //LOGE(F("CRC error: size %d  CRC %d:%d"),rx_data_length,i2c_rx_data[rx_data_length - 1], crc8((uint8_t *)(i2c_rx_data), rx_data_length - 1));
    i2c_error++;
  }
}

template<typename sample_g, typename observation_g, typename value_v> void addSample(sample_g *sample, observation_g *observation, value_v value) {
  if (ISVALID(value)) {
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
  LOGT(F("%0\t%0\t%d\t%d\t%s"), temperature_samples.values, humidity_samples.values, temperature_samples.count, humidity_samples.count, is_force_processing ? "F" : "N");

  bool is_observations_processing = false;

  //! if true, a new temperature observation was calculated
  bool is_processing_temperature = make_observation_from_samples(is_force_processing, &temperature_samples, &temperature_observations);

  //! if true, a new humidity observation was calculated
  bool is_processing_humidity = make_observation_from_samples(is_force_processing, &humidity_samples, &humidity_observations);

  if (is_processing_temperature || is_processing_humidity) {
    #if (LOG_LEVEL >= LOG_LEVEL_TRACE)
    resetBackObservation(&temperature_observations, OBSERVATION_COUNT);
    resetBackObservation(&humidity_observations, OBSERVATION_COUNT);

    uint16_t temperature = readBackObservation<observation_t, uint16_t, uint16_t>(&temperature_observations, OBSERVATION_COUNT);
    uint16_t humidity = readBackObservation<observation_t, uint16_t, uint16_t>(&humidity_observations, OBSERVATION_COUNT);

    LOGT(F("O->\t%d\t%d\t%d/%d\t%d/%d O<-"), temperature, humidity, temperature_samples.count, samples_count, humidity_samples.count, samples_count);
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

  #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
  if (is_observations_processing) {
    LOGN(F("R->"));

    if (ISVALID(readable_data_read_ptr->temperature.sample)) {
      LOGN(F("T-SMP: %d"), readable_data_read_ptr->temperature.sample);
    }
    else LOGN(F("T-SMP: -----"));

    if (ISVALID(readable_data_read_ptr->temperature.med60)) {
      LOGN(F("T-IST: %d"), readable_data_read_ptr->temperature.med60);
    }
    else LOGN(F("T-IST: -----"));

    if (ISVALID(readable_data_read_ptr->temperature.min)) {
      LOGN(F("T-MIN: %d"), readable_data_read_ptr->temperature.min);
    }
    else LOGN(F("T-MIN: -----"));

    if (ISVALID(readable_data_read_ptr->temperature.med)) {
      LOGN(F("T-MED: %d"), readable_data_read_ptr->temperature.med);
    }
    else LOGN(F("T-MED: -----"));

    if (ISVALID(readable_data_read_ptr->temperature.max)) {
      LOGN(F("T-MAX: %d"), readable_data_read_ptr->temperature.max);
    }
    else LOGN(F("T-MAX: -----"));

    if (ISVALID(readable_data_read_ptr->humidity.sample)) {
      LOGN(F("H-SMP: %d"), readable_data_read_ptr->humidity.sample);
    }
    else LOGN(F("H-SMP: -----"));

    if (ISVALID(readable_data_read_ptr->humidity.med60)) {
      LOGN(F("H-IST: %d"), readable_data_read_ptr->humidity.med60);
    }
    else LOGN(F("H-IST: -----"));

    if (ISVALID(readable_data_read_ptr->humidity.min)) {
      LOGN(F("H-MIN: %d"), readable_data_read_ptr->humidity.min);
    }
    else LOGN(F("H-MIN: -----"));

    if (ISVALID(readable_data_read_ptr->humidity.med)) {
      LOGN(F("H-MED: %d"), readable_data_read_ptr->humidity.med);
    }
    else LOGN(F("H-MED: -----"));

    if (ISVALID(readable_data_read_ptr->humidity.max)) {
      LOGN(F("H-MAX: %d"), readable_data_read_ptr->humidity.max);
    }
    else LOGN(F("H-MAX: -----"));
    LOGN(F("R<-"));
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
      if (ISVALID(current)) {
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
   static uint8_t retry_prepare;
   static uint8_t retry_get;
   static sensors_reading_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;
   static int32_t values_readed_from_sensor[VALUES_TO_READ_FROM_SENSOR_COUNT];
   static char json_sensors_data[JSON_BUFFER_LENGTH];

   switch (sensors_reading_state) {
      case SENSORS_READING_INIT:
         //! reset internal state of all sensors to default
         for (i=0; i<sensors_count; i++) {
            sensors[i]->resetPrepared();
         }

         i = 0;
         retry_prepare = 0;
         retry_get = 0;
         state_after_wait = SENSORS_READING_INIT;
         sensors_reading_state = SENSORS_SETUP_CHECK;
      break;

   case SENSORS_SETUP_CHECK:

        LOGV(F("Sensor error count: %d"),sensors[i]->getErrorCount());
     
	if (sensors[i]->getErrorCount() > SENSOR_ERROR_COUNT_MAX){
	  LOGE(F("Sensor i2c error > SENSOR_ERROR_COUNT_MAX"));
	  sensors[i]->resetSetted();
	}
	
	if (!sensors[i]->isSetted()) {
	  sensors[i]->setup();
	}

        if (sensors[i]->isSetted()) {
	  sensors_reading_state = SENSORS_READING_PREPARE;
	}else{
	  LOGE(F("Skip failed Sensor"));
	  addSample(&temperature_samples, &temperature_observations, UINT32_MAX);
	  addSample(&humidity_samples, &humidity_observations, UINT32_MAX);
	  
	  sensors_reading_state = SENSORS_READING_NEXT;
	} 

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
	  retry_prepare = 0;
        }
        //! retry
        else if ((++retry_prepare) < SENSORS_RETRY_COUNT_MAX) {
          i2c_error++;
	  delay_ms = SENSORS_RETRY_DELAY_MS;
	  start_time_ms = millis();
	  state_after_wait = SENSORS_READING_PREPARE;
	  sensors_reading_state = SENSORS_READING_WAIT_STATE;
	  LOGE(F("Sensor is prepared... [ retry ]"));
        }
        //! fail
        else {
	  sensors_reading_state = SENSORS_READING_NEXT;
	  addSample(&temperature_samples, &temperature_observations, UINT32_MAX);
	  addSample(&humidity_samples, &humidity_observations, UINT32_MAX);

	  LOGE(F("Sensor is prepared... [ %s ]"),FAIL_STRING);
	  retry_prepare = 0;
          i2c_error++;
	}
      break;

      case SENSORS_READING_GET:
         //! get VALUES_TO_READ_FROM_SENSOR_COUNT values from sensor and store it in values_readed_from_sensor.
	sensors[i]->getJson(values_readed_from_sensor, VALUES_TO_READ_FROM_SENSOR_COUNT,json_sensors_data);
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
	    i2c_error = 0;
            sensors_reading_state = SENSORS_READING_READ;
          }
          //! retry
          else if ((++retry_get) < SENSORS_RETRY_COUNT_MAX) {
	    i2c_error++;
	    delay_ms = SENSORS_RETRY_DELAY_MS;
	    start_time_ms = millis();
	    //state_after_wait = SENSORS_READING_GET;
	    state_after_wait = SENSORS_READING_PREPARE;
	    sensors_reading_state = SENSORS_READING_WAIT_STATE;
	    LOGE(F("Sensor is getted... [ retry ]"));
          }
          //! fail
          else {
	    i2c_error++;
	    LOGE(F("Sensor is getted... [ %s ]"),FAIL_STRING);
            sensors_reading_state = SENSORS_READING_READ;
          }
        }
        //! process other internal sensor state
        else {
	  LOGT(F("Sensor is getted... [ not end ]"));
          sensors_reading_state = SENSORS_READING_GET;
        }
      break;

      case SENSORS_READING_READ:
        //! read sensor value
	/*
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
	*/
	{
	  StaticJsonDocument<JSON_BUFFER_LENGTH*2> doc;
	  DeserializationError error = deserializeJson(doc,json_sensors_data);
	  if (error) {
	    LOGE(F("deserializeJson() failed with code %s"),error.f_str());
	    addSample(&temperature_samples, &temperature_observations, UINT32_MAX);
	    addSample(&humidity_samples, &humidity_observations, UINT32_MAX);
	  }else{
	    unsigned long int value = doc["B12101"] | UINT32_MAX;
	    addSample(&temperature_samples, &temperature_observations, value);
	    
	    value = doc["B13003"] | UINT32_MAX;
	    addSample(&humidity_samples, &humidity_observations, value);
	  }
	}
        sensors_reading_state = SENSORS_READING_NEXT;
      break;

      case SENSORS_READING_NEXT:
         //! go to next sensor
         if ((++i) < sensors_count) {
            retry_prepare = 0;
            retry_get = 0;
            sensors_reading_state = SENSORS_SETUP_CHECK;
         }
         //! end (there are no other sensors to read)
         else {
            //! if it is in continuous mode, do samples processing
            if (configuration.is_continuous) {
              samples_processing(false);
            }
            sensors_reading_state = SENSORS_READING_END;
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
   noInterrupts();
   readable_data_temp_ptr = readable_data_write_ptr;
   readable_data_write_ptr = readable_data_read_ptr;
   readable_data_read_ptr = readable_data_temp_ptr;
   interrupts();
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
   #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
   char buffer[30];
   #endif

   switch(i2c_rx_data[1]) {
      case I2C_TH_COMMAND_ONESHOT_START:
         #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
         strcpy(buffer, "ONESHOT START");
         #endif
         is_oneshot = true;
         is_continuous = false;
         is_start = true;
         is_stop = false;
         commands();
      break;

      case I2C_TH_COMMAND_ONESHOT_STOP:
         #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
         strcpy(buffer, "ONESHOT STOP");
         #endif
         is_oneshot = true;
         is_continuous = false;
         is_start = false;
         is_stop = true;
	 is_test_read = false;
         commands();
      break;

      case I2C_TH_COMMAND_ONESHOT_START_STOP:
         #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
         strcpy(buffer, "ONESHOT START-STOP");
         #endif
         is_oneshot = true;
         is_continuous = false;
         is_start = true;
         is_stop = true;
	 is_test_read = false;
         commands();
      break;

      case I2C_TH_COMMAND_CONTINUOUS_START:
         #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
         strcpy(buffer, "CONTINUOUS START");
         #endif
         is_oneshot = false;
         is_continuous = true;
         is_start = true;
         is_stop = false;
	 is_test_read = false;
         commands();
      break;

      case I2C_TH_COMMAND_CONTINUOUS_STOP:
         #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
         strcpy(buffer, "CONTINUOUS STOP");
         #endif
         is_oneshot = false;
         is_continuous = true;
         is_start = false;
         is_stop = true;
	 is_test_read = false;
	 commands();
      break;

      case I2C_TH_COMMAND_CONTINUOUS_START_STOP:
        #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
        strcpy(buffer, "CONTINUOUS START-STOP");
        #endif
        is_oneshot = false;
        is_continuous = true;
        is_start = true;
        is_stop = true;
	is_test_read = false;
        commands();
      break;

      case I2C_TH_COMMAND_TEST_READ:
         #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
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
        LOGN(F("Execute command [ SAVE ]"));
        save_configuration(CONFIGURATION_CURRENT);
        init_wire();
      break;
   }

   #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
   if (configuration.is_oneshot == is_oneshot || configuration.is_continuous == is_continuous) {
      LOGN(F("Execute [ %s ]"), buffer);
   }
   else if (configuration.is_oneshot == is_continuous || configuration.is_continuous == is_oneshot) {
      LOGN(F("Ignore [ %s ]"), buffer);
   }
   #endif

   noInterrupts();
   is_event_command_task = false;
   ready_tasks_count--;
   interrupts();
}

void tests() {
  if (temperature_samples.count && humidity_samples.count) {
    if (ISVALID(temperature_samples.values) && ISVALID(humidity_samples.values)) {
      readable_data_write_ptr->temperature.sample = (uint16_t) temperature_samples.values;
      readable_data_write_ptr->humidity.sample = (uint16_t) humidity_samples.values;
      LOGT(F("%0\t%0\t%d\t%d\t%s"), temperature_samples.values, humidity_samples.values, temperature_samples.count, humidity_samples.count, "T");
      exchange_buffers();
    }

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
