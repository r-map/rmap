/**
  ******************************************************************************
  * @file    rpc.hpp
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

#ifndef _RPC_H
#define _RPC_H

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

#include "canard_config.hpp"

#include <arduinoJsonRPC.h>

#include "drivers/eeprom.h"

#include "debug_F.h"

using namespace cpp_freertos;

#define SUBJECT_ID_LEN_MAX  (30)
#define UNKNOWN_ID          (0xFFu)

typedef struct
{
    configuration_t *configuration;
    system_status_t *system_status;
    cpp_freertos::BinarySemaphore *rtcLock;
    cpp_freertos::BinarySemaphore *configurationLock;
    cpp_freertos::BinarySemaphore *systemStatusLock;
    cpp_freertos::BinarySemaphore *rpcLock;
    cpp_freertos::Queue *systemMessageQueue;
    EEprom *eeprom;
} RpcParam_t;

// Class Register RPC - Manage local RPC as object
class RegisterRPC {

    // ***************** PUBLIC ACCESS *****************
    public: 

        // Constructor di classe
        RegisterRPC();
        RegisterRPC(RpcParam_t rpcParam);

        void init(JsonRPC *streamRpc);

        #if (USE_RPC_METHOD_ADMIN)
        static int admin(JsonObject params, JsonObject result);
        #endif

        #if (USE_RPC_METHOD_CONFIGURE)
        static int configure(JsonObject params, JsonObject result);
        #endif

        #if (USE_RPC_METHOD_RECOVERY && USE_MQTT)
        static int recovery(JsonObject params, JsonObject result);
        #endif

        #if (USE_RPC_METHOD_PREPARE)
        static int prepare(JsonObject params, JsonObject result);
        #endif

        #if (USE_RPC_METHOD_GETJSON)
        static int getjson(JsonObject params, JsonObject result);
        #endif

        #if (USE_RPC_METHOD_PREPANDGET)
        static int prepandget(JsonObject params, JsonObject result);
        #endif

        #if (USE_RPC_METHOD_REBOOT)
        static int reboot(JsonObject params, JsonObject result);
        #endif

        #if (USE_RPC_METHOD_TEST)
        static int rpctest(JsonObject params, JsonObject result);
        #endif

    // ***************** PRIVATE ACCESS *****************
    private:

      static bool saveConfiguration(bool is_default);
      inline static RpcParam_t param;

      inline static bool is_configuration_changed = false;
      inline static Module_Type currentModule = Module_Type::undefined;
      inline static uint8_t slaveId = UNKNOWN_ID;
      inline static uint8_t sensorId = UNKNOWN_ID;
      inline static bool isSlaveConfigure = false;
      inline static bool isMasterConfigure = false;
      inline static char subject[SUBJECT_ID_LEN_MAX];

};

#endif
