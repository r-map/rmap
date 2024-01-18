/**@file stima.ino */

/*********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>
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
#if (DEBUG_MEMORY)
#ifdef ARDUINO_ARCH_AVR
#include <MemoryUsage.h>
STACK_DECLARE
#endif
#endif

#include "stima.h"

/*!
\fn void setup()
\brief Arduino setup function. Init watchdog, hardware, debug, buffer and load configuration stored in EEPROM.
\return void.*/
void setup() {
   #if (DEBUG_MEMORY)
   #ifdef ARDUINO_ARCH_AVR
   post_StackPaint();
   #endif
   #endif
   init_wdt(WDT_TIMER);
   Serial.begin(115200);
   init_pins();
   init_wire();
   init_spi();
   init_rpc();
   init_tasks();
   init_logging();
   #if (USE_LCD)
   init_lcd();
   wdt_reset();
   #endif
   #if (USE_RTC)
   init_rtc();
   wdt_reset();
   #elif (USE_TIMER_1)
   init_timer1();
   wdt_reset();
   #endif
   init_buffers();
   load_configuration();
   init_system();
   wdt_reset();

   #if (USE_LCD)
   lcd_error |= lcd.print(F("--- www.rmap.cc ---"))==0;
   lcd_error |= lcd.setCursor(0, 1);
   lcd_error |= lcd.print(F("Stima station"))==0;
   wdt_reset();

   lcd_error |= lcd.setCursor(0, 2);
   lcd_error |= lcd.print(stima_name)==0;
   lcd_error |= lcd.print(F(" V: "))==0;
   lcd_error |= lcd.print(MODULE_MAIN_VERSION)==0;
   lcd_error |= lcd.print(F("."))==0;
   lcd_error |= lcd.print(MODULE_MINOR_VERSION)==0;
   wdt_reset();

   lcd_error |= lcd.setCursor(0, 3);
   lcd_error |= lcd.print(F("Configuration V: "))==0;
   lcd_error |= lcd.print(MODULE_CONFIGURATION_VERSION)==0;
   wdt_reset();

   delay(5000);
   wdt_reset();
   
   lcd_error |= lcd.clear();
   for (uint8_t i=0; i<readable_configuration.constantdata_count && i<2; i++) {
     lcd_error |= lcd.setCursor(0, i*2);
     lcd_error |= lcd.print(readable_configuration.constantdata[i].btable)==0;
     lcd_error |= lcd.print(":")==0;
     lcd_error |= lcd.print(readable_configuration.constantdata[i].value)==0;
     wdt_reset();
   }
   
   #endif

   delay(5000);  // wait other board go ready
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
         init_sensors();
         wdt_reset();
         state = TASKS_EXECUTION;
	 LOGV(F("INIT ---> TASKS_EXECUTION"));
      break;

      #if (USE_POWER_DOWN)
      case ENTER_POWER_DOWN:
         #if (ENABLE_SDCARD_LOGGING)
	 logFile.flush();
	 #endif
	 Serial.flush();

         init_power_down(&awakened_event_occurred_time_ms, DEBOUNCING_POWER_DOWN_TIME_MS);
         state = TASKS_EXECUTION;
	 LOGV(F("ENTER_POWER_DOWN ---> TASKS_EXECUTION"));
      break;
      #endif

      case TASKS_EXECUTION:
        // I2C Bus Check
        if (i2c_error >= I2C_MAX_ERROR_COUNT) {
          LOGE(F("Restart I2C BUS"));
          reset_wire();
          wdt_reset();
        }

	//LOGV("is_event_rtc:%t,is_event_supervisor:%t,is_event_gsm:%t,is_event_sensors_reading:%t,is_event_data_saving:%t,is_event_mqtt:%t,is_event_time:%t,is_event_rpc:%t,have_to_reboot:%t",
	//     is_event_rtc,is_event_supervisor,is_event_gsm,is_event_sensors_reading,is_event_data_saving,is_event_mqtt,is_event_time,is_event_rpc,have_to_reboot);

        if (is_event_rtc) {
          rtc_task();
          wdt_reset();
        }

        if (is_event_supervisor) {
          supervisor_task();
          wdt_reset();
        }

        #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
        if (is_event_ethernet) {
          ethernet_task();
          wdt_reset();
        }

        #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
        if (is_event_gsm) {
          gsm_task();
          wdt_reset();
        }

        #endif

        if (is_event_sensors_reading) {
          sensors_reading_task();
          wdt_reset();
        }

        #if (USE_SDCARD)
        if (is_event_data_saving) {
          data_saving_task();
          wdt_reset();
        }
        #endif

        #if (USE_MQTT)
        if (is_event_mqtt) {
          mqtt_task();
          wdt_reset();
	}
        #endif

        if (is_event_time) {
          time_task();
          wdt_reset();
        }

        streamRpc.parseStream(&is_event_rpc, &Serial);
        if (is_event_rpc) {
          wdt_reset();
        }

        if ((ready_tasks_count == 0) && (!is_event_rpc)) {
          wdt_reset();
	  if (have_to_reboot) {
	    state = REBOOT;
	    LOGV(F("TASK_EXECUTION ---> REBOOT"));
	  }else{
	    state = END;
	    LOGV(F("TASK_EXECUTION ---> END"));
	  }
        }
      break;

      case END:
         #if (DEBUG_MEMORY)
         #ifdef ARDUINO_ARCH_AVR
	 //SRamDisplay();
	 LOGN(F("Stack painted free: %d"), post_StackCount());
	 //STACKPAINT_PRINT
	 //MEMORY_PRINT_START
	 //MEMORY_PRINT_HEAPSTART
	 //MEMORY_PRINT_HEAPEND
	 //MEMORY_PRINT_STACKSTART
	 //MEMORY_PRINT_END
	 //MEMORY_PRINT_HEAPSIZE
	 //FREERAM_PRINT;
	 #endif
	 #endif

         #if (USE_POWER_DOWN)
         state = ENTER_POWER_DOWN;
	 LOGV(F("END ---> ENTER_POWER_DOWN"));
         #else
         state = TASKS_EXECUTION;
	 LOGV(F("END ---> TASK_EXECUTION"));
         #endif
      break;

      case REBOOT:
	if (strlen(rpcpayload) == 0 || !mqtt_client.isConnected()){
	  LOGN(F("Reboot"));
	  if (mqtt_client.isConnected()){
	    mqtt_client.yield(6000L);
	    wdt_reset();
	    mqtt_client.disconnect();
	    wdt_reset();
	  }
	  ipstack.disconnect();
	  wdt_reset();
	  LOGT(F("MQTT Disconnect... [ %s ]"), OK_STRING);

          #if (USE_SDCARD)
	  SD.end();
	  is_sdcard_open=false;
          #endif

	  realreboot();
	}
      break;

   }
}


void logPrefix(Print* _logOutput) {
  char dt[DATE_TIME_STRING_LENGTH];
  time_t date_time=now();
  snprintf(dt, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u", year(date_time), month(date_time), day(date_time), hour(date_time), minute(date_time), second(date_time));
  _logOutput->print("#");
  _logOutput->print(dt);
  _logOutput->print(" ");
}

void logSuffix(Print* _logOutput) {
  _logOutput->print('\n');
  _logOutput->flush();  // we use this to flush every log message
}


void init_logging(){
   #if (ENABLE_SDCARD_LOGGING)
   if (!is_sdcard_open) {
     if (sdcard_init(&SD, SDCARD_CHIP_SELECT_PIN)) {
       is_sdcard_open = true;
       is_sdcard_error = false;
     }
   }

   if (is_sdcard_open && sdcard_open_file(&SD, &logFile, SDCARD_LOGGING_FILE_NAME, O_RDWR | O_CREAT | O_APPEND)) {
     logFile.seekEnd(0);
     Log.begin(LOG_LEVEL, &loggingStream);
   } else {
     Log.begin(LOG_LEVEL, &Serial);
   }
   #else
     Log.begin(LOG_LEVEL, &Serial);
   #endif

   Log.setPrefix(logPrefix); // Uncomment to get timestamps as prefix
   Log.setSuffix(logSuffix); // Uncomment to get newline as suffix

   LOGF(F("Logging started"));
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

void init_buffers() {
}

void init_tasks() {
   ready_tasks_count = 0;

   is_event_supervisor = true;
   supervisor_state = SUPERVISOR_INIT;
   ready_tasks_count++;

   is_event_rpc = true;

   is_event_time = false;
   time_state = TIME_INIT;

   is_event_sensors_reading = false;
   is_event_sensors_reading_rpc = false;
   sensors_reading_state = SENSORS_READING_INIT;

   #if (USE_SDCARD)
   is_event_data_saving = false;
   data_saving_state = DATA_SAVING_INIT;
   is_sdcard_error = false;
   is_sdcard_open = false;
   #endif

   #if (USE_MQTT)
   is_event_mqtt = false;
   is_event_mqtt_paused = false;
   mqtt_state = MQTT_INIT;
   mqtt_session_present=false;
   #endif

   is_event_rtc = false;

   #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
   is_event_ethernet = false;
   ethernet_state = ETHERNET_INIT;

   #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
   is_event_gsm = false;
   gsm_state = GSM_INIT;

   #endif

   rpc_state = RPC_INIT;

   is_client_connected = false;
   is_client_udp_socket_open = false;

   do_ntp_sync = false;
   is_time_set = false;
   system_time = 0;
   last_ntp_sync = -NTP_TIME_FOR_RESYNC_S;

   #if (USE_LCD)
   last_lcd_begin = 0;
   #endif
   is_time_for_sensors_reading_updated = false;

   have_to_reboot = false;
   is_datetime_set = false;
}

void init_pins() {
   pinMode(CONFIGURATION_RESET_PIN, INPUT_PULLUP);

   pinMode(RTC_INTERRUPT_PIN, INPUT_PULLUP);

   #if (USE_SDCARD)
   pinMode(SDCARD_CHIP_SELECT_PIN, OUTPUT);
   digitalWrite(SDCARD_CHIP_SELECT_PIN, HIGH);
   #endif

   #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
   Ethernet.w5500_cspin = W5500_CHIP_SELECT_PIN;

   #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
   s800.init(GSM_ON_OFF_PIN);
   s800.setTimeout(IP_STACK_TIMEOUT_MS);

   #endif
}

void i2c_receive_interrupt_handler(int rx_data_length) {
  uint8_t i2c_rx_data[I2C_MAX_DATA_LENGTH];
  
  // read rx_data_length bytes of data from i2c bus
  for (uint8_t i = 0; i < rx_data_length; i++) {
    i2c_rx_data[i] = Wire.read();
  }

  if (rx_data_length < 2) {
    // no payload and CRC as for scan I2c bus
    // attention: logging inside ISR !
    //LOGN(F("No CRC: size %d"),rx_data_length);
    //! check crc: ok

  } else if (i2c_rx_data[rx_data_length - 1] == crc8((uint8_t *)i2c_rx_data, rx_data_length - 1)) {
    rx_data_length--;
    
    /*
    // is it a registers read?
    if (rx_data_length == 2 && is_readable_register(i2c_rx_data[0])) {
    // offset in readable_data_read_ptr buffer
    readable_data_address = i2c_rx_data[0];
    
    // length (in bytes) of data to be read in readable_data_read_ptr
    readable_data_length = i2c_rx_data[1];
    // attention: logging inside ISR !
    //LOGV(F("set readable_data: %d-%d"),readable_data_address,readable_data_length);
      } else */
    // is it a command?
    if (rx_data_length == 2 && is_command(i2c_rx_data[0])) {
      // attention: logging inside ISR !
      //LOGN(F("receive a command"));
      if (i2c_rx_data[1]==I2C_MASTER_COMMAND_SAVE ){    // command to be executed is save
	LOGN(F("save command"));
	save_configuration(false);
	realreboot();
      }
      /* else {
      // attention: logging inside ISR !
      //LOGE(F("invalid command"));
      }
	*/
      
      // it is a registers write?
    } else if (is_writable_register(i2c_rx_data[0])) {

      // we have to manage others registers here (i2c address)
      // but they are not in configuration_t
      
      if (i2c_rx_data[0] == I2C_MASTER_CONFIGURATION_INDEX_ADDRESS &&
	  rx_data_length <= (I2C_MASTER_CONFIGURATION_INDEX_LENGTH + I2C_MASTER_CONFIGURATION_LENGTH )) {
	rx_data_length -= 1;
	
	uint16_t index;
	((uint8_t *)&index)[0] = i2c_rx_data[1];
	((uint8_t *)&index)[1] = i2c_rx_data[2];
	rx_data_length -= 2;
	
	//LOGN(F("index %d"),index);
	
	for (int16_t i = 0; i <  rx_data_length; i++) {
	  if ((i + index)< sizeof(writable_configuration)){
	    ((uint8_t *)&writable_configuration)[i + index] = i2c_rx_data[i+3];
	    //  LOGV(F("set writable register OK"));
	    //}else{
	    //LOGE(F("set writable register FAILED"));	    
	  }
	}
	//}else{
	  //LOGE(F("writable register not conform"));
      }
      
    } else {
      //readable_data_address=0xFF;
      //readable_data_length = 0;
      // attention: logging inside ISR !
      //LOGE(F("CRC error: size %d  CRC %d:%d"),rx_data_length,i2c_rx_data[rx_data_length - 1], crc8((uint8_t *)(i2c_rx_data), rx_data_length - 1));
      i2c_error++;
    }
  }
}

void init_wire() {
   i2c_error = 0;
   Wire.begin(I2C_MASTER_DEFAULT_ADDRESS); // configuration by I2C registers ignored !
   Wire.setClock(I2C_BUS_CLOCK);
   Wire.onReceive(i2c_receive_interrupt_handler);
}


void reset_wire() {
  uint8_t i2c_bus_state = I2C_ClearBus(); // clear the I2C bus first before calling Wire.begin()

   switch (i2c_bus_state) {
   case 1:
     LOGE(F("SCL clock line held low"));
     break;

   case 2:
     LOGE(F("SCL clock line held low by slave clock stretch"));
     break;

   case 3:
     LOGE(F("SDA data line held low"));
     break;
   }

   /*
   if (i2c_bus_state) {
     LOGE(F("I2C bus error: Could not clear!!!"));
     //while(1);
    have_to_reboot = true;
   }
   */

#ifdef ARDUINO_ARCH_AVR
   Wire.end();
#endif
   init_wire();
}

void init_spi() {
   SPI.begin();
}


#if (USE_LCD)
void init_lcd() {
  lcd.setAddr(LCD_I2C_ADDRESS);
  if(lcd.begin(LCD_COLUMNS, LCD_ROWS)) // non zero status means it was unsuccesful
    {
      wdt_reset();
      LOGN(F(" Error initializing LCD primary addr"));

      lcd.setAddr(LCD_I2C_SECONDARY_ADDRESS);
      if(lcd.begin(LCD_COLUMNS, LCD_ROWS)) // non zero status means it was unsuccesful
	{
	  wdt_reset();
	  LOGE(F(" Error initializing LCD"));
	  return;
	}
    }
  
  lcd.clear();
  lcd.lineWrap();
    //lcd.autoscroll();
  lcd_error=false;
}
#endif

#if (USE_RTC)

bool set_datetime_rtc(const time_t time){

  tmElements_t tm;
  breakTime(time,tm);
  tm.Year = tmYearToY2k(tm.Year);
  return Pcf8563::setDateTime(tm.Hour,
			      tm.Minute,
			      tm.Second,
			      tm.Day,
			      tm.Month,
			      tm.Year,0,0);
}

bool get_datetime_rtc(time_t &time){

  tmElements_t tm;

  if (Pcf8563::getDateTime(&tm.Hour,
			   &tm.Minute,
			   &tm.Second,
			   &tm.Day,
			   &tm.Month,
			   &tm.Year)){

    tm.Year = y2kYearToTm(tm.Year);
    time = makeTime(tm);
    return true;
  }
  time=0;
  return false;
}

void init_rtc() {
   Pcf8563::disableAlarm();
   Pcf8563::disableTimer();
   Pcf8563::disableClockout();
   Pcf8563::setClockoutFrequency(RTC_FREQUENCY);
   Pcf8563::enableClockout();
   attachInterrupt(digitalPinToInterrupt(RTC_INTERRUPT_PIN), rtc_interrupt_handler, RISING);

   time_t datetime;
   if (get_datetime_rtc(datetime)){
     system_time=datetime;
     setTime(system_time);
     is_datetime_set = true;
     LOGN(F("Current RTC date and time: %d/%d/%d %d:%d:%d"), day(system_time), month(system_time), year(system_time), hour(system_time), minute(system_time), second(system_time));
   }
}
#endif

void init_system() {
   #if (USE_POWER_DOWN)
   #if (USE_RTC)
   set_sleep_mode(SLEEP_MODE_PWR_DOWN);
   #elif (USE_TIMER_1)
   set_sleep_mode(SLEEP_MODE_IDLE);
   #endif
   awakened_event_occurred_time_ms = millis();
   #endif

   // main loop state
   state = INIT;
}

void init_rpc() {
   #if (USE_RPC_METHOD_CONFIGURE)
   streamRpc.registerMethod("configure", &configure);
   #endif

   #if (USE_RPC_METHOD_PREPARE)
   streamRpc.registerMethod("prepare", &prepare);
   #endif

   #if (USE_RPC_METHOD_GETJSON)
   streamRpc.registerMethod("getjson", &getjson);
   #endif

   #if (USE_RPC_METHOD_PREPANDGET)
   streamRpc.registerMethod("prepandget", &prepandget);
   #endif

   #if (USE_RPC_METHOD_REBOOT)
   streamRpc.registerMethod("reboot", &reboot);
   #endif

   #if (USE_RPC_METHOD_RECOVERY && USE_MQTT)
   streamRpc.registerMethod("recovery", &recovery);
   #endif

}

void init_wdt(uint8_t wdt_timer) {
   wdt_disable();
   wdt_reset();
   wdt_enable(wdt_timer);
}

#if (USE_TIMER_1)
void init_timer1() {
   start_timer();
}

void start_timer() {
  TCCR1A = 0x00;                //!< Normal timer operation
  TCCR1B = (1<<CS10) | (1<<CS12);        //!< 1:1024 prescaler
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

/*!
\fn ISR(TIMER1_OVF_vect)
\brief Timer1 overflow interrupts routine.
\return void.
*/
ISR(TIMER1_OVF_vect) {
   //! Pre-load timer counter register
   TCNT1 = TIMER1_TCNT1_VALUE;
   interrupt_task_1s();
}

#elif (USE_RTC)
void rtc_interrupt_handler() {
   interrupt_task_1s();
}

#endif

void init_sensors () {
  do_reset_first_run = false;
  is_first_run = true;
  is_first_test = true;
  is_test = false;
  uint8_t sensors_error_count = 0;

  if (readable_configuration.sensors_count) {

    #if (USE_LCD)
    lcd_error |= lcd.clear();
    lcd_error |= lcd.setCursor(0, 0);
    #endif

    // read sensors configuration, create and setup
    for (uint8_t i=0; i<readable_configuration.sensors_count; i++) {

      LOGN(F("try --> %d: %s-%s [ 0x%x ]: %s"), i,
	   readable_configuration.sensors[i].driver,
	   readable_configuration.sensors[i].type,
	   readable_configuration.sensors[i].address,
	   readable_configuration.sensors[i].mqtt_topic);

      SensorDriver::createAndSetup(readable_configuration.sensors[i].driver,
				   readable_configuration.sensors[i].type,
				   readable_configuration.sensors[i].address,
				   readable_configuration.sensors[i].node,
				   sensors, &sensors_count);
      wdt_reset();
    }

    for (uint8_t i=0; i<sensors_count; i++) {
      if (sensors[i] == NULL) {
        sensors_error_count++;
	continue;
      }
      LOGN(F("created --> %d: %s-%s [ 0x%x ] set: %s"), i,
	   sensors[i]->getDriver(),
	   sensors[i]->getType(),
	   sensors[i]->getAddress(),
	   sensors[i]->isSetted() ? OK_STRING : FAIL_STRING);

      if (!sensors[i]->isSetted()) {
        sensors_error_count++;
	wdt_reset();
        #if (USE_LCD)
	lcd_error |= lcd.print(readable_configuration.sensors[i].type)==0;
	lcd_error |= lcd.print(":")==0;
	lcd_error |= lcd.print(FAIL_STRING)==0;
	lcd_error |= lcd.print(";")==0;
	#endif
      }
    }
    #if (USE_LCD)
    wdt_reset();
    delay(5000);
    wdt_reset();
    lcd_error |= lcd.clear();
    lcd_error |= lcd.setCursor(0, 0);
    lcd_error |= lcd.print(F("Sensor: "))==0;
    lcd_error |= lcd.print(readable_configuration.sensors_count-sensors_error_count)==0;
    lcd_error |= lcd.print(F("/"))==0;
    lcd_error |= lcd.print(readable_configuration.sensors_count)==0;
    lcd_error |= lcd.print((sensors_error_count == 0) ? " OK" : " KO")==0;
    lcd_error |= lcd.setCursor(0, 1);
    lcd_error |= lcd.print("user  : ")==0;
    lcd_error |= lcd.print(readable_configuration.mqtt_username)==0;
    lcd_error |= lcd.setCursor(0, 2);
    lcd_error |= lcd.print("statio: ")==0;
    lcd_error |= lcd.print(readable_configuration.stationslug)==0;
    lcd_error |= lcd.setCursor(0, 3);
    lcd_error |= lcd.print("board : ")==0;
    lcd_error |= lcd.print(readable_configuration.boardslug)==0;
    #endif
  }
}

void setNextTimeForSensorReading (time_t *next_time, uint16_t time_s) {
   time_t counter = (now() / time_s);
   *next_time = (time_t) ((++counter) * time_s);
}

#if (USE_MQTT)
bool mqttConnect(char *username, char *password) {
   MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
   MQTT::connackData data ;
   options.MQTTVersion = 4;   // Version of MQTT to be used.  3 = 3.1 4 = 3.1.1
   options.will.topicName.cstring = maint_topic;
   options.will.message.cstring = (char *) MQTT_ON_ERROR_MESSAGE;
   options.will.retained = true;
   options.will.qos = MQTT::QOS1;
   options.willFlag = true;
   options.clientID.cstring = client_id;
   options.username.cstring = username;
   options.password.cstring = password;
   options.cleansession = !mqtt_session_present;

   LOGT(F("MQTT clientID: %s"), options.clientID.cstring);
   LOGT(F("MQTT will message: %s"), options.will.message.cstring);
   LOGT(F("MQTT will topic: %s"), options.will.topicName.cstring);
   LOGT(F("MQTT cleansession: %s"), options.cleansession ? "true" : "false");

   bool returnstatus = (mqtt_client.connect(options,data) == 0);

   if (returnstatus) {
     LOGT(F("MQTT sessionPresent: %s"), data.sessionPresent ? "true" : "false");
     mqtt_session_present=data.sessionPresent;
   }

   wdt_reset();
   return returnstatus;
}

bool mqttPublish(const char *topic, const char *message, bool is_retained) {
  bool is_ascii = true;
  bool status = true;

  // BUG: check if all characters are printable ASCII
  for (uint8_t i = 0; i < strlen(topic); i++) {
    if ((topic[i] < 0x20) || (topic[i] > 0x7E)) {
      is_ascii = false;
      break;
    }
  }

  for (uint8_t i = 0; i < strlen(message); i++) {
    if ((message[i] < 0x20) || (message[i] > 0x7E)) {
      is_ascii = false;
      break;
    }
  }

  // if yes publish it to MQTT otherwise they are discarded (corrupted message)
  if (is_ascii) {
    MQTT::Message tx_message;
    tx_message.qos = MQTT::QOS1;
    tx_message.retained = is_retained;
    tx_message.dup = false;
    tx_message.payload = (void*) message;
    tx_message.payloadlen = strlen(message);

    LOGV(F("MQTT TX: %s"), (char*)tx_message.payload);
    LOGV(F("--> len %d"), tx_message.payloadlen);

    MQTT::returnCode rc = (MQTT::returnCode) mqtt_client.publish(topic, tx_message);
    switch (rc){
    	case MQTT::BUFFER_OVERFLOW:
     		LOGV(F("publish BUFFER_OVERFLOW"));
     	break;
   	case MQTT::FAILURE:
     		LOGV(F("publish FAILURE"));
        break;
   	case MQTT::SUCCESS:
     		LOGV(F("publish SUCCESS"));
     	break;
     }
    status = (rc == MQTT::SUCCESS);
  }else{
    LOGE(F("skip not printable ASCII message"));
  }

  wdt_reset();

  return status;
}

void mqttRxCallback(MQTT::MessageData &md) {
  MQTT::Message &rx_message = md.message;

  LOGT(F("MQTT RX: %s"), (char*)rx_message.payload);
  LOGV(F("--> len %d"), rx_message.payloadlen);
  LOGV(F("--> qos %d"), rx_message.qos);
  LOGV(F("--> retained %d"), rx_message.retained);
  LOGV(F("--> dup %d"), rx_message.dup);
  LOGV(F("--> id %d"), rx_message.id);

  is_event_rpc=true;
  while (is_event_rpc) {                                  // here we block because pahoMQTT is blocking
    streamRpc.parseCharpointer(&is_event_rpc, (char*)rx_message.payload, rx_message.payloadlen, rpcpayload, MQTT_RPC_RESPONSE_LENGTH );
  }
}
#endif

void print_configuration() {
   getStimaNameByType(stima_name, readable_configuration.module_type);
   LOGN(F("--> type: %s"), stima_name);
   LOGN(F("--> version: %d.%d"), MODULE_MAIN_VERSION, MODULE_MINOR_VERSION);
   LOGN(F("--> configuration version: %d.%d"), readable_configuration.module_main_version, readable_configuration.module_configuration_version);
   LOGN(F("--> sensors: %d"), readable_configuration.sensors_count);
   LOGN(F("--> ConstantData: %d"), readable_configuration.constantdata_count);
   for (uint8_t i=0; i<readable_configuration.constantdata_count; i++) {
     LOGN(F("--> CD %d:  %s : %s"), i,readable_configuration.constantdata[i].btable,readable_configuration.constantdata[i].value);
   }
   #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
   LOGN(F("--> dhcp: %s"), readable_configuration.is_dhcp_enable ? "on" : "off");
   LOGN(F("--> ethernet mac: %02X:%02X:%02X:%02X:%02X:%02X"), readable_configuration.ethernet_mac[0], readable_configuration.ethernet_mac[1], readable_configuration.ethernet_mac[2], readable_configuration.ethernet_mac[3], readable_configuration.ethernet_mac[4], readable_configuration.ethernet_mac[5]);

   #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
   LOGN(F("--> gsm apn: %s"), readable_configuration.gsm_apn);
   LOGN(F("--> gsm username: %s"), readable_configuration.gsm_username);
   LOGN(F("--> gsm password: %s"), readable_configuration.gsm_password);

   #endif

   #if (USE_NTP)
   LOGN(F("--> ntp server: %s"), readable_configuration.ntp_server);
   #endif

   #if (USE_MQTT)
   LOGN(F("--> mqtt server: %s"), readable_configuration.mqtt_server);
   LOGN(F("--> mqtt port: %d"), readable_configuration.mqtt_port);
   LOGN(F("--> mqtt root topic: %s"), readable_configuration.mqtt_root_topic);
   LOGN(F("--> mqtt maint topic: %s"), readable_configuration.mqtt_maint_topic);
   LOGN(F("--> mqtt rpc topic: %s"), readable_configuration.mqtt_rpc_topic);
   LOGN(F("--> mqtt username: %s"), readable_configuration.mqtt_username);
   LOGN(F("--> mqtt password: %s"), readable_configuration.mqtt_password);
   LOGN(F("--> station slug: %s"), readable_configuration.stationslug);
   LOGN(F("--> board   slug: %s"), readable_configuration.boardslug);
   #endif
}

void set_default_configuration() {
   writable_configuration.module_type = MODULE_TYPE;
   writable_configuration.module_main_version = MODULE_MAIN_VERSION;
   writable_configuration.module_configuration_version = MODULE_CONFIGURATION_VERSION;

   writable_configuration.report_seconds = 900;

   writable_configuration.sensors_count = 0;
   memset(writable_configuration.sensors, 0, sizeof(sensor_t) * SENSORS_MAX);

   writable_configuration.constantdata_count = 0;

   #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
   char temp_string[20];
   writable_configuration.is_dhcp_enable = CONFIGURATION_DEFAULT_ETHERNET_DHCP_ENABLE;
   strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_MAC);
   macStringToArray(writable_configuration.ethernet_mac, temp_string);
   strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_IP);
   ipStringToArray(writable_configuration.ip, temp_string);
   strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_NETMASK);
   ipStringToArray(writable_configuration.netmask, temp_string);
   strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_GATEWAY);
   ipStringToArray(writable_configuration.gateway, temp_string);
   strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_PRIMARY_DNS);
   ipStringToArray(writable_configuration.primary_dns, temp_string);

   #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
   strcpy(writable_configuration.gsm_apn, CONFIGURATION_DEFAULT_GSM_APN);
   strcpy(writable_configuration.gsm_username, CONFIGURATION_DEFAULT_GSM_USERNAME);
   strcpy(writable_configuration.gsm_password, CONFIGURATION_DEFAULT_GSM_PASSWORD);

   #endif

   #if (USE_NTP)
   strcpy(writable_configuration.ntp_server, CONFIGURATION_DEFAULT_NTP_SERVER);
   #endif

   #if (USE_MQTT)
   writable_configuration.mqtt_port = CONFIGURATION_DEFAULT_MQTT_PORT;
   strcpy(writable_configuration.mqtt_server, CONFIGURATION_DEFAULT_MQTT_SERVER);
   strcpy(writable_configuration.mqtt_root_topic, CONFIGURATION_DEFAULT_MQTT_ROOT_TOPIC);
   strcpy(writable_configuration.mqtt_maint_topic, CONFIGURATION_DEFAULT_MQTT_MAINT_TOPIC);
   strcpy(writable_configuration.mqtt_rpc_topic, CONFIGURATION_DEFAULT_MQTT_RPC_TOPIC);
   strcpy(writable_configuration.mqtt_username, CONFIGURATION_DEFAULT_MQTT_USERNAME);
   strcpy(writable_configuration.mqtt_password, CONFIGURATION_DEFAULT_MQTT_PASSWORD);
   strcpy(writable_configuration.stationslug, CONFIGURATION_DEFAULT_STATIONSLUG);
   strcpy(writable_configuration.boardslug, CONFIGURATION_DEFAULT_BOARDSLUG);
   #endif

   LOGN(F("Reset configuration to default value... [ %s ]"), OK_STRING);
}

void save_configuration(bool is_default) {
   if (is_default) {
      set_default_configuration();
      LOGN(F("Set default configuration..."));
   }
   
   if (writable_configuration.module_type != MODULE_TYPE || writable_configuration.module_main_version != MODULE_MAIN_VERSION
       || writable_configuration.module_configuration_version != MODULE_CONFIGURATION_VERSION
       ) {
     LOGE(F("error configuration mismatch, do not save"));
     return;
   }

   wdt_reset();
   EEPROM.put(CONFIGURATION_EEPROM_ADDRESS, writable_configuration);
   wdt_reset();

   #if (USE_SDCARD)
   // try to open file. if ok, write configuration data.
   if (!is_sdcard_open) {
     if (sdcard_init(&SD, SDCARD_CHIP_SELECT_PIN)) {
       is_sdcard_open = true;
       is_sdcard_error = false;
     } else {
       LOGE(F("error opening SDcard"));
     }
   }

   if (is_sdcard_open) {
     File configuration_ptr_file;
     if (sdcard_open_file(&SD, &configuration_ptr_file, SDCARD_CONFIG_SAVED_FILE_NAME, O_WRONLY| O_CREAT)) {       
       if (configuration_ptr_file.write(&writable_configuration, sizeof(writable_configuration)) == sizeof(writable_configuration)) {
	 configuration_ptr_file.flush();
       } else {
	 LOGE(F("error writing config file"));
       }
     } else {
       LOGE(F("error opening backup config file"));
     }
     configuration_ptr_file.close();
   }

   wdt_reset();
   #endif
   LOGN(F("Save configuration... [ %s ]"), OK_STRING);
}

void load_configuration() {

  EEPROM.get(CONFIGURATION_EEPROM_ADDRESS, writable_configuration);  

  if (digitalRead(CONFIGURATION_RESET_PIN) == LOW) {
    #if (USE_SDCARD)
    if (!is_sdcard_open) {
      if (sdcard_init(&SD, SDCARD_CHIP_SELECT_PIN)) {
	is_sdcard_open = true;
	is_sdcard_error = false;
      } else {
	LOGE(F("error opening SDcard"));
      }
    }
     
    if(is_sdcard_open){
      if (SD.exists(SDCARD_CONFIG_FILE_NAME)) {
	LOGN(F("file %s exists"),SDCARD_CONFIG_FILE_NAME );
	// try to open file. if ok, read configuration data.
	File configuration_ptr_file;
	if (sdcard_open_file(&SD, &configuration_ptr_file, SDCARD_CONFIG_FILE_NAME, O_RDONLY)) {
	  if (configuration_ptr_file.read(&writable_configuration, sizeof(writable_configuration)) == sizeof(writable_configuration)) {
	    configuration_ptr_file.close();
	    // rename file coming from recovery rpc if exist
	    SD.remove(SDCARD_CONFIG_SAVED_FILE_NAME);
	    if (SD.rename(SDCARD_CONFIG_FILE_NAME, SDCARD_CONFIG_SAVED_FILE_NAME)) {
	    LOGN(F("configuration file read and renamed"));
	      save_configuration(false);
	    } 
	  }else{
	    LOGE(F("error reading configuration file"));
	  }
	  configuration_ptr_file.close();
	} else {
	  LOGE(F("error opening configuration file"));
	}	 
      } else {
	 LOGN(F("file %s do not exists"),SDCARD_CONFIG_FILE_NAME );
      }
    }
    #endif

    LOGN(F("Wait configuration..."));
    #if (USE_LCD)
    lcd_error |= lcd.clear();
    lcd_error |= lcd.print(F("Wait configuration"))==0;
    #endif
  }

  while (digitalRead(CONFIGURATION_RESET_PIN) == LOW) {
    streamRpc.parseStream(&is_event_rpc, &Serial);
    wdt_reset();
  }
  
  LOGN(F("Configuration received... [ %s ]"), OK_STRING);
      
  EEPROM.get(CONFIGURATION_EEPROM_ADDRESS, readable_configuration);  
  
  #if (USE_MQTT)
  
  getMqttClientId(readable_configuration.mqtt_username, readable_configuration.stationslug, readable_configuration.boardslug, client_id);   
  //getMqttClientIdFromMqttTopic(readable_configuration.mqtt_rpc_topic, client_id);
  getFullTopic(maint_topic, readable_configuration.mqtt_maint_topic, MQTT_STATUS_TOPIC);
  #endif
  
  LOGN(F("Load configuration... [ %s ]"), OK_STRING);
  print_configuration();
}

#if (USE_RPC_METHOD_PREPARE || USE_RPC_METHOD_PREPANDGET || USE_RPC_METHOD_GETJSON)
bool extractSensorsParams(JsonObject &params, char *driver, char *type, uint8_t *address, uint8_t *node) {
   bool is_error = false;

   for (JsonObject::iterator it = params.begin(); it != params.end(); ++it) {
      if (strcmp(it.key().c_str(), "driver") == 0) {
         strncpy(driver, it.value().as<const char*>(), DRIVER_LENGTH);
      }
      else if (strcmp(it.key().c_str(), "type") == 0) {
         strncpy(type, it.value().as<const char*>(), TYPE_LENGTH);
      }
      else if (strcmp(it.key().c_str(), "address") == 0) {
         *address = it.value().as<unsigned char>();
      }
      else if (strcmp(it.key().c_str(), "node") == 0) {
         *node = it.value().as<unsigned char>();
      }
      else {
         is_error = true;
      }
   }

   return is_error;
}
#endif

#if (USE_RPC_METHOD_CONFIGURE)
int configure(JsonObject params, JsonObject result) {
   bool is_error = false;
   bool is_sensor_config = false;

   //LOGN(F("params.isNull %s"), params.isNull() ? "true" : "false");
   if (params.isNull()) is_mqtt_rpc_delay=true;  // configure without params is used
                                                 // to set a long delay before disconnect
                                                 // after the data are sended
   for (JsonPair it : params) {
      if (strcmp(it.key().c_str(), "reset") == 0) {
         if (it.value().as<bool>() == true) {
	    set_default_configuration();
            #if (USE_LCD)
	    lcd_error |= lcd.clear();
            lcd_error |= lcd.print(F("Reset configuration"))==0;
	    #endif
         }
      }
      else if (strcmp(it.key().c_str(), "save") == 0) {
         if (it.value().as<bool>() == true) {
            save_configuration(CONFIGURATION_CURRENT);
            #if (USE_LCD)
	    lcd_error |= lcd.clear();
            lcd_error |= lcd.print(F("Save configuration"))==0;
	    #endif
         }
      }
      #if (USE_MQTT)
      else if (strcmp(it.key().c_str(), "mqttserver") == 0) {
         strncpy(writable_configuration.mqtt_server, it.value().as<const char*>(), MQTT_SERVER_LENGTH);
      }
      else if (strcmp(it.key().c_str(), "mqttrootpath") == 0) {
         strncpy(writable_configuration.mqtt_root_topic, it.value().as<const char*>(), MQTT_ROOT_TOPIC_LENGTH);
      }
      else if (strcmp(it.key().c_str(), "mqttrpcpath") == 0) {
         strncpy(writable_configuration.mqtt_rpc_topic, it.value().as<const char*>(), MQTT_RPC_TOPIC_LENGTH);
      }
      else if (strcmp(it.key().c_str(), "mqttmaintpath") == 0) {
         strncpy(writable_configuration.mqtt_maint_topic, it.value().as<const char*>(), MQTT_MAINT_TOPIC_LENGTH);
      }
      else if (strcmp(it.key().c_str(), "mqttsampletime") == 0) {
         writable_configuration.report_seconds = it.value().as<unsigned int>();
      }
      else if (strcmp(it.key().c_str(), "mqttuser") == 0) {
         strncpy(writable_configuration.mqtt_username, it.value().as<const char*>(), MQTT_USERNAME_LENGTH);
      }
      else if (strcmp(it.key().c_str(), "mqttpassword") == 0) {
         strncpy(writable_configuration.mqtt_password, it.value().as<const char*>(), MQTT_PASSWORD_LENGTH);
      }
      else if (strcmp(it.key().c_str(), "stationslug") == 0) {
         strncpy(writable_configuration.stationslug, it.value().as<const char*>(), STATIONSLUG_LENGTH);
      }
      else if (strcmp(it.key().c_str(), "boardslug") == 0) {
         strncpy(writable_configuration.boardslug, it.value().as<const char*>(), BOARDSLUG_LENGTH);
      }
      else if (strcmp(it.key().c_str(), "mqttpskkey") == 0) {
	//skip it
      }
      else if (strcmp(it.key().c_str(), "sd") == 0) {
	for (JsonPair sd : it.value().as<JsonObject>()) {
	  strncpy(writable_configuration.constantdata[writable_configuration.constantdata_count].btable, sd.key().c_str(), CONSTANTDATA_BTABLE_LENGTH);
	  strncpy(writable_configuration.constantdata[writable_configuration.constantdata_count].value, sd.value().as<const char*>(), CONSTANTDATA_VALUE_LENGTH);

	  if (writable_configuration.sensors_count < USE_CONSTANTDATA_COUNT)
	    writable_configuration.constantdata_count++;
	  else {
	    is_error = true;
	  }
	}

      }
      #endif
      #if (USE_NTP)
      else if (strcmp(it.key().c_str(), "ntpserver") == 0) {
         strncpy(writable_configuration.ntp_server, it.value().as<const char*>(), NTP_SERVER_LENGTH);
      }
      #endif
      else if (strcmp(it.key().c_str(), "date") == 0) {
         #if (USE_RTC)

	 tmElements_t tm;
	 tm.Year   = y2kYearToTm(it.value().as<JsonArray>()[0].as<int>()-2000);
	 tm.Month  = it.value().as<JsonArray>()[1].as<int>();
	 tm.Day    = it.value().as<JsonArray>()[2].as<int>();
	 tm.Hour   = it.value().as<JsonArray>()[3].as<int>();
	 tm.Minute = it.value().as<JsonArray>()[4].as<int>();
	 tm.Second = it.value().as<JsonArray>()[5].as<int>();

	 system_time = makeTime(tm);
	 setTime(system_time);
	 /*
         Pcf8563::disable();
         Pcf8563::setDate(it.value().as<JsonArray>()[2].as<int>(), it.value().as<JsonArray>()[1].as<int>(), it.value().as<JsonArray>()[0].as<int>() - 2000, weekday()-1, 0);
         Pcf8563::setTime(it.value().as<JsonArray>()[3].as<int>(), it.value().as<JsonArray>()[4].as<int>(), it.value().as<JsonArray>()[5].as<int>());
         Pcf8563::enable();
	 */
         setSyncInterval(NTP_TIME_FOR_RESYNC_S);
         setSyncProvider(getSystemTime);
         #elif (USE_TIMER_1)
         setTime(it.value().as<JsonArray>()[3].as<int>(), it.value().as<JsonArray>()[4].as<int>(), it.value().as<JsonArray>()[5].as<int>(), it.value().as<JsonArray>()[2].as<int>(), it.value().as<JsonArray>()[1].as<int>(), it.value().as<JsonArray>()[0].as<int>() - 2000);
         #endif
      }
      else if (strcmp(it.key().c_str(), "mac") == 0) {
         #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH)
         for (uint8_t i=0; i<ETHERNET_MAC_LENGTH; i++) {
            writable_configuration.ethernet_mac[i] = it.value().as<JsonArray>()[i];
         }
	 #else
	 LOGV(F("Configuration mac parameter ignored"));
	 #endif
      }
      #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM)
      else if (strcmp(it.key().c_str(), "gsmapn") == 0) {
         strncpy(writable_configuration.gsm_apn, it.value().as<const char*>(), GSM_APN_LENGTH);
      }
      #endif
      else if (strcmp(it.key().c_str(), "driver") == 0) {
         strncpy(writable_configuration.sensors[writable_configuration.sensors_count].driver, it.value().as<const char*>(), DRIVER_LENGTH);
         is_sensor_config = true;
      }
      else if (strcmp(it.key().c_str(), "type") == 0) {
         strncpy(writable_configuration.sensors[writable_configuration.sensors_count].type, it.value().as<const char*>(), TYPE_LENGTH);
         is_sensor_config = true;
      }
      else if (strcmp(it.key().c_str(), "address") == 0) {
         writable_configuration.sensors[writable_configuration.sensors_count].address = it.value().as<unsigned char>();
         is_sensor_config = true;
      }
      else if (strcmp(it.key().c_str(), "node") == 0) {
         writable_configuration.sensors[writable_configuration.sensors_count].node = it.value().as<unsigned char>();
         is_sensor_config = true;
      }
      else if (strcmp(it.key().c_str(), "mqttpath") == 0) {
         strncpy(writable_configuration.sensors[writable_configuration.sensors_count].mqtt_topic, it.value().as<const char*>(), MQTT_SENSOR_TOPIC_LENGTH);
         is_sensor_config = true;
      }
      else {
         is_error = true;
      }
   }

   if (is_sensor_config) {
     if (writable_configuration.sensors_count < SENSORS_MAX)
       writable_configuration.sensors_count++;
     else {
       is_error = true;
     }
   }

   if (is_error) {
      result[F("state")] = F("error");
      return E_INVALID_PARAMS;
   }
   else {
      result[F("state")] = F("done");
      return E_SUCCESS;
   }
}
#endif

#if (USE_RPC_METHOD_PREPARE)
int prepare(JsonObject params, JsonObject result) {
   static int state;
   static bool is_error = false;
   static char driver[DRIVER_LENGTH];
   static char type[TYPE_LENGTH];
   static uint8_t address = 0;
   static uint8_t node = 0;
   static uint8_t i;
   static uint32_t wait_time;

   switch (rpc_state) {
      case RPC_INIT:
         state = E_BUSY;
         is_error = extractSensorsParams(params, driver, type, &address, &node);

         if (!is_error && !is_event_sensors_reading) {
            is_event_sensors_reading_rpc = true;
            rpc_state = RPC_EXECUTE;
         }
         else {
            rpc_state = RPC_END;
         }
      break;

      case RPC_EXECUTE:
         if (is_event_sensors_reading_rpc) {
            sensors_reading_task (true, false, driver, type, address, node, &i, &wait_time);
         }
         else {
            rpc_state = RPC_END;
         }
      break;

      case RPC_END:
         if (is_error) {
            result[F("state")] = F("error");
            state = E_INVALID_PARAMS;
         }
         else {
            result[F("state")] = F("done");
            result[F("waittime")] = wait_time;
            state = E_SUCCESS;
         }
         rpc_state = RPC_INIT;
      break;
   }

   return state;
}
#endif

#if (USE_RPC_METHOD_GETJSON)
int getjson(JsonObject params, JsonObject result) {
   static int state;
   static bool is_error = false;
   static char driver[DRIVER_LENGTH];
   static char type[TYPE_LENGTH];
   static uint8_t address = 0;
   static uint8_t node = 0;
   static uint8_t i;
   static uint32_t wait_time;
   static char sensor_reading_time_buffer[DATE_TIME_STRING_LENGTH];

   switch (rpc_state) {
      case RPC_INIT:
         state = E_BUSY;
         is_error = extractSensorsParams(params, driver, type, &address, &node);

         if (!is_error && !is_event_sensors_reading) {
            is_event_sensors_reading_rpc = true;
            rpc_state = RPC_EXECUTE;
         }
         else {
            rpc_state = RPC_END;
         }
      break;

      case RPC_EXECUTE:
         if (is_event_sensors_reading_rpc) {
            sensors_reading_task (false, true, driver, type, address, node, &i, &wait_time);
         }
         else {
            rpc_state = RPC_END;
         }
      break;

      case RPC_END:
         if (is_error) {
            result[F("state")] = F("error");
            state = E_INVALID_PARAMS;
         }
         else {
            StaticJsonBuffer<JSON_BUFFER_LENGTH*2> jsonBuffer;
            JsonObject &v = jsonBuffer.parseObject((const char*) &json_sensors_data[i][0]);
	    time_t date_time=now();
            snprintf(sensor_reading_time_buffer, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u",
		     year(date_time), month(date_time), day(date_time), hour(date_time), minute(date_time), second(date_time))unsigned int>() == 0) {
                  result[F("v")][(char *) it.key().c_str()] = (char *) NULL;
               }
               else {
                  result[F("v")][(char *) it.key().c_str()] = it.value().as<unsigned int>();
               }
            }

            result[F("t")] = sensor_reading_time_buffer;
            state = E_SUCCESS;
         }
         rpc_state = RPC_INIT;
      break;
   }

   return state;
}
#endif

#if (USE_RPC_METHOD_PREPANDGET)
int prepandget(JsonObject params, JsonObject result) {
   static int state;
   static bool is_error = false;
   static char driver[DRIVER_LENGTH];
   static char type[TYPE_LENGTH];
   static uint8_t address = 0;
   static uint8_t node = 0;
   static uint8_t i;
   static uint32_t wait_time;
   static char sensor_reading_time_buffer[DATE_TIME_STRING_LENGTH];

   switch (rpc_state) {
      case RPC_INIT:
         state = E_BUSY;
         is_error = extractSensorsParams(params, driver, type, &address, &node);

         if (!is_error && !is_event_sensors_reading) {
            is_event_sensors_reading_rpc = true;
            rpc_state = RPC_EXECUTE;
         }
         else {
            rpc_state = RPC_END;
         }
      break;

      case RPC_EXECUTE:
         if (is_event_sensors_reading_rpc) {
            sensors_reading_task (true, true, driver, type, address, node, &i, &wait_time);
         }
         else {
            rpc_state = RPC_END;
         }
      break;

      case RPC_END:
         if (is_error) {
            result[F("state")] = F("error");
            state = E_INVALID_PARAMS;
         }
         else {
            StaticJsonBuffer<JSON_BUFFER_LENGTH*2> jsonBuffer;
            JsonObject &v = jsonBuffer.parseObject((const char*) &json_sensors_data[i][0]);
	    time_t date_time=now();
            snprintf(sensor_reading_time_buffer, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u",
		     year(date_time), month(date_time), day(date_time), hour(date_time), minute(date_time), second(date_time));

            result[F("state")] = F("done");
            result.createNestedObject(F("v"));

            for (JsonObject::iterator it = v.begin(); it != v.end(); ++it) {
               if (it.value().as<unsigned int>() == 0) {
                  result[F("v")][(char *) it.key().c_str()] = (char *) NULL;
               }
               else {
                  result[F("v")][(char *) it.key().c_str()] = it.value().as<unsigned int>();
               }
            }

            result[F("t")] = sensor_reading_time_buffer;
            state = E_SUCCESS;
         }
         rpc_state = RPC_INIT;
      break;
   }

   return state;
}
#endif

#if (USE_RPC_METHOD_RECOVERY && USE_MQTT)

int recovery(JsonObject params, JsonObject result) {
  static int state;
  static int tmpstate;
  static time_t ptr_time;
  static File mqtt_ptr_rpc_file;

  switch (rpc_state) {
  case RPC_INIT:

    state = E_BUSY;
    {
      bool found=false;

      for (JsonPair it : params) {
	if (strcmp(it.key().c_str(), "dts") == 0) {
	  found=true;
	  if (!sdcard_open_file(&SD, &mqtt_ptr_rpc_file, SDCARD_MQTT_PTR_RPC_FILE_NAME, O_RDWR | O_CREAT)) {
	    tmpstate = E_INTERNAL_ERROR;
	    is_sdcard_error = true;
	    result[F("state")] = F("error");
	    LOGE(F("SD Card opening ptr data on file %s... [ %s ]"), SDCARD_MQTT_PTR_RPC_FILE_NAME, FAIL_STRING);
	    rpc_state = RPC_END;
	    break;
	  }

	  tmElements_t datetime;
	  datetime.Year = CalendarYrToTm(it.value().as<JsonArray>()[0].as<int>());
	  datetime.Month = it.value().as<JsonArray>()[1].as<int>();
	  datetime.Day = it.value().as<JsonArray>()[2].as<int>();
	  datetime.Hour = it.value().as<JsonArray>()[3].as<int>();
	  datetime.Minute = it.value().as<JsonArray>()[4].as<int>();
	  datetime.Second = it.value().as<JsonArray>()[5].as<int>();
	  ptr_time = makeTime(datetime);
	  LOGN(F("RPC Data pointer... [ %d/%d/%d %d:%d:%d ]"), datetime.Day, datetime.Month, tmYearToCalendar(datetime.Year), datetime.Hour, datetime.Minute, datetime.Second);

	  rpc_state = RPC_EXECUTE;

	  break;
	}
      }

      if (!found){
	tmpstate = E_INVALID_PARAMS;
	result[F("state")] = F("error");
	LOGE(F("Invalid params [ %s ]"), FAIL_STRING);

	rpc_state = RPC_END;
      }
    }
    break;

  case RPC_EXECUTE:

    if (mqtt_ptr_rpc_file.seekSet(0) && mqtt_ptr_rpc_file.write(&ptr_time, sizeof(time_t)) == sizeof(time_t)) {
      mqtt_ptr_rpc_file.flush();
      mqtt_ptr_rpc_file.close();

      LOGN(F("SD Card writing ptr data on file %s... [ %s ]"), SDCARD_MQTT_PTR_RPC_FILE_NAME, OK_STRING);
      tmpstate = E_SUCCESS;
      result[F("state")] = F("done");

    }else {
      tmpstate = E_INTERNAL_ERROR;
      result[F("state")] = F("error");
      LOGE(F("SD Card writing ptr data on file %s... [ %s ]"), SDCARD_MQTT_PTR_RPC_FILE_NAME, FAIL_STRING);
    }

    rpc_state = RPC_END;

  case RPC_END:

    rpc_state = RPC_INIT;
    state=tmpstate;
    break;
  }

  return state;
}
#endif


time_t getSystemTime() {
  return system_time;
}

void realreboot() {
  LOGF(F("bye bye"));
  init_wdt(WDTO_1S);
  while(true);
}

#if (USE_RPC_METHOD_REBOOT)
int reboot(JsonObject params, JsonObject result) {
   #if (USE_LCD)
   lcd_error |= lcd.clear();
   lcd_error |= lcd.print(F("Reboot"))==0;
   #endif
   LOGT(F("Reboot"));
   result[F("state")] = "done";
   have_to_reboot=true;
   return E_SUCCESS;
}
#endif

void interrupt_task_1s () {
  system_time++;
  setTime(system_time);

  #if (MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM)
  if (is_time_set && now() >= next_ptr_time_for_sensors_reading && next_ptr_time_for_sensors_reading) {

    sensor_reading_time.Day = day(next_ptr_time_for_sensors_reading);
    sensor_reading_time.Month = month(next_ptr_time_for_sensors_reading);
    sensor_reading_time.Year = CalendarYrToTm(year(next_ptr_time_for_sensors_reading));
    sensor_reading_time.Hour = hour(next_ptr_time_for_sensors_reading);
    sensor_reading_time.Minute = minute(next_ptr_time_for_sensors_reading);
    sensor_reading_time.Second = second(next_ptr_time_for_sensors_reading);

    setNextTimeForSensorReading((time_t *) &next_ptr_time_for_sensors_reading, readable_configuration.report_seconds);
    is_time_for_sensors_reading_updated = true;
    do_reset_first_run = true;

    noInterrupts();
    if (!is_event_sensors_reading) {
      is_test = false;
      is_event_sensors_reading = true;
      ready_tasks_count++;
    }

    #if (USE_MQTT)
    if (is_event_mqtt) {
      is_event_mqtt_paused = true;
      is_event_mqtt = false;
      ready_tasks_count--;
    }
    #endif
    interrupts();
  }

  if (is_time_set && now() >= next_ptr_time_for_testing_sensors && next_ptr_time_for_testing_sensors) {
    setNextTimeForSensorReading((time_t *) &next_ptr_time_for_testing_sensors, SENSORS_TESTING_DELAY_S);
    noInterrupts();
    if (!is_event_sensors_reading) {
      is_test = !is_first_test;
      is_event_sensors_reading = true;
      ready_tasks_count++;
    }
    interrupts();
  }
  #endif

  noInterrupts();
  if (!is_event_rtc) {
    is_event_rtc = true;
    ready_tasks_count++;
  }
  interrupts();
}

void supervisor_task() {
   static uint8_t retry;
   static supervisor_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;

   static bool is_supervisor_first_run = true;
   static bool is_time_updated;

   bool is_sdcard_ok = false;

   #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
   bool *is_event_client = &is_event_ethernet;
   #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
   bool *is_event_client = &is_event_gsm;
   #endif

   switch (supervisor_state) {
      case SUPERVISOR_INIT:
         retry = 0;
         start_time_ms = 0;
         is_time_updated = false;

         #if (MODULE_TYPE != STIMA_MODULE_TYPE_PASSIVE)
         if (!*is_event_client && is_client_connected) {
            is_event_client_executed = true;
         }
         else {
            is_event_time_executed = false;
         }
         #endif

         #if (USE_MQTT)
         if (is_event_mqtt_paused) {
            noInterrupts();
            if (!is_event_mqtt) {
               is_event_mqtt = true;
               ready_tasks_count++;
            }
            interrupts();

            supervisor_state = SUPERVISOR_END;
            LOGV(F("SUPERVISOR_INIT ---> SUPERVISOR_END"));
         }
         else {
         #endif
            #if (USE_NTP)
            // need ntp sync ?
            if (!do_ntp_sync && ((now() - last_ntp_sync > NTP_TIME_FOR_RESYNC_S) || !is_time_set)) {
               do_ntp_sync = true;
               LOGT(F("Requested NTP time sync..."));
            }

            start_time_ms = millis();
            #endif

            #if (MODULE_TYPE != STIMA_MODULE_TYPE_PASSIVE)
            supervisor_state = SUPERVISOR_CONNECTION_LEVEL_TASK;
            LOGV(F("SUPERVISOR_INIT ---> SUPERVISOR_CONNECTION_LEVEL_TASK"));
            #else
            supervisor_state = SUPERVISOR_TIME_LEVEL_TASK;
            LOGV(F("SUPERVISOR_INIT ---> SUPERVISOR_TIME_LEVEL_TASK"));
            #endif
         #if (USE_MQTT)
         }
         #endif
      break;

      case SUPERVISOR_CONNECTION_LEVEL_TASK:
         #if (MODULE_TYPE != STIMA_MODULE_TYPE_PASSIVE)
         // enable hardware related tasks for connect
         noInterrupts();
         if (!*is_event_client && !is_event_client_executed && !is_client_connected) {
            *is_event_client = true;
            ready_tasks_count++;
         }
         interrupts();
         supervisor_state = SUPERVISOR_WAIT_CONNECTION_LEVEL_TASK;
         LOGV(F("SUPERVISOR_CONNECTION_LEVEL_TASK ---> SUPERVISOR_WAIT_CONNECTION_LEVEL_TASK"));
         #endif
      break;

      case SUPERVISOR_WAIT_CONNECTION_LEVEL_TASK:
         #if (MODULE_TYPE != STIMA_MODULE_TYPE_PASSIVE)
         // success
         if (!*is_event_client && is_event_client_executed && is_client_connected) {

            // reset time task for doing ntp sync
            if (is_client_udp_socket_open && do_ntp_sync) {
               is_event_time_executed = false;
               supervisor_state = SUPERVISOR_TIME_LEVEL_TASK;
               LOGV(F("SUPERVISOR_WAIT_CONNECTION_LEVEL_TASK ---> SUPERVISOR_TIME_LEVEL_TASK"));
            }
            // doing other operations...
            else {
               do_ntp_sync = false;
               supervisor_state = SUPERVISOR_MANAGE_LEVEL_TASK;
               LOGV(F("SUPERVISOR_WAIT_CONNECTION_LEVEL_TASK ---> SUPERVISOR_MANAGE_LEVEL_TASK"));
            }
         }

         // error
         if (!*is_event_client && is_event_client_executed && !is_client_connected) {
            // retry
            if ((++retry < SUPERVISOR_CONNECTION_RETRY_COUNT_MAX) || (millis() - start_time_ms < SUPERVISOR_CONNECTION_TIMEOUT_MS)) {
               is_event_client_executed = false;
               supervisor_state = SUPERVISOR_CONNECTION_LEVEL_TASK;
	       LOGT(F("Supervisor connection... [ task ]"));
               LOGV(F("SUPERVISOR_WAIT_CONNECTION_LEVEL_TASK ---> SUPERVISOR_CONNECTION_LEVEL_TASK"));
            }
            // fail
            else {
	       LOGE(F("Supervisor connection... [ %s ]"), FAIL_STRING);
               supervisor_state = SUPERVISOR_TIME_LEVEL_TASK;
               LOGV(F("SUPERVISOR_WAIT_CONNECTION_LEVEL_TASK ---> SUPERVISOR_TIME_LEVEL_TASK"));
            }
         }
         #endif
      break;

      case SUPERVISOR_TIME_LEVEL_TASK:
         // enable time task
         noInterrupts();
         if (!is_event_time && !is_event_time_executed) {
            is_event_time = true;
            ready_tasks_count++;
         }
         interrupts();

         if (!is_event_time && is_event_time_executed) {

	    // if NTP sync fail, reset variable anyway
            if (do_ntp_sync || ((now() - last_ntp_sync) > NTP_TIME_FOR_RESYNC_S)) {
               last_ntp_sync = system_time;
               do_ntp_sync = false;
            }

            is_time_updated = true;

            #if (USE_NTP)
            if (is_client_connected) {
               do_ntp_sync = false;

               #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM)
               noInterrupts();
               if (!*is_event_client) {
                  *is_event_client = true;
                  ready_tasks_count++;
               }
               interrupts();
               #endif
            }
            #endif

	    if (is_supervisor_first_run && !is_datetime_set) {
	      have_to_reboot = true;
	      supervisor_state = SUPERVISOR_END;
	      LOGV(F("SUPERVISOR_TIME_LEVEL_TASK ---> SUPERVISOR_END"));
	    }else{
	      supervisor_state = SUPERVISOR_MANAGE_LEVEL_TASK;
	      LOGV(F("SUPERVISOR_TIME_LEVEL_TASK ---> SUPERVISOR_MANAGE_LEVEL_TASK"));
	    }
         }
      break;

      case SUPERVISOR_MANAGE_LEVEL_TASK:
         if (is_time_updated) {
	    time_t date_time=now();
            LOGN(F("Current date and time is: %d/%d/%d %d:%d:%d"),
		 day(date_time), month(date_time), year(date_time), hour(date_time), minute(date_time), second(date_time));
         }

         #if (MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM)
         if (is_supervisor_first_run && is_time_set) {
            if (readable_configuration.report_seconds) {
               setNextTimeForSensorReading((time_t *) &next_ptr_time_for_sensors_reading, readable_configuration.report_seconds);

               // testing sensors
               setNextTimeForSensorReading((time_t *) &next_ptr_time_for_testing_sensors, SENSORS_TESTING_DELAY_S);

               LOGN(F("Acquisition scheduling..."));
            }

            if (readable_configuration.report_seconds >= 60) {
               uint8_t mm = readable_configuration.report_seconds / 60;
               uint8_t ss = readable_configuration.report_seconds - mm * 60;
               if (ss) {
                  LOGN(F("--> report every %d minutes and %d seconds"), mm, ss);
               }
               else {
                  LOGN(F("--> report every %d minutes"), mm);
               }
            }
            else if (readable_configuration.report_seconds) {
               LOGN(F("--> report every %d seconds"), readable_configuration.report_seconds);
            }

            if (next_ptr_time_for_sensors_reading) {
               LOGN(F("--> starting at: %d:%d:%d"), hour(next_ptr_time_for_sensors_reading), minute(next_ptr_time_for_sensors_reading), second(next_ptr_time_for_sensors_reading));
               #if (USE_LCD)
	       //lcd_error |= lcd.clear();

	       lcd_error |= lcd.setCursor(0, 0);
               lcd_error |= lcd.print(F("            "))==0;
	       lcd_error |= lcd.setCursor(0, 0);
               lcd_error |= lcd.print(F("S: "))==0;
	       lcd_error |= lcd.print(hour(next_ptr_time_for_sensors_reading))==0;
               lcd_error |= lcd.print(F(":"))==0;
	       lcd_error |= lcd.print( minute(next_ptr_time_for_sensors_reading))==0;
               lcd_error |= lcd.print(F(":"))==0;
	       lcd_error |= lcd.print(second(next_ptr_time_for_sensors_reading))==0;

	       
               #if (USE_MQTT)
	       lcd_error |= lcd.setCursor(0, 1);
               lcd_error |= lcd.print(F("                    "))==0;
	       lcd_error |= lcd.setCursor(0, 1)==0;
	       lcd_error |= lcd.print("server: ")==0;
	       lcd_error |= lcd.print(readable_configuration.mqtt_server)==0;
               #endif

               #if (USE_NTP)
	       lcd_error |= lcd.setCursor(0, 2);
               lcd_error |= lcd.print(F("                    "))==0;
	       lcd_error |= lcd.setCursor(0, 2);
	       lcd_error |= lcd.print("ntp: ")==0;
	       lcd_error |= lcd.print(readable_configuration.ntp_server)==0;
               #endif
	       
	       #endif
            }

            if (next_ptr_time_for_testing_sensors) {
               LOGN(F("--> testing at: %d:%d:%d"), hour(next_ptr_time_for_testing_sensors), minute(next_ptr_time_for_testing_sensors), second(next_ptr_time_for_testing_sensors));
            }
         }
         #endif
	 #if (USE_LCD)
         // reinit lcd display
         if (last_lcd_begin == 0) {
            last_lcd_begin = now();
         }
         else if ((now() - last_lcd_begin > LCD_TIME_FOR_REINITIALIZE_S) || lcd_error) {
            last_lcd_begin = now();
            if (lcd_error) LOGE(F("LCD ERROR"));
	    LOGN(F("Reinitialize LCD"));
            init_lcd();
	    lcd_error |= lcd.clear();
	    lcd_error |= lcd.print(F("N: "))==0;
	    lcd_error |= lcd.print(hour(next_ptr_time_for_sensors_reading))==0;
	    lcd_error |= lcd.print(F(":"))==0;
	    lcd_error |= lcd.print(minute(next_ptr_time_for_sensors_reading))==0;
	    lcd_error |= lcd.print(F(":"))==0;
	    lcd_error |= lcd.print(second(next_ptr_time_for_sensors_reading))==0;
	    lcd_error |= lcd.setCursor(12, 0);
	    lcd_error |= lcd.print(F("LCD: KO"))==0;
         }
	 #endif

         #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH)
         #if (USE_MQTT)
         noInterrupts();
         if (!is_event_mqtt) {
            is_event_mqtt = true;
            ready_tasks_count++;
         }
         interrupts();
         #endif

         #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM)
         #if (USE_MQTT)
         if (!*is_event_client) {
            noInterrupts();
            if (!is_event_mqtt) {
               is_event_mqtt = true;
               ready_tasks_count++;
            }
            interrupts();
         }
         #endif

         #endif

         supervisor_state = SUPERVISOR_TEST_SDCARD;
      break;

      case SUPERVISOR_TEST_SDCARD:
        #if (USE_SDCARD)
        if (is_supervisor_first_run) {
	  if (!is_sdcard_open) {
	    if (sdcard_init(&SD, SDCARD_CHIP_SELECT_PIN)) {
	      LOGN(F("SDCARD opened Test SDcard"));
	      is_sdcard_open=true;
	      is_sdcard_error = false;
	    }
	  }
	  if (is_sdcard_open){
	    if (sdcard_open_file(&SD, &test_file, SDCARD_INFO_FILE_NAME, O_WRONLY | O_CREAT )) {
	      test_file.println(stima_name);
	      test_file.println(MODULE_MAIN_VERSION);
	      test_file.println(MODULE_MINOR_VERSION);
	      test_file.println(readable_configuration.mqtt_root_topic);
	      test_file.println(MQTT_SENSOR_TOPIC_LENGTH);
	      test_file.println(MQTT_MESSAGE_LENGTH);
	      is_sdcard_ok = test_file.close();
            }
          }

          if (!is_sdcard_ok) {
            LOGE(F("SD Card... [ %s ]"), FAIL_STRING);
	    LOGE(F("--> is card inserted?"));
	    LOGE(F("--> there is a valid FAT32 filesystem?"));
            #if (USE_LCD)
	    lcd_error |= lcd.setCursor(0, 3);
	    lcd_error |= lcd.print(F("SD Card: "))==0;
	    lcd_error |= lcd.print(FAIL_STRING)==0;
	    lcd_error |= lcd.print(F("         "))==0;
	    #endif
          }else{
	    // remove firmware to do not redo update the next reboot
	    if (sdcard_remove_firmware(&SD, MODULE_MAIN_VERSION, MODULE_MINOR_VERSION)){
	      LOGN(F("removed firmware version %d.%d from SD"),MODULE_MAIN_VERSION, MODULE_MINOR_VERSION);
	      lcd_error |= lcd.setCursor(0, 3);
	      lcd_error |= lcd.print( F("NEW Firmware loaded "))==0;
	    }
	  }
        }
        #endif

	// now we use flush before sleep

	//        #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_INFO)
        //delay_ms = DEBUG_WAIT_FOR_SLEEP_MS;
        //start_time_ms = millis();
        //state_after_wait = SUPERVISOR_END;
        //supervisor_state = SUPERVISOR_WAIT_STATE;
        //#else
        supervisor_state = SUPERVISOR_END;
        //#endif
        LOGV(F("SUPERVISOR_MANAGE_LEVEL_TASK ---> SUPERVISOR_END"));
      break;

      case SUPERVISOR_END:
         is_supervisor_first_run = false;
         noInterrupts();
         is_event_supervisor = false;
         ready_tasks_count--;
         interrupts();

         supervisor_state = SUPERVISOR_INIT;
         LOGV(F("SUPERVISOR_END ---> SUPERVISOR_INIT"));
      break;

      case SUPERVISOR_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            supervisor_state = state_after_wait;
         }
      break;
   }
}

void rtc_task() {
  if (is_time_set) {
    noInterrupts();
    is_event_rtc = false;
    ready_tasks_count--;
    interrupts();

    #if (MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM)
    if (is_time_for_sensors_reading_updated) {
      is_time_for_sensors_reading_updated = false;
      LOGN(F("Next acquisition scheduled at: %d:%d:%d"), hour(next_ptr_time_for_sensors_reading), minute(next_ptr_time_for_sensors_reading), second(next_ptr_time_for_sensors_reading));
      #if (USE_LCD)
      lcd_error |= lcd.clear();
      lcd_error |= lcd.print(F("N: "))==0;
      lcd_error |= lcd.print(hour(next_ptr_time_for_sensors_reading))==0;
      lcd_error |= lcd.print(F(":"))==0;
      lcd_error |= lcd.print(minute(next_ptr_time_for_sensors_reading))==0;
      lcd_error |= lcd.print(F(":"))==0;
      lcd_error |= lcd.print(second(next_ptr_time_for_sensors_reading))==0;
      #endif
    }
    #endif
  }
}

void time_task() {
   static uint8_t retry_ntp;
   static uint8_t retry_request;
   static uint8_t retry_rtc;
   static time_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;
   time_t date_time;
   
   #if (USE_NTP)
   static bool is_set_rtc_ok;
   static time_t current_ntp_time;
   time_t diff_ntp_time;
   bool is_ntp_request_ok;
   #endif

   switch (time_state) {
      case TIME_INIT:
         #if (USE_NTP)
         current_ntp_time = 0;
         is_set_rtc_ok = true;
         #endif
         retry_ntp = 0;
         retry_request = 0;
         retry_rtc = 0;
         state_after_wait = TIME_INIT;

         #if (USE_NTP)
         if (is_client_connected) {
            time_state = TIME_SEND_ONLINE_REQUEST;
            LOGV(F("TIME_INIT --> TIME_SEND_ONLINE_REQUEST"));
         }
         else {
            time_state = TIME_SET_SYNC_RTC_PROVIDER;
            LOGV(F("TIME_INIT --> TIME_SET_SYNC_RTC_PROVIDER"));
         }
         #else
         #if (USE_RTC)
         time_state = TIME_SET_SYNC_RTC_PROVIDER;
         LOGV(F("TIME_INIT --> TIME_SET_SYNC_RTC_PROVIDER"));
         #elif (USE_TIMER_1)
         time_state = TIME_END;
         LOGV(F("TIME_INIT --> TIME_END"));
         #endif
         #endif
      break;

      case TIME_SEND_ONLINE_REQUEST:
         #if (USE_NTP)
	LOGT(F("NTP send request"));
         #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
         is_ntp_request_ok = Ntp::sendRequest(&eth_udp_client, readable_configuration.ntp_server);

         #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
         is_ntp_request_ok = Ntp::sendRequest(&s800);
         #endif

         // success
         if (is_ntp_request_ok) {
	   LOGN(F("NTP send request success"));
	   retry_request = 0;
            time_state = TIME_WAIT_ONLINE_RESPONSE;
            LOGV(F("TIME_SEND_ONLINE_REQUEST --> TIME_WAIT_ONLINE_RESPONSE"));
         }
         // retry
         else if (++retry_request < NTP_RETRY_COUNT_MAX) {
            delay_ms = NTP_RETRY_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = TIME_SEND_ONLINE_REQUEST;
            time_state = TIME_WAIT_STATE;
            LOGE(F("NTP request... [ retry ]"));
            LOGV(F("TIME_SEND_ONLINE_REQUEST --> TIME_WAIT_STATE"));
         }
         // fail: use old rtc time
         else {
            LOGE(F("NTP request... [ %s ]"), FAIL_STRING);
            #if (USE_RTC)
            time_state = TIME_SET_SYNC_RTC_PROVIDER;
            LOGV(F("TIME_SEND_ONLINE_REQUEST --> TIME_SET_SYNC_RTC_PROVIDER"));
            #elif (USE_TIMER_1)
            time_state = TIME_END;
            LOGV(F("TIME_SEND_ONLINE_REQUEST --> TIME_END"));
            #endif
         }
         #endif
      break;

      case TIME_WAIT_ONLINE_RESPONSE:
         #if (USE_NTP)
	LOGT(F("NTP receive response"));
         #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
         current_ntp_time = Ntp::getResponse(&eth_udp_client);

         #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
         current_ntp_time = Ntp::getResponse(&s800);

         #endif

         if (is_time_set && (system_time > current_ntp_time)) {
           diff_ntp_time = system_time - current_ntp_time;
         } else if (is_time_set) {
           diff_ntp_time = current_ntp_time - system_time;
         } else {
           diff_ntp_time = 0;
         }

         if ((current_ntp_time > NTP_VALID_START_TIME_S) && (diff_ntp_time <= NTP_MAX_DIFF_VALID_TIME_S)) {
	    LOGN(F("NTP response... [ %s ] diff time: %lms"), OK_STRING,diff_ntp_time);
            system_time = current_ntp_time;
            setTime(system_time);
            last_ntp_sync = current_ntp_time;
	    is_datetime_set = true;
	    LOGT(F("Current NTP date and time: %d/%d/%d %d:%d:%d"),
		 day(system_time), month(system_time), year(system_time), hour(system_time), minute(system_time), second(system_time));
            #if (USE_RTC)
            time_state = TIME_SET_SYNC_NTP_PROVIDER;
            LOGV(F("TIME_WAIT_ONLINE_RESPONSE --> TIME_SET_SYNC_NTP_PROVIDER"));
            #elif (USE_TIMER_1)
            time_state = TIME_SET_SYNC_NTP_PROVIDER;
            LOGV(F("TIME_WAIT_ONLINE_RESPONSE --> TIME_SET_SYNC_NTP_PROVIDER"));
            #endif
         }
         // retry
         else if (++retry_ntp < NTP_RETRY_COUNT_MAX) {
            delay_ms = NTP_RETRY_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = TIME_SEND_ONLINE_REQUEST;
            time_state = TIME_WAIT_STATE;
            LOGE(F("NTP response... [ retry ]"));
            LOGV(F("TIME_WAIT_ONLINE_RESPONSE --> TIME_WAIT_STATE"));
         }
         // fail
         else {
            LOGE(F("NTP response... [ %s ]"), FAIL_STRING);
            #if (USE_RTC)
            time_state = TIME_SET_SYNC_RTC_PROVIDER;
            LOGV(F("TIME_WAIT_ONLINE_RESPONSE --> TIME_SET_SYNC_RTC_PROVIDER"));
            #elif (USE_TIMER_1)
            time_state = TIME_END;
            LOGV(F("TIME_WAIT_ONLINE_RESPONSE --> TIME_END"));
            #endif
         }
         #endif
      break;

      case TIME_SET_SYNC_NTP_PROVIDER:
         #if (USE_NTP)
         #if (USE_RTC)
	 is_set_rtc_ok = set_datetime_rtc(system_time);
         #endif
         if (!is_set_rtc_ok) {
           i2c_error++;
         }

         if (is_set_rtc_ok) {
	    LOGN(F("RTC set... [ %s ]"), OK_STRING);
	    i2c_error = 0;
            time_state = TIME_SET_SYNC_RTC_PROVIDER;
            LOGV(F("TIME_SET_SYNC_NTP_PROVIDER --> TIME_SET_SYNC_RTC_PROVIDER"));
         }
         // retry
         else if (++retry_rtc < NTP_RETRY_COUNT_MAX) {
            is_set_rtc_ok = true;
            delay_ms = NTP_RETRY_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = TIME_SET_SYNC_NTP_PROVIDER;
            time_state = TIME_WAIT_STATE;
            LOGE(F("RTC set... [ retry ]"));
            LOGV(F("TIME_SET_SYNC_NTP_PROVIDER --> TIME_SET_SYNC_NTP_PROVIDER"));
         }
         // fail: use old rtc time
         else {
	   LOGE(F("RTC set... [ %s ]"), FAIL_STRING);
           time_state = TIME_SET_SYNC_RTC_PROVIDER;
           LOGV(F("TIME_SET_SYNC_NTP_PROVIDER --> TIME_SET_SYNC_RTC_PROVIDER"));
         }
         #endif
      break;

      case TIME_SET_SYNC_RTC_PROVIDER:
         setSyncInterval(NTP_TIME_FOR_RESYNC_S);
         setSyncProvider(getSystemTime);
	 date_time=now();
         LOGN(F("Current System date and time: %d/%d/%d %d:%d:%d"),
	      day(date_time), month(date_time), year(date_time), hour(date_time), minute(date_time), second(date_time));
         time_state = TIME_END;
         LOGV(F("TIME_SET_SYNC_RTC_PROVIDER --> TIME_END"));
      break;

      case TIME_END:
         is_time_set = true;
         is_event_time_executed = true;
         noInterrupts();
         is_event_time = false;
         ready_tasks_count--;
         interrupts();
         time_state = TIME_INIT;
         LOGV(F("TIME_END --> TIME_INIT"));
      break;

      case TIME_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            time_state = state_after_wait;
         }
      break;
   }
}

#if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
void ethernet_task() {
   static uint8_t retry;
   static ethernet_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;

   switch (ethernet_state) {
      case ETHERNET_INIT:
         retry = 0;
         is_client_connected = false;
         is_client_udp_socket_open = false;
         state_after_wait = ETHERNET_INIT;
         ethernet_state = ETHERNET_CONNECT;
         LOGV(F("ETHERNET_INIT --> ETHERNET_CONNECT"));
      break;

      case ETHERNET_CONNECT:
         if (readable_configuration.is_dhcp_enable) {
            if (Ethernet.begin(readable_configuration.ethernet_mac)) {
               is_client_connected = true;
               LOGN(F("Ethernet: DHCP [ %s ]"), OK_STRING);
            }
         }
         else {
            Ethernet.begin(readable_configuration.ethernet_mac, IPAddress(readable_configuration.ip), IPAddress(readable_configuration.primary_dns), IPAddress(readable_configuration.gateway), IPAddress(readable_configuration.netmask));
            is_client_connected = true;
            LOGN(F("Ethernet: Static [ %s ]"), OK_STRING);
         }

         // success
         if (is_client_connected) {
            w5500.setRetransmissionTime(ETHERNET_RETRY_TIME_MS);
            w5500.setRetransmissionCount(ETHERNET_RETRY_COUNT);

            LOGN(F("--> ip: %d.%d.%d.%d"), Ethernet.localIP()[0], Ethernet.localIP()[1], Ethernet.localIP()[2], Ethernet.localIP()[3]);
            LOGN(F("--> netmask: %d.%d.%d.%d"), Ethernet.subnetMask()[0], Ethernet.subnetMask()[1], Ethernet.subnetMask()[2], Ethernet.subnetMask()[3]);
            LOGN(F("--> gateway: %d.%d.%d.%d"), Ethernet.gatewayIP()[0], Ethernet.gatewayIP()[1], Ethernet.gatewayIP()[2], Ethernet.gatewayIP()[3]);
            LOGN(F("--> primary dns: %d.%d.%d.%d"), Ethernet.dnsServerIP()[0], Ethernet.dnsServerIP()[1], Ethernet.dnsServerIP()[2], Ethernet.dnsServerIP()[3]);

            #if (USE_LCD)
	    lcd_error |= lcd.clear();
            lcd_error |= lcd.print(F("ip: "))==0;
	    lcd_error |= lcd.print(Ethernet.localIP()[0])==0;
	    lcd_error |= lcd.print(F("."))==0;
	    lcd_error |= lcd.print(Ethernet.localIP()[1])==0;
	    lcd_error |= lcd.print(F("."))==0;
	    lcd_error |= lcd.print(Ethernet.localIP()[2])==0;
	    lcd_error |= lcd.print(F("."))==0;
	    lcd_error |= lcd.print(Ethernet.localIP()[3])==0;
	    #endif
            ethernet_state = ETHERNET_OPEN_UDP_SOCKET;
            LOGV(F("ETHERNET_CONNECT --> ETHERNET_OPEN_UDP_SOCKET"));
         }
         // retry
         else if ((++retry) < ETHERNET_RETRY_COUNT_MAX) {
            delay_ms = ETHERNET_RETRY_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = ETHERNET_CONNECT;
            ethernet_state = ETHERNET_WAIT_STATE;
            LOGE(F("Ethernet retry: [ %s ]") readable_configuration.is_dhcp_enable ? "DHCP" : "Static");
            LOGV(F("ETHERNET_CONNECT --> ETHERNET_WAIT_STATE"));
         }
         // fail
         else {
            retry = 0;
            ethernet_state = ETHERNET_END;
            LOGV(F("ETHERNET_CONNECT --> ETHERNET_END"));
            LOGE(F("Ethernet %s: [ %s ]"), ERROR_STRING, readable_configuration.is_dhcp_enable ? "DHCP" : "Static");
            #if (USE_LCD)
	    lcd_error |= lcd.setCursor(0, 2);
            lcd_error |= lcd.print(F("ethernet "))==0;
	    lcd_error |= lcd.print(ERROR_STRING)==0;
	    #endif
         }
      break;

      case ETHERNET_OPEN_UDP_SOCKET:
         // success
         if (eth_udp_client.begin(ETHERNET_DEFAULT_LOCAL_UDP_PORT)) {
            LOGV(F("--> udp socket local port: %d [ OK ]"), ETHERNET_DEFAULT_LOCAL_UDP_PORT);
            is_client_udp_socket_open = true;
            ethernet_state = ETHERNET_END;
            LOGV(F("ETHERNET_OPEN_UDP_SOCKET --> ETHERNET_END"));
         }
         // retry
         else if ((++retry) < ETHERNET_RETRY_COUNT_MAX) {
            delay_ms = ETHERNET_RETRY_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = ETHERNET_OPEN_UDP_SOCKET;
            ethernet_state = ETHERNET_WAIT_STATE;
            LOGE(F("--> udp socket on local port: %d [ retry ]"), ETHERNET_DEFAULT_LOCAL_UDP_PORT);
            LOGV(F("ETHERNET_OPEN_UDP_SOCKET --> ETHERNET_WAIT_STATE"));
         }
         // fail
         else {
            LOGE(F("--> udp socket on local port: %d [ FAIL ]"), ETHERNET_DEFAULT_LOCAL_UDP_PORT);
            retry = 0;
            ethernet_state = ETHERNET_INIT;
            LOGV(F("ETHERNET_OPEN_UDP_SOCKET --> ETHERNET_INIT"));
         }
      break;

      case ETHERNET_END:
         LOGN(F(""));
         is_event_client_executed = true;
         noInterrupts();
         is_event_ethernet = false;
         ready_tasks_count--;
         interrupts();
         ethernet_state = ETHERNET_INIT;
         LOGV(F("ETHERNET_END --> ETHERNET_INIT"));
      break;

      case ETHERNET_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            ethernet_state = state_after_wait;
         }
      break;
   }
}

#elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
void gsm_task() {
   static gsm_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;
   static bool is_error;
   sim800_status_t sim800_status;
   uint8_t sim800_connection_status;
   static uint8_t power_off_mode = SIM800_POWER_OFF_BY_SWITCH;

   switch (gsm_state) {
      case GSM_INIT:
         is_error = false;
         is_client_connected = false;
         sim800_connection_status = 0;
         state_after_wait = GSM_INIT;
         gsm_state = GSM_SWITCH_ON;
	 LOGV(F("GSM_INIT ---> GSM_SWITCH_ON"));
      break;

      case GSM_SWITCH_ON:
         sim800_status = s800.switchOn();

         // success
         if (sim800_status == SIM800_OK) {
            gsm_state = GSM_AUTOBAUD;
	    LOGV(F("GSM_SWITCH_ON ---> GSM_AUTOBAUD"));
         }
         else if (sim800_status == SIM800_ERROR) {
            gsm_state = GSM_END;
	    LOGV(F("GSM_SWITCH_ON ---> GSM_END"));
         }
         // wait...
      break;

      case GSM_AUTOBAUD:
         sim800_status = s800.initAutobaud();

         // success
         if (sim800_status == SIM800_OK) {
            delay_ms = SIM800_WAIT_FOR_AUTOBAUD_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = GSM_SETUP;
            gsm_state = GSM_WAIT_STATE;
	    LOGV(F("GSM_AUTOBAUD ---> GSM_WAIT_STATE"));
         }
         // fail
         else if (sim800_status == SIM800_ERROR) {
            gsm_state = GSM_WAIT_FOR_SWITCH_OFF;
	    LOGV(F("GSM_AUTOBAUD ---> GSM_WAIT_FOR_SWITCH_OFF"));
         }
         // wait...
      break;

      case GSM_SETUP:
         sim800_status = s800.setup();

         // success
         if (sim800_status == SIM800_OK) {
            gsm_state = GSM_START_CONNECTION;
	    LOGV(F("GSM_SETUP ---> GSM_START_CONNECTION"));
         }
         // fail
         else if (sim800_status == SIM800_ERROR) {
            is_error = true;
            gsm_state = GSM_WAIT_FOR_SWITCH_OFF;
	    LOGV(F("GSM_SETUP ---> GSM_WAIT_FOR_SWITCH_OFF"));
         }
         // wait...
      break;

      case GSM_START_CONNECTION:
         sim800_status = s800.startConnection(readable_configuration.gsm_apn, readable_configuration.gsm_username, readable_configuration.gsm_password);

         // success
         if (sim800_status == SIM800_OK) {
            gsm_state = GSM_CHECK_OPERATION;
	    LOGV(F("GSM_START_CONNECTION ---> GSM_CHECK_OPERATION"));
         }
         // fail
         else if (sim800_status == SIM800_ERROR) {
            is_error = true;
            gsm_state = GSM_WAIT_FOR_SWITCH_OFF;
	    LOGV(F("GSM_START_CONNECTION ---> GSM_WAIT_FOR_SWITCH_OFF"));
         }
         // wait...
      break;

      case GSM_CHECK_OPERATION:
         // open udp socket for query NTP
         if (do_ntp_sync) {
            gsm_state = GSM_OPEN_UDP_SOCKET;
	    LOGV(F("GSM_CHECK_OPERATION ---> GSM_OPEN_UDP_SOCKET"));
         }
         // wait for mqtt send terminate
         else {
            gsm_state = GSM_SUSPEND;
            state_after_wait = GSM_STOP_CONNECTION;
	    LOGV(F("GSM_CHECK_OPERATION ---> GSM_STOP_CONNECTION"));
         }
      break;

      case GSM_OPEN_UDP_SOCKET:
         sim800_connection_status = s800.connection(SIM800_CONNECTION_UDP, readable_configuration.ntp_server, NTP_SERVER_PORT);

         // success
         if (sim800_connection_status == 1) {
            is_client_udp_socket_open = true;
            is_client_connected = true;
            is_event_client_executed = true;
            state_after_wait = GSM_STOP_CONNECTION;
            gsm_state = GSM_SUSPEND;
	    LOGV(F("GSM_OPEN_UDP_SOCKET ---> GSM_SUSPEND"));
         }
         // fail
         else if (sim800_connection_status == 2) {
            is_client_connected = false;
            is_event_client_executed = true;
            is_error = true;
            gsm_state = GSM_WAIT_FOR_SWITCH_OFF;
	    LOGV(F("GSM_OPEN_UDP_SOCKET ---> GSM_WAIT_FOR_SWITCH_OFF"));

            #if (USE_LCD)
	    lcd_error |= lcd.setCursor(12, 0);
	    lcd_error |= lcd.print(F("        "))==0;
	    lcd_error |= lcd.setCursor(12, 0);
	    lcd_error |= lcd.print(F("rf: KO"))==0;
            #endif
	 }
         // wait
      break;

      case GSM_SUSPEND:

         #if (USE_LCD)
	 uint8_t rssi;
	 uint8_t ber;
	 s800.getLastCsq(&rssi,&ber);
	 lcd_error |= lcd.setCursor(12, 0);
	 lcd_error |= lcd.print(F("        "))==0;
	 lcd_error |= lcd.setCursor(12, 0);
	 lcd_error |= lcd.print(F("rf:"))==0;
	 lcd_error |= lcd.print(rssi)==0;
	 lcd_error |= lcd.print(F("/"))==0;
	 lcd_error |= lcd.print(ber)==0;
         #endif

	 is_client_connected = true;
         is_event_client_executed = true;
         gsm_state = state_after_wait;
         noInterrupts();
         is_event_gsm = false;
         ready_tasks_count--;
         interrupts();
      break;

      case GSM_STOP_CONNECTION:
         sim800_status = s800.stopConnection();

         // success
         if (sim800_status == SIM800_OK) {
            gsm_state = GSM_SWITCH_OFF;
	    LOGV(F("GSM_STOP_CONNECTION ---> GSM_SWITCH_OFF"));
         }
         // fail
         else if (sim800_status == SIM800_ERROR) {
            is_error = true;
            gsm_state = GSM_SWITCH_OFF;
	    LOGV(F("GSM_STOP_CONNECTION ---> GSM_SWITCH_OFF"));
         }
         // wait
      break;

      case GSM_WAIT_FOR_SWITCH_OFF:
         delay_ms = SIM800_POWER_ON_TO_OFF_DELAY_MS;
         start_time_ms = millis();
         state_after_wait = GSM_SWITCH_OFF;
         gsm_state = GSM_WAIT_STATE;
	 LOGV(F("GSM_WAIT_FOR_SWITCH_OFF ---> GSM_WAIT_STATE"));
      break;

      case GSM_SWITCH_OFF:
         sim800_status = s800.switchOff(power_off_mode);

         // success
         if (sim800_status == SIM800_OK) {
            delay_ms = SIM800_WAIT_FOR_POWER_OFF_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = GSM_END;
            gsm_state = GSM_WAIT_STATE;
	    LOGV(F("GSM_SWITCH_OFF ---> GSM_WAIT_STATE"));
         }
         // fail
         else if (sim800_status == SIM800_ERROR) {
            if (power_off_mode == SIM800_POWER_OFF_BY_AT_COMMAND) {
               power_off_mode = SIM800_POWER_OFF_BY_SWITCH;
            }
            else {
               gsm_state = GSM_END;
	       LOGV(F("GSM_SWITCH_OFF ---> GSM_END"));
            }
         }
         // wait...
      break;

      case GSM_END:
         if (is_error) {
         }
         is_event_client_executed = true;
         is_client_connected = false;
         is_client_udp_socket_open = false;
         noInterrupts();
         is_event_gsm = false;
         ready_tasks_count--;
         interrupts();
         gsm_state = GSM_INIT;
	 LOGV(F("GSM_END ---> GSM_INIT"));
      break;

      case GSM_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            gsm_state = state_after_wait;
         }
      break;
   }
}

#endif

void sensors_reading_task (bool do_prepare, bool do_get, char *driver, char *type, uint8_t address, uint8_t node, uint8_t *sensor_index, uint32_t *wait_time) {
   static uint8_t i;
   static uint8_t retry;
   static sensors_reading_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;
   static bool is_sensor_found;

   switch (sensors_reading_state) {
      case SENSORS_READING_INIT:
	 sensor_reading_failed_count=0;
         i = 0;
         is_sensor_found = false;

         if (driver && type && address && node) {
            while (!is_sensor_found && (i < sensors_count)) {
	      if (sensors[i] == NULL){
		LOGE(F("Skip NULL Sensor"));
		sensor_reading_failed_count++;
		i++;
		continue;
	      }
	      is_sensor_found = strcmp(sensors[i]->getDriver(), driver) == 0 && strcmp(sensors[i]->getType(), type) == 0 && sensors[i]->getAddress() == address && sensors[i]->getNode() == node;
	      if (!is_sensor_found) {
		i++;
	      }
            }

            if (is_sensor_found) {
               *sensor_index = i;
            }
         }

         #if (USE_LCD)

	 // normal OR test: print
	 if (!is_first_run || is_test) {
	   lcd_error |= lcd.setCursor(14, 3);       // clear failed sensors
	   lcd_error |= lcd.print(F("      "))==0;
	   lcd_error |= lcd.setCursor(0, 1);
	   lcd_error |= lcd.print(F("                    "))==0;
	   lcd_error |= lcd.setCursor(0, 2);
	   lcd_error |= lcd.print(F("                    "))==0;
	   //lcd_error |= lcd.setCursor(0, 3);
	   //lcd_error |= lcd.print(F("                    "))==0;
	   lcd_error |= lcd.setCursor(0, 1);
	   if (is_test){
	     lcd_error |= lcd.print(F("T "))==0;
	     if(sensors_count > 7) display_set++;        // change set of sensors to display for test
	     if (display_set > DISPLAY_SET_MAX) display_set=1; 	     
	   }else{
	     lcd_error |= lcd.print(F("R "))==0;
	   }
	 }
	 #endif
	 
         if (do_prepare) {
            LOGN(F("Sensors reading..."));
            retry = 0;

            if (driver && type && address && node && is_sensor_found) {
               if (sensors[i]) sensors[i]->resetPrepared();
            }
            else {
               for (i=0; i<sensors_count; i++) {
		 if (sensors[i]) sensors[i]->resetPrepared(is_test);
               }
               i = 0;
            }

            state_after_wait = SENSORS_READING_INIT;
            sensors_reading_state = SENSORS_SETUP_CHECK;
            LOGV(F("SENSORS_READING_INIT ---> SENSORS_SETUP_CHECK"));
         }
         else if (do_get) {
            sensors_reading_state = SENSORS_READING_GET;
            LOGV(F("SENSORS_READING_INIT ---> SENSORS_READING_GET"));
         }
         else {
            sensors_reading_state = SENSORS_READING_END;
            LOGV(F("SENSORS_READING_INIT ---> SENSORS_READING_END"));
         }
      break;

   case SENSORS_SETUP_CHECK:

        // initialize to missing value
        json_sensors_data[i][0]='\0';

	//if driver is NULL skip it
	if (sensors[i] == NULL){
	  state_after_wait = SENSORS_SETUP_CHECK;
	  sensors_reading_state = SENSORS_READING_NEXT;
	  LOGV(F("SENSORS_READING_INIT ---> SENSORS_READING_NEXT"));
	  break;
	}
	
	if (sensors[i]->getErrorCount() > 0) LOGN(F("Sensor %s-%s-%d error count: %d"),
	     sensors[i]->getDriver(), sensors[i]->getType(), sensors[i]->getAddress(),
	     sensors[i]->getErrorCount());

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
	  sensor_reading_failed_count++;
	  sensors_reading_state = SENSORS_READING_NEXT;
	}

	break;

      case SENSORS_READING_PREPARE:
         sensors[i]->prepare(is_test);
         delay_ms = sensors[i]->getDelay();
         start_time_ms = sensors[i]->getStartTime();

         if (driver && type && address && node) {
            *wait_time = delay_ms;
         }

         if (delay_ms) {
            state_after_wait = SENSORS_READING_IS_PREPARED;
            sensors_reading_state = SENSORS_READING_WAIT_STATE;
            LOGV(F("SENSORS_READING_PREPARE ---> SENSORS_READING_WAIT_STATE"));
         }
         else {
            sensors_reading_state = SENSORS_READING_IS_PREPARED;
            LOGV(F("SENSORS_READING_PREPARE ---> SENSORS_READING_IS_PREPARED"));
         }
      break;

      case SENSORS_READING_IS_PREPARED:
         // success
         if (sensors[i]->isPrepared()) {
            retry = 0;

            if (do_get) {
               sensors_reading_state = SENSORS_READING_GET;
               LOGV(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_GET"));
            }
            else {
               sensors_reading_state = SENSORS_READING_END;
               LOGV(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_END"));
            }
         }
         // retry
         else if ((++retry) < SENSORS_RETRY_COUNT_MAX) {
            i2c_error++;
            delay_ms = SENSORS_RETRY_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SENSORS_READING_PREPARE;
            sensors_reading_state = SENSORS_READING_WAIT_STATE;
            LOGE(F("Sensor is prepared... [ retry ]"));
            LOGV(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_WAIT_STATE"));
         }
         // fail
         else {
	   LOGE(F("Sensor is prepared... [ %s ]"),FAIL_STRING);
            if (do_get) {
               sensors_reading_state = SENSORS_READING_GET;
               LOGV(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_GET"));
            }
            else {
               sensors_reading_state = SENSORS_READING_END;
               LOGV(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_END"));
            }
            retry = 0;
            i2c_error++;
         }
      break;

      case SENSORS_READING_GET:

	int32_t values_readed_from_sensor[VALUES_TO_READ_FROM_SENSOR_COUNT];
	sensors[i]->getJson(&values_readed_from_sensor[0], VALUES_TO_READ_FROM_SENSOR_COUNT,json_sensors_data_test,JSON_BUFFER_LENGTH,is_test);
        if (!is_test) {
          strcpy(json_sensors_data[i],json_sensors_data_test);
        }

        delay_ms = sensors[i]->getDelay();
        start_time_ms = sensors[i]->getStartTime();

        if (delay_ms) {
          state_after_wait = SENSORS_READING_IS_GETTED;
          sensors_reading_state = SENSORS_READING_WAIT_STATE;
          LOGV(F("SENSORS_READING_GET ---> SENSORS_READING_WAIT_STATE"));
        }
        else {
          sensors_reading_state = SENSORS_READING_IS_GETTED;
          LOGV(F("SENSORS_READING_GET ---> SENSORS_READING_IS_GETTED"));
        }
      break;

      case SENSORS_READING_IS_GETTED:
         // end
         if (sensors[i]->isEnd() && !sensors[i]->isReaded()) {
            // success
            if (sensors[i]->isSuccess()) {
               retry = 0;
	       i2c_error = 0;
               sensors_reading_state = SENSORS_READING_READ;
               LOGV(F("SENSORS_READING_IS_GETTED ---> SENSORS_READING_READ"));
            }
            // retry
            else if ((++retry) < SENSORS_RETRY_COUNT_MAX) {
               i2c_error++;
               delay_ms = SENSORS_RETRY_DELAY_MS;
               start_time_ms = millis();
               state_after_wait = SENSORS_READING_GET;
               sensors_reading_state = SENSORS_READING_WAIT_STATE;
	       LOGE(F("Sensor is getted... [ retry ] %s"),sensors[i]->getType());
               LOGV(F("SENSORS_READING_IS_GETTED ---> SENSORS_READING_WAIT_STATE"));
            }
            // fail
            else {
               retry = 0;
               i2c_error++;
               sensors_reading_state = SENSORS_READING_READ;
	       LOGE(F("Sensor is getted... [ %s ] %s"),FAIL_STRING,sensors[i]->getType());
               LOGV(F("SENSORS_READING_IS_GETTED ---> SENSORS_READING_READ"));
            }
         }
         // not end
         else {
            sensors_reading_state = SENSORS_READING_GET;
            LOGT(F("Sensor is getted... [ not end ]"));
            LOGV(F("SENSORS_READING_IS_GETTED ---> SENSORS_READING_GET"));
         }
      break;

      case SENSORS_READING_READ:

         #if (USE_LCD)
	 // normal OR test: print
	 if (!is_first_run || is_test) {
	   LOGN(F("[%s] %s-%s-%d\tJson: %s \tMetadata: %s"), is_test? "Test" : "Report",
	 	sensors[i]->getDriver(),sensors[i]->getType(),sensors[i]->getAddress(),
		&json_sensors_data_test,
		readable_configuration.sensors[i].mqtt_topic);

	   StaticJsonDocument<JSON_BUFFER_LENGTH*2> doc;
	   DeserializationError error = deserializeJson(doc,json_sensors_data_test);
	   if (error) {
	     LOGE(F("deserializeJson() failed with code %s"),error.c_str());
	     lcd_error |= lcd.setCursor(2, 1);
	     lcd_error |= lcd.print(F("Error testing sensors "))==0;
	   }else{
	     // line 1
	     int32_t value = doc["B12101"] | INT32_MAX;
	     if (ISVALID_INT32(value) && strcmp(readable_configuration.sensors[i].mqtt_topic,"254,0,0/103,2000,-,-/")==0){
	       lcd_error |= lcd.setCursor(2, 1);
	       lcd_error |= lcd.print((value - SENSOR_DRIVER_C_TO_K) / 100.0,1)==0;
	       lcd_error |= lcd.print(F("C "))==0;
	     }
	     
	     value = doc["B13003"] | INT32_MAX;
	     if (ISVALID_INT32(value) && strcmp(readable_configuration.sensors[i].mqtt_topic,"254,0,0/103,2000,-,-/")==0){
	       lcd_error |= lcd.setCursor(9, 1);
	       lcd_error |= lcd.print(value)==0;
	       lcd_error |= lcd.print(F("% "))==0;
	     }
	     
	     value = doc["B13011"] | INT32_MAX;
	     if (ISVALID_INT32(value)){
	       if  (strcmp(readable_configuration.sensors[i].mqtt_topic,"1,0,180/1,-,-,-/")==0  ||
		    strcmp(readable_configuration.sensors[i].mqtt_topic,"1,0,300/1,-,-,-/")==0  ||
		    strcmp(readable_configuration.sensors[i].mqtt_topic,"1,0,900/1,-,-,-/")==0  ||
		    strcmp(readable_configuration.sensors[i].mqtt_topic,"1,0,1800/1,-,-,-/")==0 ||
		    strcmp(readable_configuration.sensors[i].mqtt_topic,"1,0,3600/1,-,-,-/")==0
		    ){
		 lcd_error |= lcd.setCursor(13, 1);
		 lcd_error |= lcd.print((value/10.0),1)==0;
		 lcd_error |= lcd.print(F("mm "))==0;
	       }
	     }
	     
	     if (display_set == 1 ){
	       // line 2	     
	       value = doc["B25025"] | INT32_MAX;
	       if (ISVALID_INT32(value) && strcmp(readable_configuration.sensors[i].mqtt_topic,"254,0,0/265,1,-,-/")==0){
		 lcd_error |= lcd.setCursor(0, 2);
		 lcd_error |= lcd.print((value/10.),1)==0;
		 lcd_error |= lcd.print(F("Vb "))==0;
	       }
	       
	       value = doc["B25192"] | INT32_MAX;
	       if (ISVALID_INT32(value) && strcmp(readable_configuration.sensors[i].mqtt_topic,"254,0,0/265,1,-,-/")==0){
		 lcd_error |= lcd.setCursor(7, 2);
		 lcd_error |= lcd.print(value)==0;
		 lcd_error |= lcd.print(F("% "))==0;
	       }
	       
	       value = doc["B25194"] | INT32_MAX;
	       if (ISVALID_INT32(value) && strcmp(readable_configuration.sensors[i].mqtt_topic,"254,0,0/265,1,-,-/")==0){
		 lcd_error |= lcd.setCursor(12, 2);
		 lcd_error |= lcd.print((value/10.),1)==0;
		 lcd_error |= lcd.print(F("Vp "))==0;
	       }
	     }else if (display_set == 2 ){
	       // line 2	     
	       value = doc["B11002"] | INT32_MAX;
	       if (ISVALID_INT32(value) && strcmp(readable_configuration.sensors[i].mqtt_topic,"254,0,0/103,10000,-,-/")==0){
		 lcd_error |= lcd.setCursor(0, 2);
		 lcd_error |= lcd.print((value/10.),1)==0;
		 lcd_error |= lcd.print(F("ms "))==0;
	       }
	       
	       value = doc["B11001"] | INT32_MAX;
	       if (ISVALID_INT32(value) && strcmp(readable_configuration.sensors[i].mqtt_topic,"254,0,0/103,10000,-,-/")==0){
		 lcd_error |= lcd.setCursor(7, 2);
		 lcd_error |= lcd.print(value)==0;
		 lcd_error |= lcd.print(F("^ "))==0;
	       }
	       
	       value = doc["B14198"] | INT32_MAX;
	       if (ISVALID_INT32(value)){

		 if  (strcmp(readable_configuration.sensors[i].mqtt_topic,"0,0,180/1,-,-,-/")==0  ||
		      strcmp(readable_configuration.sensors[i].mqtt_topic,"0,0,300/1,-,-,-/")==0  ||
		      strcmp(readable_configuration.sensors[i].mqtt_topic,"0,0,900/1,-,-,-/")==0  ||
		      strcmp(readable_configuration.sensors[i].mqtt_topic,"0,0,1800/1,-,-,-/")==0 ||
		      strcmp(readable_configuration.sensors[i].mqtt_topic,"0,0,3600/1,-,-,-/")==0
		      ){
		   
		   lcd_error |= lcd.setCursor(12, 2);
		   lcd_error |= lcd.print(value)==0;
		   lcd_error |= lcd.print(F("wm2"))==0;
		 }
	       }
	     }
	   }
	   // line 3
	   //lcd_error |= lcd.setCursor(0, 3);
	   /*
	     else if (strcmp(sensors[i]->getType(), "OA3") == 0) {
	     if (ISVALID(values_readed_from_sensor[i][0])) {
	     lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "%.0f %.0f %.0f ug/m3", values_readed_from_sensor[i][0]/10.0, values_readed_from_sensor[i][1]/10.0, values_readed_from_sensor[i][2]/10.0);
	     }
	     else {
	     lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "--- --- --- ug/m3");
	     }
	     }else if (strcmp(sensors[i]->getType(), "LWT") == 0) {
	     if (ISVALID(values_readed_from_sensor[i][0])) {
	     lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "%.0f'", (values_readed_from_sensor[i][0]*10.0/60.0));
	     }
	     else {
	     lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "--'");
	     }
	     }
	     else if (strcmp(sensors[i]->getType(), "DW1") == 0) {
	     if (ISVALID(values_readed_from_sensor[i][1])) {
	     lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "%.1fm/s ", (values_readed_from_sensor[i][1]/10.0));
	     }
	     else {
	     lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "--.-m/s ");
	     }

	     if (ISVALID(values_readed_from_sensor[i][0])) {
	     lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "%ld%c", values_readed_from_sensor[i][0], 0b11011111);
	     }
	     else {
	     lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "---%c", 0b11011111);
	     }
	     }
	     else if (strcmp(sensors[i]->getType(), "DEP") == 0) {
	     if (ISVALID(values_readed_from_sensor[i][0])) {
	     lcd_count[0] += snprintf(&lcd_buffer[0][0]+lcd_count[0], LCD_COLUMNS-lcd_count[0], "%.1fV", (values_readed_from_sensor[i][1]/10.0));
	     }
	     else {
	     lcd_count[0] += snprintf(&lcd_buffer[0][0]+lcd_count[0], LCD_COLUMNS-lcd_count[0], "--.-V");
	     }
	     }
	   */
	 }
         #endif

	 if (driver && type && address && node) {
            sensors_reading_state = SENSORS_READING_END;
            LOGV(F("SENSORS_READING_READ ---> SENSORS_READING_END"));
         }
         else {
            sensors_reading_state = SENSORS_READING_NEXT;
            LOGV(F("SENSORS_READING_READ ---> SENSORS_READING_NEXT"));
         }
      break;

      case SENSORS_READING_NEXT:
        // next sensor
        if ((++i) < sensors_count) {
          retry = 0;
          sensors_reading_state = SENSORS_SETUP_CHECK;
          LOGV(F("SENSORS_READING_NEXT ---> SENSORS_SETUP_CHECK"));
        }
        // success: all sensors readed
        else {
          // first time: read ptr data from sdcard
          if (is_first_run && !is_test) {
            #if (USE_MQTT)
            noInterrupts();
            if (!is_event_supervisor && is_event_mqtt_paused) {
              is_event_supervisor = true;
              ready_tasks_count++;
            }
            interrupts();
            #endif
          }

          // other time but not in test: save data to sdcard
          // normal AND NOT test: save
          if (!is_first_run && !is_test) {
            #if (USE_SDCARD)
            noInterrupts();
            if (!is_event_data_saving) {
              is_event_data_saving = true;
              ready_tasks_count++;
            }
            interrupts();
            #endif
          }

          sensors_reading_state = SENSORS_READING_END;
          LOGV(F("SENSORS_READING_NEXT ---> SENSORS_READING_END"));
        }
        break;

      case SENSORS_READING_END:
        is_first_test = false;

        if (do_reset_first_run) {
          is_first_run = false;
        }

        noInterrupts();
        if (is_event_sensors_reading) {
          is_event_sensors_reading = false;
          ready_tasks_count--;
        }

        if (is_event_sensors_reading_rpc) {
          is_event_sensors_reading_rpc = false;
        }
        interrupts();


        #if (USE_LCD)
	if (!is_first_run){
	  lcd_error |= lcd.setCursor(14, 3);
	  lcd_error |= lcd.print(F("FA"))==0;
	  lcd_error |= lcd.print(sensor_reading_failed_count)==0;
	  lcd_error |= lcd.print((sensor_reading_failed_count == 0) ? "OK" : "KO")==0;
	}
        #endif

        sensors_reading_state = SENSORS_READING_INIT;
        LOGV(F("SENSORS_READING_END ---> SENSORS_READING_INIT"));
      break;

      case SENSORS_READING_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            sensors_reading_state = state_after_wait;
         }
      break;
   }
}

#if (USE_SDCARD)
void data_saving_task() {
   static uint8_t retry;
   static data_saving_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;
   static uint8_t i;
   static uint8_t k;
   static uint8_t data_count;
   static uint16_t sd_data_count;
   static char sd_buffer[MQTT_SENSOR_TOPIC_LENGTH + MQTT_MESSAGE_LENGTH];
   static char topic_buffer[VALUES_TO_READ_FROM_SENSOR_COUNT][MQTT_SENSOR_TOPIC_LENGTH];
   static char message_buffer[VALUES_TO_READ_FROM_SENSOR_COUNT][MQTT_MESSAGE_LENGTH];
   char file_name[SDCARD_FILES_NAME_MAX_LENGTH];

   switch (data_saving_state) {
      case DATA_SAVING_INIT:
         retry = 0;
         sd_data_count = 0;

         if (is_sdcard_open) {
            data_saving_state = DATA_SAVING_OPEN_FILE;
            LOGV(F("DATA_SAVING_INIT ---> DATA_SAVING_OPEN_FILE"));
         }
         else {
            data_saving_state = DATA_SAVING_OPEN_SDCARD;
            LOGV(F("DATA_SAVING_INIT ---> DATA_SAVING_OPEN_SDCARD"));
         }
      break;

      case DATA_SAVING_OPEN_SDCARD:
	 if (sdcard_init(&SD, SDCARD_CHIP_SELECT_PIN)) {
	   LOGN(F("SDCARD opened Data Saving"));

           #if (ENABLE_SDCARD_LOGGING)
	   init_logging();
	   #endif

	   retry = 0;
	   is_sdcard_open = true;
	   is_sdcard_error = false;
	   data_saving_state = DATA_SAVING_OPEN_FILE;
	   LOGV(F("DATA_SAVING_OPEN_SDCARD ---> DATA_SAVING_OPEN_FILE"));
	 }
         // retry
         else if ((++retry) < DATA_SAVING_RETRY_COUNT_MAX) {
            delay_ms = DATA_SAVING_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = DATA_SAVING_OPEN_SDCARD;
            data_saving_state = DATA_SAVING_WAIT_STATE;
            LOGE(F("SDcard data saving open sdcard... [ retry ]"));
            LOGV(F("DATA_SAVING_OPEN_SDCARD ---> DATA_SAVING_WAIT_STATE"));
         }
         // fail
         else {
            is_sdcard_error = true;
            is_sdcard_open = false;
            LOGE(F("SD Card... [ FAIL ]"));
	    LOGE(F("--> is card inserted?"));
	    LOGE(F("--> there is a valid FAT32 filesystem?"));

            data_saving_state = DATA_SAVING_END;
            LOGV(F("DATA_SAVING_OPEN_SDCARD ---> DATA_SAVING_END"));
         }
      break;

      case DATA_SAVING_OPEN_FILE:
         // open sdcard file: today!
         sdcard_make_filename(now(), file_name);

         // try to open file. if ok, write sensors data on it.
         if (sdcard_open_file(&SD, &write_data_file, file_name, O_RDWR | O_CREAT | O_APPEND)) {
            retry = 0;
            i = 0;
            data_saving_state = DATA_SAVING_SENSORS_LOOP;
            LOGV(F("DATA_SAVING_OPEN_FILE ---> DATA_SAVING_SENSORS_LOOP"));
         }
         // retry
         else if ((++retry) < DATA_SAVING_RETRY_COUNT_MAX) {
            delay_ms = DATA_SAVING_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = DATA_SAVING_OPEN_FILE;
            data_saving_state = DATA_SAVING_WAIT_STATE;
            LOGE(F("SDcard openfile... [ retry ]"));
            LOGV(F("DATA_SAVING_OPEN_FILE ---> DATA_SAVING_WAIT_STATE"));
         }
         // fail
         else {
            LOGE(F("SD Card open file %s... [ FAIL ]"), file_name);
            is_sdcard_error = true;
            data_saving_state = DATA_SAVING_END;
            LOGV(F("DATA_SAVING_OPEN_FILE ---> DATA_SAVING_END"));
         }
      break;

      case DATA_SAVING_SENSORS_LOOP:
         if (i < sensors_count) {
            k = 0;
            data_count = jsonToMqtt(&json_sensors_data[i][0], readable_configuration.sensors[i].mqtt_topic, topic_buffer, message_buffer, (tmElements_t *) &sensor_reading_time);
            data_saving_state = DATA_SAVING_DATA_LOOP;
            LOGV(F("DATA_SAVING_SENSORS_LOOP ---> DATA_SAVING_DATA_LOOP"));
         }
         else {
            LOGT(F(""));
            data_saving_state = DATA_SAVING_CLOSE_FILE;
            LOGV(F("DATA_SAVING_SENSORS_LOOP ---> DATA_SAVING_CLOSE_FILE"));
         }
      break;

      case DATA_SAVING_DATA_LOOP:
         if (k < data_count) {
            mqttToSd(&topic_buffer[k][0], &message_buffer[k][0], sd_buffer);
            data_saving_state = DATA_SAVING_WRITE_FILE;
            LOGV(F("DATA_SAVING_DATA_LOOP ---> DATA_SAVING_WRITE_FILE"));
         }
         else {
            i++;
            data_saving_state = DATA_SAVING_SENSORS_LOOP;
            LOGV(F("DATA_SAVING_DATA_LOOP ---> DATA_SAVING_SENSORS_LOOP"));
         }
      break;

      case DATA_SAVING_WRITE_FILE:
         // sdcard success
         if (write_data_file.write(sd_buffer, MQTT_SENSOR_TOPIC_LENGTH + MQTT_MESSAGE_LENGTH) == (MQTT_SENSOR_TOPIC_LENGTH + MQTT_MESSAGE_LENGTH)) {
            LOGT(F("SD <-- %s %s"), &topic_buffer[k][0], &message_buffer[k][0]);
            write_data_file.flush();
            retry = 0;
            k++;
            sd_data_count++;
            data_saving_state = DATA_SAVING_DATA_LOOP;
            LOGV(F("DATA_SAVING_WRITE_FILE ---> DATA_SAVING_DATA_LOOP"));
         }
         // retry
         else if ((++retry) < DATA_SAVING_RETRY_COUNT_MAX) {
            delay_ms = DATA_SAVING_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = DATA_SAVING_WRITE_FILE;
            data_saving_state = DATA_SAVING_WAIT_STATE;
            LOGE(F("SDcard writing data on  file %s... [ retry ]"), file_name);
            LOGV(F("DATA_SAVING_WRITE_FILE ---> DATA_SAVING_WAIT_STATE"));
         }
         // fail
         else {
            LOGE(F("SD Card writing data on file %s... [ FAIL ]"), file_name);
            is_sdcard_error = true;
            data_saving_state = DATA_SAVING_CLOSE_FILE;
            LOGV(F("DATA_SAVING_WRITE_FILE ---> DATA_SAVING_CLOSE_FILE"));
         }
      break;

      case DATA_SAVING_CLOSE_FILE:
            is_sdcard_error = !write_data_file.close();
            data_saving_state = DATA_SAVING_END;
            LOGV(F("DATA_SAVING_CLOSE_FILE ---> DATA_SAVING_END"));
         break;

      case DATA_SAVING_END:
         LOGN(F("[ %d ] data stored in sdcard... [ %s ]"), sd_data_count, is_sdcard_error ? ERROR_STRING : OK_STRING);
         #if (USE_LCD)
	 lcd_error |= lcd.setCursor(0, 3);
	 lcd_error |= lcd.print(F("             "))==0;
	 lcd_error |= lcd.setCursor(0, 3);
         lcd_error |= lcd.print(F("SD"))==0;
	 lcd_error |= lcd.print(sd_data_count)==0;
	 lcd_error |= lcd.print(is_sdcard_error ? "KO" : "OK")==0;
         #endif

         noInterrupts();
         if (!is_event_supervisor) {
            is_event_supervisor = true;
            ready_tasks_count++;
         }

         is_event_data_saving = false;
         ready_tasks_count--;
         interrupts();

         data_saving_state = DATA_SAVING_INIT;
         LOGV(F("DATA_SAVING_END ---> DATA_SAVING_INIT"));
      break;

      case DATA_SAVING_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            data_saving_state = state_after_wait;
         }
      break;
   }
}
#endif

#if (USE_MQTT)
void mqtt_task() {
   static uint8_t retry;
   static mqtt_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;
   static uint8_t i;
   static uint8_t k;
   static uint16_t mqtt_data_count;
   static uint8_t data_count;
   static char sd_buffer[MQTT_SENSOR_TOPIC_LENGTH + MQTT_MESSAGE_LENGTH];
   static char topic_buffer[JSONS_TO_READ_FROM_SENSOR_COUNT][MQTT_SENSOR_TOPIC_LENGTH];
   static char message_buffer[JSONS_TO_READ_FROM_SENSOR_COUNT][MQTT_MESSAGE_LENGTH];
   static char full_topic_buffer[MQTT_ROOT_TOPIC_LENGTH + MQTT_SENSOR_TOPIC_LENGTH];
   static bool is_mqtt_error;
   static bool is_mqtt_processing_sdcard;
   static bool is_mqtt_processing_json;
   static bool is_mqtt_published_data;
   static bool is_ptr_found;
   static bool is_ptr_updated;
   static bool is_eof_data_read;
   static bool is_mqtt_constantdata;
   static tmElements_t datetime;
   static time_t current_ptr_time_data;
   static time_t last_correct_ptr_time_data;
   static time_t next_ptr_time_data;
   static uint32_t ipstack_timeout_ms;
   uint8_t ipstack_status;
   char file_name[SDCARD_FILES_NAME_MAX_LENGTH];
   int read_bytes_count;
   static char comtopic[MQTT_RPC_TOPIC_LENGTH+3];    // static is required here for MQTTClient
   //static uint8_t mqtt_numretry_rpc_response;
   time_t date_time;
   
   // check every loop to send RPC response
   // callback driven !

   /*
   // try to send response for three times
   if (mqtt_numretry_rpc_response > 3 ){
     rpcpayload[0]=0;
     is_mqtt_error = true;
     mqtt_numretry_rpc_response=0;
   }
   */

   if (strlen(rpcpayload) > 0 && mqtt_client.isConnected()){
     char restopic[MQTT_RPC_TOPIC_LENGTH+3];
     strcpy(restopic,readable_configuration.mqtt_rpc_topic);
     strcat(restopic, "res");
     if (mqttPublish(restopic,rpcpayload)){
       LOGT(F("MQTT RPC response <-- %s %s"), restopic, rpcpayload);
       rpcpayload[0]=0;
       //mqtt_numretry_rpc_response=0;
     }else{
       LOGE(F("MQTT RPC response ERROR <-- %s %s"), restopic, rpcpayload);
       //mqtt_numretry_rpc_response++;
     }
   }

   switch (mqtt_state) {
      case MQTT_INIT:
         retry = 0;
         is_ptr_found = false;
         is_ptr_updated = false;
         is_eof_data_read = false;
         is_mqtt_error = false;
         is_mqtt_published_data = false;
	 is_mqtt_constantdata = false;
	 is_mqtt_rpc_delay =false;
         mqtt_data_count = 0;
	 //mqtt_numretry_rpc_response =0;

	 strcpy(comtopic,readable_configuration.mqtt_rpc_topic);
	 strcat(comtopic, "com");
	 rpcpayload[0]=0;

         if (!is_sdcard_open || is_sdcard_error) {
            mqtt_state = MQTT_OPEN_SDCARD;
            LOGV(F("MQTT_PTR_DATA_INIT ---> MQTT_OPEN_SDCARD"));
         }
         else if (is_sdcard_open) {
            mqtt_state = MQTT_OPEN_PTR_FILE;
            LOGV(F("MQTT_PTR_DATA_INIT ---> MQTT_OPEN_PTR_FILE"));
         }
         else {
            mqtt_state = MQTT_PTR_END;
            LOGV(F("MQTT_PTR_DATA_INIT ---> MQTT_PTR_END"));
         }
      break;

      case MQTT_OPEN_SDCARD:
         if (sdcard_init(&SD, SDCARD_CHIP_SELECT_PIN)) {
	   LOGN(F("SDCARD opened MQTT"));

            #if (ENABLE_SDCARD_LOGGING)
            init_logging();
	    #endif

            retry = 0;
            is_sdcard_open = true;
            is_sdcard_error = false;
            mqtt_state = MQTT_OPEN_PTR_FILE;
            LOGV(F("MQTT_OPEN_SDCARD ---> MQTT_OPEN_PTR_FILE"));
         }
         // retry
         else if ((++retry) < MQTT_RETRY_COUNT_MAX) {
            delay_ms = MQTT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = MQTT_OPEN_SDCARD;
            mqtt_state = MQTT_WAIT_STATE;
            LOGE(F("MQTT open SDcard... [ retry ]"));
	    LOGV(F("MQTT_OPEN_SDCARD ---> MQTT_PTR_DATA_WAIT_STATE"));
         }
         // fail
         else {
            is_sdcard_error = true;
            is_sdcard_open = false;
            retry = 0;
            LOGE(F("SD Card... [ FAIL ]"));
	    LOGE(F("--> is card inserted?"));
	    LOGE(F("--> there is a valid FAT32 filesystem?"));

            mqtt_state = MQTT_PTR_END;
            LOGV(F("MQTT_OPEN_SDCARD ---> MQTT_PTR_END"));
         }
         break;

      case MQTT_OPEN_PTR_FILE:
	 // if we have SDCARD_MQTT_PTR_RPC_FILE_NAME rename as current SDCARD_MQTT_PTR_FILE_NAME
	 if (SD.exists(SDCARD_MQTT_PTR_RPC_FILE_NAME)) {
	   LOGN(F("file %s exists"),SDCARD_MQTT_PTR_RPC_FILE_NAME );
	   if (SD.remove(SDCARD_MQTT_PTR_FILE_NAME)) {
	     LOGN(F("file %s removed"),SDCARD_MQTT_PTR_FILE_NAME );
	   }
	   // rename file coming from recovery rpc if exist
	   if (SD.rename(SDCARD_MQTT_PTR_RPC_FILE_NAME, SDCARD_MQTT_PTR_FILE_NAME)) {
	     LOGN(F("PTR RPC file renamed to PTR file"));
	   }
	 } else {
	   LOGN(F("file %s do not exists"),SDCARD_MQTT_PTR_RPC_FILE_NAME );
	 }

         // try to open file. if ok, read ptr data.
         if (sdcard_open_file(&SD, &mqtt_ptr_file, SDCARD_MQTT_PTR_FILE_NAME, O_RDWR | O_CREAT)) {
            retry = 0;
            mqtt_state = MQTT_PTR_READ;
            LOGV(F("MQTT_OPEN_PTR_FILE ---> MQTT_PTR_READ"));
         }
         // retry
         else if ((++retry) < MQTT_RETRY_COUNT_MAX) {
            delay_ms = MQTT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = MQTT_OPEN_PTR_FILE;
            mqtt_state = MQTT_WAIT_STATE;
            LOGE(F("SD Card open file %s... [ retry ]"), SDCARD_MQTT_PTR_FILE_NAME);
            LOGV(F("MQTT_OPEN_PTR_FILE ---> MQTT_PTR_DATA_WAIT_STATE"));
         }
         // fail
         else {
            LOGE(F("SD Card open file %s... [ FAIL ]"), SDCARD_MQTT_PTR_FILE_NAME);
            is_sdcard_error = true;
            retry = 0;
            mqtt_state = MQTT_PTR_END;
            LOGV(F("MQTT_OPEN_PTR_FILE ---> MQTT_PTR_END"));
         }
      break;

      case MQTT_PTR_READ:
         ptr_time_data = UINT32_MAX;
         mqtt_ptr_file.seekSet(0);
         read_bytes_count = mqtt_ptr_file.read(&ptr_time_data, sizeof(time_t));

         // found
         if (read_bytes_count == sizeof(time_t) && ptr_time_data < now()) {
            is_ptr_found = true;
            mqtt_state = MQTT_PTR_FOUND;
            LOGV(F("MQTT_PTR_READ ---> MQTT_PTR_FOUND"));
         }
         // not found (no sdcard error): find it by starting one month before
         else if (read_bytes_count >= 0) {
            LOGN(F("Data pointer... [ FIND ]"));
            ptr_time_data = now()-(SECS_PER_DAY*30);
            is_ptr_found = false;
            mqtt_state = MQTT_PTR_FIND;
            LOGV(F("MQTT_PTR_READ ---> MQTT_PTR_FIND"));
         }
         // not found (sdcard error)
         else {
            is_ptr_found = false;
            is_sdcard_error = true;
            mqtt_state = MQTT_PTR_END;
            LOGV(F("MQTT_PTR_READ ---> MQTT_PTR_END"));
         }
      break;

      case MQTT_PTR_FIND:
         // ptr not found. find it by searching in file name until today is reach.
         // if there isn't file, ptr_time_data is set to current date at 00:00:00 time.
         if (!is_ptr_found && ptr_time_data < now()) {
            sdcard_make_filename(ptr_time_data, file_name);

            if (SD.exists(file_name)) {
               is_ptr_found = true;
               is_ptr_updated = true;
               is_eof_data_read = false;
               LOGN(F("%s... [ FOUND ]"), file_name);
               mqtt_state = MQTT_PTR_END;
               LOGV(F("MQTT_PTR_FIND ---> MQTT_PTR_END"));
            }
            else {
               LOGN(F("%s... [ NOT FOUND ]"), file_name);
               ptr_time_data += SECS_PER_DAY;
            }
         }
         // ptr not found: set ptr to today at 00:00:00.
         else if (!is_ptr_found && ptr_time_data >= now()) {
            date_time=now();
            datetime.Year = CalendarYrToTm(year(date_time));
            datetime.Month = month(date_time);
            datetime.Day = day(date_time);
            datetime.Hour = 0;
            datetime.Minute = 0;
            datetime.Second = 0;
            ptr_time_data = makeTime(datetime);
            is_ptr_found = true;
            is_ptr_updated = true;
         }
         // ptr found: sooner or later the ptr will be set in any case
         else if (is_ptr_found) {
            mqtt_state = MQTT_PTR_FOUND;
            LOGV(F("MQTT_PTR_FIND ---> MQTT_PTR_FOUND"));
         }
      break;

      case MQTT_PTR_FOUND:
         // datafile read, reach eof and is today. END.
	 date_time=now();
         if (is_eof_data_read && year(date_time) == year(ptr_time_data) && month(date_time) == month(ptr_time_data) && day(date_time) == day(ptr_time_data)) {
            mqtt_state = MQTT_CLOSE_DATA_FILE;
            LOGV(F("MQTT_PTR_FOUND ---> MQTT_CLOSE_DATA_FILE"));
         }
         // datafile read, reach eof and NOT is today. go to start of next day.
         else if (is_eof_data_read) {
            datetime.Year = CalendarYrToTm(year(ptr_time_data));
            datetime.Month = month(ptr_time_data);
            datetime.Day = day(ptr_time_data);
            datetime.Hour = 0;
            datetime.Minute = 0;
            datetime.Second = 0;
            ptr_time_data = makeTime(datetime)+ SECS_PER_DAY;  // add one day
            is_ptr_updated = true;
            mqtt_state = MQTT_PTR_END;
            LOGV(F("MQTT_PTR_FOUND ---> MQTT_PTR_END"));
         }
         else {
            is_eof_data_read = false;
            mqtt_state = MQTT_PTR_END;
            LOGV(F("MQTT_PTR_FOUND ---> MQTT_PTR_END"));
         }
      break;

      case MQTT_PTR_END:
         // ptr data is found: send data saved on sdcard
         if (is_ptr_found && is_sdcard_open && !is_sdcard_error) {
            last_correct_ptr_time_data = ptr_time_data;
            LOGN(F("Data pointer... [ %d/%d/%d %d:%d:%d ] [ %s ]"), day(ptr_time_data), month(ptr_time_data), year(ptr_time_data), hour(ptr_time_data), minute(ptr_time_data), second(ptr_time_data), OK_STRING);
            mqtt_state = MQTT_OPEN;
            LOGV(F("MQTT_PTR_END ---> MQTT_OPEN"));
         }
         // ptr data is NOT found: sd card fault fallback: send last acquired sensor data
         else {
            LOGN(F("Data pointer... [ --/--/---- --:--:-- ] [ %s ]"), ERROR_STRING);
            is_sdcard_error = true;
            mqtt_state = MQTT_OPEN;
            LOGV(F("MQTT_PTR_END ---> MQTT_OPEN"));
         }
      break;

      case MQTT_OPEN:
         if (is_client_connected && mqtt_client.isConnected()) {
            mqtt_state = MQTT_CHECK;
            LOGV(F("MQTT_OPEN ---> MQTT_CHECK"));
         }
         else if (is_client_connected) {
            ipstack_timeout_ms = 0;
            mqtt_state = MQTT_CONNECT;
            LOGV(F("MQTT_OPEN ---> MQTT_CONNECT"));
         }
         // error: client not connected!
         else {
            is_mqtt_error = true;
            mqtt_state = MQTT_END;
            LOGV(F("MQTT_OPEN ---> MQTT_END"));
         }
         break;

      case MQTT_CONNECT:
         if (ipstack_timeout_ms == 0) {
            ipstack_timeout_ms = millis();
         }

	 mqtt_client.setMessageHandler(comtopic, mqttRxCallback);   // messages queued for persistent client are sended at connect time and we have to "remember" the subscription

         ipstack_status = ipstack.connect(readable_configuration.mqtt_server, readable_configuration.mqtt_port);
	 
	 char mqtt_username[MQTT_PASSWORD_LENGTH + STATIONSLUG_LENGTH + BOARDSLUG_LENGTH];
	 strcpy (mqtt_username, readable_configuration.mqtt_username);
	 strcat (mqtt_username, "/");
	 strcat (mqtt_username, readable_configuration.stationslug);
	 strcat (mqtt_username, "/");
	 strcat (mqtt_username, readable_configuration.boardslug);

         // success
         if (ipstack_status == 1 && mqttConnect(mqtt_username, readable_configuration.mqtt_password)) {
            retry = 0;
            LOGT(F("MQTT Connection... [ %s ]"), OK_STRING);
            mqtt_state = MQTT_ON_CONNECT;
            LOGV(F("MQTT_CONNECT ---> MQTT_ON_CONNECT"));
         }
         // retry
         else if (ipstack_status == 2 && (++retry) < MQTT_RETRY_COUNT_MAX) {
            delay_ms = MQTT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = MQTT_CONNECT;
            mqtt_state = MQTT_WAIT_STATE;
            LOGE(F("MQTT Connection... [ retry ]"));
            LOGV(F("MQTT_CONNECT ---> MQTT_WAIT_STATE"));
         }
         // fail
         #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH)
         else if (ipstack_status == 2 || (millis() - ipstack_timeout_ms >= ETHERNET_MQTT_TIMEOUT_MS)) {
         #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM)
         else if (ipstack_status == 2) {
         #endif
            LOGE(F("MQTT Connection... [ %s ]"), FAIL_STRING);
            is_mqtt_error = true;
            retry = 0;
            mqtt_state = MQTT_RPC_DELAY;
            LOGV(F("MQTT_CONNECT ---> MQTT_RPC_DELAY"));
         }
         // wait
      break;

      case MQTT_ON_CONNECT:
         getFullTopic(full_topic_buffer, readable_configuration.mqtt_maint_topic, MQTT_STATUS_TOPIC);
         snprintf(&message_buffer[0][0], MQTT_MESSAGE_LENGTH, MQTT_ON_CONNECT_MESSAGE,MODULE_MAIN_VERSION,MODULE_MINOR_VERSION);

         if (mqttPublish(full_topic_buffer, &message_buffer[0][0]), true) {
	   //retry = 0;
            mqtt_state = MQTT_SUBSCRIBE;
            LOGV(F("MQTT_ON_CONNECT ---> MQTT_SUBSCRIBE"));
         }
	 /*
         // retry
         else if ((++retry) < MQTT_RETRY_COUNT_MAX) {
            delay_ms = MQTT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = MQTT_ON_CONNECT;
            mqtt_state = MQTT_WAIT_STATE;
            LOGV(F("MQTT_ON_CONNECT ---> MQTT_WAIT_STATE"));
         }
	 */
         // fail
         else {
	   //retry = 0;
            LOGE(F("MQTT on connect publish message... [ %s ]"), FAIL_STRING);
            is_mqtt_error = true;
            mqtt_state = MQTT_RPC_DELAY;
            LOGV(F("MQTT_ON_CONNECT ---> MQTT_RPC_DELAY"));
         }
      break;

      case MQTT_SUBSCRIBE:

	if(!mqtt_session_present) {
	  mqtt_session_present=true;
	  mqtt_client.setMessageHandler(comtopic, NULL); // remove previous handler
	                                                 // MessageHandler in MQTTClient is not cleared betwen sessions

	  bool is_mqtt_subscribed = mqtt_client.subscribe(comtopic, MQTT::QOS1, mqttRxCallback) == 0;  // subscribe and set new handler
	  LOGT(F("MQTT Subscription %s [ %s ]"), comtopic, is_mqtt_subscribed ? OK_STRING : FAIL_STRING);
	}

	mqtt_state = MQTT_CONSTANTDATA;
	LOGV(F("MQTT_SUBSCRIBE ---> MQTT_CONSTANTDATA"));
      break;

      case MQTT_CONSTANTDATA:

	if (!is_mqtt_constantdata) {
	  i = 0;
	  is_mqtt_constantdata = true;
	}

	// publish constant station data without retry
	if (i < readable_configuration.constantdata_count) {
	  //memset(full_topic_buffer, 0, MQTT_ROOT_TOPIC_LENGTH + 14 + CONSTANTDATA_BTABLE_LENGTH);
	  strncpy(full_topic_buffer, readable_configuration.mqtt_root_topic, MQTT_ROOT_TOPIC_LENGTH);
	  strncpy(full_topic_buffer + strlen(readable_configuration.mqtt_root_topic), "-,-,-/-,-,-,-/", 14);
	  strncpy(full_topic_buffer + strlen(readable_configuration.mqtt_root_topic)+14, readable_configuration.constantdata[i].btable, CONSTANTDATA_BTABLE_LENGTH);
	  char payload[CONSTANTDATA_BTABLE_LENGTH+9];
	  // payload is a string as default; add "" around
	  strncpy(payload,"{\"v\":\"",6);
	  strncpy(payload+6,readable_configuration.constantdata[i].value,strlen(readable_configuration.constantdata[i].value));
	  strncpy(payload+6+strlen(readable_configuration.constantdata[i].value),"\"}\0",3);

	  if (mqttPublish(full_topic_buffer,payload)){
	    LOGN(F("MQTT <-- %s %s"), full_topic_buffer, payload);
	  }else{
	    is_mqtt_error = true;
	    LOGE(F("MQTT ERROR <-- %s %s"), full_topic_buffer, payload);
	  }
	}else{
	  mqtt_state = MQTT_CHECK;
	  LOGV(F("MQTT_CONSTANTDATA ---> MQTT_CHECK"));
	}

	i++;

      break;

      case MQTT_CHECK:
         // ptr data is found: send data saved on sdcard
         if (!is_sdcard_error) {
            is_mqtt_processing_json = false;
            is_mqtt_processing_sdcard = true;
            is_eof_data_read = false;
            mqtt_state = MQTT_OPEN_DATA_FILE;
            LOGV(F("MQTT_CHECK ---> MQTT_OPEN_DATA_FILE"));
         }
         // ptr data is NOT found: sd card fault fallback: send last acquired sensor data
         else {
            is_mqtt_processing_json = true;
            is_mqtt_processing_sdcard = false;
            i = 0;
            mqtt_state = MQTT_SENSORS_LOOP;
            LOGV(F("MQTT_CHECK ---> MQTT_SENSORS_LOOP"));
         }
      break;

      case MQTT_SENSORS_LOOP:
         if (i < readable_configuration.sensors_count) {
            k = 0;
            data_count = jsonToMqtt(&json_sensors_data[i][0], readable_configuration.sensors[i].mqtt_topic, topic_buffer, message_buffer, (tmElements_t *) &sensor_reading_time);
            mqtt_state = MQTT_DATA_LOOP;
            LOGV(F("MQTT_SENSORS_LOOP ---> MQTT_DATA_LOOP"));
         }
         else if (is_mqtt_processing_json) {
            mqtt_state = MQTT_RPC_DELAY;
            LOGV(F("MQTT_SENSORS_LOOP ---> MQTT_RPC_DELAY"));
         }
      break;

      case MQTT_SD_LOOP:
         memset(sd_buffer, 0, MQTT_SENSOR_TOPIC_LENGTH + MQTT_MESSAGE_LENGTH);
         read_bytes_count = read_data_file.read(sd_buffer, MQTT_SENSOR_TOPIC_LENGTH + MQTT_MESSAGE_LENGTH);

         if (read_bytes_count == MQTT_SENSOR_TOPIC_LENGTH + MQTT_MESSAGE_LENGTH) {
            sdToMqtt(sd_buffer, &topic_buffer[0][0], &message_buffer[0][0]);
            current_ptr_time_data = getDateFromMessage(&message_buffer[0][0]);

            if (current_ptr_time_data >= last_correct_ptr_time_data) {
               last_correct_ptr_time_data = current_ptr_time_data;
               mqtt_state = MQTT_DATA_LOOP;
               LOGV(F("MQTT_SD_LOOP ---> MQTT_DATA_LOOP"));
            }
         }
         // EOF: End of File
         else {
	    LOGV(F("SDcard no data read: error or end of file"));
            if (last_correct_ptr_time_data > ptr_time_data) {
               ptr_time_data = last_correct_ptr_time_data;
               is_ptr_updated = true;
	    }
	    is_eof_data_read = true;
	    mqtt_state = MQTT_PTR_FOUND;
	    LOGV(F("MQTT_SD_LOOP ---> MQTT_PTR_FOUND"));
         }
      break;

      case MQTT_DATA_LOOP:
         if (k < data_count && is_mqtt_processing_json) {
            getFullTopic(full_topic_buffer, readable_configuration.mqtt_root_topic, &topic_buffer[k][0]);
            mqtt_state = MQTT_PUBLISH;
            LOGV(F("MQTT_DATA_LOOP ---> MQTT_PUBLISH"));
         }
         else if (is_mqtt_processing_sdcard) {
            getFullTopic(full_topic_buffer, readable_configuration.mqtt_root_topic, &topic_buffer[0][0]);
            mqtt_state = MQTT_PUBLISH;
            LOGV(F("MQTT_DATA_LOOP ---> MQTT_PUBLISH"));
         }
         else {
            i++;
            mqtt_state = MQTT_SENSORS_LOOP;
            LOGV(F("MQTT_DATA_LOOP ---> MQTT_SENSORS_LOOP"));
         }
      break;

      case MQTT_PUBLISH:
         is_mqtt_published_data = true;

         // mqtt json success
         if (is_mqtt_processing_json && mqttPublish(full_topic_buffer, &message_buffer[k][0])) {
            LOGN(F("MQTT <-- %s %s"), &topic_buffer[k][0], &message_buffer[k][0]);
            //retry = 0;
            k++;
            mqtt_data_count++;
            mqtt_state = MQTT_DATA_LOOP;
            LOGV(F("MQTT_PUBLISH ---> MQTT_DATA_LOOP"));
         }
         // mqtt sdcard success
         else if (is_mqtt_processing_sdcard && mqttPublish(full_topic_buffer, &message_buffer[0][0])) {
            LOGN(F("MQTT(SD) <-- %s %s"), &topic_buffer[0][0], &message_buffer[0][0]);
            //retry = 0;
            mqtt_data_count++;
            mqtt_state = MQTT_SD_LOOP;
            LOGV(F("MQTT_PUBLISH ---> MQTT_SD_LOOP"));
         }
	 /*
         // retry
         else if ((++retry) < MQTT_RETRY_COUNT_MAX) {
            delay_ms = MQTT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = MQTT_PUBLISH;
            mqtt_state = MQTT_WAIT_STATE;
            LOGV(F("MQTT_PUBLISH ---> MQTT_WAIT_STATE"));
         }
	 */
         // fail
         else {
            ptr_time_data = current_ptr_time_data - readable_configuration.report_seconds;
            is_ptr_updated = true;

            is_eof_data_read = true;
            is_mqtt_error = true;
            LOGE(F("MQTT publish... [ %s ]"), FAIL_STRING);

            if (is_mqtt_processing_json) {
               mqtt_state = MQTT_RPC_DELAY;
               LOGV(F("MQTT_PUBLISH ---> MQTT_RPC_DELAY"));
            }
            else if (is_mqtt_processing_sdcard) {
               mqtt_state = MQTT_CLOSE_DATA_FILE;
               LOGV(F("MQTT_PUBLISH ---> MQTT_CLOSE_DATA_FILE"));
            }
         }
      break;

      case MQTT_OPEN_DATA_FILE:
         // open the file that corresponds to the next data to send
         next_ptr_time_data = ptr_time_data + readable_configuration.report_seconds;
         sdcard_make_filename(next_ptr_time_data, file_name);
	 date_time=now();
	 
         // open file for read data
	 LOGV(F("SDcard open file: %s"),file_name);
         if (sdcard_open_file(&SD, &read_data_file, file_name, O_READ)) {
            retry = 0;
            mqtt_state = MQTT_SD_LOOP;
            LOGV(F("MQTT_OPEN_DATA_FILE ---> MQTT_SD_LOOP"));
         }
         // error: file doesn't exist but if is today, end.
         else if (!is_sdcard_error &&
		  year(next_ptr_time_data) == year(date_time) &&
		  month(next_ptr_time_data) == month(date_time) &&
		  day(next_ptr_time_data) == day(date_time)) {
            mqtt_state = MQTT_PTR_UPDATE;
            LOGV(F("MQTT_OPEN_DATA_FILE ---> MQTT_PTR_UPDATE"));
         }
         // error: file doesn't exist and if it isn't today, jump to next day and search in it
         else if (!is_sdcard_error) {
            is_ptr_found = false;
            ptr_time_data = next_ptr_time_data;
            mqtt_state = MQTT_PTR_FIND;
            LOGV(F("MQTT_OPEN_DATA_FILE ---> MQTT_PTR_FIND"));
         }
         // fail
         else {
            LOGE(F("SD Card open file %s... [ FAIL ]"), file_name);
            is_sdcard_error = true;
            mqtt_state = MQTT_CHECK; // fallback
            LOGV(F("MQTT_OPEN_DATA_FILE ---> MQTT_CHECK"));
         }
         break;

      case MQTT_CLOSE_DATA_FILE:
         if (is_mqtt_processing_sdcard) {
            is_sdcard_error = !read_data_file.close();
         }
	 mqtt_state = MQTT_RPC_DELAY;
	 LOGV(F("MQTT_CLOSE_DATA_FILE ---> MQTT_RPC_DELAY"));
         break;

      case MQTT_RPC_DELAY:

	 //delay_ms = readable_configuration.report_seconds*500UL;
	 delay_ms = 300000UL;
	 start_time_ms = millis();
	 state_after_wait = MQTT_ON_DISCONNECT;
	 mqtt_state = MQTT_WAIT_STATE_RPC;

	 LOGV(F("MQTT_WAIT_STATE_RPC: %lms"),delay_ms);
	 LOGV(F("MQTT_RPC_DELAY ---> MQTT_WAIT_STATE_RPC"));

         break;

      case MQTT_ON_DISCONNECT:
         #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH)
         if (is_mqtt_error) {
         #endif

            getFullTopic(full_topic_buffer, readable_configuration.mqtt_maint_topic, MQTT_STATUS_TOPIC);
            snprintf(&message_buffer[0][0], MQTT_MESSAGE_LENGTH, MQTT_ON_DISCONNECT_MESSAGE);

            if (mqttPublish(full_topic_buffer, &message_buffer[0][0]), true) {
               retry = 0;
               mqtt_state = MQTT_DISCONNECT;
               LOGV(F("MQTT_ON_DISCONNECT ---> MQTT_DISCONNECT"));
            }
            // retry
            else if ((++retry) < MQTT_RETRY_COUNT_MAX) {
               delay_ms = MQTT_DELAY_MS;
               start_time_ms = millis();
               state_after_wait = MQTT_ON_DISCONNECT;
               mqtt_state = MQTT_WAIT_STATE;
               LOGE(F("MQTT on disconnect publish message... [ retry ]"));
               LOGV(F("MQTT_ON_DISCONNECT ---> MQTT_WAIT_STATE"));
            }
            // fail
            else {
               LOGE(F("MQTT on disconnect publish message... [ %s ]"), FAIL_STRING);
               retry = 0;
               is_mqtt_error = true;
               mqtt_state = MQTT_DISCONNECT;
               LOGV(F("MQTT_ON_DISCONNECT ---> MQTT_DISCONNECT"));
            }

         #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH)
         }
         else {
            mqtt_state = MQTT_DISCONNECT;
            LOGV(F("MQTT_ON_DISCONNECT ---> MQTT_DISCONNECT"));
         }
         #endif
      break;

      case MQTT_DISCONNECT:
         #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH)
         if (is_mqtt_error) {
         #endif

	 mqtt_client.disconnect();
	 mqtt_client.setMessageHandler(comtopic, NULL); // remove handler setted
         ipstack.disconnect();
         LOGT(F("MQTT Disconnect... [ %s ]"), OK_STRING);

         #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH)
         }
         #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM)
         // resume GSM task for closing connection
         noInterrupts();
         if (!is_event_gsm) {
            is_event_gsm = true;
            ready_tasks_count++;
         }
         interrupts();
         #endif

         mqtt_state = MQTT_PTR_UPDATE;
         LOGV(F("MQTT_DISCONNECT ---> MQTT_PTR_UPDATE"));

      break;

      case MQTT_PTR_UPDATE:
         if (is_ptr_updated) {
            // set ptr 1 second more for send next data to current ptr
            ptr_time_data++;

            // success
            if (mqtt_ptr_file.seekSet(0) && mqtt_ptr_file.write(&ptr_time_data, sizeof(time_t)) == sizeof(time_t)) {
               mqtt_ptr_file.flush();
               breakTime(ptr_time_data, datetime);
               LOGN(F("Data pointer... [ %d/%d/%d %d:%d:%d ] [ %s ]"), datetime.Day, datetime.Month, tmYearToCalendar(datetime.Year), datetime.Hour, datetime.Minute, datetime.Second, "UPDATE");
               mqtt_state = MQTT_CLOSE_PTR_FILE;
               LOGV(F("MQTT_PTR_UPDATE ---> MQTT_CLOSE_PTR_FILE"));
            }
            // retry
            else if ((++retry) < MQTT_RETRY_COUNT_MAX) {
               delay_ms = MQTT_DELAY_MS;
               start_time_ms = millis();
               state_after_wait = MQTT_PTR_UPDATE;
               mqtt_state = MQTT_WAIT_STATE;
               LOGE(F("SD Card writing ptr data on file %s... [ retry ]"), SDCARD_MQTT_PTR_FILE_NAME);
               LOGV(F("MQTT_PTR_UPDATE ---> MQTT_WAIT_STATE"));
            }
            // fail
            else {
               LOGE(F("SD Card writing ptr data on file %s... [ %s ]"), SDCARD_MQTT_PTR_FILE_NAME, FAIL_STRING);
               mqtt_state = MQTT_CLOSE_PTR_FILE;
               LOGV(F("MQTT_PTR_UPDATE ---> MQTT_CLOSE_PTR_FILE"));
            }
         }
         else {
            mqtt_state = MQTT_CLOSE_PTR_FILE;
            LOGV(F("MQTT_PTR_UPDATE ---> MQTT_CLOSE_PTR_FILE"));
         }
         break;

      case MQTT_CLOSE_PTR_FILE:
         mqtt_ptr_file.close();
         //mqtt_state = MQTT_CLOSE_SDCARD;
         mqtt_state = MQTT_END;
         LOGV(F("MQTT_CLOSE_PTR_FILE ---> MQTT_END"));
         break;

	 /*
      case MQTT_CLOSE_SDCARD:
         is_sdcard_error = false;
         is_sdcard_open = false;
         mqtt_state = MQTT_END;
         LOGV(F("MQTT_CLOSE_SDCARD ---> MQTT_END"));
         break;
	 */

      case MQTT_END:
         if (is_mqtt_published_data) {
            LOGN(F("[ %d ] data published through mqtt... [ %s ]"), mqtt_data_count, is_mqtt_error ? ERROR_STRING : OK_STRING);
            #if (USE_LCD)
	    lcd_error |= lcd.setCursor(7, 3);
            lcd_error |= lcd.print(F("MQ"))==0;
	    lcd_error |= lcd.print(mqtt_data_count)==0;
	    lcd_error |= lcd.print(is_mqtt_error ? "KO" : "OK")==0;
	    #endif
         }

         noInterrupts();
         is_event_mqtt_paused = false;
         is_event_mqtt = false;
         ready_tasks_count--;
         interrupts();

         mqtt_state = MQTT_INIT;
         LOGV(F("MQTT_END ---> MQTT_INIT"));
      break;

      case MQTT_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            mqtt_state = state_after_wait;
         }
      break;

      case MQTT_WAIT_STATE_RPC:
	if (!mqtt_client.isConnected() || !is_mqtt_rpc_delay || is_mqtt_error) {
	  LOGT(F("MQTT_WAIT_STATE_RPC: skipped"));
	  LOGV(F("mqtt_client.isConnected: %s"),mqtt_client.isConnected() ? "true" : "false");
	  LOGV(F("is_mqtt_rpc_delay: %s"),is_mqtt_rpc_delay ? "true" : "false");
	  LOGV(F("is_mqtt_error: %s"),is_mqtt_error ? "true" : "false");

	  is_mqtt_rpc_delay=false;
	  mqtt_state = state_after_wait;
	}
	mqtt_client.yield(100L);
	if (millis() - start_time_ms > delay_ms) {
	  is_mqtt_rpc_delay=false;
	  LOGT(F("MQTT_WAIT_STATE_RPC: expired"));
	  mqtt_state = state_after_wait;
	}
      break;

   }
}
#endif
