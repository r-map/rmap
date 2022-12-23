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

#define TRACE_LEVEL ACCELEROMETER_TASK_TRACE_LEVEL

#include "tasks/accelerometer_task.h"

#if (ENABLE_ACCELEROMETER)

AccelerometerTask::AccelerometerTask(const char *taskName, uint16_t stackSize, uint8_t priority, AccelerometerParam_t accelerometerParam) : Thread(taskName, stackSize, priority), param(accelerometerParam)
{
  accelerometer = Accelerometer(param.wire, param.wireLock);
  eeprom = EEprom(param.wire, param.wireLock);
  state = ACCELEROMETER_STATE_INIT;
  Start();
};

void AccelerometerTask::Run()
{
  bool is_module_ready;
  bool is_hardware_ready;
  uint8_t hardware_check_attempt;
  bool start_calibration = false;

  // System request data queue structured
  system_message_t system_request;

  while (true)
  {

    // ********* SYSTEM QUEUE REQUEST ***********
    // enqueud system request from caller task
    if (!param.systemMessageQueue->IsEmpty()) {
      // Read queue in test mode
      if (param.systemMessageQueue->Peek(&system_request, 0))
      {
        // Its request addressed into this TASK... -> pull
        if(system_request.task_dest == ACCELEROMETER_TASK_QUEUE_ID)
        {
          // Pull && elaborate command, after response if...
          param.systemMessageQueue->Dequeue(&system_request, 0);
          if(system_request.command.do_init) // == Calibrate && Save {
          {
            start_calibration = true;
          }
        }

        // Its request addressed into ALL TASK... -> no pull (only SUPERVISOR or exernal gestor)
        if(system_request.task_dest == ALL_TASK_QUEUE_ID)
        {
          // Pull && elaborate command, 
          if(system_request.command.do_sleep)
          {
            // Enter module sleep
            PowerDownModule(param.accelerometer_configuration, param.configurationLock);
            Delay(Ticks::MsToTicks(ACELLEROMETER_TASK_SLEEP_DELAY_MS));
            // Restore module
            state = ACCELEROMETER_STATE_SETUP_MODULE;
          }
        }
      }
    }
  
    // Standard TASK switch main
    switch (state)
    {
    case ACCELEROMETER_STATE_INIT:    
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_INIT -> ACCELEROMETER_STATE_CHECK_OPERATION\r\n"));
      state = ACCELEROMETER_STATE_CHECK_OPERATION;
      Delay(Ticks::MsToTicks(ACCELEROMETER_WAIT_CHECK_HARDWARE));
      hardware_check_attempt = 0;
      is_module_ready = false;
      is_hardware_ready = false;
      break;

    case ACCELEROMETER_STATE_CHECK_OPERATION:
      if (!is_module_ready)
      {
        is_hardware_ready = CheckModule(param.configurationLock);
        if (is_hardware_ready) {
          TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_CHECK_OPERATION -> ACCELEROMETER_STATE_LOAD_CONFIGURATION\r\n"));
          state = ACCELEROMETER_STATE_LOAD_CONFIGURATION;
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

    case ACCELEROMETER_STATE_LOAD_CONFIGURATION:
      LoadConfiguration(param.accelerometer_configuration, param.configurationLock);
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_LOAD_CONFIGURATION -> ACCELEROMETER_STATE_SETUP_MODULE\r\n"));
      state = ACCELEROMETER_STATE_SETUP_MODULE;
      break;

    case ACCELEROMETER_STATE_SETUP_MODULE:
      SetupModule(param.accelerometer_configuration, param.configurationLock);
      is_module_ready = true;
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_SETUP_MODULE -> ACCELEROMETER_STATE_READ\r\n"));
      state = ACCELEROMETER_STATE_READ;
      break;

    case ACCELEROMETER_STATE_READ:
      if(ReadModule(param.accelerometer_configuration, param.configurationLock)) {
        TRACE_INFO_F(F("X[ 0.%d ]  |  Y[ 0.%d ]  |  Z[ 0.%d ]\r\n"), (int)(value_x*1000), (int)(value_y*1000), (int)(value_z*1000),  OK_STRING);
        if(start_calibration) {
          TRACE_INFO_F(F("ACCELEROMETER Start calibration\r\n"));
          Calibrate(param.accelerometer_configuration, param.configurationLock, false);
          SaveConfiguration(param.accelerometer_configuration, param.configurationLock, false);
          PrintConfiguration(param.accelerometer_configuration, param.configurationLock);
          start_calibration = false;
        }
        #ifdef LOG_STACK_USAGE
        TRACE_DEBUG_F(F("ACCELEROMETER Stack Free: %d\r\n"), uxTaskGetStackHighWaterMark( NULL ));
        #endif
      }
      break;

    case ACCELEROMETER_STATE_POWER_DOWN:
      PowerDownModule(param.accelerometer_configuration, param.configurationLock);
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_POWER_DOWN -> WAIT FOR NEXT_STATE\r\n"));
      state = ACCELEROMETER_STATE_WAIT_RESUME;
      break;

    case ACCELEROMETER_STATE_SAVE_CONFIGURATION:
      SaveConfiguration(param.accelerometer_configuration, param.configurationLock, CONFIGURATION_CURRENT);
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_SAVE_CONFIGURATION -> ACCELEROMETER_STATE_LOAD_CONFIGURATION\r\n"));
      state = ACCELEROMETER_STATE_LOAD_CONFIGURATION;
      break;

    case ACCELEROMETER_STATE_WAIT_RESUME:
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_END -> SUSPEND()\r\n"));
      Suspend();
      // On Restore Next INIT
      state = ACCELEROMETER_STATE_SETUP_MODULE;
      break;

    case ACCELEROMETER_STATE_HARDWARE_FAIL:
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_FAIL -> SUSPEND()\r\n"));
      Suspend();
      // On Restore Next INIT
      state = ACCELEROMETER_STATE_SETUP_MODULE;
      break;
    }
    // MAX One switch step for Task WAiting Next Step
    DelayUntil(Ticks::MsToTicks(ACELLEROMETER_TASK_WAIT_DELAY_MS));
  }
}

bool AccelerometerTask::LoadConfiguration(accelerometer_t *configuration, BinarySemaphore *lock)
{
  bool status = false;
  //! read configuration from eeprom
  if (lock->Take())
  {
    status = true;
    TRACE_INFO_F(F("Attempt read from EEprom configuration... [ %s ]\r\n"), OK_STRING);    
    if(eeprom.Read(ACCELEROMETER_EEPROM_ADDRESS, (uint8_t *)(configuration), sizeof(accelerometer_t))) {
      TRACE_INFO_F(F("Reading from E2PROM... [ %s ]\r\n"), OK_STRING);    
    } else {
      TRACE_INFO_F(F("Reading from E2PROM... [ %s ]\r\n"), ERROR_STRING);    
    }
    lock->Give();
  }

  // Validation Byte Config (Init Defualt Value)
  if (configuration->config_valid != IIS328DQ_ID)
  {
    // Start calibration...
    status = true;
    TRACE_INFO_F(F("Reset configuration and load default... [ %s ]\r\n"), OK_STRING);
    SaveConfiguration(configuration, lock, CONFIGURATION_DEFAULT);
  }
  PrintConfiguration(configuration, lock);
  return(status);
}

void AccelerometerTask::PrintConfiguration(accelerometer_t *configuration, BinarySemaphore *lock)
{
  if (lock->Take())
  {
    TRACE_INFO_F(F("--> accelerometer config: \r\n"));
    TRACE_INFO_F(F("--> config flag: %d\r\n"), configuration->config_valid);
    TRACE_INFO_F(F("--> power mode: %d\r\n"), (int)configuration->module_power);
    TRACE_INFO_F(F("--> offset X value: %d\r\n"), (int)(configuration->offset_x * 1000));
    TRACE_INFO_F(F("--> offset Y value: %d\r\n"), (int)(configuration->offset_y * 1000));
    TRACE_INFO_F(F("--> offset Z value: %d\r\n"), (int)(configuration->offset_z * 1000));
    lock->Give();
  }
}

void AccelerometerTask::SaveConfiguration(accelerometer_t *configuration, BinarySemaphore *lock, bool is_default)
{
  if (lock->Take())
  {
    if (is_default)
    {
      configuration->config_valid = IIS328DQ_ID;
      configuration->module_power = Accelerometer::IIS328DQ_ODR_5Hz2;
      configuration->offset_x = 0.0f;
      configuration->offset_y = 0.0f;
      configuration->offset_z = 0.0f;
    }
    // ! write configuration to eeprom
    eeprom.Write(ACCELEROMETER_EEPROM_ADDRESS, (uint8_t *)(configuration), sizeof(accelerometer_t));
    TRACE_INFO_F(F("Save configuration... [ %s ]\r\n"), OK_STRING);
    lock->Give();
  }
}

void AccelerometerTask::Calibrate(accelerometer_t *configuration, BinarySemaphore *lock, bool is_default)
{
  if (lock->Take())
  {
    if (is_default)
    {
      // Init offset to 0
      configuration->offset_x = 0.0f;
      configuration->offset_y = 0.0f;
      configuration->offset_z = 0.0f;
    } else {
      // Set offset to direct realtime Read Value
      configuration->offset_x = accelerometer.iis328dq_from_fsx_to_inc(Accelerometer::coordinate::X);
      configuration->offset_y = accelerometer.iis328dq_from_fsx_to_inc(Accelerometer::coordinate::Y);
      configuration->offset_z = accelerometer.iis328dq_from_fsx_to_inc(Accelerometer::coordinate::Z);
    }
    lock->Give();
  }
}


bool AccelerometerTask::CheckModule(BinarySemaphore *lock)
{
  bool hw_check = false;
  if (lock->Take())
  {
    TRACE_INFO_F(F("Check hardware module... [ %s ]\r\n"), OK_STRING);
    /* Try whoamI response from device */
    uint8_t whoamI = 0;
      // Read ID Device
    accelerometer.iis328dq_device_id_get(&whoamI);
    if (whoamI == IIS328DQ_ID)
      hw_check = true;
    lock->Give();
  }
  return hw_check;
}

void AccelerometerTask::SetupModule(accelerometer_t *configuration, BinarySemaphore *lock)
{
  if (lock->Take())
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
    accelerometer.iis328dq_data_rate_set(configuration->module_power);
    lock->Give();
  }
}

bool AccelerometerTask::ReadModule(accelerometer_t *configuration, BinarySemaphore *lock)
{
  bool status = false;
  if (lock->Take())
  {
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
      value_x = accelerometer.iis328dq_from_fsx_to_inc(Accelerometer::coordinate::X) - configuration->offset_x;
      value_y = accelerometer.iis328dq_from_fsx_to_inc(Accelerometer::coordinate::Y) - configuration->offset_y;
      value_z = accelerometer.iis328dq_from_fsx_to_inc(Accelerometer::coordinate::Z) - configuration->offset_z;
    }
    lock->Give();
  }
  // True if data ready
  return status;
}

void AccelerometerTask::PowerDownModule(accelerometer_t *configuration, BinarySemaphore *lock)
{
  if (lock->Take())
  {
    TRACE_INFO_F(F("Power down accelerometer... [ %s ]\r\n"), OK_STRING);
    /* Set Output Data Rate to OFF */
    accelerometer.iis328dq_data_rate_set(Accelerometer::IIS328DQ_ODR_OFF);
    /* Disable Block Data Update */
    accelerometer.iis328dq_block_data_update_set(PROPERTY_DISABLE);
    lock->Give();
  }
}

#endif