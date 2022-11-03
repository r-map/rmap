/**@file net_util.cpp */

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
