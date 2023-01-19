/**@file modem_task.cpp */

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

#define TRACE_LEVEL MODEM_TASK_TRACE_LEVEL

#include "tasks/modem_task.h"

using namespace cpp_freertos;

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)

ModemTask::ModemTask(const char *taskName, uint16_t stackSize, uint8_t priority, ModemParam_t modemParam) : Thread(taskName, stackSize, priority), param(modemParam)
{
  // Start WDT controller and RunState Flags
  #if (ENABLE_WDT)
  WatchDog(param.system_status, param.systemStatusLock, WDT_TIMEOUT_BASE_US / 1000, false);
  RunState(param.system_status, param.systemStatusLock, RUNNING_START, false);
  #endif

  state = MODEM_STATE_INIT;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
void ModemTask::monitorStack(system_status_t *status, BinarySemaphore *lock)
{
  u_int16_t stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark( NULL );
  if((stackUsage) && (stackUsage < status->tasks[MODEM_TASK_ID].stack)) {
    if(lock != NULL) lock->Take();
    status->tasks[MODEM_TASK_ID].stack = stackUsage;
    if(lock != NULL) lock->Give();
  }
}
#endif

#if (ENABLE_WDT)
/// @brief local watchDog and Sleep flag Task (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void ModemTask::WatchDog(system_status_t *status, BinarySemaphore *lock, uint16_t millis_standby, bool is_sleep)
{
  // Local WatchDog update
  if(lock != NULL) lock->Take();
  // Signal Task sleep/disabled mode from request
  status->tasks[MODEM_TASK_ID].is_sleep = is_sleep;
  if((millis_standby) < (WDT_TIMEOUT_BASE_US / 1000))
    status->tasks[MODEM_TASK_ID].watch_dog = wdt_flag::set;
  else
    status->tasks[MODEM_TASK_ID].watch_dog = wdt_flag::rest;
  if(lock != NULL) lock->Give();
}

/// @brief local suspend flag and positor running state Task (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param state_position Sw_Poition (STandard 1 RUNNING_START, 2 RUNNING_EXEC, XX User define SW POS fo Logging)
/// @param is_suspend TRUE if task enter suspending mode (Disable from WDT Controller)
void ModemTask::RunState(system_status_t *status, BinarySemaphore *lock, uint8_t state_position, bool is_suspend)
{
  // Local WatchDog update
  if(lock != NULL) lock->Take();
  // Signal Task sleep/disabled mode from request
  status->tasks[MODEM_TASK_ID].is_suspend = is_suspend;
  status->tasks[MODEM_TASK_ID].running_pos = state_position;
  if(lock != NULL) lock->Give();
}
#endif


void ModemTask::Run() {
  uint8_t retry;
  bool is_error;
  error_t error;
  sim7600_status_t status;
  system_request_t request;
  system_response_t response;
  Ipv4Addr ipv4Addr;

  // Starting Task and first WDT (if required and enabled. Time < than WDT_TIMEOUT_BASE_US)
  #if (ENABLE_WDT)
  RunState(param.system_status, param.systemStatusLock, RUNNING_EXEC, false);
  #endif
  #if (ENABLE_STACK_USAGE)
  monitorStack(param.system_status, param.systemStatusLock);
  #endif

  while (true)
  {
    memset(&request, 0, sizeof(system_request_t));
    memset(&response, 0, sizeof(system_response_t));

    switch (state)
    {
    case MODEM_STATE_INIT:
      retry = 0;
      is_error = false;

      // Configure the first network interface
      interface = &netInterface[INTERFACE_0_INDEX];

      // Get default PPP settings
      pppGetDefaultSettings(&pppSettings);
      // Select the underlying interface
      pppSettings.interface = interface;
      // Default async control character map
      pppSettings.accm = 0x00000000;

      // Initialize PPP
      error = pppInit(&pppContext, &pppSettings);
      // useful for debug
      if (error)
      {
        is_error = true;

        TRACE_ERROR_F(F("%s Failed to initialize PPP... [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
        Delay(Ticks::MsToTicks(MODEM_TASK_GENERIC_RETRY_DELAY_MS));
        break;
      }

      // Set interface name
      netSetInterfaceName(interface, INTERFACE_0_NAME);
      // Select the relevant UART driver
      netSetUartDriver(interface, &uartDriver);

      // Initialize network interface
      error = netConfigInterface(interface);
      // useful for debug
      if (error)
      {
        is_error = true;

        TRACE_ERROR_F(F("%s Failed to configure interface %s [ %s ]\r\n"), Thread::GetName().c_str(), interface->name, ERROR_STRING);
        Delay(Ticks::MsToTicks(MODEM_TASK_GENERIC_RETRY_DELAY_MS));
        break;
      }

      sim7600 = SIM7600(interface, PPP0_BAUD_RATE_DEFAULT, PPP0_BAUD_RATE_MAX, PIN_GSM_EN_POW, PIN_GSM_PW_KEY, PIN_GSM_RI);

      TRACE_VERBOSE_F(F("MODEM_STATE_INIT -> MODEM_STATE_WAIT_NET_EVENT\r\n"));
      state = MODEM_STATE_WAIT_NET_EVENT;
      break;

    case MODEM_STATE_WAIT_NET_EVENT:
      is_error = false;
      retry = 0;

      // wait connection request
      if (param.systemRequestQueue->Peek(&request, portMAX_DELAY))
      {
        // do connection
        if (request.connection.do_connect)
        {
          param.configurationLock->Take();
          strSafeCopy(apn, param.configuration->gsm_apn, GSM_APN_LENGTH);
          strSafeCopy(number, param.configuration->gsm_number, GSM_NUMBER_LENGTH);
          strSafeCopy(username, param.configuration->gsm_username, GSM_USERNAME_LENGTH);
          strSafeCopy(password, param.configuration->gsm_password, GSM_PASSWORD_LENGTH);
          param.configurationLock->Give();

          param.systemRequestQueue->Dequeue(&request, 0);
          TRACE_VERBOSE_F(F("MODEM_STATE_WAIT_NET_EVENT -> MODEM_STATE_SWITCH_ON\r\n"));
          state = MODEM_STATE_SWITCH_ON;
        }
        // do disconnect
        else if (request.connection.do_disconnect)
        {
          param.systemRequestQueue->Dequeue(&request, 0);
          TRACE_VERBOSE_F(F("MODEM_STATE_WAIT_NET_EVENT -> MODEM_STATE_DISCONNECT\r\n"));
          state = MODEM_STATE_DISCONNECT;
        }
        // other
        else
        {
          Delay(Ticks::MsToTicks(MODEM_TASK_WAIT_DELAY_MS));
        }
      }
      // do something else with non-blocking wait ....
      break;

    case MODEM_STATE_SWITCH_ON:
      status = sim7600.switchOn();
      Delay(Ticks::MsToTicks(sim7600.getDelayMs()));

      if (status == SIM7600_OK)
      {
        state = MODEM_STATE_SETUP;
        TRACE_VERBOSE_F(F("MODEM_STATE_SWITCH_ON -> MODEM_STATE_SETUP\r\n"));
      }
      else if (status == SIM7600_ERROR)
      {
        is_error = true;

        state = MODEM_STATE_SWITCH_OFF;
        TRACE_VERBOSE_F(F("MODEM_STATE_SWITCH_ON -> MODEM_STATE_SWITCH_OFF\r\n"));
      }
      // Wait...
      break;

    case MODEM_STATE_SETUP:
      status = sim7600.setup();
      Delay(Ticks::MsToTicks(sim7600.getDelayMs()));

      if (status == SIM7600_OK)
      {
        state = MODEM_STATE_CONNECT;
        TRACE_VERBOSE_F(F("MODEM_STATE_SETUP -> MODEM_STATE_CONNECT\r\n"));
      }
      else if (status == SIM7600_ERROR)
      {
        is_error = true;

        state = MODEM_STATE_SWITCH_OFF;
        TRACE_VERBOSE_F(F("MODEM_STATE_SETUP -> MODEM_STATE_SWITCH_OFF\r\n"));
      }
      break;

    case MODEM_STATE_CONNECT:
      status = sim7600.connect(apn, number);
      Delay(Ticks::MsToTicks(sim7600.getDelayMs()));

      if (status == SIM7600_OK)
      {
        state = MODEM_STATE_CONNECTED;
        TRACE_VERBOSE_F(F("MODEM_STATE_CONNECT -> MODEM_STATE_CONNECTED\r\n"));
      }
      else if (status == SIM7600_ERROR)
      {
        is_error = true;

        state = MODEM_STATE_DISCONNECT;
        TRACE_VERBOSE_F(F("MODEM_STATE_CONNECT -> MODEM_STATE_DISCONNECT\r\n"));
      }
      break;

    case MODEM_STATE_CONNECTED:
      // Clear local IPv4 address
      ipv4StringToAddr("0.0.0.0", &ipv4Addr);
      ipv4SetHostAddr(interface, ipv4Addr);

      // Clear peer IPv4 address
      ipv4StringToAddr("0.0.0.0", &ipv4Addr);
      ipv4SetDefaultGateway(interface, ipv4Addr);

      // Set primary DNS server
      ipv4StringToAddr(PPP0_PRIMARY_DNS, &ipv4Addr);
      ipv4SetDnsServer(interface, 0, ipv4Addr);

      // Set secondary DNS server
      ipv4StringToAddr(PPP0_SECONDARY_DNS, &ipv4Addr);
      ipv4SetDnsServer(interface, 1, ipv4Addr);

      // Set username and password
      pppSetAuthInfo(interface, username, password);

      // Establish a PPP connection
      error = pppConnect(interface);
      // Any error to report?
      if (error)
      {
        is_error = true;

        TRACE_ERROR_F(F("%s Failed to established PPP connection... [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
        state = MODEM_STATE_DISCONNECT;
        TRACE_VERBOSE_F(F("MODEM_STATE_CONNECTED -> MODEM_STATE_DISCONNECT\r\n"));
      }
      else
      {
        param.systemStatusLock->Take();
        param.system_status->modem.ber = sim7600.getBer();
        param.system_status->modem.rssi = sim7600.getRssi();
        param.system_status->modem.creg_n = sim7600.getCregN();
        param.system_status->modem.creg_stat = sim7600.getCregStat();
        param.system_status->modem.cgreg_n = sim7600.getCgregN();
        param.system_status->modem.cgreg_stat = sim7600.getCgregStat();
        param.system_status->modem.cereg_n = sim7600.getCeregN();
        param.system_status->modem.cereg_stat = sim7600.getCeregStat();
        param.systemStatusLock->Give();

        TRACE_INFO_F(F("%s Establishing PPP connection... [ %s ]\r\n"), Thread::GetName().c_str(), OK_STRING);
        response.connection.done_connected = true;
        param.systemResponseQueue->Enqueue(&response, 0);

        state = MODEM_STATE_WAIT_NET_EVENT;
        TRACE_VERBOSE_F(F("MODEM_STATE_CONNECTED -> MODEM_STATE_WAIT_NET_EVENT\r\n"));
      }
      break;

    case MODEM_STATE_DISCONNECT:
      status = sim7600.disconnect();
      Delay(Ticks::MsToTicks(sim7600.getDelayMs()));

      if (status == SIM7600_OK)
      {
        is_error = false;
        state = MODEM_STATE_SWITCH_OFF;
        TRACE_VERBOSE_F(F("MODEM_STATE_DISCONNECT -> MODEM_STATE_SWITCH_OFF\r\n"));
      }
      else if (status == SIM7600_ERROR)
      {
        is_error = false;
        state = MODEM_STATE_SWITCH_OFF;
        TRACE_VERBOSE_F(F("MODEM_STATE_DISCONNECT -> MODEM_STATE_SWITCH_OFF\r\n"));
      }
      break;

    case MODEM_STATE_SWITCH_OFF:
      status = sim7600.switchOff();
      Delay(Ticks::MsToTicks(sim7600.getDelayMs()));

      if (status == SIM7600_OK)
      {
        is_error = false;
        state = MODEM_STATE_END;
        TRACE_VERBOSE_F(F("MODEM_STATE_SWITCH_OFF -> MODEM_STATE_END\r\n"));
      }
      else if (status == SIM7600_ERROR)
      {
        is_error = false;
        state = MODEM_STATE_END;
        TRACE_VERBOSE_F(F("MODEM_STATE_SWITCH_OFF -> MODEM_STATE_END\r\n"));
      }
      break;

    case MODEM_STATE_END:
      response.connection.done_disconnected = true;
      param.systemResponseQueue->Enqueue(&response, 0);

      // ok
      if (!is_error)
      {
        state = MODEM_STATE_WAIT_NET_EVENT;
        TRACE_VERBOSE_F(F("MODEM_STATE_END -> MODEM_STATE_WAIT_NET_EVENT\r\n"));
      }
      // retry
      else if ((++retry) < MODEM_TASK_GENERIC_RETRY)
      {
        Delay(Ticks::MsToTicks(MODEM_TASK_GENERIC_RETRY_DELAY_MS));
        TRACE_VERBOSE_F(F("MODEM_STATE_END -> MODEM_STATE_SWITCH_ON\r\n"));
        state = MODEM_STATE_SWITCH_ON;
      }
      // error
      else
      {
        state = MODEM_STATE_WAIT_NET_EVENT;
        TRACE_VERBOSE_F(F("MODEM_STATE_END -> MODEM_STATE_WAIT_NET_EVENT\r\n"));
      }
      break;
    }
  }
}

#endif