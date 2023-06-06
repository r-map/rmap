/**
  ******************************************************************************
  * @file    mqtt_task.cpp
  * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Mqtt RMAP over Cyclone TCP Source files
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

#define TRACE_LEVEL     MQTT_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   MQTT_TASK_ID

#include "tasks/mqtt_task.h"

#if (USE_MQTT)

using namespace cpp_freertos;

/* 
  List of preferred ciphersuites
  https://ciphersuite.info/cs/?security=recommended&singlepage=true&page=2&tls=all&sort=asc
  defined in tasks/mqtt_task.h -> const uint16_t cipherSuites[] = TYPE_VALUE
*/

MqttTask::MqttTask(const char *taskName, uint16_t stackSize, uint8_t priority, MqttParam_t mqttParam) : Thread(taskName, stackSize, priority), param(mqttParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(MQTT_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  // local to static member private access
  localRpcLock = param.rpcLock;
  localStreamRpc = param.streamRpc;
  localSystemStatus = param.system_status;
  localPtrMqttClientContext = &mqttClientContext;

  state = MQTT_STATE_INIT;
  version = MQTT_VERSION_3_1_1;
  transportProtocol = MQTT_TRANSPORT_PROTOCOL_TLS;
  qos = MQTT_QOS_LEVEL_1;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void MqttTask::TaskMonitorStack()
{
  u_int16_t stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark( NULL );
  if((stackUsage) && (stackUsage < param.system_status->tasks[LOCAL_TASK_ID].stack)) {
    param.systemStatusLock->Take();
    param.system_status->tasks[LOCAL_TASK_ID].stack = stackUsage;
    param.systemStatusLock->Give();
  }
}
#endif

/// @brief local watchDog and Sleep flag Task (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void MqttTask::TaskWatchDog(uint32_t millis_standby)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Update WDT Signal (Direct or Long function Timered)
  if(millis_standby)  
  {
    // Check 1/2 Freq. controller ready to WDT only SET flag
    if((millis_standby) < WDT_CONTROLLER_MS / 2) {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
    } else {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::timer;
      // Add security milimal Freq to check
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog_ms = millis_standby + WDT_CONTROLLER_MS;
    }
  }
  else
    param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
  param.systemStatusLock->Give();
}

/// @brief local suspend flag and positor running state Task (optional)
/// @param state_position Sw_Position (Local STATE)
/// @param state_subposition Sw_SubPosition (Optional Local SUB_STATE Position Monitor)
/// @param state_operation operative mode flag status for this task
void MqttTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Signal Task sleep/disabled mode from request (Auto SET WDT on Resume)
  if((param.system_status->tasks[LOCAL_TASK_ID].state == task_flag::suspended)&&
     (state_operation==task_flag::normal))
     param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
  param.system_status->tasks[LOCAL_TASK_ID].state = state_operation;
  param.system_status->tasks[LOCAL_TASK_ID].running_pos = state_position;
  param.system_status->tasks[LOCAL_TASK_ID].running_sub = state_subposition;
  param.systemStatusLock->Give();
}

void MqttTask::Run()
{
  uint8_t retry;
  bool is_error;
  bool is_subscribed;
  bool mqtt_connection_estabilished;
  error_t error;
  connection_request_t connection_request;
  connection_response_t connection_response;
  IpAddr ipAddr;
  bool is_data_publish_end = false;
  // RMAP Data queue and status local VAR
  rmap_get_request_t rmap_get_request;
  rmap_get_response_t rmap_get_response;
  bool rmap_eof;
  bool rmap_data_error;
  uint32_t countData;
  // RMAP Casting value to Uavcan Structure
  rmap_service_module_TH_Response_1_0 *rmapDataTH;
  rmap_service_module_Rain_Response_1_0 *rmapDataRain;
  rmap_service_module_Radiation_Response_1_0 *rmapDataRadiation;
  rmap_service_module_Wind_Response_1_0 *rmapDataWind;
  rmap_service_module_VWC_Response_1_0 *rmapDataVWC;
  rmap_service_module_Power_Response_1_0 *rmapDataPower;

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  while (true)
  {
    switch (state)
    {
    case MQTT_STATE_INIT:
      state = MQTT_STATE_WAIT_NET_EVENT;
      param.systemStatusLock->Take();
      param.system_status->connection.is_mqtt_connected = false;
      param.system_status->connection.is_mqtt_connecting = false;
      param.system_status->connection.is_mqtt_disconnected = true;
      param.system_status->connection.is_mqtt_disconnecting = false;
      param.system_status->connection.is_mqtt_publishing = false;
      param.system_status->connection.is_mqtt_publishing_end = false;
      param.systemStatusLock->Give();
      TRACE_VERBOSE_F(F("MQTT_STATE_INIT -> MQTT_STATE_WAIT_NET_EVENT\r\n"));
      break;

    case MQTT_STATE_WAIT_NET_EVENT:
      retry = 0;
      is_error = false;
      is_data_publish_end = false;

      // wait connection request
      // Suspend TASK Controller for queue waiting portMAX_DELAY
      TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
      if (param.connectionRequestQueue->Peek(&connection_request, portMAX_DELAY))
      {
        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);      
        // do mqtt connection
        if (connection_request.do_mqtt_connect)
        {
          // Set local pointer MqttServer to config string
          MqttServer = param.configuration->mqtt_server;
          
          // Start new connection sequence
          mqtt_connection_estabilished = false;

          param.connectionRequestQueue->Dequeue(&connection_request);
          TRACE_VERBOSE_F(F("MQTT_STATE_WAIT_NET_EVENT -> MQTT_STATE_CONNECT\r\n"));
          state = MQTT_STATE_CONNECT;
        }
      }
      break;

    case MQTT_STATE_CONNECT:
      error = mqttClientInit(&mqttClientContext);
      if (error)
      {
        TRACE_ERROR_F(F("%s, Failed to initialize MQTT client [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);
      }
      
      param.systemStatusLock->Take();
      param.system_status->connection.is_mqtt_connecting = true;
      param.systemStatusLock->Give();

      TRACE_INFO_F(F("%s Resolving mqtt server name of %s\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server);
      // Resolve MQTT server name
      TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_SUSPEND);
      error = getHostByName(NULL, param.configuration->mqtt_server, &ipAddr, 0);
      TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
      if (error)
      {
        is_error = true;
        param.systemStatusLock->Take();
        param.system_status->connection.is_dns_failed_resolve = true;
        param.systemStatusLock->Give();
        TRACE_ERROR_F(F("%s Failed to resolve mqtt server name of %s [ %s ]\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, ERROR_STRING);
        state = MQTT_STATE_DISCONNECT;
        TRACE_VERBOSE_F(F("MQTT_STATE_CONNECT -> MQTT_STATE_DISCONNECT\r\n"));
        break;
      }

      // Set the MQTT version to be used
      mqttClientSetVersion(&mqttClientContext, version);

      if (transportProtocol == MQTT_TRANSPORT_PROTOCOL_TLS)
      {
        // Shared Pointer
        MqttYarrowContext = param.yarrowContext;
        MqttClientPSKKey = param.configuration->client_psk_key;

        // Set PSK identity
        snprintf(MqttClientPSKIdentity, sizeof(MqttClientPSKIdentity), "%s/%s/%s", param.configuration->mqtt_username, param.configuration->stationslug, param.configuration->boardslug);

        // MQTT over TLS
        mqttClientSetTransportProtocol(&mqttClientContext, MQTT_TRANSPORT_PROTOCOL_TLS);
        // Register TLS initialization callback
        mqttClientRegisterTlsInitCallback(&mqttClientContext, mqttTlsInitCallback);
      }

      // Register publish callback function
      mqttClientRegisterPublishCallback(&mqttClientContext, mqttPublishCallback);

      // Set communication timeout
      mqttClientSetTimeout(&mqttClientContext, MQTT_TIMEOUT_MS);
      // Set keep-alive value
      mqttClientSetKeepAlive(&mqttClientContext, MQTT_KEEP_ALIVE_S);

      // Set client identifier
      snprintf(clientIdentifier, sizeof(clientIdentifier), "%s/%s/%s", param.configuration->mqtt_username, param.configuration->stationslug, param.configuration->boardslug);
      mqttClientSetIdentifier(&mqttClientContext, clientIdentifier);

      // Set username and password
      if (strlen(param.configuration->mqtt_username) && strlen(param.configuration->mqtt_password))
      {
        mqttClientSetAuthInfo(&mqttClientContext, param.configuration->mqtt_username, param.configuration->mqtt_password);
      }

      // Set Will message ( ON ROOT TOPIC, MQTT_STATUS_TOPIC... FROM doc.rmap.cc documentation )
      if (strlen(MQTT_ON_ERROR_MESSAGE))
      {
        // Changed MQTT_CLIENT_MAX_WILL_TOPIC_LEN -> FROM 16 TO 80 BYTES
        snprintf(topic, sizeof(topic), "%s/%s/%s/%07d,%07d/%s/%s", param.configuration->mqtt_maint_topic, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network, MQTT_STATUS_TOPIC);
        mqttClientSetWillMessage(&mqttClientContext, topic, MQTT_ON_ERROR_MESSAGE, strlen(MQTT_ON_ERROR_MESSAGE), qos, true);
      }

      TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_SUSPEND);
      // Establish connection with the MQTT server
      error = mqttClientConnect(&mqttClientContext, &ipAddr, param.configuration->mqtt_port, false);
      TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
      // Any error to report?
      if (error)
      {
        is_error = true;

        TRACE_ERROR_F(F("%s Failed to connect to mqtt server %s [ %s ]\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, ERROR_STRING);

        TRACE_VERBOSE_F(F("MQTT_STATE_CONNECT -> MQTT_STATE_DISCONNECT\r\n"));
        state = MQTT_STATE_DISCONNECT;
        break;
      }
      else
      {
        snprintf(topic, sizeof(topic), "%s/%s/%s/%07d,%07d/%s/%s", param.configuration->mqtt_maint_topic, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network, MQTT_STATUS_TOPIC);
        // publish connection message (Conn + Version and Revision)
        sprintf(message, "{\"v\":\"conn\", \"s\":%d, \"m\":%d}", param.configuration->module_main_version, param.configuration->module_minor_version);
        TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
        error = mqttClientPublish(&mqttClientContext, topic, message, strlen(message), qos, true, NULL);
        TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
        TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

        TRACE_INFO_F(F("%s Connected to mqtt server %s on port %d\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, param.configuration->mqtt_port);
      }

      // Subscribe to the desired topics (Subscribe error not blocking connection)
      snprintf(topic, sizeof(topic), "%s/%s/%s/%07d,%07d/%s/%s", param.configuration->mqtt_rpc_topic, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network, MQTT_RPC_COM_TOPIC);
      snprintf(topic_rpc_response, sizeof(topic_rpc_response), "%s/%s/%s/%07d,%07d/%s/%s", param.configuration->mqtt_rpc_topic, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network, MQTT_RPC_RES_TOPIC);
      TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_SUSPEND);
      is_subscribed = !mqttClientSubscribe(&mqttClientContext, topic, qos, NULL);
      TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
      TRACE_INFO_F(F("%s Subscribe to mqtt server %s on %s [ %s ]\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, topic, error ? ERROR_STRING : OK_STRING);

      param.systemStatusLock->Take();
      param.system_status->connection.is_mqtt_subscribed = is_subscribed;      
      param.system_status->connection.is_mqtt_connected = true;
      param.system_status->connection.is_mqtt_disconnected = false;
      param.system_status->connection.is_mqtt_connecting = false;
      // Checked for end of transmission
      param.system_status->connection.is_mqtt_publishing_end = false;
      param.system_status->connection.mqtt_data_published = 0;
      param.systemStatusLock->Give();

      // Session connection complete (send response to request command)
      // No other response from TASK with this session connection
      mqtt_connection_estabilished = true;
      // Response direct when connection complete
      memset(&connection_response, 0, sizeof(connection_response_t));
      connection_response.done_mqtt_connected = true;
      param.connectionResponseQueue->Enqueue(&connection_response);

      // Successful connection?
      state = MQTT_STATE_PUBLISH_INFO;
      TRACE_VERBOSE_F(F("MQTT_STATE_CONNECT -> MQTT_STATE_PUBLISH_INFO\r\n"));
      break;

    case MQTT_STATE_PUBLISH_INFO:
      // Local Var to Message status
      DateTime dtStatus;
      char dtBlock[30];
      byte indexPosition;
      char bitState[MQTT_PUB_MAX_BIT_STATE + 1];
      byte byteState[MQTT_PUB_MAX_BYTE_STATE];

      #if (ENABLE_STACK_USAGE)
      TaskMonitorStack();
      #endif

      // Restore TOPIC -> MQTT_STATUS_TOPIC
      snprintf(topic, sizeof(topic), "%s/%s/%s/%07d,%07d/%s/%s", param.configuration->mqtt_maint_topic, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network, MQTT_STATUS_TOPIC);
      // Prepare DateTime Generic status Message
      convertUnixTimeToDate(param.system_status->datetime.epoch_sensors_get_value, &dtStatus);
      makeDate(dtStatus, dtBlock, sizeof(dtBlock));

      // ******************************************************************
      // STATE: Publish Master Info, Message are Master Type structured
      // ******************************************************************
      // Prepare BIT Structure Type Message Start from MSB
      for(uint8_t bitIdx = 0; bitIdx < MQTT_PUB_MAX_BIT_STATE; bitIdx++) {
        bitState[bitIdx] = '0';
      }
      bitState[MQTT_PUB_MAX_BIT_STATE] = 0;
      indexPosition = 15;
      // SD CARD
      if(!param.system_status->flags.sd_card_ready) {
         bitState[indexPosition] = '1';
      }
      indexPosition--;
      // IS FIRMWARE AVAIABLE?
      if(param.system_status->data_master.fw_upgradable) {
         bitState[indexPosition] = '1';
      }
      indexPosition--;
      // CRITICAL POWER FLAG? Not Critical Power mode selected but FLAG from MPPT
      if(param.system_status->flags.power_critical) {
         bitState[indexPosition] = '1';
      }
      indexPosition--;
      // RSSI Signal to low (<10)
      if(param.system_status->modem.rssi < 10) {
         bitState[indexPosition] = '1';
      }

      // Prepare BYTE Type
      indexPosition=0;
      for(uint8_t byteIdx = 0; byteIdx < MQTT_PUB_MAX_BYTE_STATE; byteIdx++) {
        byteState[byteIdx] = 0;
      }
      // Invert OK with Error 100 - OK
      byteState[indexPosition++] = 100 - param.system_status->modem.perc_modem_connection_valid;
      byteState[indexPosition++] = param.boot_request->tot_reset;
      byteState[indexPosition++] = param.boot_request->wdt_reset;
      byteState[indexPosition] = 0;

      // publish connection message (Conn + Version and Revision)
      // sprintf(message, "{%s \"bs\":\"%s\", \"b\":\"0b%s\", \"c\":[%u,%u,%u,%u]}",
      //   dtBlock, param.configuration->board_master.module_name, bitState, byteState[0], byteState[1], byteState[2], byteState[3]);
      sprintf(message, "{%s \"bs\":\"%s\", \"b\":\"0b%s\", \"c\":[%u,%u,%u,%u]}",
        dtBlock, param.configuration->boardslug, bitState, byteState[0], byteState[1], byteState[2], byteState[3]);
      TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
      error = mqttClientPublish(&mqttClientContext, topic, message, strlen(message), qos, true, NULL);
      TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
      TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

      // ******************************************************************
      // STATE: Publish Slave Info, Message Depending by Node Module Type
      // ******************************************************************
      // For each Slave... Enabled and configured
      for(uint8_t iNodeSlave = 0; iNodeSlave < MAX_NODE_CONNECT; iNodeSlave++) {
        // SOlo per i moduli correttamente configurati
        if(param.configuration->board_slave[iNodeSlave].module_type) {
          // 3 BIT Fixed FROM Master Calcualtion and 8 BITS Relative Module Put on GET DATA from CAN TASK
          for(uint8_t bitIdx = 0; bitIdx < MQTT_PUB_MAX_BIT_STATE; bitIdx++) {
            bitState[bitIdx] = '0';
          }
          bitState[MQTT_PUB_MAX_BIT_STATE] = 0;
          indexPosition = 15;
          // Depending from type module (Message composition)
          if(!param.system_status->data_slave[iNodeSlave].is_online) {
            bitState[indexPosition] = '1';
            // Auto set 100 % Error CAN data if OffLine
            byteState[0] = 100;
          }
          indexPosition--;
          if(param.system_status->data_slave[iNodeSlave].fw_upgradable) {
            bitState[indexPosition] = '1';
          }
          indexPosition--;
          if(param.system_status->data_slave[iNodeSlave].maintenance_mode) {
            bitState[indexPosition] = '1';
          }
          indexPosition--;
          // Fill With 8xBIT Hardware Flag 8 Bits
          for(uint8_t iBit=0; iBit<8; iBit++) {
            if(param.system_status->data_slave[iNodeSlave].bit8StateFlag & (1<<iBit)) {
                bitState[indexPosition] = '1';
            }
            indexPosition--;
          }

          // Prepare BYTE Type
          for(uint8_t byteIdx = 0; byteIdx < MQTT_PUB_MAX_BYTE_STATE; byteIdx++) {
            byteState[byteIdx] = 0;
          }
          indexPosition = 0;
          // Report inverted OK (100- for Error...)
          byteState[indexPosition++] = param.system_status->data_slave[iNodeSlave].perc_can_comm_err;
          byteState[indexPosition++] = param.system_status->data_slave[iNodeSlave].byteStateFlag[0];
          byteState[indexPosition++] = param.system_status->data_slave[iNodeSlave].byteStateFlag[1];
          byteState[indexPosition] = param.system_status->data_slave[iNodeSlave].byteStateFlag[2];

          // publish connection message (Conn + Version and Revision)
          sprintf(message, "{%s \"bs\":\"%s\", \"b\":\"0b%s\", \"c\":[%u,%u,%u,%u]}",
            dtBlock, param.configuration->board_slave[iNodeSlave].module_name, bitState, byteState[0], byteState[1], byteState[2], byteState[3]);
          TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
          error = mqttClientPublish(&mqttClientContext, topic, message, strlen(message), qos, true, NULL);
          TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
          TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);
        }
      }
      // ******************************************************************

      // Successful publish info message? No more action, go to publish Data...
      // Check is need only for real data push to RMAP Server
      state = MQTT_STATE_PUBLISH;
      TRACE_VERBOSE_F(F("MQTT_STATE_PUBLISH_INFO -> MQTT_STATE_PUBLISH\r\n"));
      break;

    case MQTT_STATE_PUBLISH:

      #if (ENABLE_STACK_USAGE)
      TaskMonitorStack();
      #endif

      // *****************************************
      //  RUN GET RMAP Data Queue and Append MQTT
      // *****************************************
      if(param.system_status->flags.new_data_to_send) {

        rmap_eof = false;
        rmap_data_error = false;
        countData = 0;
        error = NO_ERROR;

        // Exit on End of data or Error from queue
        while((!rmap_data_error)&&(!rmap_eof)&&(!error)) {

          // SD have to GET Ready before Push DATA alert if not SD Ready or present into system_status
          if(!param.system_status->flags.sd_card_ready) {
            // Direct mode, get data from queue on RAM DATA queue
            TRACE_INFO_F(F("MQTT: RMAP SD was not ready, pulling last data directly from RAM queue\r\n"));
            // *********************************************************
            //      Perform RMAP Write Data direct RAM get message
            // *********************************************************
            // Fixed no error on GET from SD (not ready SD)
            rmap_data_error = false;
            if(!param.dataRmapPutQueue->IsEmpty()) {
              memset(&rmap_get_response, 0, sizeof(rmap_get_response));
              // Test message from queue ( Only if SD Trasnsaction complete, remove from Queue )
              param.dataRmapPutQueue->Dequeue(&rmap_get_response.rmap_data);
              // Simulate data Getted from SD Queue and direct check End of Data
              rmap_get_response.result.done_get_data = true;
              rmap_get_response.result.end_of_data = param.dataRmapPutQueue->IsEmpty();
            } else {
              // End of DATA...
              memset(&rmap_get_response, 0, sizeof(rmap_get_response));
              rmap_get_response.result.done_get_data = false;
              rmap_get_response.result.end_of_data = true;
            }
          } else {
            // Normal mode, get data from queue on SD CARD
            memset(&rmap_get_request, 0, sizeof(rmap_get_request));
            // Get Next data... Stop at EOF
            rmap_get_request.command.do_get_data = true;
            // Save Pointer? Optional
            // BUT is Automatic save on Get Last Data Avaiable if Normal Request (Not Recovery...)
            // rmap_get_request.command.do_save_ptr = true;
            // Push data request to queue SD
            param.dataRmapGetRequestQueue->Enqueue(&rmap_get_request);
            // Waiting response from SD with TimeOUT
            memset(&rmap_get_response, 0, sizeof(rmap_get_response));
            TaskWatchDog(FILE_IO_DATA_QUEUE_TIMEOUT);
            rmap_data_error = !param.dataRmapGetResponseQueue->Dequeue(&rmap_get_response, FILE_IO_DATA_QUEUE_TIMEOUT);
            rmap_data_error |= rmap_get_response.result.event_error;
          }

          #if (TEST_MQTT_FIXED_DATA)

          // INIZIO PROVA CON DATI FISSI...
          rmap_get_response.result.event_error = 0;

          if (countData == 0)
          {
            rmap_get_response.result.end_of_data = 0;

            rmapDataTH = (rmap_module_TH_1_0 *)&rmap_get_response.rmap_data;

            rmapDataTH->ITH.humidity.val.value = 50;
            rmapDataTH->ITH.humidity.confidence.value = 100;

            rmapDataTH->ITH.temperature.val.value = 27315;
            rmapDataTH->ITH.temperature.confidence.value = 99;

            rmapDataTH->ITH.metadata.level.LevelType1.value = 103;
            rmapDataTH->ITH.metadata.level.L1.value = 2000;
            rmapDataTH->ITH.metadata.level.LevelType2.value = UINT16_MAX;
            rmapDataTH->ITH.metadata.level.L2.value = UINT16_MAX;

            rmapDataTH->ITH.metadata.timerange.Pindicator.value = 254;
            rmapDataTH->ITH.metadata.timerange.P1.value = 0;
            rmapDataTH->ITH.metadata.timerange.P2 = 0;

            rmapDataTH->NTH.humidity.val.value = 40;
            rmapDataTH->NTH.humidity.confidence.value = 90;

            rmapDataTH->NTH.temperature.val.value = 28315;
            rmapDataTH->NTH.temperature.confidence.value = 89;

            rmapDataTH->NTH.metadata.level.LevelType1.value = 103;
            rmapDataTH->NTH.metadata.level.L1.value = 2000;
            rmapDataTH->NTH.metadata.level.LevelType2.value = UINT16_MAX;
            rmapDataTH->NTH.metadata.level.L2.value = UINT16_MAX;

            rmapDataTH->NTH.metadata.timerange.Pindicator.value = 3;
            rmapDataTH->NTH.metadata.timerange.P1.value = 0;
            rmapDataTH->NTH.metadata.timerange.P2 = 900;

            rmapDataTH->MTH.humidity.val.value = 30;
            rmapDataTH->MTH.humidity.confidence.value = 80;

            rmapDataTH->MTH.temperature.val.value = 29315;
            rmapDataTH->MTH.temperature.confidence.value = 79;

            rmapDataTH->MTH.metadata.level.LevelType1.value = 103;
            rmapDataTH->MTH.metadata.level.L1.value = 2000;
            rmapDataTH->MTH.metadata.level.LevelType2.value = UINT16_MAX;
            rmapDataTH->MTH.metadata.level.L2.value = UINT16_MAX;

            rmapDataTH->MTH.metadata.timerange.Pindicator.value = 0;
            rmapDataTH->MTH.metadata.timerange.P1.value = 0;
            rmapDataTH->MTH.metadata.timerange.P2 = 900;

            rmapDataTH->XTH.humidity.val.value = 20;
            rmapDataTH->XTH.humidity.confidence.value = 70;

            rmapDataTH->XTH.temperature.val.value = 30315;
            rmapDataTH->XTH.temperature.confidence.value = 69;

            rmapDataTH->XTH.metadata.level.LevelType1.value = 103;
            rmapDataTH->XTH.metadata.level.L1.value = 2000;
            rmapDataTH->XTH.metadata.level.LevelType2.value = UINT16_MAX;
            rmapDataTH->XTH.metadata.level.L2.value = UINT16_MAX;

            rmapDataTH->XTH.metadata.timerange.Pindicator.value = 2;
            rmapDataTH->XTH.metadata.timerange.P1.value = 0;
            rmapDataTH->XTH.metadata.timerange.P2 = 900;

            rmap_get_response.rmap_data.module_type = Module_Type::th;
          }
          else if (countData == 1)
          {
            rmap_get_response.result.end_of_data = 0;

            rmapDataRain = (rmap_module_Rain_1_0 *)&rmap_get_response.rmap_data;

            rmapDataRain->TBR.rain.val.value = 23;
            rmapDataRain->TBR.rain.confidence.value = 56;

            rmapDataRain->TBR.metadata.level.LevelType1.value = 1u;
            rmapDataRain->TBR.metadata.level.L1.value = UINT16_MAX;
            rmapDataRain->TBR.metadata.level.LevelType2.value = UINT16_MAX;
            rmapDataRain->TBR.metadata.level.L2.value = UINT16_MAX;

            rmapDataRain->TBR.metadata.timerange.Pindicator.value = 1;
            rmapDataRain->TBR.metadata.timerange.P1.value = 0;
            rmapDataRain->TBR.metadata.timerange.P2 = 900;

            rmap_get_response.rmap_data.module_type = Module_Type::rain;
          }
          else if (countData == 2)
          {
            rmap_get_response.result.end_of_data = 0;

            rmapDataRadiation = (rmap_module_Radiation_1_0 *)&rmap_get_response.rmap_data;

            rmapDataRadiation->DSA.radiation.val.value = 350;
            rmapDataRadiation->DSA.radiation.confidence.value = 49;

            rmapDataRadiation->DSA.metadata.level.LevelType1.value = 1;
            rmapDataRadiation->DSA.metadata.level.L1.value = UINT16_MAX;
            rmapDataRadiation->DSA.metadata.level.LevelType2.value = UINT16_MAX;
            rmapDataRadiation->DSA.metadata.level.L2.value = UINT16_MAX;

            rmapDataRadiation->DSA.metadata.timerange.Pindicator.value = 0;
            rmapDataRadiation->DSA.metadata.timerange.P1.value = 0;
            rmapDataRadiation->DSA.metadata.timerange.P2 = 900;

            rmap_get_response.rmap_data.module_type = Module_Type::radiation;
          }
          else if (countData == 3)
          {
            rmap_get_response.result.end_of_data = 0;

            rmapDataWind = (rmap_module_Wind_1_0 *)&rmap_get_response.rmap_data;

            rmapDataWind->DWA.direction.val.value = 125;
            rmapDataWind->DWA.direction.confidence.value = 85;

            rmapDataWind->DWA.speed.val.value = 15;
            rmapDataWind->DWA.speed.confidence.value = 75;

            rmapDataWind->DWA.metadata.level.LevelType1.value = 103;
            rmapDataWind->DWA.metadata.level.L1.value = 10000;
            rmapDataWind->DWA.metadata.level.LevelType2.value = UINT16_MAX;
            rmapDataWind->DWA.metadata.level.L2.value = UINT16_MAX;

            rmapDataWind->DWA.metadata.timerange.Pindicator.value = 254;
            rmapDataWind->DWA.metadata.timerange.P1.value = 0;
            rmapDataWind->DWA.metadata.timerange.P2 = 0;

            rmapDataWind->DWB.direction.val.value = 225;
            rmapDataWind->DWB.direction.confidence.value = 65;

            rmapDataWind->DWB.speed.val.value = 22;
            rmapDataWind->DWB.speed.confidence.value = 55;

            rmapDataWind->DWB.metadata.level.LevelType1.value = 103;
            rmapDataWind->DWB.metadata.level.L1.value = 10000;
            rmapDataWind->DWB.metadata.level.LevelType2.value = UINT16_MAX;
            rmapDataWind->DWB.metadata.level.L2.value = UINT16_MAX;

            rmapDataWind->DWB.metadata.timerange.Pindicator.value = 200;
            rmapDataWind->DWB.metadata.timerange.P1.value = 0;
            rmapDataWind->DWB.metadata.timerange.P2 = 900;

            rmapDataWind->DWC.peak.val.value = 56;
            rmapDataWind->DWC.peak.confidence.value = 93;

            rmapDataWind->DWC._long.val.value = 43;
            rmapDataWind->DWC._long.confidence.value = 83;

            rmapDataWind->DWC.metadata.level.LevelType1.value = 103;
            rmapDataWind->DWC.metadata.level.L1.value = 10000;
            rmapDataWind->DWC.metadata.level.LevelType2.value = UINT16_MAX;
            rmapDataWind->DWC.metadata.level.L2.value = UINT16_MAX;

            rmapDataWind->DWC.metadata.timerange.Pindicator.value = 2;
            rmapDataWind->DWC.metadata.timerange.P1.value = 0;
            rmapDataWind->DWC.metadata.timerange.P2 = 900;

            rmapDataWind->DWD.speed.val.value = 34;
            rmapDataWind->DWD.speed.confidence.value = 74;

            rmapDataWind->DWD.metadata.level.LevelType1.value = 103;
            rmapDataWind->DWD.metadata.level.L1.value = 10000;
            rmapDataWind->DWD.metadata.level.LevelType2.value = UINT16_MAX;
            rmapDataWind->DWD.metadata.level.L2.value = UINT16_MAX;

            rmapDataWind->DWD.metadata.timerange.Pindicator.value = 0;
            rmapDataWind->DWD.metadata.timerange.P1.value = 0;
            rmapDataWind->DWD.metadata.timerange.P2 = 900;

            rmapDataWind->DWE.class1.val.value = 11;
            rmapDataWind->DWE.class1.confidence.value = 51;
            rmapDataWind->DWE.class2.val.value = 12;
            rmapDataWind->DWE.class2.confidence.value = 52;
            rmapDataWind->DWE.class3.val.value = 13;
            rmapDataWind->DWE.class3.confidence.value = 53;
            rmapDataWind->DWE.class4.val.value = 14;
            rmapDataWind->DWE.class4.confidence.value = 54;
            rmapDataWind->DWE.class5.val.value = 15;
            rmapDataWind->DWE.class5.confidence.value = 55;
            rmapDataWind->DWE.class6.val.value = 35;
            rmapDataWind->DWE.class6.confidence.value = 56;

            rmapDataWind->DWE.metadata.level.LevelType1.value = 103;
            rmapDataWind->DWE.metadata.level.L1.value = 10000;
            rmapDataWind->DWE.metadata.level.LevelType2.value = UINT16_MAX;
            rmapDataWind->DWE.metadata.level.L2.value = UINT16_MAX;

            rmapDataWind->DWE.metadata.timerange.Pindicator.value = 9;
            rmapDataWind->DWE.metadata.timerange.P1.value = 0;
            rmapDataWind->DWE.metadata.timerange.P2 = 900;

            rmapDataWind->DWF.peak.val.value = 342;
            rmapDataWind->DWF.peak.confidence.value = 68;

            rmapDataWind->DWF._long.val.value = 349;
            rmapDataWind->DWF._long.confidence.value = 58;

            rmapDataWind->DWF.metadata.level.LevelType1.value = 103;
            rmapDataWind->DWF.metadata.level.L1.value = 10000;
            rmapDataWind->DWF.metadata.level.LevelType2.value = UINT16_MAX;
            rmapDataWind->DWF.metadata.level.L2.value = UINT16_MAX;

            rmapDataWind->DWF.metadata.timerange.Pindicator.value = 205;
            rmapDataWind->DWF.metadata.timerange.P1.value = 0;
            rmapDataWind->DWF.metadata.timerange.P2 = 900;

            rmap_get_response.rmap_data.module_type = Module_Type::wind;
          }
          else if (countData == 4)
          {
            rmap_get_response.result.end_of_data = 0;

            rmapDataVWC = (rmap_module_VWC_1_0 *)&rmap_get_response.rmap_data;

            rmapDataVWC->VWC.vwc.val.value = 153;
            rmapDataVWC->VWC.vwc.confidence.value = 87;

            rmapDataVWC->VWC.metadata.level.LevelType1.value = 103;
            rmapDataVWC->VWC.metadata.level.L1.value = 100;
            rmapDataVWC->VWC.metadata.level.LevelType2.value = UINT16_MAX;
            rmapDataVWC->VWC.metadata.level.L2.value = UINT16_MAX;

            rmapDataVWC->VWC.metadata.timerange.Pindicator.value = 0;
            rmapDataVWC->VWC.metadata.timerange.P1.value = 0;
            rmapDataVWC->VWC.metadata.timerange.P2 = 900;

            rmap_get_response.rmap_data.module_type = Module_Type::vwc;
          }
          else if (countData == 5)
          {
            rmap_get_response.result.end_of_data = 0;

            rmapDataPower = (rmap_module_Power_1_0 *)&rmap_get_response.rmap_data;

            rmapDataPower->MPP.input_voltage.val.value = 204;
            rmapDataPower->MPP.input_voltage.confidence.value = 95;

            rmapDataPower->MPP.input_current.val.value = 74;
            rmapDataPower->MPP.input_current.confidence.value = 85;

            rmapDataPower->MPP.battery_voltage.val.value = 128;
            rmapDataPower->MPP.battery_voltage.confidence.value = 75;

            rmapDataPower->MPP.battery_voltage.val.value = 15;
            rmapDataPower->MPP.battery_voltage.confidence.value = 65;

            rmapDataPower->MPP.battery_charge.val.value = 89;
            rmapDataPower->MPP.battery_charge.confidence.value = 55;

            rmapDataPower->MPP.metadata.level.LevelType1.value = 265;
            rmapDataPower->MPP.metadata.level.L1.value = 1;
            rmapDataPower->MPP.metadata.level.LevelType2.value = UINT16_MAX;
            rmapDataPower->MPP.metadata.level.L2.value = UINT16_MAX;

            rmapDataPower->MPP.metadata.timerange.Pindicator.value = 0;
            rmapDataPower->MPP.metadata.timerange.P1.value = 0;
            rmapDataPower->MPP.metadata.timerange.P2 = 900;

            rmap_get_response.rmap_data.module_type = Module_Type::power;
          }
          else
          {
            rmap_get_response.result.end_of_data = 1;
            rmap_get_response.result.event_error = 1;
          }

          rmap_get_response.rmap_data.date_time = 1679985000;
          // FINE PROVA
          #endif          

          error = NO_ERROR; // Init NoError MQTT Cyclone

          // EOF Data? (Save and Exit, after last data process)
          rmap_eof = rmap_get_response.result.end_of_data;

          if((!error)&&(!rmap_data_error)&&(rmap_get_response.result.done_get_data)) {
            countData++;
            #if TRACE_LEVEL >= TRACE_VERBOSE
            // ******************************************************************
            // Trace of Current Session Upload CountData and DateTime Block Print
            DateTime rmap_date_time_val;
            char stima_name[STIMA_MODULE_NAME_LENGTH] = {0};
            getStimaNameByType(stima_name, rmap_get_response.rmap_data.module_type);
            convertUnixTimeToDate(rmap_get_response.rmap_data.date_time, &rmap_date_time_val);
            TRACE_VERBOSE_F(F("MQTT: Publish RMAP data count id: [ %d ], module [ %s ], date/time [ %s ]\r\n"), (uint32_t)countData, stima_name, formatDate(&rmap_date_time_val, NULL));
            // ******************************************************************
            #endif
            // Process Data with casting RMAP Module Type
            switch (rmap_get_response.rmap_data.module_type) {
              case Module_Type::th:
                rmapDataTH = (rmap_service_module_TH_Response_1_0 *) rmap_get_response.rmap_data.block;
                #if (ENABLE_STACK_USAGE)
                TaskMonitorStack();
                #endif

                error = NO_ERROR;

                // check if the sensor was configured or not
                for (uint8_t slaveId = 0; slaveId < BOARDS_COUNT_MAX; slaveId++)
                {
                  if (param.configuration->board_slave[slaveId].module_type == Module_Type::th)
                  {
                    if (!error && param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_ITH])
                    {
                      error = publishSensorTH(&mqttClientContext, qos, rmapDataTH->ITH, rmap_date_time_val, param.configuration, topic, sizeof(topic), sensors_topic, sizeof(sensors_topic), message, sizeof(message));
                    }

                    if (!error && param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_NTH])
                    {
                      error = publishSensorTH(&mqttClientContext, qos, rmapDataTH->NTH, rmap_date_time_val, param.configuration, topic, sizeof(topic), sensors_topic, sizeof(sensors_topic), message, sizeof(message));
                    }

                    if (!error && param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_MTH])
                    {
                      error = publishSensorTH(&mqttClientContext, qos, rmapDataTH->MTH, rmap_date_time_val, param.configuration, topic, sizeof(topic), sensors_topic, sizeof(sensors_topic), message, sizeof(message));
                    }

                    if (!error && param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_XTH])
                    {
                      error = publishSensorTH(&mqttClientContext, qos, rmapDataTH->XTH, rmap_date_time_val, param.configuration, topic, sizeof(topic), sensors_topic, sizeof(sensors_topic), message, sizeof(message));
                    }

                    if (error)
                    {
                      // Connection to MQTT server lost?
                      state = MQTT_STATE_DISCONNECT;
                      TRACE_ERROR_F(F("MQTT_STATE_PUBLISH (Error) -> MQTT_STATE_DISCONNECT\r\n"));
                      break;
                    }
                    else
                    {
                      // Starting publishing
                      param.systemStatusLock->Take();
                      param.system_status->connection.is_mqtt_publishing = true;
                      param.system_status->connection.mqtt_data_published++;
                      param.systemStatusLock->Give();
                    }
                    break;
                  }
                }
                break;
              case Module_Type::rain:
                rmapDataRain = (rmap_service_module_Rain_Response_1_0 *)rmap_get_response.rmap_data.block;
                #if (ENABLE_STACK_USAGE)
                TaskMonitorStack();
                #endif

                error = NO_ERROR;

                // check if the sensor was configured or not
                for (uint8_t slaveId = 0; slaveId < BOARDS_COUNT_MAX; slaveId++)
                {
                  if (param.configuration->board_slave[slaveId].module_type == Module_Type::rain)
                  {
                    if (!error && param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_TBR])
                    {
                      error = publishSensorRain(&mqttClientContext, qos, rmapDataRain->TBR, rmap_date_time_val, param.configuration, topic, sizeof(topic), sensors_topic, sizeof(sensors_topic), message, sizeof(message));
                    }

                    if (error)
                    {
                      // Connection to MQTT server lost?
                      state = MQTT_STATE_DISCONNECT;
                      TRACE_ERROR_F(F("MQTT_STATE_PUBLISH (Error) -> MQTT_STATE_DISCONNECT\r\n"));
                      break;
                    }
                    else
                    {
                      // Starting publishing
                      param.systemStatusLock->Take();
                      param.system_status->connection.is_mqtt_publishing = true;
                      param.system_status->connection.mqtt_data_published++;
                      param.systemStatusLock->Give();
                    }
                    break;
                  }
                }

                break;
              case Module_Type::radiation:
                rmapDataRadiation = (rmap_service_module_Radiation_Response_1_0 *)rmap_get_response.rmap_data.block;
                #if (ENABLE_STACK_USAGE)
                TaskMonitorStack();
                #endif

                error = NO_ERROR;

                // check if the sensor was configured or not
                for (uint8_t slaveId = 0; slaveId < BOARDS_COUNT_MAX; slaveId++)
                {
                  if (param.configuration->board_slave[slaveId].module_type == Module_Type::radiation)
                  {
                    if (!error && param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_DSA])
                    {
                      error = publishSensorRadiation(&mqttClientContext, qos, rmapDataRadiation->DSA, rmap_date_time_val, param.configuration, topic, sizeof(topic), sensors_topic, sizeof(sensors_topic), message, sizeof(message));
                    }

                    if (error)
                    {
                      // Connection to MQTT server lost?
                      state = MQTT_STATE_DISCONNECT;
                      TRACE_ERROR_F(F("MQTT_STATE_PUBLISH (Error) -> MQTT_STATE_DISCONNECT\r\n"));
                      break;
                    }
                    else
                    {
                      // Starting publishing
                      param.systemStatusLock->Take();
                      param.system_status->connection.is_mqtt_publishing = true;
                      param.system_status->connection.mqtt_data_published++;
                      param.systemStatusLock->Give();
                    }
                    break;
                  }
                }

                break;
              case Module_Type::wind:
                rmapDataWind = (rmap_service_module_Wind_Response_1_0 *)rmap_get_response.rmap_data.block;
                #if (ENABLE_STACK_USAGE)
                TaskMonitorStack();
                #endif

                error = NO_ERROR;

                // check if the sensor was configured or not
                for (uint8_t slaveId = 0; slaveId < BOARDS_COUNT_MAX; slaveId++)
                {
                  if (param.configuration->board_slave[slaveId].module_type == Module_Type::wind)
                  {
                    if (!error && param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_DWA])
                    {
                      error = publishSensorWindAvgVect10(&mqttClientContext, qos, rmapDataWind->DWA, rmap_date_time_val, param.configuration, topic, sizeof(topic), sensors_topic, sizeof(sensors_topic), message, sizeof(message));
                    }
                    
                    if (!error && param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_DWB])
                    {
                      error = publishSensorWindAvgVect(&mqttClientContext, qos, rmapDataWind->DWB, rmap_date_time_val, param.configuration, topic, sizeof(topic), sensors_topic, sizeof(sensors_topic), message, sizeof(message));
                    }
                    
                    if (!error && param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_DWC])
                    {
                      error = publishSensorWindGustSpeed(&mqttClientContext, qos, rmapDataWind->DWC, rmap_date_time_val, param.configuration, topic, sizeof(topic), sensors_topic, sizeof(sensors_topic), message, sizeof(message));
                    }
                    
                    if (!error && param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_DWD])
                    {
                      error = publishSensorWindAvgSpeed(&mqttClientContext, qos, rmapDataWind->DWD, rmap_date_time_val, param.configuration, topic, sizeof(topic), sensors_topic, sizeof(sensors_topic), message, sizeof(message));
                    }
                    
                    if (!error && param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_DWE])
                    {
                      error = publishSensorWindClassSpeed(&mqttClientContext, qos, rmapDataWind->DWE, rmap_date_time_val, param.configuration, topic, sizeof(topic), sensors_topic, sizeof(sensors_topic), message, sizeof(message));
                    }
                    
                    if (!error && param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_DWF])
                    {
                      error = publishSensorWindGustDirection(&mqttClientContext, qos, rmapDataWind->DWF, rmap_date_time_val, param.configuration, topic, sizeof(topic), sensors_topic, sizeof(sensors_topic), message, sizeof(message));
                    }

                    if (error)
                    {
                      // Connection to MQTT server lost?
                      state = MQTT_STATE_DISCONNECT;
                      TRACE_ERROR_F(F("MQTT_STATE_PUBLISH (Error) -> MQTT_STATE_DISCONNECT\r\n"));
                      break;
                    }
                    else
                    {
                      // Starting publishing
                      param.systemStatusLock->Take();
                      param.system_status->connection.is_mqtt_publishing = true;
                      param.system_status->connection.mqtt_data_published++;
                      param.systemStatusLock->Give();
                    }
                    break;
                  }
                }

                break;
              case Module_Type::vwc:
                rmapDataVWC = (rmap_service_module_VWC_Response_1_0 *)rmap_get_response.rmap_data.block;
                #if (ENABLE_STACK_USAGE)
                TaskMonitorStack();
                #endif

                error = NO_ERROR;

                // check if the sensor was configured or not
                for (uint8_t slaveId = 0; slaveId < BOARDS_COUNT_MAX; slaveId++)
                {
                  if (param.configuration->board_slave[slaveId].module_type == Module_Type::vwc)
                  {
                    if (!error && param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_VWC1])
                    {
                      error = publishSensorSoil(&mqttClientContext, qos, rmapDataVWC->VWC1, rmap_date_time_val, param.configuration, topic, sizeof(topic), sensors_topic, sizeof(sensors_topic), message, sizeof(message));
                    }

                    if (!error && param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_VWC2])
                    {
                      error = publishSensorSoil(&mqttClientContext, qos, rmapDataVWC->VWC2, rmap_date_time_val, param.configuration, topic, sizeof(topic), sensors_topic, sizeof(sensors_topic), message, sizeof(message));
                    }

                    if (!error && param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_VWC3])
                    {
                      error = publishSensorSoil(&mqttClientContext, qos, rmapDataVWC->VWC3, rmap_date_time_val, param.configuration, topic, sizeof(topic), sensors_topic, sizeof(sensors_topic), message, sizeof(message));
                    }

                    if (error)
                    {
                      // Connection to MQTT server lost?
                      state = MQTT_STATE_DISCONNECT;
                      TRACE_ERROR_F(F("MQTT_STATE_PUBLISH (Error) -> MQTT_STATE_DISCONNECT\r\n"));
                      break;
                    }
                    else
                    {
                      // Starting publishing
                      param.systemStatusLock->Take();
                      param.system_status->connection.is_mqtt_publishing = true;
                      param.system_status->connection.mqtt_data_published++;
                      param.systemStatusLock->Give();
                    }
                    break;
                  }
                }

                break;
              case Module_Type::power:
                rmapDataPower = (rmap_service_module_Power_Response_1_0 *)rmap_get_response.rmap_data.block;
                #if (ENABLE_STACK_USAGE)
                TaskMonitorStack();
                #endif

                error = NO_ERROR;

                // check if the sensor was configured or not
                for (uint8_t slaveId = 0; slaveId < BOARDS_COUNT_MAX; slaveId++)
                {
                  if (!error && param.configuration->board_slave[slaveId].module_type == Module_Type::power)
                  {
                    if (param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_MPP])
                    {
                      error = publishSensorPower(&mqttClientContext, qos, rmapDataPower->MPP, rmap_date_time_val, param.configuration, topic, sizeof(topic), sensors_topic, sizeof(sensors_topic), message, sizeof(message));
                    }

                    if (error)
                    {
                      // Connection to MQTT server lost?
                      state = MQTT_STATE_DISCONNECT;
                      TRACE_ERROR_F(F("MQTT_STATE_PUBLISH (Error) -> MQTT_STATE_DISCONNECT\r\n"));
                      break;
                    }
                    else
                    {
                      // Starting publishing
                      param.systemStatusLock->Take();
                      param.system_status->connection.is_mqtt_publishing = true;
                      param.system_status->connection.mqtt_data_published++;
                      param.systemStatusLock->Give();
                    }
                    break;
                  }
                }

                break;
            }
          }
          // Non blocking task
          TaskWatchDog(TASK_WAIT_REALTIME_DELAY_MS);
          Delay(Ticks::MsToTicks(TASK_WAIT_REALTIME_DELAY_MS));
        }
        // TRACE Only Exit End Result code
        if(error) {
          TRACE_ERROR_F(F("MQTT: RMAP Publish Data, exit from upload Data MQTT [ %s ]\r\n"), ERROR_STRING);
        }
        if(rmap_data_error) {
          TRACE_ERROR_F(F("MQTT: RMAP Reading Data, exit from upload Data Queue [ %s ]\r\n"), ERROR_STRING);
        }
        // Trace END Data response
        TRACE_INFO_F(F("Uploading data RMAP Archive [ %s ]. Updated %d record\r\n"), rmap_eof ? OK_STRING : ERROR_STRING, countData);
      }
      // *****************************************
      //  END GET RMAP Data Queue and Append MQTT
      // *****************************************

      param.systemStatusLock->Take();
      is_data_publish_end = true;
      param.systemStatusLock->Give();
      state = MQTT_STATE_DISCONNECT;

      break;

    case MQTT_STATE_DISCONNECT:
      param.systemStatusLock->Take();
      // Remove first connection FLAG (Clear queue of RPC in safety mode)
      // RPC Must ececuted only from next connection without error to remote server
      if(!rmap_data_error) param.system_status->flags.clean_rpc = false;
      param.system_status->connection.is_mqtt_disconnecting = true;
      param.systemStatusLock->Give();

      // publish disconnection message
      snprintf(topic, sizeof(topic), "%s/%s/%s/%07d,%07d/%s/%s", param.configuration->mqtt_maint_topic, param.configuration->mqtt_username, param.configuration->ident, param.configuration->longitude, param.configuration->latitude, param.configuration->network, MQTT_STATUS_TOPIC);
      // Non blocking taskMQTT_NET_WAIT_TIMEOUT_PUBLISH
      TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
      error = mqttClientPublish(&mqttClientContext, topic, MQTT_ON_DISCONNECT_MESSAGE, strlen(MQTT_ON_DISCONNECT_MESSAGE), qos, true, NULL);
      TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
      if (!error)
      {
        TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, MQTT_ON_DISCONNECT_MESSAGE, error ? ERROR_STRING : OK_STRING);
      }

      TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_SUSPEND);
      // Softly disconnect to MQTT Server
      mqttClientDisconnect(&mqttClientContext);
      TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
      TRACE_INFO_F(F("%s Disconnected from mqtt server %s on port %d\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, param.configuration->mqtt_port);

      TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_SUSPEND);
      // Close connection
      mqttClientClose(&mqttClientContext);
      TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);

      TRACE_INFO_F(F("%s Close connection\r\n"), Thread::GetName().c_str(), param.configuration->mqtt_server, param.configuration->mqtt_port);
      
      state = MQTT_STATE_END;
      TRACE_VERBOSE_F(F("MQTT_STATE_DISCONNECT -> MQTT_STATE_END\r\n"));
      break;

    case MQTT_STATE_END:

      mqttClientDeinit(&mqttClientContext);

      // Response direct (error) when connection not complete
      // Only when sequence connection is not completed (otherwise response already sended)
      if(!mqtt_connection_estabilished) {
        // Check retry before error end connection MQTT (DNS Error is connection error than required forced reset connection)
        if (((++retry) < MQTT_TASK_GENERIC_RETRY) && (!param.system_status->connection.is_dns_failed_resolve))
        {
          TaskWatchDog(MQTT_TASK_GENERIC_RETRY_DELAY_MS);
          Delay(Ticks::MsToTicks(MQTT_TASK_GENERIC_RETRY_DELAY_MS));
          TRACE_VERBOSE_F(F("MQTT_STATE_END -> MQTT_STATE_CONNECT\r\n"));
          state = MQTT_STATE_CONNECT;
          break;
        } else {
          // Send response to request
          memset(&connection_response, 0, sizeof(connection_response_t));
          connection_response.error_mqtt_connected = true;
          param.connectionResponseQueue->Enqueue(&connection_response);
        }
      }

      // Exit end complete
      param.systemStatusLock->Take();
      param.system_status->connection.is_mqtt_connected = false;
      param.system_status->connection.is_mqtt_connecting = false;
      param.system_status->connection.is_mqtt_disconnected = true;
      param.system_status->connection.is_mqtt_disconnecting = false;
      param.system_status->connection.is_mqtt_subscribed = false;
      param.system_status->connection.is_mqtt_publishing = false;
      // (true if ok data send, do no clean. Only on Next Connect)
      param.system_status->connection.is_mqtt_publishing_end = is_data_publish_end;
      param.systemStatusLock->Give();

      state = MQTT_STATE_WAIT_NET_EVENT;
      TRACE_VERBOSE_F(F("MQTT_STATE_END -> MQTT_STATE_WAIT_NET_EVENT\r\n"));

      break;
    }

    #if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
    #endif

    // One step base non blocking switch
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_WAIT_DELAY_MS));

  }
}

/**
 * @brief Subscriber callback function
 * @param[in] context Pointer to the MQTT client context
 * @param[in] topic Topic name
 * @param[in] message Message payload
 * @param[in] length Length of the message payload
 * @param[in] dup Duplicate delivery of the PUBLISH packet
 * @param[in] qos QoS level used to publish the message
 * @param[in] retain This flag specifies if the message is to be retained
 * @param[in] packetId Packet identifier
 **/
void MqttTask::mqttPublishCallback(MqttClientContext *context, const char_t *topic, const uint8_t *message, size_t length, bool_t dup, MqttQosLevel qos, bool_t retain, uint16_t packetId)
{
  task_flag old_status_task_flag; // Backup state of flag of TASK State (before suspend for RPC)
  bool is_event_rpc = true;
  char rpc_response[MAXLEN_RPC_RESPONSE] = {0};
 
  TRACE_INFO_F(F("MQTT packet received...\r\n"));
  TRACE_INFO_F(F("Dup: %u\r\n"), dup);
  TRACE_INFO_F(F("QoS: %u\r\n"), qos);
  TRACE_INFO_F(F("Retain: %u\r\n"), retain);
  TRACE_INFO_F(F("Packet Identifier: %u\r\n"), packetId);
  TRACE_INFO_F(F("Message (%" PRIuSIZE " bytes):\r\n"), length);
  TRACE_INFO_F(F("%s %s\r\n"), topic, message);

  // EXCLUDE FIRST RPC CONNECTION... MQTT Clear queue of RPC with external flags
  if(localSystemStatus->flags.clean_rpc) {
    TRACE_INFO_F(F("Excluding all RPC Message for first connection in security mode...\r\n"));
  } else {
    localStreamRpc->init();
    if (localRpcLock->Take())
    {
      while (is_event_rpc)
      {
        // Security lock task_flag for External Local TASK RPC (Need for risk of WDT Reset)
        // Return to previous state on END of RPC Call execution
        old_status_task_flag = localSystemStatus->tasks[LOCAL_TASK_ID].state;
        localSystemStatus->tasks[LOCAL_TASK_ID].state = task_flag::suspended;
        // N.B. Use seial if USE "id": 0 \r\n (optional) if used
        // localStreamRpc->parseCharpointer(&is_event_rpc, (char *)message, length, rpc_response, MAXLEN_RPC_RESPONSE, RPC_TYPE_SERIAL);
        localStreamRpc->parseCharpointer(&is_event_rpc, (char *)message, length, rpc_response, MAXLEN_RPC_RESPONSE, RPC_TYPE_HTTPS);
        localSystemStatus->tasks[LOCAL_TASK_ID].state = old_status_task_flag;
        localSystemStatus->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
      }
      localRpcLock->Give();
      // Response of RPC to path...
      if(strlen(rpc_response)) {
         mqttClientPublish(localPtrMqttClientContext, topic_rpc_response, rpc_response, strlen(rpc_response), qos, true, NULL);
      }
    }
  }
}

error_t MqttTask::makeSensorTopic(rmap_metadata_Metadata_1_0 metadata, char *bvalue, char *sensors_topic, size_t sensors_topic_length)
{
  error_t error = NO_ERROR;
  osMemset(sensors_topic, 0, sensors_topic_length);

  snprintf(sensors_topic, sensors_topic_length, "%d,%d,%d/", metadata.timerange.Pindicator.value, metadata.timerange.P1.value, metadata.timerange.P2);

  if (metadata.level.LevelType1.value != UINT16_MAX)
  {
    if (snprintf(&(sensors_topic[strlen(sensors_topic)]), sensors_topic_length, "%u,", metadata.level.LevelType1.value) <= 0)
    {
      error = ERROR_FAILURE;
    }
  }
  else
  {
    if (snprintf(&(sensors_topic[strlen(sensors_topic)]), sensors_topic_length, "-,") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  if (!error)
  {
    if (metadata.level.L1.value != UINT16_MAX)
    {
      if (snprintf(&(sensors_topic[strlen(sensors_topic)]), sensors_topic_length, "%u,", metadata.level.L1.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(sensors_topic[strlen(sensors_topic)]), sensors_topic_length, "-,") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (metadata.level.LevelType2.value != UINT16_MAX)
    {
      if (snprintf(&(sensors_topic[strlen(sensors_topic)]), sensors_topic_length, "%u,", metadata.level.LevelType2.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(sensors_topic[strlen(sensors_topic)]), sensors_topic_length, "-,") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (metadata.level.L2.value != UINT16_MAX)
    {
      if (snprintf(&(sensors_topic[strlen(sensors_topic)]), sensors_topic_length, "%u", metadata.level.L2.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(sensors_topic[strlen(sensors_topic)]), sensors_topic_length, "-") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (bvalue != NULL)
    {
      if (snprintf(&(sensors_topic[strlen(sensors_topic)]), sensors_topic_length, "/%s", bvalue) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  return error;
}

error_t MqttTask::makeCommonTopic(configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length)
{
  error_t error = NO_ERROR;

  osMemset(topic, 0, topic_length);
  if (snprintf(topic, topic_length, "%s/%s/%s/%07d,%07d/%s/%s", configuration->mqtt_root_topic, configuration->mqtt_username, configuration->ident, configuration->longitude, configuration->latitude, configuration->network, sensors_topic) <= 0)
  {
    error = ERROR_FAILURE;
  }

  return error;
}

error_t MqttTask::makeDate(DateTime dateTime, char *message, size_t message_length)
{
  error_t error = NO_ERROR;
  
  if (snprintf(message, message_length, "\"t\":\"%04u-%02u-%02uT%02u:%02u:%02u\",", dateTime.year, dateTime.month, dateTime.day, dateTime.hours, dateTime.minutes, dateTime.seconds) <= 0)
  {
    error = ERROR_FAILURE;
  }

  return error;
}

// Put data into queue to create File Data Backup Older Format
void MqttTask::putRmapBackupArchiveData(DateTime dateTime, char *topic, char *message)
{
  rmap_backup_data_t archive_backup_data_line = {0};
  size_t lenTopic = strlen(topic);
  size_t lenMessage = strlen(message);
  // Prepare message
  archive_backup_data_line.date_time = convertDateToUnixTime(&dateTime);
  // Check security Len Message before push queue message data
  if((lenTopic + lenMessage) < RMAP_BACKUP_DATA_MAX_ELEMENT_SIZE) {
    strcpy((char*)archive_backup_data_line.block, topic);
    archive_backup_data_line.block[lenTopic] = ' ';
    strcpy((char*)(archive_backup_data_line.block + lenTopic + 1), message);
    // Send to queue with waiting Queue empty from SD Task if Full
    param.dataRmapPutBackupQueue->Enqueue(&archive_backup_data_line, Ticks::MsToTicks(MQTT_PUT_QUEUE_BKP_TIMEOUT_MS));
  }
}

error_t MqttTask::publishSensorTH(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_TH_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length)
{
  uint8_t error_count = 0;
  error_t error = NO_ERROR;

  // ----------------------------------------------------------------------------
  // Temperature
  // ----------------------------------------------------------------------------
  // make temperature topic
  error = makeSensorTopic(sensor.metadata, "B12101", sensors_topic, sensors_topic_length);
  // make temperature message
  if (!error)
  {
    error = makeSensorMessageTemperature(sensor.temperature, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  // publish temperature value
  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish temperature value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  // ----------------------------------------------------------------------------
  // Humidity
  // ----------------------------------------------------------------------------
  // make humidity topic
  if (!error)
  {
    error = makeSensorTopic(sensor.metadata, "B13003", sensors_topic, sensors_topic_length);
  }
  // make humidity message
  if (!error)
  {
    error = makeSensorMessageHumidity(sensor.humidity, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  // publish humidity value
  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish humidity value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  return error;
}

// 1,0,900/103,2000,-,-/B14198 {"v":903,"t":"2019-07-30T11:45:00"}
// 9,0,900/103,6000,-,-/ {"d":51,"p":[100,0,0,0,0,0],"t":"2019-07-30T11:45:00"}
error_t MqttTask::makeSensorMessageTemperature(rmap_measures_Temperature_1_0 temperature, DateTime dateTime, char *message, size_t message_length)
{
  error_t error = NO_ERROR;
  osMemset(message, 0, message_length);

  if (temperature.val.value <= rmap_tableb_B12101_1_0_MAX)
  {
    if (snprintf(message, message_length, "{\"v\":%ld,", temperature.val.value) <= 0)
    {
      error = ERROR_FAILURE;
    }
  }
  else
  {
    if (snprintf(message, message_length, "{\"v\":null,") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  if (!error)
  {
    error = makeDate(dateTime, &(message[strlen(message)]), message_length);
  }

  if (!error)
  {
    if (temperature.confidence.value <= rmap_tableb_B33199_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":%u}", temperature.confidence.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":null}") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (snprintf(&(message[strlen(message)]), message_length, "}") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  return error;
}

error_t MqttTask::makeSensorMessageHumidity(rmap_measures_Humidity_1_0 humidity, DateTime dateTime, char *message, size_t message_length)
{
  error_t error = NO_ERROR;
  osMemset(message, 0, message_length);

  if (humidity.val.value <= rmap_tableb_B13003_1_0_MAX)
  {
    if (snprintf(message, message_length, "{\"v\":%ld,", humidity.val.value) <= 0)
    {
      error = ERROR_FAILURE;
    }
  }
  else
  {
    if (snprintf(&(message[strlen(message)]), message_length, "{\"v\":null,") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  if (!error)
  {
    error = makeDate(dateTime, &(message[strlen(message)]), message_length);
  }

  if (!error)
  {
    if (humidity.confidence.value <= rmap_tableb_B33199_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":%u}", humidity.confidence.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":null}") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (snprintf(&(message[strlen(message)]), message_length, "}") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  return error;
}

error_t MqttTask::makeSensorMessageRain(rmap_measures_Rain_1_0 rain, DateTime dateTime, char *message, size_t message_length)
{
  error_t error = NO_ERROR;
  osMemset(message, 0, message_length);

  if (rain.val.value <= rmap_tableb_B13011_1_0_MAX)
  {
    if (snprintf(message, message_length, "{\"v\":%ld,", rain.val.value) <= 0)
    {
      error = ERROR_FAILURE;
    }
  }
  else
  {
    if (snprintf(message, message_length, "{\"v\":null,") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  if (!error)
  {
    error = makeDate(dateTime, &(message[strlen(message)]), message_length);
  }

  if (!error)
  {
    if (rain.confidence.value <= rmap_tableb_B33199_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":%u}", rain.confidence.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":null}") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (snprintf(&(message[strlen(message)]), message_length, "}") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  return error;
}

error_t MqttTask::publishSensorRain(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_Rain_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length)
{
  uint8_t error_count = 0;
  error_t error = NO_ERROR;

  // ----------------------------------------------------------------------------
  // Rain
  // ----------------------------------------------------------------------------
  // make rain topic
  error = makeSensorTopic(sensor.metadata, "B13011", sensors_topic, sensors_topic_length);
  // make rain message
  if (!error)
  {
    error = makeSensorMessageRain(sensor.rain, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish rain value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  return error;
}

error_t MqttTask::makeSensorMessageRadiation(rmap_measures_Radiation_1_0 radiation, DateTime dateTime, char *message, size_t message_length)
{
  error_t error = NO_ERROR;
  osMemset(message, 0, message_length);

  if (radiation.val.value <= rmap_tableb_B14198_1_0_MAX)
  {
    if (snprintf(message, message_length, "{\"v\":%ld,", radiation.val.value) <= 0)
    {
      error = ERROR_FAILURE;
    }
  }
  else
  {
    if (snprintf(message, message_length, "{\"v\":null,") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  if (!error)
  {
    error = makeDate(dateTime, &(message[strlen(message)]), message_length);
  }

  if (!error)
  {
    if (radiation.confidence.value <= rmap_tableb_B33199_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":%u}", radiation.confidence.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":null}") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (snprintf(&(message[strlen(message)]), message_length, "}") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  return error;
}

error_t MqttTask::publishSensorRadiation(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_Radiation_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length)
{
  uint8_t error_count = 0;
  error_t error = NO_ERROR;

  // ----------------------------------------------------------------------------
  // Radiation
  // ----------------------------------------------------------------------------
  // make radiation topic
  error = makeSensorTopic(sensor.metadata, "B14198", sensors_topic, sensors_topic_length);
  // make radiation message
  if (!error)
  {
    error = makeSensorMessageRadiation(sensor.radiation, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish radiation value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  return error;
}

error_t MqttTask::publishSensorWindAvgVect10(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_WindAvgVect10_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length)
{
  uint8_t error_count = 0;
  error_t error = NO_ERROR;

  // ----------------------------------------------------------------------------
  // Speed
  // ----------------------------------------------------------------------------
  // make speed topic
  error = makeSensorTopic(sensor.metadata, "B11002", sensors_topic, sensors_topic_length);
  // make speed message
  if (!error)
  {
    error = makeSensorMessageSpeed(sensor.speed, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish speed value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  // ----------------------------------------------------------------------------
  // Direction
  // ----------------------------------------------------------------------------
  // make direction topic
  if (!error)
  {
    error = makeSensorTopic(sensor.metadata, "B11001", sensors_topic, sensors_topic_length);
  }
  // make direction message
  if (!error)
  {
    error = makeSensorMessageDirection(sensor.direction, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish direction value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  return error;
}

error_t MqttTask::publishSensorWindAvgVect(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_WindAvgVect_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length)
{
  uint8_t error_count = 0;
  error_t error = NO_ERROR;

  // ----------------------------------------------------------------------------
  // Speed
  // ----------------------------------------------------------------------------
  // make speed topic
  error = makeSensorTopic(sensor.metadata, "B11002", sensors_topic, sensors_topic_length);
  // make speed message
  if (!error)
  {
    error = makeSensorMessageSpeed(sensor.speed, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish speed value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  // ----------------------------------------------------------------------------
  // Direction
  // ----------------------------------------------------------------------------
  // make direction topic
  if (!error)
  {
    error = makeSensorTopic(sensor.metadata, "B11001", sensors_topic, sensors_topic_length);
  }
  // make direction message
  if (!error)
  {
    error = makeSensorMessageDirection(sensor.direction, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish direction value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  return error;
}

error_t MqttTask::publishSensorWindGustSpeed(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_WindGustSpeed_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length)
{
  uint8_t error_count = 0;
  error_t error = NO_ERROR;

  // ----------------------------------------------------------------------------
  // Speed Peak
  // ----------------------------------------------------------------------------
  // make speed peak topic
  error = makeSensorTopic(sensor.metadata, "B11041", sensors_topic, sensors_topic_length);
  // make speed peak message
  if (!error)
  {
    error = makeSensorMessageSpeedPeak(sensor.peak, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish speed peak value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  // ----------------------------------------------------------------------------
  // Speed Long
  // ----------------------------------------------------------------------------
  // make speed long topic
  if (!error)
  {
    error = makeSensorTopic(sensor.metadata, "B11209", sensors_topic, sensors_topic_length);
  }
  // make speed long message
  if (!error)
  {
    error = makeSensorMessageSpeedLong(sensor._long, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish speed long value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  return error;
}

error_t MqttTask::publishSensorWindAvgSpeed(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_WindAvgSpeed_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length)
{
  uint8_t error_count = 0;
  error_t error = NO_ERROR;

  // ----------------------------------------------------------------------------
  // Avg Speed
  // ----------------------------------------------------------------------------
  // make speed peak topic
  error = makeSensorTopic(sensor.metadata, "B11002", sensors_topic, sensors_topic_length);
  // make speed peak message
  if (!error)
  {
    error = makeSensorMessageSpeed(sensor.speed, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish speed peak value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  return error;
}

error_t MqttTask::publishSensorWindClassSpeed(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_WindClassSpeed_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length)
{
  uint8_t error_count = 0;
  error_t error = NO_ERROR;

  // ----------------------------------------------------------------------------
  // Class Speed
  // ----------------------------------------------------------------------------
  // make class speed topic
  error = makeSensorTopic(sensor.metadata, NULL, sensors_topic, sensors_topic_length);
  // make class speed message
  if (!error)
  {
    error = makeSensorMessageClassSpeed(sensor, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish class speed value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  return error;
}

error_t MqttTask::publishSensorWindGustDirection(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_WindGustDirection_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length)
{
  uint8_t error_count = 0;
  error_t error = NO_ERROR;

  // ----------------------------------------------------------------------------
  // Direction Peak
  // ----------------------------------------------------------------------------
  // make peak topic
  error = makeSensorTopic(sensor.metadata, "B11043", sensors_topic, sensors_topic_length);
  // make speed message
  if (!error)
  {
    error = makeSensorMessageDirectionPeak(sensor.peak, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish peak value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  // ----------------------------------------------------------------------------
  // Direction Long
  // ----------------------------------------------------------------------------
  // make direction long topic
  if (!error)
  {
    error = makeSensorTopic(sensor.metadata, "B11210", sensors_topic, sensors_topic_length);
  }
  // make direction long message
  if (!error)
  {
    error = makeSensorMessageDirectionLong(sensor._long, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish direction long value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  return error;
}

// 1,0,900/103,2000,-,-/B14198 {"v":903,"t":"2019-07-30T11:45:00"}
// 9,0,900/103,6000,-,-/ {"d":51,"p":[100,0,0,0,0,0],"t":"2019-07-30T11:45:00"}
error_t MqttTask::makeSensorMessageClassSpeed(rmap_sensors_WindClassSpeed_1_0 sensor, DateTime dateTime, char *message, size_t message_length)
{
  error_t error = NO_ERROR;
  osMemset(message, 0, message_length);

  if (snprintf(message, message_length, "{\"d\":51,\"p\":[") <= 0)
  {
    error = ERROR_FAILURE;
  }

  if (!error)
  {
    if (sensor.class1.val.value <= rmap_tableb_B11211_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "%ld,", sensor.class1.val.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "null,") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (sensor.class2.val.value <= rmap_tableb_B11212_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "%ld,", sensor.class2.val.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "null,") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (sensor.class3.val.value <= rmap_tableb_B11213_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "%ld,", sensor.class3.val.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "null,") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (sensor.class4.val.value <= rmap_tableb_B11214_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "%ld,", sensor.class4.val.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "null,") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (sensor.class5.val.value <= rmap_tableb_B11215_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "%ld,", sensor.class5.val.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "null,") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (sensor.class6.val.value <= rmap_tableb_B11216_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "%ld],", sensor.class6.val.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "null],") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    error = makeDate(dateTime, &(message[strlen(message)]), message_length);
  }

  if (!error)
  {
    if (sensor.class1.confidence.value <= rmap_tableb_B33199_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":[%u,%u,%u,%u,%u,%u]}",
        sensor.class1.confidence.value, sensor.class2.confidence.value, sensor.class3.confidence.value, sensor.class4.confidence.value, sensor.class5.confidence.value, sensor.class6.confidence.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":null}") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (snprintf(&(message[strlen(message)]), message_length, "}") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  return error;
}

error_t MqttTask::makeSensorMessageSpeed(rmap_measures_WindSpeed_1_0 speed, DateTime dateTime, char *message, size_t message_length)
{
  error_t error = NO_ERROR;
  osMemset(message, 0, message_length);

  if (speed.val.value <= rmap_tableb_B11002_1_0_MAX)
  {
    if (snprintf(message, message_length, "{\"v\":%ld,", speed.val.value) <= 0)
    {
      error = ERROR_FAILURE;
    }
  }
  else
  {
    if (snprintf(message, message_length, "{\"v\":null,") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  if (!error)
  {
    error = makeDate(dateTime, &(message[strlen(message)]), message_length);
  }

  if (!error)
  {
    if (speed.confidence.value <= rmap_tableb_B33199_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":%u}", speed.confidence.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":null}") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (snprintf(&(message[strlen(message)]), message_length, "}") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  return error;
}

error_t MqttTask::makeSensorMessageDirection(rmap_measures_WindDirection_1_0 direction, DateTime dateTime, char *message, size_t message_length)
{
  error_t error = NO_ERROR;
  osMemset(message, 0, message_length);

  if (direction.val.value <= rmap_tableb_B11001_1_0_MAX)
  {
    if (snprintf(message, message_length, "{\"v\":%ld,", direction.val.value) <= 0)
    {
      error = ERROR_FAILURE;
    }
  }
  else
  {
    if (snprintf(message, message_length, "{\"v\":null,") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  if (!error)
  {
    error = makeDate(dateTime, &(message[strlen(message)]), message_length);
  }

  if (!error)
  {
    if (direction.confidence.value <= rmap_tableb_B33199_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":%u}", direction.confidence.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":null}") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (snprintf(&(message[strlen(message)]), message_length, "}") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  return error;
}

error_t MqttTask::makeSensorMessageSpeedPeak(rmap_measures_WindPeakGustSpeed_1_0 peak, DateTime dateTime, char *message, size_t message_length)
{
  error_t error = NO_ERROR;
  osMemset(message, 0, message_length);

  if (peak.val.value <= rmap_tableb_B11041_1_0_MAX)
  {
    if (snprintf(message, message_length, "{\"v\":%ld,", peak.val.value) <= 0)
    {
      error = ERROR_FAILURE;
    }
  }
  else
  {
    if (snprintf(message, message_length, "{\"v\":null,") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  if (!error)
  {
    error = makeDate(dateTime, &(message[strlen(message)]), message_length);
  }

  if (!error)
  {
    if (peak.confidence.value <= rmap_tableb_B33199_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":%u}", peak.confidence.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":null}") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (snprintf(&(message[strlen(message)]), message_length, "}") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  return error;
}

error_t MqttTask::makeSensorMessageSpeedLong(rmap_measures_WindLongGustSpeed_1_0 _long, DateTime dateTime, char *message, size_t message_length)
{
  error_t error = NO_ERROR;
  osMemset(message, 0, message_length);

  if (_long.val.value <= rmap_tableb_B11209_1_0_MAX)
  {
    if (snprintf(message, message_length, "{\"v\":%ld,", _long.val.value) <= 0)
    {
      error = ERROR_FAILURE;
    }
  }
  else
  {
    if (snprintf(message, message_length, "{\"v\":null,") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  if (!error)
  {
    error = makeDate(dateTime, &(message[strlen(message)]), message_length);
  }

  if (!error)
  {
    if (_long.confidence.value <= rmap_tableb_B33199_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":%u}", _long.confidence.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":null}") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (snprintf(&(message[strlen(message)]), message_length, "}") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  return error;
}

error_t MqttTask::makeSensorMessageDirectionPeak(rmap_measures_WindPeakGustDirection_1_0 peak, DateTime dateTime, char *message, size_t message_length)
{
  error_t error = NO_ERROR;
  osMemset(message, 0, message_length);

  if (peak.val.value <= rmap_tableb_B11043_1_0_MAX)
  {
    if (snprintf(message, message_length, "{\"v\":%ld,", peak.val.value) <= 0)
    {
      error = ERROR_FAILURE;
    }
  }
  else
  {
    if (snprintf(message, message_length, "{\"v\":null,") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  if (!error)
  {
    error = makeDate(dateTime, &(message[strlen(message)]), message_length);
  }

  if (!error)
  {
    if (peak.confidence.value <= rmap_tableb_B33199_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":%u}", peak.confidence.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":null}") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (snprintf(&(message[strlen(message)]), message_length, "}") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  return error;
}

error_t MqttTask::makeSensorMessageDirectionLong(rmap_measures_WindLongGustDirection_1_0 _long, DateTime dateTime, char *message, size_t message_length)
{
  error_t error = NO_ERROR;
  osMemset(message, 0, message_length);

  if (_long.val.value <= rmap_tableb_B11210_1_0_MAX)
  {
    if (snprintf(message, message_length, "{\"v\":%ld,", _long.val.value) <= 0)
    {
      error = ERROR_FAILURE;
    }
  }
  else
  {
    if (snprintf(message, message_length, "{\"v\":null,") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  if (!error)
  {
    error = makeDate(dateTime, &(message[strlen(message)]), message_length);
  }

  if (!error)
  {
    if (_long.confidence.value <= rmap_tableb_B33199_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":%u}", _long.confidence.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":null}") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (snprintf(&(message[strlen(message)]), message_length, "}") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  return error;
}

error_t MqttTask::publishSensorSoil(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_VWC_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length)
{
  uint8_t error_count = 0;
  error_t error = NO_ERROR;

  // ----------------------------------------------------------------------------
  // Soil
  // ----------------------------------------------------------------------------
  // make soil topic
  error = makeSensorTopic(sensor.metadata, "B13227", sensors_topic, sensors_topic_length);
  // make soil message
  if (!error)
  {
    error = makeSensorMessageSoil(sensor.vwc, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish soil value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  return error;
}

error_t MqttTask::makeSensorMessageSoil(rmap_measures_VolumetricWaterContent_1_0 soil, DateTime dateTime, char *message, size_t message_length)
{
  error_t error = NO_ERROR;
  osMemset(message, 0, message_length);

  if (soil.val.value <= rmap_tableb_B13227_1_0_MAX)
  {
    if (snprintf(message, message_length, "{\"v\":%ld,", soil.val.value) <= 0)
    {
      error = ERROR_FAILURE;
    }
  }
  else
  {
    if (snprintf(message, message_length, "{\"v\":null,") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  if (!error)
  {
    error = makeDate(dateTime, &(message[strlen(message)]), message_length);
  }

  if (!error)
  {
    if (soil.confidence.value <= rmap_tableb_B33199_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":%u}", soil.confidence.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":null}") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (snprintf(&(message[strlen(message)]), message_length, "}") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  return error;
}

error_t MqttTask::publishSensorPower(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_Power_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length)
{
  uint8_t error_count = 0;
  error_t error = NO_ERROR;

  // ----------------------------------------------------------------------------
  // InputVoltage
  // ----------------------------------------------------------------------------
  // make input voltage topic
  error = makeSensorTopic(sensor.metadata, "B25194", sensors_topic, sensors_topic_length);
  // make input voltage message
  if (!error)
  {
    error = makeSensorMessageInputVoltage(sensor.input_voltage, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish input voltage value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  // ----------------------------------------------------------------------------
  // BatteryCurrent
  // ----------------------------------------------------------------------------
  // make battery current topic
  if (!error)
  {
    error = makeSensorTopic(sensor.metadata, "B25193", sensors_topic, sensors_topic_length);
  }
  // make battery current message
  if (!error)
  {
    error = makeSensorMessageBatteryCurrent(sensor.battery_current, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish battery current value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  // ----------------------------------------------------------------------------
  // BatteryCharge
  // ----------------------------------------------------------------------------
  // make battery charge topic
  if (!error)
  {
    error = makeSensorTopic(sensor.metadata, "B25192", sensors_topic, sensors_topic_length);
  }
  // make battery charge message
  if (!error)
  {
    error = makeSensorMessageBatteryCharge(sensor.battery_charge, dateTime, message, message_length);
  }
  // make common topic
  if (!error)
  {
    error = makeCommonTopic(configuration, topic, topic_length, sensors_topic, sensors_topic_length);
  }

  if (!error)
  {
    error_count = 0;
  }
  else
  {
    error_count = MQTT_TASK_PUBLISH_RETRY;
  }

  // Saving Data Backup Older Data Format (if SD Card Ready...)
  if ((!error)&&(param.system_status->flags.sd_card_ready)) {
    putRmapBackupArchiveData(dateTime, topic, message);
  }

  // publish battery charge value
  do
  {
    TaskWatchDog(MQTT_NET_WAIT_TIMEOUT_PUBLISH);
    error = mqttClientPublish(context, topic, message, strlen(message), qos, false, NULL);
    TaskWatchDog(MQTT_TASK_WAIT_DELAY_MS);
    if (error)
    {
      error_count++;
    }
    TaskWatchDog(MQTT_TASK_PUBLISH_DELAY_MS);
    Delay(Ticks::MsToTicks(MQTT_TASK_PUBLISH_DELAY_MS));
  } while (error && (error_count < MQTT_TASK_PUBLISH_RETRY));
  TRACE_DEBUG_F(F("%s%s %s [ %s ]\r\n"), MQTT_PUB_CMD_DEBUG_PREFIX, topic, message, error ? ERROR_STRING : OK_STRING);

  return error;
}

error_t MqttTask::makeSensorMessageInputVoltage(rmap_measures_InputVoltage_1_0 inputVoltage, DateTime dateTime, char *message, size_t message_length)
{
  error_t error = NO_ERROR;
  osMemset(message, 0, message_length);

  if (inputVoltage.val.value <= rmap_tableb_B25194_1_0_MAX)
  {
    if (snprintf(message, message_length, "{\"v\":%ld,", inputVoltage.val.value) <= 0)
    {
      error = ERROR_FAILURE;
    }
  }
  else
  {
    if (snprintf(message, message_length, "{\"v\":null,") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  if (!error)
  {
    error = makeDate(dateTime, &(message[strlen(message)]), message_length);
  }

  if (!error)
  {
    if (inputVoltage.confidence.value <= rmap_tableb_B33199_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":%u}", inputVoltage.confidence.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":null}") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (snprintf(&(message[strlen(message)]), message_length, "}") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  return error;
}

error_t MqttTask::makeSensorMessageBatteryCurrent(rmap_measures_BatteryCurrent_1_0 batteryCurrent, DateTime dateTime, char *message, size_t message_length)
{
  error_t error = NO_ERROR;
  osMemset(message, 0, message_length);

  if ((batteryCurrent.val.value >= rmap_tableb_B25193_1_0_MIN) &&
      (batteryCurrent.val.value <= rmap_tableb_B25193_1_0_MAX))
  {
    if (snprintf(message, message_length, "{\"v\":%ld,", batteryCurrent.val.value) <= 0)
    {
      error = ERROR_FAILURE;
    }
  }
  else
  {
    if (snprintf(message, message_length, "{\"v\":null,") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  if (!error)
  {
    error = makeDate(dateTime, &(message[strlen(message)]), message_length);
  }

  if (!error)
  {
    if (batteryCurrent.confidence.value <= rmap_tableb_B33199_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":%u}", batteryCurrent.confidence.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":null}") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (snprintf(&(message[strlen(message)]), message_length, "}") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  return error;
}

error_t MqttTask::makeSensorMessageBatteryCharge(rmap_measures_BatteryCharge_1_0 batteryCharge, DateTime dateTime, char *message, size_t message_length)
{
  error_t error = NO_ERROR;
  osMemset(message, 0, message_length);

  if (batteryCharge.val.value <= rmap_tableb_B25192_1_0_MAX)
  {
    if (snprintf(message, message_length, "{\"v\":%ld,", batteryCharge.val.value) <= 0)
    {
      error = ERROR_FAILURE;
    }
  }
  else
  {
    if (snprintf(message, message_length, "{\"v\":null,") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  if (!error)
  {
    error = makeDate(dateTime, &(message[strlen(message)]), message_length);
  }

  if (!error)
  {
    if (batteryCharge.confidence.value <= rmap_tableb_B33199_1_0_MAX)
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":%u}", batteryCharge.confidence.value) <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
    else
    {
      if (snprintf(&(message[strlen(message)]), message_length, "\"a\":{\"B33199\":null}") <= 0)
      {
        error = ERROR_FAILURE;
      }
    }
  }

  if (!error)
  {
    if (snprintf(&(message[strlen(message)]), message_length, "}") <= 0)
    {
      error = ERROR_FAILURE;
    }
  }

  return error;
}


/**
 * @brief TLS initialization callback
 * @param[in] context Pointer to the MQTT client context
 * @param[in] tlsContext Pointer to the TLS context
 * @return Error code
 **/
error_t MqttTask::mqttTlsInitCallback(MqttClientContext *context, TlsContext *tlsContext)
{
  error_t error;

  //Debug message
  TRACE_INFO_F(F("MQTT TLS initialization callback\r\n"));

  //Set the PRNG algorithm to be used
  error = tlsSetPrng(tlsContext, YARROW_PRNG_ALGO, MqttYarrowContext);
  //Any error to report?
  if(error)
    return error;

  //Preferred cipher suite list
  error = tlsSetCipherSuites(tlsContext, MqttCipherSuites, sizeof(MqttCipherSuites));
  // Any error to report?
  if(error)
    return error;

  //Set the fully qualified domain name of the server
  error = tlsSetServerName(tlsContext, MqttServer);
  //Any error to report?
  if(error)
    return error;

  //Set the PSK identity to be used by the client
  error = tlsSetPskIdentity(tlsContext, MqttClientPSKIdentity);
  //Any error to report?
  if(error)
    return error;

  //Set the pre-shared key to be used
  error = tlsSetPsk(tlsContext, MqttClientPSKKey, CLIENT_PSK_KEY_LENGTH);
  //Any error to report?
  if(error)
    return error;

  //Successful processing
  return NO_ERROR;
}

#endif