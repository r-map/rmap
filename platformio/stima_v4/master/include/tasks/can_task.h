/**
 ******************************************************************************
 * @file    can_task.h
 * @author  Moreno Gasperini <m.gasperini@digiteco.it>
 * @brief   Uavcan over CanBus cpp_Freertos header file
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

#ifndef _CAN_TASK_H
#define _CAN_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "str.h"
#include "stima_utility.h"

#if (ENABLE_CAN)

#include <STM32FreeRTOS.h>
#include <arduinoJsonRPC.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_master_hal.hpp"

#include <STM32RTC.h>

// Configurazione modulo, definizioni ed utility generiche
#include "canard_config.hpp"

// Register EEprom
#include "register_class.hpp"

// Flash Access
#include "drivers/flash.h"

// Classe Canard
#include "canard_class_master.hpp"
// Libcanard
#include <canard.h>
#include "bxcan.h"
// Namespace UAVCAN
#include <uavcan/node/Heartbeat_1_0.h>
#include <uavcan/node/GetInfo_1_0.h>
#include <uavcan/node/ExecuteCommand_1_1.h>
#include <uavcan/node/port/List_0_1.h>
#include <uavcan/_register/Access_1_0.h>
#include <uavcan/_register/List_1_0.h>
#include <uavcan/file/Read_1_1.h>
#include <uavcan/time/Synchronization_1_0.h>
#include <uavcan/pnp/NodeIDAllocationData_1_0.h>
// Namespace RMAP
#include <rmap/_module/TH_1_0.h>
#include <rmap/service/_module/TH_1_0.h>
// Standard Library
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#include "debug_F.h"

using namespace cpp_freertos;

// Main TASK Switch Delay
#define CAN_TASK_WAIT_DELAY_MS          (20)
#define CAN_TASK_SLEEP_DELAY_MS         (700)

// Task waiting queue command/response
#define WAIT_QUEUE_REQUEST_ELABDATA_MS  (50)
#define WAIT_QUEUE_RESPONSE_ELABDATA_MS (50)
#define WAIT_QUEUE_REQUEST_COMMAND_MS   (500)

// Task waiting Semaphore Driver access
#define CAN_SEMAPHORE_MAX_WAITING_TIME_MS (1500)

#define CAN_PUT_QUEUE_RMAP_TIMEOUT_MS     (2500)

#define GENERIC_UAVCAN_MAX_RETRY          (3)

#define SEC_WAKE_UP_MODULE_FOR_QUERY      (5)   // Time to WakeUP Node for FullPower Request

#define MIN_TRANSACTION_HEARTBEAT_RX      (SEC_WAKE_UP_MODULE_FOR_QUERY)
#define MIN_VALID_HEARTBEAT_RX            (20)

// Debug Check Enable Function
// #define LOG_RX_PACKET

// Register sequence programming (Remote register phase [State Receive = SEND + 1])
#define REGISTER_STARTING   1u
#define REGISTER_01_SEND    1u
#define REGISTER_02_SEND    3u
#define REGISTER_03_SEND    5u
#define REGISTER_04_SEND    7u
#define REGISTER_05_SEND    9u
#define REGISTER_06_SEND    11u
#define REGISTER_07_SEND    13u
#define REGISTER_08_SEND    15u
#define REGISTER_COMPLETE   17u

// Mode Power HW CanBus Controller
enum CAN_ModePower
{
  CAN_INIT,
  CAN_NORMAL,
  CAN_LISTEN_ONLY,
  CAN_SLEEP
};

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  bootloader_t *boot_request;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::BinarySemaphore *registerAccessLock;  
  cpp_freertos::BinarySemaphore *canLock;
  cpp_freertos::BinarySemaphore *qspiLock;
  cpp_freertos::BinarySemaphore *rtcLock;
  cpp_freertos::BinarySemaphore *rpcLock;
  cpp_freertos::Queue *systemMessageQueue;
  cpp_freertos::Queue *dataLogPutQueue;
  cpp_freertos::Queue *dataRmapPutQueue;
  cpp_freertos::Queue *dataFileGetRequestQueue;
  cpp_freertos::Queue *dataFileGetResponseQueue;
  Flash *flash;
  EEprom *eeprom;
  EERegister *clRegister;
  JsonRPC *streamRpc;
} CanParam_t;

class CanTask : public cpp_freertos::Thread {
  typedef enum {
    CAN_STATE_CREATE,
    CAN_STATE_INIT,
    CAN_STATE_SETUP,
    CAN_STATE_CHECK
  } State_t;

public:
  CanTask(const char *taskName, uint16_t stackSize, uint8_t priority, CanParam_t canParam);

protected:
  virtual void Run();

private:

  #if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
  #endif
  void TaskWatchDog(uint32_t millis_standby);
  static void LocalTaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  static void HW_CAN_Power(CAN_ModePower ModeCan);
  static void getUniqueID(uint8_t out[uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_], uint64_t serNumb);
  static bool putFlashFile(const char* const file_name, const bool is_firmware, const bool rewrite, void* buf, size_t count);
  static bool getFlashFwInfoFile(uint8_t *module_type, uint8_t *version, uint8_t *revision, uint64_t *len);
  static uavcan_node_ExecuteCommand_Response_1_1 processRequestExecuteCommand(canardClass &clsCanard, const uavcan_node_ExecuteCommand_Request_1_1* req, uint8_t remote_node);
  static uavcan_register_Access_Response_1_0 processRequestRegisterAccess(const uavcan_register_Access_Request_1_0* req);
  static uavcan_node_GetInfo_Response_1_0 processRequestNodeGetInfo();
  static void processReceivedTransfer(canardClass &clsCanard, const CanardRxTransfer* const transfer);

  // Param local and state
  State_t state;
  CanParam_t param;

  // Acces static memeber parameter of class
  inline static cpp_freertos::Queue *localSystemMessageQueue;
  inline static cpp_freertos::Queue *localDataFileGetRequestQueue;
  inline static cpp_freertos::Queue *localDataFileGetResponseQueue;
  // Upload file to can (File server)
  inline static uint8_t firmwareState;
  inline static file_get_request_t firmwareDownloadChunck;
  inline static file_get_response_t sdcard_task_response;
  // Power module CAN
  inline static CAN_ModePower canPower;
  // RTC
  inline static STM32RTC& rtc = STM32RTC::getInstance();
  // Register access && Flash (Firmware and data log archive)
  inline static EERegister *localRegister;
  inline static cpp_freertos::BinarySemaphore *localQspiLock;
  inline static cpp_freertos::BinarySemaphore *localRegisterAccessLock;
  inline static cpp_freertos::BinarySemaphore *localSystemStatusLock;
  inline static cpp_freertos::BinarySemaphore *localRpcLock;
  inline static system_status_t *localSystemStatus;
  inline static JsonRPC *localStreamRpc;
  inline static Flash *localFlash;
  inline static uint64_t canFlashPtr = 0;
  inline static uint16_t canFlashBlock = 0;  
};

#endif
#endif