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

#include "debug_config.h"
#include "local_typedef.h"

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)

#define MODEM_TASK_WAIT_DELAY_MS          (10)
#define MODEM_TASK_GENERIC_RETRY_DELAY_MS (5000)
#define MODEM_TASK_GENERIC_RETRY          (3)

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "queue.hpp"
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
#include "drivers/uart/uart_driver.h"
#include "debug_F.h"

typedef enum
{
  MODEM_STATE_INIT,
  MODEM_STATE_WAIT_NET_EVENT,
  MODEM_STATE_SWITCH_ON,
  MODEM_STATE_SETUP,
  MODEM_STATE_CONNECT,
  MODEM_STATE_CONNECTED,
  MODEM_STATE_DISCONNECT,
  MODEM_STATE_SWITCH_OFF,
  MODEM_STATE_END
} ModemState_t;

// typedef enum
// {
//   GSM_INIT,                //!< init task variables
//   GSM_SWITCH_ON,           //!< gsm power on
//   GSM_AUTOBAUD,            //!< gsm autobaud procedure
//   GSM_SETUP,               //!< gsm setup
//   GSM_START_CONNECTION,    //!< gsm open connection
//   GSM_CHECK_OPERATION,     //!< check operations (ntp or mqtt)
//   GSM_OPEN_UDP_SOCKET,     //!< open udp socket for ntp sync
//   GSM_SUSPEND,             //!< wait other tasks for complete its operations with gsm
//   GSM_STOP_CONNECTION,     //!< gsm close connection
//   GSM_WAIT_FOR_SWITCH_OFF, //!< wait gsm for power off
//   GSM_SWITCH_OFF,          //!< gsm power off
//   GSM_END,                 //!< performs end operations and deactivate task
//   GSM_WAIT_STATE           //!< non-blocking waiting time
// } gsm_state_t;

typedef struct
{
  configuration_t *configuration;
  cpp_freertos::Queue *systemStatusQueue;
  cpp_freertos::Queue *systemRequestQueue;
  cpp_freertos::Queue *systemResponseQueue;
} ModemParam_t;

class ModemTask : public cpp_freertos::Thread {

public:
  ModemTask(const char *taskName, uint16_t stackSize, uint8_t priority, ModemParam_t ModemParam);

protected:
  NetInterface *interface;
  virtual void Run();

private:
  char taskName[configMAX_TASK_NAME_LEN];
  uint16_t stackSize;
  uint8_t priority;
  ModemState_t state;
  ModemParam_t param;

  SIM7600 sim7600;
  PppSettings pppSettings;
  PppContext pppContext;
  // HttpClientContext httpClientContext;
};

#endif
#endif
