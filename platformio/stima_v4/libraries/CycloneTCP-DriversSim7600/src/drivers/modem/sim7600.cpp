/**@file sim7600.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
<http://www.gnu.org/licenses/>.
**********************************************************************/

#define TRACE_LEVEL SIM7600_TRACE_LEVEL

#include "sim7600.h"

SIM7600::SIM7600()
{
}

#ifndef USE_FREERTOS
SIM7600::SIM7600(HardwareSerial *serial, uint32_t _low_baud_rate, uint32_t _high_baud_rate, uint8_t _enable_power_pin, uint8_t _power_pin, uint8_t _ring_indicator_pin)
{
   modem = serial;
   low_baud_rate = _low_baud_rate;
   high_baud_rate = _high_baud_rate;
   enable_power_pin = _enable_power_pin;
   power_pin = _power_pin;
   ring_indicator_pin = _ring_indicator_pin;
   init();
}
#endif

#ifdef USE_FREERTOS
SIM7600::SIM7600(NetInterface *_interface, uint32_t _low_baud_rate, uint32_t _high_baud_rate, uint8_t _enable_power_pin, uint8_t _power_pin, uint8_t _ring_indicator_pin)
{
   interface = _interface;
   low_baud_rate = _low_baud_rate;
   high_baud_rate = _high_baud_rate;
   enable_power_pin = _enable_power_pin;
   power_pin = _power_pin;
   ring_indicator_pin = _ring_indicator_pin;
   init();
}
#endif

void SIM7600::initPins()
{
   pinMode(enable_power_pin, OUTPUT);
   pinMode(power_pin, OUTPUT);
   pinMode(ring_indicator_pin, INPUT);

   digitalWrite(enable_power_pin, LOW);
   digitalWrite(power_pin, LOW);
}

void SIM7600::init()
{
   state = SIM7600_STATE_NONE;
   sim7600_at_state = SIM7600_AT_INIT;
   sim7600_power_state = SIM7600_POWER_INIT;
   sim7600_power_off_state = SIM7600_POWER_OFF_INIT;
   sim7600_setup_state = SIM7600_SETUP_INIT;
   sim7600_connection_start_state = SIM7600_CONNECTION_START_INIT;
   sim7600_connection_state = SIM7600_CONNECTION_INIT;
   sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_INIT;
   sim7600_exit_transparent_mode_state = SIM7600_EXIT_TRANSPARENT_MODE_INIT;
   rssi = RSSI_UNKNOWN;
   ber = BER_UNKNOWN;
   creg_n = CREG_N_UNKNOWN;
   creg_stat = CREG_STAT_UNKNOWN;
   cgreg_n = CREG_N_UNKNOWN;
   cgreg_stat = CREG_STAT_UNKNOWN;
   cereg_n = CREG_N_UNKNOWN;
   cereg_stat = CREG_STAT_UNKNOWN;

   delay_ms = SIM7600_GENERIC_STATE_DELAY_MS;

   #ifndef USE_FREERTOS
   modem->begin(low_baud_rate);
   #else
   uartDeInit();
   uartInitConfig(low_baud_rate);
   #endif

   initPins();
}

bool SIM7600::isOn()
{
   return (state & SIM7600_STATE_ON);
}

bool SIM7600::isInitialized()
{
   return (state & SIM7600_STATE_INITIALIZED);
}

bool SIM7600::isSetted()
{
   return (state & SIM7600_STATE_SETTED);
}

bool SIM7600::isRegistered()
{
   return (state & SIM7600_STATE_REGISTERED);
}

// return true when switch on
sim7600_status_t SIM7600::switchOn()
{
   bool is_switching_on = true;
   sim7600_status_t at_command_status;

   at_command_status = switchModem(is_switching_on);

   if (at_command_status != SIM7600_BUSY) {
      TRACE_INFO_F(F("%s switching ON... [ %s ] [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), isOn() ? ON_STRING : OFF_STRING);
   }

   return at_command_status;
}

// return true when switch off
sim7600_status_t SIM7600::switchOff(uint8_t power_off_method)
{
   bool is_switching_on = false;
   sim7600_status_t at_command_status;

   #ifndef USE_FREERTOS
   static sim7600_power_off_state_t state_after_wait;
   static uint32_t start_time_ms;
   #endif

   delay_ms = SIM7600_GENERIC_STATE_DELAY_MS;

   switch (sim7600_power_off_state)
   {
   case SIM7600_POWER_OFF_INIT:
      if (power_off_method == SIM7600_POWER_OFF_BY_SWITCH)
      {
         at_command_status = switchModem(is_switching_on);
      }
      else if (power_off_method == SIM7600_POWER_OFF_BY_AT_COMMAND)
      {
         at_command_status = sendAtCommand("AT+CPOF\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);
      }

      if (at_command_status != SIM7600_BUSY)
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_POWER_OFF_END;
         sim7600_power_off_state = SIM7600_POWER_OFF_WAIT_STATE;
         #else
         delay_ms = SIM7600_WAIT_FOR_POWER_OFF_DELAY_MS;
         sim7600_power_off_state = SIM7600_POWER_OFF_END;
         #endif
         at_command_status = SIM7600_BUSY;
      }
      break;

   case SIM7600_POWER_OFF_END:
      digitalWrite(enable_power_pin, LOW);
      digitalWrite(power_pin, LOW);
      state = (sim7600_state_t)(state & ~SIM7600_STATE_INITIALIZED);
      state = (sim7600_state_t)(state & ~SIM7600_STATE_ON);
      TRACE_INFO_F(F("%s switching OFF... [ %s ] [ %s ]\r\n"), SIM7600_NAME, printStatus(SIM7600_OK, OK_STRING, ERROR_STRING), isOn() ? ON_STRING : OFF_STRING);
      sim7600_power_off_state = SIM7600_POWER_OFF_INIT;
      at_command_status = SIM7600_OK;
      break;

   #ifndef USE_FREERTOS
   case SIM7600_POWER_OFF_WAIT_STATE:
      if (millis() - start_time_ms > delay_ms)
      {
         sim7600_power_off_state = state_after_wait;
      }
      break;
   #endif
   }

   return at_command_status;
}

sim7600_status_t SIM7600::switchModem(bool is_switching_on)
{
   #ifndef USE_FREERTOS
   static sim7600_power_state_t state_after_wait;
   static uint32_t start_time_ms;
   #endif

   static uint8_t retry;
   static bool is_error;
   static sim7600_status_t sim7600_status;
   sim7600_status_t at_command_status;

   delay_ms = SIM7600_GENERIC_STATE_DELAY_MS;

   switch (sim7600_power_state)
   {
   case SIM7600_POWER_INIT:
      retry = 0;
      is_error = false;
      sim7600_status = SIM7600_BUSY;
      delay_ms = SIM7600_POWER_ON_STABILIZATION_DELAY_MS;

      if (is_switching_on)
      {
         sim7600_power_state = SIM7600_POWER_ENABLE;
         TRACE_VERBOSE_F(F("SIM7600_POWER_INIT -> SIM7600_POWER_ENABLE\r\n"));
      }
      else
      {
         sim7600_power_state = SIM7600_POWER_IMPULSE_DOWN;
         TRACE_VERBOSE_F(F("SIM7600_POWER_INIT -> SIM7600_POWER_IMPULSE_DOWN\r\n"));
      }
      break;

   case SIM7600_POWER_ENABLE:
      digitalWrite(enable_power_pin, HIGH);
      digitalWrite(power_pin, HIGH);
      delay_ms = SIM7600_POWER_ON_STABILIZATION_DELAY_MS;
      
      #ifndef USE_FREERTOS
      start_time_ms = millis();
      state_after_wait = SIM7600_POWER_IMPULSE_DOWN;
      sim7600_power_state = SIM7600_POWER_WAIT_STATE;
      #else
      sim7600_power_state = SIM7600_POWER_IMPULSE_DOWN;
      #endif
      TRACE_VERBOSE_F(F("SIM7600_POWER_ENABLE -> SIM7600_POWER_IMPULSE_DOWN\r\n"));
      break;

   case SIM7600_POWER_IMPULSE_DOWN:
      digitalWrite(power_pin, LOW);
      if (is_switching_on)
      {
         delay_ms = SIM7600_POWER_ON_IMPULSE_DELAY_MS;
      }
      else
      {
         delay_ms = SIM7600_POWER_OFF_IMPULSE_DELAY_MS;
      }

      #ifndef USE_FREERTOS
      start_time_ms = millis();
      state_after_wait = SIM7600_POWER_IMPULSE_UP;
      sim7600_power_state = SIM7600_POWER_WAIT_STATE;
      #else
      sim7600_power_state = SIM7600_POWER_IMPULSE_UP;
      #endif
      TRACE_VERBOSE_F(F("SIM7600_POWER_IMPULSE_DOWN -> SIM7600_POWER_IMPULSE_UP\r\n"));
      break;

   case SIM7600_POWER_IMPULSE_UP:
      digitalWrite(power_pin, HIGH);
      sim7600_power_state = SIM7600_POWER_CHECK_STATUS;
      TRACE_VERBOSE_F(F("SIM7600_POWER_IMPULSE_UP -> SIM7600_POWER_CHECK_STATUS\r\n"));
      break;

   case SIM7600_POWER_CHECK_STATUS:
      if (is_switching_on)
      {
         at_command_status = sendAtCommand("", buffer_ext, sizeof(buffer_ext), AT_PB_DONE_STRING, AT_ERROR_STRING, SIM7600_WAIT_FOR_POWER_CHANGE_DELAY_MS);
      }
      else
      {
         at_command_status = sendAtCommand("AT\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);
      }

      // success: switching ON and is ON
      if (is_switching_on && (at_command_status == SIM7600_OK))
      {
         state = (sim7600_state_t) (state | SIM7600_STATE_ON);
         sim7600_power_state = SIM7600_POWER_END;
         TRACE_VERBOSE_F(F("SIM7600_POWER_CHECK_STATUS -> SIM7600_POWER_END\r\n"));
      }
      // success: switching OFF and is OFF
      else if (!is_switching_on && (at_command_status == SIM7600_ERROR))
      {
         state = (sim7600_state_t) (state & ~SIM7600_STATE_INITIALIZED);
         state = (sim7600_state_t) (state & ~SIM7600_STATE_ON);
         sim7600_power_state = SIM7600_POWER_END;
         TRACE_VERBOSE_F(F("SIM7600_POWER_CHECK_STATUS -> SIM7600_POWER_END\r\n"));
      }
      // fail: switching ON and is OFF
      else if (is_switching_on && (at_command_status == SIM7600_ERROR))
      {
         is_error = true;
         state = (sim7600_state_t) (state & ~SIM7600_STATE_INITIALIZED);
         state = (sim7600_state_t) (state & ~SIM7600_STATE_ON);
         sim7600_power_state = SIM7600_POWER_END;
         TRACE_VERBOSE_F(F("SIM7600_POWER_CHECK_STATUS -> SIM7600_POWER_END\r\n"));
      }
      // fail: switching OFF and is ON
      else if (!is_switching_on && (at_command_status == SIM7600_OK))
      {
         is_error = true;
         state = (sim7600_state_t) (state | SIM7600_STATE_ON);
         sim7600_power_state = SIM7600_POWER_END;
         TRACE_VERBOSE_F(F("SIM7600_POWER_CHECK_STATUS -> SIM7600_POWER_END\r\n"));
      }
      // wait...
      break;

   case SIM7600_POWER_END:
      // success
      if (!is_error)
      {
         sim7600_status = SIM7600_OK;
         sim7600_power_state = SIM7600_POWER_INIT;
         TRACE_VERBOSE_F(F("SIM7600_POWER_END -> SIM7600_POWER_INIT\r\n"));
      }
      // retry
      else if ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX)
      {
         digitalWrite(enable_power_pin, LOW);
         digitalWrite(power_pin, LOW);
         is_error = false;
         sim7600_status = SIM7600_BUSY;

         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_POWER_ENABLE;
         sim7600_power_state = SIM7600_POWER_WAIT_STATE;
         #else
         sim7600_power_state = SIM7600_POWER_ENABLE;
         #endif
         TRACE_VERBOSE_F(F("SIM7600_POWER_END -> SIM7600_POWER_ENABLE\r\n"));
         delay_ms = SIM7600_POWER_ON_STABILIZATION_DELAY_MS;
      }
      // fail
      else
      {
         sim7600_status = SIM7600_ERROR;
         sim7600_power_state = SIM7600_POWER_INIT;
         TRACE_VERBOSE_F(F("SIM7600_POWER_END -> SIM7600_POWER_INIT\r\n"));
      }
      break;

   #ifndef USE_FREERTOS
   case SIM7600_POWER_WAIT_STATE:
      if (millis() - start_time_ms > delay_ms)
      {
         sim7600_power_state = state_after_wait;
      }
      break;
   #endif
   }

   return sim7600_status;
}

// uint8_t SIM7600::receive(char *rx_buffer, const char *at_ok_string, const char *at_error_string) {
//    memset(rx_buffer, 0, SIM7600_BUFFER_LENGTH);
//    uint8_t rx_buffer_length = 0;
//    bool is_responce_ok = false;
//    bool is_responce_error = false;
//    char *write_ptr = rx_buffer;
//    char readed_char;

//    uint32_t start_time_ms = millis();
// // check ms timeout: set to 50 for slow baud rate
//    while ((millis() - start_time_ms <= 5) && (rx_buffer_length < (SIM7600_BUFFER_LENGTH - 1)) && !is_responce_ok && !is_responce_error) {
//       if (modem->available()) {
//          start_time_ms = millis();

//          readed_char = modem->read();

//          if (readed_char == '\r' || readed_char == '\n') {
//             if (rx_buffer_length > 2) {
//                *write_ptr++ = ' ';
//                rx_buffer_length++;
//             }
//          }
//          else {
//             *write_ptr++ = readed_char;
//             rx_buffer_length++;

//             *write_ptr = '\0';

//             if (at_ok_string) {
//                is_responce_ok = found(rx_buffer, at_ok_string);
//             }

//             if (at_error_string && !is_responce_ok){
//                is_responce_error = found(rx_buffer, at_error_string);
//             }
//          }
//       }
//    }

//    if (rx_buffer[rx_buffer_length-2] == ' ') {
//       rx_buffer_length -= 2;
//       rx_buffer[rx_buffer_length] = '\0';
//    }

//    if (is_responce_error) {
//       rx_buffer_length = 0;
//    }

//    if (rx_buffer_length) {
//      TRACE_DEBUG_F(F("SIM7600<-- %s"), rx_buffer);
//    }

//    return rx_buffer_length;
// }

void SIM7600::cleanInput () {
   #ifndef USE_FREERTOS
   while (modem->available()) {
      modem->read();
   }
   #else
   #endif
}

sim7600_status_t SIM7600::sendAtCommand(const char *command, char *response, size_t response_length, const char *at_ok_string, const char *at_error_string, uint32_t timeout_ms)
{
   #ifndef USE_FREERTOS
   static uint32_t start_time_ms;
   static sim7600_at_state_t state_after_wait;
   static uint8_t rx_data_length;
   #else
   error_t error;
   static size_t n;
   #endif

   static sim7600_status_t sim7600_status;
   static bool is_error;

   delay_ms = SIM7600_GENERIC_STATE_DELAY_MS;

   switch (sim7600_at_state)
   {
      case SIM7600_AT_INIT:
         memset(response, 0, response_length);
         n = 0;
         is_error = false;
         sim7600_status = SIM7600_BUSY;

         #ifdef USE_FREERTOS
         pppSetTimeout(interface, timeout_ms);
         #endif

         if (strlen(command))
         {
            delay_ms = SIM7600_AT_DELAY_MS;

            #ifndef USE_FREERTOS
            rx_data_length = 0;
            start_time_ms = millis();
            state_after_wait = SIM7600_AT_SEND;
            sim7600_at_state = SIM7600_AT_WAIT_STATE;
            #else
            sim7600_at_state = SIM7600_AT_SEND;
            #endif
         }
         else
         {
            sim7600_at_state = SIM7600_AT_RECEIVE;
         }
         break;

      case SIM7600_AT_SEND:
         cleanInput();

         #ifndef USE_FREERTOS
         start_time_ms = 0;
         modem->print(command)
         sim7600_at_state = SIM7600_AT_RECEIVE;
         #else
         error = pppSendAtCommand(interface, command);
         
         if(!error)
         {
            n = 0;
            sim7600_at_state = SIM7600_AT_RECEIVE;
         }
         else
         {
            is_error = true;
            sim7600_at_state = SIM7600_AT_END;
         }
         #endif
         TRACE_DEBUG_F(F("%s%s"), SIM7600_AT_TX_CMD_DEBUG_PREFIX, command);
         break;

      case SIM7600_AT_RECEIVE:
         #ifndef USE_FREERTOS
         if (start_time_ms == 0) {
            start_time_ms = millis();
         }

         rx_data_length = receive(response, at_ok_string, at_error_string);
         TRACE_DEBUG_F(F("%s%s\r\n"), SIM7600_AT_RX_CMD_DEBUG_PREFIX, response);

         // success
         if (rx_data_length) {
            sim7600_at_state = SIM7600_AT_END;
         }
         // timeout elapsed
         else if (millis() - start_time_ms > timeout_ms) {
            is_error = true;
            sim7600_at_state = SIM7600_AT_END;
         }
         // wait...
         #else
         if (n < response_length)
         {
            // Wait for a response from the modem
            error = pppReceiveAtCommand(interface, response + n, response_length - n);

            // ok
            if (!error && found(response, at_ok_string))
            {
               TRACE_DEBUG_F(F("%s%s\r\n"), SIM7600_AT_RX_CMD_DEBUG_PREFIX, response);
               sim7600_at_state = SIM7600_AT_END;
               break;
            }
            // error
            else if (error || (!error && found(response, at_error_string)))
            {
               if (n) {
                  TRACE_DEBUG_F(F("%s%s\r\n"), SIM7600_AT_RX_CMD_DEBUG_PREFIX, response);
               }
               is_error = true;
               sim7600_at_state = SIM7600_AT_END;
               break;
            }
            // wait...
            else
            {
               n = strlen(response);
            }
         }
         else
         {
            TRACE_DEBUG_F(F("%s%s\r\n"), SIM7600_AT_RX_CMD_DEBUG_PREFIX, response);
            is_error = true;
            sim7600_at_state = SIM7600_AT_END;
         }
         #endif
         break;

      case SIM7600_AT_END:
         sim7600_status = (is_error ? SIM7600_ERROR : SIM7600_OK);
         sim7600_at_state = SIM7600_AT_INIT;
         break;

      #ifndef USE_FREERTOS
      case SIM7600_AT_WAIT_STATE:
         if (millis() - start_time_ms > delay_ms) {
            sim7600_at_state = state_after_wait;
         }
         break;
      #endif
   }

   return sim7600_status;
}

uint32_t SIM7600::getDelayMs()
{
   return delay_ms;
}

void SIM7600::setPins(uint8_t _enable_power_pin, uint8_t _power_pin, uint8_t _ring_indicator_pin)
{
   enable_power_pin = _enable_power_pin;
   power_pin = _power_pin;
   ring_indicator_pin = _ring_indicator_pin;
}

#ifndef USE_FREERTOS
void SIM7600::setSerial(HardwareSerial *serial, uint32_t _low_baud_rate, uint32_t _high_baud_rate)
{
   modem = serial;
   low_baud_rate = _low_baud_rate;
   high_baud_rate = _high_baud_rate;
}
#endif

#ifdef USE_FREERTOS
void SIM7600::setInterface(NetInterface *_interface)
{
   interface = _interface;
}
#endif

// sim7600_status_t SIM7600::getGsn(char *imei) {
//    sim7600_status_t at_command_status;

//    if (!isInitialized())
//       return SIM7600_ERROR;

//    at_command_status = sendAtCommand("AT+GSN\r\n", buffer_ext);

//    if (at_command_status == SIM7600_OK) {
//       if (sscanf(buffer_ext, "%s", imei) != 1) {
//          at_command_status = SIM7600_ERROR;
//       }
//    }

//    if (at_command_status != SIM7600_BUSY) {
//       TRACE_VERBOSE_F(F("SIM7600 IMEI [ %s ] [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING), imei);
//    }

//    return at_command_status;
// }

sim7600_status_t SIM7600::sendAtCxreg(uint8_t cxreg_mode)
{
   sim7600_status_t at_command_status;

   if (cxreg_mode == SIM7600_AT_CREG_MODE)
   {
      at_command_status = sendAtCommand("AT+CREG=2", buffer_ext, sizeof(buffer_ext), "+CREG", AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);
   }
   else if (cxreg_mode == SIM7600_AT_CGREG_MODE)
   {
      at_command_status = sendAtCommand("AT+CGREG=2", buffer_ext, sizeof(buffer_ext), "+CGREG", AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);
   }
   else if (cxreg_mode == SIM7600_AT_CEREG_MODE)
   {
      at_command_status = sendAtCommand("AT+CEREG=2", buffer_ext, sizeof(buffer_ext), "+CEREG", AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);
   }

   if ((at_command_status == SIM7600_OK) && (cxreg_mode == SIM7600_AT_CREG_MODE))
   {
      if (sscanf(buffer_ext, "+CREG: %d,%d", &creg_n, &creg_stat) != 2)
      {
         at_command_status == SIM7600_ERROR;
      }
   }
   else if ((at_command_status == SIM7600_OK) && (cxreg_mode == SIM7600_AT_CGREG_MODE))
   {
      if (sscanf(buffer_ext, "+CGREG: %d,%d", &cgreg_n, &cgreg_stat) != 2)
      {
         at_command_status == SIM7600_ERROR;
      }
   }
   else if ((at_command_status == SIM7600_OK) && (cxreg_mode == SIM7600_AT_CEREG_MODE))
   {
      if (sscanf(buffer_ext, "+CEREG: %d,%d", &cereg_n, &cereg_stat) != 2)
      {
         at_command_status == SIM7600_ERROR;
      }
   }

   if ((at_command_status == SIM7600_ERROR) && (cxreg_mode == SIM7600_AT_CREG_MODE))
   {
      creg_n = CREG_N_UNKNOWN;
      creg_stat = CREG_STAT_UNKNOWN;
   }
   else if ((at_command_status == SIM7600_ERROR) && (cxreg_mode == SIM7600_AT_CREG_MODE))
   {
      cgreg_n = CREG_N_UNKNOWN;
      cgreg_stat = CREG_STAT_UNKNOWN;
   }
   else if ((at_command_status == SIM7600_ERROR) && (cxreg_mode == SIM7600_AT_CREG_MODE))
   {
      cereg_n = CREG_N_UNKNOWN;
      cereg_stat = CREG_STAT_UNKNOWN;
   }

   if ((at_command_status != SIM7600_BUSY) && (cxreg_mode == SIM7600_AT_CREG_MODE))
   {
      TRACE_VERBOSE_F(F("%s +CREG [ %s ] [ %d,%d ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), creg_n, creg_stat);
   }
   else if ((at_command_status != SIM7600_BUSY) && (cxreg_mode == SIM7600_AT_CGREG_MODE))
   {
      TRACE_VERBOSE_F(F("%s +CGREG [ %s ] [ %d,%d ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), cgreg_n, cgreg_stat);
   }
   else if ((at_command_status != SIM7600_BUSY) && (cxreg_mode == SIM7600_AT_CEREG_MODE))
   {
      TRACE_VERBOSE_F(F("%s +CEREG [ %s ] [ %d,%d ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), cereg_n, cereg_stat);
   }

   return at_command_status;
}

sim7600_status_t SIM7600::sendAtCsq()
{
   sim7600_status_t at_command_status;

   at_command_status = sendAtCommand("AT+CSQ\r\n", buffer_ext, sizeof(buffer_ext), "+CSQ:", AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

   if (at_command_status == SIM7600_OK)
   {
      if (sscanf(buffer_ext, "+CSQ: %d,%d", &rssi, &ber) != 2)
      {
         at_command_status == SIM7600_ERROR;
      }
   }

   if (at_command_status == SIM7600_ERROR)
   {
      rssi = RSSI_UNKNOWN;
      ber = BER_UNKNOWN;
   }

   if (at_command_status != SIM7600_BUSY) {
      TRACE_VERBOSE_F(F("%s CSQ [ %s ] [ %d,%d ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), rssi, ber);
   }

   return at_command_status;
}



// void SIM7600::getLastCsq(uint8_t *rssi, uint8_t *ber) {
//   *rssi=rssi;
//   *ber=ber;
// }


// sim7600_status_t SIM7600::getCgatt(bool *is_attached) {
//    sim7600_status_t at_command_status;

//    unsigned int is_gprs_attached = 0;

//    if (!isInitialized())
//       return SIM7600_ERROR;

//    at_command_status = sendAtCommand("AT+CGATT?\r\n", buffer_ext);

//    if (at_command_status == SIM7600_OK) {
//       if (sscanf(buffer_ext, "+CGATT: %d", &is_gprs_attached) != 1) {
//          at_command_status = SIM7600_ERROR;
// 	 is_gprs_attached=0;
//       }
//    }
//    *is_attached = (bool) is_gprs_attached;

//    if (at_command_status != SIM7600_BUSY) {
//       TRACE_VERBOSE_F(F("SIM7600 CGATT [ %s ] [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING), *is_attached ? YES_STRING : NO_STRING);
//    }

//    return at_command_status;
// }

// sim7600_status_t SIM7600::getCifsr(char *ip) {
//    sim7600_status_t at_command_status;

//    if (!isInitialized())
//       return SIM7600_ERROR;

//    at_command_status = sendAtCommand("AT+CIFSR\r\n", buffer_ext);

//    if (at_command_status == SIM7600_OK) {
//       if (sscanf(buffer_ext, "%s", ip) != 1) {
//          at_command_status = SIM7600_ERROR;
//          strcpy(ip, "0.0.0.0");
//       }
//    }

//    return at_command_status;
// }

// sim7600_status_t SIM7600::exitTransparentMode() {
//    static uint32_t start_time_ms;
//    static sim7600_exit_transparent_mode_state_t state_after_wait;
//    static sim7600_status_t sim7600_status;
//    static sim7600_status_t at_command_status;
//    static bool is_error;

//    switch (sim7600_exit_transparent_mode_state) {

//       case SIM7600_EXIT_TRANSPARENT_MODE_INIT:
//          is_error = false;
//          sim7600_status = SIM7600_BUSY;

//          // wait 1 second
//          delay_ms = SIM7600_WAIT_FOR_EXIT_TRANSPARENT_MODE_DELAY_MS;
//          start_time_ms = millis();
//          state_after_wait = SIM7600_EXIT_TRANSPARENT_MODE_SEND_ESCAPE_SEQUENCE;
//          sim7600_exit_transparent_mode_state = SIM7600_EXIT_TRANSPARENT_MODE_WAIT_STATE;
//          break;

//       case SIM7600_EXIT_TRANSPARENT_MODE_SEND_ESCAPE_SEQUENCE:
//          at_command_status = sendAtCommand("+++", buffer_ext);

//          // success
//          if (at_command_status == SIM7600_OK) {
//             // wait 1 second
//             delay_ms = SIM7600_WAIT_FOR_EXIT_TRANSPARENT_MODE_DELAY_MS;
//             start_time_ms = millis();
//             state_after_wait = SIM7600_EXIT_TRANSPARENT_MODE_END;
//             sim7600_exit_transparent_mode_state = SIM7600_EXIT_TRANSPARENT_MODE_WAIT_STATE;
//          }
//          else if (at_command_status == SIM7600_ERROR) {
//             is_error = true;
//             sim7600_exit_transparent_mode_state = SIM7600_EXIT_TRANSPARENT_MODE_END;
//          }
//          // wait...
//          break;

//       case SIM7600_EXIT_TRANSPARENT_MODE_END:
//          sim7600_status = (is_error ? SIM7600_ERROR : SIM7600_OK);
//          sim7600_exit_transparent_mode_state = SIM7600_EXIT_TRANSPARENT_MODE_INIT;
//          TRACE_VERBOSE_F(F("SIM7600 switch to command mode [ %s ]"), sim7600_status == SIM7600_OK ? OK_STRING : FAIL_STRING);
//          break;

//       case SIM7600_EXIT_TRANSPARENT_MODE_WAIT_STATE:
//          if (millis() - start_time_ms > delay_ms) {
//             sim7600_exit_transparent_mode_state = state_after_wait;
//          }
//          break;
//    }

//    return sim7600_status;
// }

// sim7600_status_t SIM7600::connection(const char *tipo, const char *server, const int port) {
//    static uint32_t start_time_ms;
//    static sim7600_connection_state_t state_after_wait;
//    static sim7600_status_t sim7600_status;
//    static sim7600_status_t at_command_status;
//    static bool is_error;
//    static uint8_t rx_data_length;

//    switch (sim7600_connection_state) {

//       case SIM7600_CONNECTION_INIT:
//          rx_data_length = 0;
//          is_error = false;
//          sim7600_status = SIM7600_BUSY;

//          if (isInitialized()) {
//             start_time_ms = millis();
//             delay_ms = SIM7600_WAIT_FOR_CONNECTION_DELAY_MS;
//             sim7600_connection_state = SIM7600_CONNECTION_WAIT_STATE;
//             state_after_wait = SIM7600_CONNECTION_OPEN;
//             snprintf(buffer_ext2, SIM7600_BUFFER_LENGTH, "AT+CIPSTART=\"%s\",\"%s\",\"%i\"\r\n", tipo, server, port);
//          }
//          else {
//             is_error = true;
//             sim7600_connection_state = SIM7600_CONNECTION_END;
//          }

//          break;

//       case SIM7600_CONNECTION_OPEN:
//          at_command_status = sendAtCommand(buffer_ext2, buffer_ext, AT_OK_STRING, AT_ERROR_STRING, SIM7600_CIPSTART_RESPONSE_TIME_MAX_MS);

//          // success
//          if (at_command_status == SIM7600_OK) {
//             sim7600_connection_state = SIM7600_CONNECTION_CHECK_STATUS;
//          }
//          // fail
//          else if (at_command_status == SIM7600_ERROR) {
//             is_error = true;
//             sim7600_connection_state = SIM7600_CONNECTION_END;
//          }
//          // wait...
//          break;

//       case SIM7600_CONNECTION_CHECK_STATUS:
//          if (start_time_ms == 0) {
//             start_time_ms = millis();
//          }

//          rx_data_length = receive(buffer_ext, NULL, NULL);

//          // fail
//          if (rx_data_length && found(buffer_ext, AT_CONNECT_FAIL_STRING)) {
//             LOGE(F("SIM7600 %s status... [ %s ] [ %s ]"), tipo, ERROR_STRING, buffer_ext);
//             is_error = true;
//             sim7600_connection_state = SIM7600_CONNECTION_END;
//          }
//          // success
//          else if (rx_data_length && found(buffer_ext, AT_CONNECT_OK_STRING)) {
//             TRACE_INFO_F(F("SIM7600 %s status... [ %s ] [ %s ]"), tipo, OK_STRING, buffer_ext);
//             sim7600_connection_state = SIM7600_CONNECTION_END;
//          }
//          // timeout fail
//          else if (millis() - start_time_ms > SIM7600_CIPSTART_RESPONSE_TIME_MAX_MS) {
//             LOGE(F("SIM7600 %s status... [ %s ]"), tipo, ERROR_STRING);
//             is_error = true;
//             sim7600_connection_state = SIM7600_CONNECTION_END;
//          }
//          // wait...
//          break;

//       case SIM7600_CONNECTION_END:
//          sim7600_status = (is_error ? SIM7600_ERROR : SIM7600_OK);
//          sim7600_connection_state = SIM7600_CONNECTION_INIT;
//          break;

//       case SIM7600_CONNECTION_WAIT_STATE:
//          if (millis() - start_time_ms > delay_ms) {
//             sim7600_connection_state = state_after_wait;
//          }
//          break;
//    }

//    return sim7600_status;
// }

sim7600_status_t SIM7600::setup()
{
   static uint8_t retry;
   static sim7600_status_t sim7600_status;
   static bool is_error;
   static bool is_registered;
   sim7600_status_t at_command_status;
   static uint8_t cxreg_mode;
   uint8_t stat = 0;

   #ifndef USE_FREERTOS
   static sim7600_setup_state_t state_after_wait;
   static uint32_t start_time_ms;
   #endif

   delay_ms = SIM7600_GENERIC_STATE_DELAY_MS;

   switch (sim7600_setup_state)
   {
   case SIM7600_SETUP_INIT:
      retry = 0;
      is_error = false;
      cxreg_mode = SIM7600_AT_CREG_MODE;
      sim7600_status = SIM7600_BUSY;

      if (isOn())
      {
         sim7600_setup_state = SIM7600_SETUP_RESET;
      }
      else
      {
         is_error = true;
         sim7600_setup_state = SIM7600_SETUP_END;
      }
      break;

   case SIM7600_SETUP_RESET:
      // reset to factory default
      at_command_status = sendAtCommand("AT&F\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

      // success
      if (at_command_status == SIM7600_OK)
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_SETUP_ECHO_MODE;
         sim7600_setup_state = SIM7600_SETUP_WAIT_STATE;
         #else
         sim7600_setup_state = SIM7600_SETUP_ECHO_MODE;
         #endif
         // delay_ms = SIM7600_WAIT_FOR_SETUP_DELAY_MS;
      }
      // fail
      else if (at_command_status == SIM7600_ERROR)
      {
         is_error = true;
         sim7600_setup_state = SIM7600_SETUP_END;
      }

      if (at_command_status != SIM7600_BUSY)
      {
         TRACE_INFO_F(F("%s reset to factory default [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
      }
      break;

   case SIM7600_SETUP_ECHO_MODE:
      // echo mode off
      at_command_status = sendAtCommand("ATE0\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

      // success
      if (at_command_status == SIM7600_OK)
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_SETUP_CHANGE_BAUD_RATE;
         sim7600_setup_state = SIM7600_SETUP_WAIT_STATE;
         #else
         sim7600_setup_state = SIM7600_SETUP_CHANGE_BAUD_RATE;
         #endif
      }
      // fail
      else if (at_command_status == SIM7600_ERROR)
      {
         is_error = true;
         sim7600_setup_state = SIM7600_SETUP_END;
      }

      if (at_command_status != SIM7600_BUSY)
      {
         TRACE_INFO_F(F("%s echo mode off [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
      }

      // wait...
      break;
   
   case SIM7600_SETUP_CHANGE_BAUD_RATE:
      // change baud rate
      snprintf(buffer_ext2, sizeof(buffer_ext2), "AT+IPR=%d\r\n", high_baud_rate);
      at_command_status = sendAtCommand(buffer_ext2, buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

      // success
      if (at_command_status == SIM7600_OK)
      {
         uartDeInit();
         uartInitConfig(high_baud_rate);
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_SETUP_SET_PHONE_FUNCTIONALITY;
         sim7600_setup_state = SIM7600_SETUP_WAIT_STATE;
         #else
         sim7600_setup_state = SIM7600_SETUP_SET_PHONE_FUNCTIONALITY;
         #endif
         delay_ms = SIM7600_WAIT_FOR_UART_RECONFIGURE_DELAY_MS;
      }
      // fail
      else if (at_command_status == SIM7600_ERROR)
      {
         is_error = true;
         sim7600_setup_state = SIM7600_SETUP_END;
      }

      if (at_command_status != SIM7600_BUSY)
      {
         TRACE_INFO_F(F("%s change baud rate to [ %d ] [ %s ]\r\n"), SIM7600_NAME, high_baud_rate, printStatus(at_command_status, OK_STRING, ERROR_STRING));
      }

      // wait...
      break;

   case SIM7600_SETUP_SET_PHONE_FUNCTIONALITY:
      at_command_status = sendAtCommand("AT+CFUN=1\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

      // success
      if (at_command_status == SIM7600_OK)
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_SETUP_GET_SIGNAL_QUALITY;
         sim7600_setup_state = SIM7600_SETUP_WAIT_STATE;
         #else
         sim7600_setup_state = SIM7600_SETUP_GET_SIGNAL_QUALITY;
         #endif
         delay_ms = SIM7600_WAIT_FOR_GET_SIGNAL_QUALITY_DELAY_MS;
      }
      // fail
      else if (at_command_status == SIM7600_ERROR)
      {
         is_error = true;
         sim7600_setup_state = SIM7600_SETUP_END;
      }

      if (at_command_status != SIM7600_BUSY)
      {
         TRACE_INFO_F(F("%s set full phone functionallity [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
      }

      // wait
      break;

   case SIM7600_SETUP_GET_SIGNAL_QUALITY:
      at_command_status = sendAtCsq();

      // success or fail: dont care
      if (at_command_status == SIM7600_OK || at_command_status == SIM7600_ERROR)
      {
         sim7600_setup_state = SIM7600_SETUP_WAIT_NETWORK;
      }

      if (at_command_status != SIM7600_BUSY)
      {
         TRACE_INFO_F(F("%s signal [ %s ] [ rssi %d, ber %d ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), rssi, ber);
      }

      // wait
      break;

   case SIM7600_SETUP_WAIT_NETWORK:
      is_registered = false;

      at_command_status = sendAtCxreg(cxreg_mode);

      // success
      if (at_command_status == SIM7600_OK)
      {
         if (cxreg_mode == SIM7600_AT_CREG_MODE)
         {
            stat = creg_stat;
         }
         else if (cxreg_mode == SIM7600_AT_CGREG_MODE)
         {
            stat = cgreg_stat;
         }
         else if (cxreg_mode == SIM7600_AT_CEREG_MODE)
         {
            stat = cereg_stat;
         }

         switch (stat)
         {
         case 0:
            is_registered = false;
            TRACE_INFO_F(F("%s NOT registered... [ %s ]\r\n"), SIM7600_NAME, ERROR_STRING);
            break;

         case 1:
            is_registered = true;
            TRACE_INFO_F(F("%s network registered... [ %s ]\r\n"), SIM7600_NAME, OK_STRING);
            break;

         case 2:
            is_registered = false;
            TRACE_INFO_F(F("%s searching network...\r\n"), SIM7600_NAME);
            break;

         case 3:
            is_registered = false;
            TRACE_INFO_F(F("%s network registration denied... [ %s ]\r\n"), SIM7600_NAME, ERROR_STRING);
            break;

         case 4:
            is_registered = false;
            TRACE_INFO_F(F("%s unknown network...\r\n"));
            break;

         case 5:
            is_registered = true;
            TRACE_INFO_F(F("%s network registered (roaming)... [ %s ]\r\n"), SIM7600_NAME, OK_STRING);
            break;

         case 6:
            is_registered = false;
            TRACE_INFO_F(F("%s network registered only for SMS... [ %s ]\r\n"), SIM7600_NAME, ERROR_STRING);
            break;

         case 7:
            is_registered = false;
            TRACE_INFO_F(F("%s network registered (roaming) only for SMS... [ %s ]\r\n"), SIM7600_NAME, ERROR_STRING);
            break;

         case 8:
            is_registered = false;
            TRACE_INFO_F(F("%s network registered only for emergency... [ %s ]\r\n"), SIM7600_NAME, ERROR_STRING);
            break;
         }
      }

      // success
      if ((at_command_status == SIM7600_OK) && is_registered)
      {
         retry = 0;
         cxreg_mode++;
         if (cxreg_mode > SIM7600_AT_CEREG_MODE)
         {
            sim7600_setup_state = SIM7600_SETUP_END;
         }
      }
      // retry
      else if ((at_command_status == SIM7600_OK) && !is_registered && (++retry) < SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX)
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_SETUP_WAIT_NETWORK;
         sim7600_setup_state = SIM7600_SETUP_WAIT_STATE;
         #else
         sim7600_setup_state = SIM7600_SETUP_WAIT_NETWORK;
         #endif
         delay_ms = SIM7600_WAIT_FOR_NETWORK_DELAY_MS;
      }
      // fail
      else if ((at_command_status == SIM7600_ERROR) || (retry >= SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX))
      {
         retry = 0;
         cxreg_mode++;
         if (cxreg_mode > SIM7600_AT_CEREG_MODE)
         {
            is_error = true;
            sim7600_setup_state = SIM7600_SETUP_END;
         }
      }
      // wait
      break;

   case SIM7600_SETUP_END:
      if (is_error)
      {
         sim7600_status = SIM7600_ERROR;
         state = (sim7600_state_t)(state & ~SIM7600_STATE_SETTED);
      }
      else
      {
         sim7600_status = SIM7600_OK;
         state = (sim7600_state_t)(state | SIM7600_STATE_SETTED);
      }

      sim7600_setup_state = SIM7600_SETUP_INIT;
      TRACE_INFO_F(F("%s setup... [ %s ]\r\n"), SIM7600_NAME, printStatus(sim7600_status, OK_STRING, FAIL_STRING));
      break;

   #ifndef USE_FREERTOS
   case SIM7600_SETUP_WAIT_STATE:
      if (millis() - start_time_ms > delay_ms)
      {
         sim7600_setup_state = state_after_wait;
      }
      break;
   #endif
   }

   return sim7600_status;
}

// sim7600_status_t SIM7600::startConnection(const char *apn, const char *username, const char *password) {
//    static uint8_t retry;
//    static sim7600_connection_start_state_t state_after_wait;
//    static uint32_t delay_ms;
//    static uint32_t start_time_ms;
//    static char ip[SIM7600_IP_LENGTH];
//    static sim7600_status_t sim7600_status;
//    static bool is_error;
//    static bool is_attached;
//    sim7600_status_t at_command_status;

//    switch (sim7600_connection_start_state) {

//       case SIM7600_CONNECTION_START_INIT:
//          retry = 0;
//          is_error = false;
//          sim7600_status = SIM7600_BUSY;

//          if (isSetted()) {
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_CHECK_GPRS;
//          }
//          else {
//             is_error = true;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_END;
//          }
//          break;

//       case SIM7600_CONNECTION_START_CHECK_GPRS:
//          is_attached = false;
//          at_command_status = isGprsAttached(&is_attached);

//          // success
//          if (at_command_status == SIM7600_OK && is_attached) {
//             retry = 0;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_SINGLE_IP;
//          }
//          // gprs not attached
//          else if (at_command_status == SIM7600_OK && !is_attached) {
//             retry = 0;
//             delay_ms = SIM7600_WAIT_FOR_ATTACH_GPRS_DELAY_MS;
//             start_time_ms = millis();
//             state_after_wait = SIM7600_CONNECTION_START_ATTACH_GPRS;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_WAIT_STATE;
//          }
//          // retry
//          else if (at_command_status == SIM7600_ERROR && (++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
//             start_time_ms = millis();
//             state_after_wait = SIM7600_CONNECTION_START_CHECK_GPRS;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_WAIT_STATE;
//          }
//          // fail
//          else if (at_command_status == SIM7600_ERROR || retry >= SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             retry = 0;
//             is_error = true;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_END;
//          }

//          if (at_command_status != SIM7600_BUSY) {
//             TRACE_INFO_F(F("SIM7600 GPRS attach... [ %s ] [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING), is_attached ? YES_STRING : NO_STRING);
//          }

//          // wait
//          break;

//       case SIM7600_CONNECTION_START_ATTACH_GPRS:
//          // attach GPRS
//          at_command_status = sendAtCommand("AT+CGATT=1\r\n", buffer_ext, AT_OK_STRING, AT_ERROR_STRING, SIM7600_CGATT_RESPONSE_TIME_MAX_MS);

//          // success
//          if (at_command_status == SIM7600_OK) {
//             retry = 0;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_SINGLE_IP;
//          }
//          // retry
//          else if (at_command_status == SIM7600_ERROR && (++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
//             start_time_ms = millis();
//             state_after_wait = SIM7600_CONNECTION_START_ATTACH_GPRS;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_WAIT_STATE;
//          }
//          // fail
//          else if (at_command_status == SIM7600_ERROR || retry >= SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             is_error = true;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_END;
//          }

//          if (at_command_status != SIM7600_BUSY) {
//             TRACE_INFO_F(F("SIM7600 attach GPRS... [ %s ]"), printStatus(at_command_status, OK_STRING, FAIL_STRING));
//          }

//          // wait...
//          break;

//       case SIM7600_CONNECTION_START_SINGLE_IP:
//          at_command_status = sendAtCommand("AT+CIPMUX=0\r\n", buffer_ext);

//          // success
//          if (at_command_status == SIM7600_OK) {
//             retry = 0;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_TRANSPARENT_MODE;
//          }
//          // retry
//          else if (at_command_status == SIM7600_ERROR && (++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
//             start_time_ms = millis();
//             state_after_wait = SIM7600_CONNECTION_START_SINGLE_IP;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_WAIT_STATE;
//          }
//          // fail
//          else if (at_command_status == SIM7600_ERROR || retry >= SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             is_error = true;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_END;
//          }

//          if (at_command_status != SIM7600_BUSY) {
//             TRACE_INFO_F(F("SIM7600 single IP mode... [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
//          }

//          // wait...
//          break;

//       case SIM7600_CONNECTION_START_TRANSPARENT_MODE:
//          at_command_status = sendAtCommand("AT+CIPMODE=1\r\n", buffer_ext);

//          // success
//          if (at_command_status == SIM7600_OK) {
//             retry = 0;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_TRANSPARENT_MODE_CONFIG;
//          }
//          // retry
//          else if (at_command_status == SIM7600_ERROR && (++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
//             start_time_ms = millis();
//             state_after_wait = SIM7600_CONNECTION_START_TRANSPARENT_MODE;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_WAIT_STATE;
//          }
//          // fail
//          else if (at_command_status == SIM7600_ERROR || retry >= SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             is_error = true;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_END;
//          }

//          if (at_command_status != SIM7600_BUSY) {
//             TRACE_INFO_F(F("SIM7600 switch to data mode... [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
//          }

//          // wait...
//          break;

//       // AT+CIPCCFG: (NmRetry:3-8),(WaitTm:2-10),(SendSz:1-1460),(esc:0,1) ,(Rxmode:0,1), (RxSize:50-1460),(Rxtimer:20-1000)
//       // NmRetry:    Number of retries to be made for an IP packet.
//       // WaitTm:     Number of 200ms intervals to wait for serial input before sending the packet
//       // SendSz:     Size in uint8_ts of data block to be received from serial port before sending.
//       // Esc:        Whether turn on the escape sequence, default is TRUE.
//       // Rxmode:     Whether to set time interval during output data from serial port.
//       // RxSize:     Output data length for each time, default value is 1460.
//       // Rxtimer:    Time interval (ms) to wait for serial port to output data again. Default value: 50ms
//       case SIM7600_CONNECTION_START_TRANSPARENT_MODE_CONFIG:
//          at_command_status = sendAtCommand("AT+CIPCCFG=8,2,1024,1,0,1460,50\r\n", buffer_ext);

//          // success
//          if (at_command_status == SIM7600_OK) {
//             retry = 0;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_APN_USERNAME_PASSWORD;
//          }
//          // retry
//          else if (at_command_status == SIM7600_ERROR && (++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
//             start_time_ms = millis();
//             state_after_wait = SIM7600_CONNECTION_START_TRANSPARENT_MODE_CONFIG;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_WAIT_STATE;
//          }
//          // fail
//          else if (at_command_status == SIM7600_ERROR || retry >= SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             is_error = true;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_END;
//          }

//          if (at_command_status != SIM7600_BUSY) {
//             TRACE_INFO_F(F("SIM7600 transparent mode... [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
//          }

//          // wait...
//          break;

//       case SIM7600_CONNECTION_START_APN_USERNAME_PASSWORD:
//          snprintf(buffer_ext2, SIM7600_BUFFER_LENGTH, "AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n", apn, username, password);
//          at_command_status = sendAtCommand(buffer_ext2, buffer_ext);

//          // success
//          if (at_command_status == SIM7600_OK) {
//             retry = 0;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_CONNECT;
//          }
//          // retry
//          else if (at_command_status == SIM7600_ERROR && (++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
//             start_time_ms = millis();
//             state_after_wait = SIM7600_CONNECTION_START_APN_USERNAME_PASSWORD;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_WAIT_STATE;
//          }
//          // fail
//          else if (at_command_status == SIM7600_ERROR || retry >= SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             is_error = true;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_END;
//          }

//          if (at_command_status != SIM7600_BUSY) {
//             TRACE_INFO_F(F("SIM7600 set APN, username and password... [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
//          }

//          // wait...
//          break;

//       case SIM7600_CONNECTION_START_CONNECT:
//          at_command_status = sendAtCommand("AT+CIICR\r\n", buffer_ext, AT_OK_STRING, AT_ERROR_STRING, SIM7600_CIICR_RESPONSE_TIME_MAX_MS);

//          // success
//          if (at_command_status == SIM7600_OK) {
//             retry = 0;
//             delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
//             start_time_ms = millis();
//             state_after_wait = SIM7600_CONNECTION_START_GET_IP;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_WAIT_STATE;
//          }
//          // retry
//          else if (at_command_status == SIM7600_ERROR && (++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
//             start_time_ms = millis();
//             state_after_wait = SIM7600_CONNECTION_START_CONNECT;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_WAIT_STATE;
//          }
//          // fail
//          else if (at_command_status == SIM7600_ERROR || retry >= SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             is_error = true;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_END;
//          }

//          if (at_command_status != SIM7600_BUSY) {
//             TRACE_INFO_F(F("SIM7600 setting up connection... [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
//          }

//          // wait...
//          break;

//       case SIM7600_CONNECTION_START_GET_IP:
//          at_command_status = getIp(ip);

//          // success
//          if (at_command_status == SIM7600_OK) {
//             retry = 0;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_END;
//          }
//          // retry
//          else if (at_command_status == SIM7600_ERROR && (++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
//             start_time_ms = millis();
//             state_after_wait = SIM7600_CONNECTION_START_GET_IP;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_WAIT_STATE;
//          }
//          // fail
//          else if (sim7600_status == SIM7600_ERROR || retry >= SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             retry = 0;
//             is_error = true;
//             sim7600_connection_start_state = SIM7600_CONNECTION_START_END;
//          }

//          if (at_command_status != SIM7600_BUSY) {
//             TRACE_INFO_F(F("SIM7600 IP... [ %s ] [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING), ip);
//          }

//          // wait...
//          break;

//       case SIM7600_CONNECTION_START_END:
//          sim7600_status = (is_error ? SIM7600_ERROR : SIM7600_OK);
//          sim7600_connection_start_state = SIM7600_CONNECTION_START_INIT;
//          TRACE_INFO_F(F("SIM7600 start connection... [ %s ]"), printStatus(sim7600_status, OK_STRING, ERROR_STRING));
//          break;

//       case SIM7600_CONNECTION_START_WAIT_STATE:
//          if (millis() - start_time_ms > delay_ms) {
//             sim7600_connection_start_state = state_after_wait;
//          }
//          break;
//    }

//    return sim7600_status;
// }

// sim7600_status_t SIM7600::stopConnection() {
//    static uint8_t retry;
//    static sim7600_connection_stop_state_t state_after_wait;
//    static uint32_t delay_ms;
//    static uint32_t start_time_ms;
//    static sim7600_status_t sim7600_status;
//    static bool is_error;
//    sim7600_status_t at_command_status;

//    switch (sim7600_connection_stop_state) {

//       case SIM7600_CONNECTION_STOP_INIT:
//          retry = 0;
//          is_error = false;
//          sim7600_status = SIM7600_BUSY;

//          sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_SWITCH_TO_COMMAND_MODE;
//          break;

//       case SIM7600_CONNECTION_STOP_SWITCH_TO_COMMAND_MODE:
//          at_command_status = exitTransparentMode();

//          // success
//          if (at_command_status == SIM7600_OK) {
//             retry = 0;
//             sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_CLOSE;
//          }
//          // retry
//          else if (at_command_status == SIM7600_ERROR && (++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
//             start_time_ms = millis();
//             state_after_wait = SIM7600_CONNECTION_STOP_SWITCH_TO_COMMAND_MODE;
//             sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_WAIT_STATE;
//          }
//          // fail
//          else if (sim7600_status == SIM7600_ERROR || retry >= SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             retry = 0;
//             is_error = true;
//             sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_END;
//          }
//          // wait
//          break;

//       case SIM7600_CONNECTION_STOP_CLOSE:
//          at_command_status = sendAtCommand("AT+CIPCLOSE=0\r\n", buffer_ext, AT_CIPCLOSE_OK_STRING, AT_CIPCLOSE_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

//          // success
//          if (at_command_status == SIM7600_OK) {
//             retry = 0;
//             sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_CLOSE_PDP;
//          }
//          // retry
//          else if (at_command_status == SIM7600_ERROR && (++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
//             start_time_ms = millis();
//             state_after_wait = SIM7600_CONNECTION_STOP_CLOSE;
//             sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_WAIT_STATE;
//          }
//          // fail
//          else if (sim7600_status == SIM7600_ERROR || retry >= SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             is_error = true;
//             sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_END;
//          }

//          if (at_command_status != SIM7600_BUSY) {
//             TRACE_INFO_F(F("SIM7600 stop connection... [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
//          }

//          // wait
//          break;

//       case SIM7600_CONNECTION_STOP_CLOSE_PDP:
//          at_command_status = sendAtCommand("AT+CIPSHUT\r\n", buffer_ext, AT_CIPSHUT_OK_STRING, AT_CIPSHUT_ERROR_STRING, SIM7600_CIPSHUT_RESPONSE_TIME_MAX_MS);

//          // success
//          if (at_command_status == SIM7600_OK) {
//             retry = 0;
//             sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_DETACH_GPRS;
//          }
//          // retry
//          else if (at_command_status == SIM7600_ERROR && (++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
//             start_time_ms = millis();
//             state_after_wait = SIM7600_CONNECTION_STOP_DETACH_GPRS;
//             sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_WAIT_STATE;
//          }
//          // fail
//          else if (sim7600_status == SIM7600_ERROR || retry >= SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             is_error = true;
//             sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_END;
//          }

//          if (at_command_status != SIM7600_BUSY) {
//             TRACE_INFO_F(F("SIM7600 PDP close... [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
//          }

//          // wait
//          break;

//       case SIM7600_CONNECTION_STOP_DETACH_GPRS:
//          at_command_status = sendAtCommand("AT+CGATT=0\r\n", buffer_ext, AT_OK_STRING, AT_ERROR_STRING, SIM7600_CGATT_RESPONSE_TIME_MAX_MS);

//          // success
//          if (at_command_status == SIM7600_OK) {
//             retry = 0;
//             sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_END;
//          }
//          // retry
//          else if (at_command_status == SIM7600_ERROR && (++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
//             start_time_ms = millis();
//             state_after_wait = SIM7600_CONNECTION_STOP_DETACH_GPRS;
//             sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_WAIT_STATE;
//          }
//          // fail
//          else if (sim7600_status == SIM7600_ERROR || retry >= SIM7600_GENERIC_RETRY_COUNT_MAX) {
//             is_error = true;
//             sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_END;
//          }

//          if (at_command_status != SIM7600_BUSY) {
//             TRACE_INFO_F(F("SIM7600 detach GPRS... [ %s ]"), printStatus(at_command_status, OK_STRING, ERROR_STRING));
//          }

//          // wait...
//          break;

//       case SIM7600_CONNECTION_STOP_END:
//          sim7600_status = (is_error ? SIM7600_ERROR : SIM7600_OK);
//          sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_INIT;
// 	 rssi = RSSI_UNKNOWN;
// 	 ber = BER_UNKNOWN;
//          TRACE_INFO_F(F("SIM7600 stop connection... [ %s ]"), printStatus(sim7600_status, OK_STRING, FAIL_STRING));
//          break;

//       case SIM7600_CONNECTION_STOP_WAIT_STATE:
//          if (millis() - start_time_ms > delay_ms) {
//             sim7600_connection_stop_state = state_after_wait;
//          }
//          break;
//    }

//    return sim7600_status;
// }

// // *****************************************************************************
// // etherclient compatibility
// // *****************************************************************************

// sim7600Client::sim7600Client() {}

// int sim7600Client::connect(IPAddress ip, int port) {
//    char server[16];
//    sprintf(server, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
//    return connection(SIM7600_CONNECTION_TCP, server, port);
// }

// int sim7600Client::connect(const char *server, int port) {
//    return (int) connection(SIM7600_CONNECTION_TCP, server, port);
// }

// uint8_t sim7600Client::connected() {
//    return isHttpInitialized();
// }

// int sim7600Client::available() {
//    return modem->available();
// }

// int sim7600Client::read() {
//    return modem->read();
// }

// int sim7600Client::readBytes(char *buffer, size_t size) {
//   return modem->readBytes(buffer, size);
// }

// int sim7600Client::readBytes(uint8_t *buffer, size_t size) {
//   return modem->readBytes(buffer, size);
// }

// void sim7600Client::setTimeout(uint32_t timeout_ms) {
//    modem->setTimeout(timeout_ms);
// }

// size_t sim7600Client::write(uint8_t buffer) {
//    return modem->write(buffer);
// }

// size_t sim7600Client::write(const uint8_t *buffer, size_t size) {
//    return modem->write(buffer, size);
// }

// void sim7600Client::flush() {
// }

// void sim7600Client::stop() {
// }
