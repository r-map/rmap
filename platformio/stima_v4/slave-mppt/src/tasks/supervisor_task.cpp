/**@file supervisor_task.cpp */

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

#define TRACE_LEVEL     SUPERVISOR_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   SUPERVISOR_TASK_ID

#include "tasks/supervisor_task.h"

using namespace cpp_freertos;

SupervisorTask::SupervisorTask(const char *taskName, uint16_t stackSize, uint8_t priority, SupervisorParam_t supervisorParam) : Thread(taskName, stackSize, priority), param(supervisorParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(SUPERVISOR_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  // Immediate load Config for all Task info
  loadConfiguration();

  state = SUPERVISOR_STATE_INIT;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void SupervisorTask::TaskMonitorStack()
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
void SupervisorTask::TaskWatchDog(uint32_t millis_standby)
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
void SupervisorTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Signal Task sleep/disabled mode from request (Auto SET WDT on Resume)
  if((param.system_status->tasks[LOCAL_TASK_ID].state == task_flag::suspended)&&
     (state_operation==task_flag::normal))
     param.system_status->tasks->watch_dog = wdt_flag::set;
  param.system_status->tasks[LOCAL_TASK_ID].state = state_operation;
  param.system_status->tasks[LOCAL_TASK_ID].running_pos = state_position;
  param.system_status->tasks[LOCAL_TASK_ID].running_sub = state_subposition;
  param.systemStatusLock->Give();
}

void SupervisorTask::Run()
{
  // Request response for system queue Task controlled...
  system_message_t system_message;

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  while (true)
  {
    switch (state)
    {
    case SUPERVISOR_STATE_INIT:
      // Load configuration is also done...
      // Seting Startup param TASK and INIT Function Here...
      param.systemStatusLock->Take();
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
            if(system_message.task_dest == SUPERVISOR_TASK_ID) {
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
          if(system_message.task_dest == ALL_TASK_ID)
          {
            // Pull && elaborate command, 
            // Sleep Global was called from CAN TASK When CAN Finished operation
            // from request and comuinication with Master. Master send flag Sleep...
            if(system_message.command.do_sleep)
            {
              // Check All Module Direct Controlled Entered in Sleep and performed
              // Local ShutDown periph, IO data ecc... IF ALL OK-> Enter LowPOWER Mode
              // ************ SLEEP ALL MODULE OK -> SLEEP SUPERVISOR ************
              // Sleeping or Suspend Module are same. Only normal_mode suspend Sleep Mode
              // SLEEP SUPERVISOR -> ALL MODULE IS SLEEPING. Tickless_mode CAN Start
              if((param.system_status->tasks[ACCELEROMETER_TASK_ID].state != task_flag::normal) &&
                 (param.system_status->tasks[CAN_TASK_ID].state != task_flag::normal) &&
                 (param.system_status->tasks[ELABORATE_TASK_ID].state != task_flag::normal)) {
                // Enter to Sleep Complete (Remove before queue Message TaskSleep)
                // Ready for Next Security Startup without Reenter Sleep
                // Next Enter with Other QueueMessage from CAN or Other Task...
                param.systemMessageQueue->Dequeue(&system_message, 0);
                // Enter task sleep (enable global LowPower procedure...)
                // Local WatchDog update
                TaskWatchDog(SUPERVISOR_TASK_SLEEP_DELAY_MS);
                TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
                // ... -> Enter LowPower on call Delay ... ->
                Delay(Ticks::MsToTicks(SUPERVISOR_TASK_SLEEP_DELAY_MS));
                TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
              }
            }
          }
        } else {

          #if (ENABLE_STACK_USAGE)
          TaskMonitorStack();
          #endif

          // Local TaskWatchDog update;
          TaskWatchDog(SUPERVISOR_TASK_WAIT_DELAY_MS);

          // Standard delay task
          DelayUntil(Ticks::MsToTicks(SUPERVISOR_TASK_WAIT_DELAY_MS));
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
/// @param None
void SupervisorTask::loadConfiguration()
{
  // param.configuration configuration Module
  // param.configurationLock Semaphore config Access
  // param.registerAccessLock Semaphore register Access
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
    param.registerAccessLock->Take();
    param.clRegister->read("rmap.module.identify", &val);
    param.registerAccessLock->Give();
    if(uavcan_register_Value_1_0_is_natural8_(&val) && (val.natural8.value.count != 3)) {
      register_config_valid = false;
    } else {
      param.configurationLock->Take();
      param.configuration->module_main_version = val.natural8.value.elements[0];
      param.configuration->module_minor_version = val.natural8.value.elements[1];
      param.configuration->module_type = val.natural8.value.elements[2];
      param.configurationLock->Give();
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
    param.registerAccessLock->Take();
    param.clRegister->read("rmap.module.sensor.count", &val);
    param.registerAccessLock->Give();
    if(uavcan_register_Value_1_0_is_natural8_(&val) && (val.natural8.value.count != 1)) {
      register_config_valid = false;
    } else {
      param.configurationLock->Take();
      param.configuration->sensors_count = val.natural8.value.elements[0];
      param.configurationLock->Give();
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
    param.registerAccessLock->Take();
    param.clRegister->read("rmap.module.sensor.acquisition", &val);
    param.registerAccessLock->Give();
    if(uavcan_register_Value_1_0_is_natural32_(&val) && (val.natural32.value.count != 1)) {
      register_config_valid = false;
    } else {
      param.configurationLock->Take();
      param.configuration->sensor_acquisition_delay_ms = val.natural32.value.elements[0];
      param.configurationLock->Give();
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
    param.registerAccessLock->Take();
    param.clRegister->read("rmap.module.sensor.config", &val);
    param.registerAccessLock->Give();
    if(uavcan_register_Value_1_0_is_natural8_(&val) && (val.natural32.value.count != elements)) {
      register_config_valid = false;
    } else {
      // Copy Register value to local_type config
      param.configurationLock->Take();
      for(uint8_t id = 0; id < param.configuration->sensors_count; id++) {
        param.configuration->sensors[id].i2c_address = val.natural8.value.elements[id*2];
        param.configuration->sensors[id].is_redundant = (val.natural8.value.elements[id*2+1] != 0);
      }
      param.configurationLock->Give();
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
    param.registerAccessLock->Take();
    param.clRegister->read("rmap.module.sensor.type", &val);
    param.registerAccessLock->Give();
    if(!uavcan_register_Value_1_0_is_string_(&val)) {
      register_config_valid = false;
    } else {
      param.configurationLock->Take();
      for(uint8_t id = 0; id < param.configuration->sensors_count; id++)
        memcpy(param.configuration->sensors[id].type, val._string.value.elements, val._string.value.count);
      param.configurationLock->Give();
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
    param.registerAccessLock->Take();
    param.clRegister->read("rmap.module.sensor.driver", &val);
    param.registerAccessLock->Give();
    if(!uavcan_register_Value_1_0_is_string_(&val)) {
      register_config_valid = false;
    } else {
      param.configurationLock->Take();
      for(uint8_t id = 0; id < param.configuration->sensors_count; id++)
        memcpy(param.configuration->sensors[id].driver, val._string.value.elements, val._string.value.count);
      param.configurationLock->Give();
    }
  }

  // INIT CONFIG (Config invalid)
  if(!register_config_valid) {
    // Reinit Configuration with default classic value
    saveConfiguration(CONFIGURATION_DEFAULT);
  }
  printConfiguration();
}

/// @brief Save/Init configuration base Register Class
/// @param is_default true if is need to prepare config default value
void SupervisorTask::saveConfiguration(bool is_default)
{
  // param.configuration configuration Module
  // param.configurationLock Semaphore config Access
  // param.registerAccessLock Semaphore register Access
  uint8_t sensor_count = 0;   // Number of Sensor
  uint8_t elements = 0;       // Array ID Element For Sensor
  static uavcan_register_Value_1_0 val = {0};  // Save configuration into register

  // Load default value to WRITE into config base
  if(is_default) {

    param.configurationLock->Take();

    param.configuration->module_main_version = MODULE_MAIN_VERSION;
    param.configuration->module_minor_version = MODULE_MINOR_VERSION;
    param.configuration->module_type = MODULE_TYPE;

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
    param.configuration->sensor_acquisition_delay_ms = SENSORS_ACQUISITION_DELAY_MS;

    // Get elements
    #if (USE_SENSOR_ADT)
    param.configuration->sensors[elements].i2c_address = 0x28;   // I2C ADDRESS
    param.configuration->sensors[elements++].is_redundant = 0;   // IS REDUNDANT
    #endif
    #if (USE_SENSOR_HIH)
    param.configuration->sensors[elements].i2c_address = 0x28;   // I2C ADDRESS
    param.configuration->sensors[elements++].is_redundant = 0;   // IS REDUNDANT
    #endif
    #if (USE_SENSOR_HYT)
    param.configuration->sensors[elements].i2c_address = HYT_DEFAULT_ADDRESS;   // I2C ADDRESS
    param.configuration->sensors[elements++].is_redundant = 0;  // IS REDUNDANT
    #if (USE_REDUNDANT_SENSOR)
    param.configuration->sensors[elements].i2c_address = HYT_REDUNDANT_ADDRESS; // I2C ADDRESS
    param.configuration->sensors[elements++].is_redundant = 1;  // IS REDUNDANT
    #endif
    #endif
    #if (USE_SENSOR_SHT)
    param.configuration->sensors[elements].i2c_address = SHT_DEFAULT_ADDRESS;   // I2C ADDRESS
    param.configuration->sensors[elements++].is_redundant = 0;  // IS REDUNDANT
    #if (USE_REDUNDANT_SENSOR)
    param.configuration->sensors[elements].i2c_address = SHT_REDUNDANT_ADDRESS; // I2C ADDRESS
    param.configuration->sensors[elements++].is_redundant = 1;  // IS REDUNDANT
    #endif
    #endif

    // Loading Default
    #if (USE_SENSOR_ADT)
    for(uint8_t id = 0; id < param.configuration->sensors_count; id++) {
      strcpy(param.configuration->sensors[id].type, SENSOR_TYPE_ADT);
      strcpy(param.configuration->sensors[id].driver, SENSOR_DRIVER_I2C);
    }
    #endif
    #if (USE_SENSOR_HIH)
    for(uint8_t id = 0; id < param.configuration->sensors_count; id++) {
      strcpy(param.configuration->sensors[id].type, SENSOR_TYPE_HYH);
      strcpy(param.configuration->sensors[id].driver, SENSOR_DRIVER_I2C);
    }
    #endif
    #if (USE_SENSOR_HYT)
    for(uint8_t id = 0; id < param.configuration->sensors_count; id++) {
      strcpy(param.configuration->sensors[id].type, SENSOR_TYPE_HYT);
      strcpy(param.configuration->sensors[id].driver, SENSOR_DRIVER_I2C);
    }
    #endif
    #if (USE_SENSOR_SHT)
    for(uint8_t id = 0; id < param.configuration->sensors_count; id++) {
      strcpy(param.configuration->sensors[id].type, SENSOR_TYPE_SHT);
      strcpy(param.configuration->sensors[id].driver, SENSOR_DRIVER_I2C);
    }
    #endif

    param.configurationLock->Give();

  }

  // Writing RMAP Module identify Param -> (READ)
  // uint8_t module_main_version;    module main version
  // uint8_t module_minor_version;   module minor version
  // uint8_t module_type;            module type
  // Select type register (uint_8)
  uavcan_register_Value_1_0_select_natural8_(&val);
  val.natural8.value.count       = 3;
  // Loading Default
  param.configurationLock->Take();
  val.natural8.value.elements[0] = param.configuration->module_main_version;
  val.natural8.value.elements[1] = param.configuration->module_minor_version;
  val.natural8.value.elements[2] = param.configuration->module_type;
  param.configurationLock->Give();
  param.registerAccessLock->Take();
  param.clRegister->write("rmap.module.identify", &val);
  param.registerAccessLock->Give();

  // Writing RMAP Module sensor count -> (READ)
  // uint8_t sensors_count
  // Select type register (uint_8)
  sensor_count = 0;
  uavcan_register_Value_1_0_select_natural8_(&val);
  val.natural8.value.count = 1;
  param.configurationLock->Take();
  val.natural8.value.elements[0] = sensor_count;
  param.configurationLock->Give();
  param.registerAccessLock->Take();
  param.clRegister->write("rmap.module.sensor.count", &val);
  param.registerAccessLock->Give();

  // Writing RMAP Module sensor delay acquire -> (READ/WRITE)
  // uint32_t sensor_acquisition_delay_ms;
  // Select type register (uint_32)
  uavcan_register_Value_1_0_select_natural32_(&val);
  val.natural32.value.count       = 1;
  // Loading Default
  param.configurationLock->Take();
  val.natural32.value.elements[0] = param.configuration->sensor_acquisition_delay_ms;
  param.configurationLock->Give();
  param.registerAccessLock->Take();
  param.clRegister->write("rmap.module.sensor.acquisition", &val);
  param.registerAccessLock->Give();

  // Writing RMAP Module sensor address -> (READ/WRITE)
  // uint8_t i2c_address[sensor_count];   // I2C Address
  // uint8_t is_redundant[sensor_count];  // Is Redundant sensor
  // Select type register (uint_8)
  uavcan_register_Value_1_0_select_natural8_(&val);
  // Loading Default
  // Total element = sensor_count x 2 => sensor_count == elements
  val.natural8.value.count = sensor_count * 2;
  // Copy Register value to local_type config
  param.configurationLock->Take();
  for(uint8_t id = 0; id < param.configuration->sensors_count; id++) {
    val.natural8.value.elements[id*2] = param.configuration->sensors[id].i2c_address;
    val.natural8.value.elements[id*2+1] = param.configuration->sensors[id].is_redundant;
  }
  param.configurationLock->Give();
  param.registerAccessLock->Take();
  param.clRegister->write("rmap.module.sensor.config", &val);
  param.registerAccessLock->Give();

  /// Writing RMAP Module sensor type -> (READ/WRITE)
  // char type[TYPE_LENGTH] same for sensor standard and redundant;
  // Select type register (string)
  uavcan_register_Value_1_0_select_string_(&val);
  // Total element (len of string)
  param.configurationLock->Take();
  val._string.value.count = strlen(param.configuration->sensors[0].type);
  memcpy(val._string.value.elements, param.configuration->sensors[0].type, val._string.value.count);
  param.configurationLock->Give();
  param.registerAccessLock->Take();
  param.clRegister->write("rmap.module.sensor.type", &val);
  param.registerAccessLock->Give();

  /// Writing RMAP Module sensor type -> (READ/WRITE)
  // char driver[DRIVER_LENGTH] same for sensor standard and redundant;
  // Select type register (string)
  uavcan_register_Value_1_0_select_string_(&val);
  // Total element (len of string)
  param.configurationLock->Take();
  val._string.value.count = strlen(param.configuration->sensors[0].driver);
  memcpy(val._string.value.elements, param.configuration->sensors[0].driver, val._string.value.count);
  param.configurationLock->Give();
  param.registerAccessLock->Take();
  param.clRegister->write("rmap.module.sensor.driver", &val);
  param.registerAccessLock->Give();

  if(is_default) {
    TRACE_INFO_F(F("SUPERVISOR: Save configuration...\r\n"));
  } else {
    TRACE_INFO_F(F("SUPERVISOR: Init configuration and save parameter...\r\n"));
  }
  printConfiguration();
}

/// @brief Print configuratione
/// @param None
void SupervisorTask::printConfiguration()
{
  // param.configuration configuration Module
  // param.configurationLock Semaphore config Access
  // param.registerAccessLock Semaphore register Access
  char stima_name[STIMA_MODULE_NAME_LENGTH];

  param.configurationLock->Take();

  getStimaNameByType(stima_name, param.configuration->module_type);
  TRACE_INFO_F(F("-> type: %s\r\n"), stima_name);
  TRACE_INFO_F(F("-> main version: %u\r\n"), param.configuration->module_main_version);
  TRACE_INFO_F(F("-> minor version: %u\r\n"), param.configuration->module_minor_version);
  TRACE_INFO_F(F("-> acquisition delay: %u [ms]\r\n"), param.configuration->sensor_acquisition_delay_ms);

  TRACE_INFO_F(F("-> %u configured sensors:\r\n"), param.configuration->sensors_count);
  for (uint8_t i = 0; i < param.configuration->sensors_count; i++)
  {
    //TODO:_TH_RAIN_ SENSOR_DRIVER_I2C PRINT -> "RAD."
    TRACE_INFO_F(F("--> %u: %s-%s 0x%02X [ %s ]\r\n"), i + 1, "RAD.", param.configuration->sensors[i].type, param.configuration->sensors[i].i2c_address, param.configuration->sensors[i].is_redundant ? REDUNDANT_STRING : MAIN_STRING);
  }

  param.configurationLock->Give();
}