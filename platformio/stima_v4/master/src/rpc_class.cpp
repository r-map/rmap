/**
  ******************************************************************************
  * @file    rpc_class.cpp
  * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   RPC Object Class for register RPC function, CallBack and manage data
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (C) 2022  Moreno Gasperini <m.gasperini@digiteco.it>
  * All rights reserved.</center></h2>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  * 
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
  * <http://www.gnu.org/licenses/>.
  * 
  ******************************************************************************
*/

#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "rpc_class.hpp"

// Contsructor
RegisterRPC::RegisterRPC()
{
}
RegisterRPC::RegisterRPC(RpcParam_t rpcParam)
{
  param = rpcParam;
}

void RegisterRPC::init(JsonRPC *streamRpc)
{
#if (USE_RPC_METHOD_CONFIGURE)
  streamRpc->registerMethod("configure", &configure);
#endif

#if (USE_RPC_METHOD_PREPARE)
  streamRpc->registerMethod("prepare", &prepare);
#endif

#if (USE_RPC_METHOD_GETJSON)
  streamRpc->registerMethod("getjson", &getjson);
#endif

#if (USE_RPC_METHOD_PREPANDGET)
  streamRpc->registerMethod("prepandget", &prepandget);
#endif

#if (USE_RPC_METHOD_REBOOT)
  streamRpc->registerMethod("reboot", &reboot);
#endif

#if (USE_RPC_METHOD_TEST)
  streamRpc->registerMethod("rpctest", &rpctest);
#endif

#if (USE_RPC_METHOD_RECOVERY && USE_MQTT)
  streamRpc->registerMethod("recovery", &recovery);
#endif
}

#if (USE_RPC_METHOD_CONFIGURE)
int RegisterRPC::configure(JsonObject params, JsonObject result)
{
  bool is_error = false;
  bool is_sensor_config = false;

  for (JsonPair it : params)
  {
    // loop in params
    if (strcmp(it.key().c_str(), "reset") == 0)
    {
      if (it.value().as<bool>() == true)
      {
        TRACE_INFO_F(F("DO RESET CONFIGURATION\r\n"));
        // set_default_configuration();
        // lcd_error |= lcd.clear();
        // lcd_error |= lcd.print(F("Reset configuration")) == 0;
      }
      else if (strcmp(it.key().c_str(), "save") == 0)
      {
        if (it.value().as<bool>() == true)
        {
          TRACE_INFO_F(F("DO SAVE CONFIGURATION\r\n"));
          // save_configuration(CONFIGURATION_CURRENT);
          // lcd_error |= lcd.clear();
          // lcd_error |= lcd.print(F("Save configuration")) == 0;
        }
      }
      else if (strcmp(it.key().c_str(), "board") == 0)
      {
        // it.value().as<const char *>()
      }
      else if (strcmp(it.key().c_str(), "boardtype") == 0)
      {
        // it.value().as<unsigned int>()
      }
      else if (strcmp(it.key().c_str(), "sn") == 0)
      {
        // it.value().as<const char *>()
      }
      else if (strcmp(it.key().c_str(), "cansampletime") == 0)
      {
        // it.value().as<unsigned int>()
      }
      else if (strcmp(it.key().c_str(), "node_id") == 0)
      {
        // it.value().as<unsigned int>()
      }
      else if (strcmp(it.key().c_str(), "subject") == 0)
      {
        // it.value().as<const char *>()
      }
      else if (strcmp(it.key().c_str(), "subject_id") == 0)
      {
        // it.value().as<unsigned int>()
      }
      else if (strcmp(it.key().c_str(), "driver") == 0)
      {
        // it.value().as<const char *>()
        is_sensor_config = true;
      }
      else if (strcmp(it.key().c_str(), "type") == 0)
      {
        // it.value().as<const char *>()
        is_sensor_config = true;
      }
      else if (strcmp(it.key().c_str(), "timerange") == 0)
      {
        // Pindicator
        // it.value().as<JsonArray>()[0].as<unsigned int>()

        // P1
        // it.value().as<JsonArray>()[1].as<unsigned int>()

        // P2
        // it.value().as<JsonArray>()[2].as<unsigned int>()

        is_sensor_config = true;
      }
      else if (strcmp(it.key().c_str(), "level") == 0)
      {
        // LevelType1
        // it.value().as<JsonArray>()[0].as<unsigned int>()

        // L1
        // it.value().as<JsonArray>()[1].as<unsigned int>()

        // LevelType2
        // it.value().as<JsonArray>()[2].as<unsigned int>()

        // L2
        // it.value().as<JsonArray>()[3].as<unsigned int>()

        is_sensor_config = true;
      }
      else if (strcmp(it.key().c_str(), "sd") == 0)
      {
        for (JsonPair sd : it.value().as<JsonObject>())
        {
          // constantdata btable
          // sd.key().c_str()

          // constantdata value
          // sd.value().as<const char *>()

          // constantdata index must be incremented in order to configure the next value
          // if (constantdata_count < USE_CONSTANTDATA_COUNT)
          // {
          //   constantdata_count++;
          // }
          // else
          // {
          //   is_error = true;
          // }
        }
      }
      else if (strcmp(it.key().c_str(), "stationslug") == 0)
      {
        // it.value().as<const char *>()
      }
      else if (strcmp(it.key().c_str(), "boardslug") == 0)
      {
        // it.value().as<const char *>()
      }
      else if (strcmp(it.key().c_str(), "lon") == 0)
      {
        // it.value().as<long int>()
      }
      else if (strcmp(it.key().c_str(), "lat") == 0)
      {
        // it.value().as<long int>()
      }
      else if (strcmp(it.key().c_str(), "network") == 0)
      {
        // it.value().as<const char *>()
      }
    }
    else if (strcmp(it.key().c_str(), "date") == 0)
    {
      // tmElements_t tm;
      // tm.Year = y2kYearToTm(it.value().as<JsonArray>()[0].as<int>() - 2000);
      // tm.Month = it.value().as<JsonArray>()[1].as<int>();
      // tm.Day = it.value().as<JsonArray>()[2].as<int>();
      // tm.Hour = it.value().as<JsonArray>()[3].as<int>();
      // tm.Minute = it.value().as<JsonArray>()[4].as<int>();
      // tm.Second = it.value().as<JsonArray>()[5].as<int>();
      // system_time = makeTime(tm);
    }
#if (USE_MQTT)
    else if (strcmp(it.key().c_str(), "mqttrootpath") == 0)
    {
      // mqtt_root_topic
      // it.value().as<const char *>()
    }
    else if (strcmp(it.key().c_str(), "mqttrpcpath") == 0)
    {
      // mqtt_rpc_topic
      // it.value().as<const char *>()
    }
    else if (strcmp(it.key().c_str(), "mqttmaintpath") == 0)
    {
      // mqtt_maint_topic
      // it.value().as<const char *>()
    }
    else if (strcmp(it.key().c_str(), "mqttserver") == 0)
    {
      // mqtt_server
      // it.value().as<const char *>()
    }
    else if (strcmp(it.key().c_str(), "mqttsampletime") == 0)
    {
      // report_s
      // it.value().as<unsigned int>()
    }
    else if (strcmp(it.key().c_str(), "mqttuser") == 0)
    {
      // mqtt_username
      // it.value().as<const char *>()
    }
    else if (strcmp(it.key().c_str(), "mqttpassword") == 0)
    {
      // mqtt_password
      // it.value().as<const char *>()
    }
    else if (strcmp(it.key().c_str(), "mqttpskkey") == 0)
    {
      // client_psk_key
      // trasformare da stringa come it.value().as<const char *>() array di uint8_t per salvataggio in client_psk_key
    }
#endif
#if (USE_NTP)
    else if (strcmp(it.key().c_str(), "ntpserver") == 0)
    {
      // ntp_server
      // it.value().as<const char *>()
    }
#endif
#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
    else if (strcmp(it.key().c_str(), "gsmapn") == 0)
    {
      // gsm_apn
      // it.value().as<const char *>()
    }
    else if (strcmp(it.key().c_str(), "pppnumber") == 0)
    {
      // gsm_number
      // it.value().as<const char *>()
    }
#endif
    else
    {
      is_error = true;
    }
  }

  // when is_sensor_config = true a sensor was configured, then the index sensors_count must be incremented
  // in order to configure the next sensor
  if (is_sensor_config)
  {
    // if (writable_configuration.sensors_count < SENSORS_MAX)
    // {
    //   // writable_configuration.sensors_count++;
    // }
    // else
    // {
    //   is_error = true;
    // }
  }

  if (is_error)
  {
    result[F("state")] = F("error");
    return E_INVALID_PARAMS;
  }
  else
  {
    result[F("state")] = F("done");
    return E_SUCCESS;
  }
  }
#endif

#if (USE_RPC_METHOD_PREPARE || USE_RPC_METHOD_PREPANDGET || USE_RPC_METHOD_GETJSON)
  bool extractSensorsParams(JsonObject &params, char *driver, char *type, uint8_t *address, uint8_t *node)
  {
  bool is_error = false;

  for (JsonObject::iterator it = params.begin(); it != params.end(); ++it)
  {
    if (strcmp(it.key().c_str(), "driver") == 0)
    {
      strncpy(driver, it.value().as<const char *>(), DRIVER_LENGTH);
    }
    else if (strcmp(it.key().c_str(), "type") == 0)
    {
      strncpy(type, it.value().as<const char *>(), TYPE_LENGTH);
    }
    else if (strcmp(it.key().c_str(), "address") == 0)
    {
      *address = it.value().as<unsigned char>();
    }
    else if (strcmp(it.key().c_str(), "node") == 0)
    {
      *node = it.value().as<unsigned char>();
    }
    else
    {
      is_error = true;
    }
  }

  return is_error;
  }
#endif

#if (USE_RPC_METHOD_PREPARE)
  int prepare(JsonObject params, JsonObject result)
  {
  static int state;
  static bool is_error = false;
  static char driver[DRIVER_LENGTH];
  static char type[TYPE_LENGTH];
  static uint8_t address = 0;
  static uint8_t node = 0;
  static uint8_t i;
  static uint32_t wait_time;

  switch (rpc_state)
  {
  case RPC_INIT:
    state = E_BUSY;
    is_error = extractSensorsParams(params, driver, type, &address, &node);

    if (!is_error && !is_event_sensors_reading)
    {
      is_event_sensors_reading_rpc = true;
      rpc_state = RPC_EXECUTE;
    }
    else
    {
      rpc_state = RPC_END;
    }
    break;

  case RPC_EXECUTE:
    if (is_event_sensors_reading_rpc)
    {
      sensors_reading_task(true, false, driver, type, address, node, &i, &wait_time);
    }
    else
    {
      rpc_state = RPC_END;
    }
    break;

  case RPC_END:
    if (is_error)
    {
      result[F("state")] = F("error");
      state = E_INVALID_PARAMS;
    }
    else
    {
      result[F("state")] = F("done");
      result[F("waittime")] = wait_time;
      state = E_SUCCESS;
    }
    rpc_state = RPC_INIT;
    break;
  }

  return state;
  }
#endif

#if (USE_RPC_METHOD_GETJSON)
  int getjson(JsonObject params, JsonObject result)
  {
  static int state;
  static bool is_error = false;
  static char driver[DRIVER_LENGTH];
  static char type[TYPE_LENGTH];
  static uint8_t address = 0;
  static uint8_t node = 0;
  static uint8_t i;
  static uint32_t wait_time;
  static char sensor_reading_time_buffer[DATE_TIME_STRING_LENGTH];

  switch (rpc_state)
  {
  case RPC_INIT:
    state = E_BUSY;
    is_error = extractSensorsParams(params, driver, type, &address, &node);

    if (!is_error && !is_event_sensors_reading)
    {
      is_event_sensors_reading_rpc = true;
      rpc_state = RPC_EXECUTE;
    }
    else
    {
      rpc_state = RPC_END;
    }
    break;

  case RPC_EXECUTE:
    if (is_event_sensors_reading_rpc)
    {
      sensors_reading_task(false, true, driver, type, address, node, &i, &wait_time);
    }
    else
    {
      rpc_state = RPC_END;
    }
    break;

  case RPC_END:
    if (is_error)
    {
      result[F("state")] = F("error");
      state = E_INVALID_PARAMS;
    }
    else
    {
      StaticJsonBuffer<JSON_BUFFER_LENGTH * 2> jsonBuffer;
      JsonObject &v = jsonBuffer.parseObject((const char *)&json_sensors_data[i][0]);
            snprintf(sensor_reading_time_buffer, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u", year(), month(), day(), hour(), minute(), second())unsigned int>() == 0)
            {
        result[F("v")][(char *)it.key().c_str()] = (char *)NULL;
            }
            else
            {
        result[F("v")][(char *)it.key().c_str()] = it.value().as<unsigned int>();
            }
    }

    result[F("t")] = sensor_reading_time_buffer;
    state = E_SUCCESS;
  }
  rpc_state = RPC_INIT;
  break;
  }

  return state;
  }
#endif

#if (USE_RPC_METHOD_PREPANDGET)
  int prepandget(JsonObject params, JsonObject result)
  {
  static int state;
  static bool is_error = false;
  static char driver[DRIVER_LENGTH];
  static char type[TYPE_LENGTH];
  static uint8_t address = 0;
  static uint8_t node = 0;
  static uint8_t i;
  static uint32_t wait_time;
  static char sensor_reading_time_buffer[DATE_TIME_STRING_LENGTH];

  switch (rpc_state)
  {
  case RPC_INIT:
    state = E_BUSY;
    is_error = extractSensorsParams(params, driver, type, &address, &node);

    if (!is_error && !is_event_sensors_reading)
    {
            is_event_sensors_reading_rpc = true;
            rpc_state = RPC_EXECUTE;
    }
    else
    {
            rpc_state = RPC_END;
    }
    break;

  case RPC_EXECUTE:
    if (is_event_sensors_reading_rpc)
    {
            sensors_reading_task(true, true, driver, type, address, node, &i, &wait_time);
    }
    else
    {
            rpc_state = RPC_END;
    }
    break;

  case RPC_END:
    if (is_error)
    {
            result[F("state")] = F("error");
            state = E_INVALID_PARAMS;
    }
    else
    {
            StaticJsonBuffer<JSON_BUFFER_LENGTH * 2> jsonBuffer;
            JsonObject &v = jsonBuffer.parseObject((const char *)&json_sensors_data[i][0]);
            snprintf(sensor_reading_time_buffer, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u", year(), month(), day(), hour(), minute(), second());

            result[F("state")] = F("done");
            result.createNestedObject(F("v"));

            for (JsonObject::iterator it = v.begin(); it != v.end(); ++it)
            {
        if (it.value().as<unsigned int>() == 0)
        {
          result[F("v")][(char *)it.key().c_str()] = (char *)NULL;
        }
        else
        {
          result[F("v")][(char *)it.key().c_str()] = it.value().as<unsigned int>();
        }
            }

            result[F("t")] = sensor_reading_time_buffer;
            state = E_SUCCESS;
    }
    rpc_state = RPC_INIT;
    break;
  }

  return state;
  }
#endif

#if (USE_RPC_METHOD_RECOVERY && USE_MQTT)
int recovery(JsonObject params, JsonObject result)
{
  static int state;
  static int tmpstate;
  static time_t ptr_time;
  static File mqtt_ptr_rpc_file;

  switch (rpc_state)
  {
  case RPC_INIT:

    state = E_BUSY;
    {
      bool found = false;

      for (JsonPair it : params)
      {
        if (strcmp(it.key().c_str(), "dts") == 0)
        {
          found = true;
          if (!sdcard_open_file(&SD, &mqtt_ptr_rpc_file, SDCARD_MQTT_PTR_RPC_FILE_NAME, O_RDWR | O_CREAT))
          {
            tmpstate = E_INTERNAL_ERROR;
            is_sdcard_error = true;
            result[F("state")] = F("error");
            LOGE(F("SD Card opening ptr data on file %s... [ %s ]"), SDCARD_MQTT_PTR_RPC_FILE_NAME, FAIL_STRING);
            rpc_state = RPC_END;
            break;
          }

          tmElements_t datetime;
          datetime.Year = CalendarYrToTm(it.value().as<JsonArray>()[0].as<int>());
          datetime.Month = it.value().as<JsonArray>()[1].as<int>();
          datetime.Day = it.value().as<JsonArray>()[2].as<int>();
          datetime.Hour = it.value().as<JsonArray>()[3].as<int>();
          datetime.Minute = it.value().as<JsonArray>()[4].as<int>();
          datetime.Second = it.value().as<JsonArray>()[5].as<int>();
          ptr_time = makeTime(datetime);
          LOGN(F("RPC Data pointer... [ %d/%d/%d %d:%d:%d ]"), datetime.Day, datetime.Month, tmYearToCalendar(datetime.Year), datetime.Hour, datetime.Minute, datetime.Second);

          rpc_state = RPC_EXECUTE;

          break;
        }
      }

      if (!found)
      {
        tmpstate = E_INVALID_PARAMS;
        result[F("state")] = F("error");
        LOGE(F("Invalid params [ %s ]"), FAIL_STRING);

        rpc_state = RPC_END;
      }
    }
    break;

  case RPC_EXECUTE:

    if (mqtt_ptr_rpc_file.seekSet(0) && mqtt_ptr_rpc_file.write(&ptr_time, sizeof(time_t)) == sizeof(time_t))
    {
      mqtt_ptr_rpc_file.flush();
      mqtt_ptr_rpc_file.close();

      LOGN(F("SD Card writing ptr data on file %s... [ %s ]"), SDCARD_MQTT_PTR_RPC_FILE_NAME, OK_STRING);
      tmpstate = E_SUCCESS;
      result[F("state")] = F("done");
    }
    else
    {
      tmpstate = E_INTERNAL_ERROR;
      result[F("state")] = F("error");
      LOGE(F("SD Card writing ptr data on file %s... [ %s ]"), SDCARD_MQTT_PTR_RPC_FILE_NAME, FAIL_STRING);
    }

    rpc_state = RPC_END;

  case RPC_END:

    rpc_state = RPC_INIT;
    state = tmpstate;
    break;
  }

  return state;
}
#endif

#if (USE_RPC_METHOD_REBOOT)
int RegisterRPC::reboot(JsonObject params, JsonObject result)
{
  // print lcd message before reboot

  for (JsonPair it : params)
  {
    // loop in params
    if (strcmp(it.key().c_str(), "update") == 0)
    {
      if (it.value().as<bool>() == true)
      {
        TRACE_INFO_F(F("UPDATE FIRMWARE\r\n"));
        // set_default_configuration();
        // lcd_error |= lcd.clear();
        // lcd_error |= lcd.print(F("Reset configuration")) == 0;
      }
    }
  }

  TRACE_INFO_F(F("DO RESET CONFIGURATION\r\n"));

  TRACE_INFO_F(F("Reboot\r\n"));
  result[F("state")] = "done";
  NVIC_SystemReset(); // Do reboot!
  return E_SUCCESS;
}
#endif

#if (USE_RPC_METHOD_TEST)
int RegisterRPC::rpctest(JsonObject params, JsonObject result)
{
  // print lcd message before reboot

  for (JsonPair it : params)
  {
    // loop in params
    if (strcmp(it.key().c_str(), "update") == 0)
    {
      if (it.value().as<bool>() == true)
      {
        TRACE_INFO_F(F("UPDATE FIRMWARE\r\n"));
        // set_default_configuration();
        // lcd_error |= lcd.clear();
        // lcd_error |= lcd.print(F("Reset configuration")) == 0;
      }
    }
  }

  TRACE_INFO_F(F("DO RPC TEST\r\n"));

  TRACE_INFO_F(F("Rpc TEST\r\n"));
  result[F("state")] = "done";
  // Do something
  return E_SUCCESS;
}
#endif
