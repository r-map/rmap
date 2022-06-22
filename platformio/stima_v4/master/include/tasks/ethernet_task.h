/**
 * @file ethernet_task.h
 * @brief Ethernet Task
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2022 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneCRYPTO Open.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.1.4
 **/

#ifndef _ETHERNET_TASK_H
#define _ETHERNET_TASK_H

#include "hardware_config.h"
#include "net_config.h"
#include "mydebug.h"
#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"

#include "core/net.h"
#include "drivers/spi/arduino_spi_driver.h"
#include "drivers/eth/enc28j60_driver.h"

typedef enum {
   ETHERNET_STATE_INIT,
   ETHERNET_STATE_EVENT_HANDLER,
   ETHERNET_STATE_END
} EthernetState_t;

typedef struct {
  NetInterface *interface;
  MacAddr macAddr;
  #if (IPV4_SUPPORT == ENABLED)
  #if (APP_USE_DHCP_CLIENT == DISABLED)
  Ipv4Addr ipv4Addr;
  #endif
  #endif
  #if (IPV6_SUPPORT == ENABLED)
  #if (APP_USE_SLAAC == DISABLED)
  Ipv6Addr ipv6Addr;
  #endif
  #endif
  DhcpClientSettings dhcpClientSettings;
  DhcpClientContext dhcpClientContext;
  SlaacSettings slaacSettings;
  SlaacContext slaacContext;
  uint16_t tickHandlerMs;
} EthernetParam_t;

class EthernetTask : public cpp_freertos::Thread {

public:
  EthernetTask(const char *taskName, uint16_t stackSize, uint8_t priority, EthernetParam_t EthernetParam);

protected:
  virtual void Run();

private:
  char taskName[configMAX_TASK_NAME_LEN];
  uint16_t stackSize;
  uint8_t priority;
  EthernetState_t state;
  EthernetParam_t EthernetParam;
};

#endif
