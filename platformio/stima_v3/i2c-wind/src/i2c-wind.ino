/**@file i2c-wind.ino */

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
#define SERIAL_TRACE_LEVEL I2C_WIND_SERIAL_TRACE_LEVEL

#include "i2c-wind.h"

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

  #if (USE_SENSOR_GWS)
  Serial1.begin(GWS_SERIAL_BAUD);
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

      #if (USE_SENSOR_GWS)
      if (is_event_wind_task) {
        wind_task();
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
  is_event_wind_task = false;

  wind_acquisition_count = 0;

  wind_state = WIND_INIT;

  is_oneshot = false;
  is_continuous = false;
  is_start = false;
  is_stop = false;
  is_test = false;

  interrupts();
}

void init_pins() {
  pinMode(CONFIGURATION_RESET_PIN, INPUT_PULLUP);

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
}

void init_sensors () {
  if (configuration.is_continuous) {
    SERIAL_INFO(F("\r\n"));
    SERIAL_INFO(F("--> acquiring %u~%u samples in %u minutes\r\n"), OBSERVATION_SAMPLES_COUNT_MIN, OBSERVATION_SAMPLES_COUNT_MAX, OBSERVATIONS_MINUTES);
    SERIAL_INFO(F("--> max %u samples error in %u minutes (observation)\r\n"), OBSERVATION_SAMPLE_ERROR_MAX, OBSERVATIONS_MINUTES);
    SERIAL_INFO(F("--> max %u samples error in 10 minutes\r\n"), WMO_REPORT_SAMPLE_ERROR_MAX);
    SERIAL_INFO(F("--> max %u samples error in %u minutes (report)\r\n\r\n"), RMAP_REPORT_SAMPLE_ERROR_MAX, STATISTICAL_DATA_COUNT * OBSERVATIONS_MINUTES);

    #if (USE_SENSOR_GWS)
    SERIAL_INFO(F("sc: speed sample count\r\n"));
    SERIAL_INFO(F("dc: direction sample count\r\n"));
    SERIAL_INFO(F("speed: sensor speed\r\n"));
    SERIAL_INFO(F("dir: sensor direction\r\n"));
    SERIAL_DEBUG(F("ua: average u component over 10'\r\n"));
    SERIAL_DEBUG(F("va: average v component over 10'\r\n"));
    SERIAL_INFO(F("vs10: vectorial average speed over 10'\r\n"));
    SERIAL_INFO(F("vd10: vectorial average speed over 10'\r\n"));
    SERIAL_DEBUG(F("ub: average u component over %u'\r\n"), STATISTICAL_DATA_COUNT * OBSERVATIONS_MINUTES);
    SERIAL_DEBUG(F("vb: average v component over %u'\r\n"), STATISTICAL_DATA_COUNT * OBSERVATIONS_MINUTES);
    SERIAL_INFO(F("vsr: vectorial average speed over %u'\r\n"), STATISTICAL_DATA_COUNT * OBSERVATIONS_MINUTES);
    SERIAL_INFO(F("vdr: vectorial average speed over %u'\r\n"), STATISTICAL_DATA_COUNT * OBSERVATIONS_MINUTES);
    SERIAL_INFO(F("ss: scalar average speed over %u'\r\n"), STATISTICAL_DATA_COUNT * OBSERVATIONS_MINUTES);
    SERIAL_INFO(F("pgs: peak gust speed\r\n"));
    SERIAL_INFO(F("pgd: peak gust direction\r\n"));
    SERIAL_INFO(F("lgs: long gust speed'\r\n"));
    SERIAL_INFO(F("lgd: long gust direction'\r\n"));
    SERIAL_INFO(F("C1: %% of sample <= 1.0 m/s \r\n"));
    SERIAL_INFO(F("C2: %% of sample <= 2.0 m/s \r\n"));
    SERIAL_INFO(F("C4: %% of sample <= 4.0 m/s \r\n"));
    SERIAL_INFO(F("C7: %% of sample <= 7.0 m/s \r\n"));
    SERIAL_INFO(F("C10: %% of sample <= 10.0 m/s \r\n"));
    SERIAL_INFO(F("CXX: %% of sample > 10.0 m/s \r\n\r\n"));

    #if (SERIAL_TRACE_LEVEL < SERIAL_TRACE_LEVEL_DEBUG)
    SERIAL_INFO(F("sc\tdc\tspeed\tdir\tvs10\tvd10\tvsr\tvdr\tss\tpgs\tpgd\tlgs\tlgd\tC1\tC2\tC4\tC7\tC10\tCXX\r\n\r\n"));
    #else
    SERIAL_DEBUG(F("sc\tdc\tspeed\tdir\tua\tva\tvs10\tvd10\tub\tvb\tvsr\tvdr\tss\tpgs\tpgd\tlgs\tlgd\tC1\tC2\tC4\tC7\tC10\tCXX\r\n\r\n"));
    #endif
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

  #if (USE_SENSOR_GWS)
  if (executeTimerTaskEach(timer_counter_ms, SENSORS_SAMPLE_TIME_MS, TIMER1_INTERRUPT_TIME_MS) && configuration.is_continuous) {
    if (!is_event_wind_task) {
      noInterrupts();
      is_event_wind_task = true;
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

void getSDFromUV (float u, float v, float *speed, float *direction) {
  *speed = sqrt(u*u + v*v);
  *direction = RAD_TO_DEG * atan2(u, v);
}

void make_report () {
  uint16_t valid_count_a = 0;
  uint16_t error_count_a = 0;

  uint16_t valid_count_b = 0;
  uint16_t error_count_b = 0;

  uint16_t valid_count_speed = 0;
  uint16_t error_count_speed = 0;

  uint16_t valid_count_o = 0;
  uint16_t error_count_o = 0;

  uint16_t valid_count_c = 0;
  uint16_t error_count_c = 0;

  float ua = 0;
  float va = 0;

  float ub = 0;
  float vb = 0;

  float uc = 0;
  float vc = 0;

  float vavg10_speed = 0;
  float vavg10_direction = 0;

  float vavg_speed = 0;
  float vavg_direction = 0;

  float peak_gust_speed = -1.0;
  float peak_gust_direction = 0;

  float vavg_speed_o = -1.0;
  float vavg_direction_o = 0;

  float long_gust_speed = -1.0;
  float long_gust_direction = 0;

  float avg_speed = 0;

  #if (USE_SENSOR_GWS)
  bufferPtrResetBack<sample_t, uint16_t>(&wind_speed_samples, SAMPLES_COUNT);
  #endif

  #if (USE_SENSOR_GWS)
  bufferPtrResetBack<sample_t, uint16_t>(&wind_direction_samples, SAMPLES_COUNT);
  #endif

  uint16_t sample_count = RMAP_REPORT_SAMPLES_COUNT;

  #if (USE_SENSOR_GWS)
  if (wind_speed_samples.count < sample_count) {
    sample_count = wind_speed_samples.count;
  }
  #endif

  #if (USE_SENSOR_GWS)
  if (wind_direction_samples.count < sample_count) {
    sample_count = wind_direction_samples.count;
  }
  #endif

  #if (USE_SENSORS_COUNT == 0)
  sample_count = 0;
  #endif

  for (uint16_t i = 0; i < sample_count; i++) {
    bool is_new_observation = (((i+1) % OBSERVATION_SAMPLES_COUNT_MAX) == 0);

    #if (USE_SENSOR_GWS)
    float speed = bufferReadBack<sample_t, uint16_t, float>(&wind_speed_samples, SAMPLES_COUNT);
    #endif

    #if (USE_SENSOR_GWS)
    float direction = bufferReadBack<sample_t, uint16_t, float>(&wind_direction_samples, SAMPLES_COUNT);

    #endif

    if (i == 0) {
      #if (USE_SENSOR_GWS)
      SERIAL_INFO(F("%u\t%u\t%.2f\t%.0f\t"), wind_speed_samples.count, wind_direction_samples.count, speed, direction);
      #endif
    }

    #if (USE_SENSOR_GWS)
    if (i < WMO_REPORT_SAMPLES_COUNT) {
      if (ISVALID(speed) && ISVALID(direction)) {
        valid_count_a++;
        ua += ((float) (-speed * sin(DEG_TO_RAD * direction)) - ua) / valid_count_a;
        va += ((float) (-speed * cos(DEG_TO_RAD * direction)) - va) / valid_count_a;
      }
      else {
        error_count_a++;
      }
    }

    if (ISVALID(speed) && ISVALID(direction)) {
      valid_count_b++;
      valid_count_c++;

      ub += ((float) (-speed * sin(DEG_TO_RAD * direction)) - ub) / valid_count_b;
      vb += ((float) (-speed * cos(DEG_TO_RAD * direction)) - vb) / valid_count_b;

      uc += ((float) (-speed * sin(DEG_TO_RAD * direction)) - uc) / valid_count_c;
      vc += ((float) (-speed * cos(DEG_TO_RAD * direction)) - vc) / valid_count_c;

      if (speed >= peak_gust_speed) {
        peak_gust_speed = speed;
        peak_gust_direction = direction;
      }
    }
    else {
      error_count_b++;
      error_count_c++;
    }
    #endif

    #if (USE_SENSOR_GWS)
    if (is_new_observation) {
      if (valid_count_c && (error_count_c <= OBSERVATION_SAMPLE_ERROR_MAX)) {
        valid_count_o++;
        getSDFromUV(uc, vc, &vavg_speed_o, &vavg_direction_o);

        if (vavg_speed_o >= long_gust_speed) {
          long_gust_speed = vavg_speed_o;
          long_gust_direction = vavg_direction_o;
        }
      }
      else {
        error_count_o++;
      }

      uc = 0;
      vc = 0;
      vavg_speed_o = 0;
      vavg_direction_o = 0;
      valid_count_c = 0;
      error_count_c = 0;
    }
    #endif
  }

  #if (USE_SENSOR_GWS)
    getSDFromUV(ua, va, &vavg10_speed, &vavg10_direction);
    getSDFromUV(ub, vb, &vavg_speed, &vavg_direction);
    readable_data_write_ptr->wind.vavg10_speed = vavg10_speed;
    readable_data_write_ptr->wind.vavg10_direction = vavg10_direction;
    readable_data_write_ptr->wind.vavg_speed = vavg_speed;
    readable_data_write_ptr->wind.vavg_direction = vavg_direction;
    readable_data_write_ptr->wind.peak_gust_speed = peak_gust_speed;
    readable_data_write_ptr->wind.peak_gust_direction = peak_gust_direction;
    readable_data_write_ptr->wind.long_gust_speed = long_gust_speed;
    readable_data_write_ptr->wind.long_gust_direction = long_gust_direction;
  #endif
}

void samples_processing() {
  reset_report_buffer();
  make_report();
}

#if (USE_SENSOR_GWS)
void wind_task () {
  static uint16_t retry;
  static bool is_error;
  static wind_state_t state_after_wait;
  static uint32_t delay_ms;
  static uint32_t start_time_ms;
  static uint8_t i = 0;
  float speed;
  float direction;

  switch (wind_state) {
    case WIND_INIT:
      i = 0;
      retry = 0;
      is_error = false;
      wind_acquisition_count++;

      wind_state = WIND_READING;
      SERIAL_TRACE(F("WIND_INIT --> WIND_READING\r\n"));
    break;

    case WIND_READING:
      if (Serial1.available()) {
        uart_rx_buffer_length = Serial1.readBytes(uart_rx_buffer, UART_RX_BUFFER_LENGTH);
        wind_state = WIND_ELABORATE;
        SERIAL_TRACE(F("WIND_READING --> WIND_ELABORATE\r\n"));
      }
      else {
        is_error = true;
        wind_state = WIND_ELABORATE;
        SERIAL_TRACE(F("WIND_READING --> WIND_ELABORATE\r\n"));
      }
    break;

    case WIND_ELABORATE:
      if (is_error) {
        speed = UINT16_MAX;
        direction = UINT16_MAX;
      }
      else {
        windsonic_interpreter(&speed, &direction);
      }

      addValue<sample_t, uint16_t, float>(&wind_speed_samples, SAMPLES_COUNT, speed);
      addValue<sample_t, uint16_t, float>(&wind_direction_samples, SAMPLES_COUNT, direction);
      samples_processing();

      wind_state = WIND_END;
      SERIAL_TRACE(F("WIND_ELABORATE --> WIND_END\r\n"));
    break;

    case WIND_END:
      noInterrupts();
      is_event_wind_task = false;
      ready_tasks_count--;
      interrupts();
      wind_state = WIND_INIT;
      SERIAL_TRACE(F("WIND_END --> WIND_INIT\r\n"));
    break;

    case WIND_WAIT_STATE:
      if (millis() - start_time_ms > delay_ms) {
        wind_state = state_after_wait;
      }
    break;
  }
}

bool windsonic_interpreter (float *speed, float *direction) {
  *speed = UINT16_MAX;
  *direction = UINT16_MAX;

  if ((uart_rx_buffer[GWS_STX_INDEX] == STX_VALUE) && (uart_rx_buffer[GWS_ETX_INDEX] != ETX_VALUE)) {
    char tempstr[GWS_SPEED_LENGTH+1];

    strncpy(tempstr, (const char *)(uart_rx_buffer+GWS_DIRECTION_INDEX), GWS_DIRECTION_LENGTH);
    *direction = (float) atof(tempstr);
    memset(tempstr, 0, GWS_SPEED_LENGTH+1);

    strncpy(tempstr, (const char *)(uart_rx_buffer+GWS_SPEED_INDEX), GWS_SPEED_LENGTH);
    *speed = (float) atof(tempstr);
    memset(tempstr, 0, GWS_SPEED_LENGTH+1);
  }

  return true;
}
#endif

void exchange_buffers() {
  readable_data_temp_ptr = readable_data_write_ptr;
  readable_data_write_ptr = readable_data_read_ptr;
  readable_data_read_ptr = readable_data_temp_ptr;
}

void reset_samples_buffer() {
  #if (USE_SENSOR_GWS)
  bufferReset<sample_t, uint16_t, float>(&wind_speed_samples, SAMPLES_COUNT);
  bufferReset<sample_t, uint16_t, float>(&wind_direction_samples, SAMPLES_COUNT);
  #endif
}

void reset_report_buffer () {
  readable_data_write_ptr->wind.vavg10_speed = (float) UINT16_MAX;
  readable_data_write_ptr->wind.vavg10_direction = (float) UINT16_MAX;
  readable_data_write_ptr->wind.vavg_speed = (float) UINT16_MAX;
  readable_data_write_ptr->wind.vavg_direction = (float) UINT16_MAX;
  readable_data_write_ptr->wind.avg_speed = (float) UINT16_MAX;
  readable_data_write_ptr->wind.peak_gust_speed = (float) UINT16_MAX;
  readable_data_write_ptr->wind.peak_gust_direction = (float) UINT16_MAX;
  readable_data_write_ptr->wind.long_gust_speed = (float) UINT16_MAX;
  readable_data_write_ptr->wind.long_gust_direction = (float) UINT16_MAX;
}

void command_task() {
  #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
  char buffer[30];
  #endif

  switch(i2c_rx_data[1]) {
    case I2C_WIND_COMMAND_ONESHOT_START:
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

    case I2C_WIND_COMMAND_ONESHOT_STOP:
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

    case I2C_WIND_COMMAND_ONESHOT_START_STOP:
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

    case I2C_WIND_COMMAND_CONTINUOUS_START:
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

    case I2C_WIND_COMMAND_CONTINUOUS_STOP:
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

    case I2C_WIND_COMMAND_CONTINUOUS_START_STOP:
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

    case I2C_WIND_COMMAND_TEST_READ:
      #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_TRACE)
      strcpy(buffer, "TEST READ");
      #endif
      is_test = true;
      tests();
    break;

    case I2C_WIND_COMMAND_SAVE:
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
