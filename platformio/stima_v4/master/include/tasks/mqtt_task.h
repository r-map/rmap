/**
 * @file mqtt_task.h
 * @brief Ethernet Task
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2022 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneCRYPTO Open.
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
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.1.4
 **/

#ifndef _MQTT_TASK_H
#define _MQTT_TASK_H

#include "hardware_config.h"
#include "net_config.h"
#include "Arduino.h"
#include "STM32FreeRTOS.h"
#include "thread.hpp"
#include "ticks.hpp"

#include "core/net.h"
#include "drivers/spi/arduino_spi_driver.h"
#include "drivers/eth/enc28j60_driver.h"
#include "dhcp/dhcp_client.h"
#include "ipv6/slaac.h"
#include "mqtt/mqtt_client.h"
#include "tls.h"
#include "tls_cipher_suites.h"
#include "hardware/stm32l4xx/stm32l4xx_crypto.h"
#include "rng/trng.h"
#include "rng/yarrow.h"
#include "net_util.h"
#include "mydebug.h"

typedef enum {
   MQTT_STATE_INIT,
   MQTT_STATE_OPEN_CONNECTION,
   MQTT_STATE_PUBLISH,
   MQTT_STATE_CLOSE_CONNECTION,
   MQTT_STATE_END
} MqttState_t;

typedef struct {
  NetInterface *interface;
  MqttClientContext mqttClientContext;
  uint16_t attemptDelayMs;
  uint16_t timeoutMs;
  char server[APP_MQTT_SERVER_NAME_LENGTH];
  uint16_t port;
  MqttVersion version;
  MqttTransportProtocol transportProtocol;
  MqttQosLevel qos;
  uint8_t keepAliveS;
  bool isCleanSession;
  char username[APP_MQTT_USERNAME_LENGTH];
  char password[APP_MQTT_PASSWORD_LENGTH];
  char willTopic[APP_MQTT_WILL_TOPIC_LENGTH];
  char willMsg[APP_MQTT_WILL_MSG_LENGTH];
  bool isWillMsgRetain;
  bool isPublishRetain;
  YarrowContext *yarrowContext;
  char_t *trustedCaList;
} MqttParam_t;

class MqttTask : public cpp_freertos::Thread {

public:
  MqttTask(const char *taskName, uint16_t stackSize, uint8_t priority, MqttParam_t MqttParam);
  // typedef public cpp_freertos::Thread super;

protected:
  virtual void Run();

private:
  char taskName[configMAX_TASK_NAME_LEN];
  uint16_t stackSize;
  uint8_t priority;
  MqttState_t state;
  MqttParam_t MqttParam;
  IpAddr serverIpAddr;
  bool isConnected;
  error_t error;
};

#endif