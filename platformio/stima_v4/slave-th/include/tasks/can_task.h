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

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_slave_hal.hpp"

// Configurazione modulo, definizioni ed utility generiche
#include "canard_config.hpp"

// Classe Canard
#include "canard_class_th.hpp"
// Libcanard
#include "register_class.hpp"
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
#define CAN_TASK_BASE_DELAY_MS  (10)

#define WAIT_QUEUE_REQUEST_ELABDATA_MS (50)
#define WAIT_QUEUE_RESPONSE_ELABDATA_MS (50)

// Debug Check Enable Function
#define LOG_RX_PACKET
#define LED_ON_SYNCRO_TIME

// Mode Power HW CanBus Controller
enum CAN_ModePower {
    CAN_INIT,
    CAN_NORMAL,
    CAN_LISTEN_ONLY,
    CAN_SLEEP
};

typedef struct
{
  configuration_t *configuration;
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::Queue *systemRequestQueue;
  cpp_freertos::Queue *systemResponseQueue;
  cpp_freertos::BinarySemaphore *wireLock;
  TwoWire *wire;
  cpp_freertos::Queue *requestDataQueue;
  cpp_freertos::Queue *reportDataQueue;
} CanParam_t;

class CanTask : public cpp_freertos::Thread {
  typedef enum {
    INIT,
    SETUP,
    STANDBY,
    SLEEP
  } State_t;

public:
  CanTask(const char *taskName, uint16_t stackSize, uint8_t priority, CanParam_t canParam);

protected:
  virtual void Run();

private:

  static void HW_CAN_Power(CAN_ModePower ModeCan);
  static void getUniqueID(uint8_t out[uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_]);
  static CanardPortID getModeAccessID(uint8_t modeAccessID, const char* const port_name, const char* const type_name);
  static rmap_sensors_TH_1_0 prepareSensorsDataValue(uint8_t const sensore, const report_t *report);
  static void publish_rmap_data(canardClass &clsCanard, CanParam_t *param);
  static void processMessagePlugAndPlayNodeIDAllocation(canardClass &clsCanard,  const uavcan_pnp_NodeIDAllocationData_1_0* const msg);
  static uavcan_node_ExecuteCommand_Response_1_1 processRequestExecuteCommand(canardClass &clsCanard, const uavcan_node_ExecuteCommand_Request_1_1* req, uint8_t remote_node);
  static rmap_service_module_TH_Response_1_0 processRequestGetModuleData(canardClass &clsCanard, rmap_service_module_TH_Request_1_0* req, CanParam_t *param);
  static uavcan_register_Access_Response_1_0 processRequestRegisterAccess(const uavcan_register_Access_Request_1_0* req);
  static uavcan_node_GetInfo_Response_1_0 processRequestNodeGetInfo();
  static void processReceivedTransfer(canardClass &clsCanard, const CanardRxTransfer* const transfer, void *param);

  char taskName[configMAX_TASK_NAME_LEN];
  uint16_t stackSize;
  uint8_t priority;
  CanParam_t param;
  State_t state;
  inline static uint16_t last_req_obs_time = (REPORTS_TIME_S);
  inline static CAN_ModePower canPower;
  // Register access
  inline static EERegister clRegister;
};

#endif
