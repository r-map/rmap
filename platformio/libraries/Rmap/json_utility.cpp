/**@file json_utility.cpp */

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

#include "json_utility.h"

// #if (USE_JSON)
void stringToArray(uint8_t *array, char *string, const char *delimiter, uint8_t base) {
  uint8_t i=0;
  char *token;
  token = strtok(string, delimiter);

  while (token != NULL) {
    array[i++] = (uint8_t)strtol(token, NULL, base);
    token = strtok(NULL, delimiter);
   }
}

void getLonLatFromMqttTopic(const char *topic, char *lon, char *lat) {
    char *temp;
    char string[MQTT_ROOT_TOPIC_LENGTH];
    strncpy(string, topic, MQTT_ROOT_TOPIC_LENGTH);
    char *token = strtok(string, "/");
    token = strtok(NULL, "/");
    token = strtok(NULL, "/");
    temp = strtok(token, ",");
    strcpy(lon, temp);
    temp = strtok(NULL, ",");
    strcpy(lat, temp);
}

void getMqttClientIdFromMqttTopic(const char *topic, char *client_id) {
   char *pch = strchr(topic,'/');
   uint8_t pch_len = strlen(pch);
   strncpy(client_id, pch+1, pch_len);
   client_id[pch_len-2] = 0;
}

// migrated to the new format ident/station_slug/board_slug
void getMqttClientId(const char *username,const char *stationslug,const char *boardslug, char *client_id) {
   strcpy(client_id, username);
   strcat(client_id, "/");
   strcat(client_id, stationslug);
   strcat(client_id, "/");
   strcat(client_id, boardslug);

}

// #if (USE_JSON)
uint8_t jsonToMqtt(const char *json, const char *mqtt_sensor, char topic[][MQTT_SENSOR_TOPIC_LENGTH], char message[][MQTT_MESSAGE_LENGTH], tmElements_t *sensor_reading_time) {
  uint8_t i = 0;
  memset(topic, 0, sizeof(topic[0][0]) * MQTT_SENSOR_TOPIC_LENGTH * JSONS_TO_READ_FROM_SENSOR_COUNT);
  memset(message, 0, sizeof(message[0][0]) * MQTT_MESSAGE_LENGTH * JSONS_TO_READ_FROM_SENSOR_COUNT);

  StaticJsonDocument<JSON_BUFFER_LENGTH> doc;
  DeserializationError error = deserializeJson(doc,json);
  if (error) {
    // Error reading json data: error.c_str();
    return i;
  }

  for (JsonPair it : doc.as<JsonObject>()) {
    if (strcmp(it.key().c_str(), "d") == 0) {
      snprintf(&topic[i][0], MQTT_SENSOR_TOPIC_LENGTH, "%s", mqtt_sensor);
      snprintf(&message[i][0], MQTT_MESSAGE_LENGTH, "{\"d\":%ld,\"p\":", it.value().as<int32_t>());
    }
    else if (strcmp(it.key().c_str(), "p") == 0) {
      if (it.value().is<JsonArray>()) {
        JsonArray array = it.value().as<JsonArray>();
        serializeJson (array,(char *)(&message[i][0] + strlen(&message[i][0])), MQTT_MESSAGE_LENGTH);
      }

      snprintf(&message[i][0] + strlen(&message[i][0]), MQTT_MESSAGE_LENGTH - strlen(&message[i][0]), ",\"t\":\"%04u-%02u-%02uT%02u:%02u:%02u\"}", tmYearToCalendar(sensor_reading_time->Year), sensor_reading_time->Month, sensor_reading_time->Day, sensor_reading_time->Hour, sensor_reading_time->Minute, sensor_reading_time->Second);
      i++;
    }
    else {
      snprintf(&topic[i][0], MQTT_SENSOR_TOPIC_LENGTH, "%s%s", mqtt_sensor, it.key().c_str());

      if (it.value().isNull()) {
        snprintf(&message[i][0], MQTT_MESSAGE_LENGTH, "{\"v\":null,\"t\":\"");
      }
      else {
        snprintf(&message[i][0], MQTT_MESSAGE_LENGTH, "{\"v\":%ld,\"t\":\"", it.value().as<int32_t>());
      }

      snprintf(&message[i][0] + strlen(&message[i][0]), MQTT_MESSAGE_LENGTH - strlen(&message[i][0]), "%04u-%02u-%02uT%02u:%02u:%02u\"}", tmYearToCalendar(sensor_reading_time->Year), sensor_reading_time->Month, sensor_reading_time->Day, sensor_reading_time->Hour, sensor_reading_time->Minute, sensor_reading_time->Second);
      i++;
    }
  }

  return i;
}

time_t getDateFromMessage(char *message) {
   tmElements_t _datetime;
   char temp[JSON_BUFFER_LENGTH];
   char str_buffer[11];
   uint8_t dt[3];
   uint8_t delimiter = (uint8_t) strcspn(message, "{");
   strcpy(temp, message + delimiter);

   StaticJsonDocument<JSON_BUFFER_LENGTH> doc;
   /*
   DeserializationError error = deserializeJson(doc,temp);
   if (error) {
     // Error reading json data: error.c_str();
     return i;
   }
   */
   deserializeJson(doc,temp);
   const char *datetime = doc["t"];
   
   delimiter = (uint8_t) strcspn(datetime, "T");
   strncpy(str_buffer, datetime+2, delimiter);
   str_buffer[delimiter] = '\0';
   dateStringToArray(dt, str_buffer);

   if (dt[0] <= year()-2000) {
      _datetime.Year = CalendarYrToTm(dt[0]+2000);
   }
   else {
      _datetime.Year = CalendarYrToTm(dt[0]+1900);
   }

   _datetime.Month = dt[1];
   _datetime.Day = dt[2];

   strcpy(str_buffer, datetime + delimiter + 1);
   timeStringToArray(dt, str_buffer);

   _datetime.Hour = dt[0];
   _datetime.Minute = dt[1];
   _datetime.Second = dt[2];

   return makeTime(_datetime);
}

void mqttToSd(const char *topic, const char *message, char *sd) {
   memset(sd, 0, sizeof(sd[0]) * (MQTT_SENSOR_TOPIC_LENGTH + MQTT_MESSAGE_LENGTH));
   snprintf(sd, MQTT_SENSOR_TOPIC_LENGTH + MQTT_MESSAGE_LENGTH, "%s %s", topic, message);
}

void sdToMqtt(const char *sd, char *topic, char *message) {
   memset(topic, 0, sizeof(topic[0]) * MQTT_SENSOR_TOPIC_LENGTH);
   memset(message, 0, sizeof(message[0]) * MQTT_MESSAGE_LENGTH);

   uint8_t delimiter = (uint8_t) strcspn(sd, " ");

   strncpy(topic, sd, delimiter);
   topic[delimiter] = '\0';
   strcpy(message, sd + delimiter + 1);
}

void getFullTopic(char *full_topic, const char *root_topic, const char *sensor_topic) {
   memset(full_topic, 0, MQTT_ROOT_TOPIC_LENGTH + MQTT_SENSOR_TOPIC_LENGTH);
   strncpy(full_topic, root_topic, MQTT_ROOT_TOPIC_LENGTH);
   strncpy(full_topic + strlen(root_topic), sensor_topic, MQTT_SENSOR_TOPIC_LENGTH);
}
// #endif
