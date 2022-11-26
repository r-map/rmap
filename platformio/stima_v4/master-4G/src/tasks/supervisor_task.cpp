/**@file supervisor_task.cpp */

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

#define TRACE_LEVEL SUPERVISOR_TASK_TRACE_LEVEL

#include "tasks/supervisor_task.h"

using namespace cpp_freertos;

SupervisorTask::SupervisorTask(const char *taskName, uint16_t stackSize, uint8_t priority, SupervisorParam_t supervisorParam) : Thread(taskName, stackSize, priority), param(supervisorParam)
{
  eeprom = EEprom(param.wire, param.wireLock);
  state = SUPERVISOR_STATE_INIT;
  Start();
};

void SupervisorTask::Run()
{
  system_request_t request;
  system_response_t response;

  while (true)
  {
    bool is_saved = false;
    bool is_loaded = false;

    osMemset(&request, 0, sizeof(system_request_t));
    osMemset(&response, 0, sizeof(system_response_t));

    switch (state)
    {
    case SUPERVISOR_STATE_INIT:
      retry = 0;

      TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_OPERATION -> SUPERVISOR_STATE_LOAD_CONFIGURATION\r\n"));
      state = SUPERVISOR_STATE_LOAD_CONFIGURATION;
      // saveConfiguration(configuration, configurationLock, CONFIGURATION_DEFAULT);
      break;

    case SUPERVISOR_STATE_LOAD_CONFIGURATION:
      is_loaded = loadConfiguration(param.configuration, param.configurationLock);
      
      if (is_loaded)
      {
        retry = 0;
        
        param.systemStatusLock->Take();
        param.system_status->configuration.is_loaded = true;
        param.systemStatusLock->Give();
        
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_LOAD_CONFIGURATION -> SUPERVISOR_STATE_CHECK_OPERATION\r\n"));
        state = SUPERVISOR_STATE_CHECK_OPERATION;
      }
      else if ((++retry <= SUPERVISOR_TASK_GENERIC_RETRY) && !is_loaded)
      {
        Delay(Ticks::MsToTicks(SUPERVISOR_TASK_GENERIC_RETRY_DELAY_MS));
      }
      else
      {
        param.systemStatusLock->Take();
        param.system_status->configuration.is_loaded = false;
        param.systemStatusLock->Give();

        // gestire condizione di errore di lettura della configurazione
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_LOAD_CONFIGURATION -> ??? Condizione non gestita!!!\r\n"));
        Suspend();
      }
      break;
    
    case SUPERVISOR_STATE_CHECK_OPERATION:
      if (param.system_status->configuration.is_loaded && !param.system_status->connection.is_ntp_synchronized)
      {
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_OPERATION -> SUPERVISOR_STATE_REQUEST_CONNECTION\r\n"));
        state = SUPERVISOR_STATE_REQUEST_CONNECTION;
      }
      else
      {
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_OPERATION -> ??? Condizione non gestita!!!\r\n"));
        Suspend();
      }
      break;

    case SUPERVISOR_STATE_SAVE_CONFIGURATION:
      is_saved = saveConfiguration(param.configuration, param.configurationLock, CONFIGURATION_CURRENT);

      if (is_saved)
      {
        retry = 0;

        param.systemStatusLock->Take();
        param.system_status->configuration.is_saved = true;
        param.system_status->configuration.is_loaded = true;
        param.systemStatusLock->Give();

        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_SAVE_CONFIGURATION -> SUPERVISOR_STATE_CHECK_OPERATION\r\n"));
        state = SUPERVISOR_STATE_CHECK_OPERATION;
      }
      else if ((++retry <= SUPERVISOR_TASK_GENERIC_RETRY) && !is_saved)
      {
        Delay(Ticks::MsToTicks(SUPERVISOR_TASK_GENERIC_RETRY_DELAY_MS));
      }
      else
      {
        param.systemStatusLock->Take();
        param.system_status->configuration.is_saved = false;
        param.system_status->configuration.is_loaded = false;
        param.systemStatusLock->Give();

        // gestire condizione di errore di scrittura della configurazione
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_SAVE_CONFIGURATION -> ??? Condizione non gestita!!!\r\n"));
        Suspend();
      }
      break;

    case SUPERVISOR_STATE_REQUEST_CONNECTION:
      // already connected
      if (param.system_status->connection.is_connected)
      {
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_REQUEST_CONNECTION -> SUPERVISOR_STATE_CHECK_CONNECTION_TYPE\r\n"));
        state = SUPERVISOR_STATE_CHECK_CONNECTION_TYPE;
      }
      // not connected -> request connection.
      else
      {
        request.connection.do_connect = true;
        
        param.systemStatusLock->Take();
        param.system_status->connection.is_connection_ongoing = true;
        param.systemStatusLock->Give();
        
        param.systemRequestQueue->Enqueue(&request, 0);
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_REQUEST_CONNECTION -> SUPERVISOR_STATE_CHECK_CONNECTION\r\n"));
        state = SUPERVISOR_STATE_CHECK_CONNECTION;
      }
      break;
    
    case SUPERVISOR_STATE_CHECK_CONNECTION:
      // wait connection
      if (param.systemResponseQueue->Peek(&response, portMAX_DELAY))
      {
        // ok connected
        if (response.connection.done_connected)
        {
          param.systemResponseQueue->Dequeue(&response, 0);
          param.systemStatusLock->Take();
          param.system_status->connection.is_connected = true;
          param.system_status->connection.is_connection_ongoing = false;
          param.systemStatusLock->Give();
          TRACE_INFO_F(F("%s Connection [ %s ]\r\n"), Thread::GetName().c_str(), OK_STRING);

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_CONNECTION -> SUPERVISOR_STATE_CHECK_CONNECTION_TYPE\r\n"));
          state = SUPERVISOR_STATE_CHECK_CONNECTION_TYPE;
        }
        // error: not connected
        else if (!response.connection.done_connected)
        {
          param.systemResponseQueue->Dequeue(&response, 0);
          param.systemStatusLock->Take();
          param.system_status->connection.is_connected = false;
          param.system_status->connection.is_connection_ongoing = false;
          param.systemStatusLock->Give();
          TRACE_ERROR_F(F("%s Connection [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_CONNECTION -> SUPERVISOR_STATE_END\r\n"));
          state = SUPERVISOR_STATE_CHECK_CONNECTION_TYPE;
        }
        // other
        else
        {
          Delay(Ticks::MsToTicks(SUPERVISOR_TASK_WAIT_DELAY_MS));
        }
      }
      // do something else with non-blocking wait ....
      break;

    case SUPERVISOR_STATE_CHECK_CONNECTION_TYPE:
      if (!param.system_status->connection.is_ntp_synchronized)
      {
        param.systemStatusLock->Take();
        param.system_status->connection.is_ntp_sync_ongoing = true;
        param.systemStatusLock->Give();

        // Request ntp sync
        request.connection.do_ntp_sync = true;
        param.systemRequestQueue->Enqueue(&request, 0);

        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_CONNECTION_TYPE -> SUPERVISOR_STATE_DO_NTP_SYNC\r\n"));
        state = SUPERVISOR_STATE_DO_NTP_SYNC;
      }
      else
      {
        TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_CONNECTION_TYPE -> ??? Condizione non gestita!!!\r\n"));
        Thread::Suspend();
      }
      break;

    case SUPERVISOR_STATE_DO_NTP_SYNC:
      // wait ntp to be sync
      if (param.systemResponseQueue->Peek(&response, portMAX_DELAY))
      {
        // ok ntp synchronized
        if (response.connection.done_ntp_synchronized)
        {
          param.systemResponseQueue->Dequeue(&response, 0);
          param.systemStatusLock->Take();
          param.system_status->connection.is_ntp_sync_ongoing = false;
          param.system_status->connection.is_ntp_synchronized = true;
          param.systemStatusLock->Give();

          TRACE_INFO_F(F("%s NTP synchronization [ %s ]\r\n"), Thread::GetName().c_str(), OK_STRING);

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_DO_NTP_SYNC -> SUPERVISOR_STATE_CHECK_OPERATION\r\n"));
          state = SUPERVISOR_STATE_CHECK_OPERATION;
        }
        // error: not connected
        else if (!response.connection.done_ntp_synchronized)
        {
          param.systemResponseQueue->Dequeue(&response, 0);
          param.systemStatusLock->Take();
          param.system_status->connection.is_ntp_sync_ongoing = false;
          param.system_status->connection.is_ntp_synchronized = false;
          param.systemStatusLock->Give();
          TRACE_ERROR_F(F("%s NTP synchronization [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);

          TRACE_VERBOSE_F(F("SUPERVISOR_STATE_DO_NTP_SYNC -> SUPERVISOR_STATE_CHECK_OPERATION\r\n"));
          state = SUPERVISOR_STATE_CHECK_OPERATION;
        }
        // other
        else
        {
          Delay(Ticks::MsToTicks(SUPERVISOR_TASK_WAIT_DELAY_MS));
        }
      }
      // do something else with non-blocking wait ....
      break;

    case SUPERVISOR_STATE_END:
      TRACE_VERBOSE_F(F("SUPERVISOR_STATE_END -> SUPERVISOR_STATE_CHECK_OPERATION\r\n"));
      state = SUPERVISOR_STATE_CHECK_OPERATION;
      break;
    }
  }
}

bool SupervisorTask::loadConfiguration(configuration_t *configuration, BinarySemaphore *lock)
{
  bool status = true;

  //! read configuration from eeprom
  if (lock->Take())
  {
    // error = eeprom.Read(CONFIGURATION_EEPROM_ADDRESS, (uint8_t *)(configuration), sizeof(configuration_t));
    // for (size_t i = 0; i < sizeof(configuration_t); i++)
    // {
    //   uint8_t *ptr = (uint8_t *) configuration;
    //   status &= eeprom.Read(CONFIGURATION_EEPROM_ADDRESS, &ptr[i], sizeof(uint8_t));
    // }

    lock->Give();
  }

  if (configuration->module_type != MODULE_TYPE || configuration->module_main_version != MODULE_MAIN_VERSION)
  {
    status = saveConfiguration(configuration, lock, CONFIGURATION_DEFAULT);
  }

  TRACE_INFO_F(F("Load configuration... [ %s ]\r\n"), status ? OK_STRING : ERROR_STRING);
  printConfiguration(configuration, lock);

  return status;
}

void SupervisorTask::printConfiguration(configuration_t *configuration, BinarySemaphore *lock)
{
  if (lock->Take()) {
    char stima_name[20];
    char topic[MQTT_ROOT_TOPIC_LENGTH];

    getStimaNameByType(stima_name, configuration->module_type);
    TRACE_INFO_F(F("-> type: %s\r\n"), stima_name);
    TRACE_INFO_F(F("-> main version: %u\r\n"), configuration->module_main_version);
    TRACE_INFO_F(F("-> minor version: %u\r\n"), configuration->module_minor_version);
    TRACE_INFO_F(F("-> constant data: %d\r\n"), configuration->constantdata_count);
    
    for (uint8_t i = 0; i < configuration->constantdata_count; i++)
    {
      TRACE_INFO_F(F("--> cd %d:/t%s : %s"), i, configuration->constantdata[i].btable, configuration->constantdata[i].value);
    }

    TRACE_INFO_F(F("-> %u configured sensors:\r\n"), configuration->sensors_count);

    for (uint8_t i = 0; i < configuration->sensors_count; i++)
    {
      TRACE_INFO_F(F("--> %u: %s-%s\r\n"), i + 1, configuration->sensors[i].driver, configuration->sensors[i].type);
    }

    TRACE_INFO_F(F("-> data report every %d seconds\r\n"), configuration->report_s);
    TRACE_INFO_F(F("-> data observation every %d seconds\r\n"), configuration->observation_s);
    TRACE_INFO_F(F("-> data level: %s\r\n"), configuration->data_level);
    TRACE_INFO_F(F("-> ident: %s\r\n"), configuration->ident);
    TRACE_INFO_F(F("-> longitude %07d and latitude %07d\r\n"), configuration->longitude, configuration->latitude);
    TRACE_INFO_F(F("-> network: %s\r\n"), configuration->network);

    #if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_ETH)
    TRACE_INFO_F(F("-> dhcp: %s\r\n"), configuration->is_dhcp_enable ? "on" : "off");
    TRACE_INFO_F(F("-> ethernet mac: %02X:%02X:%02X:%02X:%02X:%02X\r\n"), configuration->ethernet_mac[0], configuration->ethernet_mac[1], configuration->ethernet_mac[2], configuration->ethernet_mac[3], configuration->ethernet_mac[4], configuration->ethernet_mac[5]);
    #endif

    #if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
    TRACE_INFO_F(F("-> gsm apn: %s\r\n"), configuration->gsm_apn);
    TRACE_INFO_F(F("-> gsm number: %s\r\n"), configuration->gsm_number);
    TRACE_INFO_F(F("-> gsm username: %s\r\n"), configuration->gsm_username);
    TRACE_INFO_F(F("-> gsm password: %s\r\n"), configuration->gsm_password);
    #endif

    #if (USE_NTP)
    TRACE_INFO_F(F("-> ntp server: %s\r\n"), configuration->ntp_server);
    #endif

    #if (USE_MQTT)
    TRACE_INFO_F(F("-> mqtt server: %s\r\n"), configuration->mqtt_server);
    TRACE_INFO_F(F("-> mqtt port: %d\r\n"), configuration->mqtt_port);    
    TRACE_INFO_F(F("-> mqtt username: %s\r\n"), configuration->mqtt_username);
    TRACE_INFO_F(F("-> mqtt password: %s\r\n"), configuration->mqtt_password);
    TRACE_INFO_F(F("-> station slug: %s\r\n"), configuration->stationslug);
    TRACE_INFO_F(F("-> board slug: %s\r\n"), configuration->boardslug);
    TRACE_INFO_F(F("-> client psk key "));
    TRACE_INFO_ARRAY("", configuration->client_psk_key, CLIENT_PSK_KEY_LENGTH);    
    TRACE_INFO_F(F("-> mqtt root topic: %d/%s/%s/%s/%07d,%07d/%s/\r\n"), RMAP_PROCOTOL_VERSION, configuration->data_level, configuration->mqtt_username, configuration->ident, configuration->longitude, configuration->latitude, configuration->network);
    TRACE_INFO_F(F("-> mqtt maint topic: %d/%s/%s/%s/%07d,%07d/%s/\r\n"), RMAP_PROCOTOL_VERSION, DATA_LEVEL_MAINT, configuration->mqtt_username, configuration->ident, configuration->longitude, configuration->latitude, configuration->network);
    TRACE_INFO_F(F("-> mqtt rpc topic: %d/%s/%s/%s/%07d,%07d/%s/\r\n"), RMAP_PROCOTOL_VERSION, DATA_LEVEL_RPC, configuration->mqtt_username, configuration->ident, configuration->longitude, configuration->latitude, configuration->network);
    #endif

    lock->Give();
  }
}

bool SupervisorTask::saveConfiguration(configuration_t *configuration, BinarySemaphore *lock, bool is_default)
{
  bool status = true;

  if (lock->Take())
  {
    if (is_default)
    {
      osMemset(configuration, 0, sizeof(configuration_t));

      configuration->module_main_version = MODULE_MAIN_VERSION;
      configuration->module_minor_version = MODULE_MINOR_VERSION;
      configuration->module_type = MODULE_TYPE;

      configuration->observation_s = CONFIGURATION_DEFAULT_OBSERVATION_S;
      configuration->report_s = CONFIGURATION_DEFAULT_REPORT_S;

      strSafeCopy(configuration->ident, CONFIGURATION_DEFAULT_IDENT, IDENT_LENGTH);
      strSafeCopy(configuration->data_level, CONFIGURATION_DEFAULT_DATA_LEVEL, DATA_LEVEL_LENGTH);
      strSafeCopy(configuration->network, CONFIGURATION_DEFAULT_NETWORK, NETWORK_LENGTH);

      configuration->latitude = CONFIGURATION_DEFAULT_LATITUDE;
      configuration->longitude = CONFIGURATION_DEFAULT_LONGITUDE;

      #if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_ETH)
      char temp_string[20];
      configuration->is_dhcp_enable = CONFIGURATION_DEFAULT_ETHERNET_DHCP_ENABLE;
      strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_MAC);
      macStringToArray(configuration->ethernet_mac, temp_string);
      strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_IP);
      ipStringToArray(configuration->ip, temp_string);
      strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_NETMASK);
      ipStringToArray(configuration->netmask, temp_string);
      strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_GATEWAY);
      ipStringToArray(configuration->gateway, temp_string);
      strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_PRIMARY_DNS);
      ipStringToArray(configuration->primary_dns, temp_string);
      #endif

      #if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
      strSafeCopy(configuration->gsm_apn, CONFIGURATION_DEFAULT_GSM_APN, GSM_APN_LENGTH);
      strSafeCopy(configuration->gsm_number, CONFIGURATION_DEFAULT_GSM_NUMBER, GSM_NUMBER_LENGTH);
      strSafeCopy(configuration->gsm_username, CONFIGURATION_DEFAULT_GSM_USERNAME, GSM_USERNAME_LENGTH);
      strSafeCopy(configuration->gsm_password, CONFIGURATION_DEFAULT_GSM_PASSWORD, GSM_PASSWORD_LENGTH);
      #endif

      #if (USE_NTP)
      strSafeCopy(configuration->ntp_server, CONFIGURATION_DEFAULT_NTP_SERVER, NTP_SERVER_LENGTH);
      #endif

      #if (USE_MQTT)
      configuration->mqtt_port = CONFIGURATION_DEFAULT_MQTT_PORT;
      strSafeCopy(configuration->mqtt_server, CONFIGURATION_DEFAULT_MQTT_SERVER, MQTT_SERVER_LENGTH);
      strSafeCopy(configuration->mqtt_username, CONFIGURATION_DEFAULT_MQTT_USERNAME, MQTT_USERNAME_LENGTH);
      strSafeCopy(configuration->mqtt_password, CONFIGURATION_DEFAULT_MQTT_PASSWORD, MQTT_PASSWORD_LENGTH);
      strSafeCopy(configuration->stationslug, CONFIGURATION_DEFAULT_STATIONSLUG, STATIONSLUG_LENGTH);
      strSafeCopy(configuration->boardslug, CONFIGURATION_DEFAULT_BOARDSLUG, BOARDSLUG_LENGTH);
      #endif

      // TODO: da rimuovere da qui in avanti
      #if (USE_MQTT)
      uint8_t temp_psk_key[] = {0x4F, 0x3E, 0x7E, 0x10, 0xD2, 0xD1, 0x6A, 0xE2, 0xC5, 0xAC, 0x60, 0x12, 0x0F, 0x07, 0xEF, 0xAF};
      osMemcpy(configuration->client_psk_key, temp_psk_key, CLIENT_PSK_KEY_LENGTH);

      strSafeCopy(configuration->mqtt_username, "userv4", MQTT_USERNAME_LENGTH);
      strSafeCopy(configuration->stationslug, "stimav4", STATIONSLUG_LENGTH);
      strSafeCopy(configuration->boardslug, "stima4", BOARDSLUG_LENGTH);
      #endif
    }

    //! write configuration to eeprom
    // error = eeprom.Write(CONFIGURATION_EEPROM_ADDRESS, (uint8_t *)(configuration), sizeof(configuration_t));

    // for (size_t i = 0; i < sizeof(configuration_t); i++)
    // {
    //   uint8_t *ptr = (uint8_t *) configuration;
    //   status &= eeprom.Write(CONFIGURATION_EEPROM_ADDRESS, &ptr[i], sizeof(uint8_t));
    // }

    if (is_default)
    {
      TRACE_INFO_F(F("Save default configuration... [ %s ]\r\n"), status ? OK_STRING : ERROR_STRING);
    }
    else
    {
      TRACE_INFO_F(F("Save configuration... [ %s ]\r\n"), status ? OK_STRING : ERROR_STRING);
    }

    lock->Give();
  }

  return status;
}