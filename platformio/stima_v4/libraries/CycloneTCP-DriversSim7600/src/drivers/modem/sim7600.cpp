/**@file sim7600.cpp */

/*********************************************************************
<h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>
Moreno Gasperini <m.gasperini@digiteco.it>

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

#define USE_FREERTOS

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

void SIM7600::initPins(bool _set_direction)
{
   if(_set_direction) {
      pinMode(enable_power_pin, OUTPUT);
      pinMode(power_pin, OUTPUT);
      pinMode(ring_indicator_pin, INPUT);
   }
   // digitalWrite(enable_power_pin, LOW);
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
   sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_INIT;

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

   initPins(false);
}

bool SIM7600::isOn()
{
   return (state & SIM7600_STATE_ON);
}

bool SIM7600::isSetted()
{
   return (state & SIM7600_STATE_SETTED);
}

bool SIM7600::isConnected()
{
   return (state & SIM7600_STATE_CONNECTED);
}

// return true when switch on
sim7600_status_t SIM7600::switchOn()
{
   bool is_switching_on = true;
   sim7600_status_t at_command_status;

   at_command_status = switchModem(is_switching_on);

   if (at_command_status != SIM7600_BUSY) {
      uartDeInit();
      uartInitConfig(low_baud_rate);
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
         if (at_command_status == SIM7600_OK)
            delay_ms = SIM7600_WAIT_FOR_POWER_OFF_CPOF_DELAY_MS;
         else
            delay_ms = SIM7600_WAIT_FOR_POWER_OFF_DELAY_MS;
         sim7600_power_off_state = SIM7600_POWER_OFF_END;
         #endif
         at_command_status = SIM7600_BUSY;
      }
      break;

   case SIM7600_POWER_OFF_END:
      rssi = RSSI_UNKNOWN;
      ber = BER_UNKNOWN;
      creg_n = CREG_N_UNKNOWN;
      creg_stat = CREG_STAT_UNKNOWN;
      cgreg_n = CREG_N_UNKNOWN;
      cgreg_stat = CREG_STAT_UNKNOWN;
      cereg_n = CREG_N_UNKNOWN;
      cereg_stat = CREG_STAT_UNKNOWN;

      uartDeInit();
      // digitalWrite(enable_power_pin, LOW);
      digitalWrite(power_pin, LOW);
      uartInitConfig(low_baud_rate);

      state = SIM7600_STATE_NONE;

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
      uartDeInit();
      digitalWrite(power_pin, HIGH);
      // digitalWrite(enable_power_pin, HIGH);
      uartInitConfig(low_baud_rate);

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
         state = SIM7600_STATE_NONE;
         sim7600_power_state = SIM7600_POWER_END;
         TRACE_VERBOSE_F(F("SIM7600_POWER_CHECK_STATUS -> SIM7600_POWER_END\r\n"));
      }
      // fail: switching ON and is OFF
      else if (is_switching_on && (at_command_status == SIM7600_ERROR))
      {
         is_error = true;
         state = SIM7600_STATE_NONE;
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
         // digitalWrite(enable_power_pin, LOW);
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
         if(strncmp(command,"+++", 3)==0) {
            error = pppHdlcDriverSendAtCommand(interface, command);
         } else {
            error = pppSendAtCommand(interface, command);
         }
         
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

            // Check escape sequence... Enter command Mode (OK or NO CARRIER)
            bool bSeqEscapeOk = false;
            if(strncmp(command,"+++", 3)==0) {
               if(found(response, AT_NO_CARRIER_STRING)) {
                  bSeqEscapeOk = true;
               }
            }

            // ok
            if (!error && (found(response, at_ok_string) || bSeqEscapeOk))
            {
               TRACE_DEBUG_F(F("%s%s\r\n"), SIM7600_AT_RX_CMD_DEBUG_PREFIX, response);
               sim7600_at_state = SIM7600_AT_END;
               break;
            }
            // error
            else if (error || (!error && (found(response, at_error_string) || bSeqEscapeOk)))
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

uint8_t SIM7600::getRssi()
{
   return rssi;
}

uint8_t SIM7600::getBer()
{
   return ber;
}

uint8_t SIM7600::getCregN()
{
   return creg_n;
}

uint8_t SIM7600::getCregStat()
{
   return creg_stat;
}

uint8_t SIM7600::getCgregN()
{
   return cgreg_n;
}

uint8_t SIM7600::getCgregStat()
{
   return cgreg_stat;
}

uint8_t SIM7600::getCeregN()
{
   return cereg_n;
}

uint8_t SIM7600::getCeregStat()
{
   return cereg_stat;
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

sim7600_status_t SIM7600::sendAtCxreg(uint8_t cxreg_mode)
{
   sim7600_status_t at_command_status;

   if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM)
   {
      at_command_status = sendAtCommand("AT+CREG?\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);
   }
   else if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS)
   {
      at_command_status = sendAtCommand("AT+CGREG?\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);
   }
   else if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN)
   {
      at_command_status = sendAtCommand("AT+CEREG?\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);
   }

   if ((at_command_status == SIM7600_OK) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM))
   {
      if (sscanf(buffer_ext, "+CREG: %d,%d", &creg_n, &creg_stat) != 2)
      {
         at_command_status == SIM7600_ERROR;
      }
   }
   else if ((at_command_status == SIM7600_OK) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS))
   {
      if (sscanf(buffer_ext, "+CGREG: %d,%d", &cgreg_n, &cgreg_stat) != 2)
      {
         at_command_status == SIM7600_ERROR;
      }
   }
   else if ((at_command_status == SIM7600_OK) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN))
   {
      if (sscanf(buffer_ext, "+CEREG: %d,%d", &cereg_n, &cereg_stat) != 2)
      {
         at_command_status == SIM7600_ERROR;
      }
   }

   if ((at_command_status == SIM7600_ERROR) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM))
   {
      creg_n = CREG_N_UNKNOWN;
      creg_stat = CREG_STAT_UNKNOWN;
   }
   else if ((at_command_status == SIM7600_ERROR) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS))
   {
      cgreg_n = CREG_N_UNKNOWN;
      cgreg_stat = CREG_STAT_UNKNOWN;
   }
   else if ((at_command_status == SIM7600_ERROR) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN))
   {
      cereg_n = CREG_N_UNKNOWN;
      cereg_stat = CREG_STAT_UNKNOWN;
   }

   if ((at_command_status != SIM7600_BUSY) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM))
   {
      TRACE_VERBOSE_F(F("%s +CREG [ %s ] [ %d,%d ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), creg_n, creg_stat);
   }
   else if ((at_command_status != SIM7600_BUSY) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS))
   {
      TRACE_VERBOSE_F(F("%s +CGREG [ %s ] [ %d,%d ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), cgreg_n, cgreg_stat);
   }
   else if ((at_command_status != SIM7600_BUSY) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN))
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

sim7600_status_t SIM7600::setup(sim7600_connection_network_mode_t network_type, sim7600_type_network_registration_t network_regver, char* network_order)
{
   static uint8_t retry;
   static uint8_t retryWaitForNetwork;
   static sim7600_status_t sim7600_status;
   static bool is_error;
   static bool is_registered;
   sim7600_status_t at_command_status;
   static uint8_t cxreg_mode;
   static uint8_t cregx_level;
   uint8_t stat = 0;
   char cmdLocal[30];

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
      cxreg_mode = sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM;
      cregx_level = sim7600_type_network_registration_t::SIM7600_REG_NETWORK_NONE;
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
         retry = 0;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_SETUP_WAIT_NETWORK;
         sim7600_setup_state = SIM7600_SETUP_WAIT_STATE;
         #endif
         delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
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
         retry = 0;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_SETUP_WAIT_NETWORK;
         sim7600_setup_state = SIM7600_SETUP_WAIT_STATE;
         #endif
         delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
      }
      // fail
      else if (at_command_status == SIM7600_ERROR)
      {
         // Dont'care for ATE0 command
         is_error = false;
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
         retry = 0;
         delay_ms = SIM7600_WAIT_FOR_UART_RECONFIGURE_DELAY_MS;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_SETUP_WAIT_NETWORK;
         sim7600_setup_state = SIM7600_SETUP_WAIT_STATE;
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
         state_after_wait = SIM7600_SETUP_SET_MODE_NETWORK;
         sim7600_setup_state = SIM7600_SETUP_WAIT_STATE;
         #else
         sim7600_setup_state = SIM7600_SETUP_SET_MODE_NETWORK;
         #endif
         retry = 0;
         delay_ms = SIM7600_WAIT_FOR_SETUP_SET_MODE_NETWORK_DELAY_MS;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_SETUP_WAIT_NETWORK;
         sim7600_setup_state = SIM7600_SETUP_WAIT_STATE;
         #endif
         delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
      }
      // fail
      else if ((at_command_status == SIM7600_ERROR))
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

   case SIM7600_SETUP_SET_MODE_NETWORK:

      // No required parameter?... Continue
      if(network_type==sim7600_connection_network_mode_t::SIM7600_MODE_NETWORK_DEFAULT) {
         sim7600_setup_state = SIM7600_SETUP_SET_PRIORITY_NETWORK;
         retry = 0;
         break;
      }

      // Set mode network prefered
      sprintf(cmdLocal, "AT+CNMP=%d\r\n", network_type);
      
      at_command_status = sendAtCommand(cmdLocal, buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

      // success
      if (at_command_status == SIM7600_OK)
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_SETUP_SET_PRIORITY_NETWORK;
         sim7600_setup_state = SIM7600_SETUP_WAIT_STATE;
         #else
         sim7600_setup_state = SIM7600_SETUP_SET_PRIORITY_NETWORK;
         #endif
         retry = 0;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_SETUP_WAIT_NETWORK;
         sim7600_setup_state = SIM7600_SETUP_WAIT_STATE;
         #endif
         delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
      }
      // fail
      else if (at_command_status == SIM7600_ERROR)
      {
         is_error = true;
         sim7600_setup_state = SIM7600_SETUP_END;
      }

      if (at_command_status != SIM7600_BUSY)
      {
         TRACE_INFO_F(F("%s set network priority network type [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
      }
      break;

   case SIM7600_SETUP_SET_PRIORITY_NETWORK:

      // No required parameter?... Continue
      if(strlen(network_order)==0) {
         sim7600_setup_state = SIM7600_SETUP_GET_SIGNAL_QUALITY;
         retry = 0;
         break;
      }

      // Set mode network prefered
      sprintf(cmdLocal, "AT+CNAOP=7,%s\r\n", network_order);
      at_command_status = sendAtCommand(cmdLocal, buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

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
         retry = 0;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_SETUP_WAIT_NETWORK;
         sim7600_setup_state = SIM7600_SETUP_WAIT_STATE;
         #endif
         delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
      }
      // fail
      else if (at_command_status == SIM7600_ERROR)
      {
         is_error = true;
         sim7600_setup_state = SIM7600_SETUP_END;
      }

      if (at_command_status != SIM7600_BUSY)
      {
         TRACE_INFO_F(F("%s set network prefered connection mode to: %d - [ %s ]\r\n"), SIM7600_NAME, network_type, printStatus(at_command_status, OK_STRING, ERROR_STRING));
      }
      break;

   case SIM7600_SETUP_GET_SIGNAL_QUALITY:
      at_command_status = sendAtCsq();

      // success or fail: dont care
      if (at_command_status == SIM7600_OK || at_command_status == SIM7600_ERROR)
      {
         retry = 0;
         sim7600_setup_state = SIM7600_SETUP_ENABLE_NETWORK;
      }

      if (at_command_status != SIM7600_BUSY)
      {
         TRACE_INFO_F(F("%s signal [ %s ] [ rssi %d, ber %d ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), rssi, ber);
      }

      // wait
      break;

   case SIM7600_SETUP_ENABLE_NETWORK:
      if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM)
      {
         at_command_status = sendAtCommand("AT+CREG=2\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);
      }
      else if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS)
      {
         at_command_status = sendAtCommand("AT+CGREG=2\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);
      }
      else if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN)
      {
         at_command_status = sendAtCommand("AT+CEREG=2\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);
      }

      // success or fail
      if (at_command_status != SIM7600_BUSY)
      {
         if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM)
         {
            TRACE_INFO_F(F("%s enable CREG [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
         }
         else if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS)
         {
            TRACE_INFO_F(F("%s enable CGREG [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
         }
         else if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN)
         {
            TRACE_INFO_F(F("%s enable CEREG [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
         }

         retry = 0;
         cxreg_mode++;

         // Esco con tutti i register network abilitati richiesti in setup
         if (cxreg_mode > network_regver)
         {
            // Riavvio cxreg per la verifica se effettivamente avviene la registrazione per il tipo di connessione...
            // Waiting registration for requested network GSM/2G/4G, START FROM GSM
            cxreg_mode = sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM;

            delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
            sim7600_setup_state = SIM7600_SETUP_WAIT_NETWORK;
         }
         else
         {
            at_command_status = SIM7600_BUSY;
         }
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_SETUP_WAIT_NETWORK;
         sim7600_setup_state = SIM7600_SETUP_WAIT_STATE;
         #endif
         delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
      }
      // fail
      else if (at_command_status == SIM7600_ERROR)
      {
         is_error = true;
         sim7600_setup_state = SIM7600_SETUP_END;
      }

      // wait
      break;

   case SIM7600_SETUP_WAIT_NETWORK:
      is_registered = false;
      at_command_status = sendAtCxreg(cxreg_mode);
      switch(cxreg_mode) {
         case sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM:
            retryWaitForNetwork = SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX_GSM;
            break;
         case sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS:
            retryWaitForNetwork = SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX_GPRS;
            break;
         case sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN:
            retryWaitForNetwork = SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX_EUTRAN;
            break;
      }

      // success
      if (at_command_status == SIM7600_OK)
      {
         if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM)
         {
            stat = creg_stat;
         }
         else if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS)
         {
            stat = cgreg_stat;
         }
         else if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN)
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
            cregx_level = cxreg_mode;
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
            TRACE_INFO_F(F("%s unknown network... [ %s ]\r\n"), SIM7600_NAME, ERROR_STRING);
            break;

         case 5:
            #if (SIM7600_USE_ROAMING_REGISTER)
            TRACE_INFO_F(F("%s network registered (roaming)... [ %s ]\r\n"), SIM7600_NAME, OK_STRING);
            is_registered = true;
            cregx_level = cxreg_mode;
            #else
            TRACE_INFO_F(F("%s network registered (roaming) but rejected from SW configuration... [ %s ]\r\n"), SIM7600_NAME, OK_STRING);
            is_registered = false;
            #endif
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
         // Try all type network registration GSM -> GPRS -> LTE until Mode ( > CGREG_SETUP Request DEFAULT = LTE )
         // Exit success on all method requested (>network_regver) available (here)
         cxreg_mode++;
         if (cxreg_mode > network_regver)
         {
            sim7600_setup_state = SIM7600_SETUP_END;
            delay_ms = SIM7600_WAIT_FOR_NETWORK_DELAY_MS;
         }
         else
         {
            at_command_status = SIM7600_BUSY;
         }
      }
      // retry if available
      else if ((at_command_status == SIM7600_OK) && !is_registered && ((++retry) < retryWaitForNetwork))
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_SETUP_WAIT_NETWORK;
         sim7600_setup_state = SIM7600_SETUP_WAIT_STATE;
         #endif
         delay_ms = SIM7600_WAIT_FOR_NETWORK_DELAY_MS;
      }
      // fail or max retry
      else if ((at_command_status == SIM7600_ERROR) || (retry >= retryWaitForNetwork))
      {
         retry = 0;
         // Max registration not reached up, but if registered with CGREG_MIN (DEFAULT GPRS), connection was already OK
         // Exit success on minimal method available (here)
         if (cregx_level >= sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS)
         {
            // Minimal connection available es. GPRS (direct or roaming if enabled from configuration define)
            is_error = false;
            sim7600_setup_state = SIM7600_SETUP_END;
            delay_ms = SIM7600_WAIT_FOR_NETWORK_DELAY_MS;
         } else {
            // Optional roaming exter (UP) control...
            // Here connection not available (loss or roaming not valid)
            is_error = true;
            sim7600_setup_state = SIM7600_SETUP_END;
            delay_ms = SIM7600_WAIT_FOR_NETWORK_DELAY_MS;
            TRACE_INFO_F(F("%s network not available for registration. Connection abort... [ %s ]\r\n"), SIM7600_NAME, ERROR_STRING);
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

sim7600_status_t SIM7600::connect(const char *apn, const char *number)
{
   static uint8_t retry;
   static sim7600_status_t sim7600_status;
   sim7600_status_t at_command_status;
   static bool is_error;
   static bool is_attached;

   #ifndef USE_FREERTOS
   static sim7600_connection_start_state_t state_after_wait;
   static uint32_t start_time_ms;
   #endif

   switch (sim7600_connection_start_state)
   {

   case SIM7600_CONNECTION_START_INIT:
      retry = 0;
      is_error = false;
      sim7600_status = SIM7600_BUSY;

      if (isSetted())
      {
         sim7600_connection_start_state = SIM7600_CONNECTION_START_PDP;
      }
      else
      {
         is_error = true;
         sim7600_connection_start_state = SIM7600_CONNECTION_START_END;
      }
      break;

   case SIM7600_CONNECTION_START_PDP:
      snprintf(buffer_ext2, SIM7600_BUFFER_LENGTH, "AT+CGDCONT=1,\"IP\",\"%s\"\r\n", apn);
      at_command_status = sendAtCommand(buffer_ext2, buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

      // success
      if (at_command_status == SIM7600_OK)
      {
         retry = 0;
         sim7600_connection_start_state = SIM7600_CONNECTION_START_PDP_AUTH;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && (++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX)
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_CONNECTION_START_PDP;
         sim7600_connection_start_state = SIM7600_CONNECTION_START_WAIT_STATE;
         #endif
         delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
      }
      // fail
      else if ((at_command_status == SIM7600_ERROR) || (retry >= SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         is_error = true;
         sim7600_connection_start_state = SIM7600_CONNECTION_START_END;
      }

      if (at_command_status != SIM7600_BUSY)
      {
         TRACE_INFO_F(F("%s set PDP context... [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
      }

      // wait...
      break;

   case SIM7600_CONNECTION_START_PDP_AUTH:
      at_command_status = sendAtCommand("AT+CGAUTH=1,0\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

      // success
      if (at_command_status == SIM7600_OK)
      {
         retry = 0;
         sim7600_connection_start_state = SIM7600_CONNECTION_START_CONNECT;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_CONNECTION_START_TRANSPARENT;
         sim7600_connection_start_state = SIM7600_CONNECTION_START_WAIT_STATE;
         #endif
         delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
      }
      // fail
      else if ((at_command_status == SIM7600_ERROR) || (retry >= SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         is_error = true;
         sim7600_connection_start_state = SIM7600_CONNECTION_START_END;
      }

      if (at_command_status != SIM7600_BUSY)
      {
         TRACE_INFO_F(F("%s set PDP authentication... [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
      }

      // wait...
      break;

   case SIM7600_CONNECTION_START_CONNECT:
      snprintf(buffer_ext2, SIM7600_BUFFER_LENGTH, "ATD%s\r\n", number);
      at_command_status = sendAtCommand(buffer_ext2, buffer_ext, sizeof(buffer_ext), AT_CONNECT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

      // success
      if (at_command_status == SIM7600_OK)
      {
         retry = 0;
         sim7600_connection_start_state = SIM7600_CONNECTION_START_END;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_CONNECTION_START_CONNECT;
         sim7600_connection_start_state = SIM7600_CONNECTION_START_WAIT_STATE;
         #endif
         delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
      }
      // fail
      else if ((at_command_status == SIM7600_ERROR) || (retry >= SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         is_error = true;
         sim7600_connection_start_state = SIM7600_CONNECTION_START_END;
      }

      if (at_command_status != SIM7600_BUSY)
      {
         TRACE_INFO_F(F("%s setting up connection... [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
      }

      // wait...
      break;

   case SIM7600_CONNECTION_START_END:
      if (is_error)
      {
         sim7600_status = SIM7600_ERROR;
         state = (sim7600_state_t)(state & ~SIM7600_STATE_CONNECTED);
      }
      else
      {
         sim7600_status = SIM7600_OK;
         state = (sim7600_state_t)(state | SIM7600_STATE_CONNECTED);
      }
      
      sim7600_connection_start_state = SIM7600_CONNECTION_START_INIT;
      TRACE_INFO_F(F("%s start connection... [ %s ]\r\n"), SIM7600_NAME, printStatus(sim7600_status, OK_STRING, ERROR_STRING));
      break;

   #ifndef USE_FREERTOS
   case SIM7600_CONNECTION_START_WAIT_STATE:
      if (millis() - start_time_ms > delay_ms)
      {
         sim7600_connection_start_state = state_after_wait;
      }
      break;
   #endif
   }

   return sim7600_status;
}

sim7600_status_t SIM7600::disconnect() {
   static uint8_t retry;
   static sim7600_status_t sim7600_status;
   sim7600_status_t at_command_status;
   static bool is_error;
   static bool is_attached;

   #ifndef USE_FREERTOS
   static sim7600_connection_start_state_t state_after_wait;
   static uint32_t start_time_ms;
   #endif

   switch (sim7600_connection_stop_state)
   {
   case SIM7600_CONNECTION_STOP_INIT:
      retry = 0;
      is_error = false;
      sim7600_status = SIM7600_BUSY;

      // Delay before enter escape sequence -> command mode
      TRACE_INFO_F(F("Starting escape sequence +++\r\n"), SIM7600_NAME);
      delay_ms = SIM7600_COMMAND_MODE_WAIT_DELAY_MS;

      sim7600_connection_stop_state = SIM7600_CONNECTION_ENTER_COMMAND_MODE;
      break;

   case SIM7600_CONNECTION_ENTER_COMMAND_MODE:
      at_command_status = sendAtCommand("+++", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_FASTCMD_TIMEOUT_MS);

      // success
      if (at_command_status == SIM7600_OK)
      {
         retry = 0;
         sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_HANGUP;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_CONNECTION_ENTER_COMMAND_MODE;
         sim7600_connection_stop_state = SIM7600_CONNECTION_START_WAIT_STATE;
         #endif
         delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
      }
      // fail
      else if ((at_command_status == SIM7600_ERROR) || (retry >= SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         is_error = true;
         sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_END;
      }

      if (at_command_status != SIM7600_BUSY)
      {
         TRACE_INFO_F(F("%s enter command mode... [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
      }
      break;

   case SIM7600_CONNECTION_STOP_HANGUP:
      at_command_status = sendAtCommand("ATH\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

      // success
      if (at_command_status == SIM7600_OK)
      {
         retry = 0;
         sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_END;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         #ifndef USE_FREERTOS
         start_time_ms = millis();
         state_after_wait = SIM7600_CONNECTION_STOP_HANGUP;
         sim7600_connection_stop_state = SIM7600_CONNECTION_START_WAIT_STATE;
         #endif
         delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
      }
      // fail
      else if ((at_command_status == SIM7600_ERROR) || (retry >= SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         is_error = true;
         sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_END;
      }

      if (at_command_status != SIM7600_BUSY)
      {
         TRACE_INFO_F(F("%s close connection... [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
      }

      // wait...
      break;

   case SIM7600_CONNECTION_STOP_END:
      if (is_error)
      {
         sim7600_status = SIM7600_ERROR;
         state = (sim7600_state_t)(state & ~SIM7600_STATE_CONNECTED);
      }
      else
      {
         sim7600_status = SIM7600_OK;
         state = (sim7600_state_t)(state & ~SIM7600_STATE_CONNECTED);
      }

      sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_INIT;

      rssi = RSSI_UNKNOWN;
      ber = BER_UNKNOWN;
      creg_n = CREG_N_UNKNOWN;
      creg_stat = CREG_STAT_UNKNOWN;
      cgreg_n = CREG_N_UNKNOWN;
      cgreg_stat = CREG_STAT_UNKNOWN;
      cereg_n = CREG_N_UNKNOWN;
      cereg_stat = CREG_STAT_UNKNOWN;

      TRACE_INFO_F(F("%s disconnect... [ %s ]\r\n"), SIM7600_NAME, printStatus(sim7600_status, OK_STRING, ERROR_STRING));
      break;

   #ifndef USE_FREERTOS
   case SIM7600_CONNECTION_STOP_WAIT_STATE:
      if (millis() - start_time_ms > delay_ms)
      {
         sim7600_connection_stop_state = state_after_wait;
      }
      break;
   #endif
   }

   return sim7600_status;
}
