/**
  ******************************************************************************
  * @file    rpc_class.cpp
  * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   RPC Object Class for register RPC function, CallBack and manage data
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
  * <h2><center>All rights reserved.</center></h2>
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

#if (USE_RPC_METHOD_UPDATE)
  streamRpc->registerMethod("update", &update);
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

#if (USE_RPC_METHOD_RECOVERY)
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
  bool error_command = true;
  for (JsonPair it : params)
  {
    if (strcmp(it.key().c_str(), "fdownload") == 0)
    {
      error_command = false;
      // download all new firmwares for all of the boards
      if (it.value().as<bool>() == true)
      {
        // Starting queue request reinit structure firmware upgradable (and start download...)
        // And waiting response. After command structure firmware are resetted (after download new check for valid firmware ready)
        system_message_t system_message = {0};
        system_message.task_dest = SD_TASK_ID;
        system_message.command.do_reinit_fw = true;
        system_message.param = CMD_PARAM_REQUIRE_RESPONSE;
        param.systemMessageQueue->Enqueue(&system_message);
        // Waiting a response done before continue (reinit firmware OK!!!)
        while(true) {
          // Continuos Switching context non blocking
          // Need Waiting Task for start command on All used TASK
          taskYIELD();
          vTaskDelay(100);
          // Check response done
          if(!param.systemMessageQueue->IsEmpty()) {
            param.systemMessageQueue->Peek(&system_message);
            if(system_message.command.done_reinit_fw) {
              // Remove message (Reinit Done is OK)
              param.systemMessageQueue->Dequeue(&system_message);
              break;
            }
          }
        }
        TRACE_INFO_F(F("RPC: DO DOWNLOAD FIRMWARE\r\n"));
        // Start command sequnce for download module firmware
        param.systemStatusLock->Take();
        param.system_status->command.do_http_firmware_download = true;
        param.systemStatusLock->Give();
      }
    }
    else if (strcmp(it.key().c_str(), "cdownload") == 0)
    {
      error_command = false;
      // download lastes configuration from http command after configurate TLS Key from serial
      if (it.value().as<bool>() == true)
      {
        TRACE_INFO_F(F("RPC: DO DOWNLOAD CONFIGURATION\r\n"));
        // Start command sequnce for download module firmware
        param.systemStatusLock->Take();
        param.system_status->command.do_http_configuration_update = true;
        param.systemStatusLock->Give();
      }
    }
    else if (strcmp(it.key().c_str(), "sdinit") == 0)
    {
      error_command = false;
      // download lastes configuration from http command after configurate TLS Key from serial
      if (it.value().as<bool>() == true)
      {
        // Starting queue request truncate structure data on SD Card (Remote request)
        system_message_t system_message = {0};
        system_message.task_dest = SD_TASK_ID;
        system_message.command.do_trunc_sd = true;
        system_message.param = CMD_PARAM_REQUIRE_RESPONSE;
        param.systemMessageQueue->Enqueue(&system_message);
        // Waiting a response done before continue (reinit SD Data OK!!!)
        while(true) {
          // Continuos Switching context non blocking
          // Need Waiting Task for start command on All used TASK
          taskYIELD();
          vTaskDelay(100);
          // Check response done
          if(!param.systemMessageQueue->IsEmpty()) {
            param.systemMessageQueue->Peek(&system_message);
            if(system_message.command.done_trunc_sd) {
              // Remove message (Reinit Done is OK)
              param.systemMessageQueue->Dequeue(&system_message);
              break;
            }
          }
        }
        TRACE_INFO_F(F("RPC: DO INIT SD CARD DATA\r\n"));
      }
    }
    else if (strcmp(it.key().c_str(), "reginit") == 0)
    {
      error_command = false;
      uint8_t node_id_rpc = it.value().as<unsigned int>();
      // Starting queue request direct command remote Node over Cyphal
      if(param.configuration->board_slave[node_id_rpc].module_type != Module_Type::undefined) {        
        system_message_t system_message = {0};
        system_message.task_dest = CAN_TASK_ID;
        system_message.command.do_factory = true;
        // Parameter is Node Slave ID (Command destination Node Id)
        system_message.node_id = (Module_Type)node_id_rpc;
        param.systemMessageQueue->Enqueue(&system_message);
        TRACE_INFO_F(F("RPC: DO FACTORY RESET ON NODE ID:%d\r\n"), system_message.node_id);
      } else {
        TRACE_INFO_F(F("RPC: ERROR DO FACTORY ON UNCONFIGURED NODE\r\n"));
        error_command = true;
      }
    }
    else if (strcmp(it.key().c_str(), "pgcalib") == 0)
    {
      error_command = false;
      uint8_t node_id_rpc = it.value().as<unsigned int>();
      // Starting queue request direct command remote Node over Cyphal
      if(param.configuration->board_slave[node_id_rpc].module_type != Module_Type::undefined) {        
        system_message_t system_message = {0};
        system_message.task_dest = CAN_TASK_ID;
        system_message.command.do_calib_acc = true;
        // Parameter is Node Slave ID (Command destination Node Id)
        system_message.node_id = (Module_Type)node_id_rpc;
        param.systemMessageQueue->Enqueue(&system_message);
        TRACE_INFO_F(F("RPC: DO CALIBRATE ACCELEROMETER ON NODE ID:%d\r\n"), system_message.node_id);
      } else {
        TRACE_INFO_F(F("RPC: ERROR CALIBRATE ACCELEROMETER ON UNCONFIGURED NODE\r\n"));
        error_command = true;
      }
    }
    else if (strcmp(it.key().c_str(), "reboot") == 0)
    {
      error_command = false;
      uint8_t node_id_rpc = it.value().as<unsigned int>();
      // Starting queue request direct command remote Node over Cyphal
      if(param.configuration->board_slave[node_id_rpc].module_type != Module_Type::undefined) {        
        // Starting queue request direct commend reboot remote Node over Cyphal
        system_message_t system_message = {0};
        system_message.task_dest = CAN_TASK_ID;
        system_message.command.do_reboot_node = true;
        // Parameter is Node Slave ID (Command destination Node Id)
        system_message.node_id = (Module_Type)node_id_rpc;
        param.systemMessageQueue->Enqueue(&system_message);
        TRACE_INFO_F(F("RPC: DO DIRECT REMOTE REBOOT ON NODE ID:%d\r\n"), system_message.node_id);
      } else {
        TRACE_INFO_F(F("RPC: ERROR REMOTE REBOOT ON UNCONFIGURED NODE\r\n"));
        error_command = true;
      }
    }
    else if (strcmp(it.key().c_str(), "rstflags") == 0)
    {
      error_command = false;
      uint8_t node_id_rpc = it.value().as<unsigned int>();
      if(node_id_rpc == CMD_PARAM_MASTER_ADDRESS) {
          // Set the request on system status to reset flags
          param.systemStatusLock->Take();
          param.system_status->modem.connection_attempted = 0;
          param.system_status->modem.connection_completed = 0;
          param.system_status->modem.perc_modem_connection_valid = 100;
          param.system_status->connection.mqtt_data_exit_error = 0;
          param.systemStatusLock->Give();
          if(param.boot_request->tot_reset || param.boot_request->wdt_reset) {
            // Reset counter on new or restored firmware
            param.boot_request->tot_reset = 0;
            param.boot_request->wdt_reset = 0;
            // Save info bootloader block
            param.eeprom->Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*)param.boot_request, sizeof(bootloader_t));
          }
          TRACE_INFO_F(F("RPC: DO RESET LOCAL FLAGS ON MASTER\r\n"));
      } else {
        // Starting queue request direct command remote Node over Cyphal
        if(param.configuration->board_slave[node_id_rpc].module_type != Module_Type::undefined) {        
          system_message_t system_message = {0};
          system_message.task_dest = CAN_TASK_ID;
          system_message.command.do_reset_flags = true;
          // Parameter is Node Slave ID (Command destination Node Id)
          system_message.node_id = (Module_Type)node_id_rpc;
          param.systemMessageQueue->Enqueue(&system_message);
          TRACE_INFO_F(F("RPC: DO RESET REMOTE FLAGS ON NODE ID:%d\r\n"), system_message.node_id);
        } else {
          TRACE_INFO_F(F("RPC: ERROR RESET REMOTE FLAGS ON UNCONFIGURED NODE\r\n"));
          error_command = true;
        }
      }
    }
  }

  // error_command = Out of command context but command request valid
  // is_error = error command or out of limit parameter
  if (error_command)
  {
    // Result an error
    result[F("state")] = F("error");
    return E_INVALID_REQUEST;
  }
  else
  {
    result[F("state")] = F("done");
    return E_SUCCESS;
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
          param.configurationLock->Take();
          memset(&param.configuration->board_slave[slaveId], 0, sizeof(param.configuration->board_slave[slaveId]));
          // Reassign old valid parameter
          param.configuration->board_slave[slaveId].serial_number = boardSN;
          param.configuration->board_slave[slaveId].module_type = currentModule;
          strcpy(param.configuration->board_slave[slaveId].boardslug, boardName);
          param.configurationLock->Give();
        }
        // For Master board (boardname) is same of boardslug parameter IN with config.
        // BoardSlug (entered after this parameter) is prioritary. Oterwise boardname become boardslug
        else if(isMasterConfigure)
        {
          // Reset board parameter only (parameter was enetered and modified from new config line)
          // Default base parameter will be add at end of configuration sequence
          // Init param size is necessary
          param.configurationLock->Take();
          memset(&param.configuration->board_master, 0, sizeof(param.configuration->board_master));
          strcpy(param.configuration->board_master.boardslug, boardName);
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
        if(isMasterConfigure) {
          // Reset slaveID Index to End Master MAX Node
          initFixedConfigurationParam(slaveId + 1);
          slaveId = UNKNOWN_ID;
          saveConfiguration();
          is_configuration_changed = false;
        } else if(isSlaveConfigure) {
          // With command on system_message queue Start configuration of remote module programmed
          system_message_t system_message = {0};
          system_message.task_dest = CAN_TASK_ID;
          system_message.command.do_remote_cfg = true;
          // Parameter is Node Slave ID (Command destination Node Id)
          system_message.node_id = slaveId;
          param.systemMessageQueue->Enqueue(&system_message, 0);
        }
        else error_command = true;
        // Reset and deinit info current module and parameter pointer configure sequence
        currentModule = Module_Type::undefined;
        sensorId = UNKNOWN_ID;
        sensorMultyId = UNKNOWN_ID;
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
        // Copy board Name (Master slave depending after reading board_type def)
        memset(boardName, 0, sizeof(boardName));
        strcpy(boardName,(it.value().as<const char *>())); 
      }
      else error_command = true;
    }
    else if (strcmp(it.key().c_str(), "boardtype") == 0)
    {
      // Set Module TYPE (index) for Master o Slave module
      // Validate with sequence command if not already in cfg of one module
      if(!isMasterConfigure && !isSlaveConfigure) {
        switch(it.value().as<unsigned int>()) {
          case STIMA_MODULE_TYPE_MASTER_ETH:
          case STIMA_MODULE_TYPE_MASTER_GSM:
            // Saving param after command ..."reset"
            currentModule = (Module_Type)it.value().as<unsigned int>();
            isMasterConfigure = true;
            break;

          // First configure sensor with multi sensor on same module (sensorMultyId => SETUP_ID Mode)
          // N.B. Don't use break!!!
          case STIMA_MODULE_TYPE_VWC:
            sensorMultyId = SETUP_ID;
          case STIMA_MODULE_TYPE_RAIN:
          case STIMA_MODULE_TYPE_TH:
          case STIMA_MODULE_TYPE_WIND:
          case STIMA_MODULE_TYPE_SOLAR_RADIATION:
          case STIMA_MODULE_TYPE_POWER_MPPT:
            // Set module index (defualt START from 0xFF to point 0 index at start)
            // Index valid for all parameter while next board configure... Inc to sequential value
            // Saving param after command ..."reset"
            slaveId++;
            currentModule = (Module_Type)it.value().as<unsigned int>();
            isSlaveConfigure = true;
            break;

          default:
            error_command = true;
        }
      } else {
        error_command = true;
      }
    }
    else if (strcmp(it.key().c_str(), "cansampletime") == 0)
    {
      // can_sampletime (Time to auto publish data. Master only future use)
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
      if(isSlaveConfigure) {
        param.configurationLock->Take();
        param.configuration->board_slave[slaveId].can_address = it.value().as<unsigned int>();
        if(param.configuration->board_slave[slaveId].can_address >= 127) {
          // Fix Error configure from Address invalid
          param.configuration->board_slave[slaveId].can_address = 80 + slaveId;
        }
        switch(currentModule) {
          case STIMA_MODULE_TYPE_TH:
            param.configuration->board_slave[slaveId].can_port_id = PORT_RMAP_TH;
            break;
          case STIMA_MODULE_TYPE_RAIN:
            param.configuration->board_slave[slaveId].can_port_id = PORT_RMAP_RAIN;
            break;
          case STIMA_MODULE_TYPE_WIND:
            param.configuration->board_slave[slaveId].can_port_id = PORT_RMAP_WIND;
            break;
          case STIMA_MODULE_TYPE_SOLAR_RADIATION:
            param.configuration->board_slave[slaveId].can_port_id = PORT_RMAP_RADIATION;
            break;
          case STIMA_MODULE_TYPE_POWER_MPPT:
            param.configuration->board_slave[slaveId].can_port_id = PORT_RMAP_MPPT;
            break;
          case STIMA_MODULE_TYPE_VWC:
            param.configuration->board_slave[slaveId].can_port_id = PORT_RMAP_VWC;
            break;
        }
        param.configurationLock->Give();
      }
      else if(isMasterConfigure) {
        param.configurationLock->Take();
        #ifndef USE_NODE_MASTER_ID_FIXED
        param.configuration->board_master.can_address = it.value().as<unsigned int>();
        #else
        param.configuration->board_master.can_address = NODE_MASTER_ID;
        #endif
        param.configuration->board_master.can_port_id = PORT_SERVICE_RMAP;
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
            param.configuration->board_slave[slaveId].can_publish_id = it.value().as<unsigned int>();
            param.configurationLock->Give();
          }
          else error_command = true;
          break;
        case Module_Type::rain:
          if(strcmp(subject, "node.p") == 0) {
            param.configurationLock->Take();
            param.configuration->board_slave[slaveId].can_publish_id = it.value().as<unsigned int>();
            param.configurationLock->Give();
          }
          else error_command = true;
          break;
        case Module_Type::wind:
          if(strcmp(subject, "node.wind") == 0) {
            param.configurationLock->Take();
            param.configuration->board_slave[slaveId].can_publish_id = it.value().as<unsigned int>();
            param.configurationLock->Give();
          }
          else error_command = true;
          break;
        case Module_Type::radiation:
          if(strcmp(subject, "node.rad") == 0) {
            param.configurationLock->Take();
            param.configuration->board_slave[slaveId].can_publish_id = it.value().as<unsigned int>();
            param.configurationLock->Give();
          }
          else error_command = true;
          break;
        case Module_Type::power:
          if(strcmp(subject, "node.mppt") == 0) {
            param.configurationLock->Take();
            param.configuration->board_slave[slaveId].can_publish_id = it.value().as<unsigned int>();
            param.configurationLock->Give();
          }
          else error_command = true;
          break;
        case Module_Type::vwc:
          if(strcmp(subject, "node.svwc") == 0) {
            param.configurationLock->Take();
            param.configuration->board_slave[slaveId].can_publish_id = it.value().as<unsigned int>();
            param.configurationLock->Give();
          }
          else error_command = true;
          break;
        case Module_Type::server_gsm:
        case Module_Type::server_eth:
          if(strcmp(subject, "node.master") == 0) {
            param.configurationLock->Take();
            param.configuration->board_master.can_publish_id = it.value().as<unsigned int>();
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
        bool end_conversion = false;
        uint8_t byte_pos = 7;
        uint8_t data_read;
        // Read all HexASCII (2Char for each Time) and Put into (serial_number) at power Byte byte_pos
        // Start from MSB to LSB. Terminate if All Byte expected was read or Error Char into Input String
        // Or Input String is terminated. Each character !" HEX_TIPE (0..9,A..F) terminate function
        // Hex string can be shorter than expected. Value are convert as UINT_64 MSB Left Formatted
        boardSN = 0;
        while(byte_pos && !end_conversion) {
          end_conversion = ASCIIHexToDecimal((char**)&ptr_read, &data_read);
          boardSN |= ((uint64_t)data_read)<<(8*byte_pos--);
        }
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
            if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_ITH) == 0) {
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
            } else if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_TPR) == 0) {
              sensorId = SENSOR_METADATA_TPR;
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
            if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_MPP) == 0) {
              sensorId = SENSOR_METADATA_MPP;
            }
            else error_command = true;
            break;
          case Module_Type::vwc:
            if (strcmp(it.value().as<const char *>(), STIMA_RPC_SENSOR_NAME_VWC) == 0) {
              if(sensorMultyId == SETUP_ID) {
                sensorMultyId = 0;
              } else {
                sensorMultyId++;
              }
              sensorId = sensorMultyId;
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
        strcpy(param.configuration->board_master.boardslug, it.value().as<const char *>()); 
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
      // Set date/time from command line RPC
      STM32RTC &rtc = STM32RTC::getInstance();
      error_command = false;
      rtc.setYear(it.value().as<JsonArray>()[0].as<int>());
      rtc.setMonth(it.value().as<JsonArray>()[1].as<int>());
      rtc.setDay(it.value().as<JsonArray>()[2].as<int>());
      rtc.setHours(it.value().as<JsonArray>()[3].as<int>());
      rtc.setMinutes(it.value().as<JsonArray>()[4].as<int>());
      rtc.setSeconds(it.value().as<JsonArray>()[5].as<int>());
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
    else if (strcmp(it.key().c_str(), "mqttport") == 0)
    {
      // mqtt_port
      if(isMasterConfigure) {
        param.configurationLock->Take();
        param.configuration->mqtt_port = it.value().as<unsigned int>();
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
        memset(param.configuration->client_psk_key, 0, CLIENT_PSK_KEY_LENGTH);
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

  // error_command = Out of command context but command request valid
  // is_error = error command or out of limit parameter
  if ((error_command)||(is_error))
  {
    // Deinit index configuration varaibles
    currentModule = Module_Type::undefined;
    slaveId = UNKNOWN_ID;
    sensorId = UNKNOWN_ID;
    sensorMultyId = UNKNOWN_ID;
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

#if (USE_RPC_METHOD_UPDATE)
/// @brief RPC CallBack of update method
///        Same as configure but update direct of single parameter and saving immediatly
/// @param params JsonObject request
/// @param result JsonObject response
/// @return execute level error or ok
int RegisterRPC::update(JsonObject params, JsonObject result)
{
  bool is_error = false;
  bool error_command = false;
  bool bSaveE2 = true;

  for (JsonPair it : params)
  {
    // ************** PRINCIPAL MASTER PARAMETER LIST **************
    if (strcmp(it.key().c_str(), "sd") == 0)
    {
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
    else if (strcmp(it.key().c_str(), "stationslug") == 0)
    {
      param.configurationLock->Take();
      strcpy(param.configuration->stationslug, it.value().as<const char *>()); 
      param.configurationLock->Give();
    }
    else if (strcmp(it.key().c_str(), "boardslug") == 0)
    {
      param.configurationLock->Take();
      strcpy(param.configuration->board_master.boardslug, it.value().as<const char *>()); 
      param.configurationLock->Give();
    }
    else if (strcmp(it.key().c_str(), "lon") == 0)
    {
      param.configurationLock->Take();
      param.configuration->longitude = it.value().as<long int>(); 
      param.configurationLock->Give();
    }
    else if (strcmp(it.key().c_str(), "lat") == 0)
    {
      param.configurationLock->Take();
      param.configuration->latitude = it.value().as<long int>(); 
      param.configurationLock->Give();
    }
    else if (strcmp(it.key().c_str(), "network") == 0)
    {
      param.configurationLock->Take();
      strcpy(param.configuration->network, it.value().as<const char *>()); 
      param.configurationLock->Give();
    }
    else if (strcmp(it.key().c_str(), "date") == 0)
    {
      // Set date/time from command line RPC
      STM32RTC &rtc = STM32RTC::getInstance();
      error_command = false;
      rtc.setYear(it.value().as<JsonArray>()[0].as<int>());
      rtc.setMonth(it.value().as<JsonArray>()[1].as<int>());
      rtc.setDay(it.value().as<JsonArray>()[2].as<int>());
      rtc.setHours(it.value().as<JsonArray>()[3].as<int>());
      rtc.setMinutes(it.value().as<JsonArray>()[4].as<int>());
      rtc.setSeconds(it.value().as<JsonArray>()[5].as<int>());
    }
#if (USE_MQTT)
    else if (strcmp(it.key().c_str(), "mqttrootpath") == 0)
    {
      param.configurationLock->Take();
      strcpy(param.configuration->mqtt_root_topic, it.value().as<const char *>()); 
      param.configurationLock->Give();
    }
    else if (strcmp(it.key().c_str(), "mqttrpcpath") == 0)
    {
      param.configurationLock->Take();
      strcpy(param.configuration->mqtt_rpc_topic, it.value().as<const char *>()); 
      param.configurationLock->Give();
    }
    else if (strcmp(it.key().c_str(), "mqttmaintpath") == 0)
    {
      param.configurationLock->Take();
      strcpy(param.configuration->mqtt_maint_topic, it.value().as<const char *>()); 
      param.configurationLock->Give();
    }
    else if (strcmp(it.key().c_str(), "mqttserver") == 0)
    {
      param.configurationLock->Take();
      strcpy(param.configuration->mqtt_server, it.value().as<const char *>()); 
      param.configurationLock->Give();
    }
    else if (strcmp(it.key().c_str(), "mqttport") == 0)
    {
      param.configurationLock->Take();
      param.configuration->mqtt_port = it.value().as<unsigned int>();
      param.configurationLock->Give();
    }
    else if (strcmp(it.key().c_str(), "mqttsampletime") == 0)
    {
      param.configurationLock->Take();
      param.configuration->report_s = it.value().as<unsigned int>();
      param.configurationLock->Give();
    }
    else if (strcmp(it.key().c_str(), "mqttuser") == 0)
    {
      param.configurationLock->Take();
      strcpy(param.configuration->mqtt_username, it.value().as<const char *>()); 
      param.configurationLock->Give();
    }
    else if (strcmp(it.key().c_str(), "mqttpassword") == 0)
    {
      param.configurationLock->Take();
      strcpy(param.configuration->mqtt_password, it.value().as<const char *>()); 
      param.configurationLock->Give();
    }
    else if (strcmp(it.key().c_str(), "mqttpskkey") == 0)
    {
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
      memset(param.configuration->client_psk_key, 0, CLIENT_PSK_KEY_LENGTH);
      while((byte_pos!=CLIENT_PSK_KEY_LENGTH) && !end_conversion) {
        end_conversion = ASCIIHexToDecimal((char**)&ptr_read, &data_read);
        param.configuration->client_psk_key[byte_pos++] = data_read;
      }
      param.configurationLock->Give();
    }
#endif
#if (USE_NTP)
    else if (strcmp(it.key().c_str(), "ntpserver") == 0)
    {
      // ntp_server
      strcpy(param.configuration->ntp_server, it.value().as<const char *>());
    }
#endif
#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
    else if (strcmp(it.key().c_str(), "gsmapn") == 0)
    {
      // gsm_apn
      strcpy(param.configuration->gsm_apn, it.value().as<const char *>());
    }
    else if (strcmp(it.key().c_str(), "pppnumber") == 0)
    {
      // gsm_number
      strcpy(param.configuration->gsm_number, it.value().as<const char *>());
    }
#endif
    // Manage flags and other local param command
    else if (strcmp(it.key().c_str(), "monitor") == 0)
    {
      param.configurationLock->Take();
      param.configuration->monitor_flags = it.value().as<unsigned int>();
      param.configurationLock->Give();
    }
    // ******* Manage register Uavcan over RPC Command list ********
    else if (strcmp(it.key().c_str(), "register") == 0)
    {
      // Modify an Register Uavcan (name register)
      bSaveE2 = false; // don't save on E2
      strcpy(uavcanRegisterName, it.value().as<const char *>());
    }
    else if (strcmp(it.key().c_str(), "id") == 0)
    {
      // Modify an Register Uavcan (id Uavcan Node)
      bSaveE2 = false; // don't save on E2
      uavcanRegisterNodeId = it.value().as<unsigned int>();
    }
    else if (strcmp(it.key().c_str(), "rvs") == 0)
    {
      // Modify an Register Uavcan (register value select type)
      // Only at get value start check is format is supported
      bSaveE2 = false; // don't save on E2
      uavcanRegisterTypeValue = it.value().as<unsigned int>();
    }
    else if (strcmp(it.key().c_str(), "value") == 0)
    {
      // Prepare message before get data for casting dataValue about dataType RPC Request
      bSaveE2 = false; // don't save on E2
      system_message_t system_message = {0};

      // Modify an Register Uavcan (new value)
      switch(uavcanRegisterTypeValue) {
        case RVS_TYPE_BIT:
          system_message.value.bool_val = it.value().as<bool>();
          break;
        case RVS_TYPE_INTEGER_8:
          system_message.value.uint8_val = it.value().as<int>();
          break;
        case RVS_TYPE_INTEGER_16:
          system_message.value.uint16_val = it.value().as<int>();
          break;
        case RVS_TYPE_INTEGER_32:
        case RVS_TYPE_INTEGER_64:
          system_message.value.uint32_val = it.value().as<long int>();
          break;
        case RVS_TYPE_NATURAL_8:
          system_message.value.int8_val = it.value().as<unsigned int>();
          break;
        case RVS_TYPE_NATURAL_16:
          system_message.value.int16_val = it.value().as<unsigned int>();
          break;
        case RVS_TYPE_NATURAL_32:
        case RVS_TYPE_NATURAL_64:
          system_message.value.int32_val = it.value().as<unsigned long int>();
          break;
        case RVS_TYPE_REAL_16:
        case RVS_TYPE_REAL_32:
        case RVS_TYPE_REAL_64:
          system_message.value.float_val = it.value().as<float>();
          break;
        default:
          error_command = true;
          break;
      }

      // ?Existing configured node (clear command if not)
      if(param.configuration->board_slave[uavcanRegisterNodeId].module_type == Module_Type::undefined) {
        error_command = true;
      }

      // Send command to queue if no error
      if(!error_command) {
        // Starting queue request reload structure firmware upgradable (after download command is also send to TASK SD...)
        // And waiting response. After command structure firmware are reloaded
        system_message.task_dest = CAN_TASK_ID;
        system_message.command.do_remote_reg = true;
        strcpy(system_message.message, uavcanRegisterName);
        system_message.node_id = uavcanRegisterNodeId;
        system_message.param = uavcanRegisterTypeValue;
        // system_message.value => setted up
        param.systemMessageQueue->Enqueue(&system_message);
      }
      // Reset var value for enter new register
      memset(uavcanRegisterName, 0, sizeof(uavcanRegisterName));
      uavcanRegisterNodeId = 0;
      uavcanRegisterTypeValue = RVS_TYPE_UNKNOWN;
    }
    else
    {
      is_error = true;
    }
  }

  // error_command = Out of command context but command request valid
  // is_error = error command or out of limit parameter
  if ((error_command)||(is_error))
  {
    // Result an error
    result[F("state")] = F("error");
    if(error_command) return E_INVALID_REQUEST;
    if(is_error) return E_INVALID_PARAMS;
  }
  else
  {
    // Save on E2prom directly?
    if(bSaveE2) {
      // Reboot isn't necessary (Direct update field configuration name and value)
      // Need Saving for load new parameter on next reboot
      saveConfiguration();
    }
    // Response return
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

#if (USE_RPC_METHOD_RECOVERY)
/// @brief RPC recovery method
/// @param params 
/// @param result 
/// @return int 
int RegisterRPC::recovery(JsonObject params, JsonObject result)
{
  bool rmap_data_error = false;
  bool error_command = true;
  static int tmpstate;
  rmap_get_request_t rmap_get_request = {0};
  rmap_get_response_t rmap_get_response = {0};

  for (JsonPair it : params)
  {
    // start date
    if (strcmp(it.key().c_str(), "dts") == 0)
    {
      DateTime startDate;
      error_command = false;
      startDate.year = it.value().as<JsonArray>()[0].as<int>();
      startDate.month = it.value().as<JsonArray>()[1].as<int>();
      startDate.day = it.value().as<JsonArray>()[2].as<int>();
      startDate.hours = it.value().as<JsonArray>()[3].as<int>();
      startDate.minutes = it.value().as<JsonArray>()[4].as<int>();
      startDate.seconds = it.value().as<JsonArray>()[5].as<int>();
      TRACE_INFO_F(F("RPC start data pointer... [ %d/%d/%d %d:%d:%d ]"), startDate.day, startDate.month, startDate.year, startDate.hours, startDate.minutes, startDate.seconds);

      // *********** START OPTIONAL SET PONTER REQUEST RESPONSE **********
      uint32_t rmap_date_time_ptr = convertDateToUnixTime(&startDate);

      memset(&rmap_get_request, 0, sizeof(rmap_get_request_t));
      rmap_get_request.param = rmap_date_time_ptr;
      rmap_get_request.command.do_synch_ptr = true;
      // Optional Save Pointer in File (No need in SetPtr. Only in Get Last Data Memory OK!!!)
      rmap_get_request.command.do_save_ptr = false;
      TRACE_VERBOSE_F(F("Starting request SET Data RMAP PTR to local SD\r\n"));
      // Push data request to queue SD
      param.dataRmapGetRequestQueue->Enqueue(&rmap_get_request);

      // Seek Operation can Be Long Time Procedure. Queue can be post in waiting state without Time End
      // No Task Suspended (RPC Can entre from various Task) Time Not Problem to queue
      rmap_data_error = !param.dataRmapGetResponseQueue->Dequeue(&rmap_get_response);
      rmap_data_error |= rmap_get_response.result.event_error;
    }
    // end date
    else if (strcmp(it.key().c_str(), "dte") == 0)
    {
      DateTime endDate;
      error_command = false;
      endDate.year = it.value().as<JsonArray>()[0].as<int>();
      endDate.month = it.value().as<JsonArray>()[1].as<int>();
      endDate.day = it.value().as<JsonArray>()[2].as<int>();
      endDate.hours = it.value().as<JsonArray>()[3].as<int>();
      endDate.minutes = it.value().as<JsonArray>()[4].as<int>();
      endDate.seconds = it.value().as<JsonArray>()[5].as<int>();
      TRACE_INFO_F(F("RPC end data pointer... [ %d/%d/%d %d:%d:%d ]"), endDate.day, endDate.month, endDate.year, endDate.hours, endDate.minutes, endDate.seconds);

      // *********** START OPTIONAL SET PONTER REQUEST RESPONSE **********
      uint32_t rmap_date_time_ptr = convertDateToUnixTime(&endDate);

      memset(&rmap_get_request, 0, sizeof(rmap_get_request_t));
      rmap_get_request.param = rmap_date_time_ptr;
      rmap_get_request.command.do_end_ptr = true;
      TRACE_VERBOSE_F(F("Starting request END Data RMAP PTR to local SD\r\n"));
      // Push data request to queue SD
      param.dataRmapGetRequestQueue->Enqueue(&rmap_get_request);

      // Seek Operation can Be Long Time Procedure. Queue can be post in waiting state without Time End
      // No Task Suspended (RPC Can entre from various Task) Time Not Problem to queue
      rmap_data_error = !param.dataRmapGetResponseQueue->Dequeue(&rmap_get_response);
      rmap_data_error |= rmap_get_response.result.event_error;
    }
  }

  // ? Any Error on RMAP Set Pointer
  if (error_command)
  {
    // error_command = Out of command context but command request valid
    // is_error = error command or out of limit parameter
    // Result an error
    result[F("state")] = F("error");
    return E_INVALID_REQUEST;
  }
  else if (rmap_data_error)
  {
    tmpstate = E_INVALID_PARAMS;
    result[F("state")] = F("error");
    TRACE_ERROR_F(F("RPC invalid data pointer params [ %s ]"), FAIL_STRING);
  }
  else
  {
    TRACE_INFO_F(F("RPC: Request Recovery\r\n"));
    tmpstate = E_SUCCESS;
    result[F("state")] = F("done");
  }
  return tmpstate;
}
#endif

#if (USE_RPC_METHOD_REBOOT)
/// @brief RPC reboot method
/// @param params 
/// @param result 
/// @return int 
int RegisterRPC::reboot(JsonObject params, JsonObject result)
{
  // print lcd message before reboot
  bool inibith_reboot = false;

  TRACE_INFO_F(F("RPC: Request Reboot\r\n"));

  for (JsonPair it : params)
  {
    // do the firmware update on all of the boards
    if (strcmp(it.key().c_str(), "fupdate") == 0)
    {
      if (it.value().as<bool>() == true)
      {
        TRACE_INFO_F(F("RPC: Starting update firmware\r\n"));

        inibith_reboot = true;

        // Starting queue request reload structure firmware upgradable (after download command is also send to TASK SD...)
        // And waiting response. After command structure firmware are reloaded
        system_message_t system_message = {0};
        system_message.task_dest = SD_TASK_ID;
        system_message.command.do_reload_fw = true;
        system_message.param = CMD_PARAM_REQUIRE_RESPONSE;
        param.systemMessageQueue->Enqueue(&system_message);

        // Waiting a response done before continue (reload firmware OK!!!)
        while(true) {
          // Continuos Switching context non blocking
          // Need Waiting Task for start command on All used TASK
          taskYIELD();
          vTaskDelay(100);
          // Check response done
          if(!param.systemMessageQueue->IsEmpty()) {
            param.systemMessageQueue->Peek(&system_message);
            if(system_message.command.done_reload_fw) {
              // Remove message (Reinit Done is OK)
              param.systemMessageQueue->Dequeue(&system_message);
              break;
            }
          }
        }

        // Satrting queue request upload all system board firmware ( on CAN )
        memset(&system_message, 0, sizeof(system_message_t));
        system_message.task_dest = CAN_TASK_ID;
        system_message.command.do_update_all = true;
        param.systemMessageQueue->Enqueue(&system_message);
      }
    }
  }

  // Send a response...
  result[F("state")] = "done";

  #if (ENABLE_RPC_LOCAL_REBOOT)
  if(!inibith_reboot) {
    TRACE_INFO_F(F("RPC: Perform reboot...\r\n"));
    // Start delayed REBOOT with queue command (need to send response callback)
    system_message_t system_message = {0};
    system_message.task_dest = CAN_TASK_ID;
    system_message.command.do_reboot = true;
    param.systemMessageQueue->Enqueue(&system_message, 0);
  }
  #endif

  return E_SUCCESS;
}
#endif

#if (USE_RPC_METHOD_TEST)
/// @brief RPC test method
/// @param params 
/// @param result 
/// @return int 
int RegisterRPC::rpctest(JsonObject params, JsonObject result)
{
  // RPC Test

  for (JsonPair it : params)
  {
    // loop in params
    if (strcmp(it.key().c_str(), "update") == 0)
    {
      if (it.value().as<bool>() == true)
      {
        TRACE_INFO_F(F("UPDATE REQUEST TEST\r\n"));
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
void RegisterRPC::initFixedConfigurationParam(uint8_t lastNodeConfig)
{
  // Private param and Semaphore: param.configuration, param.configurationLock
  param.configurationLock->Take();

  param.configuration->module_main_version = MODULE_MAIN_VERSION;
  param.configuration->module_minor_version = MODULE_MINOR_VERSION;
  param.configuration->configuration_version = CONFIGURATION_VERSION;
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

  // Reset other Slave unconfigured Module
  for(uint8_t idTmp = lastNodeConfig; idTmp < MAX_NODE_CONNECT; idTmp++) {
    memset(&param.configuration->board_slave[idTmp], 0, sizeof(board_configuration_t));
    param.configuration->board_slave[idTmp].can_address = 0xFFu;
    param.configuration->board_slave[idTmp].can_port_id = 0xFFFFu;
    param.configuration->board_slave[idTmp].can_publish_id = 0xFFFFu;
  }

  param.configurationLock->Give();
}

/// @brief Save configuration to E2
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
