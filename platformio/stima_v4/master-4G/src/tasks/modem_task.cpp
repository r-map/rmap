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

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)

using namespace cpp_freertos;

ModemTask::ModemTask(const char *taskName, uint16_t stackSize, uint8_t priority, ModemParam_t modemParam) : Thread(taskName, stackSize, priority), ModemParam(modemParam) {
  state = MODEM_STATE_INIT;
  Start();
};

void ModemTask::Run() {
  // error_t error;

  digitalWrite(PIN_GSM_EN_POW, HIGH);
  digitalWrite(PIN_GSM_PW_KEY, HIGH);
  DelayUntil(Ticks::MsToTicks(500));
  digitalWrite(PIN_GSM_PW_KEY, LOW);
  Serial2.begin(115200);

  while (true)
  {
    if (Serial2.available())
    {
      Serial.write(Serial2.read());
    }
    if (Serial.available())
    {
      Serial2.write(Serial.read());
    }
    // DelayUntil(Ticks::MsToTicks(1));
    // switch (state)
    // {
    // case MODEM_STATE_INIT:
    //   TRACE_INFO_F(F("Modem %s\r\n"), "TASK");
    //   Thread::Suspend();
    //   // // Set interface name
    //   // netSetInterfaceName(ModemParam.interface, APP_IF_NAME);
    //   // // Set host name
    //   // netSetHostname(ModemParam.interface, APP_HOST_NAME);
    //   // // Set host MAC address
    //   // macStringToAddr(APP_MAC_ADDR, &ModemParam.macAddr);
    //   // netSetMacAddr(ModemParam.interface, &ModemParam.macAddr);
    //   // // Select the relevant spi driver
    //   // netSetSpiDriver(ModemParam.interface, &spiDriver);
    //   // // Select the relevant ext int driver
    //   // // netSetExtIntDriver(ModemParam.interface, &extIntDriver);
    //   // // Select the relevant network adapter
    //   // netSetDriver(ModemParam.interface, &enc28j60Driver);

    //   // // Initialize network interface
    //   // error = netConfigInterface(ModemParam.interface);
    //   // // Any error to report?
    //   // if (error) {
    //   //   // Debug message
    //   //   TRACE_ERROR("Failed to configure interface %s!\r\n", ModemParam.interface->name);
    //   // }

    //   // #if (IPV4_SUPPORT == ENABLED)
    //   // #if (APP_USE_DHCP_CLIENT == ENABLED)
    //   // // Get default settings
    //   // dhcpClientGetDefaultSettings(&ModemParam.dhcpClientSettings);
    //   // // Set the network interface to be configured by DHCP
    //   // ModemParam.dhcpClientSettings.interface = ModemParam.interface;
    //   // // Disable rapid commit option
    //   // ModemParam.dhcpClientSettings.rapidCommit = FALSE;

    //   // // DHCP client initialization
    //   // error = dhcpClientInit(&ModemParam.dhcpClientContext, &ModemParam.dhcpClientSettings);
    //   // // Failed to initialize DHCP client?
    //   // if (error)
    //   // {
    //   //   // Debug message
    //   //   TRACE_ERROR("Failed to initialize DHCP client!\r\n");
    //   // }

    //   // // Start DHCP client
    //   // error = dhcpClientStart(&ModemParam.dhcpClientContext);
    //   // // Failed to start DHCP client?
    //   // if (error)
    //   // {
    //   //   // Debug message
    //   //   TRACE_ERROR("Failed to start DHCP client!\r\n");
    //   // }
    //   // #else
    //   // // Set IPv4 host address
    //   // ipv4StringToAddr(APP_IPV4_HOST_ADDR, &ModemParam.ipv4Addr);
    //   // ipv4SetHostAddr(ModemParam.interface, ModemParam.ipv4Addr);

    //   // // Set subnet mask
    //   // ipv4StringToAddr(APP_IPV4_SUBNET_MASK, &ModemParam.ipv4Addr);
    //   // ipv4SetSubnetMask(ModemParam.interface, ModemParam.ipv4Addr);

    //   // // Set default gateway
    //   // ipv4StringToAddr(APP_IPV4_DEFAULT_GATEWAY, &ModemParam.ipv4Addr);
    //   // ipv4SetDefaultGateway(ModemParam.interface, ModemParam.ipv4Addr);

    //   // // Set primary and secondary DNS servers
    //   // ipv4StringToAddr(APP_IPV4_PRIMARY_DNS, &ModemParam.ipv4Addr);
    //   // ipv4SetDnsServer(ModemParam.interface, 0, ModemParam.ipv4Addr);
    //   // ipv4StringToAddr(APP_IPV4_SECONDARY_DNS, &ModemParam.ipv4Addr);
    //   // ipv4SetDnsServer(ModemParam.interface, 1, ModemParam.ipv4Addr);
    //   // #endif
    //   // #endif

    //   // #if (IPV6_SUPPORT == ENABLED)
    //   // #if (APP_USE_SLAAC == ENABLED)
    //   // // Get default settings
    //   // slaacGetDefaultSettings(&ModemParam.slaacSettings);
    //   // // Set the network interface to be configured
    //   // ModemParam.slaacSettings.interface = ModemParam.interface;

    //   // // SLAAC initialization
    //   // error = slaacInit(&ModemParam.slaacContext, &ModemParam.slaacSettings);
    //   // // Failed to initialize SLAAC?
    //   // if (error)
    //   // {
    //   //   // Debug message
    //   //   TRACE_ERROR("Failed to initialize SLAAC!\r\n");
    //   // }

    //   // // Start IPv6 address autoconfiguration process
    //   // error = slaacStart(&ModemParam.slaacContext);
    //   // // Failed to start SLAAC process?
    //   // if (error)
    //   // {
    //   //   // Debug message
    //   //   TRACE_ERROR("Failed to start SLAAC!\r\n");
    //   // }
    //   // #else
    //   // // Set link-local address
    //   // ipv6StringToAddr(APP_IPV6_LINK_LOCAL_ADDR, &ModemParam.ipv6Addr);
    //   // ipv6SetLinkLocalAddr(ModemParam.interface, &ModemParam.ipv6Addr);

    //   // // Set IPv6 prefix
    //   // ipv6StringToAddr(APP_IPV6_PREFIX, &ModemParam.ipv6Addr);
    //   // ipv6SetPrefix(ModemParam.interface, 0, &ModemParam.ipv6Addr, APP_IPV6_PREFIX_LENGTH);

    //   // // Set global address
    //   // ipv6StringToAddr(APP_IPV6_GLOBAL_ADDR, &ModemParam.ipv6Addr);
    //   // ipv6SetGlobalAddr(ModemParam.interface, 0, &ModemParam.ipv6Addr);

    //   // // Set default router
    //   // ipv6StringToAddr(APP_IPV6_ROUTER, &ModemParam.ipv6Addr);
    //   // ipv6SetDefaultRouter(ModemParam.interface, 0, &ModemParam.ipv6Addr);

    //   // // Set primary and secondary DNS servers
    //   // ipv6StringToAddr(APP_IPV6_PRIMARY_DNS, &ModemParam.ipv6Addr);
    //   // ipv6SetDnsServer(ModemParam.interface, 0, &ModemParam.ipv6Addr);
    //   // ipv6StringToAddr(APP_IPV6_SECONDARY_DNS, &ModemParam.ipv6Addr);
    //   // ipv6SetDnsServer(ModemParam.interface, 1, &ModemParam.ipv6Addr);
    //   // #endif
    //   // #endif

    //   state = MODEM_STATE_EVENT_HANDLER;
    //   break;

    //   case MODEM_STATE_EVENT_HANDLER:
    //     // // Thread::Suspend();
    //     // enc28j60IrqHandler(ModemParam.interface);
    //     // Delay(Ticks::MsToTicks(ModemParam.tickHandlerMs));
    //   break;

    //   case MODEM_STATE_END:
    //   break;
    //   }
  }
}

#endif