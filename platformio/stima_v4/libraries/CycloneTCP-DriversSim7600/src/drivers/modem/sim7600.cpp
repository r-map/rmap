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
#include "ppp/ppp.h"
#include "ppp/ppp_hdlc.h"

SIM7600::SIM7600()
{
}

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

void SIM7600::initPins(bool _set_direction)
{
   if(_set_direction) {
      if (enable_power_pin) pinMode(enable_power_pin, OUTPUT);
      pinMode(power_pin, OUTPUT);
      if(ring_indicator_pin) pinMode(ring_indicator_pin, INPUT);
   }
   if (enable_power_pin) digitalWrite(enable_power_pin, LOW);
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

   sim7600_rssi = RSSI_UNKNOWN;
   sim7600_ber = BER_UNKNOWN;
   sim7600_creg_n = CREG_N_UNKNOWN;
   sim7600_creg_stat = CREG_STAT_UNKNOWN;
   sim7600_cgreg_n = CREG_N_UNKNOWN;
   sim7600_cgreg_stat = CREG_STAT_UNKNOWN;
   sim7600_cereg_n = CREG_N_UNKNOWN;
   sim7600_cereg_stat = CREG_STAT_UNKNOWN;
   sim7600_cnsmod = 0;

   delay_ms = SIM7600_GENERIC_STATE_DELAY_MS;

   uartDeInit();
   uartInitConfig(low_baud_rate);

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
         if (at_command_status == SIM7600_OK)
            delay_ms = SIM7600_WAIT_FOR_POWER_OFF_CPOF_DELAY_MS;
         else
            delay_ms = SIM7600_WAIT_FOR_POWER_OFF_DELAY_MS;
         sim7600_power_off_state = SIM7600_POWER_OFF_END;
         at_command_status = SIM7600_BUSY;
      }
      break;

   case SIM7600_POWER_OFF_END:

      uartDeInit();
      if (enable_power_pin) digitalWrite(enable_power_pin, LOW);
      digitalWrite(power_pin, LOW);
      uartInitConfig(low_baud_rate);

      state = SIM7600_STATE_NONE;

      TRACE_INFO_F(F("%s switching OFF... [ %s ] [ %s ]\r\n"), SIM7600_NAME, printStatus(SIM7600_OK, OK_STRING, ERROR_STRING), isOn() ? ON_STRING : OFF_STRING);
      sim7600_power_off_state = SIM7600_POWER_OFF_INIT;
      at_command_status = SIM7600_OK;
      break;
   }

   return at_command_status;
}

sim7600_status_t SIM7600::switchModem(bool is_switching_on)
{
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
      if (enable_power_pin) digitalWrite(enable_power_pin, HIGH);
      uartInitConfig(low_baud_rate);

      delay_ms = SIM7600_POWER_ON_STABILIZATION_DELAY_MS;

      sim7600_power_state = SIM7600_POWER_IMPULSE_DOWN;

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

      sim7600_power_state = SIM7600_POWER_IMPULSE_UP;

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
         if (enable_power_pin) digitalWrite(enable_power_pin, LOW);
         digitalWrite(power_pin, LOW);
         is_error = false;
         sim7600_status = SIM7600_BUSY;

         sim7600_power_state = SIM7600_POWER_ENABLE;

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
   }

   return sim7600_status;
}

void SIM7600::cleanInput()
{
   if (interface != NULL && interface->pppContext != NULL) {
      pppHdlcDriverPurgeRxBuffer(interface->pppContext);
   }
}

sim7600_status_t SIM7600::sendAtCommand(const char *command, char *response, size_t response_length, const char *at_ok_string, const char *at_error_string, uint32_t timeout_ms)
{
   error_t error;
   static size_t n;

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

         pppSetTimeout(interface, timeout_ms);

         if (strlen(command))
         {
            delay_ms = SIM7600_AT_DELAY_MS;

            sim7600_at_state = SIM7600_AT_SEND;
         }
         else
         {
            sim7600_at_state = SIM7600_AT_RECEIVE;
         }
         break;

      case SIM7600_AT_SEND:
         // Discard URC/stale RX before each AT (validated on MDLOG field debug)
         cleanInput();

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

         TRACE_DEBUG_F(F("%s%s"), SIM7600_AT_TX_CMD_DEBUG_PREFIX, command);
         break;

      case SIM7600_AT_RECEIVE:
         if (n < response_length)
         {
            // Wait for a response from the modem
            error = pppReceiveAtCommand(interface, response + n, response_length - n);

            // Check escape sequence... Enter command Mode (OK DISCONNECTED or NO CARRIER)
            bool bSeqEscapeOk = false;
            if(strncmp(command,"+++", 3)==0) {
               if(found(response, AT_DISCONNETTING_STRING)) {
                  bSeqEscapeOk = true;
               }
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
         break;

      case SIM7600_AT_END:
         sim7600_status = (is_error ? SIM7600_ERROR : SIM7600_OK);
         sim7600_at_state = SIM7600_AT_INIT;
         break;
   }

   return sim7600_status;
}

uint32_t SIM7600::getDelayMs()
{
   return delay_ms;
}

uint8_t SIM7600::getRssi()
{
   return sim7600_rssi;
}

uint8_t SIM7600::getBer()
{
   return sim7600_ber;
}

uint8_t SIM7600::getCregN()
{
   return sim7600_creg_n;
}

uint8_t SIM7600::getCnsmod() 
{
   return sim7600_cnsmod;
}

uint8_t SIM7600::getCregStat()
{
   return sim7600_creg_stat;
}

uint8_t SIM7600::getCgregN()
{
   return sim7600_cgreg_n;
}

uint8_t SIM7600::getCgregStat()
{
   return sim7600_cgreg_stat;
}

uint8_t SIM7600::getCeregN()
{
   return sim7600_cereg_n;
}

uint8_t SIM7600::getCeregStat()
{
   return sim7600_cereg_stat;
}

void SIM7600::setPins(uint8_t _enable_power_pin, uint8_t _power_pin, uint8_t _ring_indicator_pin)
{
   enable_power_pin = _enable_power_pin;
   power_pin = _power_pin;
   ring_indicator_pin = _ring_indicator_pin;
}

void SIM7600::setInterface(NetInterface *_interface)
{
   interface = _interface;
}

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
      int n_tmp = 0;
      int stat_tmp = 0;
      /* %d richiede int*: scrivere su uint8_t corrompe i membri adiacenti. */
      if (sscanf(buffer_ext, "+CREG: %d,%d", &n_tmp, &stat_tmp) != 2)
      {
         at_command_status = SIM7600_ERROR;
      }
      else
      {
         sim7600_creg_n = (uint8_t)n_tmp;
         sim7600_creg_stat = (uint8_t)stat_tmp;
      }
   }
   else if ((at_command_status == SIM7600_OK) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS))
   {
      int n_tmp = 0;
      int stat_tmp = 0;
      if (sscanf(buffer_ext, "+CGREG: %d,%d", &n_tmp, &stat_tmp) != 2)
      {
         at_command_status = SIM7600_ERROR;
      }
      else
      {
         sim7600_cgreg_n = (uint8_t)n_tmp;
         sim7600_cgreg_stat = (uint8_t)stat_tmp;
      }
   }
   else if ((at_command_status == SIM7600_OK) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN))
   {
      int n_tmp = 0;
      int stat_tmp = 0;
      if (sscanf(buffer_ext, "+CEREG: %d,%d", &n_tmp, &stat_tmp) != 2)
      {
         at_command_status = SIM7600_ERROR;
      }
      else
      {
         sim7600_cereg_n = (uint8_t)n_tmp;
         sim7600_cereg_stat = (uint8_t)stat_tmp;
      }
   }

   if ((at_command_status == SIM7600_ERROR) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM))
   {
      sim7600_creg_n = CREG_N_UNKNOWN;
      sim7600_creg_stat = CREG_STAT_UNKNOWN;
   }
   else if ((at_command_status == SIM7600_ERROR) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS))
   {
      sim7600_cgreg_n = CREG_N_UNKNOWN;
      sim7600_cgreg_stat = CREG_STAT_UNKNOWN;
   }
   else if ((at_command_status == SIM7600_ERROR) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN))
   {
      sim7600_cereg_n = CREG_N_UNKNOWN;
      sim7600_cereg_stat = CREG_STAT_UNKNOWN;
   }

   if ((at_command_status != SIM7600_BUSY) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM))
   {
      TRACE_VERBOSE_F(F("%s +CREG [ %s ] [ %d,%d ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), sim7600_creg_n, sim7600_creg_stat);
   }
   else if ((at_command_status != SIM7600_BUSY) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS))
   {
      TRACE_VERBOSE_F(F("%s +CGREG [ %s ] [ %d,%d ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), sim7600_cgreg_n, sim7600_cgreg_stat);
   }
   else if ((at_command_status != SIM7600_BUSY) && (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN))
   {
      TRACE_VERBOSE_F(F("%s +CEREG [ %s ] [ %d,%d ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), sim7600_cereg_n, sim7600_cereg_stat);
   }

   return at_command_status;
}

sim7600_status_t SIM7600::sendAtCsq()
{
   sim7600_status_t at_command_status;

   at_command_status = sendAtCommand("AT+CSQ\r\n", buffer_ext, sizeof(buffer_ext), "+CSQ:", AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

   if (at_command_status == SIM7600_OK)
   {
      int rssi_tmp = 0;
      int ber_tmp = 0;
      if (sscanf(buffer_ext, "+CSQ: %d,%d", &rssi_tmp, &ber_tmp) != 2)
      {
         at_command_status = SIM7600_ERROR;
      }
      else
      {
         sim7600_rssi = (uint8_t)rssi_tmp;
         sim7600_ber = (uint8_t)ber_tmp;
      }
   }

   if (at_command_status == SIM7600_ERROR)
   {
      sim7600_rssi = RSSI_UNKNOWN;
      sim7600_ber = BER_UNKNOWN;
   }

   if (at_command_status != SIM7600_BUSY) {
      TRACE_VERBOSE_F(F("%s CSQ [ %s ] [ %d,%d ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), sim7600_rssi, sim7600_ber);
   }

   return at_command_status;
}

sim7600_status_t SIM7600::sendAtCnsmod()
{
   sim7600_status_t at_command_status;

   at_command_status = sendAtCommand("AT+CNSMOD?\r\n", buffer_ext, sizeof(buffer_ext), "+CNSMOD:", AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

   if (at_command_status == SIM7600_OK)
   {
      int n_tmp = 0;
      int cnsmod_tmp = 0;
      /* %d su uint8_t = UB / corruzione (IWDG tipico al 1° CNSMOD). */
      if (sscanf(buffer_ext, "+CNSMOD: %d,%d", &n_tmp, &cnsmod_tmp) != 2)
      {
         at_command_status = SIM7600_ERROR;
      }
      else
      {
         sim7600_cnsmod = (uint8_t)cnsmod_tmp;
      }
   }

   if (at_command_status == SIM7600_ERROR)
   {
      sim7600_cnsmod = CNSMOD_UNKNOWN;
   }

   if (at_command_status != SIM7600_BUSY) {
      TRACE_VERBOSE_F(F("%s CNSMOD [ %s ] [ %d ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), sim7600_cnsmod);
   }

   return at_command_status;
}

sim7600_status_t SIM7600::sendAtGpsInfo()
{
   sim7600_status_t at_command_status;

   at_command_status = sendAtCommand("AT+CGPSINFO\r\n", buffer_ext, sizeof(buffer_ext), "+CGPSINFO:", AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);
   sim7600_is_gps_ready = false;

   if (at_command_status == SIM7600_OK)
   {
      // Response valid "+CGPSINFO: 4429.967842,N,01124.141582,E,250325,160319.0,57.7,0.0,
      // Response empty "+CGPSINFO: ,,,,,,,,
      if (strstr(buffer_ext, "+CGPSINFO: ")) {
         buffer_ptr = buffer_ext + 11; // Excluding 11 char -> "+CGPSINFO: ", setted to response Pointer
         // Check response valid
         if(memcmp(buffer_ptr, ",,,", 3)) {
            convertGpsNMEADecimal(buffer_ptr, sim7600_gps_lat_pos, sim7600_gps_lon_pos, sim7600_gps_alt);
            sim7600_is_gps_ready = true;
         }
      }
   }

   if (at_command_status == SIM7600_ERROR)
   {
      // Reset GPS Info field
      memset(sim7600_gps_lat_pos, sizeof(sim7600_gps_lat_pos), 0);
      memset(sim7600_gps_lon_pos, sizeof(sim7600_gps_lon_pos), 0);
      memset(sim7600_gps_alt, sizeof(sim7600_gps_alt), 0);
   }

   if (at_command_status != SIM7600_BUSY) {
      if(sim7600_is_gps_ready) {
         TRACE_VERBOSE_F(F("%s CGPSINFO [ %s ] [ lat:%s lon:%s alt:%s ]\r\n"), SIM7600_NAME, OK_STRING, sim7600_gps_lat_pos, sim7600_gps_lon_pos, sim7600_gps_alt);
      } else {
         TRACE_VERBOSE_F(F("%s CGPSINFO [ %s ] [ GPS NOT syncronized ]\r\n"), SIM7600_NAME, OK_STRING);
      }
   }

   return at_command_status;
}

sim7600_status_t SIM7600::setup(sim7600_connection_network_mode_t network_type, sim7600_type_network_registration_t network_regver, char* network_order, bool test_connection, bool *update_flags)
{
   static uint8_t retry;
   static uint8_t retryWaitForNetwork;
   static sim7600_status_t sim7600_status;
   static bool is_error;
   static bool is_registered;
   static uint8_t test_antenna_refresh_count;   // CSQ/CNSMOD cycles for LCD antenna positioning after registration
   sim7600_status_t at_command_status;
   static uint8_t cxreg_mode;
   static uint8_t cregx_level;
   uint8_t stat = 0;
   #if (SIM7600_DISABLE_URC)
   static uint8_t urc_step;
   static sim7600_setup_state_t state_after_disable_urc;
   #endif

   delay_ms = SIM7600_GENERIC_STATE_DELAY_MS;

   switch (sim7600_setup_state)
   {
   case SIM7600_SETUP_INIT:
      retry = 0;
      is_error = false;
      test_antenna_refresh_count = 0;
      #if (SIM7600_DISABLE_URC)
      urc_step = 0;
      #endif
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
         sim7600_setup_state = SIM7600_SETUP_ECHO_MODE;
         retry = 0;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
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
         #if (SIM7600_DISABLE_URC)
         urc_step = 0;
         state_after_disable_urc = SIM7600_SETUP_CHANGE_BAUD_RATE;
         sim7600_setup_state = SIM7600_SETUP_DISABLE_URC;
         #else
         sim7600_setup_state = SIM7600_SETUP_CHANGE_BAUD_RATE;
         #endif
         retry = 0;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
      }
      // fail
      else if (at_command_status == SIM7600_ERROR)
      {
         // Dont'care for ATE0 command
         is_error = false;
         #if (SIM7600_DISABLE_URC)
         urc_step = 0;
         state_after_disable_urc = SIM7600_SETUP_CHANGE_BAUD_RATE;
         sim7600_setup_state = SIM7600_SETUP_DISABLE_URC;
         #else
         sim7600_setup_state = SIM7600_SETUP_END;
         #endif
      }

      if (at_command_status != SIM7600_BUSY)
      {
         TRACE_INFO_F(F("%s echo mode off [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
      }

      // wait...
      break;

   #if (SIM7600_DISABLE_URC)
   case SIM7600_SETUP_DISABLE_URC:
      {
         static const char *const urc_cmds[] = {
            "AT+CREG=0\r\n",
            "AT+CGREG=0\r\n",
            "AT+CEREG=0\r\n",
            "AT+CNSMOD=0\r\n"
         };
         const uint8_t urc_cmds_count = (uint8_t)(sizeof(urc_cmds) / sizeof(urc_cmds[0]));

         at_command_status = sendAtCommand(urc_cmds[urc_step], buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

         // Best-effort: continue on OK or ERROR (do not fail whole setup)
         if (at_command_status == SIM7600_OK || at_command_status == SIM7600_ERROR)
         {
            TRACE_INFO_F(F("%s disable URC step %d [ %s ]\r\n"), SIM7600_NAME, urc_step, printStatus(at_command_status, OK_STRING, ERROR_STRING));
            urc_step++;
            retry = 0;
            if (urc_step >= urc_cmds_count)
            {
               sim7600_setup_state = state_after_disable_urc;
            }
         }
      }
      break;
   #endif

   case SIM7600_SETUP_CHANGE_BAUD_RATE:
      // change baud rate
      snprintf(buffer_cmd, sizeof(buffer_cmd), "AT+IPR=%d\r\n", high_baud_rate);
      at_command_status = sendAtCommand(buffer_cmd, buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

      // success
      if (at_command_status == SIM7600_OK)
      {
         uartDeInit();
         uartInitConfig(high_baud_rate);
         sim7600_setup_state = SIM7600_SETUP_SET_PHONE_FUNCTIONALITY;
         retry = 0;
         delay_ms = SIM7600_WAIT_FOR_UART_RECONFIGURE_DELAY_MS;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
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
         sim7600_setup_state = SIM7600_SETUP_SET_MODE_NETWORK;
         retry = 0;
         delay_ms = SIM7600_WAIT_FOR_SETUP_SET_MODE_NETWORK_DELAY_MS;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
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

      if(network_type==sim7600_connection_network_mode_t::SIM7600_MODE_NETWORK_DEFAULT) {
         // Set mode network prefered (always security default)
         sprintf(buffer_cmd, "AT+CNMP=%d\r\n", sim7600_connection_network_mode_t::SIM7600_MODE_NETWORK_AUTO);
      } else {
         // Set mode network prefered (manual configured value)
         sprintf(buffer_cmd, "AT+CNMP=%d\r\n", network_type);
      }
      at_command_status = sendAtCommand(buffer_cmd, buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);      

      // success
      if (at_command_status == SIM7600_OK)
      {
         sim7600_setup_state = SIM7600_SETUP_SET_PRIORITY_NETWORK;
         retry = 0;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
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
         TRACE_INFO_F(F("%s set network mode [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
      }
      break;

   case SIM7600_SETUP_SET_PRIORITY_NETWORK:
      if ((network_order == NULL) || (strlen(network_order) == 0))
      {
         sim7600_setup_state = SIM7600_SETUP_GET_SIGNAL_QUALITY;
         retry = 0;
         break;
      }

      snprintf(buffer_cmd, sizeof(buffer_cmd), "AT+CNAOP=7,%s\r\n", network_order);
      at_command_status = sendAtCommand(buffer_cmd, buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

      if (at_command_status == SIM7600_OK)
      {
         sim7600_setup_state = SIM7600_SETUP_GET_SIGNAL_QUALITY;
         retry = 0;
      }
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
      }
      else if (at_command_status == SIM7600_ERROR)
      {
         is_error = true;
         sim7600_setup_state = SIM7600_SETUP_END;
      }

      if (at_command_status != SIM7600_BUSY)
      {
         TRACE_INFO_F(F("%s set network order CNAOP [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
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
         if (update_flags) {
            *update_flags = true;
         }
         TRACE_INFO_F(F("%s signal [ %s ] [ rssi %d, ber %d ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), sim7600_rssi, sim7600_ber);
      }

      // wait
      break;

   case SIM7600_SETUP_ENABLE_NETWORK:
      sim7600_setup_state = SIM7600_SETUP_WAIT_NETWORK;
      // if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM)
      // {
      //    at_command_status = sendAtCommand("AT+CREG=1\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);
      // }
      // else if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS)
      // {
      //    at_command_status = sendAtCommand("AT+CGREG=1\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);
      // }
      // else if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN)
      // {
      //    at_command_status = sendAtCommand("AT+CEREG=1\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);
      // }

      // // success or fail
      // if (at_command_status != SIM7600_BUSY)
      // {
      //    if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM)
      //    {
      //       TRACE_INFO_F(F("%s enable CREG [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
      //    }
      //    else if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS)
      //    {
      //       TRACE_INFO_F(F("%s enable CGREG [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
      //    }
      //    else if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN)
      //    {
      //       TRACE_INFO_F(F("%s enable CEREG [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING));
      //    }

      //    retry = 0;
      //    cxreg_mode++;

      //    if (network_regver > sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN)
      //       network_regver = sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS;

      //    // Esco con tutti i register network abilitati richiesti in setup
      //    if (cxreg_mode > network_regver)
      //    {
      //       // Riavvio cxreg per la verifica se effettivamente avviene la registrazione per il tipo di connessione...
      //       // Waiting registration for requested network GSM/2G/4G, START FROM GSM
      //       cxreg_mode = sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM;

      //       delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
      //       sim7600_setup_state = SIM7600_SETUP_WAIT_NETWORK;
      //    }
      //    else
      //    {
      //       at_command_status = SIM7600_BUSY;
      //    }
      // }
      // // retry
      // else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      // {
      //    delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
      // }
      // // fail
      // else if (at_command_status == SIM7600_ERROR)
      // {
      //    is_error = true;
      //    sim7600_setup_state = SIM7600_SETUP_END;
      // }

      // wait
      break;

   case SIM7600_SETUP_WAIT_NETWORK:
      is_registered = false;
      at_command_status = sendAtCxreg(cxreg_mode);
      switch(cxreg_mode) {
         case sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM:
            if(test_connection) {
               retryWaitForNetwork = SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX_TEST;
            } else {
               retryWaitForNetwork = SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX_GSM;
            }
            break;
         case sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS:
            if(test_connection) {
               retryWaitForNetwork = SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX_TEST;
            } else {
               retryWaitForNetwork = SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX_GPRS;
            }
            break;
         case sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN:
            if(test_connection) {
               retryWaitForNetwork = SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX_TEST;
            } else {
               retryWaitForNetwork = SIM7600_WAIT_FOR_NETWORK_RETRY_COUNT_MAX_EUTRAN;
            }
            break;
      }

      // success
      if (at_command_status == SIM7600_OK)
      {
         if (update_flags) {
            *update_flags = true;
         }
         if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GSM)
         {
            stat = sim7600_creg_stat;
         }
         else if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_GPRS)
         {
            stat = sim7600_cgreg_stat;
         }
         else if (cxreg_mode == sim7600_type_network_registration_t::SIM7600_REG_NETWORK_EUTRAN)
         {
            stat = sim7600_cereg_stat;
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
         // test_connection (LCD force): N CSQ+CNSMOD refresh for antenna positioning on display, then PPP
         if (test_connection && (test_antenna_refresh_count < SIM7600_TEST_ANTENNA_REFRESH_COUNT_MAX))
         {
            test_antenna_refresh_count++;
            sim7600_setup_state = SIM7600_SETUP_UPDATE_SIGNAL_QUALITY;
            delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
            break;
         }
         // Try all type network registration GSM -> GPRS -> LTE until Mode ( > CGREG_SETUP Request DEFAULT = LTE )
         // Exit success on all method requested (>network_regver) available (here)
         cxreg_mode++;
         if (cxreg_mode > network_regver)
         {
            retry = 0;
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
         sim7600_setup_state = SIM7600_SETUP_UPDATE_SIGNAL_QUALITY;
         delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
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

   case SIM7600_SETUP_UPDATE_SIGNAL_QUALITY:
      at_command_status = sendAtCsq();

      // success or fail: dont care
      if (at_command_status == SIM7600_OK || at_command_status == SIM7600_ERROR)
      {
         // retry = 0; Don't reset retry (master command is +cxreg?)
         sim7600_setup_state = SIM7600_SETUP_UPDATE_NETWORK_MODE;
         delay_ms = SIM7600_COMMAND_MODE_WAIT_DELAY_MS;
      }

      if (at_command_status != SIM7600_BUSY)
      {
         if (update_flags) {
            *update_flags = true;
         }
         TRACE_INFO_F(F("%s signal [ %s ] [ rssi %d, ber %d ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), sim7600_rssi, sim7600_ber);
      }

      // wait
      break;

   case SIM7600_SETUP_UPDATE_NETWORK_MODE:
      at_command_status = sendAtCnsmod();

      // success or fail: dont care
      if (at_command_status == SIM7600_OK || at_command_status == SIM7600_ERROR)
      {
         // retry = 0; Don't reset retry (master command is +cxreg?)
         sim7600_setup_state = SIM7600_SETUP_WAIT_NETWORK;
         delay_ms = SIM7600_WAIT_FOR_NETWORK_DELAY_MS;
      }

      if (at_command_status != SIM7600_BUSY)
      {
         if (update_flags) {
            *update_flags = true;
         }
         #if TRACE_LEVEL >= TRACE_INFO
         char tModeNetwork[12]; // MAX_LEN 11 (NO_SERVICE+\0)
         switch (sim7600_cnsmod) {
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_NO_SERVICE):
               strcpy(tModeNetwork,"NO SERVICE");
               break;
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_GSM):
               strcpy(tModeNetwork,"GSM");
               break;
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_GPRS):
               strcpy(tModeNetwork,"GPRS");
               break;
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_EDGE):
               strcpy(tModeNetwork,"EDGE");
               break;
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_WCDMA):
               strcpy(tModeNetwork,"WCDMA");
               break;
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_HSDPA):
               strcpy(tModeNetwork,"HSDPA");
               break;
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_HSUPA):
               strcpy(tModeNetwork,"HSUPA");
               break;
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_HSPA):
               strcpy(tModeNetwork,"HSPA");
               break;
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_LTE):
               strcpy(tModeNetwork,"LTE");
               break;
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_TDS_CDMA):
               strcpy(tModeNetwork,"TDS+CDMA");
               break;
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_TDS_HSDPA):
               strcpy(tModeNetwork,"TDS+HSDPA");
               break;
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_TDS_HSUPA):
               strcpy(tModeNetwork,"TDS+HUSPA");
               break;
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_TDS_HSPA):
               strcpy(tModeNetwork,"TDS+HSPA");
               break;
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_CDMA):
               strcpy(tModeNetwork,"CDMA");
               break;
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_EVDO):
               strcpy(tModeNetwork,"EVDO");
               break;
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_CDMA_EVDO):
               strcpy(tModeNetwork,"CDMA+EVDO");
               break;
            case (sim7600_type_network_mode_t::SIM7600_TYPE_NETWORK_CDMA_LTE):
               strcpy(tModeNetwork,"CDMA+LTE");
               break;
            default:
               strcpy(tModeNetwork,"UNKNOWN");
               break;
         }
         TRACE_VERBOSE_F(F("%s netork mode [ %s ] [ %s ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), tModeNetwork);
         #else
         TRACE_INFO_F(F("%s netork mode [ %s ] [ %d ]\r\n"), SIM7600_NAME, printStatus(at_command_status, OK_STRING, ERROR_STRING), sim7600_cnsmod);
         #endif
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
      snprintf(buffer_cmd, SIM7600_BUFFER_LENGTH, "AT+CGDCONT=1,\"IP\",\"%s\"\r\n", apn);
      at_command_status = sendAtCommand(buffer_cmd, buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

      // success
      if (at_command_status == SIM7600_OK)
      {
         retry = 0;
         sim7600_connection_start_state = SIM7600_CONNECTION_START_PDP_AUTH;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && (++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX)
      {
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
      snprintf(buffer_cmd, SIM7600_BUFFER_LENGTH, "ATD%s\r\n", number);
      at_command_status = sendAtCommand(buffer_cmd, buffer_ext, sizeof(buffer_ext), AT_CONNECT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_DEFAULT_TIMEOUT_MS);

      // success
      if (at_command_status == SIM7600_OK)
      {
         retry = 0;
         sim7600_connection_start_state = SIM7600_CONNECTION_START_END;
      }
      // retry
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
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
   }

   return sim7600_status;
}

sim7600_status_t SIM7600::disconnect(bool bUseEscapeSeq) {
   static uint8_t retry;
   static sim7600_status_t sim7600_status;
   sim7600_status_t at_command_status;
   static bool is_error;
   static bool is_attached;

   switch (sim7600_connection_stop_state)
   {
   case SIM7600_CONNECTION_STOP_INIT:
      retry = 0;
      is_error = false;
      sim7600_status = SIM7600_BUSY;

      if (!bUseEscapeSeq) {
         /* PPP già chiuso: niente +++ (evita timeout/falsi ERROR); guard time + probe AT. */
         TRACE_INFO_F(F("Waiting modem security entering command mode\r\n"), SIM7600_NAME);
         delay_ms = SIM7600_COMMAND_MODE_SWITCH_DELAY_MS;
         sim7600_connection_stop_state = SIM7600_CHECK_ENTER_COMMAND_MODE;
      } else {
         TRACE_INFO_F(F("Starting escape sequence +++\r\n"), SIM7600_NAME);
         delay_ms = SIM7600_COMMAND_MODE_WAIT_DELAY_MS;
         sim7600_connection_stop_state = SIM7600_CONNECTION_ENTER_COMMAND_MODE;
      }
      break;

   case SIM7600_CONNECTION_ENTER_COMMAND_MODE:
      TRACE_VERBOSE_F(F("Attempt sending +++\r\n"), SIM7600_NAME);
      at_command_status = sendAtCommand("+++", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_FASTCMD_TIMEOUT_MS);
      /* Aspetta fine sequenza (BUSY) prima di verificare con AT */
      if (at_command_status == SIM7600_BUSY)
      {
         break;
      }
      delay_ms = SIM7600_COMMAND_MODE_WAIT_DELAY_MS;
      sim7600_connection_stop_state = SIM7600_CHECK_ENTER_COMMAND_MODE;
      break;

   case SIM7600_CHECK_ENTER_COMMAND_MODE:
      at_command_status = sendAtCommand("AT\r\n", buffer_ext, sizeof(buffer_ext), AT_OK_STRING, AT_ERROR_STRING, SIM7600_AT_FASTCMD_TIMEOUT_MS);

      if (at_command_status == SIM7600_OK)
      {
         retry = 0;
         sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_HANGUP;
      }
      else if ((at_command_status == SIM7600_ERROR) && (!bUseEscapeSeq))
      {
         /* Senza +++: un solo tentativo AT poi esci (HW power-off a valle) */
         is_error = true;
         sim7600_connection_stop_state = SIM7600_CONNECTION_STOP_END;
      }
      else if ((at_command_status == SIM7600_ERROR) && ((++retry) < SIM7600_GENERIC_RETRY_COUNT_MAX))
      {
         /* Con +++ fallito: riprova escape poi AT */
         sim7600_connection_stop_state = SIM7600_CONNECTION_ENTER_COMMAND_MODE;
         delay_ms = SIM7600_GENERIC_WAIT_DELAY_MS;
      }
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

      TRACE_INFO_F(F("%s disconnect... [ %s ]\r\n"), SIM7600_NAME, printStatus(sim7600_status, OK_STRING, ERROR_STRING));
      break;
   }

   return sim7600_status;
}

void SIM7600::floatToString(float num, char *tOutStr)
{
   int whole_part = num;
   int digit = 0, reminder = 0;
   int log_value = log10(num), index = log_value;
   long wt = 0;

   //Extract the whole part from float num
   for(int i = 1 ; i < log_value + 2 ; i++)
   {
      wt  =  pow(10.0, i);
      reminder = whole_part  %  wt;
      digit = (reminder - digit) / (wt / 10);
      // Store digit in string
      tOutStr[index--] = digit + 48; // ASCII value of digit  = digit + 48
      if (index == -1) break;    
   }

   index = log_value + 1;
   tOutStr[index] = '.';

   float fraction_part  = num - whole_part;
   float tmp1 = fraction_part,  tmp = 0;

   // Extract the fraction part from num
   for(int i = 0; i < FTS_PRECISION; i++)
   {
      wt = 10; 
      tmp  = tmp1 * wt;
      digit = tmp;
      // Store digit in string
      tOutStr[++index] = digit + 48;  // ASCII value of digit  = digit + 48
      tmp1 = tmp - digit;
   }
}

void SIM7600::convertGpsNMEADecimal(char* tNmeaGPSInfo, char* tDecimalLat, char* tDecimalLon, char* tAltitude)
{
    char cdd[4]  = {}; // MM(M)
    char css[10] = {}; // PP.SSSSSS
    char *altPtr = &tNmeaGPSInfo[45];
    float dd, ss, latDec, lonDec;

    // INFO GPS NMEA Format Examples STRING
    // 5100.505778,N,11404.437214,W,031120,175538.0,1076.7,0.0,348.0
    
    // tNmeaLAT format -> 5100.505778,N
    // convert Latitude from NMEA DDMM.NNNNNN to decimal
    memcpy(cdd, &tNmeaGPSInfo[0], 2);
    memcpy(css, &tNmeaGPSInfo[2], 9);
    dd = strtod(cdd, NULL);      // Convert CDD string to float (DD)
    ss = strtod(css, NULL);      // Convert CSS string to float (SS)
    latDec = (dd + (ss / 60.0)); // Convert for NMEA to Decimal
    if(tNmeaGPSInfo[12] == 'S') latDec = -latDec; // N/S Convert Decimal

    // tNmeaLON format -> 11404.437214,W
    // convert Latitude from NMEA DDMM.NNNNNN to decimal
    memcpy(cdd, &tNmeaGPSInfo[14], 3);
    memcpy(css, &tNmeaGPSInfo[17], 9);
    dd = strtod(cdd, NULL);      // Convert CDD string to float (DD)
    ss = strtod(css, NULL);      // Convert CSS string to float (SS)
    lonDec = (dd + (ss / 60.0)); // Convert for NMEA to Decimal
    if(tNmeaGPSInfo[27] == 'W') lonDec = -lonDec; // N/S Convert Decimal
    floatToString(latDec, tDecimalLat); //Convert Float Latitude to String
    floatToString(lonDec, tDecimalLon); //Convert Float Longitude to String
    while(*altPtr != ',') {
      *tAltitude++ = *altPtr++;
    }
}
