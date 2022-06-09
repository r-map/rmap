#include "tasks/ethernet_task.h"

using namespace cpp_freertos;

EthernetTask::EthernetTask(uint16_t stackSize, uint8_t priority, EthernetParam_t ethernetParam) : Thread(stackSize, priority),
EthernetParam(ethernetParam)
{
  Start();
};

void EthernetTask::Run() {
  bool runOnce = true;

  error_t error;

  while (true) {
    if (runOnce) {
      runOnce = false;

      // Set interface name
      netSetInterfaceName(EthernetParam.interface, APP_IF_NAME);
      // Set host name
      netSetHostname(EthernetParam.interface, APP_HOST_NAME);
      // Set host MAC address
      macStringToAddr(APP_MAC_ADDR, &EthernetParam.macAddr);
      netSetMacAddr(EthernetParam.interface, &EthernetParam.macAddr);
      // Select the relevant spi driver
      netSetSpiDriver(EthernetParam.interface, &spiDriver);
      // Select the relevant ext int driver
      netSetExtIntDriver(EthernetParam.interface, &extIntDriver);
      // Select the relevant network adapter
      netSetDriver(EthernetParam.interface, &enc28j60Driver);

      // Initialize network interface
      error = netConfigInterface(EthernetParam.interface);
      // Any error to report?
      if (error) {
        // Debug message
        TRACE_ERROR("Failed to configure interface %s!\r\n", EthernetParam.interface->name);
      }

      TRACE_INFO("TASK\r\n");

      #if (IPV4_SUPPORT == ENABLED)
      #if (APP_USE_DHCP_CLIENT == ENABLED)
      // Get default settings
      dhcpClientGetDefaultSettings(&EthernetParam.dhcpClientSettings);
      // Set the network interface to be configured by DHCP
      EthernetParam.dhcpClientSettings.interface = EthernetParam.interface;
      // Disable rapid commit option
      EthernetParam.dhcpClientSettings.rapidCommit = FALSE;

      // DHCP client initialization
      error = dhcpClientInit(&EthernetParam.dhcpClientContext, &EthernetParam.dhcpClientSettings);
      // Failed to initialize DHCP client?
      if (error)
      {
        // Debug message
        TRACE_ERROR("Failed to initialize DHCP client!\r\n");
      }

      // Start DHCP client
      error = dhcpClientStart(&EthernetParam.dhcpClientContext);
      // Failed to start DHCP client?
      if (error)
      {
        // Debug message
        TRACE_ERROR("Failed to start DHCP client!\r\n");
      }
      #else
      // Set IPv4 host address
      ipv4StringToAddr(APP_IPV4_HOST_ADDR, &EthernetParam.ipv4Addr);
      ipv4SetHostAddr(EthernetParam.interface, EthernetParam.ipv4Addr);

      // Set subnet mask
      ipv4StringToAddr(APP_IPV4_SUBNET_MASK, &EthernetParam.ipv4Addr);
      ipv4SetSubnetMask(EthernetParam.interface, EthernetParam.ipv4Addr);

      // Set default gateway
      ipv4StringToAddr(APP_IPV4_DEFAULT_GATEWAY, &EthernetParam.ipv4Addr);
      ipv4SetDefaultGateway(EthernetParam.interface, EthernetParam.ipv4Addr);

      // Set primary and secondary DNS servers
      ipv4StringToAddr(APP_IPV4_PRIMARY_DNS, &EthernetParam.ipv4Addr);
      ipv4SetDnsServer(EthernetParam.interface, 0, EthernetParam.ipv4Addr);
      ipv4StringToAddr(APP_IPV4_SECONDARY_DNS, &EthernetParam.ipv4Addr);
      ipv4SetDnsServer(EthernetParam.interface, 1, EthernetParam.ipv4Addr);
      #endif
      #endif

      #if (IPV6_SUPPORT == ENABLED)
      #if (APP_USE_SLAAC == ENABLED)
      // Get default settings
      slaacGetDefaultSettings(&EthernetParam.slaacSettings);
      // Set the network interface to be configured
      EthernetParam.slaacSettings.interface = EthernetParam.interface;

      // SLAAC initialization
      error = slaacInit(&EthernetParam.slaacContext, &EthernetParam.slaacSettings);
      // Failed to initialize SLAAC?
      if (error)
      {
        // Debug message
        TRACE_ERROR("Failed to initialize SLAAC!\r\n");
      }

      // Start IPv6 address autoconfiguration process
      error = slaacStart(&EthernetParam.slaacContext);
      // Failed to start SLAAC process?
      if (error)
      {
        // Debug message
        TRACE_ERROR("Failed to start SLAAC!\r\n");
      }
      #else
      // Set link-local address
      ipv6StringToAddr(APP_IPV6_LINK_LOCAL_ADDR, &EthernetParam.ipv6Addr);
      ipv6SetLinkLocalAddr(EthernetParam.interface, &EthernetParam.ipv6Addr);

      // Set IPv6 prefix
      ipv6StringToAddr(APP_IPV6_PREFIX, &EthernetParam.ipv6Addr);
      ipv6SetPrefix(EthernetParam.interface, 0, &EthernetParam.ipv6Addr, APP_IPV6_PREFIX_LENGTH);

      // Set global address
      ipv6StringToAddr(APP_IPV6_GLOBAL_ADDR, &EthernetParam.ipv6Addr);
      ipv6SetGlobalAddr(EthernetParam.interface, 0, &EthernetParam.ipv6Addr);

      // Set default router
      ipv6StringToAddr(APP_IPV6_ROUTER, &EthernetParam.ipv6Addr);
      ipv6SetDefaultRouter(EthernetParam.interface, 0, &EthernetParam.ipv6Addr);

      // Set primary and secondary DNS servers
      ipv6StringToAddr(APP_IPV6_PRIMARY_DNS, &EthernetParam.ipv6Addr);
      ipv6SetDnsServer(EthernetParam.interface, 0, &EthernetParam.ipv6Addr);
      ipv6StringToAddr(APP_IPV6_SECONDARY_DNS, &EthernetParam.ipv6Addr);
      ipv6SetDnsServer(EthernetParam.interface, 1, &EthernetParam.ipv6Addr);
      #endif
      #endif
      Thread::Suspend();
    }
  }
}
