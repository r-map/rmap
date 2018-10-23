/**@file SensorDriverSensors.h */

/*********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Paolo patruno <p.patruno@iperbole.bologna.it>
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

#ifndef SENSOR_DRIVER_SENSORS_H
#define SENSOR_DRIVER_SENSORS_H

/*!
\def SENSOR_DRIVER_I2C
\brief Sensor driver's I2C driver.
*/
#define SENSOR_DRIVER_I2C         ("I2C")

/*!
\def SENSOR_TYPE_ADT
\brief Sensor driver's ADT sensor type for ADT7420.
*/
#define SENSOR_TYPE_ADT           ("ADT")

/*!
\def SENSOR_TYPE_HIH
\brief Sensor driver's HIH sensor type for HIH6100.
*/
#define SENSOR_TYPE_HIH           ("HIH")

/*!
\def SENSOR_TYPE_HYT
\brief Sensor driver's HYT sensor type for HYT271 and HYT221.
*/
#define SENSOR_TYPE_HYT           ("HYT")

/*!
\def SENSOR_TYPE_DEP
\brief Sensor driver's DEP sensor type for DigitEco Power.
*/
#define SENSOR_TYPE_DEP           ("DEP")

/*!
\def SENSOR_TYPE_HI7
\brief Sensor driver's HI7 sensor type for SI7021.
*/
#define SENSOR_TYPE_HI7           ("HI7")

/*!
\def SENSOR_TYPE_BMP
\brief Sensor driver's BMP sensor type for Bmp085.
*/
#define SENSOR_TYPE_BMP           ("BMP")

/*!
\def SENSOR_TYPE_DW1
\brief Sensor driver's DW1 sensor type for oneshot DW1.
*/
#define SENSOR_TYPE_DW1           ("DW1")

/*!
\def SENSOR_TYPE_TBS
\brief Sensor driver's TBS sensor type for oneshot tipping bucket rain gauge.
*/
#define SENSOR_TYPE_TBS           ("TBS")

/*!
\def SENSOR_TYPE_TBR
\brief Sensor driver's TBR sensor type for oneshot tipping bucket rain gauge.
*/
#define SENSOR_TYPE_TBR           ("TBR")

/*!
\def SENSOR_TYPE_STH
\brief Sensor driver's STH sensor type for oneshot istantaneous temperature and humidity.
*/
#define SENSOR_TYPE_STH           ("STH")

/*!
\def SENSOR_TYPE_ITH
\brief Sensor driver's ITH sensor type for continuous istantaneous temperature and humidity.
*/
#define SENSOR_TYPE_ITH           ("ITH")

/*!
\def SENSOR_TYPE_MTH
\brief Sensor driver's MTH sensor type for continuous average temperature and humidity.
*/
#define SENSOR_TYPE_MTH           ("MTH")

/*!
\def SENSOR_TYPE_NTH
\brief Sensor driver's NTH sensor type for continuous minimum temperature and humidity.
*/
#define SENSOR_TYPE_NTH           ("NTH")

/*!
\def SENSOR_TYPE_XTH
\brief Sensor driver's XTH sensor type for continuous maximum temperature and humidity.
*/
#define SENSOR_TYPE_XTH           ("XTH")

/*!
\def SENSOR_TYPE_SSD
\brief Sensor driver's SSD sensor type for  SSD011 oneshot.
*/
#define SENSOR_TYPE_SSD           ("SSD")

/*!
\def SENSOR_TYPE_ISD
\brief Sensor driver's ISD sensor type for SSD011 report istantaneous.
*/
#define SENSOR_TYPE_ISD           ("ISD")

/*!
\def SENSOR_TYPE_MSD
\brief Sensor driver's MSD sensor type for SSD011 report average.
*/
#define SENSOR_TYPE_MSD           ("MSD")

/*!
\def SENSOR_TYPE_NSD
\brief Sensor driver's NSD sensor type for SSD011 report minium.
*/
#define SENSOR_TYPE_NSD           ("NSD")

/*!
\def SENSOR_TYPE_XSD
\brief Sensor driver's XSD sensor type for SSD011 report maximum.
*/
#define SENSOR_TYPE_XSD           ("XSD")

/*!
\def SENSOR_TYPE_SMI
\brief Sensor driver's SMI sensor type for MICS4514 oneshot.
*/
#define SENSOR_TYPE_SMI           ("SMI")

/*!
\def SENSOR_TYPE_IMI
\brief Sensor driver's IMI sensor type for MICS4514 report istantaneous.
*/
#define SENSOR_TYPE_IMI           ("IMI")

/*!
\def SENSOR_TYPE_MMI
\brief Sensor driver's MMI sensor type for MICS4514 report average.
*/
#define SENSOR_TYPE_MMI           ("MMI")

/*!
\def SENSOR_TYPE_NMI
\brief Sensor driver's NMI sensor type for MICS4514 report minium.
*/
#define SENSOR_TYPE_NMI           ("NMI")

/*!
\def SENSOR_TYPE_XMI
\brief Sensor driver's XMI sensor type for MICS4514 report maximum.
*/
#define SENSOR_TYPE_XMI           ("XMI")

/*!
\def SENSOR_TYPE_RF24
\brief Sensor driver's RF24 sensor type for Radio RF24.
*/
#define SENSOR_TYPE_RF24          ("RF24")

#endif
