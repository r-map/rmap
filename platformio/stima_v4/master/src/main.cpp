/**
 * @file main.cpp
 * @brief Main
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2022 Marco Baldinetti. All rights reserved.
 *
 * This file is part of CycloneTCP Open.
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Marco Baldinetti <marco.baldinetti@alling.it>
 * @version 0.1
 **/

#include "STM32FreeRTOS.h"
#include "thread.hpp"
#include "tasks/led_task.h"
#include "tasks/ethernet_task.h"

#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"

void setup() {
  osInitKernel();
  SerialDebugInit(115200);

  error_t error = NO_ERROR;

  error = initCPRNG();
  if (error) {
    TRACE_ERROR("Failed to initialize Cryptographic Pseudo Random Number Generator!\r\n");
  }
  // TCP/IP stack initialization
  error = netInit();
  if (error) {
    TRACE_ERROR("Failed to initialize TCP/IP stack!\r\n");
  }

  LedParam_t ledParam1 = {LED1_PIN, 100, 900};
  LedParam_t ledParam2 = {LED2_PIN, 200, 800};
  LedParam_t ledParam3 = {LED3_PIN, 300, 700};
  EthernetParam_t ethernetParam;

  ethernetParam.state = ETHERNET_STATE_INIT;
  ethernetParam.interface = &netInterface[0];
  ethernetParam.tickHandlerMs = APP_ETHERNET_TICK_EVENT_HANDLER_MS;

  static LedTask led_1_task(100, OS_TASK_PRIORITY_NORMAL, ledParam1);
  static LedTask led_2_task(100, OS_TASK_PRIORITY_NORMAL, ledParam2);
  static LedTask led_3_task(100, OS_TASK_PRIORITY_NORMAL, ledParam3);
  static EthernetTask eth_task(8192, OS_TASK_PRIORITY_NORMAL, ethernetParam);

  cpp_freertos::Thread::StartScheduler();
}

void loop() {}

error_t initCPRNG () {
  // Global variables
  error_t error;

  // Initialize hardware cryptographic accelerator
  error = stm32l4xxCryptoInit();
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR("Failed to initialize hardware crypto accelerator!\r\n");
  }

  // Generate a random seed
  error = trngGetRandomData(seed, sizeof(seed));
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR("Failed to generate random data!\r\n");
  }

  // PRNG initialization
  error = yarrowInit(&yarrowContext);
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR("Failed to initialize PRNG!\r\n");
  }

  // Properly seed the PRNG
  error = yarrowSeed(&yarrowContext, seed, sizeof(seed));
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR("Failed to seed PRNG!\r\n");
  }

  return error;
}
