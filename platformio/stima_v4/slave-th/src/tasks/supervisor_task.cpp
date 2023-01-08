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
  // Setup register mode && Load or Init configuration
  clRegister = EERegister(param.wire, param.wireLock);
  loadConfiguration(param.configuration, param.configurationLock, param.registerAccessLock);

  state = SUPERVISOR_STATE_INIT;
  Start();
};

void SupervisorTask::Run()
{
  // Request response for system queue Task controlled...
  system_message_t system_message;

  while (true)
  {
    switch (state)
    {
    case SUPERVISOR_STATE_INIT:
      // Load configuration is done...
      // Seting Startup param TASK
      param.systemStatusLock->Take();
      // Task check data
      #ifdef LOG_STACK_USAGE
      param.system_status->task.accelerometer_stack = 0xFFFF;
      param.system_status->task.can_stack = 0xFFFF;
      param.system_status->task.elaborate_data_stack = 0xFFFF;
      param.system_status->task.supervisor_stack = 0xFFFF;
      param.system_status->task.th_sensor_stack = 0xFFFF;
      #endif
      // Done Load config && Starting function
      param.system_status->flags.is_cfg_loaded = true;
      param.systemStatusLock->Give();

      state = SUPERVISOR_STATE_CHECK_OPERATION;
      TRACE_ERROR_F(F("SUPERVISOR_STATE_INIT -> SUPERVISOR_STATE_CHECK_OPERATION\r\n"));
      break;

    case SUPERVISOR_STATE_CHECK_OPERATION:
      // true if configuration ok and loaded -> 
      if (param.system_status->flags.is_cfg_loaded)
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
                param.systemStatusLock->Take();
                if(system_message.param != 0) {
                  param.system_status->flags.is_maintenance = true;
                } else {
                  param.system_status->flags.is_maintenance = false;
                }
                param.systemStatusLock->Give();
              }
            }
          }
          // Its request addressed into ALL TASK... -> no pull (only SUPERVISOR or exernal gestor)
          if(system_message.task_dest == ALL_TASK_QUEUE_ID)
          {
            // Pull && elaborate command, 
            if(system_message.command.do_sleep)
            {
              // Check All Module Waiting for Sleep are ready to Sleep...
              if(param.system_status->task.accelerometer_sleep &&
                param.system_status->task.can_sleep &&
                param.system_status->task.elaborate_data_sleep) {
                // Enter to Sleep Complete (Remove before queue Message TaskSleep)
                // Ready for Next Security Startup without Reenter Sleep
                // Next Enter with Other QueueMessage from CAN or Other Task...
                param.systemMessageQueue->Dequeue(&system_message, 0);
                // Enter task sleep (enable global LowPower procedure...)
                Delay(Ticks::MsToTicks(SUPERVISOR_TASK_SLEEP_DELAY_MS));
              }
            }
          }
        } else {
          #ifdef LOG_STACK_USAGE
          static u_int16_t stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark( NULL );
          if((stackUsage) && (stackUsage < param.system_status->task.supervisor_stack)) {
            param.systemStatusLock->Take();
            param.system_status->task.supervisor_stack = stackUsage;
            param.systemStatusLock->Give();
          }
          #endif
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

    case SUPERVISOR_STATE_END:
      TRACE_VERBOSE_F(F("SUPERVISOR_STATE_END -> SUPERVISOR_STATE_CHECK_OPERATION\r\n"));
      state = SUPERVISOR_STATE_CHECK_OPERATION;
      break;
    }
  }
}

/// @brief Load configuration from Register
/// @param configuration param configuration module
/// @param lockConfig Semaphore config Access
/// @param lockRegister Semaphore register Access
void SupervisorTask::loadConfiguration(configuration_t *configuration, BinarySemaphore *lockConfig, BinarySemaphore *lockRegister)
{
  // Verify if config valid
  bool register_config_valid = true;
  // Param Reading
  static uavcan_register_Value_1_0 val = {0};

  TRACE_INFO_F(F("SUPERVISOR: Load configuration...\r\n"));

  // Reading RMAP Module identify Param -> (READ)
  // uint8_t module_main_version;    module main version
  // uint8_t module_minor_version;   module minor version
  // uint8_t module_type;            module type
  if(register_config_valid) {
    // Select type register (uint_8)
    uavcan_register_Value_1_0_select_natural8_(&val);
    val.natural8.value.count       = 3;
    // Loading Default
    val.natural8.value.elements[0] = MODULE_MAIN_VERSION;
    val.natural8.value.elements[1] = MODULE_MINOR_VERSION;
    val.natural8.value.elements[2] = MODULE_TYPE;
    lockRegister->Take();
    clRegister.read("rmap.module.identify", &val);
    lockRegister->Give();
    if(uavcan_register_Value_1_0_is_natural8_(&val) && (val.natural8.value.count != 3)) {
      register_config_valid = false;
    } else {
      lockConfig->Take();
      configuration->module_main_version = val.natural8.value.elements[0];
      configuration->module_minor_version = val.natural8.value.elements[1];
      configuration->module_type = val.natural8.value.elements[2];
      lockConfig->Give();
    }
  }

  // Reading RMAP Module sensor count -> (READ)
  // uint8_t sensors_count
  if(register_config_valid) {
    // Select type register (uint_8)
    uint8_t sensor_count = 0;
    uavcan_register_Value_1_0_select_natural8_(&val);
    val.natural8.value.count = 1;
    #if (USE_SENSOR_ADT)
    sensor_count++;
    #endif
    #if (USE_SENSOR_HIH)
    sensor_count++;
    #endif
    #if (USE_SENSOR_HYT)
    sensor_count++;
    #if (USE_REDUNDANT_SENSOR)
    sensor_count++;
    #endif
    #endif
    #if (USE_SENSOR_SHT)
    sensor_count++;
    #if (USE_REDUNDANT_SENSOR)
    sensor_count++;
    #endif
    #endif
    // Loading Default
    val.natural8.value.elements[0] = sensor_count;
    lockRegister->Take();
    clRegister.read("rmap.module.sensor.count", &val);
    lockRegister->Give();
    if(uavcan_register_Value_1_0_is_natural8_(&val) && (val.natural8.value.count != 1)) {
      register_config_valid = false;
    } else {
      lockConfig->Take();
      configuration->sensors_count = val.natural8.value.elements[0];
      lockConfig->Give();
    }
  }

  // Reading RMAP Module sensor delay acquire -> (READ/WRITE)
  // uint32_t sensor_acquisition_delay_ms;
  if(register_config_valid) {
    // Select type register (uint_32)
    uavcan_register_Value_1_0_select_natural32_(&val);
    val.natural32.value.count       = 1;
    // Loading Default
    val.natural32.value.elements[0] = SENSORS_ACQUISITION_DELAY_MS;
    lockRegister->Take();
    clRegister.read("rmap.module.sensor.acquisition", &val);
    lockRegister->Give();
    if(uavcan_register_Value_1_0_is_natural32_(&val) && (val.natural32.value.count != 1)) {
      register_config_valid = false;
    } else {
      lockConfig->Take();
      configuration->sensor_acquisition_delay_ms = val.natural32.value.elements[0];
      lockConfig->Give();
    }
  }

  // Reading RMAP Module sensor address -> (READ/WRITE)
  // uint8_t i2c_address[sensor_count];   // I2C Address
  // uint8_t is_redundant[sensor_count];  // Is Redundant sensor
  if(register_config_valid) {
    u_int8_t elements = 0;
    // Select type register (uint_8)
    uavcan_register_Value_1_0_select_natural8_(&val);
    // Loading Default
    #if (USE_SENSOR_ADT)
    val.natural8.value.elements[elements++] = 0x28;   // I2C ADDRESS
    val.natural8.value.elements[elements++] = 0;      // IS REDUNDANT
    #endif
    #if (USE_SENSOR_HIH)
    val.natural8.value.elements[elements++] = 0x28;   // I2C ADDRESS
    val.natural8.value.elements[elements++] = 0;      // IS REDUNDANT
    #endif
    #if (USE_SENSOR_HYT)
    val.natural8.value.elements[elements++] = HYT_DEFAULT_ADDRESS;   // ADDRESS
    val.natural8.value.elements[elements++] = 0;                     // IS REDUNDANT
    #if (USE_REDUNDANT_SENSOR)
    val.natural8.value.elements[elements++] = HYT_REDUNDANT_ADDRESS; // ADDRESS
    val.natural8.value.elements[elements++] = 1;                     // IS REDUNDANT
    #endif
    #endif
    #if (USE_SENSOR_SHT)
    val.natural8.value.elements[elements++] = SHT_DEFAULT_ADDRESS;   // ADDRESS
    val.natural8.value.elements[elements++] = 0;                     // IS REDUNDANT
    #if (USE_REDUNDANT_SENSOR)
    val.natural8.value.elements[elements++] = SHT_REDUNDANT_ADDRESS; // ADDRESS
    val.natural8.value.elements[elements++] = 1;                     // IS REDUNDANT
    #endif
    #endif
    // Total element
    val.natural8.value.count = elements;
    lockRegister->Take();
    clRegister.read("rmap.module.sensor.config", &val);
    lockRegister->Give();
    if(uavcan_register_Value_1_0_is_natural8_(&val) && (val.natural32.value.count != elements)) {
      register_config_valid = false;
    } else {
      // Copy Register value to local_type config
      lockConfig->Take();
      for(uint8_t id = 0; id < configuration->sensors_count; id++) {
        configuration->sensors[id].i2c_address = val.natural8.value.elements[id*2];
        configuration->sensors[id].is_redundant = (val.natural8.value.elements[id*2+1] != 0);
      }
      lockConfig->Give();
    }
  }

  /// Reading RMAP Module sensor address -> (READ/WRITE)
  // char type[TYPE_LENGTH];
  if(register_config_valid) {
    // Select type register (string)
    uavcan_register_Value_1_0_select_string_(&val);
    // Loading Default
    #if (USE_SENSOR_ADT)
    strcpy((char*)val._string.value.elements, SENSOR_TYPE_ADT);
    val._string.value.count = strlen(SENSOR_TYPE_ADT);
    #endif
    #if (USE_SENSOR_HIH)
    strcpy((char*)val._string.value.elements, SENSOR_TYPE_HYH);
    val._string.value.count = strlen(SENSOR_TYPE_HYH);
    #endif
    #if (USE_SENSOR_HYT)
    strcpy((char*)val._string.value.elements, SENSOR_TYPE_HYT);
    val._string.value.count = strlen(SENSOR_TYPE_HYT);
    #endif
    #if (USE_SENSOR_SHT)
    strcpy((char*)val._string.value.elements, SENSOR_TYPE_SHT);
    val._string.value.count = strlen(SENSOR_TYPE_SHT);
    #endif
    // Total element
    lockRegister->Take();
    clRegister.read("rmap.module.sensor.type", &val);
    lockRegister->Give();
    if(!uavcan_register_Value_1_0_is_string_(&val)) {
      register_config_valid = false;
    } else {
      lockConfig->Take();
      for(uint8_t id = 0; id < configuration->sensors_count; id++)
        memcpy(configuration->sensors[id].type, val._string.value.elements, val._string.value.count);
      lockConfig->Give();
    }
  }

  // Reading RMAP Module sensor driver -> (READ/WRITE)
  // char driver[DRIVER_LENGTH];
  if(register_config_valid) {
    // Select type register (string)
    uavcan_register_Value_1_0_select_string_(&val);
    // Loading Default
    #if (USE_SENSOR_ADT)
    strcpy((char*)val._string.value.elements, SENSOR_DRIVER_I2C);
    val._string.value.count = strlen(SENSOR_DRIVER_I2C);
    #endif
    #if (USE_SENSOR_HIH)
    strcpy((char*)val._string.value.elements, SENSOR_DRIVER_I2C);
    val._string.value.count = strlen(SENSOR_DRIVER_I2C);
    #endif
    #if (USE_SENSOR_HYT)
    strcpy((char*)val._string.value.elements, SENSOR_DRIVER_I2C);
    val._string.value.count = strlen(SENSOR_DRIVER_I2C);
    #endif
    #if (USE_SENSOR_SHT)
    strcpy((char*)val._string.value.elements, SENSOR_DRIVER_I2C);
    val._string.value.count = strlen(SENSOR_DRIVER_I2C);
    #endif
    // Total element
    lockRegister->Take();
    clRegister.read("rmap.module.sensor.driver", &val);
    lockRegister->Give();
    if(!uavcan_register_Value_1_0_is_string_(&val)) {
      register_config_valid = false;
    } else {
      lockConfig->Take();
      for(uint8_t id = 0; id < configuration->sensors_count; id++)
        memcpy(configuration->sensors[id].driver, val._string.value.elements, val._string.value.count);
      lockConfig->Give();
    }
  }

  // INIT CONFIG (Config invalid)
  if(!register_config_valid) {
    // Reinit Configuration with default classic value
    saveConfiguration(configuration, lockConfig, lockRegister, true);
  }
  printConfiguration(configuration, lockConfig);
}

/// @brief Save/Init configuration base Register Class
/// @param configuration Module configuration
/// @param lockConfig Semaphore config Access
/// @param lockRegister Semaphore register Access
/// @param is_default true if is need to prepare config default value
void SupervisorTask::saveConfiguration(configuration_t *configuration, BinarySemaphore *lockConfig, BinarySemaphore *lockRegister, bool is_default)
{
  uint8_t sensor_count = 0;   // Number of Sensor
  uint8_t elements = 0;       // Array ID Element For Sensor
  static uavcan_register_Value_1_0 val = {0};  // Save configuration into register

  // Load default value to WRITE into config base
  if(is_default) {

    lockConfig->Take();

    configuration->module_main_version = MODULE_MAIN_VERSION;
    configuration->module_minor_version = MODULE_MINOR_VERSION;
    configuration->module_type = MODULE_TYPE;

    // Get sensor_count
    #if (USE_SENSOR_ADT)
    sensor_count++;
    #endif
    #if (USE_SENSOR_HIH)
    sensor_count++;
    #endif
    #if (USE_SENSOR_HYT)
    sensor_count++;
    #if (USE_REDUNDANT_SENSOR)
    sensor_count++;
    #endif
    #endif
    #if (USE_SENSOR_SHT)
    sensor_count++;
    #if (USE_REDUNDANT_SENSOR)
    sensor_count++;
    #endif
    #endif

    // Acquisition time sensor default
    configuration->sensor_acquisition_delay_ms = SENSORS_ACQUISITION_DELAY_MS;

    // Get elements
    #if (USE_SENSOR_ADT)
    configuration->sensors[elements].i2c_address = 0x28;   // I2C ADDRESS
    configuration->sensors[elements++].is_redundant = 0;   // IS REDUNDANT
    #endif
    #if (USE_SENSOR_HIH)
    configuration->sensors[elements].i2c_address = 0x28;   // I2C ADDRESS
    configuration->sensors[elements++].is_redundant = 0;   // IS REDUNDANT
    #endif
    #if (USE_SENSOR_HYT)
    configuration->sensors[elements].i2c_address = HYT_DEFAULT_ADDRESS;   // I2C ADDRESS
    configuration->sensors[elements++].is_redundant = 0;  // IS REDUNDANT
    #if (USE_REDUNDANT_SENSOR)
    configuration->sensors[elements].i2c_address = HYT_REDUNDANT_ADDRESS; // I2C ADDRESS
    configuration->sensors[elements++].is_redundant = 1;  // IS REDUNDANT
    #endif
    #endif
    #if (USE_SENSOR_SHT)
    configuration->sensors[elements].i2c_address = SHT_DEFAULT_ADDRESS;   // I2C ADDRESS
    configuration->sensors[elements++].is_redundant = 0;  // IS REDUNDANT
    #if (USE_REDUNDANT_SENSOR)
    configuration->sensors[elements].i2c_address = SHT_REDUNDANT_ADDRESS; // I2C ADDRESS
    configuration->sensors[elements++].is_redundant = 1;  // IS REDUNDANT
    #endif
    #endif

    // Loading Default
    #if (USE_SENSOR_ADT)
    for(uint8_t id = 0; id < configuration->sensors_count; id++) {
      strcpy(configuration->sensors[id].type, SENSOR_TYPE_ADT);
      strcpy(configuration->sensors[id].driver, SENSOR_DRIVER_I2C);
    }
    #endif
    #if (USE_SENSOR_HIH)
    for(uint8_t id = 0; id < configuration->sensors_count; id++) {
      strcpy(configuration->sensors[id].type, SENSOR_TYPE_HYH);
      strcpy(configuration->sensors[id].driver, SENSOR_DRIVER_I2C);
    }
    #endif
    #if (USE_SENSOR_HYT)
    for(uint8_t id = 0; id < configuration->sensors_count; id++) {
      strcpy(configuration->sensors[id].type, SENSOR_TYPE_HYT);
      strcpy(configuration->sensors[id].driver, SENSOR_DRIVER_I2C);
    }
    #endif
    #if (USE_SENSOR_SHT)
    for(uint8_t id = 0; id < configuration->sensors_count; id++) {
      strcpy(configuration->sensors[id].type, SENSOR_TYPE_SHT);
      strcpy(configuration->sensors[id].driver, SENSOR_DRIVER_I2C);
    }
    #endif

    lockConfig->Give();

  }

  // Writing RMAP Module identify Param -> (READ)
  // uint8_t module_main_version;    module main version
  // uint8_t module_minor_version;   module minor version
  // uint8_t module_type;            module type
  // Select type register (uint_8)
  uavcan_register_Value_1_0_select_natural8_(&val);
  val.natural8.value.count       = 3;
  // Loading Default
  lockConfig->Take();
  val.natural8.value.elements[0] = configuration->module_main_version;
  val.natural8.value.elements[1] = configuration->module_minor_version;
  val.natural8.value.elements[2] = configuration->module_type;
  lockConfig->Give();
  lockRegister->Take();
  clRegister.write("rmap.module.identify", &val);
  lockRegister->Give();

  // Writing RMAP Module sensor count -> (READ)
  // uint8_t sensors_count
  // Select type register (uint_8)
  sensor_count = 0;
  uavcan_register_Value_1_0_select_natural8_(&val);
  val.natural8.value.count = 1;
  lockConfig->Take();
  val.natural8.value.elements[0] = sensor_count;
  lockConfig->Give();
  lockRegister->Take();
  clRegister.write("rmap.module.sensor.count", &val);
  lockRegister->Give();

  // Writing RMAP Module sensor delay acquire -> (READ/WRITE)
  // uint32_t sensor_acquisition_delay_ms;
  // Select type register (uint_32)
  uavcan_register_Value_1_0_select_natural32_(&val);
  val.natural32.value.count       = 1;
  // Loading Default
  lockConfig->Take();
  val.natural32.value.elements[0] = configuration->sensor_acquisition_delay_ms;
  lockConfig->Give();
  lockRegister->Take();
  clRegister.write("rmap.module.sensor.acquisition", &val);
  lockRegister->Give();

  // Writing RMAP Module sensor address -> (READ/WRITE)
  // uint8_t i2c_address[sensor_count];   // I2C Address
  // uint8_t is_redundant[sensor_count];  // Is Redundant sensor
  // Select type register (uint_8)
  uavcan_register_Value_1_0_select_natural8_(&val);
  // Loading Default
  // Total element = sensor_count x 2 => sensor_count == elements
  val.natural8.value.count = sensor_count * 2;
  // Copy Register value to local_type config
  lockConfig->Take();
  for(uint8_t id = 0; id < configuration->sensors_count; id++) {
    val.natural8.value.elements[id*2] = configuration->sensors[id].i2c_address;
    val.natural8.value.elements[id*2+1] = configuration->sensors[id].is_redundant;
  }
  lockConfig->Give();
  lockRegister->Take();
  clRegister.write("rmap.module.sensor.config", &val);
  lockRegister->Give();

  /// Writing RMAP Module sensor type -> (READ/WRITE)
  // char type[TYPE_LENGTH] same for sensor standard and redundant;
  // Select type register (string)
  uavcan_register_Value_1_0_select_string_(&val);
  // Total element (len of string)
  lockConfig->Take();
  val._string.value.count = strlen(configuration->sensors[0].type);
  memcpy(val._string.value.elements, configuration->sensors[0].type, val._string.value.count);
  lockConfig->Give();
  lockRegister->Take();
  clRegister.write("rmap.module.sensor.type", &val);
  lockRegister->Give();

  /// Writing RMAP Module sensor type -> (READ/WRITE)
  // char driver[DRIVER_LENGTH] same for sensor standard and redundant;
  // Select type register (string)
  uavcan_register_Value_1_0_select_string_(&val);
  // Total element (len of string)
  lockConfig->Take();
  val._string.value.count = strlen(configuration->sensors[0].driver);
  memcpy(val._string.value.elements, configuration->sensors[0].driver, val._string.value.count);
  lockConfig->Give();
  lockRegister->Take();
  clRegister.write("rmap.module.sensor.driver", &val);
  lockRegister->Give();

  if(is_default) {
    TRACE_INFO_F(F("SUPERVISOR: Save configuration...\r\n"));
  } else {
    TRACE_INFO_F(F("SUPERVISOR: Init configuration and save parameter...\r\n"));
  }
  printConfiguration(configuration, lockConfig);
}

/// @brief Print configuratione
/// @param configuration param configuration module
/// @param lockConfig Semaphore for configuration access
void SupervisorTask::printConfiguration(configuration_t *configuration, BinarySemaphore *lockConfig)
{
  char stima_name[STIMA_MODULE_NAME_LENGTH];

  lockConfig->Take();

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

  lockConfig->Give();
}