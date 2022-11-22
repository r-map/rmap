/**@file supervisor_task.cpp */

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

#define TRACE_LEVEL SUPERVISOR_TASK_TRACE_LEVEL

#include "tasks/supervisor_task.h"

using namespace cpp_freertos;

SupervisorTask::SupervisorTask(const char *taskName, uint16_t stackSize, uint8_t priority, SupervisorParam_t supervisorParam) : Thread(taskName, stackSize, priority), param(supervisorParam)
{
  eeprom = EEprom(param.wire, param.wireLock);
  state = SUPERVISOR_STATE_INIT;
  Start();
};

void SupervisorTask::Run()
{
  system_request_t request;
  system_response_t response;

  while (true)
  {
    bool is_saved = false;
    bool is_loaded = false;

    memset(&request, 0, sizeof(system_request_t));
    memset(&response, 0, sizeof(system_response_t));

    switch (state)
    {
    case SUPERVISOR_STATE_INIT:
      retry = 0;

      TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_OPERATION -> SUPERVISOR_STATE_LOAD_CONFIGURATION\r\n"));
      state = SUPERVISOR_STATE_LOAD_CONFIGURATION;
      break;

    case SUPERVISOR_STATE_CHECK_OPERATION:
      if (param.system_status->configuration.is_loaded && !param.system_status->connection.is_ntp_synchronized)
      {
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_OPERATION -> SUPERVISOR_STATE_REQUEST_CONNECTION\r\n"));
        state = SUPERVISOR_STATE_REQUEST_CONNECTION;
      }
      else
      {
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_OPERATION -> ??? Condizione non gestita!!!\r\n"));
        Suspend();
      }
      break;

    case SUPERVISOR_STATE_LOAD_CONFIGURATION:
      is_loaded = loadConfiguration(param.configuration, param.configurationLock);
      
      if (is_loaded)
      {
        retry = 0;
        
        param.systemStatusLock->Take();
        param.system_status->configuration.is_loaded = true;
        param.systemStatusLock->Give();
        
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_LOAD_CONFIGURATION -> SUPERVISOR_STATE_CHECK_OPERATION\r\n"));
        state = SUPERVISOR_STATE_CHECK_OPERATION;
      }
      else if ((++retry <= SUPERVISOR_TASK_GENERIC_RETRY) && !is_loaded)
      {
        Delay(Ticks::MsToTicks(SUPERVISOR_TASK_GENERIC_RETRY_DELAY_MS));
      }
      else
      {
        param.systemStatusLock->Take();
        param.system_status->configuration.is_loaded = false;
        param.systemStatusLock->Give();

        // gestire condizione di errore di lettura della configurazione
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_LOAD_CONFIGURATION -> ??? Condizione non gestita!!!\r\n"));
        Suspend();
      }
      break;

    case SUPERVISOR_STATE_SAVE_CONFIGURATION:
      is_saved = saveConfiguration(param.configuration, param.configurationLock, CONFIGURATION_CURRENT);

      if (is_saved)
      {
        retry = 0;

        param.systemStatusLock->Take();
        param.system_status->configuration.is_saved = true;
        param.system_status->configuration.is_loaded = true;
        param.systemStatusLock->Give();

        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_SAVE_CONFIGURATION -> SUPERVISOR_STATE_CHECK_OPERATION\r\n"));
        state = SUPERVISOR_STATE_CHECK_OPERATION;
      }
      else if ((++retry <= SUPERVISOR_TASK_GENERIC_RETRY) && !is_saved)
      {
        Delay(Ticks::MsToTicks(SUPERVISOR_TASK_GENERIC_RETRY_DELAY_MS));
      }
      else
      {
        param.systemStatusLock->Take();
        param.system_status->configuration.is_saved = false;
        param.system_status->configuration.is_loaded = false;
        param.systemStatusLock->Give();

        // gestire condizione di errore di scrittura della configurazione
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_SAVE_CONFIGURATION -> ??? Condizione non gestita!!!\r\n"));
        Suspend();
      }
      break;

    case SUPERVISOR_STATE_REQUEST_CONNECTION:
      // already connected
      if (param.system_status->connection.is_connected)
      {
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_REQUEST_CONNECTION -> SUPERVISOR_STATE_CHECK_CONNECTION_TYPE\r\n"));
        state = SUPERVISOR_STATE_CHECK_CONNECTION_TYPE;
      }
      // not connected -> request connection.
      else
      {
        request.connection.do_connect = true;
        
        param.systemStatusLock->Take();
        param.system_status->connection.is_connection_ongoing = true;
        param.systemStatusLock->Give();
        
        param.systemRequestQueue->Enqueue(&request, 0);
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_REQUEST_CONNECTION -> SUPERVISOR_STATE_CHECK_CONNECTION\r\n"));
        state = SUPERVISOR_STATE_CHECK_CONNECTION;
      }
      break;
    
    case SUPERVISOR_STATE_CHECK_CONNECTION:
      // wait connection
      if (param.systemResponseQueue->Peek(&response, portMAX_DELAY))
      {
        // ok connected
        if (response.connection.done_connected)
        {
          param.systemResponseQueue->Dequeue(&response, 0);
          param.systemStatusLock->Take();
          param.system_status->connection.is_connected = true;
          param.system_status->connection.is_connection_ongoing = false;
          param.systemStatusLock->Give();
          TRACE_INFO_F(F("%s Connection [ %s ]\r\n"), taskName, OK_STRING);

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_CONNECTION -> SUPERVISOR_STATE_CHECK_CONNECTION_TYPE\r\n"));
          state = SUPERVISOR_STATE_CHECK_CONNECTION_TYPE;
        }
        // error: not connected
        else if (!response.connection.done_connected)
        {
          param.systemResponseQueue->Dequeue(&response, 0);
          param.systemStatusLock->Take();
          param.system_status->connection.is_connected = false;
          param.system_status->connection.is_connection_ongoing = false;
          param.systemStatusLock->Give();
          TRACE_ERROR_F(F("%s Connection [ %s ]\r\n"), taskName, ERROR_STRING);

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_CONNECTION -> SUPERVISOR_STATE_END\r\n"));
          state = SUPERVISOR_STATE_CHECK_CONNECTION_TYPE;
        }
        // other
        else
        {
          Delay(Ticks::MsToTicks(SUPERVISOR_TASK_WAIT_DELAY_MS));
        }
      }
      // do something else with non-blocking wait ....
      break;

    case SUPERVISOR_STATE_CHECK_CONNECTION_TYPE:
      if (!param.system_status->connection.is_ntp_synchronized)
      {
        param.systemStatusLock->Take();
        param.system_status->connection.is_ntp_sync_ongoing = true;
        param.systemStatusLock->Give();

        // Request ntp sync
        request.connection.do_ntp_sync = true;
        param.systemRequestQueue->Enqueue(&request, 0);

        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_CONNECTION_TYPE -> SUPERVISOR_STATE_DO_NTP_SYNC\r\n"));
        state = SUPERVISOR_STATE_DO_NTP_SYNC;
      }
      else
      {
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_CONNECTION_TYPE -> ??? Condizione non gestita!!!\r\n"));
        Thread::Suspend();
      }
      break;

    case SUPERVISOR_STATE_DO_NTP_SYNC:
      // wait ntp to be sync
      if (param.systemResponseQueue->Peek(&response, portMAX_DELAY))
      {
        // ok ntp synchronized
        if (response.connection.done_ntp_synchronized)
        {
          param.systemResponseQueue->Dequeue(&response, 0);
          param.systemStatusLock->Take();
          param.system_status->connection.is_ntp_sync_ongoing = false;
          param.system_status->connection.is_ntp_synchronized = true;
          param.systemStatusLock->Give();

          TRACE_INFO_F(F("%s NTP synchronization [ %s ]\r\n"), taskName, OK_STRING);

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_DO_NTP_SYNC -> SUPERVISOR_STATE_CHECK_OPERATION\r\n"));
          state = SUPERVISOR_STATE_CHECK_OPERATION;
        }
        // error: not connected
        else if (!response.connection.done_ntp_synchronized)
        {
          param.systemResponseQueue->Dequeue(&response, 0);
          param.systemStatusLock->Take();
          param.system_status->connection.is_ntp_sync_ongoing = false;
          param.system_status->connection.is_ntp_synchronized = false;
          param.systemStatusLock->Give();
          TRACE_ERROR_F(F("%s NTP synchronization [ %s ]\r\n"), taskName, ERROR_STRING);

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_DO_NTP_SYNC -> SUPERVISOR_STATE_CHECK_OPERATION\r\n"));
          state = SUPERVISOR_STATE_CHECK_OPERATION;
        }
        // other
        else
        {
          Delay(Ticks::MsToTicks(SUPERVISOR_TASK_WAIT_DELAY_MS));
        }
      }
      // do something else with non-blocking wait ....
      break;

    case SUPERVISOR_STATE_END:
      TRACE_VERBOSE_F(F("SUPERVISOR_STATE_END -> SUPERVISOR_STATE_CHECK_OPERATION\r\n"));
      state = SUPERVISOR_STATE_CHECK_OPERATION;
      break;
    }
  }
}

bool SupervisorTask::loadConfiguration(configuration_t *configuration, BinarySemaphore *lock)
{
  bool error = false;

  //! read configuration from eeprom
  if (lock->Take())
  {
    error = eeprom.Read(CONFIGURATION_EEPROM_ADDRESS, (uint8_t *)(configuration), sizeof(configuration_t));
    lock->Give();
  }

  if (configuration->module_type != MODULE_TYPE || configuration->module_main_version != MODULE_MAIN_VERSION)
  {
    error = saveConfiguration(configuration, lock, CONFIGURATION_DEFAULT);
  }
  else
  {
    TRACE_INFO_F(F("Load configuration... [ %s ]\r\n"), OK_STRING);
    printConfiguration(configuration, lock);
  }

  return error;
}

void SupervisorTask::printConfiguration(configuration_t *configuration, BinarySemaphore *lock)
{
  if (lock->Take()) {
    char stima_name[20];
    getStimaNameByType(stima_name, configuration->module_type);
    TRACE_INFO_F(F("--> type: %s\r\n"), stima_name);
    TRACE_INFO_F(F("--> main version: %u\r\n"), configuration->module_main_version);
    TRACE_INFO_F(F("--> minor version: %u\r\n"), configuration->module_minor_version);
    // TRACE_INFO_F(F("--> acquisition delay: %u [ms]\r\n"), configuration.sensor_acquisition_delay_ms);

    // TRACE_INFO_F(F("--> %u configured sensors\r\n"), configuration.sensors_count);
    // for (uint8_t i=0; i<configuration.sensors_count; i++) {
    //   TRACE_INFO_F(F("--> %u: %s-%s 0x%02X [ %s ]\r\n"), i+1, SENSOR_DRIVER_I2C, configuration.sensors[i].type, configuration.sensors[i].i2c_address, configuration.sensors[i].is_redundant ? REDUNDANT_STRING : MAIN_STRING);
    // }
    lock->Give();
  }
}

bool SupervisorTask::saveConfiguration(configuration_t *configuration, BinarySemaphore *lock, bool is_default)
{
  bool error = false;

  if (lock->Take())
  {
    if (is_default)
    {
      TRACE_INFO_F(F("Save default configuration... [ %s ]\r\n"), OK_STRING);
      configuration->module_main_version = MODULE_MAIN_VERSION;
      configuration->module_minor_version = MODULE_MINOR_VERSION;
      configuration->module_type = MODULE_TYPE;
      // configuration.sensor_acquisition_delay_ms = SENSORS_ACQUISITION_DELAY_MS;
    }
    else
    {
      TRACE_INFO_F(F("Save configuration... [ %s ]\r\n"), OK_STRING);
    }

    //! write configuration to eeprom
    error = eeprom.Write(CONFIGURATION_EEPROM_ADDRESS, (uint8_t *)(configuration), sizeof(configuration_t));
    lock->Give();
  }

  printConfiguration(configuration, lock);

  return error;
}