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
  uint8_t retry;
  // Request response for system queue Task controlled...
  system_message_t system_message;

  while (true)
  {
    bool is_saved = false;
    bool is_loaded = false;

    switch (state)
    {
    case SUPERVISOR_STATE_INIT:
      retry = 0;

      TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_OPERATION -> SUPERVISOR_STATE_LOAD_CONFIGURATION\r\n"));
      state = SUPERVISOR_STATE_LOAD_CONFIGURATION;
      // saveConfiguration(configuration, configurationLock, CONFIGURATION_DEFAULT);
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
        TRACE_ERROR_F(F("SUPERVISOR_STATE_LOAD_CONFIGURATION -> ??? Condizione non gestita!!!\r\n"));
        Suspend();
      }
      break;
    
    case SUPERVISOR_STATE_CHECK_OPERATION:
      // true if configuration ok and loaded -> 
      if (param.system_status->configuration.is_loaded)
      {
        // Standard SUPERVISOR_OPERATION SYSTEM CHECK...

        // ********* SYSTEM QUEUE REQUEST ***********
        // Check Queue command system status
        if(!param.systemMessageQueue->IsEmpty()) {
          if(param.systemMessageQueue->Peek(&system_message, 0)) {
            if(system_message.task_dest == SUPERVISOR_TASK_QUEUE_ID) {
              // Command direct for TASK remove from queue
              param.systemMessageQueue->Dequeue(&system_message, 0);
              if(system_message.command.do_maint) {
                if(system_message.param != 0) {
                  param.system_status->flags.is_maintenance = true;
                } else {
                  param.system_status->flags.is_maintenance = false;
                }
              }
            }
          }
          // Its request addressed into ALL TASK... -> no pull (only SUPERVISOR or exernal gestor)
          if(system_message.task_dest == ALL_TASK_QUEUE_ID)
          {
            // Pull && elaborate command, 
            if(system_message.command.do_sleep)
            {
              // Enter task sleep
              Delay(Ticks::MsToTicks(SUPERVISOR_TASK_SLEEP_DELAY_MS));
              // Il superVisor elimina il messaggio dalla coda dello sleep
              param.systemMessageQueue->Dequeue(&system_message, 0);
            }
          }
        } else {
          // Standard delay task
          Delay(Ticks::MsToTicks(SUPERVISOR_TASK_WAIT_DELAY_MS));
        }
      }
      else
      {
        TRACE_ERROR_F(F("SUPERVISOR_STATE_CHECK_OPERATION -> ??? Condizione non gestita!!!\r\n"));
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
        TRACE_ERROR_F(F("SUPERVISOR_STATE_SAVE_CONFIGURATION -> ??? Condizione non gestita!!!\r\n"));
        Suspend();
      }
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
  bool status = true;

  //! read configuration from eeprom
  if (lock->Take())
  {
    status = eeprom.Read(CONFIGURATION_EEPROM_ADDRESS, (uint8_t *)(configuration), sizeof(configuration_t));
    lock->Give();
  }

  #ifndef INIT_SENSOR_CONFIG
  if (configuration->module_type != MODULE_TYPE || configuration->module_main_version != MODULE_MAIN_VERSION)
  #endif
  {
    status = saveConfiguration(configuration, lock, CONFIGURATION_DEFAULT);
  }

  TRACE_INFO_F(F("Load configuration... [ %s ]\r\n"), status ? OK_STRING : ERROR_STRING);
  printConfiguration(configuration, lock);

  return status;
}

void SupervisorTask::printConfiguration(configuration_t *configuration, BinarySemaphore *lock)
{
  if (lock->Take()) {
    char stima_name[STIMA_MODULE_NAME_LENGTH];

    getStimaNameByType(stima_name, configuration->module_type);
    TRACE_INFO_F(F("-> type: %s\r\n"), stima_name);
    TRACE_INFO_F(F("-> main version: %u\r\n"), configuration->module_main_version);
    TRACE_INFO_F(F("-> minor version: %u\r\n"), configuration->module_minor_version);
    TRACE_INFO_F(F("-> acquisition delay: %u [ms]\r\n"), configuration->sensor_acquisition_delay_ms);

    TRACE_INFO_F(F("-> %u configured sensors:\r\n"), configuration->sensors_count);
    for (uint8_t i = 0; i < configuration->sensors_count; i++)
    {
      TRACE_INFO_F(F("--> %u: %s-%s 0x%02X [ %s ]\r\n"), i + 1, SENSOR_DRIVER_I2C, configuration->sensors[i].type, configuration->sensors[i].i2c_address, configuration->sensors[i].is_redundant ? REDUNDANT_STRING : MAIN_STRING);
    }

    lock->Give();
  }
}

bool SupervisorTask::saveConfiguration(configuration_t *configuration, BinarySemaphore *lock, bool is_default)
{
  bool status = true;

  if (lock->Take())
  {
    if (is_default)
    {
      osMemset(configuration, 0, sizeof(configuration_t));

      configuration->module_main_version = MODULE_MAIN_VERSION;
      configuration->module_minor_version = MODULE_MINOR_VERSION;
      configuration->module_type = MODULE_TYPE;

      configuration->sensor_acquisition_delay_ms = SENSORS_ACQUISITION_DELAY_MS;

#if (USE_SENSOR_ADT)
      strcpy(configuration->sensors[configuration->sensors_count].type, SENSOR_TYPE_ADT);
      configuration->sensors[configuration->sensors_count].i2c_address = 0x28;
      configuration->sensors[configuration->sensors_count].is_redundant = false;
      configuration->sensors_count++;
#endif

#if (USE_SENSOR_HIH)
      strcpy(configuration->sensors[configuration->sensors_count].type, SENSOR_TYPE_HIH);
      configuration->sensors[configuration->sensors_count].i2c_address = 0x28;
      configuration->sensors[configuration->sensors_count].is_redundant = false;
      configuration->sensors_count++;
#endif

#if (USE_SENSOR_HYT)
      strcpy(configuration->sensors[configuration->sensors_count].type, SENSOR_TYPE_HYT);
      configuration->sensors[configuration->sensors_count].i2c_address = HYT_DEFAULT_ADDRESS;
      configuration->sensors[configuration->sensors_count].is_redundant = false;
      configuration->sensors_count++;

#if (USE_REDUNDANT_SENSOR)
      strcpy(configuration->sensors[configuration->sensors_count].type, SENSOR_TYPE_HYT);
      configuration->sensors[configuration->sensors_count].i2c_address = HYT_REDUNDANT_ADDRESS;
      configuration->sensors[configuration->sensors_count].is_redundant = true;
      configuration->sensors_count++;
#endif
#endif

#if (USE_SENSOR_SHT)
      strcpy(configuration->sensors[configuration->sensors_count].type, SENSOR_TYPE_SHT);
      configuration->sensors[configuration->sensors_count].i2c_address = SHT_DEFAULT_ADDRESS;
      configuration->sensors[configuration->sensors_count].is_redundant = false;
      configuration->sensors_count++;

#if (USE_REDUNDANT_SENSOR)
      strcpy(configuration->sensors[configuration->sensors_count].type, SENSOR_TYPE_SHT);
      configuration->sensors[configuration->sensors_count].i2c_address = SHT_REDUNDANT_ADDRESS;
      configuration->sensors[configuration->sensors_count].is_redundant = true;
      configuration->sensors_count++;
#endif
#endif
    }

    //! write configuration to eeprom
    status = eeprom.Write(CONFIGURATION_EEPROM_ADDRESS, (uint8_t *)(configuration), sizeof(configuration_t));

    if (is_default)
    {
      TRACE_INFO_F(F("Save default configuration... [ %s ]\r\n"), status ? OK_STRING : ERROR_STRING);
    }
    else
    {
      TRACE_INFO_F(F("Save configuration... [ %s ]\r\n"), status ? OK_STRING : ERROR_STRING);
    }

    lock->Give();
  }

  return status;
}