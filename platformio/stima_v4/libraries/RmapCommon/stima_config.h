/**@file stima_config.h */

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
\def STIMA_MODULE_NAME_MASTER_ETH
\brief The module'name for sending report over ethernet.
*/
#define STIMA_MODULE_NAME_MASTER_ETH                ("master-eth")

/*!
\def STIMA_MODULE_NAME_MASTER_GSM
\brief The module'name for sending report over gsm/gors.
*/
#define STIMA_MODULE_NAME_MASTER_GSM                ("master-gsm")

/*!
\def STIMA_MODULE_NAME_RAIN
\brief The module'name for acquiring rain tips.
*/
#define STIMA_MODULE_NAME_RAIN                      ("i2c-rain")

/*!
\def STIMA_MODULE_NAME_TH
\brief The module'name for acquiring temperature and humidity.
*/
#define STIMA_MODULE_NAME_TH                        ("i2c-th")

/*!
\def STIMA_MODULE_NAME_THR
\brief The module'name for acquiring temperature, humidity and rain.
*/
#define STIMA_MODULE_NAME_THR                       ("i2c-thr")

/*!
\def STIMA_MODULE_NAME_OPC
\brief The module'name for acquiring air particle.
*/
#define STIMA_MODULE_NAME_OPC                       ("i2c-opc")

/*!
\def STIMA_MODULE_NAME_OPC
\brief The module'name for acquiring air gas (NO2, CO2).
*/
#define STIMA_MODULE_NAME_GAS                       ("i2c-gas")

/*!
\def STIMA_MODULE_NAME_LEAF
\brief The module'name for acquiring leaf wetness.
*/
#define STIMA_MODULE_NAME_LEAF                      ("i2c-leaf")

/*!
\def STIMA_MODULE_NAME_WIND
\brief The module'name for acquiring wind sensor.
*/
#define STIMA_MODULE_NAME_WIND                      ("i2c-wind")

/*!
\def STIMA_MODULE_NAME_WIND
\brief The module'name for acquiring radiation sensor.
*/
#define STIMA_MODULE_NAME_SOLAR_RADIATION           ("i2c-radiation")

/*!
\def STIMA_MODULE_NAME_POWER_MPPT
\brief The module'name for acquiring power regulator mppt.
*/
#define STIMA_MODULE_NAME_POWER_MPPT                ("i2c-powermppt")

#define DATA_LEVEL_SAMPLE  ("sample")
#define DATA_LEVEL_REPORT  ("report")
#define DATA_LEVEL_MAINT   ("maint")
#define DATA_LEVEL_RPC     ("rpc")

#define NETWORK_FIXED      ("fixed")
#define NETWORK_MOBILE     ("mobile")
#define NETWORK_TEST       ("test")

#define STIMA_MODULE_NAME_LENGTH                   (20)

#endif
