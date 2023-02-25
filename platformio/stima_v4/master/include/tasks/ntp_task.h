/**@file ntp_task.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>

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

#ifndef _NTP_TASK_H
#define _NTP_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "str.h"

#if (USE_NTP)

#define NTP_TASK_WAIT_DELAY_MS (100)
#define NTP_TASK_GENERIC_RETRY_DELAY_MS (5000)
#define NTP_TASK_GENERIC_RETRY (3)

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_master_hal.hpp"
#include "core/net.h"
#include "sntp/sntp_client.h"
#include "date_time.h"
#include "debug_F.h"
#include "config.h"
#include "debug_config.h"
#include "str.h"

#include <STM32RTC.h>

using namespace cpp_freertos;

typedef enum
{
  NTP_STATE_CREATE,
  NTP_STATE_INIT,
  NTP_STATE_WAIT_NET_EVENT,
  NTP_STATE_DO_NTP_SYNC,
  NTP_STATE_END
} NtpState_t;

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::BinarySemaphore *rtcLock;
  cpp_freertos::Queue *systemMessageQueue;
  cpp_freertos::Queue *dataLogPutQueue;
  cpp_freertos::Queue *connectionRequestQueue;
  cpp_freertos::Queue *connectionResponseQueue;
} NtpParam_t;

class NtpTask : public cpp_freertos::Thread {

public:
  NtpTask(const char *taskName, uint16_t stackSize, uint8_t priority, NtpParam_t ntpParam);

protected:
  virtual void Run();

private:

  #if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
  #endif
  void TaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  STM32RTC &rtc = STM32RTC::getInstance();

  NtpState_t state;
  NtpParam_t param;
  SntpClientContext sntpClientContext;
};

#endif
#endif
