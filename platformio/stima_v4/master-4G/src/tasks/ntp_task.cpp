/**@file ntp_task.cpp */

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

#define TRACE_LEVEL NTP_TASK_TRACE_LEVEL

#include "tasks/ntp_task.h"

using namespace cpp_freertos;

#if (USE_NTP)

NtpTask::NtpTask(const char *taskName, uint16_t stackSize, uint8_t priority, NtpParam_t ntpParam) : Thread(taskName, stackSize, priority), param(ntpParam)
{
  state = NTP_STATE_INIT;
  Start();
};

void NtpTask::Run() {
  uint8_t retry;
  bool is_error;
  error_t error;
  uint32_t kissCode;
  time_t unixTime;
  DateTime date;
  IpAddr ipAddr;
  NtpTimestamp timestamp;

  system_request_t request;
  system_response_t response;

  while (true)
  {
    memset(&request, 0, sizeof(system_request_t));
    memset(&response, 0, sizeof(system_response_t));

    switch (state)
    {
    case NTP_STATE_INIT:
      TRACE_VERBOSE_F(F("NTP_STATE_INIT -> NTP_STATE_WAIT_NET_EVENT\r\n"));
      state = NTP_STATE_WAIT_NET_EVENT;
      break;

    case NTP_STATE_WAIT_NET_EVENT:
      is_error = false;
      retry = 0;

      // wait connection request
      if (param.systemRequestQueue->Peek(&request, portMAX_DELAY))
      {
        // do ntp sync
        if (request.connection.do_ntp_sync)
        {
          param.systemRequestQueue->Dequeue(&request, 0);
          TRACE_VERBOSE_F(F("NTP_STATE_WAIT_NET_EVENT -> NTP_STATE_DO_NTP_SYNC\r\n"));
          state = NTP_STATE_DO_NTP_SYNC;
        }
        // other
        else
        {
          Delay(Ticks::MsToTicks(NTP_TASK_WAIT_DELAY_MS));
        }
      }
      // do something else with non-blocking wait ....
      break;

    case NTP_STATE_DO_NTP_SYNC:
      sntpClientInit(&sntpClientContext);

      param.systemStatusLock->Take();
      param.system_status->connection.is_ntp_synchronizing = true;
      param.systemStatusLock->Give();
      
      TRACE_INFO_F(F("%s Resolving ntp server name of %s \r\n"), Thread::GetName().c_str(), param.configuration->ntp_server);

      // Resolve NTP server name
      error = getHostByName(NULL, param.configuration->ntp_server, &ipAddr, 0);
      // Any error to report?
      if (error)
      {
        is_error = true;
        state = NTP_STATE_END;
        TRACE_VERBOSE_F(F("NTP_STATE_DO_NTP_SYNC -> NTP_STATE_END\r\n"));

        TRACE_ERROR_F(F("%s Failed to resolve ntp server name of %s\r\n"), Thread::GetName().c_str(), param.configuration->ntp_server);
        break;
      }

      // Set timeout value for blocking operations
      error = sntpClientSetTimeout(&sntpClientContext, SNTP_CLIENT_TIMEOUT_MS);
      // Any error to report?
      if (error)
      {
        is_error = true;
        state = NTP_STATE_END;
        TRACE_VERBOSE_F(F("NTP_STATE_DO_NTP_SYNC -> NTP_STATE_END\r\n"));
        break;
      }

      // Specify the IP address of the NTP server
      error = sntpClientSetServerAddr(&sntpClientContext, &ipAddr, NTP_PORT);
      // Any error to report?
      if (error)
      {
        is_error = true;
        state = NTP_STATE_END;
        TRACE_VERBOSE_F(F("NTP_STATE_DO_NTP_SYNC -> NTP_STATE_END\r\n"));
        break;
      }

      // Retrieve current time from NTP server
      error = sntpClientGetTimestamp(&sntpClientContext, &timestamp);
      // Check status code
      if (!error)
      {
        // Unix time starts on January 1st, 1970
        unixTime = timestamp.seconds - NTP_UNIX_EPOCH;

        // system date and time
        param.systemStatusLock->Take();
        param.system_status->datetime.system_time = (uint32_t) unixTime;
        param.systemStatusLock->Give();

        // Convert Unix timestamp to date
        convertUnixTimeToDate(unixTime, &date);

        // Debug message
        TRACE_INFO_F(F("%s ntp current date/time [ %d ] %s\r\n"), Thread::GetName().c_str(), (uint32_t)unixTime, formatDate(&date, NULL));

        state = NTP_STATE_END;
        TRACE_VERBOSE_F(F("NTP_STATE_DO_NTP_SYNC -> NTP_STATE_END\r\n"));
      }
      else if (error == ERROR_REQUEST_REJECTED)
      {
        // Retrieve kiss code
        kissCode = sntpClientGetKissCode(&sntpClientContext);

        TRACE_ERROR_F(F("%s ntp received kiss code: '%c%c%c%c' [ %s ]\r\n"), Thread::GetName().c_str(), (kissCode >> 24) & 0xFF, (kissCode >> 16) & 0xFF, (kissCode >> 8) & 0xFF, kissCode & 0xFF, ERROR_STRING);

        is_error = true;
        state = NTP_STATE_END;
        TRACE_VERBOSE_F(F("NTP_STATE_DO_NTP_SYNC -> NTP_STATE_END\r\n"));
      }
      else
      {
        TRACE_ERROR_F(F("%s Failed to retrieve ntp timestamp [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);

        is_error = true;
        state = NTP_STATE_END;
        TRACE_VERBOSE_F(F("NTP_STATE_DO_NTP_SYNC -> NTP_STATE_END\r\n"));
      }
      break;

    case NTP_STATE_END:
      // ok
      if (!is_error)
      {
        response.connection.done_ntp_synchronized = true;
        param.systemResponseQueue->Enqueue(&response, 0);

        param.systemStatusLock->Take();
        param.system_status->connection.is_ntp_synchronizing = false;
        param.system_status->connection.is_ntp_synchronized = true;
        param.systemStatusLock->Give();

        sntpClientDeinit(&sntpClientContext);
        state = NTP_STATE_INIT;
        TRACE_VERBOSE_F(F("NTP_STATE_END -> NTP_STATE_INIT\r\n"));
      }
      // retry
      else if ((++retry) < NTP_TASK_GENERIC_RETRY)
      {
        Delay(Ticks::MsToTicks(NTP_TASK_GENERIC_RETRY_DELAY_MS));
        TRACE_VERBOSE_F(F("NTP_STATE_END -> NTP_STATE_DO_NTP_SYNC\r\n"));
        state = NTP_STATE_DO_NTP_SYNC;
      }
      // error
      else
      {
        response.connection.done_ntp_synchronized = false;
        param.systemResponseQueue->Enqueue(&response, 0);

        param.systemStatusLock->Take();
        param.system_status->connection.is_ntp_synchronizing = false;
        param.system_status->connection.is_ntp_synchronized = false;
        param.systemStatusLock->Give();

        sntpClientDeinit(&sntpClientContext);
        state = NTP_STATE_INIT;
        TRACE_VERBOSE_F(F("NTP_STATE_END -> NTP_STATE_INIT\r\n"));
      }
      break;
    }
  }
}

#endif