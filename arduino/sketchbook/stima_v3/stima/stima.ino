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

/*!
\def SERIAL_TRACE_LEVEL
\brief Serial debug level for this sketch.
*/
#define SERIAL_TRACE_LEVEL    (STIMA_SERIAL_TRACE_LEVEL)

/*!
\def LCD_TRACE_LEVEL
\brief LCD debug level for this sketch.
*/
#define LCD_TRACE_LEVEL       (STIMA_LCD_TRACE_LEVEL)

#include "stima.h"

/*!
\fn void setup()
\brief Arduino setup function. Init watchdog, hardware, debug, buffer and load configuration stored in EEPROM.
\return void.
*/
void setup() {
   init_wdt(WDT_TIMER);
   SERIAL_BEGIN(115200);
   init_pins();
   init_wire();
   init_rpc();
   init_tasks();
   LCD_BEGIN(&lcd, LCD_COLUMNS, LCD_ROWS);
   load_configuration();
   init_buffers();
   init_spi();
   #if (USE_RTC)
   init_rtc();
   #elif (USE_TIMER_1)
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
         init_sensors();
         wdt_reset();
         state = TASKS_EXECUTION;
      break;

      #if (USE_POWER_DOWN)
      case ENTER_POWER_DOWN:
         init_power_down(&awakened_event_occurred_time_ms, DEBOUNCING_POWER_DOWN_TIME_MS);
         state = TASKS_EXECUTION;
      break;
      #endif

      case TASKS_EXECUTION:
        // I2C Bus Check
        if (i2c_error >= I2C_MAX_ERROR_COUNT) {
          SERIAL_ERROR(F("Restart I2C BUS\r\n"));
          init_wire();
          // LCD_BEGIN(&lcd, LCD_COLUMNS, LCD_ROWS);
          wdt_reset();
        }

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
   is_mqtt_subscribed = false;
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

   last_lcd_begin = 0;

   is_time_for_sensors_reading_updated = false;
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

   #endif
}

void init_wire() {
   // uint8_t i2c_bus_state = I2C_ClearBus(); // clear the I2C bus first before calling Wire.begin()
   //
   // if (i2c_bus_state) {
   //    SERIAL_ERROR(F("I2C bus error: Could not clear!!!\r\n"));
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

   i2c_error = 0;
   Wire.end();
   Wire.begin();
   Wire.setClock(I2C_BUS_CLOCK);
}

void init_spi() {
   SPI.begin();
}

void init_rtc() {
   Pcf8563::disableAlarm();
   Pcf8563::disableTimer();
   Pcf8563::disableClockout();
   Pcf8563::setClockoutFrequency(RTC_FREQUENCY);
   Pcf8563::enableClockout();
   attachInterrupt(digitalPinToInterrupt(RTC_INTERRUPT_PIN), rtc_interrupt_handler, RISING);
}

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
  uint8_t sensors_count = 0;
  uint8_t sensors_error_count = 0;

  LCD_INFO(&lcd, false, true, F("--- www.rmap.cc ---"));
  LCD_INFO(&lcd, false, true, F("%s v. %u.%u"), stima_name, readable_configuration.module_main_version, readable_configuration.module_minor_version);

  LCD_INFO(&lcd, false, true, F("Sensors count %u"), readable_configuration.sensors_count);

  if (readable_configuration.sensors_count) {
    // read sensors configuration, create and setup
    for (uint8_t i=0; i<readable_configuration.sensors_count; i++) {
      SensorDriver::createAndSetup(readable_configuration.sensors[i].driver, readable_configuration.sensors[i].type, readable_configuration.sensors[i].address, readable_configuration.sensors[i].node, sensors, &sensors_count);
      SERIAL_INFO(F("--> %u: %s-%s [ 0x%x ]: %s\t [ %s ]\r\n"), sensors_count, readable_configuration.sensors[i].driver, readable_configuration.sensors[i].type, readable_configuration.sensors[i].address, readable_configuration.sensors[i].mqtt_topic, sensors[i]->isSetted() ? OK_STRING : FAIL_STRING);
      if (!sensors[i]->isSetted()) {
        sensors_error_count++;
        LCD_INFO(&lcd, false, true, F("%s %s"), readable_configuration.sensors[i].type, FAIL_STRING);
      }
    }

      SERIAL_INFO(F("\r\n"));
   }
}

void setNextTimeForSensorReading (time_t *next_time, uint16_t time_s) {
   time_t counter = (now() / time_s);
   *next_time = (time_t) ((++counter) * time_s);
}

#if (USE_MQTT)
bool mqttConnect(char *username, char *password) {
   MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
   data.MQTTVersion = 3;
   // data.will.topicName.cstring = maint_topic;
   // data.will.message.cstring = MQTT_ON_ERROR_MESSAGE;
   // data.will.retained = true;
   // data.will.qos = MQTT::QOS1;
   // data.willFlag = true;
   data.clientID.cstring = client_id;
   data.username.cstring = username;
   data.password.cstring = password;
   data.cleansession = false;

   // SERIAL_DEBUG(F("MQTT clientID: %s\r\n"), data.clientID.cstring);
   // SERIAL_DEBUG(F("MQTT will message: %s\r\n"), data.will.message.cstring);
   // SERIAL_DEBUG(F("MQTT will topic: %s\r\n"), data.will.topicName.cstring);

   return (mqtt_client.connect(data) == 0);
}

bool mqttPublish(const char *topic, const char *message, bool is_retained) {
   MQTT::Message tx_message;
   tx_message.qos = MQTT::QOS1;
   tx_message.retained = is_retained;
   tx_message.dup = false;
   tx_message.payload = (void*) message;
   tx_message.payloadlen = strlen(message);

   return (mqtt_client.publish(topic, tx_message) == 0);
}

void mqttRxCallback(MQTT::MessageData &md) {
   MQTT::Message &rx_message = md.message;
   // SERIAL_DEBUG(F("MQTT RX: %s\r\n"), (char*)rx_message.payload);
   // SERIAL_DEBUG(F("--> qos %u\r\n"), rx_message.qos);
   // SERIAL_DEBUG(F("--> retained %u\r\n"), rx_message.retained);
   // SERIAL_DEBUG(F("--> dup %u\r\n"), rx_message.dup);
   // SERIAL_DEBUG(F("--> id %u\r\n"), rx_message.id);
}
#endif

void print_configuration() {
   getStimaNameByType(stima_name, readable_configuration.module_type);
   SERIAL_INFO(F("--> type: %s\r\n"), stima_name);
   SERIAL_INFO(F("--> version: %d.%d\r\n"), readable_configuration.module_main_version, readable_configuration.module_minor_version);
   SERIAL_INFO(F("--> sensors: %d\r\n"), readable_configuration.sensors_count);

   #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
   SERIAL_INFO(F("--> dhcp: %s\r\n"), readable_configuration.is_dhcp_enable ? "on" : "off");
   SERIAL_INFO(F("--> ethernet mac: %02X:%02X:%02X:%02X:%02X:%02X\r\n"), readable_configuration.ethernet_mac[0], readable_configuration.ethernet_mac[1], readable_configuration.ethernet_mac[2], readable_configuration.ethernet_mac[3], readable_configuration.ethernet_mac[4], readable_configuration.ethernet_mac[5]);

   #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
   SERIAL_INFO(F("--> gsm apn: %s\r\n"), readable_configuration.gsm_apn);
   SERIAL_INFO(F("--> gsm username: %s\r\n"), readable_configuration.gsm_username);
   SERIAL_INFO(F("--> gsm password: %s\r\n"), readable_configuration.gsm_password);

   #endif

   #if (USE_NTP)
   SERIAL_INFO(F("--> ntp server: %s\r\n"), readable_configuration.ntp_server);
   #endif

   #if (USE_MQTT)
   SERIAL_INFO(F("--> mqtt server: %s\r\n"), readable_configuration.mqtt_server);
   SERIAL_INFO(F("--> mqtt port: %u\r\n"), readable_configuration.mqtt_port);
   SERIAL_INFO(F("--> mqtt root topic: %s\r\n"), readable_configuration.mqtt_root_topic);
   SERIAL_INFO(F("--> mqtt maint topic: %s\r\n"), readable_configuration.mqtt_maint_topic);
   SERIAL_INFO(F("--> mqtt subscribe topic: %s\r\n"), readable_configuration.mqtt_subscribe_topic);
   SERIAL_INFO(F("--> mqtt username: %s\r\n"), readable_configuration.mqtt_username);
   SERIAL_INFO(F("--> mqtt password: %s\r\n\r\n"), readable_configuration.mqtt_password);
   #endif
}

void set_default_configuration() {
   writable_configuration.module_type = MODULE_TYPE;
   writable_configuration.module_main_version = MODULE_MAIN_VERSION;
   writable_configuration.module_minor_version = MODULE_MINOR_VERSION;

   writable_configuration.report_seconds = 0;

   writable_configuration.sensors_count = 0;
   memset(writable_configuration.sensors, 0, sizeof(sensor_t) * USE_SENSORS_COUNT);

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
   strcpy(writable_configuration.mqtt_subscribe_topic, CONFIGURATION_DEFAULT_MQTT_SUBSCRIBE_TOPIC);
   strcpy(writable_configuration.mqtt_username, CONFIGURATION_DEFAULT_MQTT_USERNAME);
   strcpy(writable_configuration.mqtt_password, CONFIGURATION_DEFAULT_MQTT_PASSWORD);
   #endif

   SERIAL_INFO(F("Reset configuration to default value... [ %s ]\r\n"), OK_STRING);
}

void save_configuration(bool is_default) {
   if (is_default) {
      set_default_configuration();
      SERIAL_INFO(F("Save default configuration... [ %s ]\r\n"), OK_STRING);
   }
   else {
      SERIAL_INFO(F("Save configuration... [ %s ]\r\n"), OK_STRING);
   }

   ee_write(&writable_configuration, CONFIGURATION_EEPROM_ADDRESS, sizeof(configuration_t));
}

void load_configuration() {
   bool is_configuration_done = false;

   ee_read(&writable_configuration, CONFIGURATION_EEPROM_ADDRESS, sizeof(configuration_t));

   if (digitalRead(CONFIGURATION_RESET_PIN) == LOW) {
      SERIAL_INFO(F("Wait configuration...\r\n"));
      LCD_INFO(&lcd, false, true, F("Wait configuration"));
   }

   while (digitalRead(CONFIGURATION_RESET_PIN) == LOW && !is_configuration_done) {
      streamRpc.parseStream(&is_event_rpc, &Serial);
      wdt_reset();
   }

   if (is_configuration_done) {
      SERIAL_INFO(F("Configuration received... [ %s ]\r\n"), OK_STRING);
   }

   if (writable_configuration.module_type != MODULE_TYPE || writable_configuration.module_main_version != MODULE_MAIN_VERSION || writable_configuration.module_minor_version != MODULE_MINOR_VERSION) {
      save_configuration(CONFIGURATION_DEFAULT);
   }

   ee_read(&readable_configuration, CONFIGURATION_EEPROM_ADDRESS, sizeof(configuration_t));

   #if (USE_MQTT)
   getMqttClientIdFromMqttTopic(readable_configuration.mqtt_maint_topic, client_id);
   getFullTopic(maint_topic, readable_configuration.mqtt_maint_topic, MQTT_STATUS_TOPIC);
   #endif

   SERIAL_INFO(F("Load configuration... [ %s ]\r\n"), OK_STRING);
   print_configuration();
}

#if (USE_RPC_METHOD_PREPARE || USE_RPC_METHOD_PREPANDGET || USE_RPC_METHOD_GETJSON)
bool extractSensorsParams(JsonObject &params, char *driver, char *type, uint8_t *address, uint8_t *node) {
   bool is_error = false;

   for (JsonObject::iterator it = params.begin(); it != params.end(); ++it) {
      if (strcmp(it->key, "driver") == 0) {
         strncpy(driver, it->value.as<char*>(), DRIVER_LENGTH);
      }
      else if (strcmp(it->key, "type") == 0) {
         strncpy(type, it->value.as<char*>(), TYPE_LENGTH);
      }
      else if (strcmp(it->key, "address") == 0) {
         *address = it->value.as<unsigned char>();
      }
      else if (strcmp(it->key, "node") == 0) {
         *node = it->value.as<unsigned char>();
      }
      else {
         is_error = true;
      }
   }

   return is_error;
}
#endif

#if (USE_RPC_METHOD_CONFIGURE)
int configure(JsonObject &params, JsonObject &result) {
   bool is_error = false;
   bool is_sensor_config = false;

   for (JsonObject::iterator it = params.begin(); it != params.end(); ++it) {
      if (strcmp(it->key, "reset") == 0) {
         if (it->value.as<bool>() == true) {
            set_default_configuration();
            LCD_INFO(&lcd, false, true, F("Reset configuration"));
         }
      }
      else if (strcmp(it->key, "save") == 0) {
         if (it->value.as<bool>() == true) {
            save_configuration(CONFIGURATION_CURRENT);
            LCD_INFO(&lcd, false, true, F("Save configuration"));
         }
      }
      #if (USE_MQTT)
      else if (strcmp(it->key, "mqttserver") == 0) {
         strncpy(writable_configuration.mqtt_server, it->value.as<char*>(), MQTT_SERVER_LENGTH);
      }
      else if (strcmp(it->key, "mqttrootpath") == 0) {
         strncpy(writable_configuration.mqtt_root_topic, it->value.as<char*>(), MQTT_ROOT_TOPIC_LENGTH);
         strncpy(writable_configuration.mqtt_subscribe_topic, it->value.as<char*>(), MQTT_SUBSCRIBE_TOPIC_LENGTH);
         uint8_t mqtt_subscribe_topic_len = strlen(writable_configuration.mqtt_subscribe_topic);
         strncpy(writable_configuration.mqtt_subscribe_topic+mqtt_subscribe_topic_len, "rx", MQTT_SUBSCRIBE_TOPIC_LENGTH-mqtt_subscribe_topic_len);
      }
      else if (strcmp(it->key, "mqttmaintpath") == 0) {
         strncpy(writable_configuration.mqtt_maint_topic, it->value.as<char*>(), MQTT_MAINT_TOPIC_LENGTH);
      }
      else if (strcmp(it->key, "mqttsampletime") == 0) {
         writable_configuration.report_seconds = it->value.as<unsigned int>();
      }
      else if (strcmp(it->key, "mqttuser") == 0) {
         strncpy(writable_configuration.mqtt_username, it->value.as<char*>(), MQTT_USERNAME_LENGTH);
      }
      else if (strcmp(it->key, "mqttpassword") == 0) {
         strncpy(writable_configuration.mqtt_password, it->value.as<char*>(), MQTT_PASSWORD_LENGTH);
      }
      #endif
      #if (USE_NTP)
      else if (strcmp(it->key, "ntpserver") == 0) {
         strncpy(writable_configuration.ntp_server, it->value.as<char*>(), NTP_SERVER_LENGTH);
      }
      #endif
      else if (strcmp(it->key, "date") == 0) {
         #if (USE_RTC)
         Pcf8563::disable();
         Pcf8563::setDate(it->value.as<JsonArray>()[2].as<int>(), it->value.as<JsonArray>()[1].as<int>(), it->value.as<JsonArray>()[0].as<int>() - 2000, weekday()-1, 0);
         Pcf8563::setTime(it->value.as<JsonArray>()[3].as<int>(), it->value.as<JsonArray>()[4].as<int>(), it->value.as<JsonArray>()[5].as<int>());
         Pcf8563::enable();
         setSyncInterval(NTP_TIME_FOR_RESYNC_S);
         setSyncProvider(getSystemTime);
         #elif (USE_TIMER_1)
         setTime(it->value.as<JsonArray>()[3].as<int>(), it->value.as<JsonArray>()[4].as<int>(), it->value.as<JsonArray>()[5].as<int>(), it->value.as<JsonArray>()[2].as<int>(), it->value.as<JsonArray>()[1].as<int>(), it->value.as<JsonArray>()[0].as<int>() - 2000);
         #endif
      }
      #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH)
      else if (strcmp(it->key, "mac") == 0) {
         for (uint8_t i=0; i<ETHERNET_MAC_LENGTH; i++) {
            writable_configuration.ethernet_mac[i] = it->value.as<JsonArray>()[i];
         }
      }
      #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM)
      else if (strcmp(it->key, "gsmapn") == 0) {
         strncpy(writable_configuration.gsm_apn, it->value.as<char*>(), GSM_APN_LENGTH);
      }
      #endif
      else if (strcmp(it->key, "driver") == 0) {
         strncpy(writable_configuration.sensors[writable_configuration.sensors_count].driver, it->value.as<char*>(), DRIVER_LENGTH);
         is_sensor_config = true;
      }
      else if (strcmp(it->key, "type") == 0) {
         strncpy(writable_configuration.sensors[writable_configuration.sensors_count].type, it->value.as<char*>(), TYPE_LENGTH);
         is_sensor_config = true;
      }
      else if (strcmp(it->key, "address") == 0) {
         writable_configuration.sensors[writable_configuration.sensors_count].address = it->value.as<unsigned char>();
         is_sensor_config = true;
      }
      else if (strcmp(it->key, "node") == 0) {
         writable_configuration.sensors[writable_configuration.sensors_count].node = it->value.as<unsigned char>();
         is_sensor_config = true;
      }
      else if (strcmp(it->key, "mqttpath") == 0) {
         strncpy(writable_configuration.sensors[writable_configuration.sensors_count].mqtt_topic, it->value.as<char*>(), MQTT_SENSOR_TOPIC_LENGTH);
         is_sensor_config = true;
      }
      else {
         is_error = true;
      }
   }

   if (is_sensor_config) {
      writable_configuration.sensors_count++;
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
int prepare(JsonObject &params, JsonObject &result) {
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
int getjson(JsonObject &params, JsonObject &result) {
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
            snprintf(sensor_reading_time_buffer, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u", year(), month(), day(), hour(), minute(), second());

            result[F("state")] = F("done");
            result.createNestedObject(F("v"));

            for (JsonObject::iterator it = v.begin(); it != v.end(); ++it) {
               if (it->value.as<unsigned int>() == 0) {
                  result[F("v")][(char *) it->key] = (char *) NULL;
               }
               else {
                  result[F("v")][(char *) it->key] = it->value.as<unsigned int>();
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
int prepandget(JsonObject &params, JsonObject &result) {
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
            snprintf(sensor_reading_time_buffer, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u", year(), month(), day(), hour(), minute(), second());

            result[F("state")] = F("done");
            result.createNestedObject(F("v"));

            for (JsonObject::iterator it = v.begin(); it != v.end(); ++it) {
               if (it->value.as<unsigned int>() == 0) {
                  result[F("v")][(char *) it->key] = (char *) NULL;
               }
               else {
                  result[F("v")][(char *) it->key] = it->value.as<unsigned int>();
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

uint32_t getSystemTime() {
  return system_time;
}

void reboot() {
  init_wdt(WDTO_1S);
  while(true);
}

#if (USE_RPC_METHOD_REBOOT)
int reboot(JsonObject &params, JsonObject &result) {
   LCD_INFO(&lcd, false, true, F("Reboot"));
   result[F("state")] = "done";
   init_wdt(WDTO_1S);
   while(true);
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
            SERIAL_TRACE(F("SUPERVISOR_INIT ---> SUPERVISOR_END\r\n"));
         }
         else {
         #endif
            #if (USE_NTP)
            // need ntp sync ?
            if (!do_ntp_sync && ((now() - last_ntp_sync > NTP_TIME_FOR_RESYNC_S) || !is_time_set)) {
               do_ntp_sync = true;
               SERIAL_DEBUG(F("Requested NTP time sync...\r\n"));
            }

            start_time_ms = millis();
            #endif

            #if (MODULE_TYPE != STIMA_MODULE_TYPE_PASSIVE)
            supervisor_state = SUPERVISOR_CONNECTION_LEVEL_TASK;
            SERIAL_TRACE(F("SUPERVISOR_INIT ---> SUPERVISOR_CONNECTION_LEVEL_TASK\r\n"));
            #else
            supervisor_state = SUPERVISOR_TIME_LEVEL_TASK;
            SERIAL_TRACE(F("SUPERVISOR_INIT ---> SUPERVISOR_TIME_LEVEL_TASK\r\n"));
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
         SERIAL_TRACE(F("SUPERVISOR_CONNECTION_LEVEL_TASK ---> SUPERVISOR_WAIT_CONNECTION_LEVEL_TASK\r\n"));
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
               SERIAL_TRACE(F("SUPERVISOR_WAIT_CONNECTION_LEVEL_TASK ---> SUPERVISOR_TIME_LEVEL_TASK\r\n"));
            }
            // doing other operations...
            else {
               do_ntp_sync = false;
               supervisor_state = SUPERVISOR_MANAGE_LEVEL_TASK;
               SERIAL_TRACE(F("SUPERVISOR_WAIT_CONNECTION_LEVEL_TASK ---> SUPERVISOR_MANAGE_LEVEL_TASK\r\n"));
            }
         }

         // error
         if (!*is_event_client && is_event_client_executed && !is_client_connected) {
            // retry
            if ((++retry < SUPERVISOR_CONNECTION_RETRY_COUNT_MAX) || (millis() - start_time_ms < SUPERVISOR_CONNECTION_TIMEOUT_MS)) {
               is_event_client_executed = false;
               supervisor_state = SUPERVISOR_CONNECTION_LEVEL_TASK;
               SERIAL_TRACE(F("SUPERVISOR_WAIT_CONNECTION_LEVEL_TASK ---> SUPERVISOR_CONNECTION_LEVEL_TASK\r\n"));
            }
            // fail
            else {
               supervisor_state = SUPERVISOR_TIME_LEVEL_TASK;
               SERIAL_TRACE(F("SUPERVISOR_WAIT_CONNECTION_LEVEL_TASK ---> SUPERVISOR_TIME_LEVEL_TASK\r\n"));
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

            supervisor_state = SUPERVISOR_MANAGE_LEVEL_TASK;
            SERIAL_TRACE(F("SUPERVISOR_TIME_LEVEL_TASK ---> SUPERVISOR_MANAGE_LEVEL_TASK\r\n"));
         }
      break;

      case SUPERVISOR_MANAGE_LEVEL_TASK:
         if (is_time_updated) {
            SERIAL_INFO(F("Current date and time is: %02u/%02u/%04u %02u:%02u:%02u\r\n\r\n"), day(), month(), year(), hour(), minute(), second());
         }

         #if (MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM)
         if (is_supervisor_first_run && is_time_set) {
            if (readable_configuration.report_seconds) {
               setNextTimeForSensorReading((time_t *) &next_ptr_time_for_sensors_reading, readable_configuration.report_seconds);

               // testing sensors
               setNextTimeForSensorReading((time_t *) &next_ptr_time_for_testing_sensors, SENSORS_TESTING_DELAY_S);

               SERIAL_INFO(F("Acquisition scheduling...\r\n"));
               SERIAL_INFO(F("--> observations every %u minutes\r\n"), OBSERVATIONS_MINUTES);
            }

            if (readable_configuration.report_seconds >= 60) {
               uint8_t mm = readable_configuration.report_seconds / 60;
               uint8_t ss = readable_configuration.report_seconds - mm * 60;
               if (ss) {
                  SERIAL_INFO(F("--> report every %u minutes and %u seconds\r\n"), mm, ss);
               }
               else {
                  SERIAL_INFO(F("--> report every %u minutes\r\n"), mm);
               }
            }
            else if (readable_configuration.report_seconds) {
               SERIAL_INFO(F("--> report every %u seconds\r\n"), readable_configuration.report_seconds);
            }

            if (next_ptr_time_for_sensors_reading) {
               SERIAL_INFO(F("--> starting at: %02u:%02u:%02u\r\n"), hour(next_ptr_time_for_sensors_reading), minute(next_ptr_time_for_sensors_reading), second(next_ptr_time_for_sensors_reading));
               LCD_INFO(&lcd, false, true, F("start acq %02u:%02u:%02u"), hour(next_ptr_time_for_sensors_reading), minute(next_ptr_time_for_sensors_reading), second(next_ptr_time_for_sensors_reading));
            }

            if (next_ptr_time_for_testing_sensors) {
               SERIAL_INFO(F("--> testing at: %02u:%02u:%02u\r\n\r\n"), hour(next_ptr_time_for_testing_sensors), minute(next_ptr_time_for_testing_sensors), second(next_ptr_time_for_testing_sensors));
            }
         }
         #endif

         #if (LCD_TRACE_LEVEL > LCD_TRACE_LEVEL_OFF)
         // reinit lcd display
         if (last_lcd_begin == 0) {
            last_lcd_begin = now();
         }
         else if ((now() - last_lcd_begin > LCD_TIME_FOR_REINITIALIZE_S)) {
            last_lcd_begin = now();
            LCD_BEGIN(&lcd, LCD_COLUMNS, LCD_ROWS);
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
          if (sdcard_init(&SD, SDCARD_CHIP_SELECT_PIN)) {
            if (sdcard_open_file(&SD, &test_file, SDCARD_TEST_FILE_NAME, O_RDWR | O_CREAT | O_APPEND)) {
              is_sdcard_ok = test_file.close();
            }
          }

          if (!is_sdcard_ok) {
            SERIAL_ERROR(F("SD Card... [ %s ]\r\n--> is card inserted?\r\n--> there is a valid FAT32 filesystem?\r\n\r\n"), FAIL_STRING);
            LCD_INFO(&lcd, false, true, F("SD Card %s"), FAIL_STRING);
          }
        }
        #endif

        #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_INFO)
        delay_ms = DEBUG_WAIT_FOR_SLEEP_MS;
        start_time_ms = millis();
        state_after_wait = SUPERVISOR_END;
        supervisor_state = SUPERVISOR_WAIT_STATE;
        #else
        supervisor_state = SUPERVISOR_END;
        #endif
        SERIAL_TRACE(F("SUPERVISOR_MANAGE_LEVEL_TASK ---> SUPERVISOR_END\r\n"));
      break;

      case SUPERVISOR_END:
         is_supervisor_first_run = false;
         noInterrupts();
         is_event_supervisor = false;
         ready_tasks_count--;
         interrupts();

         supervisor_state = SUPERVISOR_INIT;
         SERIAL_TRACE(F("SUPERVISOR_END ---> SUPERVISOR_INIT\r\n"));
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
      SERIAL_INFO(F("Next acquisition scheduled at: %02u:%02u:%02u\r\n"), hour(next_ptr_time_for_sensors_reading), minute(next_ptr_time_for_sensors_reading), second(next_ptr_time_for_sensors_reading));
      LCD_INFO(&lcd, true, true, F("next acq %02u:%02u:%02u"), hour(next_ptr_time_for_sensors_reading), minute(next_ptr_time_for_sensors_reading), second(next_ptr_time_for_sensors_reading));
    }
    #endif
  }
}

void time_task() {
   static uint8_t retry;
   static time_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;

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
         retry = 0;
         state_after_wait = TIME_INIT;

         #if (USE_NTP)
         if (is_client_connected) {
            time_state = TIME_SEND_ONLINE_REQUEST;
            SERIAL_TRACE(F("TIME_INIT --> TIME_SEND_ONLINE_REQUEST\r\n"));
         }
         else {
            time_state = TIME_SET_SYNC_RTC_PROVIDER;
            SERIAL_TRACE(F("TIME_INIT --> TIME_SET_SYNC_RTC_PROVIDER\r\n"));
         }
         #else
         #if (USE_RTC)
         time_state = TIME_SET_SYNC_RTC_PROVIDER;
         SERIAL_TRACE(F("TIME_INIT --> TIME_SET_SYNC_RTC_PROVIDER\r\n"));
         #elif (USE_TIMER_1)
         time_state = TIME_END;
         SERIAL_TRACE(F("TIME_INIT --> TIME_END\r\n"));
         #endif
         #endif
      break;

      case TIME_SEND_ONLINE_REQUEST:
         #if (USE_NTP)
         #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
         is_ntp_request_ok = Ntp::sendRequest(&eth_udp_client, readable_configuration.ntp_server);

         #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
         is_ntp_request_ok = Ntp::sendRequest(&s800);
         #endif

         // success
         if (is_ntp_request_ok) {
            retry = 0;
            time_state = TIME_WAIT_ONLINE_RESPONSE;
            SERIAL_TRACE(F("TIME_SEND_ONLINE_REQUEST --> TIME_WAIT_ONLINE_RESPONSE\r\n"));
         }
         // retry
         else if (++retry < NTP_RETRY_COUNT_MAX) {
            delay_ms = NTP_RETRY_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = TIME_SEND_ONLINE_REQUEST;
            time_state = TIME_WAIT_STATE;
            SERIAL_TRACE(F("TIME_SEND_ONLINE_REQUEST --> TIME_WAIT_STATE\r\n"));
         }
         // fail: use old rtc time
         else {
            SERIAL_ERROR(F("NTP request... [ %s ]\r\n"), FAIL_STRING);
            #if (USE_RTC)
            time_state = TIME_SET_SYNC_RTC_PROVIDER;
            SERIAL_TRACE(F("TIME_SEND_ONLINE_REQUEST --> TIME_SET_SYNC_RTC_PROVIDER\r\n"));
            #elif (USE_TIMER_1)
            time_state = TIME_END;
            SERIAL_TRACE(F("TIME_SEND_ONLINE_REQUEST --> TIME_END\r\n"));
            #endif
         }
         #endif
      break;

      case TIME_WAIT_ONLINE_RESPONSE:
         #if (USE_NTP)
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
            retry = 0;
            system_time = current_ntp_time;
            setTime(system_time);
            last_ntp_sync = current_ntp_time;
            SERIAL_DEBUG(F("Current NTP date and time: %02u/%02u/%04u %02u:%02u:%02u\r\n"), day(), month(), year(), hour(), minute(), second());
            #if (USE_RTC)
            time_state = TIME_SET_SYNC_NTP_PROVIDER;
            SERIAL_TRACE(F("TIME_WAIT_ONLINE_RESPONSE --> TIME_WAIT_STATE\r\n"));
            #elif (USE_TIMER_1)
            time_state = TIME_SET_SYNC_NTP_PROVIDER;
            SERIAL_TRACE(F("TIME_WAIT_ONLINE_RESPONSE --> TIME_WAIT_STATE\r\n"));
            #endif
         }
         // retry
         else if (++retry < NTP_RETRY_COUNT_MAX) {
            delay_ms = NTP_RETRY_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = TIME_WAIT_ONLINE_RESPONSE;
            time_state = TIME_WAIT_STATE;
            SERIAL_TRACE(F("TIME_WAIT_ONLINE_RESPONSE --> TIME_WAIT_STATE\r\n"));
         }
         // fail
         else {
            retry = 0;
            #if (USE_RTC)
            time_state = TIME_SET_SYNC_RTC_PROVIDER;
            SERIAL_TRACE(F("TIME_WAIT_ONLINE_RESPONSE --> TIME_SET_SYNC_RTC_PROVIDER\r\n"));
            #elif (USE_TIMER_1)
            time_state = TIME_END;
            SERIAL_TRACE(F("TIME_WAIT_ONLINE_RESPONSE --> TIME_END\r\n"));
            #endif
         }
         #endif
      break;

      case TIME_SET_SYNC_NTP_PROVIDER:
         #if (USE_NTP)
         is_set_rtc_ok &= Pcf8563::disable();
         is_set_rtc_ok &= Pcf8563::setDate(day(), month(), year()-2000, weekday()-1, 0);
         is_set_rtc_ok &= Pcf8563::setTime(hour(), minute(), second());
         is_set_rtc_ok &= Pcf8563::enable();

         if (!is_set_rtc_ok) {
           i2c_error++;
         }

         if (is_set_rtc_ok) {
            retry = 0;
            time_state = TIME_SET_SYNC_RTC_PROVIDER;
            SERIAL_TRACE(F("TIME_SET_SYNC_NTP_PROVIDER --> TIME_SET_SYNC_RTC_PROVIDER\r\n"));
         }
         // retry
         else if (++retry < NTP_RETRY_COUNT_MAX) {
            is_set_rtc_ok = true;
            delay_ms = NTP_RETRY_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = TIME_SET_SYNC_NTP_PROVIDER;
            time_state = TIME_WAIT_STATE;
            SERIAL_TRACE(F("TIME_SET_SYNC_NTP_PROVIDER --> TIME_SET_SYNC_NTP_PROVIDER\r\n"));
         }
         // fail
         else {
           retry = 0;
           time_state = TIME_SET_SYNC_RTC_PROVIDER;
           SERIAL_TRACE(F("TIME_SET_SYNC_NTP_PROVIDER --> TIME_SET_SYNC_RTC_PROVIDER\r\n"));
         }
         #endif
      break;

      case TIME_SET_SYNC_RTC_PROVIDER:
         setSyncInterval(NTP_TIME_FOR_RESYNC_S);
         setSyncProvider(getSystemTime);
         SERIAL_DEBUG(F("Current System date and time: %02u/%02u/%04u %02u:%02u:%02u\r\n"), day(), month(), year(), hour(), minute(), second());
         time_state = TIME_END;
         SERIAL_TRACE(F("TIME_SET_SYNC_RTC_PROVIDER --> TIME_END\r\n"));
      break;

      case TIME_END:
         is_time_set = true;
         is_event_time_executed = true;
         noInterrupts();
         is_event_time = false;
         ready_tasks_count--;
         interrupts();
         time_state = TIME_INIT;
         SERIAL_TRACE(F("TIME_END --> TIME_INIT\r\n"));
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
         SERIAL_TRACE(F("ETHERNET_INIT --> ETHERNET_CONNECT\r\n"));
      break;

      case ETHERNET_CONNECT:
         if (readable_configuration.is_dhcp_enable) {
            if (Ethernet.begin(readable_configuration.ethernet_mac)) {
               is_client_connected = true;
               SERIAL_INFO(F("Ethernet: DHCP [ %s ]\r\n"), OK_STRING);
            }
         }
         else {
            Ethernet.begin(readable_configuration.ethernet_mac, IPAddress(readable_configuration.ip), IPAddress(readable_configuration.primary_dns), IPAddress(readable_configuration.gateway), IPAddress(readable_configuration.netmask));
            is_client_connected = true;
            SERIAL_INFO(F("Ethernet: Static [ %s ]\r\n"), OK_STRING);
         }

         // success
         if (is_client_connected) {
            w5500.setRetransmissionTime(ETHERNET_RETRY_TIME_MS);
            w5500.setRetransmissionCount(ETHERNET_RETRY_COUNT);

            SERIAL_INFO(F("--> ip: %u.%u.%u.%u\r\n"), Ethernet.localIP()[0], Ethernet.localIP()[1], Ethernet.localIP()[2], Ethernet.localIP()[3]);
            SERIAL_INFO(F("--> netmask: %u.%u.%u.%u\r\n"), Ethernet.subnetMask()[0], Ethernet.subnetMask()[1], Ethernet.subnetMask()[2], Ethernet.subnetMask()[3]);
            SERIAL_INFO(F("--> gateway: %u.%u.%u.%u\r\n"), Ethernet.gatewayIP()[0], Ethernet.gatewayIP()[1], Ethernet.gatewayIP()[2], Ethernet.gatewayIP()[3]);
            SERIAL_INFO(F("--> primary dns: %u.%u.%u.%u\r\n"), Ethernet.dnsServerIP()[0], Ethernet.dnsServerIP()[1], Ethernet.dnsServerIP()[2], Ethernet.dnsServerIP()[3]);

            LCD_INFO(&lcd, false, true, F("ip: %u.%u.%u.%u"), Ethernet.localIP()[0], Ethernet.localIP()[1], Ethernet.localIP()[2], Ethernet.localIP()[3]);

            ethernet_state = ETHERNET_OPEN_UDP_SOCKET;
            SERIAL_TRACE(F("ETHERNET_CONNECT --> ETHERNET_OPEN_UDP_SOCKET\r\n"));
         }
         // retry
         else if ((++retry) < ETHERNET_RETRY_COUNT_MAX) {
            delay_ms = ETHERNET_RETRY_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = ETHERNET_CONNECT;
            ethernet_state = ETHERNET_WAIT_STATE;
            SERIAL_TRACE(F("ETHERNET_CONNECT --> ETHERNET_WAIT_STATE\r\n"));
         }
         // fail
         else {
            retry = 0;
            ethernet_state = ETHERNET_END;
            SERIAL_TRACE(F("ETHERNET_CONNECT --> ETHERNET_END\r\n"));
            SERIAL_ERROR(F("Ethernet %s: [ %s ]\r\n"), ERROR_STRING, readable_configuration.is_dhcp_enable ? "DHCP" : "Static");
            LCD_INFO(&lcd, false, true, F("ethernet %s"), ERROR_STRING);
         }
      break;

      case ETHERNET_OPEN_UDP_SOCKET:
         // success
         if (eth_udp_client.begin(ETHERNET_DEFAULT_LOCAL_UDP_PORT)) {
            SERIAL_TRACE(F("--> udp socket local port: %u [ OK ]\r\n"), ETHERNET_DEFAULT_LOCAL_UDP_PORT);
            is_client_udp_socket_open = true;
            ethernet_state = ETHERNET_END;
            SERIAL_TRACE(F("ETHERNET_OPEN_UDP_SOCKET --> ETHERNET_END\r\n"));
         }
         // retry
         else if ((++retry) < ETHERNET_RETRY_COUNT_MAX) {
            delay_ms = ETHERNET_RETRY_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = ETHERNET_OPEN_UDP_SOCKET;
            ethernet_state = ETHERNET_WAIT_STATE;
            SERIAL_TRACE(F("ETHERNET_OPEN_UDP_SOCKET --> ETHERNET_WAIT_STATE\r\n"));
         }
         // fail
         else {
            SERIAL_ERROR(F("--> udp socket on local port: %u [ FAIL ]\r\n"), ETHERNET_DEFAULT_LOCAL_UDP_PORT);
            retry = 0;
            ethernet_state = ETHERNET_INIT;
            SERIAL_TRACE(F("ETHERNET_OPEN_UDP_SOCKET --> ETHERNET_INIT\r\n"));
         }
      break;

      case ETHERNET_END:
         SERIAL_INFO(F("\r\n"));
         is_event_client_executed = true;
         noInterrupts();
         is_event_ethernet = false;
         ready_tasks_count--;
         interrupts();
         ethernet_state = ETHERNET_INIT;
         SERIAL_TRACE(F("ETHERNET_END --> ETHERNET_INIT\r\n"));
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
      break;

      case GSM_SWITCH_ON:
         sim800_status = s800.switchOn();

         // success
         if (sim800_status == SIM800_OK) {
            gsm_state = GSM_AUTOBAUD;
         }
         else if (sim800_status == SIM800_ERROR) {
            gsm_state = GSM_END;
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

         }
         // fail
         else if (sim800_status == SIM800_ERROR) {
            gsm_state = GSM_WAIT_FOR_SWITCH_OFF;
         }
         // wait...
      break;

      case GSM_SETUP:
         sim800_status = s800.setup();

         // success
         if (sim800_status == SIM800_OK) {
            gsm_state = GSM_START_CONNECTION;
         }
         // fail
         else if (sim800_status == SIM800_ERROR) {
            is_error = true;
            gsm_state = GSM_WAIT_FOR_SWITCH_OFF;
         }
         // wait...
      break;

      case GSM_START_CONNECTION:
         sim800_status = s800.startConnection(readable_configuration.gsm_apn, readable_configuration.gsm_username, readable_configuration.gsm_password);

         // success
         if (sim800_status == SIM800_OK) {
            gsm_state = GSM_CHECK_OPERATION;
         }
         // fail
         else if (sim800_status == SIM800_ERROR) {
            is_error = true;
            gsm_state = GSM_WAIT_FOR_SWITCH_OFF;
         }
         // wait...
      break;

      case GSM_CHECK_OPERATION:
         // open udp socket for query NTP
         if (do_ntp_sync) {
            gsm_state = GSM_OPEN_UDP_SOCKET;
         }
         // wait for mqtt send terminate
         else {
            gsm_state = GSM_SUSPEND;
            state_after_wait = GSM_STOP_CONNECTION;
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
         }
         // fail
         else if (sim800_connection_status == 2) {
            is_client_connected = false;
            is_event_client_executed = true;
            is_error = true;
            gsm_state = GSM_WAIT_FOR_SWITCH_OFF;
         }
         // wait
      break;

      case GSM_SUSPEND:
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
         }
         // fail
         else if (sim800_status == SIM800_ERROR) {
            is_error = true;
            gsm_state = GSM_SWITCH_OFF;
         }
         // wait
      break;

      case GSM_WAIT_FOR_SWITCH_OFF:
         delay_ms = SIM800_POWER_ON_TO_OFF_DELAY_MS;
         start_time_ms = millis();
         state_after_wait = GSM_SWITCH_OFF;
         gsm_state = GSM_WAIT_STATE;
      break;

      case GSM_SWITCH_OFF:
         sim800_status = s800.switchOff(power_off_mode);

         // success
         if (sim800_status == SIM800_OK) {
            delay_ms = SIM800_WAIT_FOR_POWER_OFF_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = GSM_END;
            gsm_state = GSM_WAIT_STATE;
         }
         // fail
         else if (sim800_status == SIM800_ERROR) {
            if (power_off_mode == SIM800_POWER_OFF_BY_AT_COMMAND) {
               power_off_mode = SIM800_POWER_OFF_BY_SWITCH;
            }
            else {
               gsm_state = GSM_END;
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
   #if (LCD_TRACE_LEVEL >= LCD_TRACE_LEVEL_INFO)
   static char lcd_buffer[LCD_ROWS][LCD_COLUMNS];
   static int lcd_count[LCD_ROWS];
   #endif

   switch (sensors_reading_state) {
      case SENSORS_READING_INIT:
         i = 0;
         is_sensor_found = false;

         if (driver && type && address && node) {
            while (!is_sensor_found && (i < readable_configuration.sensors_count)) {
               is_sensor_found = strcmp(sensors[i]->getDriver(), driver) == 0 && strcmp(sensors[i]->getType(), type) == 0 && sensors[i]->getAddress() == address && sensors[i]->getNode() == node;
               if (!is_sensor_found) {
                  i++;
               }
            }

            if (is_sensor_found) {
               *sensor_index = i;
            }
         }

         if (do_prepare) {
            SERIAL_INFO(F("Sensors reading...\r\n"));
            retry = 0;

            if (driver && type && address && node && is_sensor_found) {
               sensors[i]->resetPrepared();
            }
            else {
               for (i=0; i<readable_configuration.sensors_count; i++) {
                  sensors[i]->resetPrepared();
               }
               i = 0;
            }

            state_after_wait = SENSORS_READING_INIT;
            sensors_reading_state = SENSORS_READING_PREPARE;
            SERIAL_TRACE(F("SENSORS_READING_INIT ---> SENSORS_READING_PREPARE\r\n"));
         }
         else if (do_get) {
            sensors_reading_state = SENSORS_READING_GET;
            SERIAL_TRACE(F("SENSORS_READING_INIT ---> SENSORS_READING_GET\r\n"));
         }
         else {
            sensors_reading_state = SENSORS_READING_END;
            SERIAL_TRACE(F("SENSORS_READING_INIT ---> SENSORS_READING_END\r\n"));
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
            SERIAL_TRACE(F("SENSORS_READING_PREPARE ---> SENSORS_READING_WAIT_STATE\r\n"));
         }
         else {
            sensors_reading_state = SENSORS_READING_IS_PREPARED;
            SERIAL_TRACE(F("SENSORS_READING_PREPARE ---> SENSORS_READING_IS_PREPARED\r\n"));
         }
      break;

      case SENSORS_READING_IS_PREPARED:
         // success
         if (sensors[i]->isPrepared()) {
            retry = 0;

            if (do_get) {
               sensors_reading_state = SENSORS_READING_GET;
               SERIAL_TRACE(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_GET\r\n"));
            }
            else {
               sensors_reading_state = SENSORS_READING_END;
               SERIAL_TRACE(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_END\r\n"));
            }
         }
         // retry
         else if ((++retry) < SENSORS_RETRY_COUNT_MAX) {
            i2c_error++;
            delay_ms = SENSORS_RETRY_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SENSORS_READING_PREPARE;
            sensors_reading_state = SENSORS_READING_WAIT_STATE;
            SERIAL_TRACE(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_WAIT_STATE\r\n"));
         }
         // fail
         else {
            if (do_get) {
               sensors_reading_state = SENSORS_READING_GET;
               SERIAL_TRACE(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_GET\r\n"));
            }
            else {
               sensors_reading_state = SENSORS_READING_END;
               SERIAL_TRACE(F("SENSORS_READING_IS_PREPARED ---> SENSORS_READING_END\r\n"));
            }
            retry = 0;
         }
      break;

      case SENSORS_READING_GET:
        if (is_test) {
          sensors[i]->get(&values_readed_from_sensor[i][0], VALUES_TO_READ_FROM_SENSOR_COUNT);
        }
        else {
          sensors[i]->getJson(&values_readed_from_sensor[i][0], VALUES_TO_READ_FROM_SENSOR_COUNT, &json_sensors_data[i][0]);
        }

        delay_ms = sensors[i]->getDelay();
        start_time_ms = sensors[i]->getStartTime();

        if (delay_ms) {
          state_after_wait = SENSORS_READING_IS_GETTED;
          sensors_reading_state = SENSORS_READING_WAIT_STATE;
          SERIAL_TRACE(F("SENSORS_READING_GET ---> SENSORS_READING_WAIT_STATE\r\n"));
        }
        else {
          sensors_reading_state = SENSORS_READING_IS_GETTED;
          SERIAL_TRACE(F("SENSORS_READING_GET ---> SENSORS_READING_IS_GETTED\r\n"));
        }
      break;

      case SENSORS_READING_IS_GETTED:
         // end
         if (sensors[i]->isEnd() && !sensors[i]->isReaded()) {
            // success
            if (sensors[i]->isSuccess()) {
               retry = 0;
               sensors_reading_state = SENSORS_READING_READ;
               SERIAL_TRACE(F("SENSORS_READING_IS_GETTED ---> SENSORS_READING_READ\r\n"));
            }
            // retry
            else if ((++retry) < SENSORS_RETRY_COUNT_MAX) {
               i2c_error++;
               delay_ms = SENSORS_RETRY_DELAY_MS;
               start_time_ms = millis();
               state_after_wait = SENSORS_READING_GET;
               sensors_reading_state = SENSORS_READING_WAIT_STATE;
               SERIAL_TRACE(F("SENSORS_READING_IS_GETTED ---> SENSORS_READING_WAIT_STATE\r\n"));
            }
            // fail
            else {
               retry = 0;
               sensors_reading_state = SENSORS_READING_READ;
               SERIAL_TRACE(F("SENSORS_READING_IS_GETTED ---> SENSORS_READING_READ\r\n"));
            }
         }
         // not end
         else {
            sensors_reading_state = SENSORS_READING_GET;
            SERIAL_TRACE(F("SENSORS_READING_IS_GETTED ---> SENSORS_READING_GET\r\n"));
         }
      break;

      case SENSORS_READING_READ:
         if (driver && type && address && node) {
            sensors_reading_state = SENSORS_READING_END;
            SERIAL_TRACE(F("SENSORS_READING_READ ---> SENSORS_READING_END\r\n"));
         }
         else {
            sensors_reading_state = SENSORS_READING_NEXT;
            SERIAL_TRACE(F("SENSORS_READING_READ ---> SENSORS_READING_NEXT\r\n"));
         }
      break;

      case SENSORS_READING_NEXT:
        // next sensor
        if ((++i) < readable_configuration.sensors_count) {
          retry = 0;
          sensors_reading_state = SENSORS_READING_PREPARE;
          SERIAL_TRACE(F("SENSORS_READING_NEXT ---> SENSORS_READING_PREPARE\r\n"));
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

          // normal OR test: print
          if (!is_first_run || is_test) {
            #if (LCD_TRACE_LEVEL >= LCD_TRACE_LEVEL_INFO)
            for (i = 0; i < LCD_ROWS; i++) {
              lcd_count[i] = 0;
            }

            for (i = 0; i < readable_configuration.sensors_count; i++) {
              SERIAL_DEBUG(F("JSON <-- %s\r\n"), &json_sensors_data[i][0]);

              // SERIAL_DEBUG(F("Valori: %ld %ld %ld\t%s\r\n"), values_readed_from_sensor[i][0], values_readed_from_sensor[i][1], values_readed_from_sensor[i][2], json_sensors_data[i]);

              if ((strcmp(sensors[i]->getType(), "ITH") == 0) || (strcmp(sensors[i]->getType(), "HYT") == 0) || (strcmp(sensors[i]->getType(), "OE3") == 0)) {
                if (isValid(values_readed_from_sensor[i][0])) {
                  lcd_count[0] += snprintf(&lcd_buffer[0][0], LCD_COLUMNS, "%.1fC ", ((values_readed_from_sensor[i][0] - SENSOR_DRIVER_C_TO_K) / 100.0));
                }
                else {
                  lcd_count[0] += snprintf(&lcd_buffer[0][0], LCD_COLUMNS, "--.-C ");
                }

                if (isValid(values_readed_from_sensor[i][1])) {
                  lcd_count[0] += snprintf(&lcd_buffer[0][0]+lcd_count[0], LCD_COLUMNS-lcd_count[0], "%ld%% ", values_readed_from_sensor[i][1]);
                }
                else {
                  lcd_count[0] += snprintf(&lcd_buffer[0][0]+lcd_count[0], LCD_COLUMNS-lcd_count[0], "---%% ");
                }
              }
              else if (strcmp(sensors[i]->getType(), "OA3") == 0) {
                if (isValid(values_readed_from_sensor[i][0])) {
                  lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "%.0f %.0f %.0f ug/m3", values_readed_from_sensor[i][0]/10.0, values_readed_from_sensor[i][1]/10.0, values_readed_from_sensor[i][2]/10.0);
                }
                else {
                  lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "--- --- --- ug/m3");
                }
              }
              else if (strcmp(sensors[i]->getType(), "TBR") == 0) {
                if (isValid(values_readed_from_sensor[i][0])) {
                  lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "%.1fmm ", (values_readed_from_sensor[i][0]/10.0));
                }
                else {
                  lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "--.-mm ");
                }
              }
              else if (strcmp(sensors[i]->getType(), "LWT") == 0) {
                if (isValid(values_readed_from_sensor[i][0])) {
                  lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "%.0f'", (values_readed_from_sensor[i][0]*10.0/60.0));
                }
                else {
                  lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "--'");
                }
              }
              else if (strcmp(sensors[i]->getType(), "DW1") == 0) {
                if (isValid(values_readed_from_sensor[i][1])) {
                  lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "%.1fm/s ", (values_readed_from_sensor[i][1]/10.0));
                }
                else {
                  lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "--.-m/s ");
                }

                if (isValid(values_readed_from_sensor[i][0])) {
                  lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "%ld%c", values_readed_from_sensor[i][0], 0b11011111);
                }
                else {
                  lcd_count[1] += snprintf(&lcd_buffer[1][0]+lcd_count[1], LCD_COLUMNS-lcd_count[1], "---%c", 0b11011111);
                }
              }
              else if (strcmp(sensors[i]->getType(), "DEP") == 0) {
                if (isValid(values_readed_from_sensor[i][0])) {
                  lcd_count[0] += snprintf(&lcd_buffer[0][0]+lcd_count[0], LCD_COLUMNS-lcd_count[0], "%.1fV", (values_readed_from_sensor[i][1]/10.0));
                }
                else {
                  lcd_count[0] += snprintf(&lcd_buffer[0][0]+lcd_count[0], LCD_COLUMNS-lcd_count[0], "--.-V");
                }
              }
            }
            for (i = 0; i < 2; i++) {
              LCD_INFO(&lcd, (i == 0), true, F("%s"), lcd_buffer[i]);
            }
            #endif
          }

          sensors_reading_state = SENSORS_READING_END;
          SERIAL_TRACE(F("SENSORS_READING_NEXT ---> SENSORS_READING_END\r\n"));
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

        sensors_reading_state = SENSORS_READING_INIT;
        SERIAL_TRACE(F("SENSORS_READING_END ---> SENSORS_READING_INIT\r\n"));
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
            SERIAL_TRACE(F("DATA_SAVING_INIT ---> DATA_SAVING_OPEN_FILE\r\n"));
         }
         else {
            data_saving_state = DATA_SAVING_OPEN_SDCARD;
            SERIAL_TRACE(F("DATA_SAVING_INIT ---> DATA_SAVING_OPEN_SDCARD\r\n"));
         }
      break;

      case DATA_SAVING_OPEN_SDCARD:
         if (sdcard_init(&SD, SDCARD_CHIP_SELECT_PIN)) {
            retry = 0;
            is_sdcard_open = true;
            data_saving_state = DATA_SAVING_OPEN_FILE;
            SERIAL_TRACE(F("DATA_SAVING_OPEN_SDCARD ---> DATA_SAVING_OPEN_FILE\r\n"));
         }
         // retry
         else if ((++retry) < DATA_SAVING_RETRY_COUNT_MAX) {
            delay_ms = DATA_SAVING_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = DATA_SAVING_OPEN_SDCARD;
            data_saving_state = DATA_SAVING_WAIT_STATE;
            SERIAL_TRACE(F("DATA_SAVING_OPEN_SDCARD ---> DATA_SAVING_WAIT_STATE\r\n"));
         }
         // fail
         else {
            is_sdcard_error = true;
            is_sdcard_open = false;
            SERIAL_ERROR(F("SD Card... [ FAIL ]\r\n--> is card inserted?\r\n--> there is a valid FAT32 filesystem?\r\n\r\n"));

            data_saving_state = DATA_SAVING_END;
            SERIAL_TRACE(F("DATA_SAVING_OPEN_SDCARD ---> DATA_SAVING_END\r\n"));
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
            SERIAL_TRACE(F("DATA_SAVING_OPEN_FILE ---> DATA_SAVING_SENSORS_LOOP\r\n"));
         }
         // retry
         else if ((++retry) < DATA_SAVING_RETRY_COUNT_MAX) {
            delay_ms = DATA_SAVING_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = DATA_SAVING_OPEN_FILE;
            data_saving_state = DATA_SAVING_WAIT_STATE;
            SERIAL_TRACE(F("DATA_SAVING_OPEN_SDCARD ---> DATA_SAVING_WAIT_STATE\r\n"));
         }
         // fail
         else {
            SERIAL_ERROR(F("SD Card open file %s... [ FAIL ]\r\n"), file_name);
            is_sdcard_error = true;
            data_saving_state = DATA_SAVING_END;
            SERIAL_TRACE(F("DATA_SAVING_OPEN_FILE ---> DATA_SAVING_END\r\n"));
         }
      break;

      case DATA_SAVING_SENSORS_LOOP:
         if (i < readable_configuration.sensors_count) {
            k = 0;
            data_count = jsonToMqtt(&json_sensors_data[i][0], readable_configuration.sensors[i].mqtt_topic, topic_buffer, message_buffer, (tmElements_t *) &sensor_reading_time);
            data_saving_state = DATA_SAVING_DATA_LOOP;
            SERIAL_TRACE(F("DATA_SAVING_SENSORS_LOOP ---> DATA_SAVING_DATA_LOOP\r\n"));
         }
         else {
            SERIAL_DEBUG(F("\r\n"));
            data_saving_state = DATA_SAVING_CLOSE_FILE;
            SERIAL_TRACE(F("DATA_SAVING_SENSORS_LOOP ---> DATA_SAVING_CLOSE_FILE\r\n"));
         }
      break;

      case DATA_SAVING_DATA_LOOP:
         if (k < data_count) {
            mqttToSd(&topic_buffer[k][0], &message_buffer[k][0], sd_buffer);
            data_saving_state = DATA_SAVING_WRITE_FILE;
            SERIAL_TRACE(F("DATA_SAVING_DATA_LOOP ---> DATA_SAVING_WRITE_FILE\r\n"));
         }
         else {
            i++;
            data_saving_state = DATA_SAVING_SENSORS_LOOP;
            SERIAL_TRACE(F("DATA_SAVING_DATA_LOOP ---> DATA_SAVING_SENSORS_LOOP\r\n"));
         }
      break;

      case DATA_SAVING_WRITE_FILE:
         // sdcard success
         if (write_data_file.write(sd_buffer, MQTT_SENSOR_TOPIC_LENGTH + MQTT_MESSAGE_LENGTH) == (MQTT_SENSOR_TOPIC_LENGTH + MQTT_MESSAGE_LENGTH)) {
            SERIAL_DEBUG(F("SD <-- %s %s\r\n"), &topic_buffer[k][0], &message_buffer[k][0]);
            write_data_file.flush();
            retry = 0;
            k++;
            sd_data_count++;
            data_saving_state = DATA_SAVING_DATA_LOOP;
            SERIAL_TRACE(F("DATA_SAVING_WRITE_FILE ---> DATA_SAVING_DATA_LOOP\r\n"));
         }
         // retry
         else if ((++retry) < DATA_SAVING_RETRY_COUNT_MAX) {
            delay_ms = DATA_SAVING_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = DATA_SAVING_WRITE_FILE;
            data_saving_state = DATA_SAVING_WAIT_STATE;
            SERIAL_TRACE(F("DATA_SAVING_WRITE_FILE ---> DATA_SAVING_WAIT_STATE\r\n"));
         }
         // fail
         else {
            SERIAL_ERROR(F("SD Card writing data on file %s... [ FAIL ]\r\n"), file_name);
            is_sdcard_error = true;
            data_saving_state = DATA_SAVING_CLOSE_FILE;
            SERIAL_TRACE(F("DATA_SAVING_WRITE_FILE ---> DATA_SAVING_CLOSE_FILE\r\n"));
         }
      break;

      case DATA_SAVING_CLOSE_FILE:
            is_sdcard_error = !write_data_file.close();
            data_saving_state = DATA_SAVING_END;
            SERIAL_TRACE(F("DATA_SAVING_CLOSE_FILE ---> DATA_SAVING_END\r\n"));
         break;

      case DATA_SAVING_END:
         SERIAL_INFO(F("[ %u ] data stored in sdcard... [ %s ]\r\n"), sd_data_count, is_sdcard_error ? ERROR_STRING : OK_STRING);
         LCD_INFO(&lcd, false, true, F("sdcard %u data %s"), sd_data_count, is_sdcard_error ? ERROR_STRING : OK_STRING);

         noInterrupts();
         if (!is_event_supervisor) {
            is_event_supervisor = true;
            ready_tasks_count++;
         }

         is_event_data_saving = false;
         ready_tasks_count--;
         interrupts();

         data_saving_state = DATA_SAVING_INIT;
         SERIAL_TRACE(F("DATA_SAVING_END ---> DATA_SAVING_INIT\r\n"));
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
   static tmElements_t datetime;
   static time_t current_ptr_time_data;
   static time_t last_correct_ptr_time_data;
   static time_t next_ptr_time_data;
   static uint32_t ipstack_timeout_ms;
   uint8_t ipstack_status;
   char file_name[SDCARD_FILES_NAME_MAX_LENGTH];
   int read_bytes_count;

   switch (mqtt_state) {
      case MQTT_INIT:
         retry = 0;
         is_ptr_found = false;
         is_ptr_updated = false;
         is_eof_data_read = false;
         is_mqtt_error = false;
         is_mqtt_published_data = false;
         mqtt_data_count = 0;

         if (!is_sdcard_open && !is_sdcard_error) {
            mqtt_state = MQTT_OPEN_SDCARD;
            SERIAL_TRACE(F("MQTT_PTR_DATA_INIT ---> MQTT_OPEN_SDCARD\r\n"));
         }
         else if (is_sdcard_open) {
            mqtt_state = MQTT_OPEN_PTR_FILE;
            SERIAL_TRACE(F("MQTT_PTR_DATA_INIT ---> MQTT_OPEN_PTR_FILE\r\n"));
         }
         else {
            mqtt_state = MQTT_PTR_END;
            SERIAL_TRACE(F("MQTT_PTR_DATA_INIT ---> MQTT_PTR_END\r\n"));
         }
      break;

      case MQTT_OPEN_SDCARD:
         if (sdcard_init(&SD, SDCARD_CHIP_SELECT_PIN)) {
            retry = 0;
            is_sdcard_open = true;
            is_sdcard_error = false;
            mqtt_state = MQTT_OPEN_PTR_FILE;
            SERIAL_TRACE(F("MQTT_OPEN_SDCARD ---> MQTT_OPEN_PTR_FILE\r\n"));
         }
         // retry
         else if ((++retry) < MQTT_RETRY_COUNT_MAX) {
            delay_ms = MQTT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = MQTT_OPEN_SDCARD;
            mqtt_state = MQTT_WAIT_STATE;
            SERIAL_TRACE(F("MQTT_OPEN_SDCARD ---> MQTT_PTR_DATA_WAIT_STATE\r\n"));
         }
         // fail
         else {
            is_sdcard_error = true;
            is_sdcard_open = false;
            SERIAL_ERROR(F("SD Card... [ FAIL ]\r\n--> is card inserted?\r\n--> there is a valid FAT32 filesystem?\r\n\r\n"));

            mqtt_state = MQTT_PTR_END;
            SERIAL_TRACE(F("MQTT_OPEN_SDCARD ---> MQTT_PTR_END\r\n"));
         }
         break;

      case MQTT_OPEN_PTR_FILE:
         // try to open file. if ok, read ptr data.
         if (sdcard_open_file(&SD, &mqtt_ptr_file, SDCARD_MQTT_PTR_FILE_NAME, O_RDWR | O_CREAT)) {
            retry = 0;
            mqtt_state = MQTT_PTR_READ;
            SERIAL_TRACE(F("MQTT_OPEN_PTR_FILE ---> MQTT_PTR_READ\r\n"));
         }
         // retry
         else if ((++retry) < MQTT_RETRY_COUNT_MAX) {
            delay_ms = MQTT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = MQTT_OPEN_PTR_FILE;
            mqtt_state = MQTT_WAIT_STATE;
            SERIAL_TRACE(F("MQTT_OPEN_PTR_FILE ---> MQTT_PTR_DATA_WAIT_STATE\r\n"));
         }
         // fail
         else {
            SERIAL_ERROR(F("SD Card open file %s... [ FAIL ]\r\n"), SDCARD_MQTT_PTR_FILE_NAME);
            is_sdcard_error = true;
            mqtt_state = MQTT_PTR_END;
            SERIAL_TRACE(F("MQTT_OPEN_PTR_FILE ---> MQTT_PTR_END\r\n"));
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
            SERIAL_TRACE(F("MQTT_PTR_READ ---> MQTT_PTR_FOUND\r\n"));
         }
         // not found (no sdcard error): find it by starting from 1th January of this year
         else if (read_bytes_count >= 0) {
            SERIAL_INFO(F("Data pointer... [ FIND ]\r\n"));
            datetime.Year = CalendarYrToTm(year(now()));
            datetime.Month = 1;
            datetime.Day = 1;
            datetime.Hour = 0;
            datetime.Minute = 0;
            datetime.Second = 0;
            ptr_time_data = makeTime(datetime);
            is_ptr_found = false;
            mqtt_state = MQTT_PTR_FIND;
            SERIAL_TRACE(F("MQTT_PTR_READ ---> MQTT_PTR_FIND\r\n"));
         }
         // not found (sdcard error)
         else {
            is_ptr_found = false;
            is_sdcard_error = true;
            mqtt_state = MQTT_PTR_END;
            SERIAL_TRACE(F("MQTT_PTR_READ ---> MQTT_PTR_END\r\n"));
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
               SERIAL_INFO(F("%s... [ FOUND ]\r\n"), file_name);
               mqtt_state = MQTT_PTR_END;
               SERIAL_TRACE(F("MQTT_PTR_FOUND ---> MQTT_PTR_END\r\n"));
            }
            else {
               SERIAL_INFO(F("%s... [ NOT FOUND ]\r\n"), file_name);
               ptr_time_data += SECS_PER_DAY;
            }
         }
         // ptr not found: set ptr to yesterday (today at 00:00:00 - readable_configuration.report_seconds time).
         else if (!is_ptr_found && ptr_time_data >= now()) {
            datetime.Year = CalendarYrToTm(year());
            datetime.Month = month();
            datetime.Day = day();
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
            SERIAL_TRACE(F("MQTT_PTR_FIND ---> MQTT_PTR_FOUND\r\n"));
         }
      break;

      case MQTT_PTR_FOUND:
         // datafile read, reach eof and is today. END.
         if (is_eof_data_read && year() == year(ptr_time_data) && month() == month(ptr_time_data) && day() == day(ptr_time_data)) {
            mqtt_state = MQTT_CLOSE_DATA_FILE;
            SERIAL_TRACE(F("MQTT_PTR_FOUND ---> MQTT_CLOSE_DATA_FILE\r\n"));
         }
         // datafile read, reach eof and NOT is today. go to end of this day.
         else if (is_eof_data_read) {
            datetime.Year = CalendarYrToTm(year(ptr_time_data));
            datetime.Month = month(ptr_time_data);
            datetime.Day = day(ptr_time_data) + 1;
            datetime.Hour = 0;
            datetime.Minute = 0;
            datetime.Second = 0;
            ptr_time_data = makeTime(datetime);
            ptr_time_data -= readable_configuration.report_seconds;
            is_ptr_updated = true;
            mqtt_state = MQTT_PTR_END;
            SERIAL_TRACE(F("MQTT_PTR_FOUND ---> MQTT_PTR_END\r\n"));
         }
         else {
            is_eof_data_read = false;
            mqtt_state = MQTT_PTR_END;
            SERIAL_TRACE(F("MQTT_PTR_FOUND ---> MQTT_PTR_END\r\n"));
         }
      break;

      case MQTT_PTR_END:
         // ptr data is found: send data saved on sdcard
         if (is_ptr_found && is_sdcard_open && !is_sdcard_error) {
            last_correct_ptr_time_data = ptr_time_data;
            SERIAL_INFO(F("Data pointer... [ %02u/%02u/%04u %02u:%02u:%02u ] [ %s ]\r\n"), day(ptr_time_data), month(ptr_time_data), year(ptr_time_data), hour(ptr_time_data), minute(ptr_time_data), second(ptr_time_data), OK_STRING);
            mqtt_state = MQTT_OPEN;
            SERIAL_TRACE(F("MQTT_PTR_END ---> MQTT_OPEN\r\n"));
         }
         // ptr data is NOT found: sd card fault fallback: send last acquired sensor data
         else {
            SERIAL_INFO(F("Data pointer... [ --/--/---- --:--:-- ] [ %s ]\r\n"), ERROR_STRING);
            is_sdcard_error = true;
            mqtt_state = MQTT_OPEN;
            SERIAL_TRACE(F("MQTT_PTR_END ---> MQTT_OPEN\r\n"));
         }
      break;

      case MQTT_OPEN:
         if (is_client_connected && mqtt_client.isConnected()) {
            mqtt_state = MQTT_CHECK;
            SERIAL_TRACE(F("MQTT_OPEN ---> MQTT_CHECK\r\n"));
         }
         else if (is_client_connected) {
            ipstack_timeout_ms = 0;
            mqtt_state = MQTT_CONNECT;
            SERIAL_TRACE(F("MQTT_OPEN ---> MQTT_CONNECT\r\n"));
         }
         // error: client not connected!
         else {
            is_mqtt_error = true;
            mqtt_state = MQTT_END;
            SERIAL_TRACE(F("MQTT_OPEN ---> MQTT_END\r\n"));
         }
         break;

      case MQTT_CONNECT:
         if (ipstack_timeout_ms == 0) {
            ipstack_timeout_ms = millis();
         }

         ipstack_status = ipstack.connect(readable_configuration.mqtt_server, readable_configuration.mqtt_port);

         // success
         if (ipstack_status == 1 && mqttConnect(readable_configuration.mqtt_username, readable_configuration.mqtt_password)) {
            retry = 0;
            SERIAL_DEBUG(F("MQTT Connection... [ %s ]\r\n"), OK_STRING);
            mqtt_state = MQTT_ON_CONNECT;
            SERIAL_TRACE(F("MQTT_CONNECT ---> MQTT_ON_CONNECT\r\n"));
         }
         // retry
         else if (ipstack_status == 2 && (++retry) < MQTT_RETRY_COUNT_MAX) {
            delay_ms = MQTT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = MQTT_CONNECT;
            mqtt_state = MQTT_WAIT_STATE;
            SERIAL_TRACE(F("MQTT_CONNECT ---> MQTT_WAIT_STATE\r\n"));
         }
         // fail
         #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH)
         else if (ipstack_status == 2 || (millis() - ipstack_timeout_ms >= ETHERNET_MQTT_TIMEOUT_MS)) {
         #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM)
         else if (ipstack_status == 2) {
         #endif
            SERIAL_ERROR(F("MQTT Connection... [ %s ]\r\n"), FAIL_STRING);
            is_mqtt_error = true;
            mqtt_state = MQTT_ON_DISCONNECT;
            SERIAL_TRACE(F("MQTT_CONNECT ---> MQTT_ON_DISCONNECT\r\n"));
         }
         // wait
      break;

      case MQTT_ON_CONNECT:
         getFullTopic(full_topic_buffer, readable_configuration.mqtt_maint_topic, MQTT_STATUS_TOPIC);
         snprintf(&message_buffer[0][0], MQTT_MESSAGE_LENGTH, MQTT_ON_CONNECT_MESSAGE);

         if (mqttPublish(full_topic_buffer, &message_buffer[0][0]), true) {
            retry = 0;
            mqtt_state = MQTT_SUBSCRIBE;
            SERIAL_TRACE(F("MQTT_ON_CONNECT ---> MQTT_SUBSCRIBE\r\n"));
         }
         // retry
         else if ((++retry) < MQTT_RETRY_COUNT_MAX) {
            delay_ms = MQTT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = MQTT_ON_CONNECT;
            mqtt_state = MQTT_WAIT_STATE;
            SERIAL_TRACE(F("MQTT_ON_CONNECT ---> MQTT_WAIT_STATE\r\n"));
         }
         // fail
         else {
            retry = 0;
            SERIAL_ERROR(F("MQTT on connect publish message... [ %s ]\r\n"), FAIL_STRING);
            is_mqtt_error = true;
            mqtt_state = MQTT_ON_DISCONNECT;
            SERIAL_TRACE(F("MQTT_ON_CONNECT ---> MQTT_ON_DISCONNECT\r\n"));
         }
      break;

      case MQTT_SUBSCRIBE:
         if (!is_mqtt_subscribed) {
            is_mqtt_subscribed = (mqtt_client.subscribe(readable_configuration.mqtt_subscribe_topic, MQTT::QOS1, mqttRxCallback) == 0);
            is_mqtt_error = !is_mqtt_subscribed;
            SERIAL_DEBUG(F("MQTT Subscription... [ %s ]\r\n"), is_mqtt_subscribed ? OK_STRING : FAIL_STRING);
         }

         mqtt_state = MQTT_CHECK;
         SERIAL_TRACE(F("MQTT_SUBSCRIBE ---> MQTT_CHECK\r\n"));
      break;

      case MQTT_CHECK:
         // ptr data is found: send data saved on sdcard
         if (!is_sdcard_error) {
            is_mqtt_processing_json = false;
            is_mqtt_processing_sdcard = true;
            is_eof_data_read = false;
            mqtt_state = MQTT_OPEN_DATA_FILE;
            SERIAL_TRACE(F("MQTT_CHECK ---> MQTT_OPEN_DATA_FILE\r\n"));
         }
         // ptr data is NOT found: sd card fault fallback: send last acquired sensor data
         else {
            is_mqtt_processing_json = true;
            is_mqtt_processing_sdcard = false;
            i = 0;
            mqtt_state = MQTT_SENSORS_LOOP;
            SERIAL_TRACE(F("MQTT_CHECK ---> MQTT_SENSORS_LOOP\r\n"));
         }
      break;

      case MQTT_SENSORS_LOOP:
         if (i < readable_configuration.sensors_count) {
            k = 0;
            data_count = jsonToMqtt(&json_sensors_data[i][0], readable_configuration.sensors[i].mqtt_topic, topic_buffer, message_buffer, (tmElements_t *) &sensor_reading_time);
            mqtt_state = MQTT_DATA_LOOP;
            SERIAL_TRACE(F("MQTT_SENSORS_LOOP ---> MQTT_DATA_LOOP\r\n"));
         }
         else if (is_mqtt_processing_json) {
            mqtt_state = MQTT_ON_DISCONNECT;
            SERIAL_TRACE(F("MQTT_SENSORS_LOOP ---> MQTT_ON_DISCONNECT\r\n"));
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
               SERIAL_TRACE(F("MQTT_SD_LOOP ---> MQTT_DATA_LOOP\r\n"));
            }
         }
         // EOF: End of File
         else {
            if (last_correct_ptr_time_data > ptr_time_data) {
               ptr_time_data = last_correct_ptr_time_data;
               is_ptr_updated = true;
            }
            is_eof_data_read = true;
            mqtt_state = MQTT_PTR_FOUND;
            SERIAL_TRACE(F("MQTT_SD_LOOP ---> MQTT_PTR_FOUND\r\n"));
         }
      break;

      case MQTT_DATA_LOOP:
         if (k < data_count && is_mqtt_processing_json) {
            getFullTopic(full_topic_buffer, readable_configuration.mqtt_root_topic, &topic_buffer[k][0]);
            mqtt_state = MQTT_PUBLISH;
            SERIAL_TRACE(F("MQTT_DATA_LOOP ---> MQTT_PUBLISH\r\n"));
         }
         else if (is_mqtt_processing_sdcard) {
            getFullTopic(full_topic_buffer, readable_configuration.mqtt_root_topic, &topic_buffer[0][0]);
            mqtt_state = MQTT_PUBLISH;
            SERIAL_TRACE(F("MQTT_DATA_LOOP ---> MQTT_PUBLISH\r\n"));
         }
         else {
            i++;
            mqtt_state = MQTT_SENSORS_LOOP;
            SERIAL_TRACE(F("MQTT_DATA_LOOP ---> MQTT_SENSORS_LOOP\r\n"));
         }
      break;

      case MQTT_PUBLISH:
         is_mqtt_published_data = true;

         // mqtt json success
         if (is_mqtt_processing_json && mqttPublish(full_topic_buffer, &message_buffer[k][0])) {
            SERIAL_DEBUG(F("MQTT <-- %s %s\r\n"), &topic_buffer[k][0], &message_buffer[k][0]);
            retry = 0;
            k++;
            mqtt_data_count++;
            mqtt_state = MQTT_DATA_LOOP;
            SERIAL_TRACE(F("MQTT_PUBLISH ---> MQTT_DATA_LOOP\r\n"));
         }
         // mqtt sdcard success
         else if (is_mqtt_processing_sdcard && mqttPublish(full_topic_buffer, &message_buffer[0][0])) {
            SERIAL_DEBUG(F("MQTT <-- %s %s\r\n"), &topic_buffer[0][0], &message_buffer[0][0]);
            retry = 0;
            mqtt_data_count++;
            mqtt_state = MQTT_SD_LOOP;
            SERIAL_TRACE(F("MQTT_PUBLISH ---> MQTT_SD_LOOP\r\n"));
         }
         // retry
         else if ((++retry) < MQTT_RETRY_COUNT_MAX) {
            delay_ms = MQTT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = MQTT_PUBLISH;
            mqtt_state = MQTT_WAIT_STATE;
            SERIAL_TRACE(F("MQTT_PUBLISH ---> MQTT_WAIT_STATE\r\n"));
         }
         // fail
         else {
            ptr_time_data = current_ptr_time_data - readable_configuration.report_seconds;
            is_ptr_updated = true;

            is_eof_data_read = true;
            is_mqtt_error = true;
            SERIAL_ERROR(F("MQTT publish... [ %s ]\r\n"), FAIL_STRING);

            if (is_mqtt_processing_json) {
               mqtt_state = MQTT_ON_DISCONNECT;
               SERIAL_TRACE(F("MQTT_PUBLISH ---> MQTT_ON_DISCONNECT\r\n"));
            }
            else if (is_mqtt_processing_sdcard) {
               mqtt_state = MQTT_CLOSE_DATA_FILE;
               SERIAL_TRACE(F("MQTT_PUBLISH ---> MQTT_CLOSE_DATA_FILE\r\n"));
            }
         }
      break;

      case MQTT_OPEN_DATA_FILE:
         // open the file that corresponds to the next data to send
         next_ptr_time_data = ptr_time_data + readable_configuration.report_seconds;
         sdcard_make_filename(next_ptr_time_data, file_name);

         // open file for read data
         if (sdcard_open_file(&SD, &read_data_file, file_name, O_READ)) {
            retry = 0;
            mqtt_state = MQTT_SD_LOOP;
            SERIAL_TRACE(F("MQTT_OPEN_DATA_FILE ---> MQTT_SD_LOOP\r\n"));
         }
         // error: file doesn't exist but if is today, end.
         else if (!is_sdcard_error && year(next_ptr_time_data) == year() && month(next_ptr_time_data) == month() && day(next_ptr_time_data) == day()) {
            mqtt_state = MQTT_PTR_UPDATE;
            SERIAL_TRACE(F("MQTT_OPEN_DATA_FILE ---> MQTT_PTR_UPDATE\r\n"));
         }
         // error: file doesn't exist and if it isn't today, jump to next day and search in it
         else if (!is_sdcard_error) {
            is_ptr_found = false;
            ptr_time_data = next_ptr_time_data;
            mqtt_state = MQTT_PTR_FIND;
            SERIAL_TRACE(F("MQTT_OPEN_DATA_FILE ---> MQTT_PTR_FIND\r\n"));
         }
         // fail
         else {
            SERIAL_ERROR(F("SD Card open file %s... [ FAIL ]\r\n"), file_name);
            is_sdcard_error = true;
            mqtt_state = MQTT_CHECK; // fallback
            SERIAL_TRACE(F("MQTT_OPEN_DATA_FILE ---> MQTT_CHECK\r\n"));
         }
         break;

      case MQTT_CLOSE_DATA_FILE:
         if (is_mqtt_processing_sdcard) {
            is_sdcard_error = !read_data_file.close();
            mqtt_state = MQTT_ON_DISCONNECT;
            SERIAL_TRACE(F("MQTT_CLOSE_DATA_FILE ---> MQTT_ON_DISCONNECT\r\n"));
         }
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
               SERIAL_TRACE(F("MQTT_ON_DISCONNECT ---> MQTT_DISCONNECT\r\n"));
            }
            // retry
            else if ((++retry) < MQTT_RETRY_COUNT_MAX) {
               delay_ms = MQTT_DELAY_MS;
               start_time_ms = millis();
               state_after_wait = MQTT_ON_DISCONNECT;
               mqtt_state = MQTT_WAIT_STATE;
               SERIAL_TRACE(F("MQTT_ON_DISCONNECT ---> MQTT_WAIT_STATE\r\n"));
            }
            // fail
            else {
               SERIAL_ERROR(F("MQTT on disconnect publish message... [ %s ]\r\n"), FAIL_STRING);
               retry = 0;
               is_mqtt_error = true;
               mqtt_state = MQTT_DISCONNECT;
               SERIAL_TRACE(F("MQTT_ON_DISCONNECT ---> MQTT_DISCONNECT\r\n"));
            }

         #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH)
         }
         else {
            mqtt_state = MQTT_DISCONNECT;
            SERIAL_TRACE(F("MQTT_ON_DISCONNECT ---> MQTT_DISCONNECT\r\n"));
         }
         #endif
      break;

      case MQTT_DISCONNECT:
         #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH)
         if (is_mqtt_error) {
         #endif

         mqtt_client.disconnect();
         ipstack.disconnect();
         SERIAL_DEBUG(F("MQTT Disconnect... [ %s ]\r\n"), OK_STRING);

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
         SERIAL_TRACE(F("MQTT_DISCONNECT ---> MQTT_PTR_UPDATE\r\n"));

      break;

      case MQTT_PTR_UPDATE:
         if (is_ptr_updated) {
            // set ptr 1 second more for send next data to current ptr
            ptr_time_data++;

            // success
            if (mqtt_ptr_file.seekSet(0) && mqtt_ptr_file.write(&ptr_time_data, sizeof(time_t)) == sizeof(time_t)) {
               mqtt_ptr_file.flush();
               breakTime(ptr_time_data, datetime);
               SERIAL_INFO(F("Data pointer... [ %02u/%02u/%04u %02u:%02u:%02u ] [ %s ]\r\n"), datetime.Day, datetime.Month, tmYearToCalendar(datetime.Year), datetime.Hour, datetime.Minute, datetime.Second, "UPDATE");
               mqtt_state = MQTT_CLOSE_PTR_FILE;
               SERIAL_TRACE(F("MQTT_PTR_UPDATE ---> MQTT_CLOSE_PTR_FILE\r\n"));
            }
            // retry
            else if ((++retry) < MQTT_RETRY_COUNT_MAX) {
               delay_ms = MQTT_DELAY_MS;
               start_time_ms = millis();
               state_after_wait = MQTT_PTR_UPDATE;
               mqtt_state = MQTT_WAIT_STATE;
               SERIAL_TRACE(F("MQTT_PTR_UPDATE ---> MQTT_WAIT_STATE\r\n"));
            }
            // fail
            else {
               SERIAL_ERROR(F("SD Card writing ptr data on file %s... [ %s ]\r\n"), SDCARD_MQTT_PTR_FILE_NAME, FAIL_STRING);
               mqtt_state = MQTT_CLOSE_PTR_FILE;
               SERIAL_TRACE(F("MQTT_PTR_UPDATE ---> MQTT_CLOSE_PTR_FILE\r\n"));
            }
         }
         else {
            mqtt_state = MQTT_CLOSE_PTR_FILE;
            SERIAL_TRACE(F("MQTT_PTR_UPDATE ---> MQTT_CLOSE_PTR_FILE\r\n"));
         }
         break;

      case MQTT_CLOSE_PTR_FILE:
         mqtt_ptr_file.close();
         mqtt_state = MQTT_CLOSE_SDCARD;
         SERIAL_TRACE(F("MQTT_CLOSE_PTR_FILE ---> MQTT_CLOSE_SDCARD\r\n"));
         break;

      case MQTT_CLOSE_SDCARD:
         is_sdcard_error = false;
         is_sdcard_open = false;
         mqtt_state = MQTT_END;
         SERIAL_TRACE(F("MQTT_CLOSE_SDCARD ---> MQTT_END\r\n"));
         break;

      case MQTT_END:
         if (is_mqtt_published_data) {
            SERIAL_INFO(F("[ %u ] data published through mqtt... [ %s ]\r\n"), mqtt_data_count, is_mqtt_error ? ERROR_STRING : OK_STRING);
            LCD_INFO(&lcd, false, true, F("mqtt %u data %s"), mqtt_data_count, is_mqtt_error ? ERROR_STRING : OK_STRING);
         }

         noInterrupts();
         is_event_mqtt_paused = false;
         is_event_mqtt = false;
         ready_tasks_count--;
         interrupts();

         mqtt_state = MQTT_INIT;
         SERIAL_TRACE(F("MQTT_END ---> MQTT_INIT\r\n"));
      break;

      case MQTT_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            mqtt_state = state_after_wait;
         }
      break;
   }
}
#endif
