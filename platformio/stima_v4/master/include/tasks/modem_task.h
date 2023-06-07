/**
 ******************************************************************************
 * @file    modem_task.h
 * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
 * @author  Moreno Gasperini <m.gasperini@digiteco.it>
 * @brief   Modem connection header file
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (C) 2022  Moreno Gasperini <m.baldinetti@digiteco.it>
 * All rights reserved.</center></h2>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************
*/

#ifndef _MODEM_TASK_H
#define _MODEM_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "str.h"

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)

#define MODEM_TASK_WAIT_DELAY_MS          (100)
#define MODEM_TASK_GENERIC_RETRY_DELAY_MS (5000)
#define MODEM_TASK_GENERIC_RETRY          (3)

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "queue.hpp"
#include "drivers/module_master_hal.hpp"
#include "core/net.h"
#include "ppp/ppp.h"
#include "drivers/modem/sim7600.h"
#include "drivers/uart/uart_driver.h"
#include "debug_F.h"

using namespace cpp_freertos;

typedef enum
{
  MODEM_STATE_CREATE,
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

typedef struct
{
  configuration_t *configuration;
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::Queue *systemMessageQueue;
  cpp_freertos::Queue *dataLogPutQueue;
  cpp_freertos::Queue *connectionRequestQueue;
  cpp_freertos::Queue *connectionResponseQueue;
} ModemParam_t;

class ModemTask : public cpp_freertos::Thread {

public:
  ModemTask(const char *taskName, uint16_t stackSize, uint8_t priority, ModemParam_t modemParam);

protected:
  NetInterface *interface;
  virtual void Run();

private:

  #if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
  #endif
  void TaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  ModemState_t state;
  ModemParam_t param;

  SIM7600 sim7600;
  PppSettings pppSettings;
  PppContext pppContext;

  char apn[GSM_APN_LENGTH];
  char number[GSM_NUMBER_LENGTH];
  char username[GSM_USERNAME_LENGTH];
  char password[GSM_PASSWORD_LENGTH];
};

#endif
#endif
