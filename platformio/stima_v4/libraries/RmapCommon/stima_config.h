/**@file stima_config.h */

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

#ifndef _STIMA_CONFIG_H
#define _STIMA_CONFIG_H

/*!
\def CONFIGURATION_CURRENT
\brief Indicate current configuration.
*/
#define CONFIGURATION_CURRENT                       (0)

/*!
\def CONFIGURATION_DEFAULT
\brief Indicate default configuration.
*/
#define CONFIGURATION_DEFAULT                       (1)

/*!
\def STIMA_MODULE_TYPE_MAX_AVAIABLE
\brief This define MAX number of module into STIMA.
*/
#define STIMA_MODULE_TYPE_MAX_AVAIABLE              (12)

/*!
\def STIMA_MODULE_TYPE_UNDEFINED
\brief Module type stima not defined.
*/
#define STIMA_MODULE_TYPE_UNDEFINED                 (0)

/*!
\def STIMA_MODULE_TYPE_MASTER_ETH
\brief This module send report over ethernet.
*/
#define STIMA_MODULE_TYPE_MASTER_ETH                (10)

/*!
\def STIMA_MODULE_TYPE_MASTER_GSM
\brief This module send sample over gsm/gprs.
*/
#define STIMA_MODULE_TYPE_MASTER_GSM                (11)

/*!
\def STIMA_MODULE_TYPE_RAIN
\brief This module acquire rain tips.
*/
#define STIMA_MODULE_TYPE_RAIN                      (20)

/*!
\def STIMA_MODULE_TYPE_TH
\brief This module acquire temperature and humidity.
*/
#define STIMA_MODULE_TYPE_TH                        (21)

/*!
\def STIMA_MODULE_TYPE_THR
\brief This module acquire temperature, humidity and rain.
*/
#define STIMA_MODULE_TYPE_THR                       (22)

/*!
\def STIMA_MODULE_TYPE_OPC
\brief This module acquire air particle.
*/
#define STIMA_MODULE_TYPE_OPC                       (23)

/*!
\def STIMA_MODULE_TYPE_LEAF
\brief This module acquire leaf wetness.
*/
#define STIMA_MODULE_TYPE_LEAF                      (24)

/*!
\def STIMA_MODULE_TYPE_WIND
\brief This module acquire wind sensor.
*/
#define STIMA_MODULE_TYPE_WIND                      (25)

/*!
\def STIMA_MODULE_TYPE_SOLAR_RADIATION
\brief This module acquire radiation sensor.
*/
#define STIMA_MODULE_TYPE_SOLAR_RADIATION           (26)

/*!
\def STIMA_MODULE_TYPE_GAS
\brief This module acquire gas (NO2, CO2).
*/
#define STIMA_MODULE_TYPE_GAS                       (27)

/*!
\def STIMA_MODULE_TYPE_POWER_MPPT
\brief This module acquire power regulator mppt.
*/
#define STIMA_MODULE_TYPE_POWER_MPPT                (28)

/*!
\def STIMA_MODULE_TYPE_VWC
\brief This module acquire soil sensor.
*/
#define STIMA_MODULE_TYPE_VWC                       (29)

/*!
\def STIMA_MODULE_OFFSET_IDENT_V4
\brief The module'name offset ident to remove "stimav4." from complete header
*/
#define STIMA_MODULE_OFFSET_IDENT_V4                (7)

/*!
\def STIMA_MODULE_NAME_MASTER_ETH
\brief The module'name for sending report over ethernet.
*/
#define STIMA_MODULE_NAME_MASTER_ETH                ("stima4.module_master_eth")
#define STIMA_MODULE_DESCRIPTION_MASTER_ETH         ("Master module 4G, CAN Cyphal V1.0")

/*!
\def STIMA_MODULE_NAME_MASTER_GSM
\brief The module'name for sending report over gsm/gors.
*/
#define STIMA_MODULE_NAME_MASTER_GSM                ("stima4.module_master_gsm")
#define STIMA_MODULE_DESCRIPTION_MASTER_GSM         ("Master module ETH, CAN Cyphal V1.0")

/*!
\def STIMA_MODULE_NAME_RAIN
\brief The module'name for acquiring rain tips.
*/
#define STIMA_MODULE_NAME_RAIN                      ("stima4.module_rain")
#define STIMA_MODULE_DESCRIPTION_RAIN               ("Slave module rain, CAN Cyphal V1.0")


/*!
\def STIMA_MODULE_NAME_TH
\brief The module'name for acquiring temperature and humidity.
*/
#define STIMA_MODULE_NAME_TH                        ("stima4.module_th")
#define STIMA_MODULE_DESCRIPTION_TH                 ("Slave module th, CAN Cyphal V1.0")


/*!
\def STIMA_MODULE_NAME_THR
\brief The module'name for acquiring temperature, humidity and rain.
*/
#define STIMA_MODULE_NAME_THR                       ("stima4.module_thr")
#define STIMA_MODULE_DESCRIPTION_THR                ("Slave module thr, CAN Cyphal V1.0")


/*!
\def STIMA_MODULE_NAME_OPC
\brief The module'name for acquiring air particle.
*/
#define STIMA_MODULE_NAME_OPC                       ("stima4.module_opc")
#define STIMA_MODULE_DESCRIPTION_OPC                ("Slave module opc, CAN Cyphal V1.0")


/*!
\def STIMA_MODULE_NAME_GAS
\brief The module'name for acquiring air gas (NO2, CO2).
*/
#define STIMA_MODULE_NAME_GAS                       ("stima4.module_gas")
#define STIMA_MODULE_DESCRIPTION_GAS                ("Slave module gas, CAN Cyphal V1.0")


/*!
\def STIMA_MODULE_NAME_LEAF
\brief The module'name for acquiring leaf wetness.
*/
#define STIMA_MODULE_NAME_LEAF                      ("stima4.module_leaf")
#define STIMA_MODULE_DESCRIPTION_LEAF               ("Slave module leaf, CAN Cyphal V1.0")


/*!
\def STIMA_MODULE_NAME_WIND
\brief The module'name for acquiring wind sensor.
*/
#define STIMA_MODULE_NAME_WIND                      ("stima4.module_wind")
#define STIMA_MODULE_DESCRIPTION_WIND               ("Slave module wind, CAN Cyphal V1.0")


/*!
\def STIMA_MODULE_NAME_SOLAR_RADIATION
\brief The module'name for acquiring radiation sensor.
*/
#define STIMA_MODULE_NAME_SOLAR_RADIATION           ("stima4.module_radiation")
#define STIMA_MODULE_DESCRIPTION_SOLAR_RADIATION    ("Slave module radiation, CAN Cyphal V1.0")


/*!
\def STIMA_MODULE_NAME_POWER_MPPT
\brief The module'name for acquiring power regulator mppt.
*/
#define STIMA_MODULE_NAME_POWER_MPPT                ("stima4.module_powermppt")
#define STIMA_MODULE_DESCRIPTION_POWER_MPPT         ("Slave module power mppt, CAN Cyphal V1.0")


/*!
\def STIMA_MODULE_NAME_VWC
\brief The module'name for acquiring VWC.
*/
#define STIMA_MODULE_NAME_VWC                       ("stima4.module_vwc")
#define STIMA_MODULE_DESCRIPTION_VWC                ("Slave module vwc, CAN Cyphal V1.0")

/*!
\brief INTEST RPC Sensor identification type list
*/
#define STIMA_RPC_SENSOR_NAME_TBR                   ("TBR")
#define STIMA_RPC_SENSOR_NAME_TPR                   ("TPR")
#define STIMA_RPC_SENSOR_NAME_STH                   ("STH")
#define STIMA_RPC_SENSOR_NAME_ITH                   ("ITH")
#define STIMA_RPC_SENSOR_NAME_MTH                   ("MTH")
#define STIMA_RPC_SENSOR_NAME_NTH                   ("NTH")
#define STIMA_RPC_SENSOR_NAME_XTH                   ("XTH")
#define STIMA_RPC_SENSOR_NAME_DWA                   ("DWA")
#define STIMA_RPC_SENSOR_NAME_DWB                   ("DWB")
#define STIMA_RPC_SENSOR_NAME_DWC                   ("DWC")
#define STIMA_RPC_SENSOR_NAME_DWD                   ("DWD")
#define STIMA_RPC_SENSOR_NAME_DWE                   ("DWE")
#define STIMA_RPC_SENSOR_NAME_DWF                   ("DWF")
#define STIMA_RPC_SENSOR_NAME_MPP                   ("MPP")
#define STIMA_RPC_SENSOR_NAME_DSA                   ("DSA")
#define STIMA_RPC_SENSOR_NAME_VWC                   ("SVW")

/*!
\brief LCD descriptions list
*/
#define STIMA_LCD_DESCRIPTION_GAS                   ("Air gas")
#define STIMA_LCD_DESCRIPTION_HUMIDITY              ("Humidity")
#define STIMA_LCD_DESCRIPTION_LEAF                  ("Leaf wetness")
#define STIMA_LCD_DESCRIPTION_OPC                   ("Air particle")
#define STIMA_LCD_DESCRIPTION_POWER_CHARGE          ("Battery charge")
#define STIMA_LCD_DESCRIPTION_POWER_INPUT           ("Input voltage")
#define STIMA_LCD_DESCRIPTION_POWER_CURRENT         ("Battery current")
#define STIMA_LCD_DESCRIPTION_RAIN                  ("Rain")
#define STIMA_LCD_DESCRIPTION_SOLAR_RADIATION       ("Solar radiation")
#define STIMA_LCD_DESCRIPTION_TEMPERATURE           ("Temperature")
#define STIMA_LCD_DESCRIPTION_VWC1                  ("Soil mosture 1")
#define STIMA_LCD_DESCRIPTION_VWC2                  ("Soil mosture 2")
#define STIMA_LCD_DESCRIPTION_VWC3                  ("Soil mosture 3")
#define STIMA_LCD_DESCRIPTION_WIND_DIRECTION        ("Wind direction")
#define STIMA_LCD_DESCRIPTION_WIND_SPEED            ("Wind speed")

/*!
\brief LCD Measure units list
*/
#define STIMA_LCD_UNIT_TYPE_CELSIUS_DEGREES         ("°C")
#define STIMA_LCD_UNIT_TYPE_DEGREES                 ("°")
#define STIMA_LCD_UNIT_TYPE_METERS                  ("m")
#define STIMA_LCD_UNIT_TYPE_METERS_PER_SECOND       ("m/s")
#define STIMA_LCD_UNIT_TYPE_MILLIMETERS             ("mm")
#define STIMA_LCD_UNIT_TYPE_PERCENTS                ("%")
#define STIMA_LCD_UNIT_TYPE_VOLTS                   ("V")
#define STIMA_LCD_UNIT_TYPE_MILLI_AMPERE            ("mA")
#define STIMA_LCD_UNIT_TYPE_WATTS_PER_SQUARE_METER  ("W/m2")

/*!
\brief LCD Decimals units list
*/
#define STIMA_LCD_DECIMALS_ZERO  0
#define STIMA_LCD_DECIMALS_ONE   1
#define STIMA_LCD_DECIMALS_TWO   2

// AVAILABLE LEVEL
#define DATA_LEVEL_SAMPLE  ("sample")
#define DATA_LEVEL_REPORT  ("report")
#define DATA_LEVEL_MAINT   ("maint")
#define DATA_LEVEL_RPC     ("rpc")

// AVAILABLE NETWORK
#define NETWORK_FIXED      ("fixed")
#define NETWORK_MOBILE     ("mobile")

/*!
\brief Max length of strings used to take the information
*/
#define STIMA_LCD_DESCRIPTION_LENGTH               (20)
#define STIMA_LCD_MEASURE_LENGTH                   (20)
#define STIMA_LCD_UNIT_TYPE_LENGTH                 (10)
#define STIMA_MODULE_DESCRIPTION_LENGTH            (50)
#define STIMA_MODULE_NAME_LENGTH                   (30)

#endif
