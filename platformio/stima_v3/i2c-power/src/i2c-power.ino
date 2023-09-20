/*********************************************************************
Copyright (C) 2023  Paolo patruno <p.patruno@iperbole.bologna.it>
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

#include "i2c-power.h"

/*!
\fn void setup()
\brief Arduino setup function.
*  Init watchdog, hardware, debug and load configuration stored in EEPROM.
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
         // disable watchdog: the next awakening is given by an interrupt and I do not know how long it will take place
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

      if (is_event_power_task) {
        power_task();
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
  memset((void *) &readable_data_read_ptr->power, UINT8_MAX, sizeof(data_t));
  //! copy readable_data_write in readable_data_read
  copy_buffers();

  reset_samples_buffer();
  reset_data(readable_data_write_ptr);
  
  readable_data_address=0xFF;
  readable_data_length=0;
  make_report(true);

}

void init_tasks() {

   // no tasks ready
  ready_tasks_count = 0;

  is_event_command_task = false;
  is_event_power_task = false;

  power_state = POWER_INIT;
  
  lastcommand=I2C_POWER_COMMAND_NONE;
  is_start = false;
  is_stop = false;
  is_test_read = false;
  transaction_time = 0;
  inside_transaction = false;
}

void init_pins() {
  pinMode(CONFIGURATION_RESET_PIN, INPUT_PULLUP);
  pinMode(SDCARD_CHIP_SELECT_PIN, OUTPUT);
  //  pinMode(POWER_ANALOG_PIN1, INPUT);
  //  pinMode(POWER_ANALOG_PIN2, INPUT);
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
  //start_timer();
}

void start_timer() {
  LOGN("start timer");
  TCCR1A = 0x00;                //!< Normal timer operation
  TCCR1B = (1<<CS10) | (1<<CS12);   //!< 1:1024 prescaler
  TCNT1 = TIMER1_TCNT1_VALUE;   //!< Pre-load timer counter register
  TIFR1 |= (1 << TOV1);         //!< Clear interrupt overflow flag register
  timer_counter_ms = 0;
  TIMSK1 |= (1 << TOIE1);       //!< Enable overflow interrupt
}

void stop_timer() {
  LOGN("stop timer");
  TCCR1B = 0x00;                //!< Stop
  TIMSK1 &= ~(1 << TOIE1);      //!< Disable overflow interrupt
  TIFR1 |= (1 << TOV1);         //!< Clear interrupt overflow flag register
  TCNT1 = TIMER1_TCNT1_VALUE;   //!< Pre-load timer counter register
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
  timer_counter_s += (uint16_t)(TIMER1_INTERRUPT_TIME_MS/1000);
  
  
  //! check if SENSORS_SAMPLE_TIME_MS ms have passed since last time. if true and if is in continuous mode and continuous start command It has been received, activate power task
  if (executeTimerTaskEach(timer_counter_ms, SENSORS_SAMPLE_TIME_MS, TIMER1_INTERRUPT_TIME_MS) && !configuration.is_oneshot) {
    if (!is_event_power_task) {
      is_event_power_task = true;
      ready_tasks_count++;
    }
  }
  
  //! reset timer_counter_ms if it has become larger than TIMER_COUNTER_VALUE_MAX_MS
  if (timer_counter_ms >= TIMER_COUNTER_VALUE_MAX_MS) {
    timer_counter_ms = 0;
  }
  
  if (timer_counter_s >= TIMER_COUNTER_VALUE_MAX_S) {
      timer_counter_s = 0;
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
#endif

void init_system() {
  #if (USE_POWER_DOWN)
  set_sleep_mode(SLEEP_MODE_IDLE);
  awakened_event_occurred_time_ms = millis();
  #endif

   // main loop state
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

  LOGN(F("--> adc panel   voltage max: %d mV"), configuration.adc_voltage_max_panel);
  LOGN(F("--> adc battery voltage max: %d mV"), configuration.adc_voltage_max_battery);

}

void save_configuration(bool is_default) {
  if (is_default) {
    LOGN(F("Save default configuration... [ %s ]"), OK_STRING);
    configuration.module_type = MODULE_TYPE;
    configuration.module_main_version = MODULE_MAIN_VERSION;
    configuration.module_configuration_version = MODULE_CONFIGURATION_VERSION;
    configuration.i2c_address = CONFIGURATION_DEFAULT_I2C_ADDRESS;
    configuration.is_oneshot = CONFIGURATION_DEFAULT_ONESHOT;

    configuration.adc_voltage_max_panel = CONFIGURATION_DEFAULT_ADC_VOLTAGE_MAX_PANEL;
    configuration.adc_voltage_max_battery = CONFIGURATION_DEFAULT_ADC_VOLTAGE_MAX_BATTERY;

    /*
    //warning: large integer implicitly truncated to unsigned type [-Woverflow]
    solaRadiationOffset(POWER_READ_COUNT, POWER_READ_DELAY_MS, &configuration.adc_voltage_offset_1, configuration.adc_voltage_min);
    */
    
  }
  else {
    LOGN(F("Save configuration... [ %s ]"), OK_STRING);
    configuration.i2c_address = writable_data.i2c_address;
    configuration.is_oneshot = writable_data.is_oneshot;

    configuration.adc_voltage_max_panel = writable_data.adc_voltage_max_panel;
    configuration.adc_voltage_max_battery = writable_data.adc_voltage_max_battery;

  }

   // write configuration to eeprom
  ee_write(&configuration, CONFIGURATION_EEPROM_ADDRESS, sizeof(configuration));

   print_configuration();
}

void load_configuration() {
  // read configuration from eeprom
  ee_read(&configuration, CONFIGURATION_EEPROM_ADDRESS, sizeof(configuration));
  
  if (configuration.module_type != MODULE_TYPE || configuration.module_main_version != MODULE_MAIN_VERSION || configuration.module_configuration_version != MODULE_CONFIGURATION_VERSION || digitalRead(CONFIGURATION_RESET_PIN) == LOW) {
    save_configuration(CONFIGURATION_DEFAULT);
  } else {
    LOGN(F("Load configuration... [ %s ]"), OK_STRING);
    print_configuration();
  }
  
   wdt_reset();

  // set configuration value to writable register
  writable_data.i2c_address = configuration.i2c_address;
  writable_data.is_oneshot = configuration.is_oneshot;
  writable_data.adc_voltage_max_panel = configuration.adc_voltage_max_panel;
  writable_data.adc_voltage_max_battery = configuration.adc_voltage_max_battery;
}

void init_sensors () {
}

void i2c_request_interrupt_handler() {
   // write readable_data_length bytes of data stored in readable_data_read_ptr (base) + readable_data_address (offset) on i2c bus
    Wire.write((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length);
    Wire.write(crc8((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length));
    // attention: logging inside ISR !
    //LOGV("request_interrupt_handler: %d-%d crc:%d",readable_data_address,readable_data_length,crc8((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length));
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

      if (i2c_rx_data[0] == I2C_POWER_ADDRESS_ADDRESS && rx_data_length == I2C_POWER_ADDRESS_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_POWER_ONESHOT_ADDRESS && rx_data_length == I2C_POWER_ONESHOT_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_POWER_VOLTAGE_MAX_PANEL_ADDRESS && rx_data_length == I2C_POWER_VOLTAGE_MAX_PANEL_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_POWER_VOLTAGE_MAX_BATTERY_ADDRESS && rx_data_length == I2C_POWER_VOLTAGE_MAX_BATTERY_LENGTH) {
        is_i2c_data_ok = true;
      }
      if (is_i2c_data_ok) {
        for (uint8_t i = 0; i < rx_data_length; i++) {
          // write rx_data_length bytes in writable_data_ptr (base) at (i2c_rx_data[i] - I2C_WRITE_REGISTER_START_ADDRESS) (position in buffer)
	  //LOGN("write DATA %d:%d",i2c_rx_data[0] - I2C_WRITE_REGISTER_START_ADDRESS + i, i2c_rx_data[i + 1]);
          ((uint8_t *)(writable_data_ptr))[i2c_rx_data[0] - I2C_WRITE_REGISTER_START_ADDRESS + i] = i2c_rx_data[i + 1];
        }
      }
      /*
      else{
	LOGE("wrong rxdata address: %d length %d",i2c_rx_data[0],rx_data_length);
      }
      */
    }
  } else {
    readable_data_address=0xFF;
    readable_data_length = 0;
    // attention: logging inside ISR !
    //LOGE(F("CRC error: size %d  CRC %d:%d"),rx_data_length,i2c_rx_data[rx_data_length - 1], crc8((uint8_t *)(i2c_rx_data), rx_data_length - 1));
    i2c_error++;
  }
}

void make_report (bool init) {
  if (init) {
    samples_count_panel=0;
    samples_error_count_panel=0;
    samples_count_battery=0;
    samples_error_count_battery=0;
    
    sample_panel=INT16_MAX;
    sample_battery=INT16_MAX;
    average_panel=INT16_MAX;
    average_battery=INT16_MAX;
  }else{

    if (ISVALID_INT16(sample_panel)){    
      samples_count_panel++;
      if (samples_count_panel == 1) average_panel=sample_panel;
      average_panel += round((float(sample_panel) - float(average_panel)) / float(samples_count_panel));
    }else{
      samples_error_count_panel++;
    }


    if (ISVALID_INT16(sample_battery)){    
      samples_count_battery++;
      if (samples_count_battery == 1) average_battery=sample_battery;
      average_battery += round((float(sample_battery) - float(average_battery)) / float(samples_count_battery));
    }else{
      samples_error_count_battery++;
    }

  }
  LOGN("panel samples_count: %d; error_count: %d; sample: %d; average: %d",samples_count_panel,samples_error_count_panel,sample_panel,average_panel);
  LOGN("battery samples_count: %d; error_count: %d; sample: %d; average: %d",samples_count_battery,samples_error_count_battery,sample_battery,average_battery);
}

/*
uint16_t powerRead(uint8_t analog){
  if (analog == 1){
    return analogRead(POWER_ANALOG_PIN1);
  }else{
    return analogRead(POWER_ANALOG_PIN2);
  }
}

uint16_t powerMean(uint8_t analog, uint8_t count, uint8_t delay_ms) {
  float value = 0;
  for (uint8_t i = 0; i < count; i++) {
    value += ((float) powerRead(analog) - value) / (float) (i+1);
    delay(delay_ms);
  }
  return round(value);
}

uint16_t getPowerVoltage (uint8_t analog, uint16_t voltage_max) {
  // Convert the analog reading (which goes from 0 – 1023) to a voltage:
  return round( float(powerMean(analog)) * (float(voltage_max) / 1023.0));
}
*/

void power_task () {
  static power_state_t state_after_wait;
  static uint32_t delay_ms;
  static uint32_t start_time_ms;
  adc_result_t adc_result;

  switch (power_state) {
    case POWER_INIT:
      power_state = POWER_READING_PANEL;
      LOGV(F("POWER_INIT --> POWER_READING_PANEL"));
    break;

    /*   use ADC internal to atmega MCU
  case POWER_READING:
      sample_panel = getPowerVoltage(1,30000);
      sample_battery = getPowerVoltage(2,15000);
      power_state = POWER_ELABORATE;
      LOGV(F("POWER_READING --> POWER_ELABORATE"));
    break;
    */

    case POWER_READING_PANEL:

      adc_result = adc1.readSingleChannel(POWER_ADC_CHANNEL_INPUT_PANEL, &sample_panel);
      
      if (adc_result == ADC_OK) {
	sample_panel = round( float(sample_panel) * (float(writable_data.adc_voltage_max_panel) / float(0X7FFF)));	
	LOGN("panel adc_value: %d",sample_panel);	

	power_state = POWER_READING_BATTERY;
	LOGV(F("POWER_READING --> POWER_READING_PANEL"));
      }
      else if (adc_result == ADC_ERROR) {
	LOGE("ADC readSingleChannel panel error");
        i2c_error++;
        sample_panel = INT16_MAX;

	power_state = POWER_READING_BATTERY;
	LOGV(F("POWER_READING --> POWER_READING_PANEL"));

      } else if (adc_result == ADC_BUSY) {
	LOGV("ADC PANEL readSingleChannel busy");
      }
     
    break;

    case POWER_READING_BATTERY:

      adc_result = adc1.readSingleChannel(POWER_ADC_CHANNEL_INPUT_BATTERY, &sample_battery);
      
      if (adc_result == ADC_OK) {
	sample_battery = round( float(sample_battery) * (float(writable_data.adc_voltage_max_battery) / float(0X7FFF)));
	LOGN("battery adc_value: %d",sample_battery);	

	power_state = POWER_ELABORATE;
	LOGV(F("POWER_READING --> POWER_ELABORATE"));
      }
      else if (adc_result == ADC_ERROR) {
	LOGE("ADC readSingleChannel battery error");
        i2c_error++;
        sample_battery = INT16_MAX;

	power_state = POWER_ELABORATE;
	LOGV(F("POWER_READING --> POWER_ELABORATE"));

      } else if (adc_result == ADC_BUSY) {
	LOGV("ADC BATTERY readSingleChannel busy");
      }

    break;    

    case POWER_ELABORATE:
      
      make_report();

      readable_data_write_ptr->power.sample_panel = round(float(sample_panel)/100.);
      readable_data_write_ptr->power.sample_battery = round(float(sample_battery)/100.);

      if (is_start && samples_count_panel > ((RMAP_REPORT_SAMPLE_ERROR_MAX_PERC*1000)/SENSORS_SAMPLE_TIME_MS)){
	if((float(samples_error_count_panel) / float(samples_count_panel) *100) <= RMAP_REPORT_SAMPLE_ERROR_MAX_PERC){ 
	  readable_data_write_ptr->power.avg_panel = round(float(average_panel)/100.);
	}else{
	  LOGE(F("REPORT_SAMPLE_ERROR_MAX_PERC error good: %d ; bad: %d"), samples_count_panel,samples_error_count_panel);
	  readable_data_write_ptr->power.avg_panel = INT16_MAX;	  
	}
      }

      if (is_start && samples_count_battery > ((RMAP_REPORT_SAMPLE_ERROR_MAX_PERC*1000)/SENSORS_SAMPLE_TIME_MS)){
	if((float(samples_error_count_battery) / float(samples_count_battery) *100) <= RMAP_REPORT_SAMPLE_ERROR_MAX_PERC){ 
	  readable_data_write_ptr->power.avg_battery = round(float(average_battery)/100.);
	}else{
	  LOGE(F("REPORT_SAMPLE_ERROR_MAX_PERC error good: %d ; bad: %d"), samples_count_battery,samples_error_count_battery);
	  readable_data_write_ptr->power.avg_battery = INT16_MAX;	  
	}
      }
      
      power_state = POWER_END;
      LOGV(F("POWER_ELABORATE --> POWER_END"));
    break;

    case POWER_END:
      noInterrupts();
      is_event_power_task = false;
      ready_tasks_count--;
      interrupts();
      power_state = POWER_INIT;
      LOGV(F("POWER_END --> POWER_INIT"));
    break;

    case POWER_WAIT_STATE:
      if (millis() - start_time_ms > delay_ms) {
        power_state = state_after_wait;
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
  samples_count_panel=0;
  sample_panel=INT16_MAX;
  samples_count_battery=0;
  sample_battery=INT16_MAX;
}

void reset_data(volatile readable_data_t *ptr) {
  ptr->power.sample_panel = INT16_MAX;
  ptr->power.avg_panel = INT16_MAX;
  ptr->power.sample_battery = INT16_MAX;
  ptr->power.avg_battery = INT16_MAX;
}

void command_task() {
  switch(lastcommand) {
    case I2C_POWER_COMMAND_ONESHOT_START:
    if (configuration.is_oneshot) {
      LOGN(F("Execute [ %s ]"), "ONESHOT START");
      is_start = true;
      is_stop = false;
      is_test_read = false;
      commands();
    } else {
      LOGE(F("Skip command [ %s ] in continous mode"), "ONESHOT START");
    }
    break;

    case I2C_POWER_COMMAND_ONESHOT_STOP:
    if (configuration.is_oneshot) {
      LOGN(F("Execute [ %s ]"), "ONESHOT STOP");
      is_start = false;
      is_stop = true;
      commands();
      inside_transaction = true;
    } else {
      LOGE(F("Skip command [ %s ] in continous mode"), "ONESHOT STOP");
    }
    break;

    case I2C_POWER_COMMAND_ONESHOT_START_STOP:
    if (configuration.is_oneshot) {
      LOGN(F("Execute [ %s ]"), "ONESHOT START-STOP");
      is_start = true;
      is_stop = true;
      is_test_read = false;
      commands();
      inside_transaction = true;
    } else {
      LOGE(F("Skip command [ %s ] in continous mode"), "ONESHOT START-STOP");
    }
    break;

    case I2C_POWER_COMMAND_CONTINUOUS_START:
    if (!configuration.is_oneshot) {
      LOGN(F("Execute [ %s ]"), "CONTINUOUS START");
      is_start = true;
      is_stop = false;
      is_test_read = false;
      commands();
    } else {
      LOGE(F("Skip command [ %s ] in oneshot mode"), "CONTINUOUS START");
    }
    break;

    case I2C_POWER_COMMAND_CONTINUOUS_STOP:
    if (!configuration.is_oneshot) {
      LOGN(F("Execute [ %s ]"), "CONTINUOUS STOP");
      is_start = false;
      is_stop = true;
      is_test_read = false;
      commands();
      inside_transaction = true;
    } else {
      LOGE(F("Skip command [ %s ] in oneshot mode"), "CONTINUOUS STOPT");
    }

    break;

    case I2C_POWER_COMMAND_CONTINUOUS_START_STOP:
    if (!configuration.is_oneshot) {
      LOGN(F("Execute [ %s ]"), "CONTINUOUS START_STOP");
      is_start = true;
      is_stop = true;
      is_test_read = false;
      commands();
      inside_transaction = true;
    } else {
      LOGE(F("Skip command [ %s ] in oneshot mode"), "CONTINUOUS START_STOPT");
    }

    break;

    case I2C_POWER_COMMAND_TEST_READ:
    if (!configuration.is_oneshot) {
      LOGN(F("Execute [ %s ]"), "CONTINUOUS TEST_READ");
      //is_start = true;
      is_stop = false;
      is_test_read = true;
      commands();
    } else {
      LOGE(F("Skip command [ %s ] in oneshot mode"), "CONTINUOUS TEST_READ");
    }
    break;

    case I2C_POWER_COMMAND_SAVE:
      is_start = false;
      is_stop = false;
      LOGN(F("Execute [ %s ]"), "SAVE");
      save_configuration(CONFIGURATION_CURRENT);
      init_wire();
    break;
  
  default:
    LOGN(F("Ignore unknow command: %s"),lastcommand);
  }
  
  noInterrupts();
  is_event_command_task = false;
  ready_tasks_count--;
  lastcommand=I2C_POWER_COMMAND_NONE;
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
    if (!is_event_power_task) {
      is_event_power_task = true;
      ready_tasks_count++;
    }
    interrupts();
  }
  //! ONESHOT STOP
  else if (configuration.is_oneshot && !is_start && is_stop) {

    readable_data_write_ptr->power.sample_panel = sample_panel;
    readable_data_write_ptr->power.sample_battery = sample_battery;
    exchange_buffers();
  }
  //! ONESHOT START-STOP
  else if (configuration.is_oneshot && is_start && is_stop) {
   
    readable_data_write_ptr->power.sample_panel = sample_panel;
    readable_data_write_ptr->power.sample_battery = sample_battery;
    exchange_buffers();
    
    noInterrupts();
    if (!is_event_power_task) {
      is_event_power_task = true;
      ready_tasks_count++;
    }
    interrupts();
  }
}



