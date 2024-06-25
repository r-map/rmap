/**@file sensors_config.h */

/*********************************************************************
<h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
authors:
Marco Baldinetti <marco.baldinetti@digiteco.it>
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

/*!
\def USE_REDUNDANT_SENSOR
\brief Enable if you want use redundant sensor. Redundant sensor MUST be equal to main sensor.
*/
#define USE_REDUNDANT_SENSOR        (false)

/*!
\def USE_JSON
\brief Enable if you want use json library for json response (getJson function in SensorDriver).
*/
#define USE_JSON                    (false)

#define USE_D_TEMPLATE              (false)

/*!
\def USE_THR
\brief Enable if you want use one module for TH and RAIN. Disable if you want use one module for TH and one for RAIN.
*/
#define USE_THR                     (false)

/*!
\def USE_SENSOR_ADT
\brief Enable if you want use ADT7420 sensor.
*/
#define USE_SENSOR_ADT              (false)

/*!
\def USE_SENSOR_HIH
\brief Enable if you want use HIH6100 sensor.
*/
#define USE_SENSOR_HIH              (false)

/*!
\def USE_SENSOR_HYT
\brief Enable if you want use HYT939, HYT271 or HYT221 sensor.
*/
#define USE_SENSOR_HYT              (false)

/*!
\def USE_SENSOR_SHT
\brief Enable if you want use SHT35 sensor.
*/
#define USE_SENSOR_SHT              (false)

/*!
\def USE_SENSOR_B28
\brief Enable if you want use BMP280 sensor.s
*/
#define USE_SENSOR_B28              (false)

/*!
\def USE_SENSOR_DEP
\brief Enable if you want use DigitEco Power sensor.
*/
#define USE_SENSOR_DEP              (false)

/*!
\def USE_SENSOR_DES
\brief Enable if you want use DigitEco Wind Speed sensor.
*/
#define USE_SENSOR_DES              (false)

/*!
\def USE_SENSOR_DED
\brief Enable if you want use DigitEco Wind Direction sensor.
*/
#define USE_SENSOR_DED              (false)

/*!
\def USE_SENSOR_GWS
\brief Enable if you want use Gill Windsonic sensor.
*/
#define USE_SENSOR_GWS              (false)

/*!
\def USE_SENSOR_DSR
\brief Enable if you want use DigitEco Global Solar Radiation sensor.
*/
#define USE_SENSOR_DSR              (false)

/*!
\def USE_SENSOR_VSR
\brief Enable if you want use 0-5V High Resolution Global Solar Radiation sensor.
*/
#define USE_SENSOR_VSR              (false)

/*!
\def USE_SENSOR_DSA
\brief Enable if you want average Global Solar Radiation sensor.
*/
#define USE_SENSOR_DSA              (false)

/*!
\def USE_SENSOR_DWA
\brief Enable if you want vectorial average Wind Speed and Direction over 10'.
*/
#define USE_SENSOR_DWA              (false)

/*!
\def USE_SENSOR_DWB
\brief Enable if you want vectorial average Wind Speed and Direction over report time.
*/
#define USE_SENSOR_DWB              (false)

/*!
\def USE_SENSOR_DWC
\brief Enable if you want gust Wind Speed and Direction over report time.
*/
#define USE_SENSOR_DWC              (false)

/*!
\def USE_SENSOR_DWD
\brief Enable if you want average Wind Speed over report time.
*/
#define USE_SENSOR_DWD              (false)

/*!
\def USE_SENSOR_DWE
\brief Enable if you want class Wind Speed over report time.
*/
#define USE_SENSOR_DWE              (false)

/*!
\def USE_SENSOR_DWF
\brief Enable if you want class Wind Speed over report time.
*/
#define USE_SENSOR_DWF              (false)

/*!
\def USE_SENSOR_OA2
\brief Enable if you want use OPC PM1, PM2.5, PM10 continuous average value.
*/
#define USE_SENSOR_OA2              (false)
#define USE_SENSOR_OA3              (false)

/*!
\def USE_SENSOR_OB2
\brief Enable if you want use OPC PM1, PM2.5, PM10 continuous standard deviation value.
*/
#define USE_SENSOR_OB2              (false)
#define USE_SENSOR_OB3              (false)

/*!
\def USE_SENSOR_OC3
\brief Enable if you want use OPC BINS continuous average value.
*/
#define USE_SENSOR_OCX_ODX_FULL_BIN (false)
#define USE_SENSOR_OC2              (false)
#define USE_SENSOR_OC3              (false)

/*!
\def USE_SENSOR_OD3
\brief Enable if you want use OPC BINS continuous standard deviation value.
*/
#define USE_SENSOR_OD2              (false)
#define USE_SENSOR_OD3              (false)

/*!
\def USE_SENSOR_OE3
\brief Enable if you want use temperature, humidity or pressure average value.
*/
#define USE_SENSOR_OE3              (false)

#define USE_SENSOR_GAS              (false)
/*!
\def USE_SENSOR_CO2
\brief Enable if you want use CO2 gas sensor.
*/
#define USE_SENSOR_CO2              (false)

/*!
\def USE_SENSOR_NO2
\brief Enable if you want use NO2 gas sensor.
*/
#define USE_SENSOR_NO2              (false)

/*!
\def USE_SENSOR_O3
\brief Enable if you want use O3 gas sensor.
*/
#define USE_SENSOR_O3               (false)

/*!
\def USE_SENSOR_CO
\brief Enable if you want use CO gas sensor.
*/
#define USE_SENSOR_CO               (false)

/*!
\def USE_SENSOR_EX
\brief Enable if you want use EX gas sensor.
*/
#define USE_SENSOR_EX               (false)

/*!
\def USE_SENSOR_LWT
\brief Enable if you want use leaf wetness time continuous value.
*/
#define USE_SENSOR_LWT              (false)

/*!
\def USE_SENSOR_HI7
\brief Enable if you want use SI7021 sensor.
*/
#define USE_SENSOR_HI7              (false)

/*!
\def USE_SENSOR_BMP
\brief Enable if you want use Bmp085 sensor.
*/
#define USE_SENSOR_BMP              (false)

/*!
\def USE_SENSOR_DW1
\brief Enable if you want use DW1 sensor.
*/
#define USE_SENSOR_DW1              (false)

/*!
\def USE_SENSOR_TBS
\brief Enable if you want use Tipping bucket rain gauge sensor.
*/
#define USE_SENSOR_TBS              (false)

/*!
\def USE_SENSOR_TBR
\brief Enable if you want use Tipping bucket rain gauge sensor.
*/
#define USE_SENSOR_TBR              (true)

/*!
\def USE_SENSOR_STH
\brief Enable if you want use Temperature and humidity oneshot sensor.
*/
#define USE_SENSOR_STH              (false)

/*!
\def USE_SENSOR_ITH
\brief Enable if you want use Temperature and humidity continuous istantaneous sensor.
*/
#define USE_SENSOR_ITH              (false)

/*!
\def USE_SENSOR_NTH
\brief Enable if you want use Temperature and humidity continuous minium sensor.
*/
#define USE_SENSOR_NTH              (false)

/*!
\def USE_SENSOR_MTH
\brief Enable if you want use Temperature and humidity continuous average sensor.
*/
#define USE_SENSOR_MTH              (false)

/*!
\def USE_SENSOR_XTH
\brief Enable if you want use Temperature and humidity continuous maximum sensor.
*/
#define USE_SENSOR_XTH              (false)

/*!
\def USE_SENSOR_SSD
\brief Enable if you want use SSD011 oneshot sensor.
*/
#define USE_SENSOR_SSD              (false)

/*!
\def USE_SENSOR_ISD
\brief Enable if you want use SSD011 report istantaneous sensor.
*/
#define USE_SENSOR_ISD              (false)

/*!
\def USE_SENSOR_NSD
\brief Enable if you want use SSD011 report minium sensor.
*/
#define USE_SENSOR_NSD              (false)

/*!
\def USE_SENSOR_MSD
\brief Enable if you want use SSD011 report average sensor.
*/
#define USE_SENSOR_MSD              (false)

/*!
\def USE_SENSOR_XSD
\brief Enable if you want use SSD011 report maximum sensor.
*/
#define USE_SENSOR_XSD              (false)

/*!
\def USE_SENSOR_SMI
\brief Enable if you want use MICS4514 oneshot sensor.
*/
#define USE_SENSOR_SMI              (false)

/*!
\def USE_SENSOR_IMI
\brief Enable if you want use MICS4514 report istantaneous sensor.
*/
#define USE_SENSOR_IMI              (false)

/*!
\def USE_SENSOR_NMI
\brief Enable if you want use MICS4514 report minium sensor.
*/
#define USE_SENSOR_NMI              (false)

/*!
\def USE_SENSOR_MMI
\brief Enable if you want use MICS4514 report average sensor.
*/
#define USE_SENSOR_MMI              (false)

/*!
\def USE_SENSOR_XMI
\brief Enable if you want use MICS4514 report maximum sensor.
*/
#define USE_SENSOR_XMI              (false)

/*!
\def USE_SENSOR_RF24
\brief Enable if you want use Radio RF24 sensor.
*/
#define USE_SENSOR_RF24             (false)

// OPC
#define VALUES_TO_READ_FROM_SENSOR_COUNT      (2)
#define JSONS_TO_READ_FROM_SENSOR_COUNT       (2)

// OPC
// #define VALUES_TO_READ_FROM_SENSOR_COUNT      (24)
// #define JSONS_TO_READ_FROM_SENSOR_COUNT       (1)

#define USE_TH_SENSORS                        (USE_SENSOR_ADT + USE_SENSOR_HIH + USE_SENSOR_HYT + USE_SENSOR_SHT + USE_SENSOR_STH + USE_SENSOR_ITH + USE_SENSOR_MTH + USE_SENSOR_NTH + USE_SENSOR_XTH)
#define USE_RAIN_SENSORS                      (USE_SENSOR_TBR + USE_SENSOR_TBS)
#define USE_RADIAITION_SENSORS                (USE_SENSOR_DSR + USE_SENSOR_VSR + USE_SENSOR_DSA)
#define USE_WIND_SENSORS                      (USE_SENSOR_DWA + USE_SENSOR_DWB + USE_SENSOR_DWC + USE_SENSOR_DWD + USE_SENSOR_DWE + USE_SENSOR_DWF)
#define USE_POWER_MPPT_SENSORS                (USE_SENSOR_DEP)

#if (USE_TH_SENSORS && (USE_RAIN_SENSORS == 0))
#define USE_MODULE_TH                         (true)
#elif ((USE_TH_SENSORS == 0) && USE_RAIN_SENSORS)
#define USE_MODULE_RAIN                       (true)
#elif (USE_TH_SENSORS && USE_RAIN_SENSORS && USE_THR)
#define USE_MODULE_THR                        (true)
#elif (USE_RADIAITION_SENSORS)
#define USE_MODULE_SOLAR_RADIATION            (true)
#elif (USE_WIND_SENSORS)
#define USE_MODULE_WIND                       (true)
#elif (USE_POWER_MPPT_SENSORS)
#define USE_MODULE_POWER_MPPT                 (true)
#endif

#if USE_TH_SENSORS
#define SENSORS_COUNT_MAX                     (USE_SENSOR_HYT + USE_SENSOR_SHT + USE_REDUNDANT_SENSOR)
#else
#define SENSORS_COUNT_MAX                     1
#endif

/*!
\def SENSORS_MAX
\brief Max number of sensor.
*/
#define SENSORS_MAX                           (SENSORS_COUNT_MAX)

/*!
\def SENSORS_UNIQUE_MAX
\brief Max number of unique sensor.
unique sensors are sensors that can have more driver but only one i2c address and only one setup and prepare
*/
#define SENSORS_UNIQUE_MAX                    (1)

#endif
