/**@file i2c-leaf.ino */

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

#include "i2c-leaf.h"

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
      power_spi_disable();
      #endif
      Serial.flush();
      // disable watchdog: the next awakening is given by an interrupt of rain and I do not know how long it will take place
      wdt_disable();

      //! enter in power down mode only if DEBOUNCING_POWER_DOWN_TIME_MS milliseconds have passed since last time (awakened_event_occurred_time_ms)
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
        LOGT(F("Restart I2C BUS"));
        init_wire();
        wdt_reset();
      }

      if (i2c_time >= I2C_MAX_TIME) {
	LOGN(F("Restart I2C BUS"));
	init_wire();
	wdt_reset();
      }
      
      if (is_event_leaf_reading) {
        leaf_reading_task();
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
    Serial.println   (F("The FAT type of the volume: "));
    Serial.println   (SD.vol()->fatType());
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
  readable_data_write_ptr->module_minor_version = MODULE_MINOR_VERSION;
  memset((void *) &readable_data_write_ptr->leaf_wetness, UINT8_MAX, sizeof(leaf_wetness_t));

  reset_samples_buffer();
  reset_report_buffer();

  //! copy readable_data_2 in readable_data_1
  memcpy((void *) readable_data_read_ptr, (const void*) readable_data_write_ptr, sizeof(readable_data_t));
}

void init_tasks() {
  noInterrupts();

  //! no tasks ready
  ready_tasks_count = 0;

  is_event_command_task = false;
  is_event_leaf_reading = false;

  leaf_reading_state = LEAF_READING_INIT;

  transaction_time = 0;
  inside_transaction = false;

  lastcommand =I2C_LEAF_COMMAND_NONE;

  interrupts();
}

void init_pins() {
  pinMode(CONFIGURATION_RESET_PIN, INPUT_PULLUP);
  pinMode(LEAF_POWER_PIN, OUTPUT);   // not used
  pinMode(LEAF_ANALOG_PIN, INPUT);
  pinMode(SDCARD_CHIP_SELECT_PIN, OUTPUT);
}

void init_wire() {
  i2c_error = 0;
  i2c_time = 0;

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
  //start_timer();
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
  LOGN(F("--> leaf wet calibration threshold: [ %d ]"), configuration.leaf_calibration_threshold);
}

void save_configuration(bool is_default) {
  if (is_default) {
    LOGN(F("Save default configuration... [ %s ]"), OK_STRING);
    configuration.module_type = MODULE_TYPE;
    configuration.module_main_version = MODULE_MAIN_VERSION;
    configuration.module_configuration_version = MODULE_CONFIGURATION_VERSION;
    configuration.i2c_address = CONFIGURATION_DEFAULT_I2C_ADDRESS;
    configuration.is_oneshot = CONFIGURATION_DEFAULT_IS_ONESHOT;
    configuration.leaf_calibration_threshold = CONFIGURATION_DEFAULT_LEAF_CALIBRATION_THRESHOLD;
  }
  else {
    LOGN(F("Save configuration... [ %s ]"), OK_STRING);
    configuration.i2c_address = writable_data.i2c_address;
    configuration.is_oneshot = writable_data.is_oneshot;
    configuration.leaf_calibration_threshold = writable_data.leaf_calibration_threshold;
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
  
  writable_data.i2c_address = configuration.i2c_address;
  writable_data.is_oneshot = configuration.is_oneshot;
}

void init_sensors () {
  LOGN(F("--> acquiring samples every %d seconds"), SENSORS_SAMPLE_TIME_MS / 1000);
}

/*!
\fn ISR(TIMER1_OVF_vect)
\brief Timer1 overflow interrupts routine.
\return void.
*/
ISR(TIMER1_OVF_vect) {
  //! Pre-load timer counter register
  TCNT1 = TIMER1_TCNT1_VALUE;
  i2c_time+=TIMER1_INTERRUPT_TIME_MS/1000;

  //! increment timer_counter_ms by TIMER1_INTERRUPT_TIME_MS
  timer_counter_ms += TIMER1_INTERRUPT_TIME_MS;

  //! check if SENSORS_SAMPLE_TIME_MS ms have passed since last time. if true and if is in continuous mode and continuous start command It has been received, activate Sensor reading task
  if (executeTimerTaskEach(timer_counter_ms, SENSORS_SAMPLE_TIME_MS, TIMER1_INTERRUPT_TIME_MS)) {
    if (!is_event_leaf_reading) {
      is_event_leaf_reading = true;
      ready_tasks_count++;
    }
  }

  //! reset timer_counter_ms if it has become larger than TIMER_COUNTER_VALUE_MAX_MS
  if (timer_counter_ms >= TIMER_COUNTER_VALUE_MAX_MS) {
    timer_counter_ms = 0;
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
  if (readable_data_length) {
    //! write readable_data_length bytes of data stored in readable_data_read_ptr (base) + readable_data_address (offset) on i2c bus
    Wire.write((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length);
    //! write crc8
    Wire.write(crc8((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length));
  }
  else {
    Wire.write(UINT16_MAX);
  }
  inside_transaction = false;
  
}

void i2c_receive_interrupt_handler(int rx_data_length) {
  bool is_i2c_data_ok = false;

  readable_data_length = 0;

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
  if (i2c_rx_data[rx_data_length - 1] == crc8((uint8_t *)(i2c_rx_data), rx_data_length - 1)) {
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
      // enable Command task
      if (!is_event_command_task) {
	lastcommand=i2c_rx_data[1];    // record command to be executed
        is_event_command_task = true;
        ready_tasks_count++;
      }
    }
    // it is a registers write?
    else if (is_writable_register(i2c_rx_data[0])) {
      rx_data_length -= 2;

      if (i2c_rx_data[0] == I2C_LEAF_ADDRESS_ADDRESS && rx_data_length == I2C_LEAF_ADDRESS_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_LEAF_ONESHOT_ADDRESS && rx_data_length == I2C_LEAF_ONESHOT_LENGTH) {
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
    i2c_error++;
  }
}

void leaf_reading_task () {
  static leaf_reading_state_t state_after_wait;
  static uint32_t delay_ms;
  static uint32_t start_time_ms;
  static uint8_t i;
  static float leaf_value;

  switch (leaf_reading_state) {
    case LEAF_READING_INIT:
      i = 0;
      leaf_value = 0;
      leaf_reading_state = LEAF_READING_READ;
      LOGV(F("LEAF_READING_INIT --> LEAF_READING_READ"));
    break;

    case LEAF_READING_READ:
      leaf_value += ((float)(analogRead(LEAF_ANALOG_PIN)) - leaf_value) / (float) (i+1);

      LOGN(F("sample leaf: %D %d"),leaf_value,i);
      
      if (i < LEAF_READ_COUNT) {
        i++;
        delay_ms = LEAF_VALUES_READ_DELAY_MS;
        start_time_ms = millis();
        state_after_wait = LEAF_READING_READ;
        leaf_reading_state = LEAF_READING_WAIT_STATE;
      }
      else {
        leaf_reading_state = LEAF_READING_END;
        LOGV(F("LEAF_READING_READ --> LEAF_READING_END"));
      }
    break;

    case LEAF_READING_END:
      if (leaf_value <= configuration.leaf_calibration_threshold) {
        is_leaf_wet = true;
      }
      else {
        is_leaf_wet = false;
      }

      LOGN(F("leaf wet: %T"),is_leaf_wet);
      
      if (is_leaf_wet) {
        readable_data_write_ptr->leaf_wetness.time += (SENSORS_SAMPLE_TIME_MS / 1000);
      }

      LOGN(F("leaf wet time: %l"),readable_data_write_ptr->leaf_wetness.time);
      
      noInterrupts();
      is_event_leaf_reading = false;
      ready_tasks_count--;
      interrupts();

      leaf_reading_state = LEAF_READING_INIT;
      LOGV(F("LEAF_READING_END --> LEAF_READING_INIT"));
    break;

    case LEAF_READING_WAIT_STATE:
      if (millis() - start_time_ms > delay_ms) {
        leaf_reading_state = state_after_wait;
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

/*
void copy_oneshot_data () {
  LOGN(F("Report Leaf wetness: [ %s ] Total [ %D ] seconds"), is_leaf_wet ? "WET" : "DRY", leaf_wetness.time);
  readable_data_write_ptr->leaf_wetness.time = leaf_wetness.time;
}
*/
void reset_samples_buffer() {
  readable_data_write_ptr->leaf_wetness.time = 0;
}

void reset_report_buffer () {
  readable_data_write_ptr->leaf_wetness.time = UINT32_MAX;
}

void command_task() {

  switch(lastcommand) {
  case I2C_LEAF_COMMAND_ONESHOT_START:
    LOGN(F("Execute [ ONESHOT START ]"));
    is_start = true;
    is_stop = false;
    is_test_read = false;
    commands();
    break;

  case I2C_LEAF_COMMAND_ONESHOT_STOP:
    LOGN(F("Execute [ ONESHOT STOP ]"));
    is_start = false;
    is_stop = true;
    is_test_read = false;
    commands();
    transaction_time = 0;
    inside_transaction = true;    
    break;

  case I2C_LEAF_COMMAND_ONESHOT_START_STOP:
    LOGN(F("ONESHOT START-STOP"));;
    is_start = true;
    is_stop = true;
    is_test_read = false;
    commands();
    transaction_time = 0;
    inside_transaction = true;
    break;
    
  case I2C_LEAF_COMMAND_CONTINUOUS_START:
    LOGN(F("Execute [ CONTINUOUS START ]"));
    is_start = true;
    is_stop = false;
    is_test_read = false;
    commands();
    break;

  case I2C_LEAF_COMMAND_CONTINUOUS_STOP:
    LOGN(F("Execute [ CONTINUOUS STOP ]"));
    is_start = false;
    is_stop = true;
    is_test_read = false;
    commands();
    transaction_time = 0;
    inside_transaction = true;
    break;
    
  case I2C_LEAF_COMMAND_CONTINUOUS_START_STOP:
    LOGN(F("Execute [ CONTINUOUS START-STOP ]"));
    is_start = true;
    is_stop = true;
    is_test_read = false;
    commands();
    transaction_time = 0;
    inside_transaction = true;
    break;
    
  case I2C_LEAF_COMMAND_TEST_READ:
    LOGN(F("Execute [ TEST READ ]"));
    is_stop = false;
    is_test_read = true;
    commands();
    break;

  case I2C_LEAF_COMMAND_SAVE:
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
  lastcommand =I2C_LEAF_COMMAND_NONE;
  interrupts();
}

void copy_buffers() {
   //! copy readable_data_2 in readable_data_1
   noInterrupts();
   memcpy((void *) readable_data_read_ptr, (const void*) readable_data_write_ptr, sizeof(readable_data_t));
   interrupts();
}

void commands() {

  if (inside_transaction) {
    LOGE(F("Transaction error"));
    return;
  }

  //! CONTINUOUS TEST
  if (!configuration.is_oneshot && is_start && !is_stop && is_test_read) {
    copy_buffers();
    //exchange_buffers();
  }
  //! CONTINUOUS START
  else if (!configuration.is_oneshot && is_start && !is_stop && !is_test_read) {

    stop_timer();
    reset_samples_buffer();
    //reset_report_buffer();
    //make_report(true);
    start_timer();
  }
  //! CONTINUOUS STOP
  else if (!configuration.is_oneshot && !is_start && is_stop) {
    stop_timer();
    copy_buffers();
    reset_report_buffer();
    //exchange_buffers();
  }
  //! CONTINUOUS START-STOP
  else if (!configuration.is_oneshot && is_start && is_stop) {
    stop_timer();
    exchange_buffers();
    reset_samples_buffer();
    //reset_report_buffer();
    //make_report(true);
    start_timer();
  }
  //! ONESHOT START
  else if (configuration.is_oneshot && is_start && !is_stop) {
    reset_samples_buffer();
    //reset_report_buffer();

    noInterrupts();
    if (!is_event_leaf_reading) {
      is_event_leaf_reading = true;
      ready_tasks_count++;
    }

    // TODO
    interrupts();
  }
  //! ONESHOT STOP
  else if (configuration.is_oneshot && !is_start && is_stop) {

    /* TODO
    readable_data_write_ptr->temperature.sample = temperature;
    readable_data_write_ptr->humidity.sample = humidity;
    exchange_buffers();
    */
  }
  //! ONESHOT START-STOP
  else if (configuration.is_oneshot && is_start && is_stop) {
   
    /* TODO
    readable_data_write_ptr->temperature.sample = temperature;
    readable_data_write_ptr->humidity.sample = humidity;
    exchange_buffers();
    */    
    noInterrupts();
    if (!is_event_leaf_reading) {
      is_event_leaf_reading = true;
      ready_tasks_count++;
    }
    interrupts();
  }
}
