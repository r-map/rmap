/**@file i2c-thr.ino */

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

#include "i2c-thr.h"

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
         #if (USE_SENSOR_TBS || USE_SENSOR_TBR)
         init_power_down(&awakened_event_occurred_time_ms, configuration.tipping_bucket_time_ms + DEBOUNCING_POWER_DOWN_TIME_MS);
         #else
         init_power_down(&awakened_event_occurred_time_ms, DEBOUNCING_POWER_DOWN_TIME_MS);
         #endif
         state = TASKS_EXECUTION;
      break;
      #endif

      case TASKS_EXECUTION:
         // I2C Bus Check
         if (i2c_error >= I2C_MAX_ERROR_COUNT) {
            LOGT(F("Restart I2C BUS"));
            init_wire();
            wdt_reset();
         }

         #if (USE_MODULE_RAIN || USE_MODULE_THR)
         if (is_event_tipping_bucket) {
            tipping_bucket_task();
            wdt_reset();
         }
         #endif

         #if (USE_MODULE_TH || USE_MODULE_THR)
         if (is_event_sensors_reading) {
            sensors_reading_task();
            wdt_reset();
         }
         #endif

         if (is_event_command_task) {
            command_task();
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

void logPrefixClear(Print* _logOutput) {
}

void logSuffixClear(Print* _logOutput) {
   _logOutput->flush();  // we use this to flush every log message
}

void init_logging() {
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

   reset_buffer();
   reset_samples_buffer();
   reset_report_buffer();

   //! copy readable_data_2 in readable_data_1
   memcpy((void *) readable_data_read_ptr, (const void*) readable_data_write_ptr, sizeof(readable_data_t));
}

void init_tasks() {
   noInterrupts();

   //! no tasks ready
   ready_tasks_count = 0;

   #if (USE_SENSOR_TBR)
   is_event_tipping_bucket = false;
   tipping_bucket_state = TIPPING_BUCKET_INIT;
   // reset tipping bucket debounce value
   rain_tips_event_occurred_time_ms = -configuration.tipping_bucket_time_ms;
   #endif

   #if (USE_SENSOR_ADT || USE_SENSOR_HIH || USE_SENSOR_HYT)
   is_event_sensors_reading = false;
   sensors_reading_state = SENSORS_READING_INIT;
   #endif

   is_event_command_task = false;

   is_oneshot = false;
   is_continuous = false;
   is_start = false;
   is_stop = false;

   interrupts();
}

void init_pins() {
   pinMode(CONFIGURATION_RESET_PIN, INPUT_PULLUP);

   #if (USE_MODULE_RAIN || USE_MODULE_THR)
   pinMode(TIPPING_BUCKET_PIN, INPUT_PULLUP);
   attachInterrupt(digitalPinToInterrupt(TIPPING_BUCKET_PIN), tipping_bucket_interrupt_handler, LOW);
   #endif

   #if (ENABLE_SDCARD_LOGGING)
   pinMode(SDCARD_CHIP_SELECT_PIN, OUTPUT);
   #endif
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
   timer_counter_ms = 0;
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
   i2c_error = 0;
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

   #if (USE_MODULE_THR || USE_MODULE_TH)
   LOGN(F("--> i2c temperature address: %X (%d)"), configuration.i2c_temperature_address, configuration.i2c_temperature_address);
   LOGN(F("--> i2c humidity address: %X (%d)"), configuration.i2c_humidity_address, configuration.i2c_humidity_address);
   #endif

   #if (USE_MODULE_THR || USE_MODULE_RAIN)
   LOGN(F("--> Tipping bucket time in milliseconds: %d"), configuration.tipping_bucket_time_ms);
   LOGN(F("--> How much mm of rain for one tip of tipping bucket rain gauge: %d"), configuration.rain_for_tip);
   #endif
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

      #if (USE_MODULE_THR || USE_MODULE_TH)
      configuration.i2c_temperature_address = CONFIGURATION_DEFAULT_TEMPERATURE_ADDRESS;
      configuration.i2c_humidity_address = CONFIGURATION_DEFAULT_HUMIDITY_ADDRESS;
      #endif

      #if (USE_MODULE_THR || USE_MODULE_RAIN)
      configuration.tipping_bucket_time_ms = CONFIGURATION_DEFAULT_TIPPING_BUCKET_TIME_MS;
      configuration.rain_for_tip = CONFIGURATION_DEFAULT_RAIN_FOR_TIP;
      #endif

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

      #if (USE_MODULE_THR || USE_MODULE_TH)
      configuration.i2c_temperature_address = writable_data.i2c_temperature_address;
      configuration.i2c_humidity_address = writable_data.i2c_humidity_address;
      #endif

      #if (USE_MODULE_THR || USE_MODULE_RAIN)
      configuration.tipping_bucket_time_ms = writable_data.tipping_bucket_time_ms;
      configuration.rain_for_tip = writable_data.rain_for_tip;
      #endif
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
   writable_data.is_continuous = configuration.is_continuous;
}

void init_sensors () {
   sensors_count = 0;

   #if (USE_MODULE_THR || USE_MODULE_TH)
   LOGN(F("Sensors..."));
   #endif

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
      LOGN(F("--> acquiring %d~%d samples in %d minutes"), OBSERVATION_SAMPLES_COUNT_MIN, OBSERVATION_SAMPLES_COUNT_MAX, OBSERVATIONS_MINUTES);
      LOGN(F("--> max %d samples error in %d minutes (observation)"), OBSERVATION_SAMPLE_ERROR_MAX, OBSERVATIONS_MINUTES);
      LOGN(F("--> max %d samples error in %d minutes (report)\r\n"), RMAP_REPORT_SAMPLE_ERROR_MAX, STATISTICAL_DATA_COUNT * OBSERVATIONS_MINUTES);
   }

   #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
   LOGN(F("tsc: temperature sample count"));
   LOGN(F("tmp: temperature sample"));
   #endif

   #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
   LOGN(F("hsc: humidity sample count"));
   LOGN(F("hum: humidity sample"));
   #endif

   #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
   LOGN(F("tist: average temperature in the ist %d minutes"), OBSERVATIONS_MINUTES);
   LOGN(F("tmin: minimum temperature"));
   LOGN(F("tavg: average temperature"));
   LOGN(F("tmax: maximum temperature"));
   LOGN(F("tsgm: sigma temperature"));
   #endif

   #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
   LOGN(F("hist: average humidity in the ist %d minutes"), OBSERVATIONS_MINUTES);
   LOGN(F("hmin: minimum humidity"));
   LOGN(F("havg: average humidity"));
   LOGN(F("hmax: maximum humidity"));
   LOGN(F("hsig: sigma humidity"));
   #endif

   #if (USE_MODULE_TBR || USE_MODULE_TBS)
   LOGN(F("rtc: rain tips count"));
   #endif

   Log.setSuffix(logSuffixClear);
   LOGN(F(""));
   Log.setPrefix(logPrefixClear);

   #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
   LOGN(F("tsc\ttmp\t"));
   #endif

   #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
   LOGN(F("hsc\thum\t"));
   #endif

   #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
   LOGN(F("tist\ttmin\ttavg\ttmax\ttsgm\t"));
   #endif

   #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
   LOGN(F("hist\thmin\thavg\thmax\thsgm\t"));
   #endif

   #if (USE_MODULE_THR && (USE_SENSOR_TBR || USE_SENSOR_TBS))
   LOGN(F("rtc"));
   #elif (USE_MODULE_RAIN && (USE_SENSOR_TBR || USE_SENSOR_TBS))
   LOGN(F("rtc %d "), rain_tips);
   #endif

   LOGN(F("\r\n"));

   Log.setPrefix(logPrefix);
   Log.setSuffix(logSuffix);
}

/*!
\fn ISR(TIMER1_OVF_vect)
\brief Timer1 overflow interrupts routine.
\return void.
*/
ISR(TIMER1_OVF_vect) {
   //! Pre-load timer counter register
   TCNT1 = TIMER1_TCNT1_VALUE;

   //! increment timer_counter_ms by TIMER1_INTERRUPT_TIME_MS
   timer_counter_ms += TIMER1_INTERRUPT_TIME_MS;

   i2c_error++;

   //! check if SENSORS_SAMPLE_TIME_MS ms have passed since ist time. if true and if is in continuous mode and continuous start command It has been received, activate Sensor reading task
   #if (USE_MODULE_THR || USE_MODULE_TH)
   if (executeTimerTaskEach(timer_counter_ms, SENSORS_SAMPLE_TIME_MS, TIMER1_INTERRUPT_TIME_MS) && configuration.is_continuous) {
      if (!is_event_sensors_reading) {
         noInterrupts();
         is_event_sensors_reading = true;
         ready_tasks_count++;
         interrupts();
      }
   }
   #endif

   //! reset timer_counter_ms if it has become larger than TIMER_COUNTER_VALUE_MAX_MS
   if (timer_counter_ms >= TIMER_COUNTER_VALUE_MAX_MS) {
      timer_counter_ms = 0;
   }
}

#if (USE_MODULE_RAIN || USE_MODULE_THR)
void tipping_bucket_interrupt_handler() {
   // reading TIPPING_BUCKET_PIN value to be sure the interrupt has occurred
   if (digitalRead(TIPPING_BUCKET_PIN) == LOW) {
      rain_tips_event_occurred_time_ms = millis();
      detachInterrupt(digitalPinToInterrupt(TIPPING_BUCKET_PIN));
      // enable Tipping bucket task
      if (!is_event_tipping_bucket) {
         is_event_tipping_bucket = true;
         ready_tasks_count++;
      }
   }
}
#endif

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

         if (i2c_rx_data[0] == I2C_THR_ADDRESS_ADDRESS && rx_data_length == I2C_THR_ADDRESS_LENGTH) {
            is_i2c_data_ok = true;
         }
         else if (i2c_rx_data[0] == I2C_THR_ONESHOT_ADDRESS && rx_data_length == I2C_THR_ONESHOT_LENGTH) {
            is_i2c_data_ok = true;
         }
         else if (i2c_rx_data[0] == I2C_THR_CONTINUOUS_ADDRESS && rx_data_length == I2C_THR_CONTINUOUS_LENGTH) {
            is_i2c_data_ok = true;
         }
         else if (i2c_rx_data[0] == I2C_THR_TEMPERATURE_ADDRESS_ADDRESS && rx_data_length == I2C_THR_TEMPERATURE_ADDRESS_LENGTH) {
            is_i2c_data_ok = true;
         }
         else if (i2c_rx_data[0] == I2C_THR_HUMIDITY_ADDRESS_ADDRESS && rx_data_length == I2C_THR_HUMIDITY_ADDRESS_LENGTH) {
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

#if (USE_MODULE_RAIN || USE_MODULE_THR)
void tipping_bucket_task () {
   static tipping_bucket_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;

   switch (tipping_bucket_state) {
      case TIPPING_BUCKET_INIT:
         // waiting half tipping_bucket_time_ms in order to re-read pin in center of impulse
         start_time_ms = rain_tips_event_occurred_time_ms;
         delay_ms = configuration.tipping_bucket_time_ms / 2;

         state_after_wait = TIPPING_BUCKET_READ;
         tipping_bucket_state = TIPPING_BUCKET_WAIT_STATE;
      break;

      case TIPPING_BUCKET_READ:
         // if (configuration.is_oneshot && is_oneshot && is_start) {
         // re-read pin status to filter spikes. Pin must be LOW in center of impulse after HIGH-to-LOW transition
         if (digitalRead(TIPPING_BUCKET_PIN) == LOW)  {
            delay_ms = configuration.tipping_bucket_time_ms * 2;
            state_after_wait = TIPPING_BUCKET_CHECK;
            tipping_bucket_state = TIPPING_BUCKET_WAIT_STATE;
         }
         else {
            LOGN(F("Skip spike"));
            tipping_bucket_state = TIPPING_BUCKET_END;
         }
         // }
         // else {
         //    LOGN(F("Rain tips!"));
         // }
      break;

      case TIPPING_BUCKET_CHECK:
         // re-read pin status to filter spikes. Pin must be HIGH at the end of impulse after LOW-to-HIGH transition
         if (digitalRead(TIPPING_BUCKET_PIN) == HIGH) {
            rain_tips++;
            LOGN(F("Rain tips count: %d"), rain_tips);
         }
         else {
            LOGE(F("wrong timing or stalled tipping bucket"));
         }

         tipping_bucket_state = TIPPING_BUCKET_END;
      break;

      case TIPPING_BUCKET_END:
         attachInterrupt(digitalPinToInterrupt(TIPPING_BUCKET_PIN), tipping_bucket_interrupt_handler, LOW);
         noInterrupts();
         rain_tips_event_occurred_time_ms = -configuration.tipping_bucket_time_ms;
         is_event_tipping_bucket = false;
         ready_tasks_count--;
         interrupts();
         tipping_bucket_state = TIPPING_BUCKET_INIT;
      break;

      case TIPPING_BUCKET_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            tipping_bucket_state = state_after_wait;
         }
      break;
   }
}
#endif

#if (USE_MODULE_THR || USE_MODULE_TH)
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
         for (i = 0; i < sensors_count; i++) {
            sensors[i]->resetPrepared();
         }

         //! reset default value
         for (i = 0; i < VALUES_TO_READ_FROM_SENSOR_COUNT; i++) {
            values_readed_from_sensor[i] = UINT16_MAX;
         }

         i = 0;
         retry = 0;
         state_after_wait = SENSORS_READING_INIT;
         sensors_reading_state = SENSORS_READING_SETUP;
      break;

      case SENSORS_READING_SETUP:
         LOGV(F("Sensor error count: %d"),sensors[i]->getErrorCount());

         if (sensors[i]->getErrorCount() > SENSORS_RETRY_COUNT_MAX){
            LOGE(F("Sensor i2c error > SENSORS_RETRY_COUNT_MAX"));
            sensors[i]->resetSetted();
         }

         if (!sensors[i]->isSetted()) {
            sensors[i]->setup();
         }

         if (sensors[i]->isSetted()) {
            sensors_reading_state = SENSORS_READING_PREPARE;
         }
         else {
            LOGE(F("Skip failed Sensor"));
            sensors_reading_state = SENSORS_READING_READ;
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
            addValue<sample_t, uint16_t, float>(&temperature_samples, SAMPLES_COUNT, values_readed_from_sensor[0]);
         }
         #endif

         #if (USE_SENSOR_HIH)
         if (strcmp(sensors[i]->getType(), SENSOR_TYPE_HIH) == 0) {
            addValue<sample_t, uint16_t, float>(&humidity_samples, SAMPLES_COUNT, values_readed_from_sensor[0]);
            addValue<sample_t, uint16_t, float>(&temperature_samples, SAMPLES_COUNT, values_readed_from_sensor[1]);
         }
         #endif

         #if (USE_SENSOR_HYT)
         if (strcmp(sensors[i]->getType(), SENSOR_TYPE_HYT) == 0) {
            addValue<sample_t, uint16_t, float>(&humidity_samples, SAMPLES_COUNT, values_readed_from_sensor[0]);
            addValue<sample_t, uint16_t, float>(&temperature_samples, SAMPLES_COUNT, values_readed_from_sensor[1]);
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
            samples_processing();

            #if (LOGT_LEVEL >= LOGT_LEVEL_INFO)
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
#endif

template<typename buffer_g, typename length_v, typename value_v> value_v bufferRead(buffer_g *buffer, length_v length) {
   value_v value = *buffer->read_ptr;

   if (buffer->read_ptr == buffer->value+length-1) {
      buffer->read_ptr = buffer->value;
   }
   else buffer->read_ptr++;

   return value;
}

template<typename buffer_g, typename length_v, typename value_v> value_v bufferReadBack(buffer_g *buffer, length_v length) {
   value_v value = *buffer->read_ptr;

   if (buffer->read_ptr == buffer->value) {
      buffer->read_ptr = buffer->value+length-1;
   }
   else buffer->read_ptr--;

   return value;
}

template<typename buffer_g, typename value_v> void bufferWrite(buffer_g *buffer, value_v value) {
   *buffer->write_ptr = value;
}

template<typename buffer_g> void bufferPtrReset(buffer_g *buffer) {
   buffer->read_ptr = buffer->value;
}

template<typename buffer_g, typename length_v> void bufferPtrResetBack(buffer_g *buffer, length_v length) {
   if (buffer->write_ptr == buffer->value) {
      buffer->read_ptr = buffer->value+length-1;
   }
   else buffer->read_ptr = buffer->write_ptr-1;
}

template<typename buffer_g, typename length_v> void incrementBuffer(buffer_g *buffer, length_v length) {
   if (buffer->count < length) {
      buffer->count++;
   }

   if (buffer->write_ptr+1 < buffer->value + length) {
      buffer->write_ptr++;
   } else buffer->write_ptr = buffer->value;
}

template<typename buffer_g, typename length_v, typename value_v> void bufferReset(buffer_g *buffer, length_v length) {
   memset(buffer->value, UINT8_MAX, length * sizeof(value_v));
   buffer->count = 0;
   buffer->read_ptr = buffer->value;
   buffer->write_ptr = buffer->value;
}

template<typename buffer_g, typename length_v, typename value_v> void addValue(buffer_g *buffer, length_v length, value_v value) {
   *buffer->write_ptr = (value_v) value;
   incrementBuffer<buffer_g, length_v>(buffer, length);
}

void make_report () {
   #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
   uint16_t valid_count_humidity = 0;
   uint16_t error_count_humidity = 0;
   #endif

   #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
   uint16_t valid_count_temperature = 0;
   uint16_t error_count_temperature = 0;
   #endif

   #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
   uint16_t valid_count_humidity_o = 0;
   uint16_t error_count_humidity_o = 0;
   #endif

   #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
   uint16_t valid_count_temperature_o = 0;
   uint16_t error_count_temperature_o = 0;
   #endif

   #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
   float avg_temperature = 0;
   float sum1_temperature = 0;
   float sum2_temperature = 0;
   #endif

   #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
   float avg_humidity = 0;
   float sum1_humidity = 0;
   float sum2_humidity = 0;
   #endif

   #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
   float ist_temperature_o = (float) (UINT16_MAX);
   float avg_temperature_o = (float) (UINT16_MAX);
   float min_temperature_o = (float) (UINT16_MAX);
   float max_temperature_o = (float) (-UINT16_MAX);
   #endif

   #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
   float ist_humidity_o = (float) (UINT16_MAX);
   float avg_humidity_o = (float) (UINT16_MAX);
   float min_humidity_o = (float) (UINT16_MAX);
   float max_humidity_o = (float) (-UINT16_MAX);
   #endif

   #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
   bufferPtrResetBack<sample_t, uint16_t>(&temperature_samples, SAMPLES_COUNT);
   #endif

   #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
   bufferPtrResetBack<sample_t, uint16_t>(&humidity_samples, SAMPLES_COUNT);
   #endif

   uint16_t sample_count = RMAP_REPORT_SAMPLES_COUNT;

   #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
   if (temperature_samples.count < RMAP_REPORT_SAMPLES_COUNT) {
      sample_count = temperature_samples.count;
   }
   #endif

   #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
   if (humidity_samples.count < RMAP_REPORT_SAMPLES_COUNT) {
      sample_count = humidity_samples.count;
   }
   #endif

   #if (USE_SENSOR_DRIVER_COUNT == 0)
   sample_count = 0;
   #endif

   Log.setSuffix(logSuffixClear);
   LOGN(F(""));
   Log.setPrefix(logPrefixClear);

   for (uint16_t i = 0; i < sample_count; i++) {
      #if (USE_SENSOR_ADT || USE_SENSOR_HIH || USE_SENSOR_HYT)
      bool is_new_observation = (((i+1) % OBSERVATION_SAMPLES_COUNT_MAX) == 0);
      #endif

      #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
      float temperature = bufferReadBack<sample_t, uint16_t, float>(&temperature_samples, SAMPLES_COUNT);
      #endif

      #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
      float humidity = bufferReadBack<sample_t, uint16_t, float>(&humidity_samples, SAMPLES_COUNT);
      #endif

      if (i == 0) {
         #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
         LOGN(F("%d\t%D\t"), temperature_samples.count, temperature);
         #endif

         #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
         LOGN(F("%d\t%D\t"), humidity_samples.count, humidity);
         #endif
      }

      #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
      if (ISVALID(temperature)) {
         valid_count_temperature++;
         avg_temperature += (float) ((temperature - avg_temperature) / valid_count_temperature);
      }
      else {
         error_count_temperature++;
      }
      #endif

      #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
      if (ISVALID(humidity)) {
         valid_count_humidity++;
         avg_humidity += (float) ((humidity - avg_humidity) / valid_count_humidity);
      }
      else {
         error_count_humidity++;
      }
      #endif

      #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
      if (is_new_observation) {
         if (valid_count_temperature && (error_count_temperature <= OBSERVATION_SAMPLE_ERROR_MAX)) {
            valid_count_temperature_o++;
            avg_temperature_o += (float) ((avg_temperature - avg_temperature_o) / valid_count_temperature_o);

            if (i <= OBSERVATION_SAMPLES_COUNT_MAX) {
               ist_temperature_o = avg_temperature;
            }

            if (avg_temperature <= min_temperature_o) {
               min_temperature_o = avg_temperature;
            }

            if (avg_temperature >= max_temperature_o) {
               max_temperature_o = avg_temperature;
            }

            sum1_temperature += avg_temperature;
            sum2_temperature += avg_temperature * avg_temperature;
         }
         else {
            error_count_temperature_o++;
         }

         avg_temperature = 0;
         valid_count_temperature = 0;
         error_count_temperature = 0;
      }
      #endif

      #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
      if (is_new_observation) {
         if (valid_count_humidity && (error_count_humidity <= OBSERVATION_SAMPLE_ERROR_MAX)) {
            valid_count_humidity_o++;
            avg_humidity_o += (float) ((avg_humidity - avg_humidity_o) / valid_count_humidity_o);

            if (i <= OBSERVATION_SAMPLES_COUNT_MAX) {
               ist_humidity_o = avg_humidity;
            }

            if (avg_humidity <= min_humidity_o) {
               min_humidity_o = avg_humidity;
            }

            if (avg_humidity >= max_humidity_o) {
               max_humidity_o = avg_humidity;
            }

            sum1_humidity += avg_humidity;
            sum2_humidity += avg_humidity * avg_humidity;
         }
         else {
            error_count_humidity_o++;
         }

         avg_humidity = 0;
         valid_count_humidity = 0;
         error_count_humidity = 0;
      }
      #endif
   }

   #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
   bufferPtrResetBack<sample_t, uint16_t>(&temperature_samples, SAMPLES_COUNT);
   float temperature = bufferReadBack<sample_t, uint16_t, float>(&temperature_samples, SAMPLES_COUNT);

   if (ISVALID(temperature)) {
      readable_data_write_ptr->thr.sample_temperature = temperature;
   }
   #endif

   #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
   bufferPtrResetBack<sample_t, uint16_t>(&humidity_samples, SAMPLES_COUNT);
   float humidity = bufferReadBack<sample_t, uint16_t, float>(&humidity_samples, SAMPLES_COUNT);

   if (ISVALID(temperature)) {
      readable_data_write_ptr->thr.sample_humidity = humidity;
   }
   #endif

   #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
   if ((valid_count_temperature_o >= RMAP_REPORT_VALID_MIN) && (error_count_temperature_o <= RMAP_REPORT_ERROR_MAX)) {
      if (!ISVALID(ist_temperature_o)) {
         if (ISVALID(min_temperature_o)) {
            ist_temperature_o = min_temperature_o;
         }
         else if (ISVALID(max_temperature_o)) {
            ist_temperature_o = max_temperature_o;
         }
      }

      readable_data_write_ptr->thr.ist_temperature = ist_temperature_o;
      readable_data_write_ptr->thr.min_temperature = min_temperature_o;
      readable_data_write_ptr->thr.avg_temperature = avg_temperature_o;
      readable_data_write_ptr->thr.max_temperature = max_temperature_o;
      readable_data_write_ptr->thr.sigma_temperature = sqrt((sum2_temperature - (sum1_temperature * sum1_temperature) / (float) (valid_count_temperature_o)) / (float) (valid_count_temperature_o));
   }
   #endif

   #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
   if ((valid_count_humidity_o >= RMAP_REPORT_VALID_MIN) && (error_count_humidity_o <= RMAP_REPORT_ERROR_MAX)) {
      if (!ISVALID(ist_humidity_o)) {
         if (ISVALID(min_humidity_o)) {
            ist_humidity_o = min_humidity_o;
         }
         else if (ISVALID(max_humidity_o)) {
            ist_humidity_o = max_humidity_o;
         }
      }

      readable_data_write_ptr->thr.ist_humidity = ist_humidity_o;
      readable_data_write_ptr->thr.min_humidity = min_humidity_o;
      readable_data_write_ptr->thr.avg_humidity = avg_humidity_o;
      readable_data_write_ptr->thr.max_humidity = max_humidity_o;
      readable_data_write_ptr->thr.sigma_humidity = sqrt((sum2_humidity - (sum1_humidity * sum1_humidity) / (float) (valid_count_humidity_o)) / (float) (valid_count_humidity_o));
   }
   #endif

   copy_oneshot_data();

   // TEST
   readable_data_write_ptr->thr.ist_temperature = 100.0;
   readable_data_write_ptr->thr.min_temperature = 120.0;
   readable_data_write_ptr->thr.avg_temperature = 140.0;
   readable_data_write_ptr->thr.max_temperature = 160.0;
   readable_data_write_ptr->thr.sigma_temperature = 180.0;
   readable_data_write_ptr->thr.ist_humidity = 200.0;
   readable_data_write_ptr->thr.min_humidity = 210.0;
   readable_data_write_ptr->thr.avg_humidity = 230.0;
   readable_data_write_ptr->thr.max_humidity = 250.0;
   readable_data_write_ptr->thr.sigma_humidity = 270.0;

   #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
   LOGN(F("%D\t%D\t%D\t%D\t%D\t"), readable_data_write_ptr->thr.ist_temperature, readable_data_write_ptr->thr.min_temperature, readable_data_write_ptr->thr.avg_temperature, readable_data_write_ptr->thr.max_temperature, readable_data_write_ptr->thr.sigma_temperature);
   #endif

   #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
   LOGN(F("%D\t%D\t%D\t%D\t%D\t"), readable_data_write_ptr->thr.ist_humidity, readable_data_write_ptr->thr.min_humidity, readable_data_write_ptr->thr.avg_humidity, readable_data_write_ptr->thr.max_humidity, readable_data_write_ptr->thr.sigma_humidity);
   #endif

   #if (USE_SENSOR_TBR)
   LOGN(F("%D\t%D\t"), readable_data_write_ptr->thr.rain_tips, readable_data_write_ptr->thr.rain);
   #endif

   LOGN(F("\r\n"));

   Log.setPrefix(logPrefix);
   Log.setSuffix(logSuffix);
}

void copy_oneshot_data () {
   #if (USE_SENSOR_TBR)
   readable_data_write_ptr->thr.rain_tips = (float) rain_tips;
   readable_data_write_ptr->thr.rain = (float) (rain_tips * configuration.rain_for_tip);

   readable_data_write_ptr->thr.rain_tips = 12.0;
   readable_data_write_ptr->thr.rain = 24.0;
   #endif
}

void samples_processing() {
   reset_report_buffer();
   make_report();
}

void exchange_buffers() {
   readable_data_temp_ptr = readable_data_write_ptr;
   readable_data_write_ptr = readable_data_read_ptr;
   readable_data_read_ptr = readable_data_temp_ptr;
}

void reset_samples_buffer() {
   #if (USE_SENSOR_ADT || USE_SENSOR_HYT)
   bufferReset<sample_t, uint16_t, float>(&temperature_samples, SAMPLES_COUNT);
   #endif

   #if (USE_SENSOR_HIH || USE_SENSOR_HYT)
   bufferReset<sample_t, uint16_t, float>(&humidity_samples, SAMPLES_COUNT);
   #endif
}

void reset_buffer() {
   #if (USE_SENSOR_TBR)
   rain_tips = 0;
   #endif
}

void reset_report_buffer () {
   readable_data_write_ptr->thr.rain_tips = (float) UINT16_MAX;
   readable_data_write_ptr->thr.rain = (float) UINT16_MAX;
   readable_data_write_ptr->thr.sample_temperature = (float) UINT16_MAX;
   readable_data_write_ptr->thr.ist_temperature = (float) UINT16_MAX;
   readable_data_write_ptr->thr.avg_temperature = (float) UINT16_MAX;
   readable_data_write_ptr->thr.max_temperature = (float) UINT16_MAX;
   readable_data_write_ptr->thr.min_temperature = (float) UINT16_MAX;
   readable_data_write_ptr->thr.sigma_temperature = (float) UINT16_MAX;
   readable_data_write_ptr->thr.sample_humidity = (float) UINT16_MAX;
   readable_data_write_ptr->thr.ist_humidity = (float) UINT16_MAX;
   readable_data_write_ptr->thr.avg_humidity = (float) UINT16_MAX;
   readable_data_write_ptr->thr.max_humidity = (float) UINT16_MAX;
   readable_data_write_ptr->thr.min_humidity = (float) UINT16_MAX;
   readable_data_write_ptr->thr.sigma_humidity = (float) UINT16_MAX;
}

void command_task() {
   #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
   char buffer[30];
   #endif

   switch(i2c_rx_data[1]) {
      case I2C_THR_COMMAND_ONESHOT_START:
      #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
      strcpy(buffer, "ONESHOT START");
      #endif
      is_oneshot = true;
      is_continuous = false;
      is_start = true;
      is_stop = false;
      commands();
      break;

      case I2C_THR_COMMAND_ONESHOT_STOP:
      #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
      strcpy(buffer, "ONESHOT STOP");
      #endif
      is_oneshot = true;
      is_continuous = false;
      is_start = false;
      is_stop = true;
      commands();
      break;

      case I2C_THR_COMMAND_ONESHOT_START_STOP:
      #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
      strcpy(buffer, "ONESHOT START-STOP");
      #endif
      is_oneshot = true;
      is_continuous = false;
      is_start = true;
      is_stop = true;
      commands();
      break;

      case I2C_THR_COMMAND_CONTINUOUS_START:
      #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
      strcpy(buffer, "CONTINUOUS START");
      #endif
      is_oneshot = false;
      is_continuous = true;
      is_start = true;
      is_stop = false;
      commands();
      break;

      case I2C_THR_COMMAND_CONTINUOUS_STOP:
      #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
      strcpy(buffer, "CONTINUOUS STOP");
      #endif
      is_oneshot = false;
      is_continuous = true;
      is_start = false;
      is_stop = true;
      commands();
      break;

      case I2C_THR_COMMAND_CONTINUOUS_START_STOP:
      #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
      strcpy(buffer, "CONTINUOUS START-STOP");
      #endif
      is_oneshot = false;
      is_continuous = true;
      is_start = true;
      is_stop = true;
      commands();
      break;

      case I2C_THR_COMMAND_TEST_READ:
      #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
      strcpy(buffer, "TEST READ");
      #endif
      tests();
      break;

      case I2C_THR_COMMAND_SAVE:
      LOGT(F("Execute command [ SAVE ]"));
      save_configuration(CONFIGURATION_CURRENT);
      init_wire();
      break;
   }

   #if (LOG_LEVEL >= LOG_LEVEL_NOTICE)
   if (configuration.is_oneshot == is_oneshot || configuration.is_continuous == is_continuous) {
      LOGT(F("Execute [ %s ]"), buffer);
   }
   else if (configuration.is_oneshot == is_continuous || configuration.is_continuous == is_oneshot) {
      LOGT(F("Ignore [ %s ]"), buffer);
   }
   #endif

   noInterrupts();
   is_event_command_task = false;
   ready_tasks_count--;
   interrupts();
}

void commands() {
   noInterrupts();

   //! CONTINUOUS START
   if (configuration.is_continuous && is_continuous && is_start && !is_stop) {
      #if (USE_MODULE_THR || USE_MODULE_TH)
      reset_buffer();
      reset_samples_buffer();
      reset_report_buffer();
      #endif
   }
   //! CONTINUOUS STOP
   else if (configuration.is_continuous && is_continuous && !is_start && is_stop) {
      #if (USE_MODULE_THR || USE_MODULE_TH)
      copy_oneshot_data();
      exchange_buffers();
      reset_buffer();
      #endif
   }
   //! CONTINUOUS START-STOP
   else if (configuration.is_continuous && is_continuous && is_start && is_stop) {
      #if (USE_MODULE_THR || USE_MODULE_TH)
      copy_oneshot_data();
      exchange_buffers();
      reset_buffer();
      #endif
   }
   //! ONESHOT START
   else if (configuration.is_oneshot && is_oneshot && is_start && !is_stop) {
      #if (USE_MODULE_RAIN)
      reset_buffer();
      reset_samples_buffer();
      reset_report_buffer();
      #endif
   }
   //! ONESHOT STOP
   else if (configuration.is_oneshot && is_oneshot && !is_start && is_stop) {
      #if (USE_MODULE_RAIN)
      copy_oneshot_data();
      exchange_buffers();
      reset_buffer();
      #endif
   }
   //! ONESHOT START-STOP
   else if (configuration.is_oneshot && is_oneshot && is_start && is_stop) {
      #if (USE_MODULE_RAIN)
      copy_oneshot_data();
      exchange_buffers();
      reset_buffer();
      #endif
   }

   interrupts();
}

void tests() {
   noInterrupts();
   copy_oneshot_data();
   exchange_buffers();
   interrupts();
}
