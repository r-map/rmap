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

/// @brief Constructor Class
RegisterRPC::RegisterRPC()
{
}
/// @brief Constructor Class with Param
/// @param rpcParam Parameter for Class
RegisterRPC::RegisterRPC(RpcParam_t rpcParam)
{
  param = rpcParam;
}

/// @brief Init RPC Class Object and Register method CallBack
/// @param streamRpc pointer to Object JsonRPC
void RegisterRPC::init(JsonRPC *streamRpc)
{
  streamRpc->init();

#if (USE_RPC_METHOD_ADMIN)
  streamRpc->registerMethod("admin", &admin);
#endif

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

#if (USE_RPC_METHOD_ADMIN)
/// @brief RPC CallBack of admin method
/// @param params JsonObject request
/// @param result JsonObject response
/// @return execute level error or ok
int RegisterRPC::admin(JsonObject params, JsonObject result)
{
  for (JsonPair it : params)
  {
    if (strcmp(it.key().c_str(), "fdownload") == 0)
    {
    }
  }
}
#endif

#if (USE_RPC_METHOD_CONFIGURE)
/// @brief RPC CallBack of configure method
///        isMasterConfigure Was set to true when configure a master_board. isSlaveConfigure alternately for slave_board
///        during the configuration the sequence of the parameters must be guaranteed and the detection of a non-compliance issues
///        an error and blocks the procedure by resetting the local configuration indexes
/// @param params JsonObject request
/// @param result JsonObject response
/// @return execute level error or ok
int RegisterRPC::configure(JsonObject params, JsonObject result)
{
  bool is_error = false;
  bool error_command = false;
  
  for (JsonPair it : params)
  {
    // ************** SHARED COMMAND CONFIGURATION LIST **************
    if (strcmp(it.key().c_str(), "reset") == 0)
    {
      if (it.value().as<bool>() == true)
      {
        // Reset pointed relative area configuration from RAM structure
        TRACE_INFO_F(F("RPC: DO RESET CONFIGURATION\r\n"));
        if(isSlaveConfigure) {
          // Parameter from slave also getted (serial number, to be copied before reset all node param)
          uint64_t remote_sn = param.configuration->board_slave[slaveId].serial_number;
          param.configurationLock->Take();
          memset(&param.configuration->board_slave[slaveId], 0, sizeof(param.configuration->board_slave[slaveId]));
          param.configuration->board_slave[slaveId].serial_number = remote_sn;
          param.configuration->board_slave[slaveId].module_type = currentModule;
          param.configurationLock->Give();
        }
        else if(isMasterConfigure)
        {
          // Reset board parameter only (parameter was enetered and modified from new config line)
          // Default base parameter will be add at end of configuration sequence
          param.configurationLock->Take();
          memset(&param.configuration->board_master, 0, sizeof(param.configuration->board_master));
          param.configurationLock->Give();
        }
        else error_command = true;
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "save") == 0)
    {
      if (it.value().as<bool>() == true)
      {
        TRACE_INFO_F(F("RPC: DO SAVE CONFIGURATION\r\n"));
        // Save only with last command (Master Config final) Preserve 8 x 1,5K Bytes Resave continue with EEProm
        // On method reboot if is called before Saving is possible saving by check cfg_modified flag;
        is_configuration_changed = true;
        // No more action here if save configuration.
        // TODO:Action to start configuration module before rebot or deinit remote node and restart PnP to Reset
        // Pnp Reset is need only if is changed node_id remote otherwise no more action required here
        if(isMasterConfigure) {
          //TODO: if node_id master <> node_old master... Reconfigure Node Id Slave !!!
          initFixedConfigurationParam();
          saveConfiguration();
          is_configuration_changed = false;
        } else if(isSlaveConfigure) {
          // With command on system_message queue Start configuration of remote module programmed
          system_message_t system_message = {0};
          system_message.task_dest = CAN_TASK_ID;
          system_message.command.do_remotecfg = true;
          system_message.param = slaveId;
          param.systemMessageQueue->Enqueue(&system_message, 0);
        }
        else error_command = true;
        // Reset and deinit info current module and parameter pointer configure sequence
        currentModule = Module_Type::undefined;
        slaveId = UNKNOWN_ID;
        sensorId = UNKNOWN_ID;
        isSlaveConfigure = false;
        isMasterConfigure = false;
        id_constant_data = 0;
        subject[0] = 0;
      }
      else error_command = true;
    }
    // ************** SHARED PARAMETER CONFIGURATION LIST **************
    // loop in params order by sequence in examples stimacan config github notification
    else if (strcmp(it.key().c_str(), "board") == 0)
    {
      // Respect the configure sequence only if configure module was terminated
      // Can start new parameter sequence command. Modify relative param only
      // when relative flag are UP (isMasterConfigure = true) accept master_command
      // otherwise refuse command. Also for module slave. Module slave have slaveId (Array Index)
      // is board stimacan (slave)
      if(!isMasterConfigure && !isSlaveConfigure) {
        if (strstr(it.value().as<const char *>(), "stimacan")) {
          char *str_pos;
          str_pos = strstr(it.value().as<const char *>(), "stimacan") + 8;
          uint8_t requestIndex = (uint8_t)atoi(str_pos) - 1;
          if(requestIndex < BOARDS_COUNT_MAX) {
            // Start configure slave module ID: slaveId            
            isSlaveConfigure = true;
            slaveId = requestIndex;
          }
          else error_command = true;
        }
        else if (strcmp(it.value().as<const char *>(), "stimav4") == 0) {
          // Start configure master module
          isMasterConfigure = true;
        }
        else is_error = true;
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "boardtype") == 0)
    {
      // Set Module TYPE (index) for Master o Slave module
      // Validate with sequence command
      if(isSlaveConfigure) {
        param.configurationLock->Take();
        param.configuration->board_slave[slaveId].module_type = (Module_Type)it.value().as<unsigned int>();
        currentModule = (Module_Type)it.value().as<unsigned int>();
        param.configurationLock->Give();
      }
      else if(isMasterConfigure) {
        param.configurationLock->Take();
        param.configuration->board_master.module_type = (Module_Type)it.value().as<unsigned int>();
        currentModule = (Module_Type)it.value().as<unsigned int>();
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "cansampletime") == 0)
    {
      // TODO:CAN_SAMPLE_TIME
      // can_sampletime (Time to auto publish data. Master future use)
      if(isSlaveConfigure) {
        param.configurationLock->Take();
        param.configuration->board_slave[slaveId].can_sampletime = it.value().as<unsigned int>();
        param.configurationLock->Give();
      }
      else if(isMasterConfigure) {
        // can_sampletime unused for Master at the moment
        param.configurationLock->Take();
        param.configuration->board_master.can_sampletime = it.value().as<unsigned int>();
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "node_id") == 0)
    {
      // TODO:unset registration node, register_node_new. Delete if ID <>. Nothing if ==
      if(isSlaveConfigure) {
        param.configurationLock->Take();
        param.configuration->board_slave[slaveId].can_address = it.value().as<unsigned int>();
        param.configurationLock->Give();
      }
      else if(isMasterConfigure) {
        // can_sampletime are porting to observation_s require
        param.configurationLock->Take();
        param.configuration->board_master.can_address = it.value().as<unsigned int>();
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "subject") == 0)
    {
      // Subject for next subject_id identification (at the moment check only type_name subject valid)
      // In future can assign more than one subject - subject_id for more service avaiable on CAN module
      if(isSlaveConfigure||isMasterConfigure) {
        strcpy(subject, it.value().as<const char *>());
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "subject_id") == 0)
    {
      // Check list from avaiable configured register subject/port for module_type
      // Enter value in apposite register and configure the remote register
      // isSlaveConfigure/isMasterConfigure unused when check currentModule (also resetted when error sequence occurs)
      switch (currentModule) {
        case Module_Type::th:
          if(strcmp(subject, "node.th") == 0) {
            param.configurationLock->Take();
            param.configuration->board_slave[slaveId].can_port_id = 50 + slaveId;
            param.configuration->board_slave[slaveId].can_publish_id = it.value().as<unsigned int>();
            param.configurationLock->Give();
          }
          else error_command = true;
          break;
        case Module_Type::rain:
          if(strcmp(subject, "node.p") == 0) {
            param.configurationLock->Take();
            param.configuration->board_slave[slaveId].can_port_id = 50 + slaveId;
            param.configuration->board_slave[slaveId].can_publish_id = it.value().as<unsigned int>();
            param.configurationLock->Give();
          }
          else error_command = true;
          break;
        case Module_Type::wind:
          if(strcmp(subject, "node.wind") == 0) {
            param.configurationLock->Take();
            param.configuration->board_slave[slaveId].can_port_id = 50 + slaveId;
            param.configuration->board_slave[slaveId].can_publish_id = it.value().as<unsigned int>();
            param.configurationLock->Give();
          }
          else error_command = true;
          break;
        case Module_Type::radiation:
          if(strcmp(subject, "node.rad") == 0) {
            param.configurationLock->Take();
            param.configuration->board_slave[slaveId].can_port_id = 50 + slaveId;
            param.configuration->board_slave[slaveId].can_publish_id = it.value().as<unsigned int>();
            param.configurationLock->Give();
          }
          else error_command = true;
          break;
        case Module_Type::power:
          if(strcmp(subject, "node.pwr") == 0) {
            param.configurationLock->Take();
            param.configuration->board_slave[slaveId].can_port_id = 50 + slaveId;
            param.configuration->board_slave[slaveId].can_publish_id = it.value().as<unsigned int>();
            param.configurationLock->Give();
          }
          else error_command = true;
          break;
        case Module_Type::vwc:
          if(strcmp(subject, "node.vwc") == 0) {
            param.configurationLock->Take();
            param.configuration->board_slave[slaveId].can_port_id = 50 + slaveId;
            param.configuration->board_slave[slaveId].can_publish_id = it.value().as<unsigned int>();
            param.configurationLock->Give();
          }
          else error_command = true;
          break;
        case Module_Type::server_gsm:
        case Module_Type::server_eth:
          if(strcmp(subject, "node.master") == 0) {
            param.configurationLock->Take();
            param.configuration->board_slave[slaveId].can_port_id = 100;
            param.configuration->board_slave[slaveId].can_publish_id = it.value().as<unsigned int>();
            param.configurationLock->Give();
          }
          else error_command = true;
          break;
        default:
          error_command = true;
      }
    }
    // ************** PRINCIPAL SLAVE PARAMETER LIST **************
    else if (strcmp(it.key().c_str(), "sn") == 0)
    {
      // Set Serial Number (from Hex ASCII) only for Slave module
      // Validate with sequence command
      if(isSlaveConfigure) {
        const char *ptr_read = it.value().as<const char *>() + 2; // Point to SN String 0x->
        uint64_t sn_read = 0;
        bool end_conversion = false;
        uint8_t byte_pos = 7;
        uint8_t data_read;
        // Read all HexASCII (2Char for each Time) and Put into (serial_number) at power Byte byte_pos
        // Start from MSB to LSB. Terminate if All Byte expected was read or Error Char into Input String
        // Or Input String is terminated. Each character !" HEX_TIPE (0..9,A..F) terminate function
        // Hex string can be shorter than expected. Value are convert as UINT_64 MSB Left Formatted
        while(byte_pos && !end_conversion) {
          end_conversion = ASCIIHexToDecimal((char**)&ptr_read, &data_read);
          sn_read |= ((uint64_t)data_read)<<(8*byte_pos--);
        }
        param.configurationLock->Take();
        param.configuration->board_slave[slaveId].serial_number = sn_read;
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "driver") == 0)
    {
      // Start Configure a local CAN sensor, close opened sesnorIndex Array ( Only for Slave Module )
      // Next Start "type"... verified with starting sensorId = SETUP_ID for correct sequence command
      if(isSlaveConfigure) {
        if (strcmp(it.value().as<const char *>(), "CAN") == 0) {
          sensorId = SETUP_ID;
        }
        else error_command = true;
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "type") == 0)
    {
      // Start Configure a local CAN sensor, Open First/Next sesnorIndex Array ( Only for Slave Module )
      // starting "type"... verified with starting sensorId = SETUP_ID for correct sequence command
      if(sensorId==SETUP_ID) {
        switch (currentModule) {
          case Module_Type::th:
            if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_STH) == 0) {
              sensorId = SENSOR_METADATA_STH;
            } else if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_ITH) == 0) {
              sensorId = SENSOR_METADATA_ITH;
            } else if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_MTH) == 0) {
              sensorId = SENSOR_METADATA_MTH;
            } else if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_NTH) == 0) {
              sensorId = SENSOR_METADATA_NTH;
            } else if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_XTH) == 0) {
              sensorId = SENSOR_METADATA_XTH;
            }
            else error_command = true;
            break;
          case Module_Type::rain:
            if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_TBR) == 0) {
              sensorId = SENSOR_METADATA_TBR;
            }
            else error_command = true;
            break;
          case Module_Type::wind:
            if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_DWA) == 0) {
              sensorId = SENSOR_METADATA_DWA;
            } else if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_DWB) == 0) {
              sensorId = SENSOR_METADATA_DWB;
            } else if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_DWC) == 0) {
              sensorId = SENSOR_METADATA_DWC;
            } else if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_DWD) == 0) {
              sensorId = SENSOR_METADATA_DWD;
            } else if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_DWE) == 0) {
              sensorId = SENSOR_METADATA_DWE;
            } else if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_DWF) == 0) {
              sensorId = SENSOR_METADATA_DWF;
            }
            else error_command = true;
            break;
          case Module_Type::radiation:
            if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_DSA) == 0) {
              sensorId = SENSOR_METADATA_DSA;
            }
            else error_command = true;
            break;
          case Module_Type::power:
            if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_DEP) == 0) {
              sensorId = SENSOR_METADATA_DEP;
            }
            else error_command = true;
            break;
          case Module_Type::vwc:
            if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_VWC) == 0) {
              sensorId = SENSOR_METADATA_VWC;
            }
            else error_command = true;
            break;
          default:
            error_command = true;
        }
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "timerange") == 0)
    {
      // Pindicator - P1 - P2 it.value().as<JsonArray>()[0,1,2].as<unsigned int>()
      // it.value().as<JsonArray>()[0].as<unsigned int>()
      // Coorect sequence only is here sensorID have real value index
      if((sensorId<SETUP_ID)&&(isSlaveConfigure)) {
        param.configurationLock->Take();
        param.configuration->board_slave[slaveId].is_configured[sensorId] = true;

        param.configuration->board_slave[slaveId].metadata[sensorId].timerangePindicator =
          it.value().as<JsonArray>()[0].as<unsigned int>();
        param.configuration->board_slave[slaveId].metadata[sensorId].timerangeP1 =
          it.value().as<JsonArray>()[1].as<unsigned int>();
        param.configuration->board_slave[slaveId].metadata[sensorId].timerangeP2 =
          it.value().as<JsonArray>()[2].as<unsigned int>();
        // Duplicate ITH into STH Only for TH Module (Param not send in config)
        if((currentModule == Module_Type::th) && (sensorId ==SENSOR_METADATA_ITH)) {
          param.configuration->board_slave[slaveId].metadata[SENSOR_METADATA_STH].timerangePindicator =
            it.value().as<JsonArray>()[0].as<unsigned int>();
          param.configuration->board_slave[slaveId].metadata[SENSOR_METADATA_STH].timerangeP1 =
            it.value().as<JsonArray>()[1].as<unsigned int>();
          param.configuration->board_slave[slaveId].metadata[SENSOR_METADATA_STH].timerangeP2 =
            it.value().as<JsonArray>()[2].as<unsigned int>();
        }
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "level") == 0)
    {
      // LevelType1, L1, LevelType2, L2
      // it.value().as<JsonArray>()[0,1,2,3].as<unsigned int>()
      // need to be checked if it is null value. if null a UINT16_MAX must be assigned
      // Coorect sequence only is here sensorID have real value index
      if((sensorId<SETUP_ID)&&(isSlaveConfigure)) {
        param.configurationLock->Take();

        if (it.value().as<JsonArray>()[0].isNull())
        {
          param.configuration->board_slave[slaveId].metadata[sensorId].levelType1 = UINT16_MAX;
        }
        else
        {
          param.configuration->board_slave[slaveId].metadata[sensorId].levelType1 =
              it.value().as<JsonArray>()[0].as<unsigned int>();
        }

        if (it.value().as<JsonArray>()[1].isNull())
        {
          param.configuration->board_slave[slaveId].metadata[sensorId].level1 = UINT16_MAX;
        }
        else
        {
          param.configuration->board_slave[slaveId].metadata[sensorId].level1 =
              it.value().as<JsonArray>()[1].as<unsigned int>();
        }

        if (it.value().as<JsonArray>()[2].isNull())
        {
          param.configuration->board_slave[slaveId].metadata[sensorId].levelType2 = UINT16_MAX;
        }
        else
        {
          param.configuration->board_slave[slaveId].metadata[sensorId].levelType2 =
              it.value().as<JsonArray>()[2].as<unsigned int>();
        }

        if (it.value().as<JsonArray>()[3].isNull())
        {
          param.configuration->board_slave[slaveId].metadata[sensorId].level2 = UINT16_MAX;
        }
        else
        {
          param.configuration->board_slave[slaveId].metadata[sensorId].level2 =
              it.value().as<JsonArray>()[3].as<unsigned int>();
        }

        // Duplicate ITH into STH Only for TH Module (Param not send in config)
        if((currentModule == Module_Type::th) && (sensorId ==SENSOR_METADATA_ITH)) {
          param.configuration->board_slave[slaveId].metadata[SENSOR_METADATA_STH].levelType1 =
              param.configuration->board_slave[slaveId].metadata[sensorId].levelType1;
          param.configuration->board_slave[slaveId].metadata[SENSOR_METADATA_STH].level1 =
              param.configuration->board_slave[slaveId].metadata[sensorId].level1;
          param.configuration->board_slave[slaveId].metadata[SENSOR_METADATA_STH].levelType2 =
              param.configuration->board_slave[slaveId].metadata[sensorId].levelType2;
          param.configuration->board_slave[slaveId].metadata[SENSOR_METADATA_STH].level2 =
              param.configuration->board_slave[slaveId].metadata[sensorId].level2;
        }
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    // ************** PRINCIPAL MASTER PARAMETER LIST **************
    else if (strcmp(it.key().c_str(), "sd") == 0)
    {
      if(isMasterConfigure) {        
        for (JsonPair sd : it.value().as<JsonObject>())
        {
          param.configurationLock->Take();
          strcpy(param.configuration->constantdata[id_constant_data].btable, sd.key().c_str());
          strcpy(param.configuration->constantdata[id_constant_data].value, sd.value().as<const char *>());
          param.configurationLock->Give();
          if(++id_constant_data >= USE_CONSTANTDATA_COUNT) {
            is_error = true;
            break;
          }
        }
        if(!is_error) {
          param.configurationLock->Take();
          param.configuration->constantdata_count = id_constant_data;
          param.configurationLock->Give();
        }
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "stationslug") == 0)
    {
      if(isMasterConfigure) {
        param.configurationLock->Take();
        strcpy(param.configuration->stationslug, it.value().as<const char *>()); 
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "boardslug") == 0)
    {
      if(isMasterConfigure) {
        param.configurationLock->Take();
        strcpy(param.configuration->boardslug, it.value().as<const char *>()); 
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "lon") == 0)
    {
      if(isMasterConfigure) {
        param.configurationLock->Take();
        param.configuration->longitude = it.value().as<long int>(); 
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "lat") == 0)
    {
      if(isMasterConfigure) {
        param.configurationLock->Take();
        param.configuration->latitude = it.value().as<long int>(); 
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "network") == 0)
    {
      if(isMasterConfigure) {
        param.configurationLock->Take();
        strcpy(param.configuration->network, it.value().as<const char *>()); 
        param.configurationLock->Give();
      }
      else error_command = true;
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
      if(isMasterConfigure) {
        param.configurationLock->Take();
        strcpy(param.configuration->mqtt_root_topic, it.value().as<const char *>()); 
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "mqttrpcpath") == 0)
    {
      // mqtt_rpc_topic
      if(isMasterConfigure) {
        param.configurationLock->Take();
        strcpy(param.configuration->mqtt_rpc_topic, it.value().as<const char *>()); 
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "mqttmaintpath") == 0)
    {
      // mqtt_maint_topic
      if(isMasterConfigure) {
        param.configurationLock->Take();
        strcpy(param.configuration->mqtt_maint_topic, it.value().as<const char *>()); 
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "mqttserver") == 0)
    {
      // mqtt_server
      if(isMasterConfigure) {
        param.configurationLock->Take();
        strcpy(param.configuration->mqtt_server, it.value().as<const char *>()); 
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "mqttsampletime") == 0)
    {
      // report_s
      if(isMasterConfigure) {
        param.configurationLock->Take();
        param.configuration->report_s = it.value().as<unsigned int>();
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "mqttuser") == 0)
    {
      // mqtt_username
      if(isMasterConfigure) {
        param.configurationLock->Take();
        strcpy(param.configuration->mqtt_username, it.value().as<const char *>()); 
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "mqttpassword") == 0)
    {
      // mqtt_password
      if(isMasterConfigure) {
        param.configurationLock->Take();
        strcpy(param.configuration->mqtt_password, it.value().as<const char *>()); 
        param.configurationLock->Give();
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "mqttpskkey") == 0)
    {
      // client_psk_key
      if(isMasterConfigure) {
        uint8_t byte_pos = 0;
        const char *ptr_read = it.value().as<const char *>() + 2; // Point to PSK_KEY String 0x->
        bool end_conversion = false;
        uint8_t data_read;
        // Read all HexASCII (2Char for each Time) and Put into (serial_number) at power Byte byte_pos
        // Start from MSB to LSB. Terminate if All Byte expected was read or Error Char into Input String
        // Or Input String is terminated. Each character !" HEX_TIPE (0..9,A..F) terminate function
        // Hex string can be shorter than expected. Value are convert as UINT_64 MSB Left Formatted
        param.configurationLock->Take();
        // Reset PSK_KEY
        memcpy(param.configuration->client_psk_key, 0, CLIENT_PSK_KEY_LENGTH);
        while((byte_pos!=CLIENT_PSK_KEY_LENGTH) && !end_conversion) {
          end_conversion = ASCIIHexToDecimal((char**)&ptr_read, &data_read);
          param.configuration->client_psk_key[byte_pos++] = data_read;
        }
        param.configurationLock->Give();
      }
      else error_command = true;
    }
#endif
#if (USE_NTP)
    else if (strcmp(it.key().c_str(), "ntpserver") == 0)
    {
      // ntp_server
      if(isMasterConfigure) {
        strcpy(param.configuration->ntp_server, it.value().as<const char *>());
      }
      else error_command = true;
    }
#endif
#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
    else if (strcmp(it.key().c_str(), "gsmapn") == 0)
    {
      // gsm_apn
      if(isMasterConfigure) {
        strcpy(param.configuration->gsm_apn, it.value().as<const char *>());
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "pppnumber") == 0)
    {
      // gsm_number
      if(isMasterConfigure) {
        strcpy(param.configuration->gsm_number, it.value().as<const char *>());
      }
      else error_command = true;
    }
#endif
    else
    {
      is_error = true;
    }
  }

  // when is_sensor_config = true a sensor was configured, then the index sensors_count must be incremented
  // in order to configure the next sensor
  // if (is_sensor_config)
  // {
    // if (writable_configuration.sensors_count < SENSORS_MAX)
    // {
    //   // writable_configuration.sensors_count++;
    // }
    // else
    // {
    //   is_error = true;
    // }
  // }

  // error_command = Out of command context but command request valid
  // is_error = error command or out of limit parameter
  if ((error_command)||(is_error))
  {
    // Deinit index configuration varaibles
    currentModule = Module_Type::undefined;
    slaveId = UNKNOWN_ID;
    sensorId = UNKNOWN_ID;
    isSlaveConfigure = false;
    isMasterConfigure = false;
    id_constant_data = 0;
    subject[0] = 0;
    // Result an error
    result[F("state")] = F("error");
    if(error_command) return E_INVALID_REQUEST;
    if(is_error) return E_INVALID_PARAMS;
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

  TRACE_INFO_F(F("RPC: Request Reboot\r\n"));
  result[F("state")] = "done";

  #if (ENABLE_RPC_LOCAL_REBOOT)
  // Start REBOOT with queue command
  system_message_t system_message = {0};
  system_message.task_dest = CAN_TASK_ID;
  system_message.command.do_reboot = true;
  param.systemMessageQueue->Enqueue(&system_message, 0);
  #endif

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

/// @brief Init configuration fixed param unused on RPC Connfiguration
/// @param  None
void RegisterRPC::initFixedConfigurationParam(void)
{
  // Private param and Semaphore: param.configuration, param.configurationLock
  param.configurationLock->Take();

  param.configuration->module_main_version = MODULE_MAIN_VERSION;
  param.configuration->module_minor_version = MODULE_MINOR_VERSION;
  param.configuration->module_type = (Module_Type)MODULE_TYPE;

  param.configuration->observation_s = CONFIGURATION_DEFAULT_OBSERVATION_S;

  strSafeCopy(param.configuration->ident, CONFIGURATION_DEFAULT_IDENT, IDENT_LENGTH);

  #if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_ETH)
  char temp_string[20];
  param.configuration->is_dhcp_enable = CONFIGURATION_DEFAULT_ETHERNET_DHCP_ENABLE;
  strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_MAC);
  macStringToArray(param.configuration->ethernet_mac, temp_string);
  strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_IP);
  ipStringToArray(param.configuration->ip, temp_string);
  strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_NETMASK);
  ipStringToArray(param.configuration->netmask, temp_string);
  strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_GATEWAY);
  ipStringToArray(param.configuration->gateway, temp_string);
  strcpy(temp_string, CONFIGURATION_DEFAULT_ETHERNET_PRIMARY_DNS);
  ipStringToArray(param.configuration->primary_dns, temp_string);
  #endif

  #if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
  strSafeCopy(param.configuration->gsm_username, CONFIGURATION_DEFAULT_GSM_USERNAME, GSM_USERNAME_LENGTH);
  strSafeCopy(param.configuration->gsm_password, CONFIGURATION_DEFAULT_GSM_PASSWORD, GSM_PASSWORD_LENGTH);
  #endif

  param.configuration->board_master.serial_number = StimaV4GetSerialNumber();

  param.configurationLock->Give();
}

/// @brief Save configuration to E2
/// @param None
/// @return true is saving is done
bool RegisterRPC::saveConfiguration(void)
{
  // Private param and Semaphore: param.configuration, param.configurationLock
  bool status = true;

  if (param.configurationLock->Take())
  {
    //! write configuration to eeprom
    status = param.eeprom->Write(CONFIGURATION_EEPROM_ADDRESS, (uint8_t *)(param.configuration), sizeof(configuration_t));
    TRACE_INFO_F(F("Save configuration... [ %s ]\r\n"), status ? OK_STRING : ERROR_STRING);
    param.configurationLock->Give();
  }

  return status;
}

/// @brief Convert ASCII Hex 2 Format CHAR to uint8 value and increment string pointer to long string conversion (with error check)
/// @param str pointer to string (will be incremented if hex char are found and converted)
/// @param value_out pointer to data return value converted
/// @return true if error occurs, false if conversion is ready
bool RegisterRPC::ASCIIHexToDecimal(char** str, uint8_t *value_out) {
  bool is_error = false;

  if(isxdigit(**str)) {
    if(isdigit(**str)) {
      *value_out = **str - 48;
    } else {
      if(isupper(**str)) {
        *value_out = **str - 55;
      } else {
        *value_out = **str - 87;
      }
    }
    // Valid OK, Increment Char pointer
    *value_out<<=4;
    (*str)++;
  } else is_error=true;

  if(!is_error && isxdigit(**str)) {
    if(isdigit(**str)) {
      *value_out += **str - 48;
    } else {
      if(isupper(**str)) {
        *value_out += **str - 55;
      } else {
        *value_out += **str - 87;
      }
    }
    // Valid OK, Increment Char pointer
    (*str)++;
  } else is_error = true;

	return is_error;
}
