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

#include "i2c-radiation.h"

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

      #if (USE_SENSOR_DSR)
      if (is_event_solar_radiation_task) {
        solar_radiation_task();
        wdt_reset();
      }
      #endif

      #if (USE_SENSOR_VSR)
      if (is_event_solar_radiation_task) {
        solar_radiation_task_hr();
        wdt_reset();
      }
      #endif

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
  memset((void *) &readable_data_read_ptr->solar_radiation, UINT8_MAX, sizeof(data_t));
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
  is_event_solar_radiation_task = false;

  #if (USE_SENSOR_DSR)
  solar_radiation_state = SOLAR_RADIATION_INIT;
  #endif

  #if (USE_SENSOR_VSR)
  solar_radiation_hr_state = SOLAR_RADIATION_HR_INIT;
  #endif

  lastcommand=I2C_SOLAR_RADIATION_COMMAND_NONE;
  is_start = false;
  is_stop = false;
  is_test_read = false;
  transaction_time = 0;
  inside_transaction = false;
}

void init_pins() {
  pinMode(CONFIGURATION_RESET_PIN, INPUT_PULLUP);
  pinMode(SDCARD_CHIP_SELECT_PIN, OUTPUT);

  #if (USE_SENSOR_DSR)
  pinMode(SOLAR_RADIATION_POWER_PIN, OUTPUT);
  pinMode(SOLAR_RADIATION_ANALOG_PIN, INPUT);
  solarRadiationPowerOn();
  #endif

  #if (USE_SENSOR_VSR)
  pinMode(SOLAR_RADIATION_POWER_PIN, OUTPUT);
  solarRadiationPowerOn();
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
  
  
  //! check if SENSORS_SAMPLE_TIME_MS ms have passed since last time. if true and if is in continuous mode and continuous start command It has been received, activate Sensor RADIATION task
  #if (USE_SENSOR_DSR || USE_SENSOR_VSR)
  if (executeTimerTaskEach(timer_counter_ms, SENSORS_SAMPLE_TIME_MS, TIMER1_INTERRUPT_TIME_MS) && !configuration.is_oneshot) {
    if (!is_event_solar_radiation_task) {
      is_event_solar_radiation_task = true;
      ready_tasks_count++;
    }
  }
  #endif
  
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

  #if (USE_SENSOR_DSR)
  LOGN(F("--> adc voltage offset +: %0"), configuration.adc_voltage_offset_1);
  LOGN(F("--> adc voltage offset *: %3"), configuration.adc_voltage_offset_2);
  LOGN(F("--> adc voltage min: %0 mV"), configuration.adc_voltage_min);
  LOGN(F("--> adc voltage max: %0 mV"), configuration.adc_voltage_max);
  #endif

  #if (USE_SENSOR_VSR)
  LOGN(F("--> AINx\toffset\tgain\tradmax\tvoltagemax"));

  for (uint8_t i = 0; i < ADS1115_CHANNEL_COUNT; i++) {
    LOGN(F("--> AIN%d\t%D\t%D\t%D\t%D"), i, configuration.adc_calibration_offset[i], configuration.adc_calibration_gain[i], configuration.sensor_rad_max[i], configuration.sensor_voltage_max[i]);
  }
  #endif
}

void save_configuration(bool is_default) {
  if (is_default) {
    LOGN(F("Save default configuration... [ %s ]"), OK_STRING);
    configuration.module_type = MODULE_TYPE;
    configuration.module_main_version = MODULE_MAIN_VERSION;
    configuration.module_configuration_version = MODULE_CONFIGURATION_VERSION;
    configuration.i2c_address = CONFIGURATION_DEFAULT_I2C_ADDRESS;
    configuration.is_oneshot = CONFIGURATION_DEFAULT_ONESHOT;

    configuration.adc_voltage_offset_1 = CONFIGURATION_DEFAULT_ADC_VOLTAGE_OFFSET_1;
    configuration.adc_voltage_offset_2 = CONFIGURATION_DEFAULT_ADC_VOLTAGE_OFFSET_2;
    configuration.adc_voltage_min = CONFIGURATION_DEFAULT_ADC_VOLTAGE_MIN;
    configuration.adc_voltage_max = CONFIGURATION_DEFAULT_ADC_VOLTAGE_MAX;

    /*
    #if (USE_SENSOR_DSR)
    solarRadiationPowerOn();
    delay(SOLAR_RADIATION_READ_DELAY_MS);
    //warning: large integer implicitly truncated to unsigned type [-Woverflow]
    solaRadiationOffset(SOLAR_RADIATION_READ_COUNT, SOLAR_RADIATION_READ_DELAY_MS, &configuration.adc_voltage_offset_1, configuration.adc_voltage_min);
    solarRadiationPowerOff();
    #endif
    */
    
    #if (USE_SENSOR_VSR)
    for (uint8_t i = 0; i < ADS1115_CHANNEL_COUNT; i++) {
      configuration.adc_calibration_offset[i] = CONFIGURATION_DEFAULT_ADC_VOLTAGE_OFFSET;
      configuration.adc_calibration_gain[i] =   CONFIGURATION_DEFAULT_ADC_VOLTAGE_GAIN;
      configuration.sensor_rad_max[i] = CONFIGURATION_DEFAULT_SENSOR_RADIATION_MAX;
      configuration.sensor_voltage_max[i] = CONFIGURATION_DEFAULT_SENSOR_VOLTAGE_MAX;
      // }
    }
    #endif
  }
  else {
    LOGN(F("Save configuration... [ %s ]"), OK_STRING);
    configuration.i2c_address = writable_data.i2c_address;
    configuration.is_oneshot = writable_data.is_oneshot;

    #if (USE_SENSOR_DSR)
    configuration.adc_voltage_offset_1 = writable_data.adc_voltage_offset_1;
    configuration.adc_voltage_offset_2 = writable_data.adc_voltage_offset_2;
    configuration.adc_voltage_min = writable_data.adc_voltage_min;
    configuration.adc_voltage_max = writable_data.adc_voltage_max;
    #endif

    #if (USE_SENSOR_VSR)
    for (uint8_t i = 0; i < ADS1115_CHANNEL_COUNT; i++) {
      configuration.adc_calibration_offset[i] = writable_data.adc_calibration_offset[i];
      configuration.adc_calibration_gain[i] = writable_data.adc_calibration_gain[i];
      configuration.sensor_rad_max[i] = writable_data.sensor_rad_max[i];
      configuration.sensor_voltage_max[i] = writable_data.sensor_voltage_max[i];
    }
    #endif
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

  #if (USE_SENSOR_DSR)
  writable_data.adc_voltage_offset_1 = configuration.adc_voltage_offset_1;
  writable_data.adc_voltage_offset_2 = configuration.adc_voltage_offset_2;
  writable_data.adc_voltage_min      = configuration.adc_voltage_min;
  writable_data.adc_voltage_max      = configuration.adc_voltage_max;
  #endif

  #if (USE_SENSOR_VSR)
  for (uint8_t i = 0; i < ADS1115_CHANNEL_COUNT; i++) {
    writable_data.adc_calibration_offset[i] = configuration.adc_calibration_offset[i];
    writable_data.adc_calibration_gain[i]   = configuration.adc_calibration_gain[i];
    writable_data.sensor_rad_max[i]         = configuration.sensor_rad_max[i];
    writable_data.sensor_voltage_max[i]     = configuration.sensor_voltage_max[i];
  }
  #endif
  
}

void init_sensors () {

  if (!configuration.is_oneshot) {

    #if (USE_SENSOR_DSR)
    LOGN(F("sc: sample count"));
    LOGN(F("rad: solar radiation [ W/m2 ]"));
    LOGN(F("avg: average solar radiation over %l' [ W/m2 ]"), STATISTICAL_DATA_COUNT * OBSERVATIONS_MINUTES);

    LOGN(F("sc\trad\tavg"));
    #endif
  }
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

      if (i2c_rx_data[0] == I2C_SOLAR_RADIATION_ADDRESS_ADDRESS && rx_data_length == I2C_SOLAR_RADIATION_ADDRESS_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_SOLAR_RADIATION_ONESHOT_ADDRESS && rx_data_length == I2C_SOLAR_RADIATION_ONESHOT_LENGTH) {
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
    samples_count=0;
    samples_error_count=0;
    sample=INT16_MAX;
    average=INT16_MAX;
  }else{

    if (ISVALID_INT16(sample)){    
      samples_count++;
      if (samples_count == 1) average=sample;
      average += round((float(sample) - float(average)) / float(samples_count));
    }else{
      samples_error_count++;
    }
  }
  LOGN("samples_count: %d; error_count: %d; sample: %d; average: %d",samples_count,samples_error_count,sample,average);
}


#if (USE_SENSOR_DSR || USE_SENSOR_VSR)
void solarRadiationPowerOff () {
  digitalWrite(SOLAR_RADIATION_POWER_PIN, LOW);
  is_solar_radiation_on = false;
}

void solarRadiationPowerOn () {
  digitalWrite(SOLAR_RADIATION_POWER_PIN, HIGH);
  is_solar_radiation_on = true;
}
#endif

#if (USE_SENSOR_DSR)
float getSolarRadiationMv (float adc_value, float offset_mv) {
  float value = (float) INT16_MAX;

  if ((adc_value >= ADC_MIN) && (adc_value <= ADC_MAX)) {
    value = ADC_VOLTAGE_MAX / ADC_MAX * adc_value;
    //value = round(value / 10.0) * 10.0;                    // perchè arrotondare alle decine?
    value += offset_mv;
  }

  return value;
}

float getSolarRadiation (float adc_value) {
  float value = getSolarRadiationMv(adc_value, configuration.adc_voltage_offset_1);

  if ((value < (configuration.adc_voltage_min - SOLAR_RADIATION_ERROR_VOLTAGE_MIN)) || (value > (configuration.adc_voltage_max + SOLAR_RADIATION_ERROR_VOLTAGE_MAX))) {
    value = (float) INT16_MAX;
  }
  else {
  value = ((value - configuration.adc_voltage_min) / (configuration.adc_voltage_max - configuration.adc_voltage_min) * SOLAR_RADIATION_MAX);
  }

  return value;
}

void solaRadiationOffset(uint8_t count, uint8_t delay_ms, float *offset, float ideal) {
  float value = 0;

  for (uint8_t i = 0; i < count; i++) {
    value += ((float) solarRadiationRead() - value) / (float) (i+1);
    delay(delay_ms);
  }

  *offset = ideal - getSolarRadiationMv(value, 0);

  LOGN(F("Solar Radiation: ideal %0 mV - read %0 mV = offset %0 mV"), ideal, getSolarRadiationMv(value, 0), *offset);
}

void solar_radiation_task () {
  static solar_radiation_state_t state_after_wait;
  static uint32_t delay_ms;
  static uint32_t start_time_ms;
  static float solar_radiation;

  switch (solar_radiation_state) {
    case SOLAR_RADIATION_INIT:
      solarRadiationPowerOn();
      solarRadiationRead();
      delay_ms = SOLAR_RADIATION_READ_DELAY_MS;
      start_time_ms = millis();
      state_after_wait = SOLAR_RADIATION_READING;
      solar_radiation_state = SOLAR_RADIATION_WAIT_STATE;
      LOGV(F("SOLAR_RADIATION_INIT --> SOLAR_RADIATION_READING"));
    break;

    case SOLAR_RADIATION_READING:
      solar_radiation = solarRadiationRead();

      solar_radiation_state = SOLAR_RADIATION_ELABORATE;
      LOGV(F("SOLAR_RADIATION_READING --> SOLAR_RADIATION_ELABORATE"));

    break;

    case SOLAR_RADIATION_ELABORATE:
      
      sample = round(getSolarRadiation(solar_radiation));
      make_report();

      readable_data_write_ptr->solar_radiation.sample = sample;

      if (is_start && samples_count > ((RMAP_REPORT_SAMPLE_ERROR_MAX_PERC*1000)/SENSORS_SAMPLE_TIME_MS)){
	if((float(samples_error_count) / float(samples_count) *100) <= RMAP_REPORT_SAMPLE_ERROR_MAX_PERC){ 
	  readable_data_write_ptr->solar_radiation.avg = average;
	}else{
	  LOGE(F("REPORT_SAMPLE_ERROR_MAX_PERC error good: %d ; bad: %d"), samples_count,samples_error_count);
	  readable_data_write_ptr->solar_radiation.avg = INT16_MAX;	  
	}
      }

      solar_radiation_state = SOLAR_RADIATION_END;
      LOGV(F("SOLAR_RADIATION_ELABORATE --> SOLAR_RADIATION_END"));
    break;

    case SOLAR_RADIATION_END:
      solarRadiationPowerOff();
      noInterrupts();
      is_event_solar_radiation_task = false;
      ready_tasks_count--;
      interrupts();
      solar_radiation_state = SOLAR_RADIATION_INIT;
      LOGV(F("SOLAR_RADIATION_END --> SOLAR_RADIATION_INIT"));
    break;

    case SOLAR_RADIATION_WAIT_STATE:
      if (millis() - start_time_ms > delay_ms) {
        solar_radiation_state = state_after_wait;
      }
    break;
  }
}
#endif

#if (USE_SENSOR_VSR)
float getAdcCalibratedValue (float adc_value, float offset, float gain) {
  float value = (float) INT16_MAX;

  if (!isnan(adc_value) && (adc_value >= ADC_MIN) && (adc_value <= ADC_MAX)) {
    value = adc_value;
    value += offset;
    value *= gain;
  }

  return value;
}

float getAdcAnalogValue (float adc_value, float min, float max) {
  float value = (float) INT16_MAX;

  value = adc_value;
  value *= (ADC_VOLTAGE_MAX - ADC_VOLTAGE_MIN) / (float(ADC_MAX)-float(ADC_MIN));
  value += ADC_VOLTAGE_OFFSET;

  return value;
}

float getSolarRadiation (float adc_value, float sensor_voltage_max, float sensor_radiation_max) {
  float value = adc_value;

  if ((value < (SOLAR_RADIATION_SENSOR_VOLTAGE_MIN - SOLAR_RADIATION_ERROR_VOLTAGE_MIN)) || (value > (SOLAR_RADIATION_SENSOR_VOLTAGE_MAX + SOLAR_RADIATION_ERROR_VOLTAGE_MAX))) {
    value = (float) INT16_MAX;
  }
  else {
    value = ((value - SOLAR_RADIATION_SENSOR_VOLTAGE_MIN) / ( sensor_voltage_max - SOLAR_RADIATION_SENSOR_VOLTAGE_MIN)) * (sensor_radiation_max-SOLAR_RADIATION_SENSOR_RADIATION_MIN);
  }

  return value;
}

void solar_radiation_task_hr () {
  static solar_radiation_hr_state_t state_after_wait;
  static uint32_t delay_ms;
  static uint32_t start_time_ms;
  static int16_t adc_value;
  static bool is_error;
  static float value;
  adc_result_t adc_result;


  switch (solar_radiation_hr_state) {
    case SOLAR_RADIATION_HR_INIT:
      solarRadiationPowerOn();
      is_error = false;

      delay_ms = SOLAR_RADIATION_READ_DELAY_MS;
      start_time_ms = millis();
      state_after_wait = SOLAR_RADIATION_HR_READ;
      solar_radiation_hr_state = SOLAR_RADIATION_HR_WAIT_STATE;
      LOGV(F("SOLAR_RADIATION_HR_INIT --> SOLAR_RADIATION_HR_READ"));
    break;

    case SOLAR_RADIATION_HR_READ:
      adc_result = adc1.readSingleChannel(SOLAR_RADIATION_ADC_CHANNEL_INPUT, &adc_value);

      if (adc_result == ADC_OK) {
	LOGN("adc_value: %d",adc_value);	
        value = (float) adc_value;
	solar_radiation_hr_state = SOLAR_RADIATION_HR_EVALUATE;
	LOGV(F("SOLAR_RADIATION_HR_READ --> SOLAR_RADIATION_HR_EVALUATE"));
      }
      else if (adc_result == ADC_ERROR) {
	LOGE("ADC readSingleChannel error");
        i2c_error++;
        value = (float) INT16_MAX;
        is_error = true;
	solar_radiation_hr_state = SOLAR_RADIATION_HR_EVALUATE;
	LOGV(F("SOLAR_RADIATION_HR_READ --> SOLAR_RADIATION_HR_EVALUATE"));
      } else if (adc_result == ADC_BUSY) {
	LOGV("ADC readSingleChannel busy");
      }
      
    break;

    case SOLAR_RADIATION_HR_EVALUATE:
      LOGN(F("Calibrated channel %d ==> (%D + %D) * %D = "), SOLAR_RADIATION_ADC_CHANNEL_INPUT, value
	   , configuration.adc_calibration_offset[SOLAR_RADIATION_ADC_CHANNEL_INPUT]
	   , configuration.adc_calibration_gain[SOLAR_RADIATION_ADC_CHANNEL_INPUT]);

      if (!is_error) {
        value = getAdcCalibratedValue(value
				  , configuration.adc_calibration_offset[SOLAR_RADIATION_ADC_CHANNEL_INPUT]
				  , configuration.adc_calibration_gain[SOLAR_RADIATION_ADC_CHANNEL_INPUT]);
	LOGN(F("%D"), value);

	if (value != float(INT16_MAX)){
	  value = getAdcAnalogValue(value, ADC_VOLTAGE_MIN, ADC_VOLTAGE_MAX);
	  LOGN(F("voltage: %D"), value);
	  LOGN(F("rad     max: %D"), configuration.sensor_rad_max[SOLAR_RADIATION_ADC_CHANNEL_INPUT]);
	  LOGN(F("voltage max: %D"), configuration.sensor_voltage_max[SOLAR_RADIATION_ADC_CHANNEL_INPUT]);
	  
	  if (value != float(INT16_MAX)){
	    value = getSolarRadiation(value
				      , configuration.sensor_rad_max[SOLAR_RADIATION_ADC_CHANNEL_INPUT]
				      , configuration.sensor_voltage_max[SOLAR_RADIATION_ADC_CHANNEL_INPUT]);
	    LOGN(F("radiation: %D w/m^2"), value);
	  }
	}
	sample=round(value);

      } else {
	sample=INT16_MAX;
      }

      readable_data_write_ptr->solar_radiation.sample = sample;
      LOGN(F("%D [ %s ]"), value, is_error ? ERROR_STRING : OK_STRING);

      
      solar_radiation_hr_state = SOLAR_RADIATION_HR_PROCESS;
      LOGV(F("SOLAR_RADIATION_HR_EVALUATE --> SOLAR_RADIATION_HR_PROCESS"));
    break;

    case SOLAR_RADIATION_HR_PROCESS:

      make_report();
      if (is_start && samples_count > ((RMAP_REPORT_SAMPLE_MIN_TIME*1000Lu)/SENSORS_SAMPLE_TIME_MS)){
	if((float(samples_error_count) / float(samples_count) *100) <= RMAP_REPORT_SAMPLE_ERROR_MAX_PERC){ 
	  readable_data_write_ptr->solar_radiation.avg = average;
	}else{
	  LOGE(F("REPORT_SAMPLE_ERROR_MAX_PERC error good: %d ; bad: %d"), samples_count,samples_error_count);
	  readable_data_write_ptr->solar_radiation.avg = INT16_MAX;	  
	}
      }
             
      solar_radiation_hr_state = SOLAR_RADIATION_HR_END;
      LOGV(F("SOLAR_RADIATION_HR_PROCESS --> SOLAR_RADIATION_HR_END"));
    break;

    case SOLAR_RADIATION_HR_END:

      noInterrupts();
      is_event_solar_radiation_task = false;
      ready_tasks_count--;
      interrupts();
      solar_radiation_hr_state = SOLAR_RADIATION_HR_INIT;
      LOGV(F("SOLAR_RADIATION_HR_END --> SOLAR_RADIATION_HR_INIT"));
    break;

    case SOLAR_RADIATION_HR_WAIT_STATE:
      if (millis() - start_time_ms > delay_ms) {
        solar_radiation_hr_state = state_after_wait;
      }
    break;
  }
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
  samples_count=0;
  sample=INT16_MAX;
}

void reset_data(volatile readable_data_t *ptr) {
  ptr->solar_radiation.sample = INT16_MAX;
  ptr->solar_radiation.avg = INT16_MAX;
}

void command_task() {
  switch(lastcommand) {
    case I2C_SOLAR_RADIATION_COMMAND_ONESHOT_START:
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

    case I2C_SOLAR_RADIATION_COMMAND_ONESHOT_STOP:
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

    case I2C_SOLAR_RADIATION_COMMAND_ONESHOT_START_STOP:
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

    case I2C_SOLAR_RADIATION_COMMAND_CONTINUOUS_START:
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

    case I2C_SOLAR_RADIATION_COMMAND_CONTINUOUS_STOP:
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

    case I2C_SOLAR_RADIATION_COMMAND_CONTINUOUS_START_STOP:
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

    case I2C_SOLAR_RADIATION_COMMAND_TEST_READ:
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

    case I2C_SOLAR_RADIATION_COMMAND_SAVE:
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
  lastcommand=I2C_SOLAR_RADIATION_COMMAND_NONE;
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
    if (!is_event_solar_radiation_task) {
      is_event_solar_radiation_task = true;
      ready_tasks_count++;
    }
    interrupts();
  }
  //! ONESHOT STOP
  else if (configuration.is_oneshot && !is_start && is_stop) {

    readable_data_write_ptr->solar_radiation.sample = sample;
    exchange_buffers();
  }
  //! ONESHOT START-STOP
  else if (configuration.is_oneshot && is_start && is_stop) {
   
    readable_data_write_ptr->solar_radiation.sample = sample;
    exchange_buffers();
    
    noInterrupts();
    if (!is_event_solar_radiation_task) {
      is_event_solar_radiation_task = true;
      ready_tasks_count++;
    }
    interrupts();
  }
}



