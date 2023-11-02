/**
  ******************************************************************************
  * @file    accelerometer_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   accelerometer cpp_Freertos source file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (C) 2022  Moreno Gasperini <m.gasperini@digiteco.it>
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

#define TRACE_LEVEL     ACCELEROMETER_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   ACCELEROMETER_TASK_ID

#include "tasks/accelerometer_task.h"

#if (ENABLE_ACCELEROMETER)

AccelerometerTask::AccelerometerTask(const char *taskName, uint16_t stackSize, uint8_t priority, AccelerometerParam_t accelerometerParam) : Thread(taskName, stackSize, priority), param(accelerometerParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(ACCELEROMETER_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  // Setup register mode && Load or Init configuration
  loadConfiguration();

  // Starting Class
  accelerometer = Accelerometer(param.wire, param.wireLock);

  state = ACCELEROMETER_STATE_INIT;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void AccelerometerTask::TaskMonitorStack()
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
void AccelerometerTask::TaskWatchDog(uint32_t millis_standby)
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
void AccelerometerTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
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

void AccelerometerTask::Run()
{
  // Local parameter for command and state accelerometer
  bool is_module_ready;
  uint8_t hardware_check_attempt;
  bool start_calibration = false;

  // System message data queue structured
  system_message_t system_message;

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  while (true)
  {

    // ********* SYSTEM QUEUE MESSAGE ***********
    // enqueud system message from caller task
    if (!param.systemMessageQueue->IsEmpty()) {
      // Read queue in test mode
      if (param.systemMessageQueue->Peek(&system_message, 0))
      {
        // Its request addressed into this TASK... -> pull
        if(system_message.task_dest == ACCELEROMETER_TASK_ID)
        {
          // Pull && elaborate command, after response if...
          param.systemMessageQueue->Dequeue(&system_message);
          if(system_message.command.do_calib) // == Calibrate && Save {
          {
            start_calibration = true;
          }
        }
        // Its request addressed into ALL TASK... -> no pull (only SUPERVISOR or exernal gestor)
        else if(system_message.task_dest == ALL_TASK_ID)
        {
          // Pull && elaborate command, 
          if(system_message.command.do_sleep)
          {
            // Start module sleep procedure
            powerDownModule();
            // Enter sleep module OK and update WDT
            TaskWatchDog(ACCELEROMETER_TASK_SLEEP_DELAY_MS);
            TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
            Delay(Ticks::MsToTicks(ACCELEROMETER_TASK_SLEEP_DELAY_MS));
            TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
            // Restore module from Sleep
            state = ACCELEROMETER_STATE_SETUP_MODULE;
          }
        }
      }
    }
  
    // Standard TASK switch main
    switch (state)
    {
    case ACCELEROMETER_STATE_INIT:
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_INIT -> ACCELEROMETER_STATE_CHECK_HARDWARE\r\n"));
      state = ACCELEROMETER_STATE_CHECK_HARDWARE;
      // Signal reset error to system state
      param.systemStatusLock->Take();
      param.system_status->events.is_accelerometer_error = true;
      param.system_status->events.is_bubble_level_error = false;
      param.systemStatusLock->Give();
      Delay(Ticks::MsToTicks(ACCELEROMETER_WAIT_CHECK_HARDWARE));
      hardware_check_attempt = 0;
      is_module_ready = false;
      break;

    case ACCELEROMETER_STATE_CHECK_HARDWARE:
      if (!is_module_ready)
      {
        if (checkModule()) {
          TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_CHECK_HARDWARE -> ACCELEROMETER_STATE_SETUP_MODULE\r\n"));
          state = ACCELEROMETER_STATE_SETUP_MODULE;
          break;
        }
        // Wait for HW Check
        hardware_check_attempt++;
        Delay(Ticks::MsToTicks(ACCELEROMETER_WAIT_CHECK_HARDWARE));
        if(hardware_check_attempt >= ACCELEROMETER_MAX_CHECK_ATTEMPT)
          state = ACCELEROMETER_STATE_HARDWARE_FAIL;        
      } else {        
        state = ACCELEROMETER_STATE_WAIT_RESUME;
      }
      break;

    case ACCELEROMETER_STATE_SETUP_MODULE:
      setupModule();
      is_module_ready = true;
      // System inform Accelerometer ready hardware OK
      param.systemStatusLock->Take();
      param.system_status->events.is_accelerometer_error = false;
      param.systemStatusLock->Give();
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_SETUP_MODULE -> ACCELEROMETER_STATE_CHECK_OPERATION\r\n"));
      state = ACCELEROMETER_STATE_CHECK_OPERATION;
      break;

    case ACCELEROMETER_STATE_CHECK_OPERATION:
      if(readModule()) {
        // Checking bubble error limit (with Mirror Error "Z" become 0.001 -> 0.999 X,Y 0.001 -> -0.001)
        if(((abs(value_x) > BUBBLE_ANGLE_ERROR) && (abs(value_x) < BUBBLE_ANGLE_MIRROR)) ||
           ((abs(value_y) > BUBBLE_ANGLE_ERROR) && (abs(value_y) < BUBBLE_ANGLE_MIRROR)) ||
           ((abs(value_z) > BUBBLE_ANGLE_ERROR) && (abs(value_z) < BUBBLE_ANGLE_MIRROR)))
        {
          if(!param.system_status->events.is_bubble_level_error) {
            param.systemStatusLock->Take();
            param.system_status->events.is_bubble_level_error = true;
            param.systemStatusLock->Give();
          }
        } else {
          if(param.system_status->events.is_bubble_level_error) {
            param.systemStatusLock->Take();
            param.system_status->events.is_bubble_level_error = false;
            param.systemStatusLock->Give();
          }
        }
        TRACE_INFO_F(F("X[ 0.%03d ]  |  Y[ 0.%03d ]  |  Z[ 0.%03d ]\r\n"), (int)(abs(value_x)*1000), (int)(abs(value_y)*1000), (int)(abs(value_z)*1000),  OK_STRING);
        if(start_calibration) {
          TRACE_INFO_F(F("ACCELEROMETER Start calibration\r\n"));
          calibrate(false, true);
          printConfiguration();
          start_calibration = false;
        }
      }
      break;

    case ACCELEROMETER_STATE_WAIT_RESUME:
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_END -> SUSPEND()\r\n"));
      // Disable control TASK
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
      Suspend();
      TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
      // On Restore Next INIT
      state = ACCELEROMETER_STATE_SETUP_MODULE;
      break;

    case ACCELEROMETER_STATE_HARDWARE_FAIL:
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_FAIL -> SUSPEND()\r\n"));
      // Disable control TASK
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
      Suspend();
      TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
      // On Restore Next INIT
      state = ACCELEROMETER_STATE_SETUP_MODULE;
      break;
    }

    #if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
    #endif

    // Local WatchDog update;
    TaskWatchDog(ACCELEROMETER_TASK_WAIT_DELAY_MS);

    // MAX One switch step for Task Waiting Next Step
    DelayUntil(Ticks::MsToTicks(ACCELEROMETER_TASK_WAIT_DELAY_MS));
  }
}

/// @brief Load configuration accelleration module
/// @param None
void AccelerometerTask::loadConfiguration(void)
{
  // Verify config valid param
  bool register_config_valid = true;
  // Param Reading
  static uavcan_register_Value_1_0 val = {0};

  TRACE_INFO_F(F("ACCELEROMETER: Load configuration...\r\n"));

  //! read configuration from register
  // Reading RMAP Module identify Param -> (READ/WRITE)
  // uint8_t config_valid;    TYPE ELEMENT ID
  // uint8_t module_power;    Frequency get data
  if(register_config_valid) {
    // Select type register (uint_8)
    uavcan_register_Value_1_0_select_natural8_(&val);
    val.natural8.value.count       = 2;
    // Loading Default
    val.natural8.value.elements[0] = IIS328DQ_ID;
    val.natural8.value.elements[1] = Accelerometer::IIS328DQ_ODR_5Hz2;
    param.registerAccessLock->Take();
    param.clRegister->read("rmap.accelerometer.config", &val);
    param.registerAccessLock->Give();
    if(uavcan_register_Value_1_0_is_natural8_(&val) && (val.natural8.value.count != 2)) {
      register_config_valid = false;
    } else {
      accelerometer_configuration.config_valid = val.natural8.value.elements[0];
      accelerometer_configuration.module_power = (Accelerometer::iis328dq_dr_t) val.natural8.value.elements[1];
    }
  }

  // Reading RMAP Module Offset Config -> (READ/WRITE)
  // float_t x;    Offset (X)
  // float_t y;    Offset (Y)
  // float_t z;    Offset (Z)
  if(register_config_valid) {
    // Select type register (float_)
    uavcan_register_Value_1_0_select_real32_(&val);
    val.real32.value.count       = 3;
    // Loading Default
    val.real32.value.elements[0] = 0.0;
    val.real32.value.elements[1] = 0.0;
    val.real32.value.elements[2] = 0.0;
    param.registerAccessLock->Take();
    param.clRegister->read("rmap.accelerometer.offset", &val);
    param.registerAccessLock->Give();
    if(uavcan_register_Value_1_0_is_real32_(&val) && (val.real32.value.count != 3)) {
      register_config_valid = false;
    } else {
      accelerometer_configuration.offset_x = val.real32.value.elements[0];
      accelerometer_configuration.offset_y = val.real32.value.elements[1];
      accelerometer_configuration.offset_z = val.real32.value.elements[2];
    }
  }

  // Validation Byte Config (Init Defualt Value)
  if (!register_config_valid || (accelerometer_configuration.config_valid != IIS328DQ_ID))
  {
    // Reset configuration...
    saveConfiguration(true);
  }
  printConfiguration();
}

/// @brief Print configuration Accelerometer
/// @param None
void AccelerometerTask::printConfiguration(void)
{
  TRACE_INFO_F(F("--> accelerometer config: \r\n"));
  TRACE_INFO_F(F("--> config flag: %d\r\n"), accelerometer_configuration.config_valid);
  TRACE_INFO_F(F("--> power mode: %d\r\n"), (int)accelerometer_configuration.module_power);
  TRACE_INFO_F(F("--> offset X value: %d\r\n"), (int)(accelerometer_configuration.offset_x * 1000));
  TRACE_INFO_F(F("--> offset Y value: %d\r\n"), (int)(accelerometer_configuration.offset_y * 1000));
  TRACE_INFO_F(F("--> offset Z value: %d\r\n"), (int)(accelerometer_configuration.offset_z * 1000));
}

/// @brief Init/Save configuration param Accelerometer
/// @param is_default true if need to reset register configuration value default
void AccelerometerTask::saveConfiguration(bool is_default)
{
  // Param Writing
  static uavcan_register_Value_1_0 val = {0};

  TRACE_INFO_F(F("Attempt to write accelerometer configuration... [ %s ]\r\n"), OK_STRING);    

  // Loading defualt request
  if (is_default)
  {
    accelerometer_configuration.config_valid = IIS328DQ_ID;
    accelerometer_configuration.module_power = Accelerometer::IIS328DQ_ODR_5Hz2;
    accelerometer_configuration.offset_x = 0.0f;
    accelerometer_configuration.offset_y = 0.0f;
    accelerometer_configuration.offset_z = 0.0f;
  }

  // Writing RMAP Module identify Param -> (READ/WRITE)
  // uint8_t config_valid;    TYPE ELEMENT ID
  // uint8_t module_power;    Frequency get data
  // Select type register (uint_8)
  uavcan_register_Value_1_0_select_natural8_(&val);
  val.natural8.value.count       = 2;
  val.natural8.value.elements[0] = IIS328DQ_ID;
  val.natural8.value.elements[1] = accelerometer_configuration.module_power;
  param.registerAccessLock->Take();
  param.clRegister->write("rmap.accelerometer.config", &val);
  param.registerAccessLock->Give();

  // Reading RMAP Module Offset Config -> (READ/WRITE)
  // float_t x;    Offset (X)
  // float_t y;    Offset (Y)
  // float_t z;    Offset (Z)
  // Select type register (float_)
  uavcan_register_Value_1_0_select_real32_(&val);
  val.real32.value.count       = 3;
  // Loading Default
  val.real32.value.elements[0] = accelerometer_configuration.offset_x;
  val.real32.value.elements[1] = accelerometer_configuration.offset_y;
  val.real32.value.elements[2] = accelerometer_configuration.offset_z;
  param.registerAccessLock->Take();
  param.clRegister->write("rmap.accelerometer.offset", &val);
  param.registerAccessLock->Give();
}

/// @brief Calibrate accelereometer position X-Y-Z to actual value (set offset from 0)
/// @param configuration configuration accelerometer
/// @param registerLock Semaphore register access
/// @param is_default require default value data
/// @param save_register request to save calibration in register
void AccelerometerTask::calibrate(bool is_default, bool save_register)
{
  if (is_default)
  {
    // Init offset to 0
    accelerometer_configuration.offset_x = 0.0f;
    accelerometer_configuration.offset_y = 0.0f;
    accelerometer_configuration.offset_z = 0.0f;
  } else {
    // Set offset to direct realtime Read Value
    accelerometer_configuration.offset_x = accelerometer.iis328dq_from_fsx_to_inc(Accelerometer::coordinate::X);
    accelerometer_configuration.offset_y = accelerometer.iis328dq_from_fsx_to_inc(Accelerometer::coordinate::Y);
    accelerometer_configuration.offset_z = accelerometer.iis328dq_from_fsx_to_inc(Accelerometer::coordinate::Z);
  }

  // Save Register Eeprom
  if(save_register) {
    // Param Writing
    static uavcan_register_Value_1_0 val = {0};
    // Reading RMAP Module Offset Config -> (READ/WRITE)
    // float_t x;    Offset (X)
    // float_t y;    Offset (Y)
    // float_t z;    Offset (Z)
    // Select type register (float_)
    uavcan_register_Value_1_0_select_real32_(&val);
    val.real32.value.count       = 3;
    // Loading Default
    val.real32.value.elements[0] = accelerometer_configuration.offset_x;
    val.real32.value.elements[1] = accelerometer_configuration.offset_y;
    val.real32.value.elements[2] = accelerometer_configuration.offset_z;
    param.registerAccessLock->Take();
    param.clRegister->write("rmap.accelerometer.offset", &val);
    param.registerAccessLock->Give();
  }
}

/// @brief Check hardware module
/// @return True is module ready and OK
bool AccelerometerTask::checkModule(void)
{
  bool hw_check = false;

  TRACE_INFO_F(F("Check hardware module... [ %s ]\r\n"), OK_STRING);
  /* Try whoamI response from device */
  uint8_t whoamI = 0;
    // Read ID Device
  accelerometer.iis328dq_device_id_get(&whoamI);
  if (whoamI == IIS328DQ_ID) hw_check = true;

  return hw_check;
}

/// @brief Setup hardware configuration
/// @param configuration Configuration hardware param to Accelerometer
void AccelerometerTask::setupModule(void)
{
  TRACE_INFO_F(F("Setup hardware module... [ %s ]\r\n"), OK_STRING);
  /* Enable Block Data Update */
  accelerometer.iis328dq_block_data_update_set(PROPERTY_ENABLE);
  /* Set full scale */
  accelerometer.iis328dq_full_scale_set(Accelerometer::IIS328DQ_2g);
  /* Configure filtering chain */
  /* Accelerometer - High Pass / Slope path */
  accelerometer.iis328dq_hp_path_set(Accelerometer::IIS328DQ_HP_DISABLE);
  /* Set Output Data Rate */
  accelerometer.iis328dq_data_rate_set(accelerometer_configuration.module_power);
}

/// @brief Read data from module accelerometer
/// @param configuration Param configuratione
/// @return true if data is ready from module
bool AccelerometerTask::readModule(void)
{
  bool status = false;
  /* Read output only if new value is available */
  Accelerometer::iis328dq_reg_t reg;
  accelerometer.iis328dq_status_reg_get(&reg.status_reg);
  // Is New Data avaiable
  if (reg.status_reg.zyxda) {
    status = true;
    /* Read acceleration data */
    int16_t data_raw_acceleration[3];
    memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
    accelerometer.iis328dq_acceleration_raw_get(data_raw_acceleration);
    accelerometer.push_raw_data(data_raw_acceleration);
    value_x = accelerometer.iis328dq_from_fsx_to_inc(Accelerometer::coordinate::X) - accelerometer_configuration.offset_x;
    value_y = accelerometer.iis328dq_from_fsx_to_inc(Accelerometer::coordinate::Y) - accelerometer_configuration.offset_y;
    value_z = accelerometer.iis328dq_from_fsx_to_inc(Accelerometer::coordinate::Z) - accelerometer_configuration.offset_z;
  }
  // True if data ready
  return status;
}

/// @brief Activate power saving hardware module
void AccelerometerTask::powerDownModule(void)
{
  TRACE_INFO_F(F("Power down accelerometer... [ %s ]\r\n"), OK_STRING);
  /* Set Output Data Rate to OFF */
  accelerometer.iis328dq_data_rate_set(Accelerometer::IIS328DQ_ODR_OFF);
  /* Disable Block Data Update */
  accelerometer.iis328dq_block_data_update_set(PROPERTY_DISABLE);
}

#endif