/**@file accelerometer_task.cpp */

/*********************************************************************
Copyright (C) 2022  Moreno Gasperini <m.gasperini@digiteco.it>
authors:
Moreno Gasperini <m.gasperini@digiteco.it>

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

#define TRACE_LEVEL ACCELEROMETER_TASK_TRACE_LEVEL

#include "tasks/accelerometer_task.h"

using namespace cpp_freertos;

AccelerometerTask::AccelerometerTask(const char *taskName, uint16_t stackSize, uint8_t priority, AccelerometerParam_t accelerometerParam) : Thread(taskName, stackSize, priority), AccelerometerParam(accelerometerParam) {
  accelerometer = Accelerometer(AccelerometerParam.wire, AccelerometerParam.wireLock);
  eeprom = EEprom(AccelerometerParam.wire, AccelerometerParam.wireLock);
  state = ACCELEROMETER_STATE_INIT;
  Start();
};

void AccelerometerTask::Run()
{
  bool is_module_ready = false;

  while (true)
  {
    switch (state)
    {
    case ACCELEROMETER_STATE_INIT:
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_INIT -> ACCELEROMETER_STATE_CHECK_OPERATION\r\n"));
      state = ACCELEROMETER_STATE_CHECK_OPERATION;
      break;

    case ACCELEROMETER_STATE_CHECK_OPERATION:
      if (!is_module_ready)
      {
        TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_CHECK_OPERATION -> ACCELEROMETER_STATE_LOAD_CONFIGURATION\r\n"));
        state = ACCELEROMETER_STATE_LOAD_CONFIGURATION;
      } else {        
        state = ACCELEROMETER_STATE_WAIT_FOREVER;
      }
      break;

    case ACCELEROMETER_STATE_LOAD_CONFIGURATION:
      LoadConfiguration(AccelerometerParam.configuration, AccelerometerParam.configurationLock);
      is_module_ready = true;
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_LOAD_CONFIGURATION -> ACCELEROMETER_STATE_SETUP_MODULE\r\n"));
      state = ACCELEROMETER_STATE_SETUP_MODULE;
      break;

    case ACCELEROMETER_STATE_SETUP_MODULE:
      SetupModule(AccelerometerParam.configuration, AccelerometerParam.configurationLock);
      is_module_ready = true;
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_SETUP_MODULE -> ACCELEROMETER_STATE_READ\r\n"));
      state = ACCELEROMETER_STATE_READ;
      break;

    case ACCELEROMETER_STATE_READ:
      ReadModule(AccelerometerParam.configuration, AccelerometerParam.configurationLock);
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_READ -> WAIT FOR NEXT_STATE\r\n"));
      break;

    case ACCELEROMETER_STATE_POWER_DOWN:
      PowerDownModule(AccelerometerParam.configuration, AccelerometerParam.configurationLock);
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_POWER_DOWN -> WAIT FOR NEXT_STATE\r\n"));
      state = ACCELEROMETER_STATE_WAIT_FOREVER;
      break;

    case ACCELEROMETER_STATE_SAVE_CONFIGURATION:
      SaveConfiguration(AccelerometerParam.configuration, AccelerometerParam.configurationLock, CONFIGURATION_CURRENT);
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_SAVE_CONFIGURATION -> ACCELEROMETER_STATE_LOAD_CONFIGURATION\r\n"));
      state = ACCELEROMETER_STATE_LOAD_CONFIGURATION;
      break;

    case ACCELEROMETER_STATE_WAIT_FOREVER:
      TRACE_VERBOSE_F(F("ACCELEROMETER_STATE_END -> SUSPEND()\r\n"));
      Suspend();
      // On Restore Next INIT
      state = ACCELEROMETER_STATE_SETUP_MODULE;
      break;
    }
  }
}

void AccelerometerTask::LoadConfiguration(accelerometer_t *configuration, BinarySemaphore *lock)
{
  //! read configuration from eeprom
  if (lock->Take())
  {
    eeprom.Read(ACCELEROMETER_EEPROM_ADDRESS, (uint8_t *)(configuration), sizeof(accelerometer_t));
    lock->Give();
  }

  // Validation Byte Config (Init Defualt Value)
  if (configuration->config_valid != 0x50)
  {
    TRACE_INFO_F(F("Reset configuration and load default... [ %s ]\r\n"), OK_STRING);
    SaveConfiguration(configuration, lock, CONFIGURATION_DEFAULT);
  }
  else
  {
    LoadConfiguration(configuration, lock);
  }
  PrintConfiguration(configuration, lock);
}

void AccelerometerTask::PrintConfiguration(accelerometer_t *configuration, BinarySemaphore *lock)
{
  if (lock->Take()) {
    TRACE_INFO_F(F("--> accelerometer config: \r\n"));
    TRACE_INFO_F(F("--> power mode: %u\r\n"), configuration->module_power);
    TRACE_INFO_F(F("--> offset X value: %f\r\n"), configuration->offset_x);
    TRACE_INFO_F(F("--> offset Y value: %f\r\n"), configuration->offset_y);
    TRACE_INFO_F(F("--> offset Z value: %f\r\n"), configuration->offset_z);
    lock->Give();
  }
}

void AccelerometerTask::SaveConfiguration(accelerometer_t *configuration, BinarySemaphore *lock, bool is_default)
{
  if (lock->Take())
  {
    if (is_default)
    {
      configuration->config_valid = 0x50;
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

void AccelerometerTask::ReadModule(accelerometer_t *configuration, BinarySemaphore *lock)
{
  if (lock->Take())
  {
    TRACE_INFO_F(F("Reading data accelerometer... [ %s ]\r\n"), OK_STRING);
    /* Read output only if new value is available */
    Accelerometer::iis328dq_reg_t reg;
    accelerometer.iis328dq_status_reg_get(&reg.status_reg);

    // Is New Data avaiable
    if (reg.status_reg.zyxda) {
      TRACE_INFO_F(F("New data is ready for get!!!... [ %s ]\r\n"), OK_STRING);
      /* Read acceleration data */
      int16_t data_raw_acceleration[3];
      memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
      accelerometer.iis328dq_acceleration_raw_get(data_raw_acceleration);
      accelerometer.push_raw_data(data_raw_acceleration);
      value_x = accelerometer.iis328dq_from_fsx_to_inc(Accelerometer::coordinate::X);
      value_y = accelerometer.iis328dq_from_fsx_to_inc(Accelerometer::coordinate::Y);
      value_z = accelerometer.iis328dq_from_fsx_to_inc(Accelerometer::coordinate::Z);
    }
    lock->Give();
  }
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
