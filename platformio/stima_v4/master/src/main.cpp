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

#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"

void setup() {
  osInitKernel();
  debugInit(115200);

  error_t error = NO_ERROR;

  // Initialize hardware cryptographic accelerator
  error = stm32l4xxCryptoInit();
  // Any error to report?
  if (error) {
    // Debug message
    TRACE_ERROR("Failed to initialize hardware crypto accelerator!\r\n");
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
  MqttParam_t mqttParam;

  ethernetParam.interface = &netInterface[0];
  ethernetParam.tickHandlerMs = APP_ETHERNET_TICK_EVENT_HANDLER_MS;

  mqttParam.timeoutMs = APP_MQTT_TIMEOUT_MS;
  mqttParam.attemptDelayMs = APP_MQTT_ATTEMPT_DELAY_MS;
  strncpy(mqttParam.server, APP_MQTT_SERVER_NAME, APP_MQTT_SERVER_NAME_LENGTH);
  mqttParam.port = APP_MQTT_SERVER_PORT;
  mqttParam.version = MQTT_VERSION_3_1_1;
  mqttParam.transportProtocol = MQTT_TRANSPORT_PROTOCOL_TLS;
  mqttParam.qos = MQTT_QOS_LEVEL_1;
  mqttParam.keepAliveS = APP_MQTT_KEEP_ALIVE_S;
  memset(mqttParam.clientIdentifier, 0, APP_MQTT_CLIENT_IDENTIFIER_LENGTH);
  memset(mqttParam.username, 0, APP_MQTT_USERNAME_LENGTH);
  memset(mqttParam.password, 0, APP_MQTT_USERNAME_LENGTH);
  // strncpy(mqttParam.username, "username", APP_MQTT_USERNAME_LENGTH);
  // strncpy(mqttParam.password, "password", APP_MQTT_PASSWORD_LENGTH);
  memset(mqttParam.willTopic, 0, APP_MQTT_WILL_TOPIC_LENGTH);
  // strncpy(mqttParam.willTopic, "board/status", APP_MQTT_WILL_TOPIC_LENGTH);
  memset(mqttParam.willMsg, 0, APP_MQTT_WILL_MSG_LENGTH);
  // strncpy(mqttParam.willMsg, "offline", APP_MQTT_WILL_MSG_LENGTH);
  mqttParam.isWillMsgRetain = false;
  mqttParam.isCleanSession = true;
  mqttParam.isPublishRetain = false;
  // mqttParam.clientPsk = (uint8_t *)clientPsk;
  // mqttParam.cipherSuites = (uint16_t *)cipherSuites;

  static LedTask led_1_task("LED 1 TASK", 100, OS_TASK_PRIORITY_01, ledParam1);
  static LedTask led_2_task("LED 2 TASK", 100, OS_TASK_PRIORITY_01, ledParam2);
  static LedTask led_3_task("LED 3 TASK", 100, OS_TASK_PRIORITY_01, ledParam3);
  static EthernetTask eth_task("ETH TASK", 100, OS_TASK_PRIORITY_03, ethernetParam);
  static MqttTask mqtt_task("MQTT TASK", 1024, OS_TASK_PRIORITY_02, mqttParam);

  cpp_freertos::Thread::StartScheduler();
}

void loop() {}
