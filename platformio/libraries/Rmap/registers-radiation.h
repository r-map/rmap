/**@file registers-radiation.h */

/*********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>
Marco Baldinetti <m.baldinetti@digiteco.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#ifndef _REGISTERS_SOLAR_RADIATION_H
#define _REGISTERS_SOLAR_RADIATION_H

#include "registers.h"

/*!
\def I2C_SOLAR_RADIATION_DEFAULT_ADDRESS
\brief Default address for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_DEFAULT_ADDRESS                  (0x47)

/*!
\def I2C_SOLAR_RADIATION_COMMAND_NONE
\brief NO command for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_COMMAND_NONE                 (0x00)

/*!
\def I2C_SOLAR_RADIATION_COMMAND_SAVE
\brief Save command for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_COMMAND_SAVE                     (0x01)

/*!
\def I2C_SOLAR_RADIATION_COMMAND_ONESHOT_START
\brief Oneshot start command for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_COMMAND_ONESHOT_START            (0x02)

/*!
\def I2C_SOLAR_RADIATION_COMMAND_ONESHOT_STOP
\brief Oneshot stop command for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_COMMAND_ONESHOT_STOP             (0x03)

/*!
\def I2C_SOLAR_RADIATION_COMMAND_ONESHOT_START_STOP
\brief Oneshot start-stop command for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_COMMAND_ONESHOT_START_STOP       (0x04)

/*!
\def I2C_SOLAR_RADIATION_COMMAND_CONTINUOUS_START
\brief Continuous start command for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_COMMAND_CONTINUOUS_START         (0x05)

/*!
\def I2C_SOLAR_RADIATION_COMMAND_CONTINUOUS_STOP
\brief Continuous stop command for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_COMMAND_CONTINUOUS_STOP          (0x06)

/*!
\def I2C_SOLAR_RADIATION_COMMAND_CONTINUOUS_START_STOP
\brief Continuous start-stop command for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_COMMAND_CONTINUOUS_START_STOP    (0x07)

/*!
\def I2C_SOLAR_RADIATION_COMMAND_READ_TEST
\brief Continuous test command for i2c-th module.
*/
#define I2C_SOLAR_RADIATION_COMMAND_TEST_READ                (0x08)

/*********************************************************************
* Readable registers: Specifying the length in bytes of the data by I2C_{MODULE_NAME}_{DATA_NAME}_LENGTH, the corresponding address is calculated automatically
*********************************************************************/

/*!
\def I2C_SOLAR_RADIATION_TYPE_LENGTH
\brief length of the type variable for i2c-th module.
*/
#define I2C_SOLAR_RADIATION_TYPE_LENGTH                      (0x01)

/*!
\def I2C_SOLAR_RADIATION_TYPE_ADDRESS
\brief address of the type variable for i2c-th module.
*/
#define I2C_SOLAR_RADIATION_TYPE_ADDRESS                     (I2C_READ_REGISTER_START_ADDRESS)

/*!
\def I2C_SOLAR_RADIATION_MAIN_VERSION_LENGTH
\brief length of the main version variable for i2c-th module.
*/
#define I2C_SOLAR_RADIATION_MAIN_VERSION_LENGTH                   (0x01)

/*!
\def I2C_SOLAR_RADIATION_MAIN_VERSION_ADDRESS
\brief address of the main version variable for i2c-th module.
*/
#define I2C_SOLAR_RADIATION_MAIN_VERSION_ADDRESS                  (I2C_SOLAR_RADIATION_TYPE_ADDRESS + I2C_SOLAR_RADIATION_TYPE_LENGTH)

/*!
\def I2C_SOLAR_RADIATION_MINOR_VERSION_LENGTH
\brief length of the minor version variable for i2c-th module.
*/
#define I2C_SOLAR_RADIATION_MINOR_VERSION_LENGTH                   (0x01)

/*!
\def I2C_SOLAR_RADIATION_MINOR_VERSION_ADDRESS
\brief address of the minor version variable for i2c-th module.
*/
#define I2C_SOLAR_RADIATION_MINOR_VERSION_ADDRESS                  (I2C_SOLAR_RADIATION_MAIN_VERSION_ADDRESS + I2C_SOLAR_RADIATION_MAIN_VERSION_LENGTH)

/*!
\def I2C_SOLAR_RADIATION_SAMPLE_LENGTH
\brief length of the sample variable for i2c-th module.
*/
#define I2C_SOLAR_RADIATION_SAMPLE_LENGTH        (0x02)

/*!
\def I2C_SOLAR_RADIATION_SAMPLE_ADDRESS
\brief address of the sample variable for i2c-th module.
*/
#define I2C_SOLAR_RADIATION_SAMPLE_ADDRESS       (I2C_SOLAR_RADIATION_MINOR_VERSION_ADDRESS + I2C_SOLAR_RADIATION_MINOR_VERSION_LENGTH)

/*!
\def I2C_SOLAR_RADIATION_AVERAGE_LENGTH
\brief length of the radiation wetness timer variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_AVERAGE_LENGTH                  (0x02)

/*!
\def I2C_SOLAR_RADIATION_AVERAGE_ADDRESS
\brief address of the radiation wetness timer variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_AVERAGE_ADDRESS                 (I2C_SOLAR_RADIATION_SAMPLE_ADDRESS + I2C_SOLAR_RADIATION_SAMPLE_LENGTH)

/*!
\def I2C_SOLAR_RADIATION_MAX_LENGTH
\brief length of the radiation wetness timer variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_MAX_LENGTH                      (0x02)

/*!
\def I2C_SOLAR_RADIATION_MAX_ADDRESS
\brief address of the radiation wetness timer variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_MAX_ADDRESS                     (I2C_SOLAR_RADIATION_AVERAGE_ADDRESS + I2C_SOLAR_RADIATION_AVERAGE_LENGTH)

/*!
\def I2C_SOLAR_RADIATION_FLUX_ADDRESS
\brief address of the radiation wetness timer variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_FLUX_ADDRESS                    (I2C_SOLAR_RADIATION_MAX_ADDRESS + I2C_SOLAR_RADIATION_MAX_LENGTH)

/*!
\def I2C_SOLAR_RADIATION_FLUX_LENGTH
\brief length of the radiation wetness timer variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_FLUX_LENGTH                     (0x02)

/*!
\def I2C_SOLAR_RADIATION_READABLE_DATA_LENGTH
\brief length of the readable variables for i2c-radiation module. Need to be update with with last 2 define!!!
*/
#define I2C_SOLAR_RADIATION_READABLE_DATA_LENGTH            (I2C_SOLAR_RADIATION_FLUX_ADDRESS + I2C_SOLAR_RADIATION_MAX_LENGTH - I2C_READ_REGISTER_START_ADDRESS)

/*********************************************************************
* Writable registers: Specifying the length in bytes of the data by I2C_{MODULE_NAME}_{DATA_NAME}_LENGTH, the corresponding address is calculated automatically
*********************************************************************/
/*!
\def I2C_SOLAR_RADIATION_ADDRESS_LENGTH
\brief length of the address variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_ADDRESS_LENGTH                   (0x01)

/*!
\def I2C_SOLAR_RADIATION_ADDRESS_ADDRESS
\brief address of the address variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_ADDRESS_ADDRESS                  (I2C_WRITE_REGISTER_START_ADDRESS)

/*!
\def I2C_SOLAR_RADIATION_ONESHOT_LENGTH
\brief length of the oneshot variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_ONESHOT_LENGTH                   (0x01)

/*!
\def I2C_SOLAR_RADIATION_ONESHOT_ADDRESS
\brief address of the oneshot variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_ONESHOT_ADDRESS                  (I2C_SOLAR_RADIATION_ADDRESS_ADDRESS + I2C_SOLAR_RADIATION_ADDRESS_LENGTH)

/* TO BE ADDED HERE
   // 10 bit
   float adc_voltage_offset_1;
   float adc_voltage_offset_2;
   float adc_voltage_min;
   float adc_voltage_max;
*/

#define CHANNEL_COUNT (4)

/*!
\def I2C_SOLAR_RADIATION_ADC_CALIBRATION_OFFSET_LENGTH
\brief length of the adc calibration offset variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_ADC_CALIBRATION_OFFSET_LENGTH                   (0x04*CHANNEL_COUNT)

/*!
\def I2C_SOLAR_RADIATION_ADC_CALIBRATION_OFFSET_ADDRESS
\brief address of the adc calibration offset variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_ADC_CALIBRATION_OFFSET_ADDRESS                  (I2C_SOLAR_RADIATION_ONESHOT_ADDRESS + I2C_SOLAR_RADIATION_ONESHOT_LENGTH)

/*!
\def I2C_SOLAR_RADIATION_ADC_CALIBRATION_GAIN_LENGTH
\brief length of the adc calibration gain variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_ADC_CALIBRATION_GAIN_LENGTH                   (0x04*CHANNEL_COUNT)

/*!
\def I2C_SOLAR_RADIATION_ADC_CALIBRATION_GAIN_ADDRESS
\brief address of the adc calibration gain variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_ADC_CALIBRATION_GAIN_ADDRESS                  (I2C_SOLAR_RADIATION_ADC_CALIBRATION_OFFSET_ADDRESS + I2C_SOLAR_RADIATION_ADC_CALIBRATION_OFFSET_LENGTH)

/*!
\def I2C_SOLAR_RADIATION_SENSOR_VOLTAGE_MAX_LENGTH
\brief length of the sensor voltage max variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_SENSOR_VOLTAGE_MAX_LENGTH                   (0x04*CHANNEL_COUNT)

/*!
\def I2C_SOLAR_RADIATION_SENSOR_VOLTAGE_MAX_ADDRESS
\brief address of the sensor voltage max variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_SENSOR_VOLTAGE_MAX_ADDRESS                  (I2C_SOLAR_RADIATION_ADC_CALIBRATION_GAIN_ADDRESS + I2C_SOLAR_RADIATION_ADC_CALIBRATION_GAIN_LENGTH)

/*!
\def I2C_SOLAR_RADIATION_SENSOR_RADIATION_MAX_LENGTH
\brief length of the sensor radiation max variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_SENSOR_RADIATION_MAX_LENGTH                   (0x04*CHANNEL_COUNT)

/*!
\def I2C_SOLAR_RADIATION_SENSOR_RADIATION_MAX_ADDRESS
\brief address of the sensor radiation max variable for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_SENSOR_RADIATION_MAX_ADDRESS                  (I2C_SOLAR_RADIATION_SENSOR_VOLTAGE_MAX_ADDRESS + I2C_SOLAR_RADIATION_SENSOR_VOLTAGE_MAX_LENGTH)

/*!
\def I2C_SOLAR_RADIATION_WRITABLE_DATA_LENGTH
\brief length of the writable variables for i2c-radiation module.
*/
#define I2C_SOLAR_RADIATION_WRITABLE_DATA_LENGTH             (I2C_SOLAR_RADIATION_SENSOR_RADIATION_MAX_ADDRESS + I2C_SOLAR_RADIATION_SENSOR_RADIATION_MAX_LENGTH - I2C_WRITE_REGISTER_START_ADDRESS)

// Readable registers errors checking
#if I2C_SOLAR_RADIATION_READ_REGISTERS_LENGTH > I2C_READ_REGISTER_LEN
#error "ERROR! Too many readable registers found in radiation module!!!"
#endif

// Writeable registers errors checking
#if I2C_SOLAR_RADIATION_WRITABLE_DATA_LENGTH > I2C_WRITE_REGISTER_LEN
#error "ERROR! Too many writable registers found in radiation module!!!"
#endif

#endif
