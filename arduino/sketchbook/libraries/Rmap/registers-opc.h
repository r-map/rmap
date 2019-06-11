/**@file registers-opc.h */

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

#ifndef _REGISTERS_OPC_H
#define _REGISTERS_OPC_H

#include "registers.h"

/*!
\def I2C_OPC_DEFAULT_ADDRESS
\brief Default address for i2c-opc module.
*/
#define I2C_OPC_DEFAULT_ADDRESS                  (0x55)

/*!
\def I2C_OPC_COMMAND_SAVE
\brief Save command for i2c-opc module.
*/
#define I2C_OPC_COMMAND_SAVE                     (0x01)

/*!
\def I2C_OPC_COMMAND_ONESHOT_START
\brief Oneshot start command for i2c-opc module.
*/
#define I2C_OPC_COMMAND_ONESHOT_START            (0x02)

/*!
\def I2C_OPC_COMMAND_ONESHOT_STOP
\brief Oneshot stop command for i2c-opc module.
*/
#define I2C_OPC_COMMAND_ONESHOT_STOP             (0x03)

/*!
\def I2C_OPC_COMMAND_ONESHOT_START_STOP
\brief Oneshot start-stop command for i2c-opc module.
*/
#define I2C_OPC_COMMAND_ONESHOT_START_STOP       (0x04)

/*!
\def I2C_OPC_COMMAND_CONTINUOUS_START
\brief Continuous start command for i2c-opc module.
*/
#define I2C_OPC_COMMAND_CONTINUOUS_START         (0x05)

/*!
\def I2C_OPC_COMMAND_CONTINUOUS_STOP
\brief Continuous stop command for i2c-opc module.
*/
#define I2C_OPC_COMMAND_CONTINUOUS_STOP          (0x06)

/*!
\def I2C_OPC_COMMAND_CONTINUOUS_START_STOP
\brief Continuous start-stop command for i2c-opc module.
*/
#define I2C_OPC_COMMAND_CONTINUOUS_START_STOP    (0x07)

/*!
\def I2C_OPC_COMMAND_TEST_READ
\brief Continuous start-stop command for i2c-opc module.
*/
#define I2C_OPC_COMMAND_TEST_READ                (0x08)

/*********************************************************************
* Readable registers: Specifying the length in bytes of the data by I2C_{MODULE_NAME}_{DATA_NAME}_LENGTH, the corresponding address is calculated automatically
*********************************************************************/
/*!
\def I2C_OPC_TYPE_LENGTH
\brief length of the type variable for i2c-opc module.
*/
#define I2C_OPC_TYPE_LENGTH                       (0x01)

/*!
\def I2C_OPC_TYPE_ADDRESS
\brief address of the type variable for i2c-opc module.
*/
#define I2C_OPC_TYPE_ADDRESS                      (I2C_READ_REGISTER_START_ADDRESS)

/*!
\def I2C_OPC_VERSION_LENGTH
\brief length of the version variable for i2c-opc module.
*/
#define I2C_OPC_VERSION_LENGTH                    (0x01)

/*!
\def I2C_OPC_VERSION_ADDRESS
\brief address of the version variable for i2c-opc module.
*/
#define I2C_OPC_VERSION_ADDRESS                   (I2C_OPC_TYPE_ADDRESS + I2C_OPC_TYPE_LENGTH)

/*!
\def I2C_OPC_PM_SAMPLE_LENGTH
\brief length of the PM sample variable for i2c-opc module.
*/
#define I2C_OPC_PM_SAMPLE_LENGTH                  (3 * 0x04)

/*!
\def I2C_OPC_PM_SAMPLE_ADDRESS
\brief address of the PM sample variable for i2c-opc module.
*/
#define I2C_OPC_PM_SAMPLE_ADDRESS                 (I2C_OPC_VERSION_ADDRESS + I2C_OPC_VERSION_LENGTH)

/*!
\def I2C_OPC_PM_MED_LENGTH
\brief length of the PM average variable for i2c-opc module.
*/
#define I2C_OPC_PM_MED_LENGTH                     (3 * 0x04)

/*!
\def I2C_OPC_PM_MED_ADDRESS
\brief address of the PM average variable for i2c-opc module.
*/
#define I2C_OPC_PM_MED_ADDRESS                    (I2C_OPC_PM_SAMPLE_ADDRESS + I2C_OPC_PM_SAMPLE_LENGTH)

/*!
\def I2C_OPC_PM_SIGMA_LENGTH
\brief length of the PM sigma variable for i2c-opc module.
*/
#define I2C_OPC_PM_SIGMA_LENGTH                   (3 * 0x04)

/*!
\def I2C_OPC_PM_SIGMA_ADDRESS
\brief address of the PM sigma variable for i2c-opc module.
*/
#define I2C_OPC_PM_SIGMA_ADDRESS                  (I2C_OPC_PM_MED_ADDRESS + I2C_OPC_PM_MED_LENGTH)

/*!
\def I2C_OPC_BIN_0_7_MED_LENGTH
\brief length of the bin [0-7] average variable for i2c-opc module.
*/
#define I2C_OPC_BIN_0_7_MED_LENGTH                (8 * 0x02)

/*!
\def I2C_OPC_BIN_0_7_MED_ADDRESS
\brief address of the bin [0-7] average variable for i2c-opc module.
*/
#define I2C_OPC_BIN_0_7_MED_ADDRESS                (I2C_OPC_PM_SIGMA_ADDRESS + I2C_OPC_PM_SIGMA_LENGTH)

/*!
\def I2C_OPC_BIN_8_15_MED_LENGTH
\brief length of the bin [8-15] average variable for i2c-opc module.
*/
#define I2C_OPC_BIN_8_15_MED_LENGTH                 (8 * 0x02)

/*!
\def I2C_OPC_BIN_8_15_MED_ADDRESS
\brief address of the bin [8-15] average variable for i2c-opc module.
*/
#define I2C_OPC_BIN_8_15_MED_ADDRESS                (I2C_OPC_BIN_0_7_MED_ADDRESS + I2C_OPC_BIN_0_7_MED_LENGTH)

/*!
\def I2C_OPC_BIN_16_23_MED_LENGTH
\brief length of the bin [16-23] average variable for i2c-opc module.
*/
#define I2C_OPC_BIN_16_23_MED_LENGTH                 (8 * 0x02)

/*!
\def I2C_OPC_BIN_16_23_MED_ADDRESS
\brief address of the bin [16-23] average variable for i2c-opc module.
*/
#define I2C_OPC_BIN_16_23_MED_ADDRESS                (I2C_OPC_BIN_8_15_MED_ADDRESS + I2C_OPC_BIN_8_15_MED_LENGTH)

/*!
\def I2C_OPC_BIN_0_7_SIGMA_LENGTH
\brief length of the bin [0-7] sigma variable for i2c-opc module.
*/
#define I2C_OPC_BIN_0_7_SIGMA_LENGTH                 (8 * 0x02)

/*!
\def I2C_OPC_BIN_0_7_SIGMA_ADDRESS
\brief address of the bin [0-7] sigma variable for i2c-opc module.
*/
#define I2C_OPC_BIN_0_7_SIGMA_ADDRESS                (I2C_OPC_BIN_16_23_MED_ADDRESS + I2C_OPC_BIN_16_23_MED_LENGTH)

/*!
\def I2C_OPC_BIN_8_15_SIGMA_LENGTH
\brief length of the bin [8-15] sigma variable for i2c-opc module.
*/
#define I2C_OPC_BIN_8_15_SIGMA_LENGTH                 (8 * 0x02)

/*!
\def I2C_OPC_BIN_8_15_SIGMA_ADDRESS
\brief address of the bin [8-15] sigma variable for i2c-opc module.
*/
#define I2C_OPC_BIN_8_15_SIGMA_ADDRESS                (I2C_OPC_BIN_0_7_SIGMA_ADDRESS + I2C_OPC_BIN_0_7_SIGMA_LENGTH)

/*!
\def I2C_OPC_BIN_16_23_SIGMA_LENGTH
\brief length of the bin [16-23] sigma variable for i2c-opc module.
*/
#define I2C_OPC_BIN_16_23_SIGMA_LENGTH                 (8 * 0x02)

/*!
\def I2C_OPC_BIN_16_23_SIGMA_ADDRESS
\brief address of the bin [16-23] sigma variable for i2c-opc module.
*/
#define I2C_OPC_BIN_16_23_SIGMA_ADDRESS                (I2C_OPC_BIN_8_15_SIGMA_ADDRESS + I2C_OPC_BIN_8_15_SIGMA_LENGTH)

/*!
\def I2C_OPC_TEMPERATURE_MED_ADDRESS_LENGTH
\brief length of the bin [16-23] sigma variable for i2c-opc module.
*/
#define I2C_OPC_BIN_16_23_SIGMA_LENGTH                 (8 * 0x02)

/*!
\def I2C_OPC_BIN_16_23_SIGMA_ADDRESS
\brief address of the bin [16-23] sigma variable for i2c-opc module.
*/
#define I2C_OPC_BIN_16_23_SIGMA_ADDRESS                (I2C_OPC_BIN_8_15_SIGMA_ADDRESS + I2C_OPC_BIN_8_15_SIGMA_LENGTH)

/*!
\def I2C_OPC_TH_SAMPLE_LENGTH
\brief length of the temperature and humidity samples variable for i2c-opc module.
\*/
#define I2C_OPC_TH_SAMPLE_LENGTH								        (2 * 0x04)

/*!
\def I2C_OPC_TH_SAMPLE_ADDRESS
\brief address of the temperature and humidity samples variable for i2c-opc module.
\*/
#define I2C_OPC_TH_SAMPLE_ADDRESS								       (I2C_OPC_BIN_16_23_SIGMA_ADDRESS + I2C_OPC_BIN_16_23_SIGMA_LENGTH)

/*!
\def I2C_OPC_TH_MED_LENGTH
\brief length of the temperature and humidity average variable for i2c-opc module.
\*/
#define I2C_OPC_TH_MED_LENGTH								        (2 * 0x04)

/*!
\def I2C_OPC_TH_MED_ADDRESS
\brief address of the temperature and humidity average variable for i2c-opc module.
\*/
#define I2C_OPC_TH_MED_ADDRESS								       (I2C_OPC_TH_SAMPLE_ADDRESS + I2C_OPC_TH_MED_LENGTH)

/*!
\def I2C_OPC_READABLE_DATA_LENGTH
\brief length of the readable variables for i2c-opc module. Need to be update with with last 2 define!!!
*/
#define I2C_OPC_READABLE_DATA_LENGTH                (I2C_OPC_TH_MED_ADDRESS + I2C_OPC_TH_MED_LENGTH - I2C_READ_REGISTER_START_ADDRESS)

// /*!
// \def I2C_OPC_PM1_SAMPLE_LENGTH
// \brief length of the PM1 sample variable for i2c-opc module.
// */
// #define I2C_OPC_PM1_SAMPLE_LENGTH                 (0x04)
//
// /*!
// \def I2C_OPC_PM1_SAMPLE_ADDRESS
// \brief address of the PM1 sample variable for i2c-opc module.
// */
// #define I2C_OPC_PM1_SAMPLE_ADDRESS                (I2C_OPC_VERSION_ADDRESS + I2C_OPC_VERSION_LENGTH)
//
// /*!
// \def I2C_OPC_PM25_SAMPLE_LENGTH
// \brief length of the PM25 sample variable for i2c-opc module.
// */
// #define I2C_OPC_PM25_SAMPLE_LENGTH                 (0x04)
//
// /*!
// \def I2C_OPC_PM25_SAMPLE_ADDRESS
// \brief address of the PM25 sample variable for i2c-opc module.
// */
// #define I2C_OPC_PM25_SAMPLE_ADDRESS                (I2C_OPC_PM1_SAMPLE_ADDRESS + I2C_OPC_PM1_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_PM10_SAMPLE_LENGTH
// \brief length of the PM10 sample variable for i2c-opc module.
// */
// #define I2C_OPC_PM10_SAMPLE_LENGTH                 (0x04)
//
// /*!
// \def I2C_OPC_PM10_SAMPLE_ADDRESS
// \brief address of the PM10 sample variable for i2c-opc module.
// */
// #define I2C_OPC_PM10_SAMPLE_ADDRESS                (I2C_OPC_PM25_SAMPLE_ADDRESS + I2C_OPC_PM25_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_PM1_MED_LENGTH
// \brief length of the PM1 average variable for i2c-opc module.
// */
// #define I2C_OPC_PM1_MED_LENGTH                    (0x04)
//
// /*!
// \def I2C_OPC_PM1_SAMPLE_ADDRESS
// \brief address of the PM1 sample variable for i2c-opc module.
// */
// #define I2C_OPC_PM1_MED_ADDRESS                   (I2C_OPC_PM10_SAMPLE_ADDRESS + I2C_OPC_PM10_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_PM25_MED_LENGTH
// \brief length of the PM2.5 average variable for i2c-opc module.
// */
// #define I2C_OPC_PM25_MED_LENGTH                   (0x04)
//
// /*!
// \def I2C_OPC_PM25_MED_ADDRESS
// \brief address of the PM2.5 sample variable for i2c-opc module.
// */
// #define I2C_OPC_PM25_MED_ADDRESS                  (I2C_OPC_PM1_MED_ADDRESS + I2C_OPC_PM1_MED_LENGTH)
//
// /*!
// \def I2C_OPC_PM10_MED_LENGTH
// \brief length of the PM10 average variable for i2c-opc module.
// */
// #define I2C_OPC_PM10_MED_LENGTH                   (0x04)
//
// /*!
// \def I2C_OPC_PM3_MED_ADDRESS
// \brief address of the PM10 sample variable for i2c-opc module.
// */
// #define I2C_OPC_PM10_MED_ADDRESS                  (I2C_OPC_PM25_MED_ADDRESS + I2C_OPC_PM25_MED_LENGTH)
//
// /*!
// \def I2C_OPC_PM1_SIGMA_LENGTH
// \brief length of the PM1 average variable for i2c-opc module.
// */
// #define I2C_OPC_PM1_SIGMA_LENGTH                  (0x04)
//
// /*!
// \def I2C_OPC_PM1_SIGMA_ADDRESS
// \brief address of the PM1 sample variable for i2c-opc module.
// */
// #define I2C_OPC_PM1_SIGMA_ADDRESS                 (I2C_OPC_PM10_MED_ADDRESS + I2C_OPC_PM10_MED_LENGTH)
//
// /*!
// \def I2C_OPC_PM25_SIGMA_LENGTH
// \brief length of the PM2.5 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_PM25_SIGMA_LENGTH                 (0x04)
//
// /*!
// \def I2C_OPC_PM25_SIGMA_ADDRESS
// \brief address of the PM2.5 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_PM25_SIGMA_ADDRESS                 (I2C_OPC_PM1_SIGMA_ADDRESS + I2C_OPC_PM1_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_PM10_SIGMA_LENGTH
// \brief length of the PM10 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_PM10_SIGMA_LENGTH                 (0x04)
//
// /*!
// \def I2C_OPC_PM10_SIGMA_ADDRESS
// \brief address of the PM10 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_PM10_SIGMA_ADDRESS                 (I2C_OPC_PM25_SIGMA_ADDRESS + I2C_OPC_PM25_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN0_MED_LENGTH
// \brief length of the bin0 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN0_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN0_MED_ADDRESS
// \brief address of the bin0 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN0_MED_ADDRESS                  (I2C_OPC_PM10_SIGMA_ADDRESS + I2C_OPC_PM10_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN1_MED_LENGTH
// \brief length of the bin1 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN1_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN1_MED_ADDRESS
// \brief address of the bin1 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN1_MED_ADDRESS									(I2C_OPC_BIN0_MED_ADDRESS + I2C_OPC_BIN0_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN2_MED_LENGTH
// \brief length of the bin2 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN2_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN2_MED_ADDRESS
// \brief address of the bin2 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN2_MED_ADDRESS									(I2C_OPC_BIN1_MED_ADDRESS + I2C_OPC_BIN1_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN3_MED_LENGTH
// \brief length of the bin3 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN3_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN3_MED_ADDRESS
// \brief address of the bin3 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN3_MED_ADDRESS									(I2C_OPC_BIN2_MED_ADDRESS + I2C_OPC_BIN2_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN4_MED_LENGTH
// \brief length of the bin4 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN4_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN4_MED_ADDRESS
// \brief address of the bin4 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN4_MED_ADDRESS									(I2C_OPC_BIN3_MED_ADDRESS + I2C_OPC_BIN3_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN5_MED_LENGTH
// \brief length of the bin5 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN5_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN5_MED_ADDRESS
// \brief address of the bin5 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN5_MED_ADDRESS									(I2C_OPC_BIN4_MED_ADDRESS + I2C_OPC_BIN4_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN6_MED_LENGTH
// \brief length of the bin6 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN6_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN6_MED_ADDRESS
// \brief address of the bin6 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN6_MED_ADDRESS									(I2C_OPC_BIN5_MED_ADDRESS + I2C_OPC_BIN5_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN7_MED_LENGTH
// \brief length of the bin7 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN7_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN7_MED_ADDRESS
// \brief address of the bin7 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN7_MED_ADDRESS									(I2C_OPC_BIN6_MED_ADDRESS + I2C_OPC_BIN6_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN8_MED_LENGTH
// \brief length of the bin8 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN8_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN8_MED_ADDRESS
// \brief address of the bin8 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN8_MED_ADDRESS									(I2C_OPC_BIN7_MED_ADDRESS + I2C_OPC_BIN7_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN9_MED_LENGTH
// \brief length of the bin9 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN9_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN9_MED_ADDRESS
// \brief address of the bin9 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN9_MED_ADDRESS									(I2C_OPC_BIN8_MED_ADDRESS + I2C_OPC_BIN8_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN10_MED_LENGTH
// \brief length of the bin10 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN10_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN10_MED_ADDRESS
// \brief address of the bin10 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN10_MED_ADDRESS									(I2C_OPC_BIN9_MED_ADDRESS + I2C_OPC_BIN9_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN11_MED_LENGTH
// \brief length of the bin11 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN11_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN11_MED_ADDRESS
// \brief address of the bin11 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN11_MED_ADDRESS									(I2C_OPC_BIN10_MED_ADDRESS + I2C_OPC_BIN10_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN12_MED_LENGTH
// \brief length of the bin12 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN12_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN12_MED_ADDRESS
// \brief address of the bin12 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN12_MED_ADDRESS									(I2C_OPC_BIN11_MED_ADDRESS + I2C_OPC_BIN11_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN13_MED_LENGTH
// \brief length of the bin13 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN13_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN13_MED_ADDRESS
// \brief address of the bin13 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN13_MED_ADDRESS									(I2C_OPC_BIN12_MED_ADDRESS + I2C_OPC_BIN12_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN14_MED_LENGTH
// \brief length of the bin14 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN14_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN14_MED_ADDRESS
// \brief address of the bin14 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN14_MED_ADDRESS									(I2C_OPC_BIN13_MED_ADDRESS + I2C_OPC_BIN13_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN15_MED_LENGTH
// \brief length of the bin15 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN15_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN15_MED_ADDRESS
// \brief address of the bin15 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN15_MED_ADDRESS									(I2C_OPC_BIN14_MED_ADDRESS + I2C_OPC_BIN14_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN16_MED_LENGTH
// \brief length of the bin16 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN16_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN16_MED_ADDRESS
// \brief address of the bin16 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN16_MED_ADDRESS									(I2C_OPC_BIN15_MED_ADDRESS + I2C_OPC_BIN15_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN17_MED_LENGTH
// \brief length of the bin17 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN17_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN17_MED_ADDRESS
// \brief address of the bin17 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN17_MED_ADDRESS									(I2C_OPC_BIN16_MED_ADDRESS + I2C_OPC_BIN16_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN18_MED_LENGTH
// \brief length of the bin18 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN18_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN18_MED_ADDRESS
// \brief address of the bin18 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN18_MED_ADDRESS									(I2C_OPC_BIN17_MED_ADDRESS + I2C_OPC_BIN17_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN19_MED_LENGTH
// \brief length of the bin19 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN19_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN19_MED_ADDRESS
// \brief address of the bin19 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN19_MED_ADDRESS									(I2C_OPC_BIN18_MED_ADDRESS + I2C_OPC_BIN18_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN20_MED_LENGTH
// \brief length of the bin20 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN20_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN20_MED_ADDRESS
// \brief address of the bin20 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN20_MED_ADDRESS									(I2C_OPC_BIN19_MED_ADDRESS + I2C_OPC_BIN19_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN21_MED_LENGTH
// \brief length of the bin21 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN21_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN21_MED_ADDRESS
// \brief address of the bin21 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN21_MED_ADDRESS									(I2C_OPC_BIN20_MED_ADDRESS + I2C_OPC_BIN20_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN22_MED_LENGTH
// \brief length of the bin22 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN22_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN22_MED_ADDRESS
// \brief address of the bin22 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN22_MED_ADDRESS									(I2C_OPC_BIN21_MED_ADDRESS + I2C_OPC_BIN21_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN23_MED_LENGTH
// \brief length of the bin23 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN23_MED_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN23_MED_ADDRESS
// \brief address of the bin23 average variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN23_MED_ADDRESS									(I2C_OPC_BIN22_MED_ADDRESS + I2C_OPC_BIN22_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN0_SIGMA_LENGTH
// \brief length of the bin0 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN0_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN0_SIGMA_ADDRESS
// \brief address of the bin0 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN0_SIGMA_ADDRESS									(I2C_OPC_BIN23_MED_ADDRESS + I2C_OPC_BIN0_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN1_SIGMA_LENGTH
// \brief length of the bin1 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN1_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN1_SIGMA_ADDRESS
// \brief address of the bin1 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN1_SIGMA_ADDRESS									(I2C_OPC_BIN0_SIGMA_ADDRESS + I2C_OPC_BIN0_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN2_SIGMA_LENGTH
// \brief length of the bin2 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN2_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN2_SIGMA_ADDRESS
// \brief address of the bin2 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN2_SIGMA_ADDRESS									(I2C_OPC_BIN1_SIGMA_ADDRESS + I2C_OPC_BIN1_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN3_SIGMA_LENGTH
// \brief length of the bin3 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN3_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN3_SIGMA_ADDRESS
// \brief address of the bin3 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN3_SIGMA_ADDRESS									(I2C_OPC_BIN2_SIGMA_ADDRESS + I2C_OPC_BIN2_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN4_SIGMA_LENGTH
// \brief length of the bin4 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN4_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN4_SIGMA_ADDRESS
// \brief address of the bin4 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN4_SIGMA_ADDRESS									(I2C_OPC_BIN3_SIGMA_ADDRESS + I2C_OPC_BIN3_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN5_SIGMA_LENGTH
// \brief length of the bin5 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN5_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN5_SIGMA_ADDRESS
// \brief address of the bin5 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN5_SIGMA_ADDRESS									(I2C_OPC_BIN4_SIGMA_ADDRESS + I2C_OPC_BIN4_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN6_SIGMA_LENGTH
// \brief length of the bin6 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN6_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN6_SIGMA_ADDRESS
// \brief address of the bin6 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN6_SIGMA_ADDRESS									(I2C_OPC_BIN5_SIGMA_ADDRESS + I2C_OPC_BIN5_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN7_SIGMA_LENGTH
// \brief length of the bin7 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN7_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN7_SIGMA_ADDRESS
// \brief address of the bin7 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN7_SIGMA_ADDRESS									(I2C_OPC_BIN6_SIGMA_ADDRESS + I2C_OPC_BIN6_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN8_SIGMA_LENGTH
// \brief length of the bin8 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN8_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN8_SIGMA_ADDRESS
// \brief address of the bin8 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN8_SIGMA_ADDRESS									(I2C_OPC_BIN7_SIGMA_ADDRESS + I2C_OPC_BIN7_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN9_SIGMA_LENGTH
// \brief length of the bin9 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN9_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN9_SIGMA_ADDRESS
// \brief address of the bin9 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN9_SIGMA_ADDRESS									(I2C_OPC_BIN8_SIGMA_ADDRESS + I2C_OPC_BIN8_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN10_SIGMA_LENGTH
// \brief length of the bin10 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN10_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN10_SIGMA_ADDRESS
// \brief address of the bin10 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN10_SIGMA_ADDRESS									(I2C_OPC_BIN9_SIGMA_ADDRESS + I2C_OPC_BIN9_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN11_SIGMA_LENGTH
// \brief length of the bin11 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN11_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN11_SIGMA_ADDRESS
// \brief address of the bin11 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN11_SIGMA_ADDRESS									(I2C_OPC_BIN10_SIGMA_ADDRESS + I2C_OPC_BIN10_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN12_SIGMA_LENGTH
// \brief length of the bin12 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN12_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN12_SIGMA_ADDRESS
// \brief address of the bin12 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN12_SIGMA_ADDRESS									(I2C_OPC_BIN11_SIGMA_ADDRESS + I2C_OPC_BIN11_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN13_SIGMA_LENGTH
// \brief length of the bin13 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN13_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN13_SIGMA_ADDRESS
// \brief address of the bin13 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN13_SIGMA_ADDRESS									(I2C_OPC_BIN12_SIGMA_ADDRESS + I2C_OPC_BIN12_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN14_SIGMA_LENGTH
// \brief length of the bin14 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN14_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN14_SIGMA_ADDRESS
// \brief address of the bin14 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN14_SIGMA_ADDRESS									(I2C_OPC_BIN13_SIGMA_ADDRESS + I2C_OPC_BIN13_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN15_SIGMA_LENGTH
// \brief length of the bin15 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN15_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN15_SIGMA_ADDRESS
// \brief address of the bin15 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN15_SIGMA_ADDRESS									(I2C_OPC_BIN14_SIGMA_ADDRESS + I2C_OPC_BIN14_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN16_SIGMA_LENGTH
// \brief length of the bin16 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN16_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN16_SIGMA_ADDRESS
// \brief address of the bin16 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN16_SIGMA_ADDRESS									(I2C_OPC_BIN15_SIGMA_ADDRESS + I2C_OPC_BIN15_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN17_SIGMA_LENGTH
// \brief length of the bin17 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN17_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN17_SIGMA_ADDRESS
// \brief address of the bin17 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN17_SIGMA_ADDRESS									(I2C_OPC_BIN16_SIGMA_ADDRESS + I2C_OPC_BIN16_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN18_SIGMA_LENGTH
// \brief length of the bin18 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN18_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN18_SIGMA_ADDRESS
// \brief address of the bin18 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN18_SIGMA_ADDRESS									(I2C_OPC_BIN17_SIGMA_ADDRESS + I2C_OPC_BIN17_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN19_SIGMA_LENGTH
// \brief length of the bin19 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN19_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN19_SIGMA_ADDRESS
// \brief address of the bin19 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN19_SIGMA_ADDRESS									(I2C_OPC_BIN18_SIGMA_ADDRESS + I2C_OPC_BIN18_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN20_SIGMA_LENGTH
// \brief length of the bin20 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN20_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN20_SIGMA_ADDRESS
// \brief address of the bin20 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN20_SIGMA_ADDRESS									(I2C_OPC_BIN19_SIGMA_ADDRESS + I2C_OPC_BIN19_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN21_SIGMA_LENGTH
// \brief length of the bin21 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN21_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN21_SIGMA_ADDRESS
// \brief address of the bin21 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN21_SIGMA_ADDRESS									(I2C_OPC_BIN20_SIGMA_ADDRESS + I2C_OPC_BIN20_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN22_SIGMA_LENGTH
// \brief length of the bin22 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN22_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN22_SIGMA_ADDRESS
// \brief address of the bin22 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN22_SIGMA_ADDRESS									(I2C_OPC_BIN21_SIGMA_ADDRESS + I2C_OPC_BIN21_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN23_SIGMA_LENGTH
// \brief length of the bin23 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN23_SIGMA_LENGTH										(0x02)
//
// /*!
// \def I2C_OPC_BIN23_SIGMA_ADDRESS
// \brief address of the bin23 sigma variable for i2c-opc module.
// \*/
// #define I2C_OPC_BIN23_SIGMA_ADDRESS									(I2C_OPC_BIN22_SIGMA_ADDRESS + I2C_OPC_BIN22_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_TEMPERATURE_MED_LENGTH
// \brief length of the temperature average variable for i2c-opc module.
// \*/
// #define I2C_OPC_TEMPERATURE_MED_LENGTH								(0x04)
//
// /*!
// \def I2C_OPC_TEMPERATURE_MED_ADDRESS
// \brief address of the temperature average variable for i2c-opc module.
// \*/
// #define I2C_OPC_TEMPERATURE_MED_ADDRESS								(I2C_OPC_BIN23_SIGMA_ADDRESS + I2C_OPC_BIN23_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_HUMIDITY_MED_LENGTH
// \brief length of the humidity average variable for i2c-opc module.
// \*/
// #define I2C_OPC_HUMIDITY_MED_LENGTH								   (0x04)
//
// /*!
// \def I2C_OPC_HUMIDITY_MED_ADDRESS
// \brief address of the humidity average variable for i2c-opc module.
// \*/
// #define I2C_OPC_HUMIDITY_MED_ADDRESS								(I2C_OPC_TEMPERATURE_MED_ADDRESS + I2C_OPC_TEMPERATURE_MED_LENGTH)
//
// /*!
// \def I2C_OPC_READABLE_DATA_LENGTH
// \brief length of the readable variables for i2c-opc module. Need to be update with with last 2 define!!!
// */
// #define I2C_OPC_READABLE_DATA_LENGTH                (I2C_OPC_HUMIDITY_MED_ADDRESS + I2C_OPC_HUMIDITY_MED_LENGTH - I2C_READ_REGISTER_START_ADDRESS)






// /*!
// \def I2C_OPC_PM1_SAMPLE_LENGTH
// \brief length of the PM1 sample variable for i2c-opc module.
// */
// #define I2C_OPC_PM1_SAMPLE_LENGTH                 (0x04)
//
// /*!
// \def I2C_OPC_PM1_SAMPLE_ADDRESS
// \brief address of the PM1 sample variable for i2c-opc module.
// */
// #define I2C_OPC_PM1_SAMPLE_ADDRESS                (I2C_OPC_VERSION_ADDRESS + I2C_OPC_VERSION_LENGTH)
//
// /*!
// \def I2C_OPC_PM1_MED_LENGTH
// \brief length of the PM1 average variable for i2c-opc module.
// */
// #define I2C_OPC_PM1_MED_LENGTH                    (0x04)
//
// /*!
// \def I2C_OPC_PM1_MED_ADDRESS
// \brief address of the PM 1 average variable for i2c-opc module.
// */
// #define I2C_OPC_PM1_MED_ADDRESS                   (I2C_OPC_PM1_SAMPLE_ADDRESS + I2C_OPC_PM1_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_PM1_SIGMA_LENGTH
// \brief length of the PM1 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_PM1_SIGMA_LENGTH                  (0x04)
//
// /*!
// \def I2C_OPC_PM1_SIGMA_ADDRESS
// \brief address of the PM1 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_PM1_SIGMA_ADDRESS                 (I2C_OPC_PM1_MED_ADDRESS + I2C_OPC_PM1_MED_LENGTH)
//
// /*!
// \def I2C_OPC_PM25_SAMPLE_LENGTH
// \brief length of the PM2.5 sample variable for i2c-opc module.
// */
// #define I2C_OPC_PM25_SAMPLE_LENGTH                (0x04)
//
// /*!
// \def I2C_OPC_PM25_SAMPLE_ADDRESS
// \brief address of the PM2.5 sample variable for i2c-opc module.
// */
// #define I2C_OPC_PM25_SAMPLE_ADDRESS               (I2C_OPC_PM1_SIGMA_ADDRESS + I2C_OPC_PM1_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_PM25_MED_LENGTH
// \brief length of the PM2.5 average variable for i2c-opc module.
// */
// #define I2C_OPC_PM25_MED_LENGTH                   (0x04)
//
// /*!
// \def I2C_OPC_PM25_MED_ADDRESS
// \brief address of the PM2.5 average variable for i2c-opc module.
// */
// #define I2C_OPC_PM25_MED_ADDRESS                  (I2C_OPC_PM25_SAMPLE_ADDRESS + I2C_OPC_PM25_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_PM25_SIGMA_LENGTH
// \brief length of the PM2.5 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_PM25_SIGMA_LENGTH                 (0x04)
//
// /*!
// \def I2C_OPC_PM25_SIGMA_ADDRESS
// \brief address of the PM2.5 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_PM25_SIGMA_ADDRESS                (I2C_OPC_PM25_MED_ADDRESS + I2C_OPC_PM25_MED_LENGTH)
//
// /*!
// \def I2C_OPC_PM10_SAMPLE_LENGTH
// \brief length of the PM10 sample variable for i2c-opc module.
// */
// #define I2C_OPC_PM10_SAMPLE_LENGTH                (0x04)
//
// /*!
// \def I2C_OPC_PM10_SAMPLE_ADDRESS
// \brief address of the PM10 sample variable for i2c-opc module.
// */
// #define I2C_OPC_PM10_SAMPLE_ADDRESS               (I2C_OPC_PM25_SIGMA_ADDRESS + I2C_OPC_PM25_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_PM10_MED_LENGTH
// \brief length of the PM10 average variable for i2c-opc module.
// */
// #define I2C_OPC_PM10_MED_LENGTH                   (0x04)
//
// /*!
// \def I2C_OPC_PM10_MED_ADDRESS
// \brief address of the PM10 average variable for i2c-opc module.
// */
// #define I2C_OPC_PM10_MED_ADDRESS                  (I2C_OPC_PM10_SAMPLE_ADDRESS + I2C_OPC_PM10_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_PM10_SIGMA_LENGTH
// \brief length of the PM10 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_PM10_SIGMA_LENGTH                 (0x04)
//
// /*!
// \def I2C_OPC_PM10_SIGMA_ADDRESS
// \brief address of the PM10 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_PM10_SIGMA_ADDRESS                (I2C_OPC_PM10_MED_ADDRESS + I2C_OPC_PM10_MED_LENGTH)
//
// /*!
// \def I2C_OPC_BIN0_SAMPLE_LENGTH
// \brief length of the BIN0 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN0_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN0_SAMPLE_ADDRESS
// \brief address of the BIN0 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN0_SAMPLE_ADDRESS               (I2C_OPC_PM10_SIGMA_ADDRESS + I2C_OPC_PM10_SIGMA_LENGTH)
// /*!
// \def I2C_OPC_BIN0_SAMPLE_LENGTH
// \brief length of the BIN0 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN0_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN0_SAMPLE_ADDRESS
// \brief address of the BIN0 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN0_SAMPLE_ADDRESS               (I2C_OPC_BIN[x-1]_SIGMA_ADDRESS + I2C_OPC_BIN[x-1]_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN0_MED_LENGTH
// \brief length of the BIN0 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN0_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN0_MED_ADDRESS
// \brief address of the BIN0 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN0_MED_ADDRESS                  (I2C_OPC_BIN0_SAMPLE_ADDRESS + I2C_OPC_BIN0_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_BIN0_SIGMA_LENGTH
// \brief length of the BIN0 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN0_SIGMA_LENGTH                 (0x02)
//
// /*!
// \def I2C_OPC_BIN0_SIGMA_ADDRESS
// \brief address of the BIN0 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN0_SIGMA_ADDRESS                (I2C_OPC_BIN0_MED_ADDRESS + I2C_OPC_BIN0_MED_LENGTH)
// /*!
// \def I2C_OPC_BIN1_SAMPLE_LENGTH
// \brief length of the BIN1 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN1_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN1_SAMPLE_ADDRESS
// \brief address of the BIN1 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN1_SAMPLE_ADDRESS               (I2C_OPC_BIN0_SIGMA_ADDRESS + I2C_OPC_BIN0_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN1_MED_LENGTH
// \brief length of the BIN1 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN1_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN1_MED_ADDRESS
// \brief address of the BIN1 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN1_MED_ADDRESS                  (I2C_OPC_BIN1_SAMPLE_ADDRESS + I2C_OPC_BIN1_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_BIN1_SIGMA_LENGTH
// \brief length of the BIN1 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN1_SIGMA_LENGTH                 (0x02)
//
// /*!
// \def I2C_OPC_BIN1_SIGMA_ADDRESS
// \brief address of the BIN1 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN1_SIGMA_ADDRESS                (I2C_OPC_BIN1_MED_ADDRESS + I2C_OPC_BIN1_MED_LENGTH)
// /*!
// \def I2C_OPC_BIN2_SAMPLE_LENGTH
// \brief length of the BIN2 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN2_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN2_SAMPLE_ADDRESS
// \brief address of the BIN2 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN2_SAMPLE_ADDRESS               (I2C_OPC_BIN1_SIGMA_ADDRESS + I2C_OPC_BIN1_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN2_MED_LENGTH
// \brief length of the BIN2 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN2_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN2_MED_ADDRESS
// \brief address of the BIN2 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN2_MED_ADDRESS                  (I2C_OPC_BIN2_SAMPLE_ADDRESS + I2C_OPC_BIN2_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_BIN2_SIGMA_LENGTH
// \brief length of the BIN2 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN2_SIGMA_LENGTH                 (0x02)
//
// /*!
// \def I2C_OPC_BIN2_SIGMA_ADDRESS
// \brief address of the BIN2 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN2_SIGMA_ADDRESS                (I2C_OPC_BIN2_MED_ADDRESS + I2C_OPC_BIN2_MED_LENGTH)
// /*!
// \def I2C_OPC_BIN3_SAMPLE_LENGTH
// \brief length of the BIN3 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN3_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN3_SAMPLE_ADDRESS
// \brief address of the BIN3 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN3_SAMPLE_ADDRESS               (I2C_OPC_BIN2_SIGMA_ADDRESS + I2C_OPC_BIN2_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN3_MED_LENGTH
// \brief length of the BIN3 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN3_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN3_MED_ADDRESS
// \brief address of the BIN3 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN3_MED_ADDRESS                  (I2C_OPC_BIN3_SAMPLE_ADDRESS + I2C_OPC_BIN3_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_BIN3_SIGMA_LENGTH
// \brief length of the BIN3 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN3_SIGMA_LENGTH                 (0x02)
//
// /*!
// \def I2C_OPC_BIN3_SIGMA_ADDRESS
// \brief address of the BIN3 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN3_SIGMA_ADDRESS                (I2C_OPC_BIN3_MED_ADDRESS + I2C_OPC_BIN3_MED_LENGTH)
// /*!
// \def I2C_OPC_BIN4_SAMPLE_LENGTH
// \brief length of the BIN4 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN4_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN4_SAMPLE_ADDRESS
// \brief address of the BIN4 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN4_SAMPLE_ADDRESS               (I2C_OPC_BIN3_SIGMA_ADDRESS + I2C_OPC_BIN3_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN4_MED_LENGTH
// \brief length of the BIN4 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN4_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN4_MED_ADDRESS
// \brief address of the BIN4 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN4_MED_ADDRESS                  (I2C_OPC_BIN4_SAMPLE_ADDRESS + I2C_OPC_BIN4_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_BIN4_SIGMA_LENGTH
// \brief length of the BIN4 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN4_SIGMA_LENGTH                 (0x02)
//
// /*!
// \def I2C_OPC_BIN4_SIGMA_ADDRESS
// \brief address of the BIN4 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN4_SIGMA_ADDRESS                (I2C_OPC_BIN4_MED_ADDRESS + I2C_OPC_BIN4_MED_LENGTH)
// /*!
// \def I2C_OPC_BIN5_SAMPLE_LENGTH
// \brief length of the BIN5 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN5_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN5_SAMPLE_ADDRESS
// \brief address of the BIN5 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN5_SAMPLE_ADDRESS               (I2C_OPC_BIN4_SIGMA_ADDRESS + I2C_OPC_BIN4_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN5_MED_LENGTH
// \brief length of the BIN5 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN5_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN5_MED_ADDRESS
// \brief address of the BIN5 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN5_MED_ADDRESS                  (I2C_OPC_BIN5_SAMPLE_ADDRESS + I2C_OPC_BIN5_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_BIN5_SIGMA_LENGTH
// \brief length of the BIN5 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN5_SIGMA_LENGTH                 (0x02)
//
// /*!
// \def I2C_OPC_BIN5_SIGMA_ADDRESS
// \brief address of the BIN5 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN5_SIGMA_ADDRESS                (I2C_OPC_BIN5_MED_ADDRESS + I2C_OPC_BIN5_MED_LENGTH)
// /*!
// \def I2C_OPC_BIN6_SAMPLE_LENGTH
// \brief length of the BIN6 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN6_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN6_SAMPLE_ADDRESS
// \brief address of the BIN6 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN6_SAMPLE_ADDRESS               (I2C_OPC_BIN5_SIGMA_ADDRESS + I2C_OPC_BIN5_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN6_MED_LENGTH
// \brief length of the BIN6 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN6_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN6_MED_ADDRESS
// \brief address of the BIN6 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN6_MED_ADDRESS                  (I2C_OPC_BIN6_SAMPLE_ADDRESS + I2C_OPC_BIN6_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_BIN6_SIGMA_LENGTH
// \brief length of the BIN6 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN6_SIGMA_LENGTH                 (0x02)
//
// /*!
// \def I2C_OPC_BIN6_SIGMA_ADDRESS
// \brief address of the BIN6 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN6_SIGMA_ADDRESS                (I2C_OPC_BIN6_MED_ADDRESS + I2C_OPC_BIN6_MED_LENGTH)
// /*!
// \def I2C_OPC_BIN7_SAMPLE_LENGTH
// \brief length of the BIN7 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN7_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN7_SAMPLE_ADDRESS
// \brief address of the BIN7 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN7_SAMPLE_ADDRESS               (I2C_OPC_BIN6_SIGMA_ADDRESS + I2C_OPC_BIN6_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN7_MED_LENGTH
// \brief length of the BIN7 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN7_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN7_MED_ADDRESS
// \brief address of the BIN7 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN7_MED_ADDRESS                  (I2C_OPC_BIN7_SAMPLE_ADDRESS + I2C_OPC_BIN7_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_BIN7_SIGMA_LENGTH
// \brief length of the BIN7 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN7_SIGMA_LENGTH                 (0x02)
//
// /*!
// \def I2C_OPC_BIN7_SIGMA_ADDRESS
// \brief address of the BIN7 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN7_SIGMA_ADDRESS                (I2C_OPC_BIN7_MED_ADDRESS + I2C_OPC_BIN7_MED_LENGTH)
// /*!
// \def I2C_OPC_BIN8_SAMPLE_LENGTH
// \brief length of the BIN8 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN8_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN8_SAMPLE_ADDRESS
// \brief address of the BIN8 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN8_SAMPLE_ADDRESS               (I2C_OPC_BIN7_SIGMA_ADDRESS + I2C_OPC_BIN7_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN8_MED_LENGTH
// \brief length of the BIN8 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN8_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN8_MED_ADDRESS
// \brief address of the BIN8 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN8_MED_ADDRESS                  (I2C_OPC_BIN8_SAMPLE_ADDRESS + I2C_OPC_BIN8_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_BIN8_SIGMA_LENGTH
// \brief length of the BIN8 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN8_SIGMA_LENGTH                 (0x02)
//
// /*!
// \def I2C_OPC_BIN8_SIGMA_ADDRESS
// \brief address of the BIN8 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN8_SIGMA_ADDRESS                (I2C_OPC_BIN8_MED_ADDRESS + I2C_OPC_BIN8_MED_LENGTH)
// /*!
// \def I2C_OPC_BIN9_SAMPLE_LENGTH
// \brief length of the BIN9 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN9_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN9_SAMPLE_ADDRESS
// \brief address of the BIN9 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN9_SAMPLE_ADDRESS               (I2C_OPC_BIN8_SIGMA_ADDRESS + I2C_OPC_BIN8_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN9_MED_LENGTH
// \brief length of the BIN9 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN9_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN9_MED_ADDRESS
// \brief address of the BIN9 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN9_MED_ADDRESS                  (I2C_OPC_BIN9_SAMPLE_ADDRESS + I2C_OPC_BIN9_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_BIN9_SIGMA_LENGTH
// \brief length of the BIN9 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN9_SIGMA_LENGTH                 (0x02)
//
// /*!
// \def I2C_OPC_BIN9_SIGMA_ADDRESS
// \brief address of the BIN9 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN9_SIGMA_ADDRESS                (I2C_OPC_BIN9_MED_ADDRESS + I2C_OPC_BIN9_MED_LENGTH)
// /*!
// \def I2C_OPC_BIN10_SAMPLE_LENGTH
// \brief length of the BIN10 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN10_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN10_SAMPLE_ADDRESS
// \brief address of the BIN10 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN10_SAMPLE_ADDRESS               (I2C_OPC_BIN9_SIGMA_ADDRESS + I2C_OPC_BIN9_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN10_MED_LENGTH
// \brief length of the BIN10 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN10_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN10_MED_ADDRESS
// \brief address of the BIN10 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN10_MED_ADDRESS                  (I2C_OPC_BIN10_SAMPLE_ADDRESS + I2C_OPC_BIN10_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_BIN10_SIGMA_LENGTH
// \brief length of the BIN10 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN10_SIGMA_LENGTH                 (0x02)
//
// /*!
// \def I2C_OPC_BIN10_SIGMA_ADDRESS
// \brief address of the BIN10 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN10_SIGMA_ADDRESS                (I2C_OPC_BIN10_MED_ADDRESS + I2C_OPC_BIN10_MED_LENGTH)
// /*!
// \def I2C_OPC_BIN11_SAMPLE_LENGTH
// \brief length of the BIN11 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN11_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN11_SAMPLE_ADDRESS
// \brief address of the BIN11 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN11_SAMPLE_ADDRESS               (I2C_OPC_BIN10_SIGMA_ADDRESS + I2C_OPC_BIN10_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN11_MED_LENGTH
// \brief length of the BIN11 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN11_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN11_MED_ADDRESS
// \brief address of the BIN11 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN11_MED_ADDRESS                  (I2C_OPC_BIN11_SAMPLE_ADDRESS + I2C_OPC_BIN11_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_BIN11_SIGMA_LENGTH
// \brief length of the BIN11 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN11_SIGMA_LENGTH                 (0x02)
//
// /*!
// \def I2C_OPC_BIN11_SIGMA_ADDRESS
// \brief address of the BIN11 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN11_SIGMA_ADDRESS                (I2C_OPC_BIN11_MED_ADDRESS + I2C_OPC_BIN11_MED_LENGTH)
// /*!
// \def I2C_OPC_BIN12_SAMPLE_LENGTH
// \brief length of the BIN12 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN12_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN12_SAMPLE_ADDRESS
// \brief address of the BIN12 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN12_SAMPLE_ADDRESS               (I2C_OPC_BIN11_SIGMA_ADDRESS + I2C_OPC_BIN11_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN12_MED_LENGTH
// \brief length of the BIN12 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN12_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN12_MED_ADDRESS
// \brief address of the BIN12 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN12_MED_ADDRESS                  (I2C_OPC_BIN12_SAMPLE_ADDRESS + I2C_OPC_BIN12_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_BIN12_SIGMA_LENGTH
// \brief length of the BIN12 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN12_SIGMA_LENGTH                 (0x02)
//
// /*!
// \def I2C_OPC_BIN12_SIGMA_ADDRESS
// \brief address of the BIN12 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN12_SIGMA_ADDRESS                (I2C_OPC_BIN12_MED_ADDRESS + I2C_OPC_BIN12_MED_LENGTH)
// /*!
// \def I2C_OPC_BIN13_SAMPLE_LENGTH
// \brief length of the BIN13 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN13_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN13_SAMPLE_ADDRESS
// \brief address of the BIN13 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN13_SAMPLE_ADDRESS               (I2C_OPC_BIN12_SIGMA_ADDRESS + I2C_OPC_BIN12_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN13_MED_LENGTH
// \brief length of the BIN13 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN13_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN13_MED_ADDRESS
// \brief address of the BIN13 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN13_MED_ADDRESS                  (I2C_OPC_BIN13_SAMPLE_ADDRESS + I2C_OPC_BIN13_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_BIN13_SIGMA_LENGTH
// \brief length of the BIN13 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN13_SIGMA_LENGTH                 (0x02)
//
// /*!
// \def I2C_OPC_BIN13_SIGMA_ADDRESS
// \brief address of the BIN13 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN13_SIGMA_ADDRESS                (I2C_OPC_BIN13_MED_ADDRESS + I2C_OPC_BIN13_MED_LENGTH)
// /*!
// \def I2C_OPC_BIN14_SAMPLE_LENGTH
// \brief length of the BIN14 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN14_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN14_SAMPLE_ADDRESS
// \brief address of the BIN14 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN14_SAMPLE_ADDRESS               (I2C_OPC_BIN13_SIGMA_ADDRESS + I2C_OPC_BIN13_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN14_MED_LENGTH
// \brief length of the BIN14 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN14_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN14_MED_ADDRESS
// \brief address of the BIN14 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN14_MED_ADDRESS                  (I2C_OPC_BIN14_SAMPLE_ADDRESS + I2C_OPC_BIN14_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_BIN14_SIGMA_LENGTH
// \brief length of the BIN14 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN14_SIGMA_LENGTH                 (0x02)
//
// /*!
// \def I2C_OPC_BIN14_SIGMA_ADDRESS
// \brief address of the BIN14 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN14_SIGMA_ADDRESS                (I2C_OPC_BIN14_MED_ADDRESS + I2C_OPC_BIN14_MED_LENGTH)
// /*!
// \def I2C_OPC_BIN15_SAMPLE_LENGTH
// \brief length of the BIN15 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN15_SAMPLE_LENGTH                (0x02)
//
// /*!
// \def I2C_OPC_BIN15_SAMPLE_ADDRESS
// \brief address of the BIN15 sample variable for i2c-opc module.
// */
// #define I2C_OPC_BIN15_SAMPLE_ADDRESS               (I2C_OPC_BIN14_SIGMA_ADDRESS + I2C_OPC_BIN14_SIGMA_LENGTH)
//
// /*!
// \def I2C_OPC_BIN15_MED_LENGTH
// \brief length of the BIN15 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN15_MED_LENGTH                   (0x02)
//
// /*!
// \def I2C_OPC_BIN15_MED_ADDRESS
// \brief address of the BIN15 average variable for i2c-opc module.
// */
// #define I2C_OPC_BIN15_MED_ADDRESS                  (I2C_OPC_BIN15_SAMPLE_ADDRESS + I2C_OPC_BIN15_SAMPLE_LENGTH)
//
// /*!
// \def I2C_OPC_BIN15_SIGMA_LENGTH
// \brief length of the BIN15 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN15_SIGMA_LENGTH                 (0x02)
//
// /*!
// \def I2C_OPC_BIN15_SIGMA_ADDRESS
// \brief address of the BIN15 sigma variable for i2c-opc module.
// */
// #define I2C_OPC_BIN15_SIGMA_ADDRESS                (I2C_OPC_BIN15_MED_ADDRESS + I2C_OPC_BIN15_MED_LENGTH)
//
// /*!
// \def I2C_OPC_READABLE_DATA_LENGTH
// \brief length of the readable variables for i2c-opc module. Need to be update with with last 2 define!!!
// */
// #define I2C_OPC_READABLE_DATA_LENGTH             (I2C_OPC_BIN15_SIGMA_ADDRESS + I2C_OPC_BIN15_SIGMA_LENGTH - I2C_READ_REGISTER_START_ADDRESS)

/*********************************************************************
* Writable registers: Specifying the length in bytes of the data by I2C_{MODULE_NAME}_{DATA_NAME}_LENGTH, the corresponding address is calculated automatically
*********************************************************************/
/*!
\def I2C_OPC_ADDRESS_LENGTH
\brief length of the address variable for i2c-opc module.
*/
#define I2C_OPC_ADDRESS_LENGTH                   (0x01)

/*!
\def I2C_OPC_ADDRESS_ADDRESS
\brief address of the address variable for i2c-opc module.
*/
#define I2C_OPC_ADDRESS_ADDRESS                  (I2C_WRITE_REGISTER_START_ADDRESS)

/*!
\def I2C_OPC_ONESHOT_LENGTH
\brief length of the oneshot variable for i2c-opc module.
*/
#define I2C_OPC_ONESHOT_LENGTH                   (0x01)

/*!
\def I2C_OPC_ONESHOT_ADDRESS
\brief address of the oneshot variable for i2c-opc module.
*/
#define I2C_OPC_ONESHOT_ADDRESS                  (I2C_OPC_ADDRESS_ADDRESS + I2C_OPC_ADDRESS_LENGTH)

/*!
\def I2C_OPC_CONTINUOUS_LENGTH
\brief length of the continuous variable for i2c-opc module.
*/
#define I2C_OPC_CONTINUOUS_LENGTH                (0x01)

/*!
\def I2C_OPC_CONTINUOUS_ADDRESS
\brief address of the continuous variable for i2c-opc module.
*/
#define I2C_OPC_CONTINUOUS_ADDRESS               (I2C_OPC_ONESHOT_ADDRESS + I2C_OPC_ONESHOT_LENGTH)

/*!
\def I2C_OPC_WRITABLE_DATA_LENGTH
\brief length of the writable variables for i2c-opc module.
*/
#define I2C_OPC_WRITABLE_DATA_LENGTH             (I2C_OPC_CONTINUOUS_ADDRESS + I2C_OPC_CONTINUOUS_LENGTH - I2C_WRITE_REGISTER_START_ADDRESS)

// Readable registers errors checking
#if I2C_OPC_READ_REGISTERS_LENGTH > I2C_READ_REGISTER_END_ADDRESS
#error "ERROR! Too many readable registers found in OPC module!!!"
#endif

// Writeable registers errors checking
#if I2C_OPC_WRITE_REGISTERS_LENGTH > I2C_WRITE_REGISTER_END_ADDRESS
#error "ERROR! Too many writable registers found in OPC module!!!"
#endif

#endif
