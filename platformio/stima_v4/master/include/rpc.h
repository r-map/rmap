/**
  ******************************************************************************
  * @file    rpc.h
  * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   RPC Init and callback function header file
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
#include "stima_utility.h"
#include "task_util.h"
#include "drivers/module_master_hal.hpp"

#include <STM32RTC.h>
#include "STM32LowPower.h"

#include <IWatchdog.h>

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "semaphore.hpp"
#include "queue.hpp"

#include <arduinoJsonRPC.h>

#include "debug_F.h"

using namespace cpp_freertos;

void init_rpc(JsonRPC *streamRpc);

#if (USE_RPC_METHOD_CONFIGURE)
int configure(JsonObject params, JsonObject result);
#endif

#if (USE_RPC_METHOD_RECOVERY && USE_MQTT)
int recovery(JsonObject params, JsonObject result);
#endif

#if (USE_RPC_METHOD_PREPARE)
int prepare(JsonObject params, JsonObject result);
#endif

#if (USE_RPC_METHOD_GETJSON)
int getjson(JsonObject params, JsonObject result);
#endif

#if (USE_RPC_METHOD_PREPANDGET)
int prepandget(JsonObject params, JsonObject result);
#endif

#if (USE_RPC_METHOD_REBOOT)
int reboot(JsonObject params, JsonObject result);
#endif

#if (USE_RPC_METHOD_TEST)
int rpctest(JsonObject params, JsonObject result);
#endif

#endif
