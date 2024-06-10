/**
  ******************************************************************************
  * @file    rpc.hpp
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

#ifndef _RPC_H
#define _RPC_H

#include "config.h"
#include "register_class.hpp"

#include "debug_config.h"
#include "local_typedef.h"
#include "stima_utility.h"
#include "str.h"
#include "task_util.h"
#include "drivers/module_master_hal.hpp"

#include <STM32RTC.h>
#include "STM32LowPower.h"

#include <IWatchdog.h>

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "semaphore.hpp"
#include "queue.hpp"

#include "date_time.h"

#include "canard_config.hpp"

#include <arduinoJsonRPC.h>

#include "drivers/eeprom.h"

#include "debug_F.h"

using namespace cpp_freertos;

#define SUBJECT_ID_LEN_MAX  (30)
#define UNKNOWN_ID          (0xFFu)
#define SETUP_ID            (0xF0u)

/// @brief struct RpcParam_t global paramater config access
typedef struct
{
    configuration_t *configuration;                     ///< System configuration structure
    system_status_t *system_status;                     ///< System status structure
    bootloader_t *boot_request;                         ///< Bootloader structure
    cpp_freertos::BinarySemaphore *rtcLock;             ///< Semaphore for RTC access
    cpp_freertos::BinarySemaphore *configurationLock;   ///< Semaphore for configuration access
    cpp_freertos::BinarySemaphore *systemStatusLock;    ///< Semaphore for system access
    cpp_freertos::BinarySemaphore *rpcLock;             ///< Semaphore for RPC
    cpp_freertos::Queue *systemMessageQueue;            ///< Access to systemMessage queue
    cpp_freertos::Queue *dataLogPutQueue;               ///< Access to Log queue
    cpp_freertos::Queue *dataRmapGetRequestQueue;       ///< Access to Data RMAP queue Request
    cpp_freertos::Queue *dataRmapGetResponseQueue;      ///< Access to Data RMAP queue Response
    EEprom *eeprom;                                     ///< Eeprom Class
} RpcParam_t;

/// @brief Class Register RPC - Manage local RPC as object
class RegisterRPC {

    // ***************** PUBLIC ACCESS *****************
    public: 
        
        RegisterRPC();                      ///< Constructor of class
        RegisterRPC(RpcParam_t rpcParam);   ///< Constructor of class with param

        void init(JsonRPC *streamRpc);      ///< Init JSON Rpc

        #if (USE_RPC_METHOD_ADMIN)
        static int admin(JsonObject params, JsonObject result);       ///< Method Admin
        #endif

        #if (USE_RPC_METHOD_CONFIGURE)
        static int configure(JsonObject params, JsonObject result);   ///< Method configure
        #endif

        #if (USE_RPC_METHOD_UPDATE)
        static int update(JsonObject params, JsonObject result);      ///< Method update
        #endif

        #if (USE_RPC_METHOD_RECOVERY)
        static int recovery(JsonObject params, JsonObject result);    ///< Method recovery
        #endif

        #if (USE_RPC_METHOD_PREPARE)
        static int prepare(JsonObject params, JsonObject result);     ///< Method prepare
        #endif

        #if (USE_RPC_METHOD_GETJSON)
        static int getjson(JsonObject params, JsonObject result);     ///< Method getjson
        #endif

        #if (USE_RPC_METHOD_PREPANDGET)
        static int prepandget(JsonObject params, JsonObject result);  ///< Method prepandget
        #endif

        #if (USE_RPC_METHOD_REBOOT)
        static int reboot(JsonObject params, JsonObject result);      ///< Method reboot
        #endif

        #if (USE_RPC_METHOD_TEST)
        static int rpctest(JsonObject params, JsonObject result);     ///< Method test (purpose only)
        #endif

    // ***************** PRIVATE ACCESS *****************
    private:

      static bool saveConfiguration(void);                                    ///< Saving configuration
      static void initFixedConfigurationParam(uint8_t lasNodeConfig);         ///< Init default configuration with fixed value

      static bool ASCIIHexToDecimal(char** str, uint8_t *value_out);          ///< Convert ASCII HEX to Decimal for JSON Convert

      inline static RpcParam_t param;                                         ///< Access to paramaeter configuration

      inline static char boardName[BOARDSLUG_LENGTH] = {0};                   ///< Name board (boardslug) pre loaded for saving in config param
      inline static uint64_t boardSN = {0};                                   ///< S.N. board pre loaded for saving in config param
      inline static bool isSlaveConfigure = false;                            ///< is module actual in reconfiguration
      inline static bool isMasterConfigure = false;                           ///< is master actual in reconfiguration
      inline static bool is_configuration_changed = false;                    ///< configuration was changed (Need reset PNP Slave?)
      inline static Module_Type currentModule = Module_Type::undefined;       ///< Current type module of Master or Slave actual in reconfiguration
      inline static uint8_t slaveId = UNKNOWN_ID;                             ///< Index of slave in reconfiguration stimacan"X"
      inline static uint8_t sensorId = UNKNOWN_ID;                            ///< Sensor index in actual reconfiguration (ITH/MTH...)
      inline static uint8_t sensorMultyId = UNKNOWN_ID;                       ///< Sensor index in actual reconfiguration (with same index type sequence, multi sensor as soil vwc)
      inline static uint8_t id_constant_data = 0;                             ///< Constant data pointer in reconfiguration of master
      inline static char subject[SUBJECT_ID_LEN_MAX];                         ///< subject string for module check in reconfiguration
      inline static uint8_t uavcanRegisterNodeId = 0;                         ///< register nodeId for RPC remote update on request configure method
      inline static char uavcanRegisterName[MEM_UAVCAN_LEN_INTEST_REG] = {0}; ///< register name for RPC remote update on request configure method
      inline static uint8_t uavcanRegisterTypeValue = 0;                      ///< register selected type for RPC remote update on request configure method
};

#endif
