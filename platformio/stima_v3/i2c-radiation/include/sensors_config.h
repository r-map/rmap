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
\def USE_JSON
\brief Enable if you want use json library for json response (getJson function in SensorDriver).
*/
#define USE_JSON                    (true)

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
\def USE_SENSOR_DSR
\brief Enable if you want use DigitEco Global Solar Radiation sensor.
*/
#define USE_SENSOR_DSR              (false)

/*!
\def USE_SENSOR_VSR
\brief Enable if you want use 0-5V High Resolution Global Solar Radiation sensor.
*/
#define USE_SENSOR_VSR              (true)

/*!
\def USE_SENSOR_DSA
\brief Enable if you want average Global Solar Radiation sensor.
*/
#define USE_SENSOR_DSA              (true)

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
\def USE_SENSOR_TBS
\brief Enable if you want use Tipping bucket rain gauge sensor.
*/
#define USE_SENSOR_TBS              (false)

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
\def RAIN_FOR_TIP
\brief How much mm of rain for one tip of tipping bucket rain gauge.
*/
#define RAIN_FOR_TIP                (1)

/*!
\def VALUES_TO_READ_FROM_SENSOR_COUNT
Maximum number of values to be read by the sensors.
*/
#define VALUES_TO_READ_FROM_SENSOR_COUNT      (3)
#define JSONS_TO_READ_FROM_SENSOR_COUNT       (3)

// sampling every 3-15 seconds --> watchdog timer (SENSORS_SAMPLE_TIME_S in relative modules)
// observations with processing every 1-10 minutes (minutes for processing sampling)
// report every 5-60 minutes (> OBSERVATIONS_MINUTES)
// reported data is calulate by moving average on STATISTICAL_DATA_MINUTES window

// observations every 1-10 minutes (minutes for processing samples)
// report every 5-60 minutes (minutes for report. > n * OBSERVATIONS_MINUTES)

/*!
\def OBSERVATIONS_MINUTES
\brief How much minutes for calculate an observations by processing sampling. Tipically 1-10 minutes.
*/
#define OBSERVATIONS_MINUTES                 (1)

/*!
\def STATISTICAL_DATA_COUNT
\brief How much observations are needed for generating a report.
*/
#define STATISTICAL_DATA_COUNT               (15)

/*!
\def OBSERVATION_COUNT
\brief Observations buffer length.
*/
#define OBSERVATION_COUNT                    (STATISTICAL_DATA_COUNT * 2)

/*!
\def OBSERVATION_COUNT_TOLLERANCE
\brief Tolerance of observations for generating a valid report.
*/
#define OBSERVATION_COUNT_TOLLERANCE         (1)

#if (OBSERVATION_COUNT < STATISTICAL_DATA_COUNT)
#error OBSERVATION_COUNT must be major of STATISTICAL_DATA_COUNT !!!
#endif

#endif
