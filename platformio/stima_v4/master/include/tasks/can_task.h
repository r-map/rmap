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
#define CAN_TASK_WAIT_MAXSPEED_DELAY_MS (1)
#define CAN_TASK_SLEEP_DELAY_MS         (1250)

// Task waiting queue command/response
#define WAIT_QUEUE_REQUEST_ELABDATA_MS  (50)
#define WAIT_QUEUE_RESPONSE_ELABDATA_MS (50)
#define WAIT_QUEUE_REQUEST_COMMAND_MS   (500)

// Task waiting Semaphore Driver access
#define CAN_SEMAPHORE_MAX_WAITING_TIME_MS (1000)
#define FLASH_SEMAPHORE_MAX_WAITING_TIME_MS (3000)

// Debug Check Enable Function
// #define LOG_RX_PACKET
// #define LED_ON_SYNCRO_TIME

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
  TwoWire *wire;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::BinarySemaphore *registerAccessLock;  
  cpp_freertos::BinarySemaphore *wireLock;
  cpp_freertos::BinarySemaphore *canLock;
  cpp_freertos::BinarySemaphore *qspiLock;
  cpp_freertos::Queue *systemMessageQueue;
  // cpp_freertos::Queue *requestDataQueue;
  // cpp_freertos::Queue *reportDataQueue;
} CanParam_t;

class CanTask : public cpp_freertos::Thread {
  typedef enum {
    INIT,
    SETUP,
    STANDBY
  } State_t;

public:
  CanTask(const char *taskName, uint16_t stackSize, uint8_t priority, CanParam_t CanParam);

protected:
  virtual void Run();

private:

  static void HW_CAN_Power(CAN_ModePower ModeCan);
  static void getUniqueID(uint8_t out[uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_]);
  static bool putDataFile(const char* const file_name, const bool is_firmware, const bool rewrite, void* buf, size_t count);
  static bool getInfoFwFile(uint8_t *version, uint8_t *revision, uint64_t *len);
  static uavcan_node_ExecuteCommand_Response_1_1 processRequestExecuteCommand(canardClass &clsCanard, const uavcan_node_ExecuteCommand_Request_1_1* req, uint8_t remote_node);
  static uavcan_register_Access_Response_1_0 processRequestRegisterAccess(const uavcan_register_Access_Request_1_0* req);
  static uavcan_node_GetInfo_Response_1_0 processRequestNodeGetInfo();
  static void processReceivedTransfer(canardClass &clsCanard, const CanardRxTransfer* const transfer);

  uint16_t stackSize;
  State_t state;
  EEprom memEprom;
  CanParam_t param;
  inline static cpp_freertos::Queue *localSystemMessageQueue;
  inline static CAN_ModePower canPower;
  inline static STM32RTC& rtc = STM32RTC::getInstance();
  // Register access && Flash (Firmware and data log archive)
  inline static EERegister clRegister;
  inline static Flash memFlash;
  inline static cpp_freertos::BinarySemaphore *localQspiLock;
  inline static cpp_freertos::BinarySemaphore *localRegisterAccessLock;
  inline static uint64_t flashPtr = 0;
  inline static uint16_t flashBlock = 0;
};

#endif
