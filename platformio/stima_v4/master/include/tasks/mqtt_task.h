/**
  ******************************************************************************
  * @file    mqtt_task.cpp
  * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Mqtt RMAP over Cyclone TCP Header files
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

/*!
\def USE_MINIMAL_WAIT_SECOND_CONNECT
\brief Enable using function delay minimal second waiting maintenance MQTT connection active
*/
#define USE_MINIMAL_WAIT_SECOND_CONNECT   (true)

#define MQTT_MINIMAL_SECOND_CONNECTION    (15)                        // Minimal connection second maintenance for receive RPC

#define MQTT_PUT_QUEUE_BKP_TIMEOUT_MS     (2500)

#define MQTT_NET_WAIT_TIMEOUT_SUSPEND     (120000)                    // Time out mqtt suspend operation timeout
#define MQTT_NET_WAIT_TIMEOUT_PUBLISH     (MQTT_TIMEOUT_MS + 2500)    // Time out mqtt suspend operation timeout

#define MIN_ERR_REPORT_CONNECTION_VALID   (90.0)

#define MQTT_PUB_CMD_DEBUG_PREFIX         (">")
#define MQTT_SUB_CMD_DEBUG_PREFIX         ("<")

#define MQTT_PUB_MAX_BIT_STATE            (16)    // Tot. RMAP BIT Status Bit sended for any module
#define MQTT_PUB_MAX_BYTE_STATE           (4)     // Tot. RMAP BYTE Status Byte sended for any module

// Local buffer for RPC Respone Buffer size dimension
#define MQTT_TASK_QUEUE_RPC_RESP_LENGTH   (MQTT_RPC_RESPONSE_LENGTH)  // Set to MQTT_CONFIG RPC Response Lenght
#define MQTT_TASK_QUEUE_RPC_RESP_ELEMENT  (10)    // Local defined max element to create response queue for RPC

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
#include "stima_utility.h"

#include <STM32RTC.h>

// Canard Type data
#include <rmap/_module/TH_1_0.h>
#include <rmap/service/_module/TH_1_0.h>
#include <rmap/_module/Rain_1_0.h>
#include <rmap/service/_module/Rain_1_0.h>
#include <rmap/_module/Power_1_0.h>
#include <rmap/service/_module/Power_1_0.h>
#include <rmap/_module/Radiation_1_0.h>
#include <rmap/service/_module/Radiation_1_0.h>
#include <rmap/_module/VWC_1_0.h>
#include <rmap/service/_module/VWC_1_0.h>
#include <rmap/_module/Wind_1_0.h>
#include <rmap/service/_module/Wind_1_0.h>

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
  TLS_PSK_WITH_AES_256_CCM                        // WEAK BUT WORK
  // TLS_DHE_PSK_WITH_AES_128_GCM_SHA256          // RECOMMENDED BUT NOT WORK
  // TLS_DHE_PSK_WITH_AES_256_GCM_SHA384          // RECOMMENDED BUT NOT WORK
  // TLS_ECDHE_PSK_WITH_CHACHA20_POLY1305_SHA256  // RECOMMENDED BUT NOT WORK
  // TLS_DHE_PSK_WITH_CHACHA20_POLY1305_SHA256    // RECOMMENDED BUT NOT WORK

  // TLS_PSK_WITH_AES_256_CBC_SHA,                // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_PSK_WITH_AES_256_GCM_SHA384,             // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384,       // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA,          // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_DHE_PSK_WITH_AES_256_CBC_SHA384,         // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_PSK_WITH_AES_256_CBC_SHA384,             // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_DHE_PSK_WITH_AES_128_GCM_SHA256,         // RECOMMENDED BUT NOT WORK
  // TLS_PSK_WITH_AES_128_GCM_SHA256,             // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256,       // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA,          // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_DHE_PSK_WITH_AES_128_CBC_SHA256,         // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_DHE_PSK_WITH_AES_128_CBC_SHA,            // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_PSK_WITH_AES_128_CBC_SHA256,             // WEAK BUT NOT WORK (PREVIOUSLY WORK)
  // TLS_PSK_WITH_AES_128_CBC_SHA                 // WEAK BUT NOT WORK (PREVIOUSLY WORK)

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

/// @brief struct local elaborate data parameter
typedef struct
{
  configuration_t *configuration;                     //!< system configuration pointer struct
  system_status_t *system_status;                     //!< system status pointer struct
  bootloader_t *boot_request;                         //!< Boot struct pointer
  cpp_freertos::BinarySemaphore *configurationLock;   //!< Semaphore to configuration access
  cpp_freertos::BinarySemaphore *systemStatusLock;    //!< Semaphore to system status access
  cpp_freertos::Queue *systemMessageQueue;            //!< Queue for system message
  cpp_freertos::Queue *dataRmapGetRequestQueue;       //!< Queue for access data RMAP Set Request
  cpp_freertos::Queue *dataRmapGetResponseQueue;      //!< Queue for access data RMAP Get Response
  cpp_freertos::Queue *dataRmapPutQueue;              //!< Queue for access data RMAP access Put Get Queue
  cpp_freertos::Queue *dataRmapPutBackupQueue;        //!< Queue for access data RMAP Put backup data
  cpp_freertos::Queue *dataLogPutQueue;               //!< Queue for system logging put data
  cpp_freertos::Queue *connectionRequestQueue;        //!< Queue for connection Set request
  cpp_freertos::Queue *connectionResponseQueue;       //!< Queue for connection Get response
  cpp_freertos::BinarySemaphore *rpcLock;             //!< Semaphore to RPC Access over MQTT
  YarrowContext *yarrowContext;                       //!< yarrowContext access to CycloneTCP Library
  JsonRPC *streamRpc;                                 //!< Object Stream C++ access for RPC
} MqttParam_t;

/// @brief MQTT TASK cpp_freertos class
class MqttTask : public cpp_freertos::Thread {

  /// @brief Enum for state switch of running method
  typedef enum
  {
    MQTT_STATE_CREATE,
    MQTT_STATE_INIT,
    MQTT_STATE_WAIT_NET_EVENT,
    MQTT_STATE_CONNECT,
    MQTT_STATE_PUBLISH,
    MQTT_STATE_PUBLISH_INFO,
    MQTT_STATE_DISCONNECT_LOST_DATA,
    MQTT_STATE_DISCONNECT,
    MQTT_STATE_END
  } MqttState_t;

public:
  MqttTask(const char *taskName, uint16_t stackSize, uint8_t priority, MqttParam_t mqttParam);

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
  error_t makeCommonTopic(configuration_t *configuration, char *topic, char *sensors_topic, size_t topic_length);
  error_t makeDate(DateTime dateTime, char *message, size_t message_length);

  void putRmapBackupArchiveData(DateTime dateTime, char *localTopic, char *localMessage);

  error_t publishSensorTH(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_TH_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length);
  error_t makeSensorMessageTemperature(rmap_measures_Temperature_1_0 temperature, DateTime dateTime, char *message, size_t message_length);
  error_t makeSensorMessageHumidity(rmap_measures_Humidity_1_0 humidity, DateTime dateTime, char *message, size_t message_length);

  error_t publishSensorRain(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_Rain_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length);
  error_t publishSensorRainRate(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_RainRate_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length);
  error_t makeSensorMessageRain(rmap_measures_Rain_1_0 rain, DateTime dateTime, char *message, size_t message_length);
  error_t makeSensorMessageRainShortRate(rmap_measures_RainShortRate_1_0 rainLongRate, DateTime dateTime, char *message, size_t message_length);
  error_t makeSensorMessageRainLongRate(rmap_measures_RainLongRate_1_0 rainLongRate, DateTime dateTime, char *message, size_t message_length);

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
  error_t makeSensorMessageBatteryCurrent(rmap_measures_BatteryCurrent_1_0 batteryCurrent, DateTime dateTime, char *message, size_t message_length);
  error_t makeSensorMessageBatteryCharge(rmap_measures_BatteryCharge_1_0 batteryCharge, DateTime dateTime, char *message, size_t message_length);

  error_t publishSensorSoil(MqttClientContext *context, MqttQosLevel qos, rmap_sensors_VWC_1_0 sensor, DateTime dateTime, configuration_t *configuration, char *topic, size_t topic_length, char *sensors_topic, size_t sensors_topic_length, char *message, size_t message_length);
  error_t makeSensorMessageSoil(rmap_measures_VolumetricWaterContent_1_0 soil, DateTime dateTime, char *message, size_t message_length);

  MqttState_t state;
  MqttParam_t param;

  STM32RTC &rtc = STM32RTC::getInstance();

  MqttClientContext mqttClientContext;

  MqttVersion version;
  MqttTransportProtocol transportProtocol;
  MqttQosLevel qos;

  char topic[MQTT_ROOT_TOPIC_LENGTH + MQTT_SENSOR_TOPIC_LENGTH];
  char sensors_topic[MQTT_SENSOR_TOPIC_LENGTH];
  char message[MQTT_MESSAGE_LENGTH];
  char clientIdentifier[MQTT_CLIENT_ID_LENGTH];
  // Static RPC Procedure access and Semaphore
  char topic_rpc_response[MQTT_ROOT_TOPIC_LENGTH + MQTT_SENSOR_TOPIC_LENGTH];
    
  inline static char rpc_response[MQTT_TASK_QUEUE_RPC_RESP_ELEMENT][MQTT_TASK_QUEUE_RPC_RESP_LENGTH];
  inline static int8_t rpc_response_index;
 
  inline static cpp_freertos::BinarySemaphore *localRpcLock;
  inline static JsonRPC *localStreamRpc;

  inline static MqttClientContext *localPtrMqttClientContext;

  inline static system_status_t *localSystemStatus;

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
