/**@file sensors_config.h */

/*********************************************************************
<h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>
Paolo patruno <p.patruno@iperbole.bologna.it>

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

#ifndef _SENSORS_CONFIG_H
#define _SENSORS_CONFIG_H

/// @brief Enable redundant sensor
#define USE_REDUNDANT_SENSOR        (false)

/// @brief Enable if you want use json library for json response (getJson function in SensorDriver).
#define USE_JSON                    (false)

/// @brief Enable D Template
#define USE_D_TEMPLATE              (false)

/// @brief Enable if you want use one module for TH and RAIN. Disable if you want use one module for TH and one for RAIN.
#define USE_THR                     (false)

/// @brief Enable if you want use ADT7420 sensor.
#define USE_SENSOR_ADT              (false)

/// @brief Enable if you want use HIH6100 sensor.
#define USE_SENSOR_HIH              (false)

/// @brief Enable if you want use HYT939, HYT271 or HYT221 sensor.
#define USE_SENSOR_HYT              (false)

/// @brief Enable if you want use SHT35 sensor.
#define USE_SENSOR_SHT              (false)

/// @brief Enable if you want use BMP280 sensor.s
#define USE_SENSOR_B28              (false)

/// @brief Enable if you want use DigitEco Power sensor.
#define USE_SENSOR_DEP              (false)

/// @brief Enable if you want use DigitEco Wind Speed sensor.
#define USE_SENSOR_DES              (false)

/// @brief Enable if you want use DigitEco Wind Direction sensor.
#define USE_SENSOR_DED              (false)

/// @brief Enable if you want use Gill Windsonic sensor.
#define USE_SENSOR_GWS              (false)

/// @brief Enable if you want use DigitEco Global Solar Radiation sensor.
#define USE_SENSOR_DSR              (false)

/// @brief Enable if you want use 0-5V High Resolution Global Solar Radiation sensor.
#define USE_SENSOR_VSR              (false)

/// @brief Enable if you want average Global Solar Radiation sensor.
#define USE_SENSOR_DSA              (false)

/// @brief Enable if you want average Level sensor.
#define USE_SENSOR_LVM              (false)

/// @brief Enable if you want vectorial average Wind Speed and Direction over 10'.
#define USE_SENSOR_DWA              (false)

/// @brief Enable if you want vectorial average Wind Speed and Direction over report time.
#define USE_SENSOR_DWB              (false)

/// @brief Enable if you want gust Wind Speed and Direction over report time.
#define USE_SENSOR_DWC              (false)

/// @brief Enable if you want average Wind Speed over report time.
#define USE_SENSOR_DWD              (false)

/// @brief Enable if you want class Wind Speed over report time.
#define USE_SENSOR_DWE              (false)

/// @brief Enable if you want class Wind Speed over report time.
#define USE_SENSOR_DWF              (false)

/// @brief Enable if you want use OPC PM1, PM2.5, PM10 continuous average value.
#define USE_SENSOR_OA2              (false)
#define USE_SENSOR_OA3              (false)

/// @brief Enable if you want use OPC PM1, PM2.5, PM10 continuous standard deviation value.
#define USE_SENSOR_OB2              (false)
#define USE_SENSOR_OB3              (false)

/// @brief Enable if you want use OPC BINS continuous average value.
#define USE_SENSOR_OCX_ODX_FULL_BIN (false)
#define USE_SENSOR_OC2              (false)
#define USE_SENSOR_OC3              (false)

/// @brief Enable if you want use OPC BINS continuous standard deviation value.
#define USE_SENSOR_OD2              (false)
#define USE_SENSOR_OD3              (false)

/// @brief Enable if you want use temperature, humidity or pressure average value.
#define USE_SENSOR_OE3              (false)

/// @brief Enable Gas sensor
#define USE_SENSOR_GAS              (false)

/// @brief Enable if you want use CO2 gas sensor.
#define USE_SENSOR_CO2              (false)

/// @brief Enable if you want use NO2 gas sensor.
#define USE_SENSOR_NO2              (false)

/// @brief Enable if you want use O3 gas sensor.
#define USE_SENSOR_O3               (false)

/// @brief Enable if you want use CO gas sensor.
#define USE_SENSOR_CO               (false)

/// @brief Enable if you want use EX gas sensor.
#define USE_SENSOR_EX               (false)

/// @brief Enable if you want use leaf wetness time continuous value.
#define USE_SENSOR_LWT              (false)

/// @brief Enable if you want use SI7021 sensor.
#define USE_SENSOR_HI7              (false)

/// @brief Enable if you want use Bmp085 sensor.
#define USE_SENSOR_BMP              (false)

/// @brief Enable if you want use DW1 sensor.
#define USE_SENSOR_DW1              (false)

/// @brief Enable if you want use Tipping bucket rain gauge sensor.
#define USE_SENSOR_TBS              (false)

/// @brief Enable if you want use Tipping bucket rain gauge sensor.
#define USE_SENSOR_TBR              (false)

/// @brief Enable if you want use Temperature and humidity oneshot sensor.
#define USE_SENSOR_STH              (false)

/// @brief Enable if you want use Temperature and humidity continuous istantaneous sensor.
#define USE_SENSOR_ITH              (false)

/// @brief Enable if you want use Temperature and humidity continuous minium sensor.
#define USE_SENSOR_NTH              (false)

/// @brief Enable if you want use Temperature and humidity continuous average sensor.
#define USE_SENSOR_MTH              (false)

/// @brief Enable if you want use Temperature and humidity continuous maximum sensor.
#define USE_SENSOR_XTH              (false)

/// @brief Enable if you want use SSD011 oneshot sensor.
#define USE_SENSOR_SSD              (false)

/// @brief Enable if you want use SSD011 report istantaneous sensor.
#define USE_SENSOR_ISD              (false)

/// @brief Enable if you want use SSD011 report minium sensor.
#define USE_SENSOR_NSD              (false)

/// @brief Enable if you want use SSD011 report average sensor.
#define USE_SENSOR_MSD              (false)

/// @brief Enable if you want use SSD011 report maximum sensor.
#define USE_SENSOR_XSD              (false)

/// @brief Enable if you want use MICS4514 oneshot sensor.
#define USE_SENSOR_SMI              (false)

/// @brief Enable if you want use MICS4514 report istantaneous sensor.
#define USE_SENSOR_IMI              (false)

/// @brief Enable if you want use MICS4514 report minium sensor.
#define USE_SENSOR_NMI              (false)

/// @brief Enable if you want use MICS4514 report average sensor.
#define USE_SENSOR_MMI              (false)

/// @brief Enable if you want use MICS4514 report maximum sensor.
#define USE_SENSOR_XMI              (false)

/// @brief Enable if you want use Radio RF24 sensor.
#define USE_SENSOR_RF24             (false)

/// @brief How much mm of rain for one tip of tipping bucket rain gauge.
#define RAIN_FOR_TIP                (1)

// OPC
#define VALUES_TO_READ_FROM_SENSOR_COUNT      (2)
#define JSONS_TO_READ_FROM_SENSOR_COUNT       (2)

// OPC
// #define VALUES_TO_READ_FROM_SENSOR_COUNT      (24)
// #define JSONS_TO_READ_FROM_SENSOR_COUNT       (1)

#define USE_TH_SENSORS                        (USE_SENSOR_ADT + USE_SENSOR_HIH + USE_SENSOR_HYT + USE_SENSOR_SHT + USE_SENSOR_STH + USE_SENSOR_ITH + USE_SENSOR_MTH + USE_SENSOR_NTH + USE_SENSOR_XTH)
#define USE_RAIN_SENSORS                      (USE_SENSOR_TBR + USE_SENSOR_TBS)
#define USE_RADIATION_SENSORS                (USE_SENSOR_DSR + USE_SENSOR_VSR + USE_SENSOR_DSA)
#define USE_WIND_SENSORS                      (USE_SENSOR_DWA + USE_SENSOR_DWB + USE_SENSOR_DWC + USE_SENSOR_DWD + USE_SENSOR_DWE + USE_SENSOR_DWF)
#define USE_POWER_MPPT_SENSORS                (USE_SENSOR_DEP)
#define USE_LEVEL_SENSOR                      (USE_SENSOR_LVM)

#if (USE_TH_SENSORS && (USE_RAIN_SENSORS == 0))
#define USE_MODULE_TH                         (true)
#elif ((USE_TH_SENSORS == 0) && USE_RAIN_SENSORS)
#define USE_MODULE_RAIN                       (true)
#elif (USE_TH_SENSORS && USE_RAIN_SENSORS && USE_THR)
#define USE_MODULE_THR                        (true)
#elif (USE_RADIATION_SENSORS)
#define USE_MODULE_SOLAR_RADIATION            (true)
#elif (USE_WIND_SENSORS)
#define USE_MODULE_WIND                       (true)
#elif (USE_POWER_MPPT_SENSORS)
#define USE_MODULE_POWER_MPPT                 (true)
#elif (USE_LEVEL_SENSOR)
#define USE_MODULE_LEVEL                      (true)
#endif

/// @brief Max count of all used sensor.
#define SENSORS_COUNT_MAX                     (USE_SENSOR_ADT + USE_SENSOR_HIH + USE_SENSOR_HYT + USE_SENSOR_SHT + USE_REDUNDANT_SENSOR)

/// @brief Max number of sensor.
#define SENSORS_MAX                           (SENSORS_COUNT_MAX)

/// @brief Max number of unique sensor.
/// unique sensors are sensors that can have more driver but only one i2c address and only one setup and prepare
#define SENSORS_UNIQUE_MAX                    (1)


/// @brief Max number of sensor remote CAN max remote boards present.
/// sensor are remote sensor CAN UAVCANfor present on a single board
#define CAN_SENSOR_COUNT_MAX                  (6)


/// @brief Max number of unique boards present.
/// boards are module CAN UAVCAN that have sensor on board
#define BOARDS_COUNT_MAX                      (8)

#endif
