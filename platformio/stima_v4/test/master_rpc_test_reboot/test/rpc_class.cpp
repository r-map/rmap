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

#include "unity.h"

// ************************************************************************
// ******************* TEST RPC FUNCTION DECLARATIONS *********************
// ************************************************************************

void test_rpc_init(void);
void test_rpc_test(void);

// ************************************************************************
// ******************* TEST RPC FUNCTION IMPLEMENTATIONS ******************
// ************************************************************************

/**
 * @brief TEST: Init RPC Class Object and Register method CallBack
 *
 */
void test_rpc_init() {
    TEST_ASSERT_TRUE(true);
}

/**
 * @brief TEST: RPC function
 *
 */
void test_rpc_test() {
    TEST_ASSERT_TRUE(true);
}

/// @brief Constructor Class
RegisterRPC::RegisterRPC() {
}
/// @brief Constructor Class with Param
/// @param rpcParam Parameter for Class
RegisterRPC::RegisterRPC(RpcParam_t rpcParam) {
    param = rpcParam;
}

/// @brief Init RPC Class Object and Register method CallBack
/// @param streamRpc pointer to Object JsonRPC
void RegisterRPC::init(JsonRPC *streamRpc) {
    streamRpc->init();
    streamRpc->registerMethod("reboot", &reboot);
    streamRpc->registerMethod("rpctest", &rpctest);

    // ************************************************************************
    // ***************************** TEST BEGIN *******************************
    // ************************************************************************

    UNITY_BEGIN();

    RUN_TEST(test_rpc_init);
}

int RegisterRPC::reboot(JsonObject params, JsonObject result) {
    // print lcd message before reboot

    for (JsonPair it : params) {
        // loop in params
        if (strcmp(it.key().c_str(), "update") == 0) {
            if (it.value().as<bool>() == true) {
                TRACE_INFO_F(F("UPDATE FIRMWARE\r\n"));
                // set_default_configuration();
                // lcd_error |= lcd.clear();
                // lcd_error |= lcd.print(F("Reset configuration")) == 0;
            }
        }
    }

    TRACE_INFO_F(F("RPC: Request Reboot\r\n"));
    result[F("state")] = "done";

    // Reboot
    NVIC_SystemReset();

    return E_SUCCESS;
}

int RegisterRPC::rpctest(JsonObject params, JsonObject result) {
    // print lcd message before reboot

    for (JsonPair it : params) {
        // loop in params
        if (strcmp(it.key().c_str(), "update") == 1) {
            if (it.value().as<bool>() == true) {
                TRACE_INFO_F(F("UPDATE FIRMWARE\r\n"));
                // set_default_configuration();
                // lcd_error |= lcd.clear();
                // lcd_error |= lcd.print(F("Reset configuration")) == 0;
            }
        }
    }

    RUN_TEST(test_rpc_test);

    UNITY_END();

    // ************************************************************************
    // ***************************** TEST END *********************************
    // ************************************************************************

    TRACE_INFO_F(F("DO RPC TEST\r\n"));

    TRACE_INFO_F(F("Rpc TEST\r\n"));
    result[F("state")] = "done";
    // Do something
    return E_SUCCESS;
}