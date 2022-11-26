/**@file mqtt_task.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
<http://www.gnu.org/licenses/>.
**********************************************************************/

#ifndef _MQTT_TASK_H
#define _MQTT_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "str.h"

#if (USE_MQTT)

#define MQTT_TASK_WAIT_DELAY_MS           (100)
#define MQTT_TASK_GENERIC_RETRY_DELAY_MS  (5000)
#define MQTT_TASK_GENERIC_RETRY           (3)

#define MQTT_PUB_CMD_DEBUG_PREFIX         (">")
#define MQTT_SUB_CMD_DEBUG_PREFIX         ("<")

#include <STM32FreeRTOS.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "queue.hpp"
#include "drivers/module_master_hal.hpp"
#include "core/net.h"
#include "mqtt/mqtt_client.h"

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_ETH)
// #include "drivers/spi/arduino_spi_driver.h"
// #include "drivers/eth/enc28j60_driver.h"
// #include "dhcp/dhcp_client.h"
#endif

#include "tls.h"
#include "tls_cipher_suites.h"
#include "hardware/stm32l4xx/stm32l4xx_crypto.h"
#include "rng/trng.h"
#include "rng/yarrow.h"
#include "debug_F.h"

error_t mqttTlsInitCallback(MqttClientContext *context, TlsContext *tlsContext);
void mqttPublishCallback(MqttClientContext *context, const char_t *topic, const uint8_t *message, size_t length, bool_t dup, MqttQosLevel qos, bool_t retain, uint16_t packetId);

typedef enum
{
  MQTT_STATE_INIT,
  MQTT_STATE_WAIT_NET_EVENT,
  MQTT_STATE_CONNECT,
  MQTT_STATE_PUBLISH,
  MQTT_STATE_DISCONNECT,
  MQTT_STATE_END
} MqttState_t;

typedef struct
{
  configuration_t *configuration;
  // system_status_t *system_status;
  cpp_freertos::BinarySemaphore *configurationLock;
  // BinarySemaphore *systemStatusLock;
  // cpp_freertos::Queue *systemStatusQueue;
  cpp_freertos::Queue *systemRequestQueue;
  cpp_freertos::Queue *systemResponseQueue;
  YarrowContext *yarrowContext;
} MqttParam_t;

class MqttTask : public cpp_freertos::Thread {

public:
  MqttTask(const char *taskName, uint16_t stackSize, uint8_t priority, MqttParam_t MqttParam);

protected:
  virtual void Run();

private:
  MqttState_t state;
  MqttParam_t param;

  MqttClientContext mqttClientContext;

  MqttVersion version;
  MqttTransportProtocol transportProtocol;
  MqttQosLevel qos;

  bool isCleanSession;
  bool isWillMsgRetain;
  bool isPublishRetain;

  char clientIdentifier[MQTT_CLIENT_ID_LENGTH];
  char willTopic[MQTT_WILL_TOPIC_LENGTH];
  char willMsg[MQTT_WILL_MSG_LENGTH];
};

#endif
#endif