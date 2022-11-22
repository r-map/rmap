/**@file modem_task.cpp */

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

#define TRACE_LEVEL MODEM_TASK_TRACE_LEVEL

#include "tasks/modem_task.h"

using namespace cpp_freertos;

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)

ModemTask::ModemTask(const char *taskName, uint16_t stackSize, uint8_t priority, ModemParam_t modemParam) : Thread(taskName, stackSize, priority), param(modemParam)
{
  state = MODEM_STATE_INIT;
  Start();
};

void ModemTask::Run() {
  error_t error;
  sim7600_status_t status;
  system_request_t request;
  system_response_t response;
  Ipv4Addr ipv4Addr;

  memset(&request, 0, sizeof(system_request_t));
  memset(&response, 0, sizeof(system_response_t));

  while (true)
  {
    switch (state)
    {
    case MODEM_STATE_INIT:
      // Configure the first network interface
      interface = &netInterface[INTERFACE_0_INDEX];

      // Get default PPP settings
      pppGetDefaultSettings(&pppSettings);
      // Select the underlying interface
      pppSettings.interface = interface;
      // Default async control character map
      pppSettings.accm = 0x00000000;
      // Allowed authentication protocols
      pppSettings.authProtocol = PPP_AUTH_PROTOCOL_PAP | PPP_AUTH_PROTOCOL_CHAP_MD5;

      // Initialize PPP
      error = pppInit(&pppContext, &pppSettings);
      // Any error to report?
      if (error)
      {
        // Debug message
        TRACE_ERROR_F(F("Failed to initialize PPP!\r\n"));
        Delay(Ticks::MsToTicks(MODEM_TASK_GENERIC_RETRY_DELAY_MS));
        break;
      }

      // Set interface name
      netSetInterfaceName(interface, INTERFACE_0_NAME);
      // Select the relevant UART driver
      netSetUartDriver(interface, &uartDriver);

      // Initialize network interface
      error = netConfigInterface(interface);
      // Any error to report?
      if (error)
      {
        // Debug message
        TRACE_ERROR_F(F("Failed to configure interface %s!\r\n"), interface->name);
        Delay(Ticks::MsToTicks(MODEM_TASK_GENERIC_RETRY_DELAY_MS));
        break;
      }

      sim7600 = SIM7600(interface, PPP0_BAUD_RATE_DEFAULT, PPP0_BAUD_RATE_MAX, PIN_GSM_EN_POW, PIN_GSM_PW_KEY, PIN_GSM_RI);

      TRACE_VERBOSE_F(F("MODEM_STATE_INIT -> MODEM_STATE_WAIT_NET_EVENT\r\n"));
      state = MODEM_STATE_WAIT_NET_EVENT;
      break;

    case MODEM_STATE_WAIT_NET_EVENT:
      // wait connection request
      if (param.systemRequestQueue->Dequeue(&request, portMAX_DELAY))
      {
        // do connection
        if (request.connection.do_connect)
        {
          TRACE_VERBOSE_F(F("MODEM_STATE_WAIT_NET_EVENT -> MODEM_STATE_SWITCH_ON\r\n"));
          state = MODEM_STATE_SWITCH_ON;
        }
        else if (request.connection.do_disconnect)
        {
          TRACE_VERBOSE_F(F("MODEM_STATE_WAIT_NET_EVENT -> MODEM_STATE_DISCONNECT\r\n"));
          state = MODEM_STATE_DISCONNECT;
        }
        else
        {
          TRACE_VERBOSE_F(F("MODEM_STATE_WAIT_NET_EVENT -> ??? Condizione non gestita!!!\r\n"));
          Thread::Suspend();
        }
      }
      // do something else with non-blocking wait ....
      break;

    case MODEM_STATE_SWITCH_ON:
      status = sim7600.switchOn();
      Delay(Ticks::MsToTicks(sim7600.getDelayMs()));

      if (status == SIM7600_OK)
      {
        state = MODEM_STATE_SETUP;
        TRACE_VERBOSE_F(F("MODEM_STATE_SWITCH_ON -> MODEM_STATE_SETUP\r\n"));
      }
      else if (status == SIM7600_ERROR)
      {
        state = MODEM_STATE_END;
        TRACE_VERBOSE_F(F("MODEM_STATE_SWITCH_ON -> MODEM_STATE_END\r\n"));
      }
      // Wait...
      break;

    case MODEM_STATE_SETUP:
      status = sim7600.setup();
      Delay(Ticks::MsToTicks(sim7600.getDelayMs()));

      if (status == SIM7600_OK)
      {
        state = MODEM_STATE_CONNECT;
        TRACE_VERBOSE_F(F("MODEM_STATE_SETUP -> MODEM_STATE_CONNECT\r\n"));
      }
      else if (status == SIM7600_ERROR)
      {
        state = MODEM_STATE_END;
        TRACE_VERBOSE_F(F("MODEM_STATE_SETUP -> MODEM_STATE_END\r\n"));
      }
      break;

    case MODEM_STATE_CONNECT:
      status = sim7600.connect("internet.wind");
      Delay(Ticks::MsToTicks(sim7600.getDelayMs()));

      if (status == SIM7600_OK)
      {
        // Clear local IPv4 address
        ipv4StringToAddr("0.0.0.0", &ipv4Addr);
        ipv4SetHostAddr(interface, ipv4Addr);

        // Clear peer IPv4 address
        ipv4StringToAddr("0.0.0.0", &ipv4Addr);
        ipv4SetDefaultGateway(interface, ipv4Addr);

        // Set primary DNS server
        ipv4StringToAddr(PPP0_PRIMARY_DNS, &ipv4Addr);
        ipv4SetDnsServer(interface, 0, ipv4Addr);

        // Set secondary DNS server
        ipv4StringToAddr(PPP0_SECONDARY_DNS, &ipv4Addr);
        ipv4SetDnsServer(interface, 1, ipv4Addr);

        // Set username and password
        pppSetAuthInfo(interface, "", "");

        // Debug message
        TRACE_INFO_F(F("Establishing PPP connection...\r\n"));

        // Establish a PPP connection
        error = pppConnect(interface);
        // Any error to report?
        if (error)
        {
          TRACE_ERROR_F(F("Failed to established PPP connection!\r\n"));
          state = MODEM_STATE_DISCONNECT;
          TRACE_VERBOSE_F(F("MODEM_STATE_CONNECT -> MODEM_STATE_DISCONNECT\r\n"));
        }

        state = MODEM_STATE_WAIT_NET_EVENT;
        TRACE_VERBOSE_F(F("MODEM_STATE_CONNECT -> MODEM_STATE_WAIT_NET_EVENT\r\n"));
      }
      else if (status == SIM7600_ERROR)
      {
        state = MODEM_STATE_DISCONNECT;
        TRACE_VERBOSE_F(F("MODEM_STATE_CONNECT -> MODEM_STATE_DISCONNECT\r\n"));
      }
      break;

    case MODEM_STATE_DISCONNECT:
      Thread::Suspend();
      // status = sim7600.disconnect();
      // Delay(Ticks::MsToTicks(sim7600.getDelayMs()));

      if (status == SIM7600_OK)
      {
        state = MODEM_STATE_SWITCH_OFF;
        TRACE_VERBOSE_F(F("MODEM_STATE_DISCONNECT -> MODEM_STATE_SWITCH_OFF\r\n"));
      }
      else if (status == SIM7600_ERROR)
      {
        state = MODEM_STATE_END;
        TRACE_VERBOSE_F(F("MODEM_STATE_DISCONNECT -> MODEM_STATE_END\r\n"));
      }
      break;

    case MODEM_STATE_SWITCH_OFF:
      status = sim7600.switchOff();
      Delay(Ticks::MsToTicks(sim7600.getDelayMs()));

      if (status == SIM7600_OK)
      {
        state = MODEM_STATE_END;
        TRACE_VERBOSE_F(F("MODEM_STATE_SWITCH_OFF -> MODEM_STATE_END\r\n"));
      }
      else if (status == SIM7600_ERROR)
      {
        state = MODEM_STATE_END;
        TRACE_VERBOSE_F(F("MODEM_STATE_SWITCH_OFF -> MODEM_STATE_END\r\n"));
      }
      break;

    case MODEM_STATE_END:
      Thread::Suspend();
      state = MODEM_STATE_WAIT_NET_EVENT;
      TRACE_VERBOSE_F(F("MODEM_STATE_END -> MODEM_STATE_WAIT_NET_EVENT\r\n"));
      break;
    }
  }
}

#endif