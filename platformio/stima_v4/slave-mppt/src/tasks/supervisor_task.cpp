/**@file supervisor_task.cpp */

/*********************************************************************
<h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
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
  uint16_t stackUsage = (uint16_t)uxTaskGetStackHighWaterMark( NULL );
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
     param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
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
        // Security autoremove maintenance after 1 hour from calling starting method
        if(param.system_status->flags.is_maintenance) {
          if((rtc.getEpoch() - param.system_status->datetime.time_start_maintenance) > SUPERVISOR_AUTO_END_MAINTENANCE_SEC) {
            param.systemStatusLock->Take();
            param.system_status->flags.is_maintenance = false;
            param.systemStatusLock->Give();
          }
        }
        // Standard SUPERVISOR_OPERATION SYSTEM CHECK...
        // ********* SYSTEM QUEUE REQUEST ***********
        // Check Queue command system status
        if(!param.systemMessageQueue->IsEmpty()) {
          if(param.systemMessageQueue->Peek(&system_message, 0)) {
            if(system_message.task_dest == SUPERVISOR_TASK_ID) {
              // Command direct for TASK remove from queue
              param.systemMessageQueue->Dequeue(&system_message);
              if(system_message.command.do_maint) {
                param.systemStatusLock->Take();
                if(system_message.param != 0) {
                  param.system_status->flags.is_maintenance = true;
                  // Save maintenance start epoch (reset automatic)
                  param.system_status->datetime.time_start_maintenance = rtc.getEpoch();
                } else {
                  param.system_status->flags.is_maintenance = false;
                }
                param.systemStatusLock->Give();
              }
            }
            // Its request addressed into ALL TASK... -> no pull (only SUPERVISOR or exernal gestor)
            else if(system_message.task_dest == ALL_TASK_ID)
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
                if((param.system_status->tasks[CAN_TASK_ID].state != task_flag::normal) &&
                  (param.system_status->tasks[ELABORATE_TASK_ID].state != task_flag::normal)) {
                  // Enter to Sleep Complete (Remove before queue Message TaskSleep)
                  // Ready for Next Security Startup without Reenter Sleep
                  // Next Enter with Other QueueMessage from CAN or Other Task...
                  param.systemMessageQueue->Dequeue(&system_message);
                  // Enter task sleep (enable global LowPower procedure...)
                  // Local WatchDog update
                  TaskWatchDog(SUPERVISOR_TASK_SLEEP_DELAY_MS);
                  TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
                  // ... -> Enter LowPower on call Delay ... ->
                  Delay(Ticks::MsToTicks(SUPERVISOR_TASK_SLEEP_DELAY_MS));
                  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
                }
              }
            }
          }
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

    // one step switch non blocking
    #if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
    #endif

    // Local TaskWatchDog update;
    TaskWatchDog(SUPERVISOR_TASK_WAIT_DELAY_MS);

    // Standard delay task
    DelayUntil(Ticks::MsToTicks(SUPERVISOR_TASK_WAIT_DELAY_MS));

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

  // Reading RMAP Module identify Param -> (READ) + Check(REWRITE) -> [Readonly]
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
      // Parameter change for update firmare or local modify register?
      // Reinit value: Register are readonly purpose utility remote application
      if((param.configuration->module_main_version != MODULE_MAIN_VERSION)||
         (param.configuration->module_minor_version != MODULE_MINOR_VERSION)||
         (param.configuration->module_type != MODULE_TYPE)) {
        // Rewrite Register...
        val.natural8.value.elements[0] = MODULE_MAIN_VERSION;
        val.natural8.value.elements[1] = MODULE_MINOR_VERSION;
        val.natural8.value.elements[2] = MODULE_TYPE;
        // Reload parameter correct
        param.configuration->module_main_version = MODULE_MAIN_VERSION;
        param.configuration->module_minor_version = MODULE_MINOR_VERSION;
        param.configuration->module_type = MODULE_TYPE;
        param.registerAccessLock->Take();
        param.clRegister->write("rmap.module.identify", &val);
        param.registerAccessLock->Give();
      }
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

    // Acquisition time sensor default
    param.configuration->sensor_acquisition_delay_ms = SENSORS_ACQUISITION_DELAY_MS;

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

  param.configurationLock->Give();
}
