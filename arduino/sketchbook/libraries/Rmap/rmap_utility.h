/**@file rmap_utility.h */

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

#ifndef _RMAP_UTILITY_H
#define _RMAP_UTILITY_H

#include <TimeLib.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "stima_module.h"
#include <mqtt_config.h>
#include <sensors_config.h>

#if (USE_JSON)
#include <json_config.h>
#include <ArduinoJson.h>
#endif

/*!
\def executeTimerTaskEach(current, desidered, offset)
\brief Return true or false if current == desidered calculated in each offset.
*/
#define executeTimerTaskEach(current, desidered, offset) (desidered % offset == 0 ? (current % desidered == 0 ? 1 : 0) : 0)

#if (USE_JSON)
/*!
\fn uint8_t jsonToMqtt(const char *json, const char *mqtt_sensor, char topic[][MQTT_SENSOR_TOPIC_LENGTH], char message[][MQTT_MESSAGE_LENGTH], tmElements_t *sensor_reading_time)
\brief Transform SensorDriver json string in MQTT root and value.
\param[in] *json SensorDriver json string.
\param[in] *mqtt_sensor MQTT sensor root topic ({aaa},{bbb},{ccc}/{ddd},{eee},{fff},{ggg}/)
\param[out] *topic[][MQTT_SENSOR_TOPIC_LENGTH] buffer for storing mqtt full sensor's topic ({path}/{user}/{lon},{lat}/{typo}/{aaa},{bbb},{ccc}/{ddd},{eee},{fff},{ggg}/BXXXXX).
\param[out] *message[][MQTT_MESSAGE_LENGTH] buffer for storing mqtt sensor's value ({"v":xxx,"t":"yyyy-mm-ddTHH:MM:SS"}).
\param[in] *sensor_reading_time the sensor's reading time.
\return number of sensors found in SensorDriver's json string.
*/
uint8_t jsonToMqtt(const char *json, const char *mqtt_sensor, char topic[][MQTT_SENSOR_TOPIC_LENGTH], char message[][MQTT_MESSAGE_LENGTH], tmElements_t *sensor_reading_time);

/*!
\fn time_t getDateFromMessage(char *message)
\brief Return a time_t time from a mqtt sensor's value.
\param[in] *message mqtt sensor's value ({"v":xxx,"t":"yyyy-mm-ddTHH:MM:SS"}).
\return time extracted from a mqtt sensor's value.
*/
time_t getDateFromMessage(char *message);
#endif

/*!
\fn void mqttToSd(const char *topic, const char *message, char *sd)
\brief Return a sdcard string from mqtt sensor's topic and mqtt sensor's value for storing on sdcard file.
\param[in] *topic full mqtt sensor's topic ({path}/{user}/{lon},{lat}/{typo}/{aaa},{bbb},{ccc}/{ddd},{eee},{fff},{ggg}/BXXXXX)
\param[in] *message mqtt sensor's value.
\param[out] *sd sdcard string ({aaa},{bbb},{ccc}/{ddd},{eee},{fff},{ggg}/BXXXXX {"v":xxx,"t":"yyyy-mm-ddTHH:MM:SS"}).
\return void.
*/
void mqttToSd(const char *topic, const char *message, char *sd);

/*!
\fn void sdToMqtt(const char *sd, char *topic, char *message)
\brief Break sdcard string in mqtt sensor's topic and mqtt sensors' value.
\param[in] *sd sdcard string ({aaa},{bbb},{ccc}/{ddd},{eee},{fff},{ggg}/BXXXXX {"v":xxx,"t":"yyyy-mm-ddTHH:MM:SS"}).
\param[out] *topic mqtt sensor's topic ({aaa},{bbb},{ccc}/{ddd},{eee},{fff},{ggg}/BXXXXX).
\param[out] *message mqtt sensor's topic ({"v":xxx,"t":"yyyy-mm-ddTHH:MM:SS"}).
\return void.
*/
void sdToMqtt(const char *sd, char *topic, char *message);

/*!
\fn void getFullTopic(char *full_topic, const char *root_topic, const char *sensor_topic)
\brief Return complete mqtt topic from root topic and sensor topic.
\param[out] *full_topic a complete topic buffer.
\param[in] *root_topic mqtt root topic ({path}/{user}/{lon},{lat}/{typo}/).
\param[in] *sensor_topic mqtt sensor's topic ({aaa},{bbb},{ccc}/{ddd},{eee},{fff},{ggg}/BXXXXX).
\return void.
*/
void getFullTopic(char *full_topic, const char *root_topic, const char *sensor_topic);

/*!
\fn void getStimaNameByType(char *name, uint8_t type)
\brief Return a STIMA's name starting from a module type stored in configuration.
\param[out] *name STIMA's name.
\param[in] *type module type stored in configuration.
\return void.
*/
void getStimaNameByType(char *name, uint8_t type);

/*!
\fn void stringToArray(uint8_t *array, char *string, const char *delimiter, uint8_t base)
\brief Divide string in multiple value, searching by delimiter. Then convert the value in numerical format specified by base and save the values in array.
\param[out] *array pointer to array obtained from string.
\param[in] *string source string to divide searching by delimiter.
\param[in] *delimiter a delimiter for divide string in multiple values.
\param[in] base it is a base for strtol function: numerical base (radix) that determines the valid characters and their interpretation.
\return void.
*/
void stringToArray(uint8_t *array, char *string, const char *delimiter, uint8_t base);

/*!
\fn void getLonLatFromMqttTopic(const char *topic, char *lon, char *lat)
\brief Extract longitue and latitude from mqtt root topic.
\param[in] *topic mqtt topic.
\param[out] *lon longitue buffer.
\param[out] *lat latitude buffer.
\return void.
*/
void getLonLatFromMqttTopic(const char *topic, char *lon, char *lat);

/*!
\fn void getMqttClientIdFromMqttTopic(const char *topic, char *client_id)
\brief Extract mqtt client id from mqtt root topic.
\param[in] *topic mqtt topic.
\param[out] *client_id mqtt client id.
\return void.
*/
void getMqttClientIdFromMqttTopic(const char *topic, char *client_id);

/*!
\def macStringToArray(mac, string)
\brief Return an array of value rappresenting a mac address value extracted by its canonical string (xx:xx:xx:xx:xx:xx).
*/
#define macStringToArray(mac, string)     (stringToArray(mac, string, ":", 16))

/*!
\def ipStringToArray(ip, string)
\brief Return an array of value rappresenting a ip address value extracted by its canonical string (xxx.xxx.xxx.xxx).
*/
#define ipStringToArray(ip, string)       (stringToArray(ip, string, ".", 10))

/*!
\def dateStringToArray(date, string)
\brief Return an array of value rappresenting a date value extracted by its canonical string (xx-xx-xxxx).
*/
#define dateStringToArray(date, string)   (stringToArray(date, string, "-", 10))

/*!
\def timeStringToArray(time, string)
\brief Return an array of value rappresenting a time value extracted by its canonical string (xx:xx:xx).
*/
#define timeStringToArray(time, string)   (stringToArray(time, string, ":", 10))

#endif
