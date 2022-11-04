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

SupervisorTask::SupervisorTask(const char *taskName, uint16_t stackSize, uint8_t priority, SupervisorParam_t supervisorParam) : Thread(taskName, stackSize, priority), SupervisorParam(supervisorParam) {
  eeprom = EEprom(SupervisorParam.wireLock);
  state = SUPERVISOR_STATE_INIT;
  Start();
};

void SupervisorTask::Run()
{
  bool is_configuration_loaded = false;
  
  while (true)
  {
    switch (state)
    {
    case SUPERVISOR_STATE_INIT:
      TRACE_VERBOSE(F("SUPERVISOR_STATE_INIT -> SUPERVISOR_STATE_CHECK_OPERATION\r\n"));
      state = SUPERVISOR_STATE_CHECK_OPERATION;
      break;

    case SUPERVISOR_STATE_CHECK_OPERATION:
      if (!is_configuration_loaded)
      {
        TRACE_VERBOSE(F("SUPERVISOR_STATE_CHECK_OPERATION -> SUPERVISOR_STATE_LOAD_CONFIGURATION\r\n"));
        state = SUPERVISOR_STATE_LOAD_CONFIGURATION;
      }
      else {
        Suspend();
      }
      break;

    case SUPERVISOR_STATE_LOAD_CONFIGURATION:
      LoadConfiguration(SupervisorParam.configuration, SupervisorParam.configurationLock);
      is_configuration_loaded = true;
      TRACE_VERBOSE(F("SUPERVISOR_STATE_LOAD_CONFIGURATION -> SUPERVISOR_STATE_END\r\n"));
      state = SUPERVISOR_STATE_END;
      break;

    case SUPERVISOR_STATE_SAVE_CONFIGURATION:
      SaveConfiguration(SupervisorParam.configuration, SupervisorParam.configurationLock, CONFIGURATION_CURRENT);
      TRACE_VERBOSE(F("SUPERVISOR_STATE_SAVE_CONFIGURATION -> SUPERVISOR_STATE_END\r\n"));
      state = SUPERVISOR_STATE_END;
      break;
    
    case SUPERVISOR_STATE_END:
      TRACE_VERBOSE(F("SUPERVISOR_STATE_END -> SUPERVISOR_STATE_CHECK_OPERATION\r\n"));
      state = SUPERVISOR_STATE_CHECK_OPERATION;
      break;
    }
  }
}

void SupervisorTask::LoadConfiguration(configuration_t *configuration, BinarySemaphore *lock)
{
  //! read configuration from eeprom
  if (lock->Take())
  {
    eeprom.Read(CONFIGURATION_EEPROM_ADDRESS, (uint8_t *)(configuration), sizeof(configuration_t));
    lock->Give();
  }

  if (configuration->module_type != MODULE_TYPE || configuration->module_main_version != MODULE_MAIN_VERSION)
  {
    SaveConfiguration(configuration, lock, DEFAULT_CONFIGURATION);
  }
  else
  {
    TRACE_INFO(F("Load configuration... [ %s ]\r\n"), OK_STRING);
    PrintConfiguration(configuration, lock);
  }
}

void SupervisorTask::PrintConfiguration(configuration_t *configuration, BinarySemaphore *lock)
{
  if (lock->Take()) {
    char stima_name[20];
    getStimaNameByType(stima_name, configuration->module_type);
    TRACE_INFO(F("--> type: %s\r\n"), stima_name);
    TRACE_INFO(F("--> main version: %u\r\n"), configuration->module_main_version);
    TRACE_INFO(F("--> minor version: %u\r\n"), configuration->module_minor_version);
    // TRACE_INFO(F("--> acquisition delay: %u [ms]\r\n"), configuration.sensor_acquisition_delay_ms);

    // TRACE_INFO(F("--> %u configured sensors\r\n"), configuration.sensors_count);
    // for (uint8_t i=0; i<configuration.sensors_count; i++) {
    //   TRACE_INFO(F("--> %u: %s-%s 0x%02X [ %s ]\r\n"), i+1, SENSOR_DRIVER_I2C, configuration.sensors[i].type, configuration.sensors[i].i2c_address, configuration.sensors[i].is_redundant ? REDUNDANT_STRING : MAIN_STRING);
    // }
    lock->Give();
  }
}

void SupervisorTask::SaveConfiguration(configuration_t *configuration, BinarySemaphore *lock, bool is_default)
{
  if (lock->Take())
  {
    if (is_default)
    {
      TRACE_INFO(F("Save default configuration... [ %s ]\r\n"), OK_STRING);
      configuration->module_main_version = MODULE_MAIN_VERSION;
      configuration->module_minor_version = MODULE_MINOR_VERSION;
      configuration->module_type = MODULE_TYPE;
      // configuration.sensor_acquisition_delay_ms = SENSORS_ACQUISITION_DELAY_MS;
    }
    else
    {
      TRACE_INFO(F("Save configuration... [ %s ]\r\n"), OK_STRING);
    }

    //! write configuration to eeprom
    eeprom.Write(CONFIGURATION_EEPROM_ADDRESS, (uint8_t *)(configuration), sizeof(configuration_t));
    lock->Give();
  }

  PrintConfiguration(configuration, lock);
}