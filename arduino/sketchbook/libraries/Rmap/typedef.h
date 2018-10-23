/**@file typedef.h */

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

#ifndef _TYPEDEF_H
#define _TYPEDEF_H

#include <mqtt_config.h>

/*!
\def DRIVER_LENGTH
\brief Sensor driver's buffer length.
*/
#define DRIVER_LENGTH  (5)

/*!
\def TYPE_LENGTH
\brief Sensor type's buffer length.
*/
#define TYPE_LENGTH    (5)

/*!
\struct sensor_t
\brief Sensor struct for storing sensor configuration parameter.
*/
typedef struct {
  char driver[DRIVER_LENGTH];            //!< sensor's string driver
  char type[TYPE_LENGTH];                //!< sensor's string type
  uint8_t address;                              //!< sensor's address
  uint8_t node;                                 //!< sensor's node
  char mqtt_topic[MQTT_SENSOR_TOPIC_LENGTH];    //!< sensor's mqtt topic path
} sensor_t;

/*!
\struct rain_t
\brief Rain tips struct for storing rain tips count.
*/
typedef struct {
  uint16_t tips_count;  //!< rain gauge tips counter
} rain_t;

/*!
\struct value_t
\brief Value struct for storing sample, observation and minium, average and maximum measurement.
*/
typedef struct {
  uint16_t sample; //!< last sample
  uint16_t med60;  //!< last observation
  uint16_t med;    //!< average values of observations
  uint16_t max;    //!< maximum values of observations
  uint16_t min;    //!< minium values of observations
  uint16_t sigma;  //!< standard deviation of observations
} value_t;

#endif
