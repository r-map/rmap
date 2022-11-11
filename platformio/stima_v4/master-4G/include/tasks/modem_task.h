/**@file modem_task.h */

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

#ifndef _MODEM_TASK_H
#define _MODEM_TASK_H

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)

#include "debug_config.h"
#include "local_typedef.h"
#include "STM32FreeRTOS.h"
#include "thread.hpp"
#include "ticks.hpp"
#include "drivers/module_master_hal.hpp"
#include "core/net.h"
#include "ppp/ppp.h"
#include "http/http_client.h"
#include "tls.h"
#include "tls_cipher_suites.h"
#include "hardware/stm32l4xx/stm32l4xx_crypto.h"
#include "rng/trng.h"
#include "rng/yarrow.h"
#include "drivers/modem/sim7600.h"
#include "drivers/uart/arduino_uart_driver.h"
#include "debug_F.h"

typedef enum {
   MODEM_STATE_INIT,
   MODEM_STATE_EVENT_HANDLER,
   MODEM_STATE_END
} ModemState_t;

typedef struct {
  NetInterface *interface;
  // MacAddr macAddr;
  // #if (IPV4_SUPPORT == ENABLED)
  // #if (APP_USE_DHCP_CLIENT == DISABLED)
  // Ipv4Addr ipv4Addr;
  // #endif
  // #endif
  // #if (IPV6_SUPPORT == ENABLED)
  // #if (APP_USE_SLAAC == DISABLED)
  // Ipv6Addr ipv6Addr;
  // #endif
  // #endif
  // DhcpClientSettings dhcpClientSettings;
  // DhcpClientContext dhcpClientContext;
  // SlaacSettings slaacSettings;
  // SlaacContext slaacContext;
  // uint16_t tickHandlerMs;
} ModemParam_t;

class ModemTask : public cpp_freertos::Thread {

public:
  ModemTask(const char *taskName, uint16_t stackSize, uint8_t priority, ModemParam_t ModemParam);

protected:
  virtual void Run();

private:
  char taskName[configMAX_TASK_NAME_LEN];
  uint16_t stackSize;
  uint8_t priority;
  ModemState_t state;
  ModemParam_t ModemParam;
};

#endif
#endif
