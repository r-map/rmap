#include <debug_config.h>

#define SERIAL_TRACE_LEVEL SERIAL_TRACE_LEVEL_INFO

#include <debug.h>
#include <i2c_config.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <Wire.h>
#include <i2c_utility.h>

#define I2C_ADDRESS_1               (1)
#define I2C_ADDRESS_2               (2)
#define I2C_ADDRESS_3               (3)
#define I2C_ADDRESS_4               (4)

#define I2C_MY_ADDRESS              I2C_ADDRESS_4

#define I2C_MAX_ERROR_COUNT         (10)
#define I2C_CHECK_DELAY_MS          (5000)

#define I2C_REQUEST_MIN_DELAY_MS    (100)
#define I2C_REQUEST_MAX_DELAY_MS    (4000)

#define I2C_SEND_DATA_MIN_DELAY_MS  (100)
#define I2C_SEND_DATA_MAX_DELAY_MS  (4000)

#define PRINT_STATUS_DELAY_MS       (1000)

#define WDT_TIMER                   (WDTO_8S)

typedef enum {
  INIT,                      //!< init tasks and sensors
  #if (USE_POWER_DOWN)
  ENTER_POWER_DOWN,          //!< if no task is running, activate power down
  #endif
  TASKS_EXECUTION,           //!< execute active tasks
  END                        //!< go to ENTER_POWER_DOWN or TASKS_EXECUTION
} state_t;

state_t state;
volatile uint8_t ready_tasks_count;
uint32_t awakened_event_occurred_time_ms;

bool is_event_request_data;
bool is_event_send_data;

uint32_t start_print_status_delay_ms;

uint32_t start_i2c_check_delay_ms;

uint32_t i2c_request_delay_ms;
uint32_t start_time_i2c_request_delay_ms;

uint32_t i2c_send_delay_ms;
uint32_t start_time_i2c_send_delay_ms;

volatile static uint8_t rx_buffer[I2C_MAX_DATA_LENGTH];
volatile static uint8_t rx_buffer_length;

uint8_t tx_buffer[I2C_MAX_DATA_LENGTH + 1];
uint8_t tx_buffer_length;

uint8_t rq_buffer[I2C_MAX_DATA_LENGTH + 1];
uint8_t rq_buffer_length;

uint16_t i2c_rx_error_count;
uint16_t i2c_rx_success_count;

uint16_t i2c_tx_error_count;
uint16_t i2c_tx_success_count;

uint16_t i2c_rq_error_count;
uint16_t i2c_rq_success_count;

uint8_t I2C_ClearBus_NEW() {
  #if defined(TWCR) && defined(TWEN)
  TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
  #endif

  pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
  pinMode(SCL, INPUT_PULLUP);

  boolean SCL_LOW = (digitalRead(SCL) == LOW); // Check is SCL is Low.
  if (SCL_LOW) { //If it is held low Arduno cannot become the I2C master.
    return 1; //I2C bus error. Could not clear SCL clock line held low
  }

  boolean SDA_LOW = (digitalRead(SDA) == LOW);  // vi. Check SDA input.
  int clockCount = 20; // > 2x9 clock

  while (SDA_LOW && (clockCount > 0)) { //  vii. If SDA is Low,
    clockCount--;
    // Note: I2C bus is open collector so do NOT drive SCL or SDA high.
    pinMode(SCL, INPUT); // release SCL pullup so that when made output it will be LOW
    pinMode(SCL, OUTPUT); // then clock SCL Low
    delayMicroseconds(10); //  for >5uS
    pinMode(SCL, INPUT); // release SCL LOW
    pinMode(SCL, INPUT_PULLUP); // turn on pullup resistors again
    // do not force high as slave may be holding it low for clock stretching.
    delayMicroseconds(10); //  for >5uS
    // The >5uS is so that even the slowest I2C devices are handled.
    SCL_LOW = (digitalRead(SCL) == LOW); // Check if SCL is Low.
    int counter = 20;
    while (SCL_LOW && (counter > 0)) {  //  loop waiting for SCL to become High only wait 2sec.
      counter--;
      delay(100);
      SCL_LOW = (digitalRead(SCL) == LOW);
    }
    if (SCL_LOW) { // still low after 2 sec error
      return 2; // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
    }
    SDA_LOW = (digitalRead(SDA) == LOW); //   and check SDA input again and loop
  }
  if (SDA_LOW) { // still low
    return 3; // I2C bus error. Could not clear. SDA data line held low
  }

  if (digitalRead(SCL) == HIGH && digitalRead(SDA) == LOW) {
    pinMode(SDA, OUTPUT);      // is connected to SDA
    digitalWrite(SDA, LOW);
    delay(2000);              //maybe too long
    pinMode(SDA, INPUT);       // reset pin
    delay(50);
  }

  // else pull SDA line low for Start or Repeated Start
  pinMode(SDA, INPUT); // remove pullup.
  pinMode(SDA, OUTPUT);  // and then make it LOW i.e. send an I2C Start or Repeated start control.
  // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
  /// A Repeat Start is a Start occurring after a Start with no intervening Stop.
  delayMicroseconds(10); // wait >5uS
  pinMode(SDA, INPUT); // remove output low
  pinMode(SDA, INPUT_PULLUP); // and make SDA high i.e. send I2C STOP control.
  delayMicroseconds(10); // x. wait >5uS
  pinMode(SDA, INPUT); // and reset pins as tri-state inputs which is the default state on reset
  pinMode(SCL, INPUT);
  return 0; // all ok
}

bool check_i2c_bus () {
  if (i2c_rx_error_count + i2c_tx_error_count + i2c_rq_error_count > I2C_MAX_ERROR_COUNT) {
    i2c_rx_error_count = 0;
    i2c_rx_success_count = 0;

    i2c_tx_error_count = 0;
    i2c_tx_success_count = 0;

    i2c_rq_error_count = 0;
    i2c_rq_success_count = 0;

    init_wire();
  }
    // // http://www.forward.com.au/pfod/ArduinoProgramming/I2C_ClearBus/index.html
    // // clear the I2C bus first before calling Wire.begin()
    // uint8_t i2c_bus_state = I2C_ClearBus_NEW();
    //
    // if (i2c_bus_state > 0) {
    //   SERIAL_ERROR(F("%u: [ %s ] I2C bus error. Could not clear!!!\r\n"), I2C_MY_ADDRESS, ERROR_STRING);
    //
    //   if (i2c_bus_state == 1) {
    //     SERIAL_ERROR(F("%u: [ %s ] SCL clock line held low\r\n"), I2C_MY_ADDRESS, ERROR_STRING);
    //   } else if (i2c_bus_state == 2) {
    //     SERIAL_ERROR(F("%u: [ %s ] SCL clock line held low by slave clock stretch\r\n"), I2C_MY_ADDRESS, ERROR_STRING);
    //   } else if (i2c_bus_state == 3) {
    //     SERIAL_ERROR(F("%u: [ %s ] SDA data line held low\r\n"), I2C_MY_ADDRESS, ERROR_STRING);
    //   } else {
    //     SERIAL_ERROR(F("%u: [ %s ] Other I2C Error\r\n"), I2C_MY_ADDRESS, ERROR_STRING);
    //   }
    //
    //   i2c_send_delay_ms += 10000;
    //   start_time_i2c_send_delay_ms = millis();
    //   noInterrupts();
    //   if (is_event_send_data) {
    //     is_event_send_data = false;
    //     ready_tasks_count--;
    //   }
    //   interrupts();
    //
    //   i2c_request_delay_ms += 10000;
    //   start_time_i2c_request_delay_ms = millis();
    //   noInterrupts();
    //   if (is_event_request_data) {
    //     is_event_request_data = false;
    //     ready_tasks_count--;
    //   }
    //   interrupts();

      // bus clear
      // re-enable Wire
      // now can start Wire Arduino master
  //     init_wire();
  //     return false;
  //   } else {
  //     return true;
  //   }
  // }
  // return true;
}

void i2c_request_interrupt_handler() {
  uint8_t start_value = 0;
  uint8_t length = I2C_MAX_DATA_LENGTH;
  uint8_t buffer[I2C_MAX_DATA_LENGTH + 1];

  switch (I2C_MY_ADDRESS) {
    case I2C_ADDRESS_1: start_value = 128; break;
    case I2C_ADDRESS_2: start_value = 160; break;
    case I2C_ADDRESS_3: start_value = 192; break;
    case I2C_ADDRESS_4: start_value = 224; break;
  }

  for (uint8_t i = 0; i < I2C_MAX_DATA_LENGTH; i++) {
    buffer[i] = start_value + i;
  }

  buffer[length] = crc8(buffer, length);
  Wire.write(buffer, length + 1);

  //! write readable_data_length bytes of data stored in readable_data_read_ptr (base) + readable_data_address (offset) on i2c bus
  // Wire.write((uint8_t *)readable_data_read_ptr+readable_data_address, readable_data_length);
}

void i2c_receive_interrupt_handler(int rx_data_length) {
  rx_buffer_length = rx_data_length;

  for (uint8_t i = 0; i < rx_data_length; i++) {
    if (i < I2C_MAX_DATA_LENGTH + 1) {
      rx_buffer[i] = Wire.read();
    }
  }

  if (rx_buffer_length > I2C_MAX_DATA_LENGTH + 1) {
    rx_buffer_length = I2C_MAX_DATA_LENGTH + 1;
  }

  // bool is_i2c_data_ok = false;
  //
  // //! read rx_data_length bytes of data from i2c bus
  // for (uint8_t i=0; i<rx_data_length; i++) {
  //   i2c_rx_data[i] = Wire.read();
  // }
  //
  // //! it is a registers read?
  // if (rx_data_length == 2 && is_readable_register(i2c_rx_data[0])) {
  //   //! offset in readable_data_read_ptr buffer
  //   readable_data_address = i2c_rx_data[0];
  //
  //   //! length (in bytes) of data to be read in readable_data_read_ptr
  //   readable_data_length = i2c_rx_data[1];
  // }
  // //! it is a command?
  // else if (rx_data_length == 2 && is_command(i2c_rx_data[0])) {
  //   noInterrupts();
  //   //! enable Command task
  //   if (!is_event_command_task) {
  //     is_event_command_task = true;
  //     ready_tasks_count++;
  //   }
  //   interrupts();
  // }
  // //! it is a registers write?
  // else if (is_writable_register(i2c_rx_data[0])) {
  //   if (i2c_rx_data[0] == I2C_TH_ADDRESS_ADDRESS && rx_data_length == (I2C_TH_ADDRESS_LENGTH+2)) {
  //     is_i2c_data_ok = true;
  //   }
  //   else if (i2c_rx_data[0] == I2C_TH_ONESHOT_ADDRESS && rx_data_length == (I2C_TH_ONESHOT_LENGTH+2)) {
  //     is_i2c_data_ok = true;
  //   }
  //   else if (i2c_rx_data[0] == I2C_TH_CONTINUOUS_ADDRESS && rx_data_length == (I2C_TH_CONTINUOUS_LENGTH+2)) {
  //     is_i2c_data_ok = true;
  //   }
  //   else if (i2c_rx_data[0] == I2C_TH_TEMPERATURE_ADDRESS_ADDRESS && rx_data_length == (I2C_TH_TEMPERATURE_ADDRESS_LENGTH+2)) {
  //     is_i2c_data_ok = true;
  //   }
  //   else if (i2c_rx_data[0] == I2C_TH_HUMIDITY_ADDRESS_ADDRESS && rx_data_length == (I2C_TH_HUMIDITY_ADDRESS_LENGTH+2)) {
  //     is_i2c_data_ok = true;
  //   }
  //
  //   if (is_i2c_data_ok) {
  //     for (uint8_t i=2; i<rx_data_length; i++) {
  //       // write rx_data_length bytes in writable_data_ptr (base) at (i2c_rx_data[i] - I2C_WRITE_REGISTER_START_ADDRESS) (position in buffer)
  //       ((uint8_t *)writable_data_ptr)[i2c_rx_data[0] - I2C_WRITE_REGISTER_START_ADDRESS] = i2c_rx_data[i];
  //     }
  //   }
  // }
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

void init_wire() {
  //! clear the I2C bus first before calling Wire.begin()
  // uint8_t i2c_bus_state = I2C_ClearBus();
  //
  // if (i2c_bus_state) {
  //    SERIAL_ERROR(F("I2C bus error: Could not clear!!!\r\n"));
  //    //! wait for watchdog reboot
  //    while(1);
  // }
  //
  // switch (i2c_bus_state) {
  //    case 1:
  //       SERIAL_ERROR(F("SCL clock line held low\r\n"));
  //    break;
  //
  //    case 2:
  //       SERIAL_ERROR(F("SCL clock line held low by slave clock stretch\r\n"));
  //    break;
  //
  //    case 3:
  //       SERIAL_ERROR(F("SDA data line held low\r\n"));
  //    break;
  // }

  SERIAL_INFO(F("%u: Init Wire..\r\n"), I2C_MY_ADDRESS);

  i2c_rx_error_count = 0;
  i2c_rx_success_count = 0;

  i2c_tx_error_count = 0;
  i2c_tx_success_count = 0;

  i2c_rq_error_count = 0;
  i2c_rq_success_count = 0;

  reset_i2c_buffer((void *) rx_buffer, (uint8_t *) &rx_buffer_length);
  reset_i2c_buffer(tx_buffer, &tx_buffer_length);
  reset_i2c_buffer(rq_buffer, &rq_buffer_length);

  Wire.end();
  Wire.begin(I2C_MY_ADDRESS);
  Wire.setClock(I2C_BUS_CLOCK);

  digitalWrite(SDA, HIGH);
  digitalWrite(SCL, HIGH);

  Wire.onRequest(i2c_request_interrupt_handler);
  Wire.onReceive(i2c_receive_interrupt_handler);
}

void init_system() {
  #if (USE_POWER_DOWN)
  set_sleep_mode(SLEEP_MODE_IDLE);
  awakened_event_occurred_time_ms = millis();
  #endif

  //! main loop state
  state = INIT;
}

void init_tasks() {
  noInterrupts();

  //! no tasks ready
  ready_tasks_count = 0;

  start_print_status_delay_ms = millis();

  start_i2c_check_delay_ms = millis();

  //--------------------------------------------------------------------------------------------------------
  // MASTER reader <-- SLAVE writer
  //--------------------------------------------------------------------------------------------------------
  // MASTER reader: Lettura richiesti allo SLAVE (lo slave trasmette i dati)
  i2c_request_delay_ms = I2C_REQUEST_MAX_DELAY_MS;
  start_time_i2c_request_delay_ms = millis();

  //--------------------------------------------------------------------------------------------------------
  // MASTER writer --> SLAVE reader
  //--------------------------------------------------------------------------------------------------------
  // MASTER: invio dati allo slave
  i2c_send_delay_ms = I2C_SEND_DATA_MAX_DELAY_MS;
  start_time_i2c_send_delay_ms = millis();

  is_event_request_data = false;
  is_event_send_data = false;

  interrupts();
}

void init_sensors () {
}

void reset_i2c_buffer(void *buffer, uint8_t *length) {
  *length = 0;
  memset(buffer, 0, I2C_MAX_DATA_LENGTH);
}

void received_data () {
  bool is_rx_error = false;
  uint8_t crc = crc8((uint8_t *) rx_buffer, rx_buffer_length - 1);

  if (crc != rx_buffer[rx_buffer_length - 1]) {
    // SERIAL_ERROR(F("%u: [ %s ] RX: CRC mismatch %u != %u\r\n"), I2C_MY_ADDRESS, ERROR_STRING, crc, rx_buffer[rx_buffer_length - 1]);
    is_rx_error = true;
    i2c_rx_error_count++;
  }

  if (!is_rx_error) {
    i2c_rx_success_count++;
  }

  SERIAL_DEBUG(F("%u: Received %u bytes: "), I2C_MY_ADDRESS, rx_buffer_length);
  SERIAL_DEBUG_ARRAY_CLEAN((void *) rx_buffer, rx_buffer_length, UINT8, F("%u  "));
  SERIAL_DEBUG_CLEAN(F("\r\n"));
  reset_i2c_buffer((void *) rx_buffer, (uint8_t *) &rx_buffer_length);
}

void send_data (uint8_t write_data_length, uint8_t i2c_other_address) {
  tx_buffer_length = write_data_length;

  uint8_t start_value = 0;

  switch (I2C_MY_ADDRESS) {
    case I2C_ADDRESS_1: start_value = 0; break;
    case I2C_ADDRESS_2: start_value = 32; break;
    case I2C_ADDRESS_3: start_value = 64; break;
    case I2C_ADDRESS_4: start_value = 96; break;
  }

  for (uint8_t i = 0; i < write_data_length; i++) {
    tx_buffer[i] = start_value + i;
  }

  tx_buffer[write_data_length] = crc8(tx_buffer, write_data_length);

  SERIAL_DEBUG(F("%u: Send %u bytes: "), I2C_MY_ADDRESS, write_data_length + 1);
  SERIAL_DEBUG_ARRAY_CLEAN(tx_buffer, write_data_length + 1, UINT8, F("%u  "));
  SERIAL_DEBUG_CLEAN(F("\r\n"));

  Wire.beginTransmission(i2c_other_address);
  Wire.write(tx_buffer, write_data_length + 1);
  uint8_t status = Wire.endTransmission();

  if (status) {
    // SERIAL_ERROR(F("%u: [ %s ] TX: %u\r\n"), I2C_MY_ADDRESS, ERROR_STRING, status);
    i2c_tx_error_count++;
  } else {
    i2c_tx_success_count++;
  }

  // check_i2c_bus();
  reset_i2c_buffer(tx_buffer, &tx_buffer_length);
}

void request_data (uint8_t length, uint8_t i2c_other_address) {
  bool is_rq_error = false;

  Wire.requestFrom(i2c_other_address, length + 1);

  while  (Wire.requestAvailable() > 0) {
    rq_buffer[rq_buffer_length] = Wire.requestRead();
    rq_buffer_length++;
  }

  if (rq_buffer_length != (length + 1)) {
    i2c_rq_error_count++;
    is_rq_error = true;
  } else {
    uint8_t crc = crc8(rq_buffer, rq_buffer_length - 1);

    if (crc != rq_buffer[rq_buffer_length - 1]) {
      is_rq_error = true;
      i2c_rq_error_count++;
    }

    if (!is_rq_error) {
      i2c_rq_success_count++;
    }
  }

  if (is_rq_error) {
    if (rq_buffer_length != length + 1) {
      // Error: ricevuto una quantità errata di dati
      // SERIAL_ERROR(F("%u: [ %s ] RQ %u bytes but got %u bytes\r\n"), I2C_MY_ADDRESS, ERROR_STRING, length + 1, rq_buffer_length);
    } else {
      // Error: il numero di bytes ricevuti è corretto ma i dati sono corrotti
      // SERIAL_INFO(F("%u: [ %s ] RQ %u bytes: CRC mismatch!\r\n"), I2C_MY_ADDRESS, ERROR_STRING, rq_buffer_length);
    }
  } else {
    // No error: just print for debug request data
    SERIAL_DEBUG(F("%u: RQ %u bytes but got %u bytes: "), I2C_MY_ADDRESS, rq_buffer_length, length + 1);
    SERIAL_DEBUG_ARRAY_CLEAN((void *) rq_buffer, rq_buffer_length, UINT8, F("%u  "));
    SERIAL_DEBUG_CLEAN(F("\r\n"));
  }

  // check_i2c_bus();
  reset_i2c_buffer(rq_buffer, &rq_buffer_length);
}

void setup() {
  init_wdt(WDT_TIMER);
  SERIAL_BEGIN(115200);
  // init_buffers();
  init_wire();
  init_system();
  randomSeed(analogRead(0));
  SERIAL_INFO(F("\r\n%u: Partito...\r\n\r\n"), I2C_MY_ADDRESS);
  wdt_reset();
}

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

    //--------------------------------------------------------------------------------------------------------
    // MASTER writer --> SLAVE reader
    //--------------------------------------------------------------------------------------------------------

    // MASTER: invio dati allo slave
    if (!is_event_send_data && millis() - start_time_i2c_send_delay_ms >= i2c_send_delay_ms) {
      noInterrupts();
      if (!is_event_send_data) {
        is_event_send_data = true;
        ready_tasks_count++;
      }
      interrupts();

      start_time_i2c_send_delay_ms = millis();
      i2c_send_delay_ms = random(I2C_SEND_DATA_MIN_DELAY_MS, I2C_SEND_DATA_MAX_DELAY_MS);
    }

    if (is_event_send_data) {
      const uint8_t write_data_length = I2C_MAX_DATA_LENGTH;
      uint8_t i2c_other_address;

      // 1 --> 2 --> 3 --> 4 --> 1
      switch (I2C_MY_ADDRESS) {
        case I2C_ADDRESS_1: i2c_other_address = I2C_ADDRESS_2; break;
        case I2C_ADDRESS_2: i2c_other_address = I2C_ADDRESS_3; break;
        case I2C_ADDRESS_3: i2c_other_address = I2C_ADDRESS_4; break;
        case I2C_ADDRESS_4: i2c_other_address = I2C_ADDRESS_1; break;
        // case I2C_ADDRESS_1: i2c_other_address = I2C_ADDRESS_2; break;
        // case I2C_ADDRESS_2: i2c_other_address = I2C_ADDRESS_1; break;
      }

      send_data(write_data_length, i2c_other_address);

      noInterrupts();
      if (is_event_send_data) {
        is_event_send_data = false;
        ready_tasks_count--;
      }
      interrupts();

      wdt_reset();
    }

    // SLAVE: Lettura dati trasmessi dal MASTER
    if (rx_buffer_length) {
      received_data();
      wdt_reset();
    }

    //--------------------------------------------------------------------------------------------------------
    // MASTER reader <-- SLAVE writer
    //--------------------------------------------------------------------------------------------------------

    // MASTER reader: Lettura richiesti allo SLAVE (lo slave trasmette i dati)
    if (!is_event_request_data && millis() - start_time_i2c_request_delay_ms >= i2c_request_delay_ms) {
      noInterrupts();
      if (!is_event_request_data) {
        is_event_request_data = true;
        ready_tasks_count++;
      }
      interrupts();

      start_time_i2c_request_delay_ms = millis();
      i2c_request_delay_ms = random(I2C_REQUEST_MIN_DELAY_MS, I2C_REQUEST_MAX_DELAY_MS);
    }

    if (is_event_request_data) {
      const uint8_t length = I2C_MAX_DATA_LENGTH;
      uint8_t i2c_other_address;

      // 4 --> 3 --> 2 --> 1 --> 4
      switch (I2C_MY_ADDRESS) {
        case I2C_ADDRESS_4: i2c_other_address = I2C_ADDRESS_3; break;
        case I2C_ADDRESS_3: i2c_other_address = I2C_ADDRESS_2; break;
        case I2C_ADDRESS_2: i2c_other_address = I2C_ADDRESS_1; break;
        case I2C_ADDRESS_1: i2c_other_address = I2C_ADDRESS_4; break;
        // case I2C_ADDRESS_1: i2c_other_address = I2C_ADDRESS_2; break;
        // case I2C_ADDRESS_2: i2c_other_address = I2C_ADDRESS_1; break;
      }

      request_data(length, i2c_other_address);

      noInterrupts();
      if (is_event_request_data) {
        is_event_request_data = false;
        ready_tasks_count--;
      }
      interrupts();

      wdt_reset();
    }

    // Only for debug
    if (millis() - start_print_status_delay_ms >= PRINT_STATUS_DELAY_MS) {
      start_print_status_delay_ms = millis();
      SERIAL_INFO(F("%u: I2C [ ERR/SUC ]:\t\tRX [ %u/%u ]\t\tTX [ %u/%u ]\t\tRQ [ %u/%u ]\t\tTT [ %u/%u ]\r\n"), I2C_MY_ADDRESS, i2c_rx_error_count, i2c_rx_success_count, i2c_tx_error_count, i2c_tx_success_count, i2c_rq_error_count, i2c_rq_success_count, i2c_rx_error_count + i2c_tx_error_count + i2c_rq_error_count, i2c_rx_success_count + i2c_tx_success_count + i2c_rq_success_count);
    }

    // I2C Bus Check
    if (millis() - start_i2c_check_delay_ms >= I2C_CHECK_DELAY_MS) {
      start_i2c_check_delay_ms = millis();
      check_i2c_bus();
    }

    // ALTRO
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
