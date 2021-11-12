/**@file sim800.cpp */

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

#include "sim800Client.h"

/*!
\var buffer_ext
\brief Buffer for send AT command and receive response.
*/
char buffer_ext[SIM800_BUFFER_LENGTH];

/*!
\var buffer_ext2
\brief Buffer for send AT command and receive response.
*/
char buffer_ext2[SIM800_BUFFER_LENGTH];

bool SIM800::isOn() {
   return (state & SIM800_STATE_ON);
}

bool SIM800::isInitialized() {
   return (state & SIM800_STATE_INITIALIZED);
}

bool SIM800::isSetted() {
   return (state & SIM800_STATE_SETTED);
}

bool SIM800::isRegistered() {
   return (state & SIM800_STATE_REGISTERED);
}

bool SIM800::isHttpInitialized() {
   return (state & SIM800_STATE_HTTP_INITIALIZED);
}

// return true when switch on
sim800_status_t SIM800::switchOn() {
   bool is_switching_on = true;
   sim800_status_t at_command_status;

   at_command_status = switchModem(is_switching_on);

   if (at_command_status != SIM800_BUSY) {
      LOGN(F("SIM800 switching ON... [ %s ] [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING), isOn() ? ON_STRING : OFF_STRING);
   }

   return at_command_status;
}

// return true when switch off
sim800_status_t SIM800::switchOff(uint8_t power_off_method) {
   bool is_switching_on = false;
   sim800_status_t at_command_status = SIM800_OK;

   if (power_off_method == SIM800_POWER_OFF_BY_SWITCH) {
      at_command_status = switchModem(is_switching_on);
   }
   else if (power_off_method == SIM800_POWER_OFF_BY_AT_COMMAND) {
      at_command_status = softwareSwitchOff();
   }

   if (at_command_status != SIM800_BUSY) {
      LOGN(F("SIM800 switching OFF... [ %s ] [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING), isOn() ? ON_STRING : OFF_STRING);
   }

   return at_command_status;
}

sim800_status_t SIM800::switchModem(bool is_switching_on) {
   static uint8_t retry;
   static sim800_power_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;
   static bool is_error;
   static sim800_status_t sim800_status;
   sim800_status_t at_command_status;

   switch (sim800_power_state) {

      case SIM800_POWER_INIT:
         retry = 0;
         is_error = false;
         sim800_status = SIM800_BUSY;
         sim800_power_state = SIM800_POWER_SET_PIN_LOW;
         break;

      // pin low for SIM800_POWER_ON_OFF_SWITCH_DELAY_MS milliseconds
      case SIM800_POWER_SET_PIN_LOW:
         digitalWrite(on_off_pin, LOW);
         delay_ms = SIM800_POWER_ON_OFF_SWITCH_DELAY_MS;
         start_time_ms = millis();
         state_after_wait = SIM800_POWER_SET_PIN_HIGH;
         sim800_power_state = SIM800_POWER_WAIT_STATE;
         break;

      // pin high for SIM800_POWER_ON_OFF_DONE_DELAY_MS milliseconds
      case SIM800_POWER_SET_PIN_HIGH:
         digitalWrite(on_off_pin, HIGH);
         delay_ms = SIM800_POWER_ON_OFF_DONE_DELAY_MS;
         start_time_ms = millis();
         state_after_wait = SIM800_POWER_CHECK_STATUS;
         sim800_power_state = SIM800_POWER_WAIT_STATE;
         break;

      case SIM800_POWER_CHECK_STATUS:
         at_command_status = sendAt();

         // success: switching ON and is ON
         if (is_switching_on && (at_command_status == SIM800_OK)) {
            state |= SIM800_STATE_ON;
            sim800_power_state = SIM800_POWER_END;
         }
         //success: switching OFF and is OFF
         else if (!is_switching_on && (at_command_status == SIM800_ERROR)) {
            state &= ~SIM800_STATE_INITIALIZED;
            state &= ~SIM800_STATE_ON;
            sim800_power_state = SIM800_POWER_END;
         }
         // fail: switching ON and is OFF
         else if (is_switching_on && (at_command_status == SIM800_ERROR)) {
            is_error = true;
            state &= ~SIM800_STATE_INITIALIZED;
            state &= ~SIM800_STATE_ON;
            sim800_power_state = SIM800_POWER_END;
         }
         // fail: switching OFF and is ON
         else if (!is_switching_on && (at_command_status == SIM800_OK)) {
            is_error = true;
            state |= SIM800_STATE_ON;
            sim800_power_state = SIM800_POWER_END;
         }
         // wait...
         break;

      case SIM800_POWER_END:
         // success
         if (!is_error) {
            sim800_status = SIM800_OK;
            sim800_power_state = SIM800_POWER_INIT;
         }
         // retry
         else if ((++retry) < SIM800_GENERIC_RETRY_COUNT_MAX) {
            is_error = false;
            sim800_status = SIM800_BUSY;
            sim800_power_state = SIM800_POWER_SET_PIN_LOW;
         }
         // fail
         else {
            sim800_status = SIM800_ERROR;
            sim800_power_state = SIM800_POWER_INIT;
         }
         break;

      case SIM800_POWER_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            sim800_power_state = state_after_wait;
         }
         break;
   }

   return sim800_status;
}

SIM800::SIM800() {}

/*
bool SIM800::httpGET(const char* server, int port, const char* path, char* result, int resultlength)
{
  char bufcommand[BUFCOMMAND_LENGTH];
  char buf[BUF_LENGTH];
  char* newpath;

  IF_SDEBUG(Serial.println(F("#sim800:start httpget ...")));

  if (!isInitialized()) return SIM800_ERROR;
  if (!isRegistered())  return SIM800_ERROR;

  if (isHttpInitialized()){
    ATcommand("+HTTPTERM", buf);
    state &= ~STATE_HTTPINITIALIZED;
  }

  if(!ATcommand("+HTTPINIT", buf)){
    ATcommand("+HTTPTERM", buf);
    if(!ATcommand("+HTTPINIT", buf)) return false;
    state |= STATE_HTTPINITIALIZED;
  }

  if(!ATcommand("+HTTPPARA=\"CID\",1", buf)) return false;
  newpath = str_replace(path,"\"", "%22");
  sprintf(bufcommand, "+HTTPPARA=\"URL\",\"%s%s\"",server,newpath );
  free(newpath);
  if(!ATcommand(bufcommand, buf)) return false;
  if(!ATcommand("+HTTPACTION=0", buf)) return false;
  //receive(buf,5000,"\n",NULL);             // here we receive some spourious \r\n; do not wait for it
  receive(buf,20000,"+HTTPACTION",NULL);     // timeout for response 20 sec
  receive(buf,5000,"\n",NULL);
  int method;
  int status;
  int datalen;
  int token_count = sscanf(buf,":%i,%i,%i",&method,&status,&datalen);
  if ( token_count == 3 ){
    IF_SDEBUG(Serial.print(F("#sim800:method: ")));
    IF_SDEBUG(Serial.println(method));
    IF_SDEBUG(Serial.print(F("#sim800:status: ")));
    IF_SDEBUG(Serial.println(status));
    IF_SDEBUG(Serial.print(F("#sim800:datalen: ")));
    IF_SDEBUG(Serial.println(datalen));
    }else{
    IF_SDEBUG(Serial.println(F("#sim800:ERROR httpaction")));
    IF_SDEBUG(Serial.print(F("#sim800:token count: ")));
    IF_SDEBUG(Serial.println(token_count));
    IF_SDEBUG(Serial.println(buf));
    return false;
  }
  if (status != 200) return false;
  send("AT+HTTPREAD\r\n");
  if(!receive(buf,5000,"\r\n",NULL)) return false;  // get +HTTPREAD: n
  if(!receive(buf,5000,"\r\n",NULL)) return false;   // gel null line

  if (datalen+1 > resultlength){
    IF_SDEBUG(Serial.println(F("#sim800:ERROR no buffer space for http response")));
    return false;
  }
  if(!receivelen(result,5000,datalen)) return false;
  if(!receive(buf,5000,"OK\r\n",NULL)) return false;  
  if(!ATcommand("+HTTPTERM", buf)) return false;
  state &= ~STATE_HTTPINITIALIZED;

  return true;
}
*/

uint8_t SIM800::receive(char *rx_buffer, const char *at_ok_string, const char *at_error_string) {
   memset(rx_buffer, 0, SIM800_BUFFER_LENGTH);
   uint8_t rx_buffer_length = 0;
   bool is_responce_ok = false;
   bool is_responce_error = false;
   char *write_ptr = rx_buffer;
   char readed_char;

   uint32_t start_time_ms = millis();
// check ms timeout: set to 50 for slow baud rate
   while ((millis() - start_time_ms <= 5) && (rx_buffer_length < (SIM800_BUFFER_LENGTH - 1)) && !is_responce_ok && !is_responce_error) {
      if (modem->available()) {
         start_time_ms = millis();

         readed_char = modem->read();

         if (readed_char == '\r' || readed_char == '\n') {
            if (rx_buffer_length > 2) {
               *write_ptr++ = ' ';
               rx_buffer_length++;
            }
         }
         else {
            *write_ptr++ = readed_char;
            rx_buffer_length++;

            *write_ptr = '\0';

            if (at_ok_string) {
               is_responce_ok = found(rx_buffer, at_ok_string);
            }

            if (at_error_string && !is_responce_ok){
               is_responce_error = found(rx_buffer, at_error_string);
            }
         }
      }
   }

   if (rx_buffer[rx_buffer_length-2] == ' ') {
      rx_buffer_length -= 2;
      rx_buffer[rx_buffer_length] = '\0';
   }

   if (is_responce_error) {
      rx_buffer_length = 0;
   }

   if (rx_buffer_length) {
     LOGV(F("SIM800<-- %s"), rx_buffer);
   }

   return rx_buffer_length;
}

void SIM800::cleanInput () {
   while (modem->available()) {
      modem->read();
   }
}

sim800_status_t SIM800::sendAtCommand(const char *command, char *buffer, const char *at_ok_string, const char *at_error_string, uint32_t timeout_ms) {
   static uint32_t delay_ms;
   static uint32_t start_time_ms;
   static sim800_at_state_t state_after_wait;
   static sim800_status_t sim800_status;
   static bool is_error;
   static uint8_t rx_data_length;

   switch (sim800_at_state) {

      case SIM800_AT_INIT:
         memset(buffer, 0, SIM800_BUFFER_LENGTH);
         rx_data_length = 0;
         is_error = false;
         sim800_status = SIM800_BUSY;

         delay_ms = SIM800_AT_DELAY_MS;
         start_time_ms = millis();
         state_after_wait = SIM800_AT_SEND;
         sim800_at_state = SIM800_AT_WAIT_STATE;
         break;

      case SIM800_AT_SEND:
         cleanInput();
         start_time_ms = 0;
         strncpy(buffer, command, SIM800_BUFFER_LENGTH);
         send(buffer);
         LOGV(F("SIM800--> %s"), buffer);
         sim800_at_state = SIM800_AT_RECEIVE;
         break;

      case SIM800_AT_RECEIVE:
         if (start_time_ms == 0) {
            start_time_ms = millis();
         }

         rx_data_length = receive(buffer, at_ok_string, at_error_string);

         // success
         if (rx_data_length) {
            sim800_at_state = SIM800_AT_END;
         }
         // timeout elapsed
         else if (millis() - start_time_ms > timeout_ms) {
            is_error = true;
            sim800_at_state = SIM800_AT_END;
         }
         // wait...
         break;

      case SIM800_AT_END:
         sim800_status = (is_error ? SIM800_ERROR : SIM800_OK);
         sim800_at_state = SIM800_AT_INIT;
         break;

      case SIM800_AT_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            sim800_at_state = state_after_wait;
         }
         break;
   }

   return sim800_status;
}

bool SIM800::init(uint8_t _on_off_pin, uint8_t _reset_pin) {
   on_off_pin = _on_off_pin;
   reset_pin = _reset_pin;

   pinMode(on_off_pin, OUTPUT);
   digitalWrite(on_off_pin, HIGH);
   LOGT(F("SIM800 on/off pin [ %d ]"), on_off_pin);

   if (reset_pin != 0xFF) {
      pinMode(reset_pin, OUTPUT);
      digitalWrite(reset_pin, HIGH);
      LOGT(F("SIM800 reset pin [ %d ]"), reset_pin);
   }

   state = SIM800_STATE_NONE;
   sim800_at_state = SIM800_AT_INIT;
   sim800_power_state = SIM800_POWER_INIT;
   sim800_setup_state = SIM800_SETUP_INIT;
   sim800_connection_start_state = SIM800_CONNECTION_START_INIT;
   sim800_connection_state = SIM800_CONNECTION_INIT;
   sim800_connection_stop_state = SIM800_CONNECTION_STOP_INIT;
   sim800_exit_transparent_mode_state = SIM800_EXIT_TRANSPARENT_MODE_INIT;

   modem->begin(SIM800_SERIAL_PORT_BAUD_RATE);

   return true;
}

void SIM800::setSerial(HardwareSerial *serial) {
   modem = serial;
}

sim800_status_t SIM800::sendAt() {
   return sendAtCommand("AT\r\n", buffer_ext);
}

sim800_status_t SIM800::initAutobaud() {
   sim800_status_t at_command_status;

   // sync autobaud
   at_command_status = sendAt();

   if (at_command_status == SIM800_OK) {
      state |= SIM800_STATE_INITIALIZED;
   }

   if (at_command_status != SIM800_BUSY) {
      LOGT(F("SIM800 autobaud... [ %s ]"), printStatus(at_command_status, OK_STRING, FAIL_STRING));
   }

   return at_command_status;
}

sim800_status_t SIM800::getGsn(char *imei) {
   sim800_status_t at_command_status;

   if (!isInitialized())
      return SIM800_ERROR;

   at_command_status = sendAtCommand("AT+GSN\r\n", buffer_ext);

   if (at_command_status == SIM800_OK) {
      if (sscanf(buffer_ext, "%s", imei) != 1) {
         at_command_status = SIM800_ERROR;
      }
   }

   if (at_command_status != SIM800_BUSY) {
      LOGT(F("SIM800 IMEI [ %s ] [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING), imei);
   }

   return at_command_status;
}

sim800_status_t SIM800::getCreg(uint8_t *n, uint8_t *stat) {
   sim800_status_t at_command_status;

   if (!isInitialized())
      return SIM800_ERROR;

   unsigned int nn,statstat;

   at_command_status = sendAtCommand("AT+CREG?\r\n", buffer_ext);

   if (at_command_status == SIM800_OK) {
     if (sscanf(buffer_ext, "+CREG: %d,%d", &nn, &statstat) != 2) {
       //if (sscanf(buffer_ext, "+CREG: %hhu,%hhu", n, stat) != 2) {
       at_command_status = SIM800_ERROR;
       *n = 0;
       *stat = 0;
     }else{
       *n=nn;
       *stat=statstat;
     }
   } else {
     *n = 0;
     *stat = 0;
   }

   if (at_command_status != SIM800_BUSY) {
      LOGT(F("SIM800 CREG [ %s ] [ %d,%d ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING), nn, statstat);
   }

   return at_command_status;
}

sim800_status_t SIM800::getCsq(uint8_t *rssi, uint8_t *ber) {
   sim800_status_t at_command_status;
   unsigned int rssirssi, berber;

   if (!isInitialized())
      return SIM800_ERROR;

   at_command_status = sendAtCommand("AT+CSQ\r\n", buffer_ext);

   if (at_command_status == SIM800_OK) {
     //if (sscanf(buffer_ext, "+CSQ: %hhu,%hhu", rssi, ber) != 2) {
     if (sscanf(buffer_ext, "+CSQ: %d,%d", &rssirssi, &berber) != 2) {
       at_command_status = SIM800_ERROR;
       *rssi = SIM800_RSSI_UNKNOWN;
       *ber = SIM800_BER_UNKNOWN;
     } else {
       *rssi=rssirssi;
       *ber=berber;
     }
   } else {
      *rssi = SIM800_RSSI_UNKNOWN;
      *ber = SIM800_BER_UNKNOWN;
   }

   if (at_command_status != SIM800_BUSY) {
      LOGT(F("SIM800 CSQ [ %s ] [ %d,%d ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING), rssirssi, berber);
   }

   return at_command_status;
}

sim800_status_t SIM800::getCgatt(bool *is_attached) {
   sim800_status_t at_command_status;

   unsigned int is_gprs_attached = 0;

   if (!isInitialized())
      return SIM800_ERROR;

   at_command_status = sendAtCommand("AT+CGATT?\r\n", buffer_ext);

   if (at_command_status == SIM800_OK) {
      if (sscanf(buffer_ext, "+CGATT: %d", &is_gprs_attached) != 1) {
         at_command_status = SIM800_ERROR;
	 is_gprs_attached=0;
      }
   }
   *is_attached = (bool) is_gprs_attached;

   if (at_command_status != SIM800_BUSY) {
      LOGT(F("SIM800 CGATT [ %s ] [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING), *is_attached ? YES_STRING : NO_STRING);
   }

   return at_command_status;
}

sim800_status_t SIM800::getCifsr(char *ip) {
   sim800_status_t at_command_status;

   if (!isInitialized())
      return SIM800_ERROR;

   at_command_status = sendAtCommand("AT+CIFSR\r\n", buffer_ext);

   if (at_command_status == SIM800_OK) {
      if (sscanf(buffer_ext, "%s", ip) != 1) {
         at_command_status = SIM800_ERROR;
         strcpy(ip, "0.0.0.0");
      }
   }

   return at_command_status;
}

sim800_status_t SIM800::sendCpowd() {
   sim800_status_t at_command_status;

   at_command_status = sendAtCommand("AT+CPOWD=1\r\n", buffer_ext, AT_NORMAL_POWER_DOWN_STRING, NULL, SIM800_AT_DEFAULT_TIMEOUT_MS);

   if (at_command_status == SIM800_OK || at_command_status == SIM800_ERROR) {
      state = SIM800_STATE_NONE;
   }

   return at_command_status;
}

sim800_status_t SIM800::exitTransparentMode() {
   static uint32_t delay_ms;
   static uint32_t start_time_ms;
   static sim800_exit_transparent_mode_state_t state_after_wait;
   static sim800_status_t sim800_status;
   static sim800_status_t at_command_status;
   static bool is_error;

   switch (sim800_exit_transparent_mode_state) {

      case SIM800_EXIT_TRANSPARENT_MODE_INIT:
         is_error = false;
         sim800_status = SIM800_BUSY;

         // wait 1 second
         delay_ms = SIM800_WAIT_FOR_EXIT_TRANSPARENT_MODE_DELAY_MS;
         start_time_ms = millis();
         state_after_wait = SIM800_EXIT_TRANSPARENT_MODE_SEND_ESCAPE_SEQUENCE;
         sim800_exit_transparent_mode_state = SIM800_EXIT_TRANSPARENT_MODE_WAIT_STATE;
         break;

      case SIM800_EXIT_TRANSPARENT_MODE_SEND_ESCAPE_SEQUENCE:
         at_command_status = sendAtCommand("+++", buffer_ext);

         // success
         if (at_command_status == SIM800_OK) {
            // wait 1 second
            delay_ms = SIM800_WAIT_FOR_EXIT_TRANSPARENT_MODE_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SIM800_EXIT_TRANSPARENT_MODE_END;
            sim800_exit_transparent_mode_state = SIM800_EXIT_TRANSPARENT_MODE_WAIT_STATE;
         }
         else if (at_command_status == SIM800_ERROR) {
            is_error = true;
            sim800_exit_transparent_mode_state = SIM800_EXIT_TRANSPARENT_MODE_END;
         }
         // wait...
         break;

      case SIM800_EXIT_TRANSPARENT_MODE_END:
         sim800_status = (is_error ? SIM800_ERROR : SIM800_OK);
         sim800_exit_transparent_mode_state = SIM800_EXIT_TRANSPARENT_MODE_INIT;
         LOGT(F("SIM800 switch to command mode [ %s ]"), sim800_status == SIM800_OK ? OK_STRING : FAIL_STRING);
         break;

      case SIM800_EXIT_TRANSPARENT_MODE_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            sim800_exit_transparent_mode_state = state_after_wait;
         }
         break;
   }

   return sim800_status;
}

sim800_status_t SIM800::connection(const char *tipo, const char *server, const int port) {
   static uint32_t delay_ms;
   static uint32_t start_time_ms;
   static sim800_connection_state_t state_after_wait;
   static sim800_status_t sim800_status;
   static sim800_status_t at_command_status;
   static bool is_error;
   static uint8_t rx_data_length;

   switch (sim800_connection_state) {

      case SIM800_CONNECTION_INIT:
         rx_data_length = 0;
         is_error = false;
         sim800_status = SIM800_BUSY;

         if (isInitialized()) {
            start_time_ms = millis();
            delay_ms = SIM800_WAIT_FOR_CONNECTION_DELAY_MS;
            sim800_connection_state = SIM800_CONNECTION_WAIT_STATE;
            state_after_wait = SIM800_CONNECTION_OPEN;
            snprintf(buffer_ext2, SIM800_BUFFER_LENGTH, "AT+CIPSTART=\"%s\",\"%s\",\"%i\"\r\n", tipo, server, port);
         }
         else {
            is_error = true;
            sim800_connection_state = SIM800_CONNECTION_END;
         }

         break;

      case SIM800_CONNECTION_OPEN:
         at_command_status = sendAtCommand(buffer_ext2, buffer_ext, AT_OK_STRING, AT_ERROR_STRING, SIM800_CIPSTART_RESPONSE_TIME_MAX_MS);

         // success
         if (at_command_status == SIM800_OK) {
            sim800_connection_state = SIM800_CONNECTION_CHECK_STATUS;
         }
         // fail
         else if (at_command_status == SIM800_ERROR) {
            is_error = true;
            sim800_connection_state = SIM800_CONNECTION_END;
         }
         // wait...
         break;

      case SIM800_CONNECTION_CHECK_STATUS:
         if (start_time_ms == 0) {
            start_time_ms = millis();
         }

         rx_data_length = receive(buffer_ext, NULL, NULL);

         // fail
         if (rx_data_length && found(buffer_ext, AT_CONNECT_FAIL_STRING)) {
            LOGE(F("SIM800 %s status... [ %s ] [ %s ]"), tipo, ERROR_STRING, buffer_ext);
            is_error = true;
            sim800_connection_state = SIM800_CONNECTION_END;
         }
         // success
         else if (rx_data_length && found(buffer_ext, AT_CONNECT_OK_STRING)) {
            LOGN(F("SIM800 %s status... [ %s ] [ %s ]"), tipo, OK_STRING, buffer_ext);
            sim800_connection_state = SIM800_CONNECTION_END;
         }
         // timeout fail
         else if (millis() - start_time_ms > SIM800_CIPSTART_RESPONSE_TIME_MAX_MS) {
            LOGE(F("SIM800 %s status... [ %s ]"), tipo, ERROR_STRING);
            is_error = true;
            sim800_connection_state = SIM800_CONNECTION_END;
         }
         // wait...
         break;

      case SIM800_CONNECTION_END:
         sim800_status = (is_error ? SIM800_ERROR : SIM800_OK);
         sim800_connection_state = SIM800_CONNECTION_INIT;
         break;

      case SIM800_CONNECTION_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            sim800_connection_state = state_after_wait;
         }
         break;
   }

   return sim800_status;
}

sim800_status_t SIM800::setup() {
   static uint8_t retry;
   static sim800_setup_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;
   static sim800_status_t sim800_status;
   static bool is_error;
   static bool is_registered;
   sim800_status_t at_command_status;
   uint8_t n;
   uint8_t stat;
   uint8_t rssi;
   uint8_t ber;

   switch (sim800_setup_state) {

      case SIM800_SETUP_INIT:
         retry = 0;
         is_error = false;
         sim800_status = SIM800_BUSY;

         // is baud set?
         if (isInitialized()) {
            sim800_setup_state = SIM800_SETUP_RESET;
         }
         else {
            is_error = true;
            sim800_setup_state = SIM800_SETUP_END;
         }
         break;

      case SIM800_SETUP_RESET:
         // reset to factory default
         at_command_status = sendAtCommand("AT&F\r\n", buffer_ext);

         // success
         if (at_command_status == SIM800_OK) {
            start_time_ms = millis();
            delay_ms = SIM800_WAIT_FOR_SETUP_DELAY_MS;
            state_after_wait = SIM800_SETUP_ECHO_MODE;
            sim800_setup_state = SIM800_SETUP_WAIT_STATE;
         }
         // fail
         else if (at_command_status == SIM800_ERROR) {
            is_error = true;
            sim800_setup_state = SIM800_SETUP_END;
         }

         if (at_command_status != SIM800_BUSY) {
            LOGN(F("SIM800 reset to factory default [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
         }
         break;

      case SIM800_SETUP_ECHO_MODE:
         // echo mode off
         at_command_status = sendAtCommand("ATE0\r\n", buffer_ext);

         // success
         if (at_command_status == SIM800_OK) {
            start_time_ms = millis();
            delay_ms = SIM800_WAIT_FOR_GET_SIGNAL_QUALITY_DELAY_MS;
            state_after_wait = SIM800_SETUP_GET_SIGNAL_QUALITY;
            sim800_setup_state = SIM800_SETUP_WAIT_STATE;
         }
         // fail
         else if (at_command_status == SIM800_ERROR) {
            is_error = true;
            sim800_setup_state = SIM800_SETUP_END;
         }

         if (at_command_status != SIM800_BUSY) {
            LOGN(F("SIM800 echo mode off [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
         }

         // wait...
         break;

      case SIM800_SETUP_GET_SIGNAL_QUALITY:
         at_command_status = getSignalQuality(&rssi, &ber);

         // success or fail: dont care
         if (at_command_status == SIM800_OK || at_command_status == SIM800_ERROR) {
            sim800_setup_state = SIM800_SETUP_WAIT_NETWORK;
         }

         if (at_command_status != SIM800_BUSY) {
	   LOGN(F("SIM800 signal [ %s ] [ rssi %d, ber %d ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING), (int)rssi, (int)ber);
         }

         // wait
         break;

      case SIM800_SETUP_WAIT_NETWORK:
         is_registered = false;

         at_command_status = getNetworkStatus(&n, &stat);

         // success
         if (at_command_status == SIM800_OK) {
            switch (stat) {
               case 0:
                  is_registered = false;
                  LOGN(F("SIM800 NOT registered... [ %s ]"), ERROR_STRING);
                  break;

               case 1:
                  is_registered = true;
                  LOGN(F("SIM800 network registered... [ %s ]"), OK_STRING);
                  break;

               case 2:
                  is_registered = false;
                  LOGN(F("SIM800 searching network..."));
                  break;

               case 3:
                  is_registered = false;
                  LOGN(F("SIM800 network registration denied... [ %s ]"), ERROR_STRING);
                  break;

               case 4:
                  is_registered = false;
                  LOGN(F("SIM800 unknown network..."));
                  break;

               case 5:
                  is_registered = true;
                  LOGN(F("SIM800 network registered (roaming)... [ %s ]"), OK_STRING);
                  break;
            }
         }

         // success
         if (at_command_status == SIM800_OK && is_registered) {
            retry = 0;
            sim800_setup_state = SIM800_SETUP_END;
         }
         // retry
         else if (at_command_status == SIM800_OK && !is_registered && (++retry) < SIM800_WAIT_FOR_NETWORK_RETRY_COUNT_MAX) {
            start_time_ms = millis();
            delay_ms = SIM800_WAIT_FOR_NETWORK_DELAY_MS;
            state_after_wait = SIM800_SETUP_WAIT_NETWORK;
            sim800_setup_state = SIM800_SETUP_WAIT_STATE;
         }
         // fail
         else if (at_command_status == SIM800_ERROR || retry >= SIM800_WAIT_FOR_NETWORK_RETRY_COUNT_MAX) {
            retry = 0;
            is_error = true;
            sim800_setup_state = SIM800_SETUP_END;
         }
         // wait
         break;

      case SIM800_SETUP_END:
         if (is_error) {
            sim800_status = SIM800_ERROR;
            state &= ~SIM800_STATE_SETTED;
         }
         else {
            sim800_status = SIM800_OK;
            state |= SIM800_STATE_SETTED;
         }

         sim800_setup_state = SIM800_SETUP_INIT;
         LOGN(F("SIM800 setup... [ %s ]"), printStatus(sim800_status, OK_STRING, FAIL_STRING));
         break;

      case SIM800_SETUP_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            sim800_setup_state = state_after_wait;
         }
         break;
   }

   return sim800_status;
}

sim800_status_t SIM800::startConnection(const char *apn, const char *username, const char *password) {
   static uint8_t retry;
   static sim800_connection_start_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;
   static char ip[SIM800_IP_LENGTH];
   static sim800_status_t sim800_status;
   static bool is_error;
   static bool is_attached;
   sim800_status_t at_command_status;

   switch (sim800_connection_start_state) {

      case SIM800_CONNECTION_START_INIT:
         retry = 0;
         is_error = false;
         sim800_status = SIM800_BUSY;

         if (isSetted()) {
            sim800_connection_start_state = SIM800_CONNECTION_START_CHECK_GPRS;
         }
         else {
            is_error = true;
            sim800_connection_start_state = SIM800_CONNECTION_START_END;
         }
         break;

      case SIM800_CONNECTION_START_CHECK_GPRS:
         is_attached = false;
         at_command_status = isGprsAttached(&is_attached);

         // success
         if (at_command_status == SIM800_OK && is_attached) {
            retry = 0;
            sim800_connection_start_state = SIM800_CONNECTION_START_SINGLE_IP;
         }
         // gprs not attached
         else if (at_command_status == SIM800_OK && !is_attached) {
            retry = 0;
            delay_ms = SIM800_WAIT_FOR_ATTACH_GPRS_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SIM800_CONNECTION_START_ATTACH_GPRS;
            sim800_connection_start_state = SIM800_CONNECTION_START_WAIT_STATE;
         }
         // retry
         else if (at_command_status == SIM800_ERROR && (++retry) < SIM800_GENERIC_RETRY_COUNT_MAX) {
            delay_ms = SIM800_GENERIC_WAIT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SIM800_CONNECTION_START_CHECK_GPRS;
            sim800_connection_start_state = SIM800_CONNECTION_START_WAIT_STATE;
         }
         // fail
         else if (at_command_status == SIM800_ERROR || retry >= SIM800_GENERIC_RETRY_COUNT_MAX) {
            retry = 0;
            is_error = true;
            sim800_connection_start_state = SIM800_CONNECTION_START_END;
         }

         if (at_command_status != SIM800_BUSY) {
            LOGN(F("SIM800 GPRS attach... [ %s ] [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING), is_attached ? YES_STRING : NO_STRING);
         }

         // wait
         break;

      case SIM800_CONNECTION_START_ATTACH_GPRS:
         // attach GPRS
         at_command_status = sendAtCommand("AT+CGATT=1\r\n", buffer_ext, AT_OK_STRING, AT_ERROR_STRING, SIM800_CGATT_RESPONSE_TIME_MAX_MS);

         // success
         if (at_command_status == SIM800_OK) {
            retry = 0;
            sim800_connection_start_state = SIM800_CONNECTION_START_SINGLE_IP;
         }
         // retry
         else if (at_command_status == SIM800_ERROR && (++retry) < SIM800_GENERIC_RETRY_COUNT_MAX) {
            delay_ms = SIM800_GENERIC_WAIT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SIM800_CONNECTION_START_ATTACH_GPRS;
            sim800_connection_start_state = SIM800_CONNECTION_START_WAIT_STATE;
         }
         // fail
         else if (at_command_status == SIM800_ERROR || retry >= SIM800_GENERIC_RETRY_COUNT_MAX) {
            is_error = true;
            sim800_connection_start_state = SIM800_CONNECTION_START_END;
         }

         if (at_command_status != SIM800_BUSY) {
            LOGN(F("SIM800 attach GPRS... [ %s ]"), printStatus(at_command_status, OK_STRING, FAIL_STRING));
         }

         // wait...
         break;

      case SIM800_CONNECTION_START_SINGLE_IP:
         at_command_status = sendAtCommand("AT+CIPMUX=0\r\n", buffer_ext);

         // success
         if (at_command_status == SIM800_OK) {
            retry = 0;
            sim800_connection_start_state = SIM800_CONNECTION_START_TRANSPARENT_MODE;
         }
         // retry
         else if (at_command_status == SIM800_ERROR && (++retry) < SIM800_GENERIC_RETRY_COUNT_MAX) {
            delay_ms = SIM800_GENERIC_WAIT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SIM800_CONNECTION_START_SINGLE_IP;
            sim800_connection_start_state = SIM800_CONNECTION_START_WAIT_STATE;
         }
         // fail
         else if (at_command_status == SIM800_ERROR || retry >= SIM800_GENERIC_RETRY_COUNT_MAX) {
            is_error = true;
            sim800_connection_start_state = SIM800_CONNECTION_START_END;
         }

         if (at_command_status != SIM800_BUSY) {
            LOGN(F("SIM800 single IP mode... [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
         }

         // wait...
         break;

      case SIM800_CONNECTION_START_TRANSPARENT_MODE:
         at_command_status = sendAtCommand("AT+CIPMODE=1\r\n", buffer_ext);

         // success
         if (at_command_status == SIM800_OK) {
            retry = 0;
            sim800_connection_start_state = SIM800_CONNECTION_START_TRANSPARENT_MODE_CONFIG;
         }
         // retry
         else if (at_command_status == SIM800_ERROR && (++retry) < SIM800_GENERIC_RETRY_COUNT_MAX) {
            delay_ms = SIM800_GENERIC_WAIT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SIM800_CONNECTION_START_TRANSPARENT_MODE;
            sim800_connection_start_state = SIM800_CONNECTION_START_WAIT_STATE;
         }
         // fail
         else if (at_command_status == SIM800_ERROR || retry >= SIM800_GENERIC_RETRY_COUNT_MAX) {
            is_error = true;
            sim800_connection_start_state = SIM800_CONNECTION_START_END;
         }

         if (at_command_status != SIM800_BUSY) {
            LOGN(F("SIM800 switch to data mode... [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
         }

         // wait...
         break;

      // AT+CIPCCFG: (NmRetry:3-8),(WaitTm:2-10),(SendSz:1-1460),(esc:0,1) ,(Rxmode:0,1), (RxSize:50-1460),(Rxtimer:20-1000)
      // NmRetry:    Number of retries to be made for an IP packet.
      // WaitTm:     Number of 200ms intervals to wait for serial input before sending the packet
      // SendSz:     Size in uint8_ts of data block to be received from serial port before sending.
      // Esc:        Whether turn on the escape sequence, default is TRUE.
      // Rxmode:     Whether to set time interval during output data from serial port.
      // RxSize:     Output data length for each time, default value is 1460.
      // Rxtimer:    Time interval (ms) to wait for serial port to output data again. Default value: 50ms
      case SIM800_CONNECTION_START_TRANSPARENT_MODE_CONFIG:
         at_command_status = sendAtCommand("AT+CIPCCFG=8,2,1024,1,0,1460,50\r\n", buffer_ext);

         // success
         if (at_command_status == SIM800_OK) {
            retry = 0;
            sim800_connection_start_state = SIM800_CONNECTION_START_APN_USERNAME_PASSWORD;
         }
         // retry
         else if (at_command_status == SIM800_ERROR && (++retry) < SIM800_GENERIC_RETRY_COUNT_MAX) {
            delay_ms = SIM800_GENERIC_WAIT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SIM800_CONNECTION_START_TRANSPARENT_MODE_CONFIG;
            sim800_connection_start_state = SIM800_CONNECTION_START_WAIT_STATE;
         }
         // fail
         else if (at_command_status == SIM800_ERROR || retry >= SIM800_GENERIC_RETRY_COUNT_MAX) {
            is_error = true;
            sim800_connection_start_state = SIM800_CONNECTION_START_END;
         }

         if (at_command_status != SIM800_BUSY) {
            LOGN(F("SIM800 transparent mode... [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
         }

         // wait...
         break;

      case SIM800_CONNECTION_START_APN_USERNAME_PASSWORD:
         snprintf(buffer_ext2, SIM800_BUFFER_LENGTH, "AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n", apn, username, password);
         at_command_status = sendAtCommand(buffer_ext2, buffer_ext);

         // success
         if (at_command_status == SIM800_OK) {
            retry = 0;
            sim800_connection_start_state = SIM800_CONNECTION_START_CONNECT;
         }
         // retry
         else if (at_command_status == SIM800_ERROR && (++retry) < SIM800_GENERIC_RETRY_COUNT_MAX) {
            delay_ms = SIM800_GENERIC_WAIT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SIM800_CONNECTION_START_APN_USERNAME_PASSWORD;
            sim800_connection_start_state = SIM800_CONNECTION_START_WAIT_STATE;
         }
         // fail
         else if (at_command_status == SIM800_ERROR || retry >= SIM800_GENERIC_RETRY_COUNT_MAX) {
            is_error = true;
            sim800_connection_start_state = SIM800_CONNECTION_START_END;
         }

         if (at_command_status != SIM800_BUSY) {
            LOGN(F("SIM800 set APN, username and password... [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
         }

         // wait...
         break;

      case SIM800_CONNECTION_START_CONNECT:
         at_command_status = sendAtCommand("AT+CIICR\r\n", buffer_ext, AT_OK_STRING, AT_ERROR_STRING, SIM800_CIICR_RESPONSE_TIME_MAX_MS);

         // success
         if (at_command_status == SIM800_OK) {
            retry = 0;
            delay_ms = SIM800_GENERIC_WAIT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SIM800_CONNECTION_START_GET_IP;
            sim800_connection_start_state = SIM800_CONNECTION_START_WAIT_STATE;
         }
         // retry
         else if (at_command_status == SIM800_ERROR && (++retry) < SIM800_GENERIC_RETRY_COUNT_MAX) {
            delay_ms = SIM800_GENERIC_WAIT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SIM800_CONNECTION_START_CONNECT;
            sim800_connection_start_state = SIM800_CONNECTION_START_WAIT_STATE;
         }
         // fail
         else if (at_command_status == SIM800_ERROR || retry >= SIM800_GENERIC_RETRY_COUNT_MAX) {
            is_error = true;
            sim800_connection_start_state = SIM800_CONNECTION_START_END;
         }

         if (at_command_status != SIM800_BUSY) {
            LOGN(F("SIM800 setting up connection... [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
         }

         // wait...
         break;

      case SIM800_CONNECTION_START_GET_IP:
         at_command_status = getIp(ip);

         // success
         if (at_command_status == SIM800_OK) {
            retry = 0;
            sim800_connection_start_state = SIM800_CONNECTION_START_END;
         }
         // retry
         else if (at_command_status == SIM800_ERROR && (++retry) < SIM800_GENERIC_RETRY_COUNT_MAX) {
            delay_ms = SIM800_GENERIC_WAIT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SIM800_CONNECTION_START_GET_IP;
            sim800_connection_start_state = SIM800_CONNECTION_START_WAIT_STATE;
         }
         // fail
         else if (sim800_status == SIM800_ERROR || retry >= SIM800_GENERIC_RETRY_COUNT_MAX) {
            retry = 0;
            is_error = true;
            sim800_connection_start_state = SIM800_CONNECTION_START_END;
         }

         if (at_command_status != SIM800_BUSY) {
            LOGN(F("SIM800 IP... [ %s ] [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING), ip);
         }

         // wait...
         break;

      case SIM800_CONNECTION_START_END:
         sim800_status = (is_error ? SIM800_ERROR : SIM800_OK);
         sim800_connection_start_state = SIM800_CONNECTION_START_INIT;
         LOGN(F("SIM800 start connection... [ %s ]"), printStatus(sim800_status, OK_STRING, ERROR_STRING));
         break;

      case SIM800_CONNECTION_START_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            sim800_connection_start_state = state_after_wait;
         }
         break;
   }

   return sim800_status;
}

sim800_status_t SIM800::stopConnection() {
   static uint8_t retry;
   static sim800_connection_stop_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;
   static sim800_status_t sim800_status;
   static bool is_error;
   sim800_status_t at_command_status;

   switch (sim800_connection_stop_state) {

      case SIM800_CONNECTION_STOP_INIT:
         retry = 0;
         is_error = false;
         sim800_status = SIM800_BUSY;

         sim800_connection_stop_state = SIM800_CONNECTION_STOP_SWITCH_TO_COMMAND_MODE;
         break;

      case SIM800_CONNECTION_STOP_SWITCH_TO_COMMAND_MODE:
         at_command_status = exitTransparentMode();

         // success
         if (at_command_status == SIM800_OK) {
            retry = 0;
            sim800_connection_stop_state = SIM800_CONNECTION_STOP_CLOSE;
         }
         // retry
         else if (at_command_status == SIM800_ERROR && (++retry) < SIM800_GENERIC_RETRY_COUNT_MAX) {
            delay_ms = SIM800_GENERIC_WAIT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SIM800_CONNECTION_STOP_SWITCH_TO_COMMAND_MODE;
            sim800_connection_stop_state = SIM800_CONNECTION_STOP_WAIT_STATE;
         }
         // fail
         else if (sim800_status == SIM800_ERROR || retry >= SIM800_GENERIC_RETRY_COUNT_MAX) {
            retry = 0;
            is_error = true;
            sim800_connection_stop_state = SIM800_CONNECTION_STOP_END;
         }
         // wait
         break;

      case SIM800_CONNECTION_STOP_CLOSE:
         at_command_status = sendAtCommand("AT+CIPCLOSE=0\r\n", buffer_ext, AT_CIPCLOSE_OK_STRING, AT_CIPCLOSE_ERROR_STRING, SIM800_AT_DEFAULT_TIMEOUT_MS);

         // success
         if (at_command_status == SIM800_OK) {
            retry = 0;
            sim800_connection_stop_state = SIM800_CONNECTION_STOP_CLOSE_PDP;
         }
         // retry
         else if (at_command_status == SIM800_ERROR && (++retry) < SIM800_GENERIC_RETRY_COUNT_MAX) {
            delay_ms = SIM800_GENERIC_WAIT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SIM800_CONNECTION_STOP_CLOSE;
            sim800_connection_stop_state = SIM800_CONNECTION_STOP_WAIT_STATE;
         }
         // fail
         else if (sim800_status == SIM800_ERROR || retry >= SIM800_GENERIC_RETRY_COUNT_MAX) {
            is_error = true;
            sim800_connection_stop_state = SIM800_CONNECTION_STOP_END;
         }

         if (at_command_status != SIM800_BUSY) {
            LOGN(F("SIM800 stop connection... [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
         }

         // wait
         break;

      case SIM800_CONNECTION_STOP_CLOSE_PDP:
         at_command_status = sendAtCommand("AT+CIPSHUT\r\n", buffer_ext, AT_CIPSHUT_OK_STRING, AT_CIPSHUT_ERROR_STRING, SIM800_CIPSHUT_RESPONSE_TIME_MAX_MS);

         // success
         if (at_command_status == SIM800_OK) {
            retry = 0;
            sim800_connection_stop_state = SIM800_CONNECTION_STOP_DETACH_GPRS;
         }
         // retry
         else if (at_command_status == SIM800_ERROR && (++retry) < SIM800_GENERIC_RETRY_COUNT_MAX) {
            delay_ms = SIM800_GENERIC_WAIT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SIM800_CONNECTION_STOP_DETACH_GPRS;
            sim800_connection_stop_state = SIM800_CONNECTION_STOP_WAIT_STATE;
         }
         // fail
         else if (sim800_status == SIM800_ERROR || retry >= SIM800_GENERIC_RETRY_COUNT_MAX) {
            is_error = true;
            sim800_connection_stop_state = SIM800_CONNECTION_STOP_END;
         }

         if (at_command_status != SIM800_BUSY) {
            LOGN(F("SIM800 PDP close... [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
         }

         // wait
         break;

      case SIM800_CONNECTION_STOP_DETACH_GPRS:
         at_command_status = sendAtCommand("AT+CGATT=0\r\n", buffer_ext, AT_OK_STRING, AT_ERROR_STRING, SIM800_CGATT_RESPONSE_TIME_MAX_MS);

         // success
         if (at_command_status == SIM800_OK) {
            retry = 0;
            sim800_connection_stop_state = SIM800_CONNECTION_STOP_END;
         }
         // retry
         else if (at_command_status == SIM800_ERROR && (++retry) < SIM800_GENERIC_RETRY_COUNT_MAX) {
            delay_ms = SIM800_GENERIC_WAIT_DELAY_MS;
            start_time_ms = millis();
            state_after_wait = SIM800_CONNECTION_STOP_DETACH_GPRS;
            sim800_connection_stop_state = SIM800_CONNECTION_STOP_WAIT_STATE;
         }
         // fail
         else if (sim800_status == SIM800_ERROR || retry >= SIM800_GENERIC_RETRY_COUNT_MAX) {
            is_error = true;
            sim800_connection_stop_state = SIM800_CONNECTION_STOP_END;
         }

         if (at_command_status != SIM800_BUSY) {
            LOGN(F("SIM800 detach GPRS... [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
         }

         // wait...
         break;

      case SIM800_CONNECTION_STOP_END:
         sim800_status = (is_error ? SIM800_ERROR : SIM800_OK);
         sim800_connection_stop_state = SIM800_CONNECTION_STOP_INIT;
         LOGN(F("SIM800 stop connection... [ %s ]"), printStatus(sim800_status, OK_STRING, FAIL_STRING));
         break;

      case SIM800_CONNECTION_STOP_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            sim800_connection_stop_state = state_after_wait;
         }
         break;
   }

   return sim800_status;
}

// *****************************************************************************
// etherclient compatibility
// *****************************************************************************

sim800Client::sim800Client() {}

int sim800Client::connect(IPAddress ip, int port) {
   char server[16];
   sprintf(server, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
   return connection(SIM800_CONNECTION_TCP, server, port);
}

int sim800Client::connect(const char *server, int port) {
   return (int) connection(SIM800_CONNECTION_TCP, server, port);
}

uint8_t sim800Client::connected() {
   return isHttpInitialized();
}

int sim800Client::available() {
   return modem->available();
}

int sim800Client::read() {
   return modem->read();
}

int sim800Client::readBytes(char *buffer, size_t size) {
  return modem->readBytes(buffer, size);
}

int sim800Client::readBytes(uint8_t *buffer, size_t size) {
  return modem->readBytes(buffer, size);
}

void sim800Client::setTimeout(uint32_t timeout_ms) {
   modem->setTimeout(timeout_ms);
}

size_t sim800Client::write(uint8_t buffer) {
   return modem->write(buffer);
}

size_t sim800Client::write(const uint8_t *buffer, size_t size) {
   return modem->write(buffer, size);
}

void sim800Client::flush() {
}

void sim800Client::stop() {
}
