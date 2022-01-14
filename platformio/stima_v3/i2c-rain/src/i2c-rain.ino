/**@file i2c-rain.ino */

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

#include "i2c-rain.h"

/*!
\fn void setup()
\brief Arduino setup function. Init watchdog, hardware, debug, buffer and load configuration stored in EEPROM.
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

   power_adc_disable();
   #if !(ENABLE_SDCARD_LOGGING)   
   power_spi_disable();
   #endif
   //power_timer0_disable();
   #if (USE_TIMER_1 == false)
   power_timer1_disable(); 
   #endif
   power_timer2_disable();
   
   init_system();
   wdt_reset();
}

/*!
\fn void loop()
\brief Arduino loop function. First, initialize tasks and sensors, then execute the tasks and activates the power down if no task is running.
\return void.
*/
void loop() {
   switch (state) {
      case INIT:
         init_tasks();
         init_sensors();
         state = TASKS_EXECUTION;
      break;

      #if (USE_POWER_DOWN)
      case ENTER_POWER_DOWN:
         #if (ENABLE_SDCARD_LOGGING)   
	 logFile.flush();
	 power_spi_disable();
	 #endif
	 Serial.flush();	
         // disable watchdog: the next awakening is given by an interrupt of rain and I do not know how long it will take place
         wdt_disable();

         // enter in power down mode only if DEBOUNCING_POWER_DOWN_TIME_MS milliseconds have passed since last time (awakened_event_occurred_time_ms)
         init_power_down(&awakened_event_occurred_time_ms, DEBOUNCING_POWER_DOWN_TIME_MS);

         // enable watchdog
         init_wdt(WDT_TIMER);
         #if (ENABLE_SDCARD_LOGGING)   
	 power_spi_enable();
	 #endif
         state = TASKS_EXECUTION;
      break;
      #endif

      case TASKS_EXECUTION:
        // I2C Bus Check
        if (i2c_error >= I2C_MAX_ERROR_COUNT) {
          LOGE(F("Restart I2C BUS"));
          init_wire();
          wdt_reset();
        }

        if (is_event_tipping_bucket) {
          tipping_bucket_task();
          wdt_reset();
        }

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
   wdt_reset();
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

		power_timer0_disable();
      #if (USE_TIMER_1 == false)
      power_timer1_disable();
      #endif
		noInterrupts ();
		sleep_enable();

		interrupts ();

		sleep_cpu();
		sleep_disable();

		power_timer0_enable();
      #if (USE_TIMER_1 == false)
      power_timer1_enable();
      #endif
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
   memset((void *) &readable_data_read_ptr->rain, UINT8_MAX, sizeof(rain_t));
   rain.tips_count = UINT16_MAX;

   // copy readable_data_2 in readable_data_1
   memcpy((void *) readable_data_read_ptr, (const void*) readable_data_write_ptr, sizeof(readable_data_t));
}

void init_tasks() {
   noInterrupts();

   // no tasks ready
   ready_tasks_count = 0;

   is_event_command_task = false;
   is_event_tipping_bucket = false;

   tipping_bucket_state = TIPPING_BUCKET_INIT;

   // reset tipping bucket debounce value
   rain_tips_event_occurred_time_ms = -configuration.tipping_bucket_time_ms;
   interrupts();
}

void init_pins() {
   pinMode(CONFIGURATION_RESET_PIN, INPUT_PULLUP);
   pinMode(TIPPING_BUCKET_PIN, INPUT_PULLUP);
   attachInterrupt(digitalPinToInterrupt(TIPPING_BUCKET_PIN), tipping_bucket_interrupt_handler, LOW);
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
  SPI.begin();
}

void init_rtc() {
}

#if (USE_TIMER_1)
void init_timer1() {
}
#endif

void init_system() {
   #if (USE_POWER_DOWN)
   set_sleep_mode(SLEEP_MODE_PWR_DOWN);
   awakened_event_occurred_time_ms = millis();
   #endif

   // main loop state
   state = INIT;
}

void init_sensors () {
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
   LOGN(F("--> Tipping bucket time in milliseconds: %d"), configuration.tipping_bucket_time_ms);
   LOGN(F("--> How much mm of rain for one tip of tipping bucket rain gauge: %d"), configuration.rain_for_tip);
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
      configuration.tipping_bucket_time_ms=CONFIGURATION_DEFAULT_TIPPING_BUCKET_TIME_MS;
      configuration.rain_for_tip=CONFIGURATION_DEFAULT_RAIN_FOR_TIP;
   }
   else {
      LOGN(F("Save configuration... [ %s ]"), OK_STRING);
      configuration.i2c_address = writable_data_ptr->i2c_address;
      configuration.is_oneshot = writable_data_ptr->is_oneshot;
      configuration.is_continuous = writable_data_ptr->is_continuous;
      configuration.tipping_bucket_time_ms = writable_data_ptr->tipping_bucket_time_ms;
      configuration.rain_for_tip = writable_data_ptr->rain_for_tip;
   }

   // write configuration to eeprom
   ee_write(&configuration, CONFIGURATION_EEPROM_ADDRESS, sizeof(configuration));

   print_configuration();
}

void load_configuration() {
   // read configuration from eeprom
   ee_read(&configuration, CONFIGURATION_EEPROM_ADDRESS, sizeof(configuration));

   if (configuration.module_type != MODULE_TYPE || configuration.module_main_version != MODULE_MAIN_VERSION|| configuration.module_configuration_version != MODULE_CONFIGURATION_VERSION || digitalRead(CONFIGURATION_RESET_PIN) == LOW) {
      save_configuration(CONFIGURATION_DEFAULT);
   }
   else {
      LOGN(F("Load configuration... [ %s ]"), OK_STRING);
      print_configuration();
   }

   // set configuration value to writable register
   writable_data.i2c_address = configuration.i2c_address;
   writable_data.is_oneshot = configuration.is_oneshot;
   writable_data.is_continuous = configuration.is_continuous;
   writable_data.tipping_bucket_time_ms = configuration.tipping_bucket_time_ms;
   writable_data.rain_for_tip = configuration.rain_for_tip;
}

void tipping_bucket_interrupt_handler() {
   // reading TIPPING_BUCKET_PIN value to be sure the interrupt has occurred
   if (digitalRead(TIPPING_BUCKET_PIN) == LOW) {
      detachInterrupt(digitalPinToInterrupt(TIPPING_BUCKET_PIN));
      //noInterrupts();
      // enable Tipping bucket task
      if (!is_event_tipping_bucket) {
         is_event_tipping_bucket = true;
         ready_tasks_count++;
      }
      //interrupts();
   }
}

void i2c_request_interrupt_handler() {
   // write readable_data_length bytes of data stored in readable_data_read_ptr (base) + readable_data_address (offset) on i2c bus
   Wire.write((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length);
   Wire.write(crc8((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length));
}

void i2c_receive_interrupt_handler(int rx_data_length) {
  bool is_i2c_data_ok = false;

  readable_data_length = 0;

  // read rx_data_length bytes of data from i2c bus
  for (uint8_t i=0; i<rx_data_length; i++) {
    i2c_rx_data[i] = Wire.read();
  }

  if (rx_data_length < 2) {
    // no payload and CRC as for scan I2c bus
    readable_data_length = 0;
    //LOGN(F("No CRC: size %d"),rx_data_length);
  } else if (i2c_rx_data[rx_data_length - 1] == crc8((uint8_t *)(i2c_rx_data), rx_data_length - 1)) {
    //! check crc: ok    
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
      rx_data_length -= 2;

      if (i2c_rx_data[0] == I2C_RAIN_ADDRESS_ADDRESS && rx_data_length == I2C_RAIN_ADDRESS_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_RAIN_ONESHOT_ADDRESS && rx_data_length == I2C_RAIN_ONESHOT_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_RAIN_CONTINUOUS_ADDRESS && rx_data_length == I2C_RAIN_CONTINUOUS_LENGTH) {
        is_i2c_data_ok = true;
      }

      if (is_i2c_data_ok) {
        for (uint8_t i = 0; i < rx_data_length; i++) {
          // write rx_data_length bytes in writable_data_ptr (base) at (i2c_rx_data[i] - I2C_WRITE_REGISTER_START_ADDRESS) (position in buffer)
          ((uint8_t *)(writable_data_ptr))[i2c_rx_data[0] - I2C_WRITE_REGISTER_START_ADDRESS + i] = i2c_rx_data[i + 2];
        }
      }
    }
  } else {
    readable_data_length = 0;
    //LOGE(F("CRC error: size %d  CRC %d:%d"),rx_data_length,i2c_rx_data[rx_data_length - 1], crc8((uint8_t *)(i2c_rx_data), rx_data_length - 1));
    i2c_error++;
  }
}

void tipping_bucket_task () {
   static tipping_bucket_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;

   switch (tipping_bucket_state) {
      case TIPPING_BUCKET_INIT:
	start_time_ms=millis();
	delay_ms=configuration.tipping_bucket_time_ms/2;
	state_after_wait=TIPPING_BUCKET_READ;
	tipping_bucket_state = TIPPING_BUCKET_WAIT_STATE;
	
      break;

      case TIPPING_BUCKET_READ:
         // increment rain tips if oneshot mode is on and oneshot start command It has been received
         if (configuration.is_oneshot && is_oneshot && is_start) {
	   // re-read pin status to filter spikes 
	   if (digitalRead(TIPPING_BUCKET_PIN) == LOW)  {
	     rain.tips_count++;
	     rain.rain=rain.tips_count * configuration.rain_for_tip;
	     LOGN(F("Rain tips count: %d"), rain.tips_count);
	   }else{
	     LOGN(F("Skip spike"));
	     tipping_bucket_state = TIPPING_BUCKET_END;
	     break;
	   }
	 }
	 else {
	   LOGN(F("Rain tips!"));
	 }

         //tipping_bucket_state = TIPPING_BUCKET_END;
	start_time_ms=millis();
	delay_ms=configuration.tipping_bucket_time_ms*2;
	state_after_wait=TIPPING_BUCKET_END;
	tipping_bucket_state = TIPPING_BUCKET_WAIT_STATE;

     break;

      
      case TIPPING_BUCKET_END:

	 if (digitalRead(TIPPING_BUCKET_PIN) == LOW)  {
	     LOGE(F("wrong timing or stalled tipping bucket"));

	     start_time_ms=millis();
	     delay_ms=configuration.tipping_bucket_time_ms;
	     state_after_wait=TIPPING_BUCKET_END;
	     tipping_bucket_state = TIPPING_BUCKET_WAIT_STATE;
	 }

	 noInterrupts();
         is_event_tipping_bucket = false;
         ready_tasks_count--;
         interrupts();
         tipping_bucket_state = TIPPING_BUCKET_INIT;

	 attachInterrupt(digitalPinToInterrupt(TIPPING_BUCKET_PIN), tipping_bucket_interrupt_handler, LOW);

      break;

      case TIPPING_BUCKET_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            tipping_bucket_state = state_after_wait;
         }
      break;
   }
}

void exchange_buffers() {
   readable_data_temp_ptr = readable_data_write_ptr;
   readable_data_write_ptr = readable_data_read_ptr;
   readable_data_read_ptr = readable_data_temp_ptr;
}

void reset_buffers() {
   memset((void *) &readable_data_write_ptr->rain, UINT8_MAX, sizeof(rain_t));
   rain.tips_count = 0;
   rain.rain = 0;
}

void command_task() {
   #ifndef DISABLE_LOGGING
   char buffer[30];
   #endif

   switch(i2c_rx_data[1]) {
      case I2C_RAIN_COMMAND_ONESHOT_START:
         #ifndef DISABLE_LOGGING
         strcpy(buffer, "ONESHOT START");
         #endif
         is_oneshot = true;
         is_continuous = false;
         is_start = true;
         is_stop = false;
         commands();
      break;

      case I2C_RAIN_COMMAND_ONESHOT_STOP:
         #ifndef DISABLE_LOGGING
         strcpy(buffer, "ONESHOT STOP");
         #endif
         is_oneshot = true;
         is_continuous = false;
         is_start = false;
         is_stop = true;
         commands();
      break;

      case I2C_RAIN_COMMAND_ONESHOT_START_STOP:
         #ifndef DISABLE_LOGGING
         strcpy(buffer, "ONESHOT START-STOP");
         #endif
         is_oneshot = true;
         is_continuous = false;
         is_start = true;
         is_stop = true;
         commands();
      break;

      case I2C_RAIN_COMMAND_TEST_READ:
         #ifndef DISABLE_LOGGING
         strcpy(buffer, "TEST READ");
         #endif
         is_test_read = true;
         tests();
      break;

      case I2C_RAIN_COMMAND_SAVE:
         #ifndef DISABLE_LOGGING
         strcpy(buffer, "SAVE");
	 #endif
         //is_oneshot = false;
         //is_continuous = false;
         //is_start = false;
         //is_stop = false;
         save_configuration(CONFIGURATION_CURRENT);
         init_wire();
      break;
      #ifndef DISABLE_LOGGING
      default:
         strcpy(buffer, "UNKNOWN");
      #endif
   }

   #ifndef DISABLE_LOGGING
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
  noInterrupts();
  readable_data_write_ptr->rain.tips_count = rain.tips_count;
  readable_data_write_ptr->rain.rain = rain.rain;
  exchange_buffers();
  is_test_read = false;
  interrupts();
}

void commands() {
   noInterrupts();

   if (configuration.is_oneshot && is_oneshot && is_stop) {
      readable_data_write_ptr->rain.tips_count = rain.tips_count;
      readable_data_write_ptr->rain.rain = rain.rain;
      exchange_buffers();
   }

   if (configuration.is_oneshot && is_oneshot && is_start) {
      reset_buffers();
   }
   else if (is_start) {
      reset_buffers();
      exchange_buffers();
   }

   interrupts();
   LOGN(F("Total tips : %d"), readable_data_read_ptr->rain.tips_count);
   LOGN(F("Total rain : %d"), readable_data_read_ptr->rain.rain);
}
