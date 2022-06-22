#include "net_util.h"

bool isNetReady(NetInterface *interface, uint8_t addrListIndex) {
  if (!netGetLinkState(interface)) {
    return false;
  }

  #if (IPV4_SUPPORT == ENABLED)
  if (interface->ipv4Context.addrList[addrListIndex].state != IPV4_ADDR_STATE_VALID) {
    return false;
  }
  #endif

  #if (IPV6_SUPPORT == ENABLED)
  #endif

  return true;
}
