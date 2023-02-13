/**@file http_task.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@digiteco.it>
authors:
Marco Baldinetti <marco.baldinetti@digiteco.it>

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

#ifndef _HTTP_TASK_H
#define _HTTP_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "str.h"
#include "stima_utility.h"

#if (USE_HTTP)

#define HTTP_TASK_WAIT_DELAY_MS (100)
#define HTTP_TASK_GENERIC_RETRY_DELAY_MS (5000)
#define HTTP_TASK_GENERIC_RETRY (3)

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_master_hal.hpp"
#include "core/net.h"
#include "http/http_client.h"
#include "debug_F.h"

using namespace cpp_freertos;

typedef enum
{
  HTTP_STATE_CREATE,
  HTTP_STATE_INIT,
  HTTP_STATE_WAIT_NET_EVENT,
  HTTP_STATE_SEND_REQUEST,
  HTTP_STATE_GET_RESPONSE,
  HTTP_STATE_END
} HttpState_t;

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::Queue *connectionRequestQueue;
  cpp_freertos::Queue *connectionResponseQueue;
} HttpParam_t;

class HttpTask : public cpp_freertos::Thread {

public:
  HttpTask(const char *taskName, uint16_t stackSize, uint8_t priority, HttpParam_t httpParam);

protected:
  char_t http_buffer[HTTP_BUFFER_SIZE];
  size_t http_buffer_length;
  virtual void Run();

private:

  #if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
  #endif
  void TaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  HttpState_t state;
  HttpParam_t param;
  HttpClientContext httpClientContext;
};

#endif
#endif
