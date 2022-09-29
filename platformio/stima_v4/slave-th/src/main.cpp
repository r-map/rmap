/**@file main.cpp */

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

#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"

void setup() {
  // error_t error = NO_ERROR;

  osInitKernel();
  init_debug(115200);
  init_wire();
  init_tasks();

  load_configuration();

  init_sensors();

  LedParam_t ledParam = {LED2_PIN, 100, 900};

  TemperatureHumidtySensorParam_t th_sensor_param;
  th_sensor_param.sensors_count = &configuration.sensors_count;
  th_sensor_param.sensors = configuration.sensors;
  th_sensor_param.acquisition_delay_ms = &configuration.sensor_acquisition_delay_ms;

  static LedTask led_task("LED 2 TASK", 100, OS_TASK_PRIORITY_01, ledParam);
  static TemperatureHumidtySensorTask th_sensor_task("TH SENSOR TASK", 1024, OS_TASK_PRIORITY_02, th_sensor_param);

  cpp_freertos::Thread::StartScheduler();
}

void loop() {}

void print_configuration() {
  char stima_name[20];
  getStimaNameByType(stima_name, configuration.module_type);
  TRACE_INFO(F("--> type: %s\r\n"), stima_name);
  TRACE_INFO(F("--> main version: %u\r\n"), configuration.module_main_version);
  TRACE_INFO(F("--> minor version: %u\r\n"), configuration.module_minor_version);
  TRACE_INFO(F("--> acquisition delay: %u [ms]\r\n"), configuration.sensor_acquisition_delay_ms);

  TRACE_INFO(F("--> %u configured sensors\r\n"), configuration.sensors_count);
  for (uint8_t i=0; i<configuration.sensors_count; i++) {
    TRACE_INFO(F("--> %u: %s-%s 0x%02X [ %s ]\r\n"), i+1, SENSOR_DRIVER_I2C, configuration.sensors[i].type, configuration.sensors[i].i2c_address, configuration.sensors[i].is_redundant ? REDUNDANT_STRING : MAIN_STRING);
  }
}

void save_configuration(bool is_default) {
  if (is_default) {
    TRACE_INFO(F("Save default configuration... [ %s ]\r\n"), OK_STRING);
    configuration.module_main_version = MODULE_MAIN_VERSION;
    configuration.module_minor_version = MODULE_MINOR_VERSION;
    configuration.module_type = MODULE_TYPE;
    configuration.sensor_acquisition_delay_ms = ACQUISITION_DELAY_MS;
    configuration.sensors_count = 0;

    #if (USE_SENSOR_ADT)
    strcpy(configuration.sensors[configuration.sensors_count].type, SENSOR_TYPE_ADT);
    configuration.sensors[configuration.sensors_count].i2c_address = 0x28;
    configuration.sensors[configuration.sensors_count].is_redundant = false;
    configuration.sensors_count++;
    #endif

    #if (USE_SENSOR_HIH)
    strcpy(configuration.sensors[configuration.sensors_count].type, SENSOR_TYPE_HIH);
    configuration.sensors[configuration.sensors_count].i2c_address = 0x28;
    configuration.sensors[configuration.sensors_count].is_redundant = false;
    configuration.sensors_count++;
    #endif

    #if (USE_SENSOR_HYT)
    strcpy(configuration.sensors[configuration.sensors_count].type, SENSOR_TYPE_HYT);
    configuration.sensors[configuration.sensors_count].i2c_address = HYT_DEFAULT_ADDRESS;
    configuration.sensors[configuration.sensors_count].is_redundant = false;
    configuration.sensors_count++;
    #endif

    #if (USE_SENSOR_SHT)
    strcpy(configuration.sensors[configuration.sensors_count].type, SENSOR_TYPE_SHT);
    configuration.sensors[configuration.sensors_count].i2c_address = SHT_DEFAULT_ADDRESS;
    configuration.sensors[configuration.sensors_count].is_redundant = false;
    configuration.sensors_count++;
    #endif
  }
  else {
    TRACE_INFO(F("Save configuration... [ %s ]\r\n"), OK_STRING);
  }

  //! write configuration to eeprom
  // ee_write(&configuration, CONFIGURATION_EEPROM_ADDRESS, sizeof(configuration));

  print_configuration();
}

void load_configuration() {
  //! read configuration from eeprom
  // ee_read(&configuration, CONFIGURATION_EEPROM_ADDRESS, sizeof(configuration));

  // if (configuration.module_type != MODULE_TYPE || configuration.module_version != MODULE_VERSION || digitalRead(CONFIGURATION_RESET_PIN) == LOW) {
  if (true) {
    save_configuration(CONFIGURATION_DEFAULT);
  }
  else {
    TRACE_INFO(F("Load configuration... [ %s ]\r\n"), OK_STRING);
    print_configuration();
  }
}

void init_tasks() {
}

void init_sensors () {
}

void init_wire() {
  #if (HARDWARE_I2C == ENABLE)
  Wire.setSCL(I2C1_SCL);
  Wire.setSDA(I2C1_SDA);
  Wire.begin();
  Wire.setClock(I2C1_BUS_CLOCK);
  #endif
}

void samples_processing() {
}

void exchange_buffers() {
}

void reset_samples_buffer() {
}

void reset_buffer() {
  #if (USE_SENSOR_TBR)
  rain_tips = 0;
  #endif
}
