/**
 ******************************************************************************
 * @file    modem_task.cpp
 * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
 * @author  Moreno Gasperini <m.gasperini@digiteco.it>
 * @brief   Modem connection cpp file
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

#define TRACE_LEVEL     MODEM_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   MODEM_TASK_ID

#include "tasks/modem_task.h"

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)

using namespace cpp_freertos;

ModemTask::ModemTask(const char *taskName, uint16_t stackSize, uint8_t priority, ModemParam_t modemParam) : Thread(taskName, stackSize, priority), param(modemParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(MODEM_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  state = MODEM_STATE_INIT;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void ModemTask::TaskMonitorStack()
{
  u_int16_t stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark( NULL );
  if((stackUsage) && (stackUsage < param.system_status->tasks[LOCAL_TASK_ID].stack)) {
    param.systemStatusLock->Take();
    param.system_status->tasks[LOCAL_TASK_ID].stack = stackUsage;
    param.systemStatusLock->Give();
  }
}
#endif

/// @brief local watchDog and Sleep flag Task (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void ModemTask::TaskWatchDog(uint32_t millis_standby)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Update WDT Signal (Direct or Long function Timered)
  if(millis_standby)  
  {
    // Check 1/2 Freq. controller ready to WDT only SET flag
    if((millis_standby) < WDT_CONTROLLER_MS / 2) {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
    } else {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::timer;
      // Add security milimal Freq to check
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog_ms = millis_standby + WDT_CONTROLLER_MS;
    }
  }
  else
    param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
  param.systemStatusLock->Give();
}

/// @brief local suspend flag and positor running state Task (optional)
/// @param state_position Sw_Position (Local STATE)
/// @param state_subposition Sw_SubPosition (Optional Local SUB_STATE Position Monitor)
/// @param state_operation operative mode flag status for this task
void ModemTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Signal Task sleep/disabled mode from request (Auto SET WDT on Resume)
  if((param.system_status->tasks[LOCAL_TASK_ID].state == task_flag::suspended)&&
     (state_operation==task_flag::normal))
     param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
  param.system_status->tasks[LOCAL_TASK_ID].state = state_operation;
  param.system_status->tasks[LOCAL_TASK_ID].running_pos = state_position;
  param.system_status->tasks[LOCAL_TASK_ID].running_sub = state_subposition;
  param.systemStatusLock->Give();
}

void ModemTask::Run() {
  uint8_t retry;
  bool is_error;
  error_t error;
  sim7600_status_t status;
  Ipv4Addr ipv4Addr;
  bool try_connection = false;

  connection_request_t connection_request;
  connection_response_t connection_response;

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  while (true)
  {
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

        param.systemStatusLock->Take();
        param.system_status->flags.ppp_error = true;
        param.systemStatusLock->Give();

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

        param.systemStatusLock->Take();
        param.system_status->flags.ppp_error = true;
        param.systemStatusLock->Give();

        TRACE_ERROR_F(F("%s Failed to configure interface %s [ %s ]\r\n"), Thread::GetName().c_str(), interface->name, ERROR_STRING);
        TaskWatchDog(MODEM_TASK_GENERIC_RETRY_DELAY_MS);
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
      // Suspend TASK Controller for queue waiting portMAX_DELAY
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);      
      if (param.connectionRequestQueue->Peek(&connection_request, portMAX_DELAY))
      {
        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);      
        // do connection
        if (connection_request.do_connect)
        {
          // Test connection estabilished ok (return done or error)
          try_connection = true;
          param.configurationLock->Take();
          strSafeCopy(apn, param.configuration->gsm_apn, GSM_APN_LENGTH);
          strSafeCopy(number, param.configuration->gsm_number, GSM_NUMBER_LENGTH);
          strSafeCopy(username, param.configuration->gsm_username, GSM_USERNAME_LENGTH);
          strSafeCopy(password, param.configuration->gsm_password, GSM_PASSWORD_LENGTH);
          param.configurationLock->Give();

          param.connectionRequestQueue->Dequeue(&connection_request);
          TRACE_VERBOSE_F(F("MODEM_STATE_WAIT_NET_EVENT -> MODEM_STATE_SWITCH_ON\r\n"));
          state = MODEM_STATE_SWITCH_ON;
        }
        // do disconnec
        else if (connection_request.do_disconnect)
        {
          param.connectionRequestQueue->Dequeue(&connection_request);
          TRACE_VERBOSE_F(F("MODEM_STATE_WAIT_NET_EVENT -> MODEM_STATE_DISCONNECT\r\n"));
          state = MODEM_STATE_DISCONNECT;
        }
      }
      break;

    case MODEM_STATE_SWITCH_ON:
      // Suspend TASK Controller for external Delay controller
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
      status = sim7600.switchOn();
      Delay(Ticks::MsToTicks(sim7600.getDelayMs()));
      // Resume task state WDT
      TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

      if (status == SIM7600_OK)
      {
        state = MODEM_STATE_SETUP;
        TRACE_VERBOSE_F(F("MODEM_STATE_SWITCH_ON -> MODEM_STATE_SETUP\r\n"));
      }
      else if (status == SIM7600_ERROR)
      {
        is_error = true;

        param.systemStatusLock->Take();
        param.system_status->flags.ppp_error = true;
        param.systemStatusLock->Give();

        state = MODEM_STATE_SWITCH_OFF;
        TRACE_VERBOSE_F(F("MODEM_STATE_SWITCH_ON -> MODEM_STATE_SWITCH_OFF\r\n"));
      }
      // Wait...
      break;

    case MODEM_STATE_SETUP:

      // Suspend TASK Controller for external Delay controller
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
      status = sim7600.setup();
      Delay(Ticks::MsToTicks(sim7600.getDelayMs()));
      // Resume
      TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

      if (status == SIM7600_OK)
      {
        state = MODEM_STATE_CONNECT;
        TRACE_VERBOSE_F(F("MODEM_STATE_SETUP -> MODEM_STATE_CONNECT\r\n"));
      }
      else if (status == SIM7600_ERROR)
      {
        is_error = true;

        param.systemStatusLock->Take();
        param.system_status->flags.ppp_error = true;
        param.systemStatusLock->Give();

        state = MODEM_STATE_SWITCH_OFF;
        TRACE_VERBOSE_F(F("MODEM_STATE_SETUP -> MODEM_STATE_SWITCH_OFF\r\n"));
      }
      break;

    case MODEM_STATE_CONNECT:
      // Suspend TASK Controller for external Delay controller
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
      status = sim7600.connect(apn, number);
      Delay(Ticks::MsToTicks(sim7600.getDelayMs()));
      // Resume
      TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

      if (status == SIM7600_OK)
      {
        state = MODEM_STATE_CONNECTED;
        TRACE_VERBOSE_F(F("MODEM_STATE_CONNECT -> MODEM_STATE_CONNECTED\r\n"));
      }
      else if (status == SIM7600_ERROR)
      {
        is_error = true;

        param.systemStatusLock->Take();
        param.system_status->flags.ppp_error = true;
        param.systemStatusLock->Give();

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
      // Suspend TASK Controller for external Delay controller
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
      error = pppConnect(interface);
      // Resume
      TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
      // Any error to report?
      if (error)
      {
        is_error = true;

        param.systemStatusLock->Take();
        param.system_status->flags.ppp_error = true;
        param.systemStatusLock->Give();

        TRACE_ERROR_F(F("%s Failed to established PPP connection... [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
        state = MODEM_STATE_DISCONNECT;
        TRACE_VERBOSE_F(F("MODEM_STATE_CONNECTED -> MODEM_STATE_DISCONNECT\r\n"));
      }
      else
      {
        // Connection EXEC completed PPP (remove testing var)
        try_connection = false;
        // Saving last state of modemParam signal state
        param.systemStatusLock->Take();
        param.system_status->flags.ppp_error = false;
        param.system_status->connection.is_ppp_estabilished = true;
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

        memset(&connection_response, 0, sizeof(connection_response_t));
        connection_response.done_connected = true;
        param.connectionResponseQueue->Enqueue(&connection_response);

        state = MODEM_STATE_WAIT_NET_EVENT;
        TRACE_VERBOSE_F(F("MODEM_STATE_CONNECTED -> MODEM_STATE_WAIT_NET_EVENT\r\n"));

      }
      break;

    case MODEM_STATE_DISCONNECT:
      // At begin Close PPP Connection
      if(param.system_status->connection.is_ppp_estabilished) {
        // Detach a PPP connection
        error = pppClose(interface);
        // Any error to report?
        if (error)
        {
          is_error = true;
          TRACE_ERROR_F(F("%s Failed to close PPP connection... [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
        } else {
          param.systemStatusLock->Take();
          param.system_status->connection.is_ppp_estabilished = false;
          param.systemStatusLock->Give();
          TRACE_INFO_F(F("%s Closing PPP connection... [ %s ]\r\n"), Thread::GetName().c_str(), OK_STRING);
        }
      }

      // Suspend TASK Controller for external Delay controller
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
      status = sim7600.disconnect();
      Delay(Ticks::MsToTicks(sim7600.getDelayMs()));
      // Resume
      TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

      if (status == SIM7600_OK)
      {
        is_error = false;
        state = MODEM_STATE_SWITCH_OFF;
        TRACE_VERBOSE_F(F("MODEM_STATE_DISCONNECT [OK] -> MODEM_STATE_SWITCH_OFF\r\n"));
      }
      else if (status == SIM7600_ERROR)
      {
        is_error = false;
        state = MODEM_STATE_SWITCH_OFF;
        TRACE_VERBOSE_F(F("MODEM_STATE_DISCONNECT [ERROR] -> MODEM_STATE_SWITCH_OFF\r\n"));
      }
      break;

    case MODEM_STATE_SWITCH_OFF:
      // Suspend TASK Controller for external Delay controller
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
      status = sim7600.switchOff();
      Delay(Ticks::MsToTicks(sim7600.getDelayMs()));
      // Resume
      TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

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
      memset(&connection_response, 0, sizeof(connection_response_t));
      if(try_connection) {
        // If required connection -> Error to connect
        connection_response.error_connected = true;
      } else {
        // If required disconnection -> done_disconnect
        connection_response.done_disconnected = true;
      }
      param.connectionResponseQueue->Enqueue(&connection_response);
      
      // Exit (Retry connection is extern from Modem)
      state = MODEM_STATE_WAIT_NET_EVENT;
      TRACE_VERBOSE_F(F("MODEM_STATE_END -> MODEM_STATE_WAIT_NET_EVENT\r\n"));
      break;
    }

    #if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
    #endif

    // One step base non blocking switch
    TaskWatchDog(MODEM_TASK_WAIT_DELAY_MS);
    Delay(Ticks::MsToTicks(MODEM_TASK_WAIT_DELAY_MS));

  }
}

#endif