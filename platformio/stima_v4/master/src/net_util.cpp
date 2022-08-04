#define TRACE_LEVEL ERROR_TRACE_LEVEL

#include "net_util.h"

bool isNetReady(NetInterface *interface, uint8_t addrListIndex) {
  //Use default network interface?
  if(interface == NULL) {
     interface = netGetDefaultInterface();
  }

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

error_t initCPRNG (YarrowContext *yarrowContext) {
  // Global variables
  uint8_t seed[32];
  error_t error;

  // Generate a random seed
  error = trngGetRandomData(seed, sizeof(seed));
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR("Failed to generate random data!\r\n");
  }

  // PRNG initialization
  error = yarrowInit(yarrowContext);
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR("Failed to initialize PRNG!\r\n");
  }

  // Properly seed the PRNG
  error = yarrowSeed(yarrowContext, seed, sizeof(seed));
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR("Failed to seed PRNG!\r\n");
  }

  return error;
}
