/**@file mqtt_task.h */

/*********************************************************************
<h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>

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
#define MQTT_TASK_PUBLISH_DELAY_MS        (5)
#define MQTT_TASK_PUBLISH_RETRY           (5)

#define MQTT_PUB_CMD_DEBUG_PREFIX         (">")
#define MQTT_SUB_CMD_DEBUG_PREFIX         ("<")

#include <STM32FreeRTOS.h>
#include <arduinoJsonRPC.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_master_hal.hpp"
#include "core/net.h"
#include "mqtt/mqtt_client.h"
#include "canard_config.hpp"

// Canard Type data
#include <rmap/_module/TH_1_0.h>
#include <rmap/service/_module/TH_1_0.h>
#include <rmap/_module/RAIN_1_0.h>
#include <rmap/service/_module/RAIN_1_0.h>
#include <rmap/_module/POWER_1_0.h>
#include <rmap/service/_module/POWER_1_0.h>
#include <rmap/_module/RADIATION_1_0.h>
#include <rmap/service/_module/RADIATION_1_0.h>
#include <rmap/_module/VWC_1_0.h>
#include <rmap/service/_module/VWC_1_0.h>
#include <rmap/_module/WIND_1_0.h>
#include <rmap/service/_module/WIND_1_0.h>

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

//List of preferred ciphersuites
//https://ciphersuite.info/cs/?security=recommended&singlepage=true&page=2&tls=all&sort=asc
const uint16_t MqttCipherSuites[] =
{
  // rmap server psk ciphers
  TLS_PSK_WITH_AES_256_CCM                      // WEAK BUT WORK
  // TLS_DHE_PSK_WITH_AES_128_GCM_SHA256            // RECOMMENDED BUT NOT WORK
  // TLS_DHE_PSK_WITH_AES_256_GCM_SHA384            // RECOMMENDED BUT NOT WORK
  // TLS_ECDHE_PSK_WITH_CHACHA20_POLY1305_SHA256    // RECOMMENDED BUT NOT WORK
  // TLS_DHE_PSK_WITH_CHACHA20_POLY1305_SHA256      // RECOMMENDED BUT NOT WORK

  // TLS_PSK_WITH_AES_256_CBC_SHA,            // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_PSK_WITH_AES_256_GCM_SHA384,         // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384,   // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA,      // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_DHE_PSK_WITH_AES_256_CBC_SHA384,     // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_PSK_WITH_AES_256_CBC_SHA384,         // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_DHE_PSK_WITH_AES_128_GCM_SHA256,     // RECOMMENDED BUT NOT WORK
  // TLS_PSK_WITH_AES_128_GCM_SHA256,         // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256,   // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA,      // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_DHE_PSK_WITH_AES_128_CBC_SHA256,     // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_DHE_PSK_WITH_AES_128_CBC_SHA,        // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_PSK_WITH_AES_128_CBC_SHA256,         // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_PSK_WITH_AES_128_CBC_SHA             // WEAK BUT NOT WORK (PREVIOUSLY WORK)

  // Recommended psk ciphers
  // TLS_DHE_PSK_WITH_AES_128_GCM_SHA256,
  // TLS_DHE_PSK_WITH_AES_256_GCM_SHA384,
  // TLS_DHE_PSK_WITH_CAMELLIA_128_GCM_SHA256,
  // TLS_DHE_PSK_WITH_CAMELLIA_256_GCM_SHA384,
  // TLS_DHE_PSK_WITH_ARIA_128_GCM_SHA256,
  // TLS_DHE_PSK_WITH_ARIA_256_GCM_SHA384,
  // TLS_DHE_PSK_WITH_CHACHA20_POLY1305_SHA256,
  // TLS_ECDHE_PSK_WITH_AES_128_GCM_SHA256,
  // TLS_ECDHE_PSK_WITH_AES_256_GCM_SHA384,
  // TLS_ECDHE_PSK_WITH_CHACHA20_POLY1305_SHA256
};

using namespace cpp_freertos;

typedef enum
{
  MQTT_STATE_CREATE,
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
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::Queue *systemMessageQueue;
  cpp_freertos::Queue *dataRmapGetRequestQueue;
  cpp_freertos::Queue *dataRmapGetResponseQueue;
  cpp_freertos::Queue *dataLogPutQueue;
  cpp_freertos::Queue *connectionRequestQueue;
  cpp_freertos::Queue *connectionResponseQueue;
  YarrowContext *yarrowContext;
  cpp_freertos::BinarySemaphore *rpcLock;
  JsonRPC *streamRpc;
} MqttParam_t;

class MqttTask : public cpp_freertos::Thread {

public:
  MqttTask(const char *taskName, uint16_t stackSize, uint8_t priority, MqttParam_t MqttParam);

protected:
  virtual void Run();

private:

  #if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
  #endif
  void TaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  static error_t mqttTlsInitCallback(MqttClientContext *context, TlsContext *tlsContext);
  static void mqttPublishCallback(MqttClientContext *context, const char_t *topic, const uint8_t *message, size_t length, bool_t dup, MqttQosLevel qos, bool_t retain, uint16_t packetId);

  error_t makeSensorTopic(rmap_metadata_Metadata_1_0 metadata, char *bvalue, char *sensors_topic, size_t sensors_topic_length);
  error_t makeCommonTopic(configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length);
  error_t makeDate(DateTime dateTime, char *message, size_t message_length);

  error_t publishSensorTH(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_TH_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length);
  error_t makeSensorMessageTemperature(rmap_measures_Temperature_1_0 temperature, DateTime dateTime, char *message, size_t message_length);
  error_t makeSensorMessageHumidity(rmap_measures_Humidity_1_0 humidity, DateTime dateTime, char *message, size_t message_length);

  error_t publishSensorRain(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_Rain_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length);
  error_t makeSensorMessageRain(rmap_measures_Rain_1_0 rain, DateTime dateTime, char *message, size_t message_length);

  error_t publishSensorRadiation(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_Radiation_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length);
  error_t makeSensorMessageRadiation(rmap_measures_Radiation_1_0 radiation, DateTime dateTime, char *message, size_t message_length);

  error_t publishSensorWindAvgVect10(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_WindAvgVect10_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length);
  error_t publishSensorWindAvgVect(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_WindAvgVect_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length);
  error_t publishSensorWindGustSpeed(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_WindGustSpeed_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length);
  error_t publishSensorWindAvgSpeed(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_WindAvgSpeed_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length);
  error_t publishSensorWindClassSpeed(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_WindClassSpeed_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length);
  error_t publishSensorWindGustDirection(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_WindGustDirection_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length);

  error_t makeSensorMessageSpeed(rmap_measures_WindSpeed_1_0 speed, DateTime dateTime, char *message, size_t message_length);
  error_t makeSensorMessageDirection(rmap_measures_WindDirection_1_0 direction, DateTime dateTime, char *message, size_t message_length);
  error_t makeSensorMessageSpeedPeak(rmap_measures_WindPeakGustSpeed_1_0 peak, DateTime dateTime, char *message, size_t message_length);
  error_t makeSensorMessageSpeedLong(rmap_measures_WindLongGustSpeed_1_0 _long, DateTime dateTime, char *message, size_t message_length);
  error_t makeSensorMessageDirectionPeak(rmap_measures_WindPeakGustDirection_1_0 peak, DateTime dateTime, char *message, size_t message_length);
  error_t makeSensorMessageDirectionLong(rmap_measures_WindLongGustDirection_1_0 _long, DateTime dateTime, char *message, size_t message_length);
  error_t makeSensorMessageClassSpeed(rmap_sensors_WindClassSpeed_1_0 sensor, DateTime dateTime, char *message, size_t message_length);

  error_t publishSensorPower(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_Power_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length);
  error_t makeSensorMessageInputVoltage(rmap_measures_InputVoltage_1_0 inputVoltage, DateTime dateTime, char *message, size_t message_length);
  error_t makeSensorMessageInputCurrent(rmap_measures_InputCurrent_1_0 inputCurrent, DateTime dateTime, char *message, size_t message_length);
  error_t makeSensorMessageBatteryVoltage(rmap_measures_BatteryVoltage_1_0 batteryVoltage, DateTime dateTime, char *message, size_t message_length);
  error_t makeSensorMessageBatteryCurrent(rmap_measures_BatteryCurrent_1_0 batteryCurrent, DateTime dateTime, char *message, size_t message_length);
  error_t makeSensorMessageBatteryCharge(rmap_measures_BatteryCharge_1_0 batteryCharge, DateTime dateTime, char *message, size_t message_length);

  error_t publishSensorSoil(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_VWC_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length);
  error_t makeSensorMessageSoil(rmap_measures_VolumetricWaterContent_1_0 soil, DateTime dateTime, char *message, size_t message_length);

  MqttState_t state;
  MqttParam_t param;

  MqttClientContext mqttClientContext;

  MqttVersion version;
  MqttTransportProtocol transportProtocol;
  MqttQosLevel qos;

  char topic[MQTT_ROOT_TOPIC_LENGTH + MQTT_SENSOR_TOPIC_LENGTH];
  char sensors_topic[MQTT_SENSOR_TOPIC_LENGTH];
  char message[MQTT_MESSAGE_LENGTH];
  char clientIdentifier[MQTT_CLIENT_ID_LENGTH];
  
  // Static RPC Procedure access and Semaphore
  inline static cpp_freertos::BinarySemaphore *localRpcLock;
  inline static JsonRPC *localStreamRpc;

  inline static YarrowContext *MqttYarrowContext;

  // Client's PSK key
  inline static uint8_t *MqttClientPSKKey;

  // Client's PSK identity
  inline static char_t MqttClientPSKIdentity[CLIENT_PSK_IDENTITY_LENGTH];

  inline static char_t *MqttServer;

  bool is_event_rpc;
};

#endif
#endif