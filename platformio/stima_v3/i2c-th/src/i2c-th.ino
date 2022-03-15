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

	noInterrupts();
        if (ready_tasks_count == 0) {
          wdt_reset();
          state = END;
        }
	interrupts();	
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

   //! copy readable_data_write in readable_data_read
   copy_buffers();

   reset_samples_buffer();
   reset_data(readable_data_write_ptr);

   readable_data_address=0xFF;
   readable_data_length=0;
}

void init_tasks() {

   //! no tasks ready
   ready_tasks_count = 0;

   is_event_command_task = false;
   is_event_sensors_reading = false;

   sensors_reading_state = SENSORS_READING_INIT;

   lastcommand=I2C_TH_COMMAND_NONE;
   is_start = false;
   is_stop = false;
   is_test_read = false;
   transaction_time = 0;
   inside_transaction = false;
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
   TCCR1B = (1<<CS10) | (1<<CS12);   //!< 1:1024 prescaler
   TCNT1 = TIMER1_TCNT1_VALUE;   //!< Pre-load timer counter register
   TIFR1 |= (1 << TOV1);         //!< Clear interrupt overflow flag register
   timer_counter = 0;
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
   for (uint8_t i=0; i< 2; i++){
     if (strlen(configuration.sensors[i].type) == 3){
       LOGN(F("--> sensor[%d] type: %s"), i, configuration.sensors[i].type);
       LOGN(F("--> sensor[%d] i2c address: %X (%d)"), i, configuration.sensors[i].i2c_address, configuration.sensors[i].i2c_address);
     }
   }
}

void save_configuration(bool is_default) {
   if (is_default) {
      LOGN(F("Save default configuration... [ %s ]"), OK_STRING);
      configuration.module_type = MODULE_TYPE;
      configuration.module_main_version = MODULE_MAIN_VERSION;
      configuration.module_configuration_version = MODULE_CONFIGURATION_VERSION;
      configuration.i2c_address = CONFIGURATION_DEFAULT_I2C_ADDRESS;
      configuration.is_oneshot = CONFIGURATION_DEFAULT_IS_ONESHOT;

      strncpy (configuration.sensors[0].type,SENSOR_TYPE_HYT,4);
      configuration.sensors[0].i2c_address = HYT2X1_DEFAULT_ADDRESS;
      configuration.sensors[1].type[0]='\0';
      configuration.sensors[1].i2c_address = 255;	  
   }
   else {
      LOGN(F("Save configuration... [ %s ]"), OK_STRING);
      configuration.i2c_address = writable_data.i2c_address;
      configuration.is_oneshot = writable_data.is_oneshot;
      strncpy(configuration.sensors[0].type,writable_data.sensors[0].type,4);
      configuration.sensors[0].i2c_address=writable_data.sensors[0].i2c_address;
      strncpy(configuration.sensors[1].type,writable_data.sensors[1].type,4);
      configuration.sensors[1].i2c_address=writable_data.sensors[1].i2c_address;
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

   wdt_reset();
   
   if (!configuration.is_oneshot) {
     LOGN(F("--> samples every %d ms: "),SENSORS_SAMPLE_TIME_MS);
     LOGN(F("--> number of samples in %d minutes: %d"),OBSERVATIONS_MINUTES, OBSERVATION_SAMPLES_COUNT);
   }

   wdt_reset();
   
   writable_data.i2c_address = configuration.i2c_address;
   writable_data.is_oneshot = configuration.is_oneshot;

   strncpy(writable_data.sensors[0].type, configuration.sensors[0].type,4);
   writable_data.sensors[0].i2c_address=configuration.sensors[0].i2c_address;
   strncpy(writable_data.sensors[1].type, configuration.sensors[1].type,4);
   writable_data.sensors[1].i2c_address=configuration.sensors[1].i2c_address;
}

void init_sensors () {
  sensors_count = 0;

  LOGN(F("Sensors..."));
  
  for (uint8_t i=0; i < 2; i++){
    if (strlen(configuration.sensors[i].type) == 3){
      SensorDriver::createAndSetup(SENSOR_DRIVER_I2C, configuration.sensors[i].type, configuration.sensors[i].i2c_address, 1, sensors, &sensors_count);
      LOGN(F("--> %d: %s-%s: %s\t [ %s ]"), sensors_count, SENSOR_DRIVER_I2C, configuration.sensors[i].type, "", sensors[sensors_count-1]->isSetted() ? OK_STRING : FAIL_STRING);
    }
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
   if (executeTimerTaskEach(timer_counter, SENSORS_SAMPLE_TIME_MS, TIMER1_INTERRUPT_TIME_MS) && !configuration.is_oneshot && is_start) {
      if (!is_event_sensors_reading) {
         is_event_sensors_reading = true;
         ready_tasks_count++;
      }
   }

   //! reset timer_counter if it has become larger than TIMER1_VALUE_MAX_MS
   if (timer_counter >= TIMER1_VALUE_MAX_MS) {
      timer_counter = 0;
   }

   if (inside_transaction) {
     //! increment transaction_time by TIMER1_INTERRUPT_TIME_MS
     transaction_time += TIMER1_INTERRUPT_TIME_MS;
     
     if (transaction_time >= TRANSACTION_TIMEOUT_MS) {
       transaction_time = 0;
       inside_transaction = false;
     }
   }
}

void i2c_request_interrupt_handler() {

  /*
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
  */

  if (readable_data_length) {
    //! write readable_data_length bytes of data stored in readable_data_read_ptr (base) + readable_data_address (offset) on i2c bus
    Wire.write((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length);
    Wire.write(crc8((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length));
    // attention: logging inside ISR !
    //LOGV("request_interrupt_handler: %d-%d crc:%d",readable_data_address,readable_data_length,crc8((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length));
  }

  readable_data_address=0xFF;
  readable_data_length=0;
  inside_transaction = false;
 
}

void i2c_receive_interrupt_handler(int rx_data_length) {
  bool is_i2c_data_ok = false;

  // read rx_data_length bytes of data from i2c bus
  for (uint8_t i = 0; i < rx_data_length; i++) {
    i2c_rx_data[i] = Wire.read();
  }

  if (rx_data_length < 2) {
    // no payload and CRC as for scan I2c bus
    // attention: logging inside ISR !
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
      // attention: logging inside ISR !
      //LOGV(F("set readable_data: %d-%d"),readable_data_address,readable_data_length);
    }
    // it is a command?
    else if (rx_data_length == 2 && is_command(i2c_rx_data[0])) {
      //noInterrupts();
      // enable Command task
      if (!is_event_command_task) {
	reset_data(readable_data_read_ptr);    // make shure read old data wil be impossible
	lastcommand=i2c_rx_data[1];    // record command to be executed
        is_event_command_task = true;  // activate command task
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
      else if (i2c_rx_data[0] == I2C_TH_SENSOR1_TYPE_ADDRESS && rx_data_length == I2C_TH_SENSOR1_TYPE_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_TH_SENSOR1_I2C_ADDRESS_ADDRESS && rx_data_length == I2C_TH_SENSOR1_I2C_ADDRESS_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_TH_SENSOR2_TYPE_ADDRESS && rx_data_length == I2C_TH_SENSOR2_TYPE_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_TH_SENSOR2_I2C_ADDRESS_ADDRESS && rx_data_length == I2C_TH_SENSOR2_I2C_ADDRESS_LENGTH) {
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
    // attention: logging inside ISR !
    //LOGE(F("CRC error: size %d  CRC %d:%d"),rx_data_length,i2c_rx_data[rx_data_length - 1], crc8((uint8_t *)(i2c_rx_data), rx_data_length - 1));
    i2c_error++;
  }
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
         sensors_reading_state = SENSORS_READING_SETUP_CHECK;
      break;

   case SENSORS_READING_SETUP_CHECK:

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
	  addValue<sample_t, uint16_t, int32_t>(&temperature_samples, SAMPLES_COUNT_MAX, INT32_MAX);
	  addValue<sample_t, uint16_t, int32_t>(&humidity_samples, SAMPLES_COUNT_MAX, INT32_MAX);
	  
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
	  addValue<sample_t, uint16_t, int32_t>(&temperature_samples, SAMPLES_COUNT_MAX, INT32_MAX);
	  addValue<sample_t, uint16_t, int32_t>(&humidity_samples, SAMPLES_COUNT_MAX, INT32_MAX);

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
	{
	  StaticJsonDocument<JSON_BUFFER_LENGTH*2> doc;
	  DeserializationError error = deserializeJson(doc,json_sensors_data);
	  if (error) {
	    LOGE(F("deserializeJson() failed with code %s"),error.f_str());
            addValue<sample_t, uint16_t, int32_t>(&temperature_samples, SAMPLES_COUNT_MAX, INT32_MAX);
            addValue<sample_t, uint16_t, int32_t>(&humidity_samples, SAMPLES_COUNT_MAX, INT32_MAX);
	  }else{
	    unsigned long int value = doc["B12101"] | INT32_MAX;
            addValue<sample_t, uint16_t, int32_t>(&temperature_samples, SAMPLES_COUNT_MAX, value);
	    LOGN(F("Temperature sample: %d"), value);
	    
	    value = doc["B13003"] | INT32_MAX;
            addValue<sample_t, uint16_t, int32_t>(&humidity_samples, SAMPLES_COUNT_MAX, value);
	    LOGN(F("Humidity sample: %d"), value);
	  }
	}
        sensors_reading_state = SENSORS_READING_NEXT;
      break;

      case SENSORS_READING_NEXT:
         //! go to next sensor
         if ((++i) < sensors_count) {
            retry_prepare = 0;
            retry_get = 0;
            sensors_reading_state = SENSORS_READING_SETUP_CHECK;
         }
         //! end (there are no other sensors to read)
         else {
            //! if it is in continuous mode, do samples processing
            if (!configuration.is_oneshot) {
              samples_processing();
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

// legge dato puntato e incrementa puntatore
template<typename buffer_g, typename length_v, typename value_v> value_v bufferRead(buffer_g *buffer, length_v length) {
   value_v value = *buffer->read_ptr;

   if (buffer->read_ptr == buffer->values+length-1) {
      buffer->read_ptr = buffer->values;
   }
   else buffer->read_ptr++;

   return value;
}

// legge dato puntato e decrementa puntatore
template<typename buffer_g, typename length_v, typename value_v> value_v bufferReadBack(buffer_g *buffer, length_v length) {
   value_v value = *buffer->read_ptr;

   if (buffer->read_ptr == buffer->values) {
      buffer->read_ptr = buffer->values+length-1;
   }
   else buffer->read_ptr--;

   return value;
}

// setta il puntatore di lettura sul dato ultimo scritto
template<typename buffer_g, typename length_v> void bufferPtrResetBack(buffer_g *buffer, length_v length) {
   if (buffer->write_ptr == buffer->values) {
      buffer->read_ptr = buffer->values+length-1;
   }
   else buffer->read_ptr = buffer->write_ptr-1;
}


// reset of buffer (no data) setting all data to missing
template<typename buffer_g, typename length_v, typename value_v> void bufferReset(buffer_g *buffer, length_v length) {
   memset(buffer->values, UINT8_MAX, length * sizeof(value_v));
   buffer->count = 0;
   buffer->read_ptr = buffer->values;
   buffer->write_ptr = buffer->values;
}


// used by addValue
template<typename buffer_g, typename length_v> void incrementBuffer(buffer_g *buffer, length_v length) {
   if (buffer->count < length) {
      buffer->count++;
   }

   if (buffer->write_ptr+1 < buffer->values + length) {
      buffer->write_ptr++;
   } else buffer->write_ptr = buffer->values;
}

// add a value at the end of the circular buffer
template<typename buffer_g, typename length_v, typename value_v> void addValue(buffer_g *buffer, length_v length, value_v value) {
   *buffer->write_ptr = (value_v) value;
   incrementBuffer<buffer_g, length_v>(buffer, length);
}

void make_report (bool init=false) {
  
  static uint16_t valid_count_humidity_o;
  static uint16_t error_count_humidity_o;
  
  static uint16_t valid_count_temperature_o;
  static uint16_t error_count_temperature_o;
  
  static int32_t avg_temperature_o;
  static int32_t min_temperature_o;
  static int32_t max_temperature_o;
  
  static int32_t avg_humidity_o;
  static int32_t min_humidity_o;
  static int32_t max_humidity_o;

  static float sum1_temperature;
  static float sum2_temperature;

  static float sum1_humidity;
  static float sum2_humidity;

  static uint16_t temperature_sample_for_observation;
  static uint16_t humidity_sample_for_observation;

  
  if (init) {
    
    valid_count_humidity_o = 0;
    error_count_humidity_o = 0;
    
    valid_count_temperature_o = 0;
    error_count_temperature_o = 0;
    
    avg_temperature_o = 0;
    min_temperature_o = INT32_MAX;
    max_temperature_o = INT32_MIN;
    
    avg_humidity_o = 0;
    min_humidity_o = INT32_MAX;
    max_humidity_o = INT32_MIN;

    sum1_temperature=0;
    sum2_temperature=0;

    sum1_humidity=0;
    sum2_humidity=0;

    temperature_sample_for_observation=OBSERVATION_SAMPLES_COUNT-1;
    humidity_sample_for_observation=OBSERVATION_SAMPLES_COUNT-1;
    
    return;
  }
  
  // TEMPERATURE
  
  uint16_t valid_count_temperature=0;
  uint16_t error_count_temperature=0;
  int32_t avg_temperature=0;
  
  bufferPtrResetBack<sample_t, uint16_t>(&temperature_samples, SAMPLES_COUNT_MAX);
  int32_t temperature = bufferReadBack<sample_t, uint16_t, int32_t>(&temperature_samples, SAMPLES_COUNT_MAX);
  LOGN(F("sample temperature value: %l"), temperature);
  if (ISVALID_INT32(temperature)) {
    readable_data_write_ptr->temperature.sample = temperature;
  }else{
    readable_data_write_ptr->temperature.sample =  UINT16_MAX;    
  }
  bufferPtrResetBack<sample_t, uint16_t>(&temperature_samples, SAMPLES_COUNT_MAX);

  LOGN(F("temperature sample count %d:%d"),temperature_samples.count,temperature_sample_for_observation);

  if (temperature_samples.count == temperature_sample_for_observation) {
    temperature_sample_for_observation=OBSERVATION_SAMPLES_COUNT;
    for (uint16_t i = 0; i < temperature_samples.count; i++) {
    
      temperature = bufferReadBack<sample_t, uint16_t, int32_t>(&temperature_samples, SAMPLES_COUNT_MAX);
      LOGT(F("Temperature %d value: %l"), i, temperature);
      
      if (ISVALID_INT32(temperature)) {
	valid_count_temperature++;
	avg_temperature += round((float)(temperature - avg_temperature) / valid_count_temperature);
      } else {
	error_count_temperature++;
      }
    }

    bufferReset<sample_t, uint16_t, int32_t>(&temperature_samples, SAMPLES_COUNT_MAX);
    
    LOGN(F("temperature new observation with %d:%d errors"),error_count_temperature,OBSERVATION_SAMPLE_ERROR_MAX);
      
    if (valid_count_temperature && (error_count_temperature <= OBSERVATION_SAMPLE_ERROR_MAX)) {
      valid_count_temperature_o++;
      
      avg_temperature_o += round((float) (avg_temperature - avg_temperature_o) / valid_count_temperature_o);
      
      readable_data_write_ptr->temperature.med60 = avg_temperature;
      
      if (avg_temperature <= min_temperature_o) {
	min_temperature_o = avg_temperature;
      }
      
      if (avg_temperature >= max_temperature_o) {
	max_temperature_o = avg_temperature;
      }
      
      sum1_temperature += avg_temperature;
      sum2_temperature += avg_temperature * avg_temperature;
    } else {
      readable_data_write_ptr->temperature.med60 =  UINT16_MAX;
      error_count_temperature_o++;
    }     
  }
  
  LOGN(F("REPORT temperature valid_count:%d VALID_MIN:%d error_count:%d ERROR_MAX:%d"),valid_count_temperature_o, RMAP_REPORT_VALID_MIN, error_count_temperature_o, RMAP_REPORT_ERROR_MAX);
  
  if ((valid_count_temperature_o >= RMAP_REPORT_VALID_MIN) && (error_count_temperature_o <= RMAP_REPORT_ERROR_MAX)) {
    readable_data_write_ptr->temperature.min = min_temperature_o;
    readable_data_write_ptr->temperature.med = avg_temperature_o;
    readable_data_write_ptr->temperature.max = max_temperature_o;
    readable_data_write_ptr->temperature.sigma = sqrt((sum2_temperature - (sum1_temperature * sum1_temperature) / (float) (valid_count_temperature_o)) / (float) (valid_count_temperature_o));
  }else{
    readable_data_write_ptr->temperature.min   =  UINT16_MAX;
    readable_data_write_ptr->temperature.med   =  UINT16_MAX;
    readable_data_write_ptr->temperature.max   =  UINT16_MAX;
    readable_data_write_ptr->temperature.sigma =  UINT16_MAX;
  }

  LOGN(F("temperature sample:%d\tmed60:%d\tmin:%d\tmed:%d\tmax:%d\tsigma:%d"), readable_data_write_ptr->temperature.sample,readable_data_write_ptr->temperature.med60, readable_data_write_ptr->temperature.min, readable_data_write_ptr->temperature.med, readable_data_write_ptr->temperature.max, readable_data_write_ptr->temperature.sigma);
  
  
  // HUMIDITY
    
  uint16_t valid_count_humidity=0;
  uint16_t error_count_humidity=0;
  int32_t avg_humidity=0;
      
  bufferPtrResetBack<sample_t, uint16_t>(&humidity_samples, SAMPLES_COUNT_MAX);   
  int32_t humidity = bufferReadBack<sample_t, uint16_t, int32_t>(&humidity_samples, SAMPLES_COUNT_MAX);      
  LOGN(F("sample humidity value: %l"), humidity);   
  if (ISVALID_INT32(humidity)) {
    readable_data_write_ptr->humidity.sample = humidity;
  }else{
    readable_data_write_ptr->humidity.sample =  UINT16_MAX;    
  }
  
  bufferPtrResetBack<sample_t, uint16_t>(&humidity_samples, SAMPLES_COUNT_MAX);   

  LOGN(F("humidity sample count %d:%d"),humidity_samples.count,humidity_sample_for_observation);

  if (humidity_samples.count == humidity_sample_for_observation) {
    humidity_sample_for_observation=OBSERVATION_SAMPLES_COUNT;
    for (uint16_t i = 0; i < humidity_samples.count; i++) {
    
      humidity = bufferReadBack<sample_t, uint16_t, int32_t>(&humidity_samples, SAMPLES_COUNT_MAX);      
      LOGT(F("Humidity %d value: %l"), i, humidity);   
      
      if (ISVALID_INT32(humidity)) {
	valid_count_humidity++;
	avg_humidity += round((float) (humidity - avg_humidity) / valid_count_humidity);
      } else {
	error_count_humidity++;
      }
    }
    bufferReset<sample_t, uint16_t, int32_t>(&humidity_samples, SAMPLES_COUNT_MAX);
    
    LOGN(F("humidity new observation with %d:%d errors"),error_count_humidity,OBSERVATION_SAMPLE_ERROR_MAX);
    if (valid_count_humidity && (error_count_humidity <= OBSERVATION_SAMPLE_ERROR_MAX)) {
      valid_count_humidity_o++;

      avg_humidity_o += round((float) (avg_humidity - avg_humidity_o) / valid_count_humidity_o);
      
      readable_data_write_ptr->humidity.med60 = avg_humidity;
      
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
      readable_data_write_ptr->humidity.med60 =  UINT16_MAX;    
      error_count_humidity_o++;
    }
  }
  
  LOGN(F("REPORT humidity valid_count:%d VALID_MIN:%d error_count:%d ERROR_MAX:%d"),valid_count_humidity_o, RMAP_REPORT_VALID_MIN, error_count_humidity_o, RMAP_REPORT_ERROR_MAX);
  
  if ((valid_count_humidity_o >= RMAP_REPORT_VALID_MIN) && (error_count_humidity_o <= RMAP_REPORT_ERROR_MAX)) {  
    readable_data_write_ptr->humidity.min = min_humidity_o;
    readable_data_write_ptr->humidity.med = avg_humidity_o;
    readable_data_write_ptr->humidity.max = max_humidity_o;
    readable_data_write_ptr->humidity.sigma = sqrt((sum2_humidity - (sum1_humidity * sum1_humidity) / (float) (valid_count_humidity_o)) / (float) (valid_count_humidity_o));
  }else{
    readable_data_write_ptr->humidity.min  =  UINT16_MAX;
    readable_data_write_ptr->humidity.med  =  UINT16_MAX;
    readable_data_write_ptr->humidity.max  =  UINT16_MAX;
    readable_data_write_ptr->humidity.sigma = UINT16_MAX;
  }    
  
  LOGN(F("humidity   sample:%d\tmed60:%d\tmin:%d\tmed:%d\tmax:%d\tsigma:%d"), readable_data_write_ptr->humidity.sample,readable_data_write_ptr->humidity.med60, readable_data_write_ptr->humidity.min, readable_data_write_ptr->humidity.med, readable_data_write_ptr->humidity.max, readable_data_write_ptr->humidity.sigma);
  
}

void samples_processing() {
  LOGN(F("SAMPLE PROCESSING"));
  //reset_data(readable_data_write_ptr);
  make_report();
}

void exchange_buffers() {
   noInterrupts();
   readable_data_temp_ptr = readable_data_write_ptr;
   readable_data_write_ptr = readable_data_read_ptr;
   readable_data_read_ptr = readable_data_temp_ptr;
   interrupts();
}

void reset_samples_buffer() {
   bufferReset<sample_t, uint16_t, int32_t>(&temperature_samples, SAMPLES_COUNT_MAX);
   bufferReset<sample_t, uint16_t, int32_t>(&humidity_samples, SAMPLES_COUNT_MAX);
}

void reset_data(volatile readable_data_t *ptr) {
   ptr->temperature.sample =  UINT16_MAX;
   ptr->temperature.med60 =  UINT16_MAX;
   ptr->temperature.med =  UINT16_MAX;
   ptr->temperature.max =  UINT16_MAX;
   ptr->temperature.min =  UINT16_MAX;
   ptr->temperature.sigma =  UINT16_MAX;
   ptr->humidity.sample =  UINT16_MAX;
   ptr->humidity.med60 =  UINT16_MAX;
   ptr->humidity.med =  UINT16_MAX;
   ptr->humidity.max =  UINT16_MAX;
   ptr->humidity.min =  UINT16_MAX;
   ptr->humidity.sigma = UINT16_MAX;
}

void command_task() {

   switch(lastcommand) {
      case I2C_TH_COMMAND_ONESHOT_START:
	 LOGN(F("Execute [ ONESHOT START ]"));
         is_start = true;
         is_stop = false;
	 is_test_read = false;
         commands();
      break;

      case I2C_TH_COMMAND_ONESHOT_STOP:
	 LOGN(F("Execute [ ONESHOT STOP ]"));
         is_start = false;
         is_stop = true;
	 is_test_read = false;
         commands();
	 inside_transaction = true;
      break;

      case I2C_TH_COMMAND_ONESHOT_START_STOP:
	 LOGN(F("Execute [ ONESHOT START-STOP ]"));
         is_start = true;
         is_stop = true;
	 is_test_read = false;
         commands();
	 inside_transaction = true;
      break;

      case I2C_TH_COMMAND_CONTINUOUS_START:
	 LOGN(F("Execute [ CONTINUOUS START ]"));
         is_start = true;
         is_stop = false;
	 is_test_read = false;
         commands();
      break;

      case I2C_TH_COMMAND_CONTINUOUS_STOP:
	 LOGN(F("Execute [ CONTINUOUS STOP ]"));
         is_start = false;
         is_stop = true;
	 is_test_read = false;
	 commands();
	 inside_transaction = true;
      break;

      case I2C_TH_COMMAND_CONTINUOUS_START_STOP:
	LOGN(F("Execute [ CONTINUOUS START-STOP ]"));
        is_start = true;
        is_stop = true;
	is_test_read = false;
        commands();
	inside_transaction = true;
      break;

      case I2C_TH_COMMAND_TEST_READ:
	 LOGN(F("Execute [ TEST READ ]"));
         //is_start = true;
         is_stop = false;
	 is_test_read = true;
	 commands();
      break;

      case I2C_TH_COMMAND_SAVE:
        is_start = false;
        is_stop = false;
        LOGN(F("Execute command [ SAVE ]"));
        save_configuration(CONFIGURATION_CURRENT);
        init_wire();
      break;

      default:
	LOGN(F("Ignore unknow command"));
	
   }

   noInterrupts();
   is_event_command_task = false;
   ready_tasks_count--;
   lastcommand =I2C_TH_COMMAND_NONE;
   interrupts();
}

void copy_buffers() {
   //! copy readable_data_2 in readable_data_1
   noInterrupts();
   memcpy((void *) readable_data_read_ptr, (const void*) readable_data_write_ptr, sizeof(readable_data_t));
   interrupts();
}

void commands() {

  if (inside_transaction) return;
  
  //! CONTINUOUS TEST
  if (!configuration.is_oneshot && is_start && !is_stop && is_test_read) {
    copy_buffers();
    //exchange_buffers();
  }
  //! CONTINUOUS START
  else if (!configuration.is_oneshot && is_start && !is_stop && !is_test_read) {

    stop_timer();
    reset_samples_buffer();
    reset_data(readable_data_write_ptr);
    make_report(true);
    start_timer();
  }
  //! CONTINUOUS STOP
  else if (!configuration.is_oneshot && !is_start && is_stop) {
    copy_buffers();
    //exchange_buffers();
  }
  //! CONTINUOUS START-STOP
  else if (!configuration.is_oneshot && is_start && is_stop) {
    stop_timer();
    exchange_buffers();
    reset_samples_buffer();
    reset_data(readable_data_write_ptr);
    make_report(true);
    start_timer();
  }
  //! ONESHOT START
  else if (configuration.is_oneshot && is_start && !is_stop) {
    reset_samples_buffer();

    noInterrupts();
    if (!is_event_sensors_reading) {
      is_event_sensors_reading = true;
      ready_tasks_count++;
    }
    interrupts();
  }
  //! ONESHOT STOP
  else if (configuration.is_oneshot && !is_start && is_stop) {

    bufferPtrResetBack<sample_t, uint16_t>(&temperature_samples, SAMPLES_COUNT_MAX);
    int32_t temperature = bufferReadBack<sample_t, uint16_t, int32_t>(&temperature_samples, SAMPLES_COUNT_MAX);
    bufferPtrResetBack<sample_t, uint16_t>(&humidity_samples, SAMPLES_COUNT_MAX);
    int32_t humidity = bufferReadBack<sample_t, uint16_t, int32_t>(&humidity_samples, SAMPLES_COUNT_MAX);
   
    readable_data_write_ptr->temperature.sample = temperature;
    readable_data_write_ptr->humidity.sample = humidity;
    exchange_buffers();
  }
  //! ONESHOT START-STOP
  else if (configuration.is_oneshot && is_start && is_stop) {
    bufferPtrResetBack<sample_t, uint16_t>(&temperature_samples, SAMPLES_COUNT_MAX);
    int32_t temperature = bufferReadBack<sample_t, uint16_t, int32_t>(&temperature_samples, SAMPLES_COUNT_MAX);
    bufferPtrResetBack<sample_t, uint16_t>(&humidity_samples, SAMPLES_COUNT_MAX);
    int32_t humidity = bufferReadBack<sample_t, uint16_t, int32_t>(&humidity_samples, SAMPLES_COUNT_MAX);
   
    readable_data_write_ptr->temperature.sample = temperature;
    readable_data_write_ptr->humidity.sample = humidity;
    exchange_buffers();
    
    noInterrupts();
    if (!is_event_sensors_reading) {
      is_event_sensors_reading = true;
      ready_tasks_count++;
    }
    interrupts();
  }
}
