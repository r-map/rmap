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

#include <debug_config.h>

/*!
\def SERIAL_TRACE_LEVEL
\brief Serial debug level for this sketch.
*/
#define SERIAL_TRACE_LEVEL I2C_RAIN_SERIAL_TRACE_LEVEL

#include "i2c-rain.h"

/*!
\fn void setup()
\brief Arduino setup function. Init watchdog, hardware, debug, buffer and load configuration stored in EEPROM.
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
         // disable watchdog: the next awakening is given by an interrupt of rain and I do not know how long it will take place
         wdt_disable();

         // enter in power down mode only if DEBOUNCING_POWER_DOWN_TIME_MS milliseconds have passed since last time (awakened_event_occurred_time_ms)
         init_power_down(&awakened_event_occurred_time_ms, DEBOUNCING_POWER_DOWN_TIME_MS);

         // enable watchdog
         init_wdt(WDT_TIMER);

         state = TASKS_EXECUTION;
      break;
      #endif

      case TASKS_EXECUTION:
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


		// perche non usare void sleep_bod_disable()  ?
		// http://avr-libc.nongnu.org/user-manual/group__avr__sleep.html#gabf889562cc5ea768ee80cfc8a5bb0312
		// turn off brown-out
		MCUCR = bit (BODS) | bit (BODSE);
		MCUCR = bit (BODS);
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
   reset_buffers();

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
   rain_tips_event_occurred_time_ms = -DEBOUNCING_TIPPING_BUCKET_TIME_MS;
   interrupts();
}

void init_pins() {
   pinMode(CONFIGURATION_RESET_PIN, INPUT_PULLUP);
   pinMode(TIPPING_BUCKET_PIN, INPUT_PULLUP);
   attachInterrupt(digitalPinToInterrupt(TIPPING_BUCKET_PIN), tipping_bucket_interrupt_handler, FALLING);
}

void init_wire() {
   // clear the I2C bus first before calling Wire.begin()
   uint8_t i2c_bus_state = I2C_ClearBus();

   if (i2c_bus_state) {
      SERIAL_ERROR(F("I2C bus error: Could not clear!!!\r\n"));
      // wait for watchdog reboot
      while(1);
   }

   switch (i2c_bus_state) {
      case 1:
         SERIAL_ERROR(F("SCL clock line held low\r\n"));
      break;

      case 2:
         SERIAL_ERROR(F("SCL clock line held low by slave clock stretch\r\n"));
      break;

      case 3:
         SERIAL_ERROR(F("SDA data line held low\r\n"));
      break;
   }

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
   SERIAL_INFO(F("--> type: %s\r\n"), stima_name);
   SERIAL_INFO(F("--> version: %d\r\n"), configuration.module_version);
   SERIAL_INFO(F("--> i2c address: 0x%X (%d)\r\n"), configuration.i2c_address, configuration.i2c_address);
   SERIAL_INFO(F("--> oneshot: %s\r\n"), configuration.is_oneshot ? ON_STRING : OFF_STRING);
   SERIAL_INFO(F("--> continuous: %s\r\n"), configuration.is_continuous ? ON_STRING : OFF_STRING);
}

void save_configuration(bool is_default) {
   if (is_default) {
      SERIAL_INFO(F("Save default configuration... [ %s ]\r\n"), OK_STRING);
      configuration.module_type = MODULE_TYPE;
      configuration.module_version = MODULE_VERSION;
      configuration.i2c_address = CONFIGURATION_DEFAULT_I2C_ADDRESS;
      configuration.is_oneshot = CONFIGURATION_DEFAULT_IS_ONESHOT;
      configuration.is_continuous = CONFIGURATION_DEFAULT_IS_CONTINUOUS;
   }
   else {
      SERIAL_INFO(F("Save configuration... [ %s ]\r\n"), OK_STRING);
      configuration.i2c_address = writable_data.i2c_address;
      configuration.is_oneshot = writable_data.is_oneshot;
      configuration.is_continuous = writable_data.is_continuous;
   }

   // write configuration to eeprom
   ee_write(&configuration, CONFIGURATION_EEPROM_ADDRESS, sizeof(configuration));

   print_configuration();
}

void load_configuration() {
   // read configuration from eeprom
   ee_read(&configuration, CONFIGURATION_EEPROM_ADDRESS, sizeof(configuration));

   if (configuration.module_type != MODULE_TYPE || configuration.module_version != MODULE_VERSION || digitalRead(CONFIGURATION_RESET_PIN) == LOW) {
      save_configuration(CONFIGURATION_DEFAULT);
   }
   else {
      SERIAL_INFO(F("Load configuration... [ %s ]\r\n"), OK_STRING);
      print_configuration();
   }

   // set configuration value to writable register
   writable_data.i2c_address = configuration.i2c_address;
   writable_data.is_oneshot = configuration.is_oneshot;
}

void tipping_bucket_interrupt_handler() {
   // reading TIPPING_BUCKET_PIN value to be sure the interrupt has occurred
   if (digitalRead(TIPPING_BUCKET_PIN) == LOW) {
      noInterrupts();
      // enable Tipping bucket task
      if (!is_event_tipping_bucket) {
         is_event_tipping_bucket = true;
         ready_tasks_count++;
      }
      interrupts();
   }
}

void i2c_request_interrupt_handler() {
   // write readable_data_length bytes of data stored in readable_data_read_ptr (base) + readable_data_address (offset) on i2c bus
   Wire.write((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length);
}

void i2c_receive_interrupt_handler(int rx_data_length) {
   bool is_i2c_data_ok = false;

   // read rx_data_length bytes of data from i2c bus
   for (uint8_t i=0; i<rx_data_length; i++) {
      i2c_rx_data[i] = Wire.read();
   }

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
      if (i2c_rx_data[0] == I2C_RAIN_ADDRESS_ADDRESS && rx_data_length == (I2C_RAIN_ADDRESS_LENGTH+2)) {
         is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_RAIN_ONESHOT_ADDRESS && rx_data_length == (I2C_RAIN_ONESHOT_LENGTH+2)) {
         is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_RAIN_CONTINUOUS_ADDRESS && rx_data_length == (I2C_RAIN_CONTINUOUS_LENGTH+2)) {
         is_i2c_data_ok = true;
      }

      if (is_i2c_data_ok) {
         for (uint8_t i=2; i<rx_data_length; i++) {
            // write rx_data_length bytes in writable_data_ptr (base) at (i2c_rx_data[i] - I2C_WRITE_REGISTER_START_ADDRESS) (position in buffer)
            ((uint8_t *)writable_data_ptr)[i2c_rx_data[0] - I2C_WRITE_REGISTER_START_ADDRESS] = i2c_rx_data[i];
         }
      }
   }
}

void tipping_bucket_task () {
   static tipping_bucket_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;

   switch (tipping_bucket_state) {
      case TIPPING_BUCKET_INIT:
         // check if last tipping bucket event has happened for more than DEBOUNCING_TIPPING_BUCKET_TIME_MS ms
         if (millis() - rain_tips_event_occurred_time_ms > DEBOUNCING_TIPPING_BUCKET_TIME_MS) {
            rain_tips_event_occurred_time_ms = millis();
            tipping_bucket_state = TIPPING_BUCKET_READ;
         } else {
            tipping_bucket_state = TIPPING_BUCKET_END;
         }
      break;

      case TIPPING_BUCKET_READ:
         // increment rain tips if oneshot mode is on and oneshot start command It has been received
         if (configuration.is_oneshot && is_oneshot && is_start) {
            rain.tips_count++;
            SERIAL_INFO(F("Rain tips count: %u\r\n"), rain.tips_count);
         }
         else {
            SERIAL_INFO(F("Rain tips!\r\n"));
         }

         tipping_bucket_state = TIPPING_BUCKET_END;
      break;

      case TIPPING_BUCKET_END:
         noInterrupts();
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

void exchange_buffers() {
   readable_data_temp_ptr = readable_data_write_ptr;
   readable_data_write_ptr = readable_data_read_ptr;
   readable_data_read_ptr = readable_data_temp_ptr;
}

void reset_buffers() {
   memset((void *) &readable_data_write_ptr->rain, UINT8_MAX, sizeof(rain_t));
   rain.tips_count = 0;
}

void command_task() {
   #if (SERIAL_TRACE_LEVEL > SERIAL_TRACE_LEVEL_OFF)
   char buffer[30];
   #endif

   switch(i2c_rx_data[1]) {
      case I2C_RAIN_COMMAND_ONESHOT_START:
         #if (SERIAL_TRACE_LEVEL > SERIAL_TRACE_LEVEL_OFF)
         strcpy(buffer, "ONESHOT START");
         #endif
         is_oneshot = true;
         is_continuous = false;
         is_start = true;
         is_stop = false;
         commands();
      break;

      case I2C_RAIN_COMMAND_ONESHOT_STOP:
         #if (SERIAL_TRACE_LEVEL > SERIAL_TRACE_LEVEL_OFF)
         strcpy(buffer, "ONESHOT STOP");
         #endif
         is_oneshot = true;
         is_continuous = false;
         is_start = false;
         is_stop = true;
         commands();
      break;

      case I2C_RAIN_COMMAND_ONESHOT_START_STOP:
         #if (SERIAL_TRACE_LEVEL > SERIAL_TRACE_LEVEL_OFF)
         strcpy(buffer, "ONESHOT START-STOP");
         #endif
         is_oneshot = true;
         is_continuous = false;
         is_start = true;
         is_stop = true;
         commands();
      break;

      case I2C_RAIN_COMMAND_SAVE:
         SERIAL_DEBUG(F("Execute [ %s ]\r\n"), SAVE_STRING);
         is_oneshot = false;
         is_continuous = false;
         is_start = false;
         is_stop = false;
         save_configuration(CONFIGURATION_CURRENT);
         init_wire();
      break;
   }

   #if (SERIAL_TRACE_LEVEL > SERIAL_TRACE_LEVEL_OFF)
   if (configuration.is_oneshot == is_oneshot || configuration.is_continuous == is_continuous) {
      SERIAL_DEBUG(F("Execute [ %s ]\r\n"), buffer);
   }
   else if (configuration.is_oneshot == is_continuous || configuration.is_continuous == is_oneshot) {
      SERIAL_DEBUG(F("Ignore [ %s ]\r\n"), buffer);
   }
   #endif

   noInterrupts();
   is_event_command_task = false;
   ready_tasks_count--;
   interrupts();
}

void commands() {
   noInterrupts();

   if (configuration.is_oneshot && is_oneshot && is_stop) {
      readable_data_write_ptr->rain.tips_count = rain.tips_count;
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
   SERIAL_INFO(F("Total rain : %u\r\n"), readable_data_read_ptr->rain.tips_count);
}
