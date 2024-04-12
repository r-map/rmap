/**@file sensors_config.h */

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

#ifndef SENSOR_CONFIG_H
#define SENSOR_CONFIG_H


/*!
\def SENSOR_MAX
\brief Max number of sensor.
Max value here is 25 for stima atmega1284
*/
#define SENSORS_MAX      (6)

/*!
\def SENSOR_UNIQUE_MAX
\brief Max number of unique sensor (Stima modules).
unique sensors are sensors that can have more driver but only one i2c address and only one setup and prepare
Max value here is 10
*/
#define SENSORS_UNIQUE_MAX      (6)

/*!
\def USE_JSON
\brief Enable if you want use json library for json response (getJson function in SensorDriver).
*/
#define USE_JSON                    (true)

/*!
\def USE_SENSOR_SPS
\brief Enable if you want use Sensirion SPS30 PM sensor.
*/
#define USE_SENSOR_SPS              (true)

/*!
\def USE_SENSOR_SCD
\brief Enable if you want use Sensiorion SCD30 CO2 sensor.
*/
#define USE_SENSOR_SCD              (true)

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
\brief Enable if you want use HYT271 or HYT221 sensor.
*/
#define USE_SENSOR_HYT              (false)

/*!
\def SENSOR_TYPE_SHT
\brief Enable if you want use Sensirion humidity and temperature sensor support
Supported sensors:
- SHTC1
- SHTW1
- SHTW2
- SHT3x-DIS (I2C)
- SHT3x-ARP (ratiometric analog voltage output)
*/
#define USE_SENSOR_SHT              (true)

/*!
\def USE_SENSOR_DEP
\brief Enable if you want use DigitEco Power sensor.
*/
#define USE_SENSOR_DEP              (false)

/*!
\def USE_SENSOR_POW
\brief Enable if you want use Power panel and battery sensor.
*/
#define USE_SENSOR_POW              (false)

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
\brief Enable if you want vectorial average Wind Speed and Direction over 10' (WMO).
*/
#define USE_SENSOR_DWA              (false)

/*!
\def USE_SENSOR_DWB
\brief Enable if you want vectorial average Wind Speed and Direction over report time.
*/
#define USE_SENSOR_DWB              (false)

/*!
\def USE_SENSOR_DWC
\brief Enable if you want gust and long gust Wind Speed over report time.
*/
#define USE_SENSOR_DWC              (false)

/*!
\def USE_SENSOR_DWD
\brief Enable if you want average Wind Speed over report time.
*/
#define USE_SENSOR_DWD              (false)

/*!
\def USE_SENSOR_DWE
\brief Enable if you want frequency class Wind Speed over report time.
*/
#define USE_SENSOR_DWE              (false)

/*!
\def USE_SENSOR_DWF
\brief Enable if you want gust and long gust Wind Direction over report time.
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
\def USE_SENSOR_OC2
\brief Enable if you want use OPC BINS continuous average value.
*/
#define USE_SENSOR_OC2              (false)
#define USE_SENSOR_OC3              (false)

/*!
\def USE_SENSOR_OD2
\brief Enable if you want use OPC BINS continuous standard deviation value.
*/
#define USE_SENSOR_OD2              (false)
#define USE_SENSOR_OD3              (false)

/*!
\def USE_SENSOR_OD2
\brief Enable if you want use OPC BINS continuous standard deviation value.
*/
#define USE_SENSOR_OE3              (false)

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
\def USE_SENSOR_TBR
\brief Enable if you want use Tipping bucket rain gauge sensor.
*/
#define USE_SENSOR_TBR              (false)

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
\def USE_SENSOR_GWS
\brief Enable if you want use Gill Windsonic sensor.
*/
#define USE_SENSOR_GWS              (false)

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

/*!
\def VALUES_TO_READ_FROM_SENSOR_COUNT
Maximum number of values to be read by the sensors.
*/
#define VALUES_TO_READ_FROM_SENSOR_COUNT      (9)
#define JSONS_TO_READ_FROM_SENSOR_COUNT       (9)

#endif
