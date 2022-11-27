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

#include "i2c-wind.h"

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

  #if (USE_SENSOR_GWS)
  Serial1.setTimeout(GWS_SERIAL_TIMEOUT_MS);
  Serial1.begin(GWS_SERIAL_BAUD);
  serial1_reset();
  #endif
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

    #if (USE_SENSOR_DED || USE_SENSOR_DES || USE_SENSOR_GWS)
    if (is_event_wind_task) {
      wind_task();
      wdt_reset();
    }
    #endif

    if (is_event_command_task) {
      command_task();
      wdt_reset();
    }

    // I2C Bus Check
    if (i2c_error >= I2C_MAX_ERROR_COUNT) {
      LOGE(F("Restart I2C BUS by errorcount"));
      init_wire();
      wdt_reset();
    }

    if (i2c_time >= I2C_MAX_TIME) {
      LOGN(F("Restart I2C BUS"));
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
  memset((void *) &readable_data_write_ptr->wind, UINT8_MAX, sizeof(report_t));
  
  //! copy readable_data_write in readable_data_read
  copy_buffers();

  cb_direction.clear();
  cb_speed.clear();
  
  reset_samples_buffer();
  
  readable_data_address=0xFF;
  readable_data_length=0;
  make_report(true);
}

void init_tasks() {

  //! no tasks ready
  ready_tasks_count = 0;

  is_event_command_task = false;
  is_event_wind_task = false;

  wind_state = WIND_INIT;

  lastcommand=I2C_WIND_COMMAND_NONE;
  is_start = false;
  is_stop = false;
  is_test = false;
  transaction_time = 0;
  inside_transaction = false;
}

void init_pins() {
  pinMode(CONFIGURATION_RESET_PIN, INPUT_PULLUP);
  pinMode(SDCARD_CHIP_SELECT_PIN, OUTPUT);

  #if (USE_SENSOR_DED || USE_SENSOR_DES)
  pinMode(WIND_POWER_PIN, OUTPUT);
  pinMode(WIND_DIRECTION_ANALOG_PIN, INPUT);
  windPowerOn();
  #endif

  #if (USE_SENSOR_DES)
  wind_speed = 0;
  wind_speed_count = 0;
  attachInterrupt(digitalPinToInterrupt(WIND_SPEED_DIGITAL_PIN), wind_speed_interrupt_handler, FALLING);
  #endif

  #if (USE_SENSOR_GWS)
  pinMode(WIND_POWER_PIN, OUTPUT);
  windPowerOn();
  #endif
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
  timer_counter_ms = 0;
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
  LOGN(F("--> adc voltage offset +: %f"), configuration.adc_voltage_offset_1);
  LOGN(F("--> adc voltage offset *: %f"), configuration.adc_voltage_offset_2);
  LOGN(F("--> adc voltage min: %f mV"), configuration.adc_voltage_min);
  LOGN(F("--> adc voltage max: %f mV"), configuration.adc_voltage_max);
}

void save_configuration(bool is_default) {
  if (is_default) {
    LOGN(F("Save default configuration... [ %s ]"), OK_STRING);
    configuration.module_type = MODULE_TYPE;
    configuration.module_main_version = MODULE_MAIN_VERSION;
    configuration.module_configuration_version = MODULE_CONFIGURATION_VERSION;
    configuration.i2c_address = CONFIGURATION_DEFAULT_I2C_ADDRESS;
    configuration.is_oneshot = CONFIGURATION_DEFAULT_IS_ONESHOT;
    
    #if (USE_SENSOR_DES || USE_SENSOR_DED)
    configuration.adc_voltage_offset_1 = CONFIGURATION_DEFAULT_ADC_VOLTAGE_OFFSET_1;
    configuration.adc_voltage_offset_2 = CONFIGURATION_DEFAULT_ADC_VOLTAGE_OFFSET_2;
    configuration.adc_voltage_min = CONFIGURATION_DEFAULT_ADC_VOLTAGE_MIN;
    configuration.adc_voltage_max = CONFIGURATION_DEFAULT_ADC_VOLTAGE_MAX;
    #endif

    #if (USE_SENSOR_DED)
    windPowerOn();
    delay(WIND_SETUP_DELAY_MS);
    calibrationValue(WIND_READ_COUNT, WIND_READ_DELAY_MS, &configuration.adc_voltage_max);
    LOGN(F("--> Set adc voltage max to %f mV"), configuration.adc_voltage_max);
    windPowerOff();
    #endif
  }
  else {
    LOGN(F("Save configuration... [ %s ]"), OK_STRING);
    configuration.i2c_address = writable_data.i2c_address;
    configuration.is_oneshot = writable_data.is_oneshot;
    configuration.adc_voltage_offset_1 = writable_data.adc_voltage_offset_1;
    configuration.adc_voltage_offset_2 = writable_data.adc_voltage_offset_2;
    configuration.adc_voltage_min = writable_data.adc_voltage_min;
    configuration.adc_voltage_max = writable_data.adc_voltage_max;
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
    LOGN(F("--> samples every %d ms"),SENSORS_SAMPLE_TIME_MS);
  }
  
  wdt_reset();
  
  writable_data.i2c_address = configuration.i2c_address;
  writable_data.is_oneshot = configuration.is_oneshot;
  writable_data.adc_voltage_offset_1 = configuration.adc_voltage_offset_1;
  writable_data.adc_voltage_offset_2 = configuration.adc_voltage_offset_2;
  writable_data.adc_voltage_min = configuration.adc_voltage_min;
  writable_data.adc_voltage_max = configuration.adc_voltage_max;
}

void init_sensors () {
  if (!configuration.is_oneshot) {
    #if (USE_SENSOR_GWS)
    LOGN(F("sc: speed sample count"));
    LOGN(F("dc: direction sample count"));
    LOGN(F("speed: sensor speed"));
    LOGN(F("dir: sensor direction"));
    LOGN(F("ua: average u component over 10'"));
    LOGN(F("va: average v component over 10'"));
    LOGN(F("vs10: vectorial average speed"));
    LOGN(F("vd10: vectorial average direction"));
    LOGN(F("ub: average u component"));
    LOGN(F("vb: average v component"));
    LOGN(F("vsr: vectorial average speed"));
    LOGN(F("vdr: vectorial average direction'"));
    LOGN(F("ss: scalar average speed"));
    LOGN(F("pgs: peak gust speed"));
    LOGN(F("pgd: peak gust direction"));
    LOGN(F("lgs: long gust speed'"));
    LOGN(F("lgd: long gust direction'"));
    LOGN(F("C1: %% of sample <= 1.0 m/s "));
    LOGN(F("C2: %% of sample <= 2.0 m/s "));
    LOGN(F("C4: %% of sample <= 4.0 m/s "));
    LOGN(F("C7: %% of sample <= 7.0 m/s "));
    LOGN(F("C10: %% of sample <= 10.0 m/s "));
    LOGN(F("CXX: %% of sample > 10.0 m/s "));

    LOGN(F("sc\tdc\tspeed\tdir\tua\tva\tvs10\tvd10\tub\tvb\tvsr\tvdr\tss\tpgs\tpgd\tlgs\tlgd\tC1\tC2\tC4\tC7\tC10\tCXX"));

    #elif (USE_SENSOR_DES)
    LOGN(F("sc\tss\tC1\tC2\tC4\tC7\tC10\tCXX\ttotal"));
    #elif (USE_SENSOR_DED)
    LOGN(F("dc"));
    #endif
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
  i2c_time+=TIMER1_INTERRUPT_TIME_MS/1000;
  
  //! increment timer_counter_ms by TIMER1_INTERRUPT_TIME_MS
  timer_counter_ms += TIMER1_INTERRUPT_TIME_MS;
  
  if (inside_transaction) {
    //! increment transaction_time by TIMER1_INTERRUPT_TIME_MS
    transaction_time += TIMER1_INTERRUPT_TIME_MS;
    
    if (transaction_time >= TRANSACTION_TIMEOUT_MS) {
      transaction_time = 0;
      inside_transaction = false;
    }
  }
  
#if (USE_SENSOR_DES)
  if (executeTimerTaskEach(timer_counter_ms, SENSORS_ACQ_TIME_MS, TIMER1_INTERRUPT_TIME_MS) && !configuration.is_oneshot) {
    wind_speed = wind_speed_count;
    wind_speed_count = 0;
  }
#endif

#if (USE_SENSOR_DED || USE_SENSOR_DES)
  if (executeTimerTaskEach(timer_counter_ms, SENSORS_WARMUP_TIME_MS, TIMER1_INTERRUPT_TIME_MS) && !configuration.is_oneshot) {
    if (isWindOff()) {
      windPowerOn();
    }
  }
#endif
    
#if (USE_SENSOR_DED || USE_SENSOR_DES || USE_SENSOR_GWS)
  if (executeTimerTaskEach(timer_counter_ms, SENSORS_SAMPLE_TIME_MS, TIMER1_INTERRUPT_TIME_MS) && !configuration.is_oneshot) {
    if (!is_event_wind_task) {
      is_event_wind_task = true;
      ready_tasks_count++;
    }
  }
#endif
    
  //! reset timer_counter_ms if it has become larger than TIMER_COUNTER_VALUE_MAX_MS
  if (timer_counter_ms >= TIMER_COUNTER_VALUE_MAX_MS) {
    timer_counter_ms = 0;
  }
  
}

void i2c_request_interrupt_handler() {
  if (readable_data_length) {
    //! write readable_data_length bytes of data stored in readable_data_read_ptr (base) + readable_data_address (offset) on i2c bus
    Wire.write((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length);
    Wire.write(crc8((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length));
    //! write crc8
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
        is_event_command_task = true;
        ready_tasks_count++;
      }
      //interrupts();
    }
    // it is a registers write?
    else if (is_writable_register(i2c_rx_data[0])) {
      rx_data_length -= 1;

      if (i2c_rx_data[0] == I2C_WIND_ADDRESS_ADDRESS && rx_data_length == I2C_WIND_ADDRESS_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_WIND_ONESHOT_ADDRESS && rx_data_length == I2C_WIND_ONESHOT_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_WIND_CONTINUOUS_ADDRESS && rx_data_length == I2C_WIND_CONTINUOUS_LENGTH) {
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

/*
// reset of buffer (no data) setting all data to missing
template<typename buffer_g, typename length_v, typename value_v> void bufferReset(buffer_g *buffer, length_v length) {
  memset(buffer->value, UINT8_MAX, length * sizeof(value_v));
  buffer->count = 0;
  buffer->read_ptr = buffer->value;
  buffer->write_ptr = buffer->value;
}

// legge dato puntato e incrementa puntatore
template<typename buffer_g, typename length_v, typename value_v> value_v bufferRead(buffer_g *buffer, length_v length) {
  value_v value = *buffer->read_ptr;

  if (buffer->read_ptr == buffer->value+length-1) {
    buffer->read_ptr = buffer->value;
  }
  else buffer->read_ptr++;

  return value;
}

// legge dato puntato e decrementa puntatore
template<typename buffer_g, typename length_v, typename value_v> value_v bufferReadBack(buffer_g *buffer, length_v length) {
  value_v value = *buffer->read_ptr;

  if (buffer->read_ptr == buffer->value) {
    buffer->read_ptr = buffer->value+length-1;
  }
  else buffer->read_ptr--;

  return value;
}


// setta il puntatore di lettura sul dato ultimo scritto
template<typename buffer_g, typename length_v> void bufferPtrResetBack(buffer_g *buffer, length_v length) {
  if (buffer->write_ptr == buffer->value) {
    buffer->read_ptr = buffer->value+length-1;
  }
  else buffer->read_ptr = buffer->write_ptr-1;
}

// used by addValue
template<typename buffer_g, typename length_v> void incrementBuffer(buffer_g *buffer, length_v length) {
  if (buffer->count < length) {
    buffer->count++;
  }

  if (buffer->write_ptr+1 < buffer->value + length) {
    buffer->write_ptr++;
  } else buffer->write_ptr = buffer->value;
}

// add a value at the end of the circular buffer
template<typename buffer_g, typename length_v, typename value_v> void addValue(buffer_g *buffer, length_v length, value_v value) {
  *buffer->write_ptr = (value_v) value;
  incrementBuffer<buffer_g, length_v>(buffer, length);
}
*/

void getSDFromUV (float u, float v, uint16_t *speed, uint16_t *direction) {

  if (u < CALM_WIND_MAX_MS && v < CALM_WIND_MAX_MS){
    *speed = 0;
    *direction = 0;
  } else{
    *speed = round(sqrt(u*u + v*v));
    *direction = round(RAD_TO_DEG * atan2(-u, -v));
    *direction = *direction % 360;
  }
  
  if (*direction == 0) *direction = 360;
  
}

void make_report  (bool init) {

  static uint16_t valid_count;
  static uint16_t error_count;
  
  static uint16_t valid_count_speed;
  static uint16_t error_count_speed;
  
  static uint16_t valid_count_direction;
  static uint16_t error_count_direction;
  
  static float avg_speed;
  
  static float ua;
  static float va;
  
  static uint16_t peak_gust_speed;
  static uint16_t peak_gust_direction;
  
  static uint16_t long_gust_speed;
  static uint16_t long_gust_direction;
  
  static uint16_t class_1_count;
  static uint16_t class_2_count;
  static uint16_t class_3_count;
  static uint16_t class_4_count;
  static uint16_t class_5_count;
  static uint16_t class_6_count;

  reset_data(readable_data_write_ptr);
  
  if (init) {

    reset_samples_buffer();
    
    valid_count = 0;
    error_count = 0;
    
    valid_count_speed = 0;
    error_count_speed = 0;
    
    valid_count_direction = 0;
    error_count_direction = 0;

    avg_speed = 0.;
    ua = 0;
    va = 0;
    
    peak_gust_speed = 0;
    peak_gust_direction = 0;
    
    long_gust_speed = 0;
    long_gust_direction = 0;
    
    class_1_count = 0;
    class_2_count = 0;
    class_3_count = 0;
    class_4_count = 0;
    class_5_count = 0;
    class_6_count = 0;

    return;
  }

  uint16_t speed=UINT16_MAX;
  uint16_t direction=UINT16_MAX;
  
  #if (USE_SENSOR_DES || USE_SENSOR_GWS)
  speed = cb_speed.first();
  #endif
  
  #if (USE_SENSOR_DED || USE_SENSOR_GWS)
  direction = cb_direction.first();
  #endif    

  LOGV(F("circular buffer peek: %d , %d"),speed,direction);
  
  if (ISVALID(speed)) {
    valid_count_speed++;

    avg_speed += (float(speed) - avg_speed) / valid_count_speed;

    if (speed > peak_gust_speed) peak_gust_speed = speed;
    
    if (speed < WIND_CLASS_1_MAX*100.) {
      class_1_count++;
    }
    else if (speed < WIND_CLASS_2_MAX*100.) {
      class_2_count++;
    }
    else if (speed < WIND_CLASS_3_MAX*100.) {
      class_3_count++;
    }
    else if (speed < WIND_CLASS_4_MAX*100.) {
      class_4_count++;
    }
    else if (speed < WIND_CLASS_5_MAX*100.) {
      class_5_count++;
    }
    else {
      class_6_count++;
    }


    if((float(error_count) / float(valid_count_speed) *100) <= RMAP_REPORT_SAMPLE_ERROR_MAX_PERC){   

      readable_data_write_ptr->wind.peak_gust_speed = peak_gust_speed/100.;
      readable_data_write_ptr->wind.avg_speed = avg_speed/100.;
    
      uint8_t class_1 = round(float(class_1_count) / float(valid_count_speed)*100.);
      uint8_t class_2 = round(float(class_2_count) / float(valid_count_speed)*100.);
      uint8_t class_3 = round(float(class_3_count) / float(valid_count_speed)*100.);
      uint8_t class_4 = round(float(class_4_count) / float(valid_count_speed)*100.);
      uint8_t class_5 = round(float(class_5_count) / float(valid_count_speed)*100.);
      uint8_t class_6 = round(float(class_6_count) / float(valid_count_speed)*100.);

      readable_data_write_ptr->wind.class_1 = class_1;
      readable_data_write_ptr->wind.class_2 = class_2;
      readable_data_write_ptr->wind.class_3 = class_3;
      readable_data_write_ptr->wind.class_4 = class_4;
      readable_data_write_ptr->wind.class_5 = class_5;
      readable_data_write_ptr->wind.class_6 = class_6;

      
    }
    
  }else{
    error_count_speed++;
  }
  
  if (ISVALID(direction)) {
    valid_count_direction++;      
  }else{
    error_count_direction++;
  }
  
  if (ISVALID(speed) && ISVALID(direction)) {
    valid_count++;
    
    if (speed == peak_gust_speed) {
      peak_gust_direction = direction;    // add direction to long gust
    }
    
    ua += ((-float(speed) * sin(DEG_TO_RAD * float(direction))) - ua) / float(valid_count);
    va += ((-float(speed) * cos(DEG_TO_RAD * float(direction))) - va) / float(valid_count);

    if((float(error_count) / float(valid_count) *100) <= RMAP_REPORT_SAMPLE_ERROR_MAX_PERC){   
      getSDFromUV(ua, va, &speed, &direction);
      readable_data_write_ptr->wind.vavg10_speed = speed/100.;
      readable_data_write_ptr->wind.vavg10_direction = direction;

      readable_data_write_ptr->wind.peak_gust_direction = peak_gust_direction;
      
    }
    
  } else {
    error_count++;
  }

  // elaborate 60" wind mean in the last period
  
  float ub = 0.;
  float vb = 0.;
  uint16_t cb_error_count=max((int16_t)(LONG_GUST_SAMPLES_COUNT - cb_speed.size()),0);

  for (uint16_t i = 0; i < min(LONG_GUST_SAMPLES_COUNT,cb_speed.size()); i++) {  
    speed = cb_speed[i];
    direction = cb_direction[i];
    LOGV("%d, long gust speed:%d    direction:%d",i, speed,direction);
    if (ISVALID(speed) && ISVALID(direction)) {
      ub += ((-float(speed) * sin(DEG_TO_RAD * float(direction))) - ub) / float(valid_count);
      vb += ((-float(speed) * cos(DEG_TO_RAD * float(direction))) - vb) / float(valid_count);
      
      getSDFromUV(ub, vb, &speed, &direction);
      readable_data_write_ptr->wind.vavg_speed = speed/100.;
      readable_data_write_ptr->wind.vavg_direction = direction;
      
    } else {
      cb_error_count++;
    }
  }

  //LOGE("%d , %d  ,%D",LONG_GUST_SAMPLES_COUNT,cb_error_count,RMAP_REPORT_SAMPLE_ERROR_MAX_PERC);
  
  if((float(cb_error_count) / float(LONG_GUST_SAMPLES_COUNT) *100) <= RMAP_REPORT_SAMPLE_ERROR_MAX_PERC){   
    getSDFromUV(ub, vb, &speed, &direction);
    
    if (speed > long_gust_speed) {
      long_gust_speed = speed;
      long_gust_direction = direction;
    }

    readable_data_write_ptr->wind.long_gust_speed = long_gust_speed/100.;
    readable_data_write_ptr->wind.long_gust_direction = long_gust_direction;
    
  }else{
    LOGE(F("long gust error rate -> total: %d ; bad: %d"), LONG_GUST_SAMPLES_COUNT,cb_error_count);
  }

  // elaborate WMO wind (mean in the last period)
  
  ub = 0.;
  vb = 0.;
  cb_error_count=max((int16_t)WMO_REPORT_SAMPLES_COUNT-cb_speed.size(),0);
  
for (uint16_t i = 0; i < cb_speed.size(); i++) {  
    speed = cb_speed[i];
    direction = cb_direction[i];
    LOGV("%d, wmo speed:%d    direction:%d",i,speed,direction);
    
    if (ISVALID(speed) && ISVALID(direction)) {    
      ub += ((-float(speed) * sin(DEG_TO_RAD * float(direction))) - ub) / float(valid_count);
      vb += ((-float(speed) * cos(DEG_TO_RAD * float(direction))) - vb) / float(valid_count);
    } else {
      cb_error_count++;
    }
  }
  
  if((float(cb_error_count) / float(WMO_REPORT_SAMPLES_COUNT) *100) <= RMAP_REPORT_SAMPLE_ERROR_MAX_PERC){   
    
    getSDFromUV(ub, vb, &speed, &direction);
    if (direction == 0) direction=360;                    // traslate 0 -> 360
    if (speed == WIND_SPEED_MIN*100.) direction=0;        // wind calm
    
  }else{
    LOGE(F("WMO error rate -> total: %d ; bad: %d"), WMO_REPORT_SAMPLES_COUNT,cb_error_count);
  }
  
  #if ((USE_SENSOR_DES && USE_SENSOR_DED) || USE_SENSOR_GWS)
  LOGN(F("%D\t%D\t%D\t%D\t%D\t%D\t%D\t%D\t%D\t%D\t%D\t%D\t%D\t%D\t%D\t%D\t%D\t%D\t%D"),
       ua/100., va/100., readable_data_write_ptr->wind.vavg10_speed, readable_data_write_ptr->wind.vavg10_direction,
       ub, vb, readable_data_write_ptr->wind.vavg_speed, readable_data_write_ptr->wind.vavg_direction,
       readable_data_write_ptr->wind.avg_speed,
       readable_data_write_ptr->wind.peak_gust_speed, readable_data_write_ptr->wind.peak_gust_direction,
       readable_data_write_ptr->wind.long_gust_speed, readable_data_write_ptr->wind.long_gust_direction,
       readable_data_write_ptr->wind.class_1,
       readable_data_write_ptr->wind.class_2,
       readable_data_write_ptr->wind.class_3,
       readable_data_write_ptr->wind.class_4,
       readable_data_write_ptr->wind.class_5,
       readable_data_write_ptr->wind.class_6);
  #elif (USE_SENSOR_DES)
  LOGN(F("%D\t%D\t%D\t%D\t%D\t%D\t%D"),
       readable_data_write_ptr->vavg_speed,
       readable_data_write_ptr->wind.class_1,
       readable_data_write_ptr->wind.class_2,
       readable_data_write_ptr->wind.class_3,
       readable_data_write_ptr->wind.class_4,
       readable_data_write_ptr->wind.class_5,
       readable_data_write_ptr->wind.class_6);
  #elif (USE_SENSOR_DED)
  LOGN(F("\r\n"));
  #endif
}

#if (USE_SENSOR_DED || USE_SENSOR_DES || USE_SENSOR_GWS)
void windPowerOff () {
  digitalWrite(WIND_POWER_PIN, LOW);
  is_wind_on = false;
}

void windPowerOn () {
  digitalWrite(WIND_POWER_PIN, HIGH);
  is_wind_on = true;
}
#endif

#if (USE_SENSOR_DES)
void wind_speed_interrupt_handler() {
  noInterrupts();
  wind_speed_count++;
  interrupts();
}

float getWindSpeed (float count) {
  float value = (count / SENSORS_ACQ_TIME_MS * 1000.0 / WIND_SPEED_HZ_TURN);
  value = value * WIND_SPEED_MAX / WIND_SPEED_HZ_MAX *100.;
  return value;
}
#endif

#if (USE_SENSOR_DED)
void calibrationOffset(uint8_t count, uint8_t delay_ms, float *offset, float ideal) {
  float value = 0;

  for (uint8_t i = 0; i < count; i++) {
    value += ((float) windDirectionRead() - value) / (float) (i+1);
    SERIAL_INFO(F("%0.f\t"), value);
    delay(delay_ms);
  }

  *offset = ideal - getWindMv(value, 0);
  SERIAL_INFO(F("Wind Direction: ideal %f mV - read %f mV = offset %f mV\r\n"), ideal, getWindMv(value, 0), *offset);
}

void calibrationValue(uint8_t count, uint8_t delay_ms, float *val) {
  float value = 0;

  for (uint8_t i = 0; i < count; i++) {
    value += ((float) windDirectionRead() - value) / (float) (i+1);
    delay(delay_ms);
  }

  *val = getWindMv(value, 0);
}

float getWindMv (float adc_value, float offset_mv) {
  float value = (float) UINT16_MAX;

  value = ADC_VOLTAGE_MAX / ADC_MAX * adc_value;
  value = round(value / 10.0) * 10.0;
  value += offset_mv;

  return value;
}

float getWindDirection (float adc_value) {
  float value = getWindMv(adc_value, configuration.adc_voltage_offset_1);

  value = ((value - configuration.adc_voltage_min) / (configuration.adc_voltage_max - configuration.adc_voltage_min) * WIND_DIRECTION_MAX);

  if ((value <= WIND_DIRECTION_MIN) || (value >= WIND_DIRECTION_MAX)) {
    value = WIND_DIRECTION_MIN;
  }

  return round(value);
}
#endif

#if (USE_SENSOR_DED || USE_SENSOR_DES)
void wind_task () {
  static bool is_error;
  static wind_state_t state_after_wait;
  static uint32_t delay_ms;
  static uint32_t start_time_ms;
  static uint8_t i = 0;
  static float wind_direction;

  switch (wind_state) {
    case WIND_INIT:
      i = 0;
      is_error = false;
      #if (USE_SENSOR_DED)
      windDirectionRead();
      delay_ms = WIND_SETUP_DELAY_MS;
      start_time_ms = millis();
      state_after_wait = WIND_READING;
      wind_state = WIND_WAIT_STATE;
      LOGV(F("WIND_INIT --> WIND_READING"));
      #else
      wind_state = WIND_ELABORATE;
      LOGV(F("WIND_INIT --> WIND_ELABORATE"));
      #endif
    break;

    case WIND_READING:
      #if (USE_SENSOR_DED)
      wind_direction += ((float) windDirectionRead() - wind_direction) / (float) (i+1);

      if (i < WIND_READ_COUNT) {
        i++;
        delay_ms = WIND_READ_DELAY_MS;
        start_time_ms = millis();
        state_after_wait = WIND_READING;
        wind_state = WIND_WAIT_STATE;
      }
      else {
        wind_state = WIND_ELABORATE;
        LOGV(F("WIND_READING --> WIND_ELABORATE"));
      }
      #endif
    break;

    case WIND_ELABORATE:
      #if (USE_SENSOR_DED)
      wind_direction = getWindDirection(wind_direction);
      //bufferPtrResetBack<sample_t, uint16_t>(&wind_direction_samples, WMO_REPORT_SAMPLES_COUNT);
      //addValue<sample_t, uint16_t, uint16_t>(&wind_direction_samples, WMO_REPORT_SAMPLES_COUNT, wind_direction);
      cb_directionm.unshift(wind_direction);
      #endif

      #if (USE_SENSOR_DES)
      wind_speed = getWindSpeed(wind_speed);
      //bufferPtrResetBack<sample_t, uint16_t>(&wind_speed_samples, WMO_REPORT_SAMPLES_COUNT);
      //addValue<sample_t, uint16_t, uint16_t>(&wind_speed_samples, WMO_REPORT_SAMPLES_COUNT, wind_speed);
      cb_speed.unshift(wind_speed);
      #endif

      make_report();

      wind_state = WIND_END;
      LOGV(F("WIND_ELABORATE --> WIND_END"));
    break;

    case WIND_END:
      windPowerOff();
      noInterrupts();
      is_event_wind_task = false;
      ready_tasks_count--;
      interrupts();
      wind_state = WIND_INIT;
      LOGV(F("WIND_END --> WIND_INIT"));
    break;

    case WIND_WAIT_STATE:
      if (millis() - start_time_ms > delay_ms) {
        wind_state = state_after_wait;
      }
    break;
  }
}
#endif

#if (USE_SENSOR_GWS)
void wind_task () {
  static bool is_error;
  static uint8_t error_count=0;
  static wind_state_t state_after_wait;
  static uint32_t delay_ms;
  static uint32_t start_time_ms;
  uint16_t speed;
  uint16_t direction;

  switch (wind_state) {
    case WIND_INIT:
      is_error = false;
      serial1_reset();

      if (isWindOff()) {
        windPowerOn();
        delay_ms = WIND_POWER_ON_DELAY_MS;
        start_time_ms = millis();
        state_after_wait = WIND_READING;
        wind_state = WIND_WAIT_STATE;
        LOGV(F("WIND_READING --> WIND_READING"));
      } else {

	if(Serial1.available()){
	  flush();
	  is_error=true;
	}

	/*
	  When in the Polled mode, an output is only generated when the host system sends a Poll 
	  signal to the WindSonic consisting of the WindSonic Unit Identifier that is, the relevant 
	  letter A - Z.
	  The commands available in this mode are:
	  Description                       Command            WindSonic response
	  WindSonic Unit Identifier         A ..... Z          Wind speed output generated
	  Enable Polled mode                ?                  (None)
	  Disable Polled mode               !                  (None)
	  Request WindSonic Unit Identifier ?&                 A ..... Z (as configured)
	  Enter Configuration mode          *<N>               CONFIGURATION MODE
	  
	  Where <N> is the unit identifier, if used in a multidrop system then it is recommended that 
	  ID's A to F and KMNP are not used as these characters can be present in the data string.
	  
	  It is suggested that in polled mode the following sequence is used for every poll for 
	  information.
	  ? Ensures that the Sensor is enabled to cover the event that a power down has occurred.
	  A-Z Appropriate unit designator sent to retrieve a line of data.
	  ! Sent to disable poll mode and reduce possibility of erroneous poll generation.
	  
	  When in polled mode the system will respond to the data command within 130mS with the 
	  last valid data sample as calculated by the Output rate (P Mode Setting).
	*/
	
	Serial1.print("?Q!\n");
        delay_ms = WIND_POWER_RESPONSE_DELAY_MS;
        start_time_ms = millis();
        state_after_wait = WIND_READING;
        wind_state = WIND_WAIT_STATE;
        LOGV(F("WIND_READING --> WIND_READING"));
      }
    break;

    case WIND_READING:

      uart_rx_buffer_length = Serial1.readBytesUntil('\n',uart_rx_buffer, UART_RX_BUFFER_LENGTH);

      if (uart_rx_buffer_length > 0 && uart_rx_buffer_length < UART_RX_BUFFER_LENGTH ){
	uart_rx_buffer[uart_rx_buffer_length]='\n';
	uart_rx_buffer_length++;	
	for (uint8_t i = 0; i < uart_rx_buffer_length; i++) {
	  LOGV(F("windsonic data:%c"),uart_rx_buffer[i]);
	}
	wind_state = WIND_ELABORATE;
	LOGV(F("WIND_READING --> WIND_ELABORATE"));
      } else {
	LOGE(F("error reading windsonic data"));
        is_error = true;
        wind_state = WIND_ELABORATE;
        LOGV(F("WIND_READING --> WIND_ELABORATE"));
      }
    break;

    case WIND_ELABORATE:
      if (is_error) {
        speed = UINT16_MAX;
        direction = UINT16_MAX;
      }
      else {
        if (!windsonic_interpreter(&speed, &direction)){
	  is_error = true;
	}
      }

      LOGV(F("windsonic data: %d , %d"),speed,direction);
      
      //bufferPtrResetBack<sample_t, uint16_t>(&wind_direction_samples, WMO_REPORT_SAMPLES_COUNT);
      //addValue<sample_t, uint16_t, uint16_t>(&wind_speed_samples, WMO_REPORT_SAMPLES_COUNT, speed);
      //bufferPtrResetBack<sample_t, uint16_t>(&wind_direction_samples, WMO_REPORT_SAMPLES_COUNT);
      //addValue<sample_t, uint16_t, uint16_t>(&wind_direction_samples, WMO_REPORT_SAMPLES_COUNT, direction);

      cb_speed.unshift(speed);      
      cb_direction.unshift(direction);      
      
      make_report();

      wind_state = WIND_END;
      LOGV(F("WIND_ELABORATE --> WIND_END"));
    break;

    case WIND_END:
      if (is_error) {
	error_count++;
	if (error_count >= GWS_ERROR_COUNT_MAX){
	  error_count=0;
	  //ATTENTION here all is blocking!
	  windPowerOff();
	  delay(1000);
	  wdt_reset();
	  windPowerOn();
	  delay(WIND_POWER_ON_DELAY_MS);
	  wdt_reset();
	  Serial1.setTimeout(500);
	  configureWindsonic();
	  Serial1.setTimeout(GWS_SERIAL_TIMEOUT_MS);
	}
      }
      noInterrupts();
      is_event_wind_task = false;
      ready_tasks_count--;
      interrupts();
      wind_state = WIND_INIT;
      LOGV(F("WIND_END --> WIND_INIT"));
    break;

    case WIND_WAIT_STATE:
      if (millis() - start_time_ms > delay_ms) {
        wind_state = state_after_wait;
      }
    break;
  }
}

void serial1_reset() {
  while (Serial1.available()) {
     Serial1.read();
  }
  memset(uart_rx_buffer, 0, uart_rx_buffer_length);
  uart_rx_buffer_length = 0;
}

void receive_message(const char terminator){
  Serial1.readBytesUntil(terminator, uart_rx_buffer, UART_RX_BUFFER_LENGTH);
}

void flush(void){
  while (Serial1.available() > 0){
    Serial1.read();
  }
  wdt_reset();
}

void configureWindsonic(void){

  LOGN(F("configure Windsonic"));

  LOGV(F("enter configure mode"));
  Serial1.print("*Q");
  receive_message('\n');
  //CONFIGURATION MODE
  receive_message('\n');
  flush();

  LOGV(F("send: L1"));
  Serial1.print("L1\n");
  receive_message('\n');
  flush();
  
  LOGV(F("send: C2"));
  Serial1.print("C2\n");
  receive_message('\n');
  flush();
  
  LOGV(F("send: H2"));
  Serial1.print("H2\n");
  receive_message('\n');
  flush();

  LOGV(F("send: K50"));
  Serial1.print("K50\n");
  receive_message('\n');
  flush();

  LOGV(F("send: M4"));
  Serial1.print("M4\n");
  receive_message('\n');
  flush();

  LOGV(F("send: NQ"));
  Serial1.print("NQ\n");
  receive_message('\n');
  flush();

  LOGV(F("send: O1"));
  Serial1.print("O1\n");
  receive_message('\n');
  flush();

  LOGV(F("send: U1"));
  Serial1.print("U1\n");
  receive_message('\n');
  flush();

  LOGV(F("send: Y1"));
  Serial1.print("Y1\n");
  receive_message('\n');
  flush();

  /*  
  Serial1.print("D3\n");
  receive_message('\n');
  flush();

  Serial1.print("D5\n");
  receive_message('\n');
  flush();
  */

  LOGV(F("exit configure mode"));
  Serial1.print("Q\r\n");
  delay(10);
  Serial1.print("Q\r\n");
  flush();
  delay(1000);
  flush();  
}

bool windsonic_interpreter (uint16_t *speed, uint16_t *direction) {

  /* sample messages:
     Q,,000.03,M,00,2D
     Q,,000.04,M,00,2A
     Q,349,000.05,M,00,15
     Q,031,000.06,M,00,1A
     Q,103,000.06,M,00,1A
     

     Gill format Polar, Continuous (Default format)
     
     <STX>Q, 229, 002.74, M, 00, <ETX>16
     
     Where:
     <STX> = Start of string character (ASCII value 2)
     WindSonic node address = Unit identifier
     Wind direction = Wind Direction
     Wind speed = Wind Speed
     Units = Units of measure (knots, m/s etc)
     Status = Anemometer status code (see Section 11.5 for further details)
     <ETX> = End of string character (ASCII value 3)
     Checksum = This is the EXCLUSIVE OR of the bytes between (and not including) the <STX> and <ETX> characters.
     <CR> ASCII character
     <LF> ASCII characte
     
     The Status code is sent as part of each wind measurement message 
     Code  Status                 Condition
     00    OK                     Sufficient samples in average period
     01    Axis 1 failed          Insufficient samples in average period on U axis
     02    Axis 2 failed          Insufficient samples in average period on V axis
     04    Axis 1 and 2 failed    Insufficient samples in average period on both axes
     08    NVM error              NVM checksum failed
     09    ROM error              ROM checksum failed
     
  */
  
  #define GWS_STX_INDEX                                   (0)
  #define GWS_ETX_INDEX                                   (19)

  #define GWS_WITHOUT_DIRECTION_OFFSET                    (3)

  #define GWS_DIRECTION_INDEX                             (3)
  #define GWS_DIRECTION_LENGTH                            (3)
  #define GWS_SPEED_INDEX                                 (7)
  #define GWS_SPEED_LENGTH                                (6)

  #define GWS_STATUS_INDEX                                (16)
  #define GWS_STATUS_LENGTH                               (2)

  #define GWS_CRC_INDEX                                   (20)
  #define GWS_CRC_LENGTH                                  (2)
  #define STX_VALUE                                       (2)
  #define ETX_VALUE                                       (3)
  #define CR_VALUE                                        (13)
  #define LF_VALUE                                        (10)

  char tempstr[GWS_SPEED_LENGTH+1];
  char *tempstrptr;
  uint8_t myCrc = 0;
  uint8_t crc;

  *speed = UINT16_MAX;
  *direction = UINT16_MAX;

  uint8_t offset;

  if ((uart_rx_buffer[GWS_STX_INDEX] == STX_VALUE) &&
      (uart_rx_buffer[GWS_ETX_INDEX] == ETX_VALUE) &&
      (uart_rx_buffer[uart_rx_buffer_length-2] == CR_VALUE) &&
      (uart_rx_buffer[uart_rx_buffer_length-1] == LF_VALUE)) {

    LOGV(F("windsonic message with direction"));    
    offset = 0;

  } else if ((uart_rx_buffer[GWS_STX_INDEX] == STX_VALUE) &&
	     (uart_rx_buffer[GWS_ETX_INDEX - GWS_WITHOUT_DIRECTION_OFFSET] == ETX_VALUE) &&
	     (uart_rx_buffer[uart_rx_buffer_length-2] == CR_VALUE) &&
	     (uart_rx_buffer[uart_rx_buffer_length-1] == LF_VALUE)) {

    LOGV(F("windsonic message without direction"));    
    offset = GWS_WITHOUT_DIRECTION_OFFSET;

  }else{
    LOGE(F("error in windsonic message"));
    return false;
  }

  // check crc
  for (uint8_t i = GWS_STX_INDEX+1; i < (GWS_ETX_INDEX-offset); i++) {
    myCrc ^= uart_rx_buffer[i];
  }
  memset(tempstr, 0, GWS_SPEED_LENGTH+1);
  strncpy(tempstr, (const char *)(uart_rx_buffer+GWS_CRC_INDEX-offset), GWS_CRC_LENGTH);
  crc = (uint8_t) strtol(tempstr, &tempstrptr, 16);
  
  if (crc != myCrc){
    LOGE(F("CRC error in windsonic message"));
    return false;
  }
  
  // check status
  memset(tempstr, 0, GWS_SPEED_LENGTH+1);
  strncpy(tempstr, (const char *)(uart_rx_buffer+GWS_STATUS_INDEX-offset), GWS_STATUS_LENGTH);

  if (strncmp(tempstr,"01",2)==0){
    LOGV(F("Axis 1 failed: Insufficient samples in average period on U axis"));
  }
  if (strncmp(tempstr,"02",2)==0){
    LOGN(F("Axis 2 failed: Insufficient samples in average period on V axis"));
  }
  if (strncmp(tempstr,"04",2)==0){
    LOGN(F("Axis 1 and 2 failed: Insufficient samples in average period on both axes"));
  }
  if (strncmp(tempstr,"08",2)==0){
    LOGN(F("NVM error: NVM checksum failed"));
  }
  if (strncmp(tempstr,"09",2)==0){
    LOGN(F("ROM error: ROM checksum failed"));
  }

  //uint8_t status = (uint8_t) atoi(tempstr);
  int status =strncmp(tempstr,"00",2);
  if (status != 0) {
    LOGE(F("status error in windsonic message"));
    return false;
  }
  
  /*   check units
       Metres per second (default) M
       Knots                       N
       Miles per hour              P
       Kilometres per hour         K
       Feet per minute             F
  */
  memset(tempstr, 0, GWS_SPEED_LENGTH+1);
  strncpy(tempstr, (const char *)(uart_rx_buffer+GWS_SPEED_INDEX+GWS_SPEED_LENGTH+1-offset), 1);
  if (strncmp(tempstr,"M",1) != 0) {
    LOGE(F("units error in windsonic message"));
    return false;
  }

  if(offset == 0){
    memset(tempstr, 0, GWS_SPEED_LENGTH+1);
    strncpy(tempstr, (const char *)(uart_rx_buffer+GWS_DIRECTION_INDEX), GWS_DIRECTION_LENGTH);
    *direction = (uint16_t) atoi(tempstr);
  } else {
     *direction = WIND_DIRECTION_MIN;
  }
  
  memset(tempstr, 0, GWS_SPEED_LENGTH+1);
  strncpy(tempstr, (const char *)(uart_rx_buffer+GWS_SPEED_INDEX-offset), GWS_SPEED_LENGTH);
  *speed = (uint16_t) (atof(tempstr)*100.);


  // check with extreme values
  if (*direction < WIND_DIRECTION_MIN || *direction > WIND_DIRECTION_MAX) {
    *direction = UINT16_MAX;
  }
  if (*speed < WIND_SPEED_MIN*100 || *speed > WIND_SPEED_MAX*100) {
    *speed = UINT16_MAX;
  }

  /*
    Low Wind Speeds (below 0.05ms)
    Whilst the wind speed is below 0.05 metres/sec, the wind direction will not be calculated.
    In both CSV mode and in Fixed Field mode, Channel 2 wind direction output
    will freeze at
    the last known valid direction value until a new valid value can be calculated.
    The above applies with the K command set for K50. If K for instance is set for 100 then the
    above applies at 0.1m/s.
  */
  
  if (*speed < CALM_WIND_MAX_MS*100.) {
    *speed = WIND_SPEED_MIN*100.;                         // set calm when speed too low
  } else if (*speed > WIND_SPEED_MAX*100.) {
    *speed = UINT16_MAX;                                  // wind speed missed when too big
  }

  if (*direction == 0) *direction=360;                    // traslate 0 -> 360
  if (*speed == WIND_SPEED_MIN*100.) *direction=0;        // wind calm

  return true;
  
}
#endif

void exchange_buffers() {
  noInterrupts();
  readable_data_temp_ptr = readable_data_write_ptr;
  readable_data_write_ptr = readable_data_read_ptr;
  readable_data_read_ptr = readable_data_temp_ptr;
  interrupts();
}

void reset_samples_buffer() {
  #if (USE_SENSOR_DES)
  cb_speed.clear();


#endif

  #if (USE_SENSOR_DED)
  cb_direction.clear();
  #endif

  #if (USE_SENSOR_GWS)
  cb_speed.clear();
  cb_direction.clear();
  #endif
}

void reset_data(volatile readable_data_t *ptr) {
  ptr->wind.vavg10_speed = (float) UINT16_MAX;
  ptr->wind.vavg10_direction = (float) UINT16_MAX;
  ptr->wind.vavg_speed = (float) UINT16_MAX;
  ptr->wind.vavg_direction = (float) UINT16_MAX;
  ptr->wind.avg_speed = (float) UINT16_MAX;
  ptr->wind.peak_gust_speed = (float) UINT16_MAX;
  ptr->wind.peak_gust_direction = (float) UINT16_MAX;
  ptr->wind.long_gust_speed = (float) UINT16_MAX;
  ptr->wind.long_gust_direction = (float) UINT16_MAX;
  ptr->wind.class_1 = (float) UINT16_MAX;
  ptr->wind.class_2 = (float) UINT16_MAX;
  ptr->wind.class_3 = (float) UINT16_MAX;
  ptr->wind.class_4 = (float) UINT16_MAX;
  ptr->wind.class_5 = (float) UINT16_MAX;
  ptr->wind.class_6 = (float) UINT16_MAX;
}

void command_task() {

  switch(lastcommand) {
  case I2C_WIND_COMMAND_ONESHOT_START:
    LOGN(F("Execute [ ONESHOT START ]"));
    is_start = true;
    is_stop = false;
    is_test = false;
    commands();
    break;
    
  case I2C_WIND_COMMAND_ONESHOT_STOP:
    LOGN(F("Execute [ ONESHOT STOP ]"));
    is_start = false;
    is_stop = true;
    is_test = false;
    commands();
    inside_transaction = true;
    break;
    
  case I2C_WIND_COMMAND_ONESHOT_START_STOP:
    LOGN(F("Execute [ ONESHOT START-STOP ]"));
    is_start = true;
    is_stop = true;
    is_test = false;
    commands();
    inside_transaction = true;
    break;
    
  case I2C_WIND_COMMAND_CONTINUOUS_START:
    LOGN(F("Execute [ CONTINUOUS START ]"));
    is_start = true;
    is_stop = false;
    is_test = false;
    commands();
    break;
    
  case I2C_WIND_COMMAND_CONTINUOUS_STOP:
    LOGN(F("Execute [ CONTINUOUS STOP ]"));
    is_start = false;
    is_stop = true;
    is_test = false;
    commands();
    inside_transaction = true;
    break;
	 
  case I2C_WIND_COMMAND_CONTINUOUS_START_STOP:
    LOGN(F("Execute [ CONTINUOUS START-STOP ]"));
    is_start = true;
    is_stop = true;
    is_test = false;
    commands();
    inside_transaction = true;
    break;
    
  case I2C_WIND_COMMAND_TEST_READ:
    LOGN(F("Execute [ TEST READ ]"));
    //is_start = true;
    is_stop = false;
    is_test = true;
    commands();
    break;
    
  case I2C_WIND_COMMAND_SAVE:
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
  lastcommand =I2C_WIND_COMMAND_NONE;
  interrupts();
}

void copy_buffers() {
   //! copy readable_data_2 in readable_data_1
   noInterrupts();
   memcpy((void *) readable_data_read_ptr, (const void*) readable_data_write_ptr, sizeof(readable_data_t));
   interrupts();
}

void commands() {
  //! CONTINUOUS START
  if (inside_transaction) return;
  
  //! CONTINUOUS TEST
  if (!configuration.is_oneshot && is_start && !is_stop && is_test) {
    copy_buffers();
    //exchange_buffers();
  }
  //! CONTINUOUS START
  else if (!configuration.is_oneshot && is_start && !is_stop && !is_test) {

    stop_timer();
    reset_samples_buffer();
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
    make_report(true);
    start_timer();
  }
  //! ONESHOT START
  else if (configuration.is_oneshot && is_start && !is_stop) {
    reset_samples_buffer();
    //! ONESHOT STOP
    noInterrupts();
    if (!is_event_wind_task) {
      is_event_wind_task = true;
      ready_tasks_count++;
    }
    interrupts();
  }
}
  
