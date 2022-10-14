/**@file i2c-radiation.ino */

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
#define SERIAL_TRACE_LEVEL I2C_RADIATION_SERIAL_TRACE_LEVEL

#include "i2c-radiation.h"

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
      // I2C Bus Check
      if (i2c_error >= I2C_MAX_ERROR_COUNT) {
        SERIAL_DEBUG(F("Restart I2C BUS\r\n"));
        init_wire();
        wdt_reset();
      }

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

      // if (digitalRead(CONFIGURATION_RESET_PIN) == LOW) {
      //   save_configuration(CONFIGURATION_CURRENT);
      // }

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

    //! turn off brown-out
    // MCUCR = bit (BODS) | bit (BODSE);
    // MCUCR = bit (BODS);
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
  is_event_solar_radiation_task = false;

  #if (USE_SENSOR_DSR)
  solar_radiation_state = SOLAR_RADIATION_INIT;
  #endif

  #if (USE_SENSOR_VSR)
  solar_radiation_hr_state = SOLAR_RADIATION_HR_INIT;
  #endif

  solar_radiation_acquisition_count = 0;

  is_oneshot = false;
  is_continuous = false;
  is_start = false;
  is_stop = false;
  is_test = false;

  interrupts();
}

void init_pins() {
  pinMode(CONFIGURATION_RESET_PIN, INPUT_PULLUP);

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
  if (i2c_error > 0) {
    i2c_error = 0;
  }

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
  SERIAL_INFO(F("--> type: %s\r\n"), stima_name);
  SERIAL_INFO(F("--> version: %d\r\n"), configuration.module_version);
  SERIAL_INFO(F("--> i2c address: 0x%X (%d)\r\n"), configuration.i2c_address, configuration.i2c_address);
  SERIAL_INFO(F("--> oneshot: %s\r\n"), configuration.is_oneshot ? ON_STRING : OFF_STRING);
  SERIAL_INFO(F("--> continuous: %s\r\n"), configuration.is_continuous ? ON_STRING : OFF_STRING);

  #if (USE_SENSOR_DSR)
  SERIAL_INFO(F("--> adc voltage offset +: %.0f\r\n"), configuration.adc_voltage_offset_1);
  SERIAL_INFO(F("--> adc voltage offset *: %.3f\r\n"), configuration.adc_voltage_offset_2);
  SERIAL_INFO(F("--> adc voltage min: %.0f mV\r\n"), configuration.adc_voltage_min);
  SERIAL_INFO(F("--> adc voltage max: %.0f mV\r\n"), configuration.adc_voltage_max);
  #endif

  #if (USE_SENSOR_VSR)
  SERIAL_INFO(F("--> ADC i\tAINx\toffset\t\tgain\t\tmin\t\tmax\r\n\r\n"));

  for (uint8_t j = 0; j < ADC_COUNT; j++) {
    for (uint8_t i = 0; i < ADS1115_CHANNEL_COUNT; i++) {
      SERIAL_INFO(F("--> ADC %u\tAIN%u\t%f\t%f\t%f\t%f\r\n"), j+1, i, configuration.adc_calibration_offset[j][i], configuration.adc_calibration_gain[j][i], configuration.adc_analog_min[j][i], configuration.adc_analog_max[j][i]);
    }
    SERIAL_INFO_CLEAN(F("\r\n"));
  }
  #endif
}

void save_configuration(bool is_default) {
  if (is_default) {
    SERIAL_INFO(F("Save default configuration... [ %s ]\r\n"), OK_STRING);
    configuration.module_type = MODULE_TYPE;
    configuration.module_version = MODULE_VERSION;
    configuration.i2c_address = CONFIGURATION_DEFAULT_I2C_ADDRESS;
    configuration.is_oneshot = CONFIGURATION_DEFAULT_IS_ONESHOT;
    configuration.is_continuous = CONFIGURATION_DEFAULT_IS_CONTINUOUS;

    configuration.adc_voltage_offset_1 = CONFIGURATION_DEFAULT_ADC_VOLTAGE_OFFSET_1;
    configuration.adc_voltage_offset_2 = CONFIGURATION_DEFAULT_ADC_VOLTAGE_OFFSET_2;
    configuration.adc_voltage_min = CONFIGURATION_DEFAULT_ADC_VOLTAGE_MIN;
    configuration.adc_voltage_max = CONFIGURATION_DEFAULT_ADC_VOLTAGE_MAX;

    #if (USE_SENSOR_DSR)
    solarRadiationPowerOn();
    delay(SOLAR_RADIATION_READ_DELAY_MS);
    solaRadiationOffset(SOLAR_RADIATION_READ_COUNT, SOLAR_RADIATION_READ_DELAY_MS, &configuration.adc_voltage_offset_1, configuration.adc_voltage_min);
    solarRadiationPowerOff();
    #endif

    #if (USE_SENSOR_VSR)
    for (uint8_t j = 0; j < ADC_COUNT; j++) {
      for (uint8_t i = 0; i < ADS1115_CHANNEL_COUNT; i++) {
        // if ((j == 0) && (i == 0)) {
        //   configuration.adc_calibration_offset[j][i] = -1750.0;
        //   configuration.adc_calibration_gain[j][i] = 4.6300;
        //   configuration.adc_analog_min[j][i] = 4.0;
        //   configuration.adc_analog_max[j][i] = 20.0;
        // }
        // else {
          configuration.adc_calibration_offset[j][i] = -1.0;
          configuration.adc_calibration_gain[j][i] = 1.2355;
          configuration.adc_analog_min[j][i] = 0.0;
          configuration.adc_analog_max[j][i] = 5000.0;
        // }
      }
    }
    #endif
  }
  else {
    SERIAL_INFO(F("Save configuration... [ %s ]\r\n"), OK_STRING);
    configuration.i2c_address = writable_data.i2c_address;
    configuration.is_oneshot = writable_data.is_oneshot;
    configuration.is_continuous = writable_data.is_continuous;

    #if (USE_SENSOR_DSR)
    configuration.adc_voltage_offset_1 = writable_data.adc_voltage_offset_1;
    configuration.adc_voltage_offset_2 = writable_data.adc_voltage_offset_2;
    configuration.adc_voltage_min = writable_data.adc_voltage_min;
    configuration.adc_voltage_max = writable_data.adc_voltage_max;
    #endif

    #if (USE_SENSOR_VSR)
    for (uint8_t j = 0; j < ADC_COUNT; j++) {
      for (uint8_t i = 0; i < ADS1115_CHANNEL_COUNT; i++) {
        configuration.adc_calibration_offset[j][i] = writable_data.adc_calibration_offset[j][i];
        configuration.adc_calibration_gain[j][i] = writable_data.adc_calibration_gain[j][i];
        configuration.adc_analog_min[j][i] = writable_data.adc_analog_min[j][i];
        configuration.adc_analog_max[j][i] = writable_data.adc_analog_max[j][i];
      }
    }
    #endif
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
  writable_data.is_continuous = configuration.is_continuous;
  writable_data.adc_voltage_offset_1 = configuration.adc_voltage_offset_1;
  writable_data.adc_voltage_offset_2 = configuration.adc_voltage_offset_2;
  writable_data.adc_voltage_min = configuration.adc_voltage_min;
  writable_data.adc_voltage_max = configuration.adc_voltage_max;
}

void init_sensors () {
  #if (USE_SENSOR_VSR)
  for (uint8_t j = 0; j < ADC_COUNT; j++) {
    for (uint8_t i = 0; i < ADS1115_CHANNEL_COUNT; i++) {
      use_adc_channel[j][i] = ADS1115_CHN_OFF_ID;
    }
  }

  use_adc_channel[SOLAR_RADIATION_ADC_INDEX][SOLAR_RADIATION_ADC_CHANNEL_INPUT] = ADS1115_VOLTAGE_ID;
  #endif

  if (configuration.is_continuous) {
    SERIAL_INFO(F("\r\n"));
    SERIAL_INFO(F("--> acquiring %u~%u samples in %u minutes\r\n"), OBSERVATION_SAMPLES_COUNT_MIN, OBSERVATION_SAMPLES_COUNT_MAX, OBSERVATIONS_MINUTES);
    SERIAL_INFO(F("--> max %u samples error in %u minutes (observation)\r\n"), OBSERVATION_SAMPLE_ERROR_MAX, OBSERVATIONS_MINUTES);
    SERIAL_INFO(F("--> max %u samples error in %u minutes (report)\r\n\r\n"), RMAP_REPORT_SAMPLE_ERROR_MAX, STATISTICAL_DATA_COUNT * OBSERVATIONS_MINUTES);

    #if (USE_SENSOR_DSR || USE_SENSOR_VSR)
    SERIAL_INFO(F("sc: sample count\r\n"));
    SERIAL_INFO(F("rad: solar radiation [ W/m2 ]\r\n"));
    SERIAL_INFO(F("avg: average solar radiation over %u' [ W/m2 ]\r\n"), STATISTICAL_DATA_COUNT * OBSERVATIONS_MINUTES);

    SERIAL_INFO(F("sc\trad\tavg\r\n\r\n"));
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

  //! increment timer_counter_ms by TIMER1_INTERRUPT_TIME_MS
  timer_counter_ms += TIMER1_INTERRUPT_TIME_MS;
  timer_counter_s += (uint16_t)(TIMER1_INTERRUPT_TIME_MS/1000);

  // if ((timer_counter_s >= I2C_DELAY_FOR_SET_I2C_ERROR_S) && (i2c_error < (UINT8_MAX - I2C_SET_ERROR_COUNT))) {
  if (i2c_error < (UINT8_MAX - I2C_SET_ERROR_COUNT)) {
    i2c_error++;
  }

  //! check if SENSORS_SAMPLE_TIME_MS ms have passed since last time. if true and if is in continuous mode and continuous start command It has been received, activate Sensor RADIATION task
  #if (USE_SENSOR_DSR || USE_SENSOR_VSR)
  if (executeTimerTaskEach(timer_counter_ms, SENSORS_SAMPLE_TIME_MS, TIMER1_INTERRUPT_TIME_MS) && configuration.is_continuous) {
    if (!is_event_solar_radiation_task) {
      noInterrupts();
      is_event_solar_radiation_task = true;
      ready_tasks_count++;
      interrupts();
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
}

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

      if (i2c_rx_data[0] == I2C_SOLAR_RADIATION_ADDRESS_ADDRESS && rx_data_length == I2C_SOLAR_RADIATION_ADDRESS_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_SOLAR_RADIATION_ONESHOT_ADDRESS && rx_data_length == I2C_SOLAR_RADIATION_ONESHOT_LENGTH) {
        is_i2c_data_ok = true;
      }
      else if (i2c_rx_data[0] == I2C_SOLAR_RADIATION_CONTINUOUS_ADDRESS && rx_data_length == I2C_SOLAR_RADIATION_CONTINUOUS_LENGTH) {
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
  uint16_t error_count = 0;
  uint16_t valid_count = 0;

  float avg = 0;

  #if (USE_SENSOR_DSR || USE_SENSOR_VSR)
  bufferPtrResetBack<sample_t, uint16_t>(&solar_radiation_samples, SAMPLES_COUNT);
  #endif

  uint16_t sample_count = RMAP_REPORT_SAMPLES_COUNT;

  #if (USE_SENSOR_DSR || USE_SENSOR_VSR)
  if (solar_radiation_samples.count < sample_count) {
    sample_count = solar_radiation_samples.count;
  }
  #endif

  #if (USE_SENSORS_COUNT == 0)
  sample_count = 0;
  #endif

  for (uint16_t i = 0; i < sample_count; i++) {
    bool is_new_observation = (((i+1) % OBSERVATION_SAMPLES_COUNT_MAX) == 0);

    #if (USE_SENSOR_DSR || USE_SENSOR_VSR)
    float solar_radiation = bufferReadBack<sample_t, uint16_t, float>(&solar_radiation_samples, SAMPLES_COUNT);
    #endif

    if (i == 0) {
      #if (USE_SENSOR_DSR || USE_SENSOR_VSR)
      SERIAL_INFO(F("%u\t%.0f\t"), solar_radiation_samples.count, solar_radiation);
      #endif
    }

    #if (USE_SENSOR_DSR || USE_SENSOR_VSR)
    if (isValid(solar_radiation)) {
      valid_count++;
      avg += (float) ((solar_radiation - avg) / valid_count);
    }
    else {
      error_count++;
    }
    #endif
  }

  #if (USE_SENSOR_DSR || USE_SENSOR_VSR)
  if ((valid_count >= RMAP_REPORT_SAMPLE_VALID_MIN) && (error_count <= RMAP_REPORT_SAMPLE_ERROR_MAX)) {
    readable_data_write_ptr->solar_radiation.avg = round(avg);
  }
  #endif

  #if (USE_SENSOR_DSR || USE_SENSOR_VSR)
  SERIAL_INFO_CLEAN(F("%.0f\r\n"), readable_data_write_ptr->solar_radiation.avg);
  #endif
}

void samples_processing() {
  reset_report_buffer();
  make_report();
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
  float value = (float) UINT16_MAX;

  if ((adc_value >= ADC_MIN) && (adc_value <= ADC_MAX)) {
    value = ADC_VOLTAGE_MAX / ADC_MAX * adc_value;
    value = round(value / 10.0) * 10.0;
    value += offset_mv;
  }

  return value;
}

float getSolarRadiation (float adc_value) {
  float value = getSolarRadiationMv(adc_value, configuration.adc_voltage_offset_1);

  if ((value < (configuration.adc_voltage_min - SOLAR_RADIATION_ERROR_VOLTAGE_MIN)) || (value > (configuration.adc_voltage_max + SOLAR_RADIATION_ERROR_VOLTAGE_MAX))) {
    value = UINT16_MAX;
  }
  else {
    value = ((value - configuration.adc_voltage_min) / (configuration.adc_voltage_max - configuration.adc_voltage_min) * SOLAR_RADIATION_MAX);
  }

  return round(value);
}

void solaRadiationOffset(uint8_t count, uint8_t delay_ms, float *offset, float ideal) {
  float value = 0;

  for (uint8_t i = 0; i < count; i++) {
    value += ((float) solarRadiationRead() - value) / (float) (i+1);
    delay(delay_ms);
  }

  *offset = ideal - getSolarRadiationMv(value, 0);

  SERIAL_INFO(F("Solar Radiation: ideal %.0f mV - read %.0f mV = offset %.0f mV\r\n"), ideal, getSolarRadiationMv(value, 0), *offset);
}

void solar_radiation_task () {
  static uint8_t retry;
  static bool is_error;
  static solar_radiation_state_t state_after_wait;
  static uint32_t delay_ms;
  static uint32_t start_time_ms;
  static uint8_t i = 0;
  static float solar_radiation;

  switch (solar_radiation_state) {
    case SOLAR_RADIATION_INIT:
      i = 0;
      retry = 0;
      is_error = false;
      #if (USE_SENSOR_DSR)
      solarRadiationPowerOn();
      solarRadiationRead();
      delay_ms = SOLAR_RADIATION_READ_DELAY_MS;
      start_time_ms = millis();
      state_after_wait = SOLAR_RADIATION_READING;
      solar_radiation_state = SOLAR_RADIATION_WAIT_STATE;
      SERIAL_TRACE(F("SOLAR_RADIATION_INIT --> SOLAR_RADIATION_READING\r\n"));
      #else
      solar_radiation_state = SOLAR_RADIATION_ELABORATE;
      SERIAL_TRACE(F("SOLAR_RADIATION_INIT --> SOLAR_RADIATION_ELABORATE\r\n"));
      #endif
    break;

    case SOLAR_RADIATION_READING:
      #if (USE_SENSOR_DSR)
      solar_radiation += ((float) solarRadiationRead() - solar_radiation) / (float) (i+1);

      if (i < SOLAR_RADIATION_READ_COUNT) {
        i++;
        delay_ms = SOLAR_RADIATION_VALUES_READ_DELAY_MS;
        start_time_ms = millis();
        state_after_wait = SOLAR_RADIATION_READING;
        solar_radiation_state = SOLAR_RADIATION_WAIT_STATE;
      }
      else {
        solar_radiation_state = SOLAR_RADIATION_ELABORATE;
        SERIAL_TRACE(F("SOLAR_RADIATION_READING --> SOLAR_RADIATION_ELABORATE\r\n"));
      }
      #endif
    break;

    case SOLAR_RADIATION_ELABORATE:
      #if (USE_SENSOR_DSR)
      solar_radiation = getSolarRadiation(solar_radiation);
      addValue<sample_t, uint16_t, float>(&solar_radiation_samples, SAMPLES_COUNT, solar_radiation);
      #endif

      samples_processing();

      solar_radiation_state = SOLAR_RADIATION_END;
      SERIAL_TRACE(F("SOLAR_RADIATION_ELABORATE --> SOLAR_RADIATION_END\r\n"));
    break;

    case SOLAR_RADIATION_END:
      solarRadiationPowerOff();
      noInterrupts();
      is_event_solar_radiation_task = false;
      ready_tasks_count--;
      interrupts();
      solar_radiation_state = SOLAR_RADIATION_INIT;
      SERIAL_TRACE(F("SOLAR_RADIATION_END --> SOLAR_RADIATION_INIT\r\n"));
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
  float value = (float) UINT16_MAX;

  if (!isnan(adc_value) && (adc_value >= ADC_MIN) && (adc_value <= ADC_MAX)) {
    value = adc_value;
    value += offset;
    value *= gain;
  }

  return value;
}

float getAdcAnalogValue (float adc_value, float min, float max) {
  float value = (float) UINT16_MAX;

  if (!isnan(adc_value)) {
    value = adc_value;
    value *= (((max - min) / (float)(ADC_MAX)));
    value += min;
  }

  return value;
}

float getSolarRadiation (float adc_value, float adc_voltage_min, float adc_voltage_max) {
  float value = adc_value;

  if ((value < (adc_voltage_min + SOLAR_RADIATION_ERROR_VOLTAGE_MIN)) || (value > (adc_voltage_max + SOLAR_RADIATION_ERROR_VOLTAGE_MAX))) {
    value = UINT16_MAX;
  }
  else {
    value = ((value - adc_voltage_min) / (adc_voltage_max - adc_voltage_min) * SOLAR_RADIATION_MAX);
  }

  return round(value);
}

void solar_radiation_task_hr () {
  static uint8_t retry;
  static solar_radiation_hr_state_t state_after_wait;
  static uint32_t delay_ms;
  static uint32_t start_time_ms;
  static uint8_t adc_index;
  static uint8_t adc_channel;
  static int16_t adc_value;
  static float value;
  static bool is_error;
  adc_result_t adc_result;

  switch (solar_radiation_hr_state) {
    case SOLAR_RADIATION_HR_INIT:
      solarRadiationPowerOn();
      adc_index = 0;
      adc_channel = 0;
      retry = 0;
      is_error = false;
      solar_radiation_acquisition_count++;
      solar_radiation_hr_state = SOLAR_RADIATION_HR_SET;
      SERIAL_TRACE(F("SOLAR_RADIATION_HR_INIT --> SOLAR_RADIATION_HR_SET\r\n"));
    break;

    case SOLAR_RADIATION_HR_SET:
      is_error = false;
      while (isAdsChnDisabled(use_adc_channel[adc_index][adc_channel]) && (adc_channel < ADS1115_CHANNEL_COUNT)) {
        adc_channel++;
      }

      if (adc_channel < ADS1115_CHANNEL_COUNT) {
        value = (float) (UINT16_MAX);
        solar_radiation_hr_state = SOLAR_RADIATION_HR_READ;
        SERIAL_TRACE(F("SOLAR_RADIATION_HR_SET --> SOLAR_RADIATION_HR_READ\r\n"));
      }
      else if (adc_index < (ADC_COUNT - 1)) {
        adc_index++;
        adc_channel = 0;
      }
      else {
        solar_radiation_hr_state = SOLAR_RADIATION_HR_PROCESS;
        SERIAL_TRACE(F("SOLAR_RADIATION_HR_SET --> SOLAR_RADIATION_HR_PROCESS\r\n"));
      }
    break;

    case SOLAR_RADIATION_HR_READ:
      if (adc_index == 0) {
        adc_result = adc1.readSingleChannel(adc_channel, &adc_value);
      }
      #if (ADC_COUNT > 1)
      else if (adc_index == 1) {
        adc_result = adc2.readSingleChannel(adc_channel, &adc_value);
      }
      #endif
      #if (ADC_COUNT > 2)
      else if (adc_index == 2) {
        adc_result = adc3.readSingleChannel(adc_channel, &adc_value);
      }
      #endif

      if (adc_result == ADC_OK) {
        value = (float) (adc_value);
        solar_radiation_hr_state = SOLAR_RADIATION_HR_EVALUATE;
        SERIAL_TRACE(F("SOLAR_RADIATION_HR_READ --> SOLAR_RADIATION_HR_EVALUATE\r\n"));
      }
      else if (adc_result == ADC_ERROR) {
        i2c_error++;
        value = (float) (UINT16_MAX);
        is_error = true;
        solar_radiation_hr_state = SOLAR_RADIATION_HR_EVALUATE;
        SERIAL_TRACE(F("SOLAR_RADIATION_HR_READ --> SOLAR_RADIATION_HR_EVALUATE\r\n"));
      }
    break;

    case SOLAR_RADIATION_HR_EVALUATE:
      #if (IS_CALIBRATION)
      SERIAL_INFO(F("ADC %u\tAIN%u ==> (%.6f + %.6f) * %.6f = "), adc_index, adc_channel, value, configuration.adc_calibration_offset[adc_index][adc_channel], configuration.adc_calibration_gain[adc_index][adc_channel]);
      #endif

      if (!is_error) {
        value = getAdcCalibratedValue (value, configuration.adc_calibration_offset[adc_index][adc_channel], configuration.adc_calibration_gain[adc_index][adc_channel]);
        value = getAdcAnalogValue(value, configuration.adc_analog_min[adc_index][adc_channel], configuration.adc_analog_max[adc_index][adc_channel]);
      }

      #if (IS_CALIBRATION)
      SERIAL_INFO_CLEAN(F("%.6f [ %s ]\r\n"), value, is_error ? ERROR_STRING : OK_STRING);
      #endif

      if (!is_error) {
        value = getSolarRadiation(value, configuration.adc_analog_min[adc_index][adc_channel], configuration.adc_analog_max[adc_index][adc_channel]);
      }

      if (false) {}
      else if ((adc_index == SOLAR_RADIATION_ADC_INDEX) && (adc_channel == SOLAR_RADIATION_ADC_CHANNEL_INPUT)) {
        addValue<sample_t, uint16_t, float>(&solar_radiation_samples, SAMPLES_COUNT, value);
      }

      adc_channel++;
      solar_radiation_hr_state = SOLAR_RADIATION_HR_SET;
      SERIAL_TRACE(F("SOLAR_RADIATION_HR_EVALUATE --> SOLAR_RADIATION_HR_SET\r\n"));
    break;

    case SOLAR_RADIATION_HR_PROCESS:
      samples_processing();
      solar_radiation_hr_state = SOLAR_RADIATION_HR_END;
      SERIAL_TRACE(F("SOLAR_RADIATION_HR_PROCESS --> SOLAR_RADIATION_HR_END\r\n"));
    break;

    case SOLAR_RADIATION_HR_END:
      #if (IS_CALIBRATION)
      SERIAL_INFO_CLEAN(F("\r\n"));
      #endif

      if ((solar_radiation_acquisition_count >= ACQUISITION_COUNT_FOR_POWER_RESET) || is_error) {
        solar_radiation_acquisition_count = 0;
        solarRadiationPowerOff();
      }

      noInterrupts();
      is_event_solar_radiation_task = false;
      ready_tasks_count--;
      interrupts();
      solar_radiation_hr_state = SOLAR_RADIATION_HR_INIT;
      SERIAL_TRACE(F("SOLAR_RADIATION_HR_END --> SOLAR_RADIATION_HR_INIT\r\n"));
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
  readable_data_temp_ptr = readable_data_write_ptr;
  readable_data_write_ptr = readable_data_read_ptr;
  readable_data_read_ptr = readable_data_temp_ptr;
}

void reset_samples_buffer() {
  #if (USE_SENSOR_DSR || USE_SENSOR_VSR)
  bufferReset<sample_t, uint16_t, float>(&solar_radiation_samples, SAMPLES_COUNT);
  #endif
}

void reset_report_buffer () {
  readable_data_write_ptr->solar_radiation.avg = (float) UINT16_MAX;
}

void command_task() {
  #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
  char buffer[30];
  #endif

  switch(i2c_rx_data[1]) {
    case I2C_SOLAR_RADIATION_COMMAND_ONESHOT_START:
      #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
      strcpy(buffer, "ONESHOT START");
      #endif
      is_oneshot = true;
      is_continuous = false;
      is_start = true;
      is_stop = false;
      is_test = false;
      commands();
    break;

    case I2C_SOLAR_RADIATION_COMMAND_ONESHOT_STOP:
      #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
      strcpy(buffer, "ONESHOT STOP");
      #endif
      is_oneshot = true;
      is_continuous = false;
      is_start = false;
      is_stop = true;
      is_test = false;
      commands();
    break;

    case I2C_SOLAR_RADIATION_COMMAND_ONESHOT_START_STOP:
      #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
      strcpy(buffer, "ONESHOT START-STOP");
      #endif
      is_oneshot = true;
      is_continuous = false;
      is_start = true;
      is_stop = true;
      is_test = false;
      commands();
    break;

    case I2C_SOLAR_RADIATION_COMMAND_CONTINUOUS_START:
      #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
      strcpy(buffer, "CONTINUOUS START");
      #endif
      is_oneshot = false;
      is_continuous = true;
      is_start = true;
      is_stop = false;
      is_test = false;
      commands();
    break;

    case I2C_SOLAR_RADIATION_COMMAND_CONTINUOUS_STOP:
      #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
      strcpy(buffer, "CONTINUOUS STOP");
      #endif
      is_oneshot = false;
      is_continuous = true;
      is_start = false;
      is_stop = true;
      is_test = false;
      commands();
    break;

    case I2C_SOLAR_RADIATION_COMMAND_CONTINUOUS_START_STOP:
      #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
      strcpy(buffer, "CONTINUOUS START-STOP");
      #endif
      is_oneshot = false;
      is_continuous = true;
      is_start = true;
      is_stop = true;
      is_test = false;
      commands();
    break;

    case I2C_SOLAR_RADIATION_COMMAND_TEST_READ:
      #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
      strcpy(buffer, "TEST READ");
      #endif
      is_test = true;
      tests();
    break;

    case I2C_SOLAR_RADIATION_COMMAND_SAVE:
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

void commands() {
  noInterrupts();

  //! CONTINUOUS START
  if (configuration.is_continuous && is_continuous && is_start && !is_stop) {
    reset_samples_buffer();
    reset_report_buffer();
  }
  //! CONTINUOUS STOP
  else if (configuration.is_continuous && is_continuous && !is_start && is_stop) {
    exchange_buffers();
  }
  //! CONTINUOUS START-STOP
  else if (configuration.is_continuous && is_continuous && is_start && is_stop) {
    exchange_buffers();
  }
  //! ONESHOT START
  else if (configuration.is_oneshot && is_oneshot && is_start && !is_stop) {
  }
  //! ONESHOT STOP
  else if (configuration.is_oneshot && is_oneshot && !is_start && is_stop) {
  }
  //! ONESHOT START-STOP
  else if (configuration.is_oneshot && is_oneshot && is_start && is_stop) {
  }

  interrupts();
}

void tests() {
  noInterrupts();

  //! TEST
  if (is_test) {
    exchange_buffers();
  }

  interrupts();
}
