/**
  ******************************************************************************
  * @file    can_task.h
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Uavcan over CanBus cpp_Freertos header file
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

#include <STM32RTC.h>
#include <STM32LowPower.h>

// Configurazione modulo, definizioni ed utility generiche
#include "canard_config.hpp"

// Register EEprom
#include "register_class.hpp"

// Flash Access
#include "drivers/flash.h"

// Classe Canard
#include "canard_class_leaf.hpp"
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
#include <rmap/_module/Leaf_1_0.h>
#include <rmap/service/_module/Leaf_1_0.h>
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
#define CAN_TASK_SLEEP_DELAY_MS         (850)

// Task waiting Semaphore Driver access
#define CAN_SEMAPHORE_MAX_WAITING_TIME_MS (1000)
#define FLASH_SEMAPHORE_MAX_WAITING_TIME_MS (3000)

// Bit flag return maintenance mode on state command
#define CAN_FLAG_IS_MAINTENANCE_MODE    (0x08)
#define CAN_FLAG_MASK_MAINTENANCE_MODE  (0x07)

// Debug Check Enable Function
// #define LOG_RX_PACKET
// #define LED_ON_SYNCRO_TIME

/// @brief Mode Power HW CanBus Controller state
enum CAN_ModePower {
    CAN_INIT,             //!< CAN is in init or configuration mode
    CAN_NORMAL,           //!< CAN is in normal state (TX and RX Ready)
    CAN_LISTEN_ONLY,      //!< CAN in only listen mode (turn off TX board)
    CAN_SLEEP             //!< Power CAN is OFF
};

/// @brief struct local elaborate data parameter
typedef struct
{
  configuration_t *configuration;                     //!< system configuration pointer struct
  system_status_t *system_status;                     //!< system status pointer struct
  bootloader_t *boot_request;                         //!< Boot struct pointer
  cpp_freertos::BinarySemaphore *configurationLock;   //!< Semaphore to configuration access
  cpp_freertos::BinarySemaphore *systemStatusLock;    //!< Semaphore to system status access
  cpp_freertos::BinarySemaphore *registerAccessLock;  //!< Semaphore to register Cyphal access
  cpp_freertos::BinarySemaphore *canLock;             //!< Semaphore to CAN Bus access
  cpp_freertos::BinarySemaphore *qspiLock;            //!< Semaphore to QSPI Memory flash access
  cpp_freertos::BinarySemaphore *rtcLock;             //!< Semaphore to RTC Access
  cpp_freertos::Queue *systemMessageQueue;            //!< Queue for system message
  cpp_freertos::Queue *requestDataQueue;              //!< Queue to request data
  cpp_freertos::Queue *reportDataQueue;               //!< Queue to report data
  Flash *flash;                                       //!< Object Flash C++ access
  EEprom *eeprom;                                     //!< Object EEprom C++ access
  EERegister *clRegister;                             //!< Object Register C++ access
} CanParam_t;

/// @brief CAN TASK cpp_freertos class
class CanTask : public cpp_freertos::Thread {

  /// @brief Enum for state switch of running method
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
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  static void HW_CAN_Power(CAN_ModePower ModeCan);
  static void getUniqueID(uint8_t out[uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_], uint64_t serNumb);
  static CanardPortID getModeAccessID(uint8_t modeAccessID, const char* const port_name, const char* const type_name);
  static bool putFlashFile(const char* const file_name, const bool is_firmware, const bool rewrite, void* buf, size_t count);
  static bool getFlashFwInfoFile(uint8_t *module_type, uint8_t *version, uint8_t *revision, uint64_t *len);
  static void prepareSensorsDataValue(uint8_t const sensore, const report_t *report, rmap_module_Leaf_1_0 *rmap_data);
  static void prepareSensorsDataValue(uint8_t const sensore, const report_t *report, rmap_service_module_Leaf_Response_1_0 *rmap_data);
  static void publish_rmap_data(canardClass &clsCanard, CanParam_t *param);
  static void processMessagePlugAndPlayNodeIDAllocation(canardClass &clsCanard,  const uavcan_pnp_NodeIDAllocationData_1_0* const msg);
  static uavcan_node_ExecuteCommand_Response_1_1 processRequestExecuteCommand(canardClass &clsCanard, const uavcan_node_ExecuteCommand_Request_1_1* req, uint8_t remote_node);
  static rmap_service_module_Leaf_Response_1_0 processRequestGetModuleData(canardClass &clsCanard, rmap_service_module_Leaf_Request_1_0* req, CanParam_t *param);
  static uavcan_register_Access_Response_1_0 processRequestRegisterAccess(const uavcan_register_Access_Request_1_0* req);
  static uavcan_node_GetInfo_Response_1_0 processRequestNodeGetInfo();
  static void processRequestUpdateRTC(canardClass &clsCanard, const uavcan_pnp_NodeIDAllocationData_1_0* const msg);
  static void processReceivedTransfer(canardClass &clsCanard, const CanardRxTransfer* const transfer, void *param);

  State_t state;
  CanParam_t param;

  // Acces static memeber parameter of class
  inline static bootloader_t *boot_state;
  inline static EEprom *localEeprom;
  inline static cpp_freertos::Queue *localSystemMessageQueue;
  inline static uint16_t last_req_rpt_time = (REPORTS_TIME_S);
  inline static uint16_t last_req_obs_time = (OBSERVATIONS_TIME_S);
  inline static CAN_ModePower canPower;
  inline static STM32RTC& rtc = STM32RTC::getInstance();
  // Register access && Flash (Firmware and data log archive)
  inline static EERegister *localRegister;
  inline static cpp_freertos::BinarySemaphore *localQspiLock;
  inline static cpp_freertos::BinarySemaphore *localRegisterAccessLock;
  inline static Flash *localFlash;
  inline static uint64_t canFlashPtr = 0;
  inline static uint16_t canFlashBlock = 0;
};

#endif
