/**@file http_task.h */

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

#ifndef _HTTP_TASK_H
#define _HTTP_TASK_H

#include "debug_config.h"
#include "local_typedef.h"
#include "str.h"
#include "stima_utility.h"

#if (USE_HTTP)

#define HTTP_TASK_WAIT_DELAY_MS           (100)
#define HTTP_TASK_FAST_DELAY_MS           (5)
#define HTTP_TASK_GENERIC_RETRY_DELAY_MS  (5000)
#define HTTP_TASK_GENERIC_RETRY           (3)
#define HTTP_TASK_RPC_WAIT_DELAY_MS       (400)

#include <STM32FreeRTOS.h>
#include <arduinoJsonRPC.h>
#include "thread.hpp"
#include "ticks.hpp"
#include "semaphore.hpp"
#include "queue.hpp"
#include "drivers/module_master_hal.hpp"
#include "core/net.h"
#include "http/http_client.h"
#include "tls.h"
#include "tls_cipher_suites.h"
#include "hardware/stm32l4xx/stm32l4xx_crypto.h"
#include "rng/trng.h"
#include "rng/yarrow.h"
#include <string>
#include "debug_F.h"

//List of preferred ciphersuites
//https://ciphersuite.info/cs/?security=recommended&singlepage=true&page=2&tls=all&sort=asc
const uint16_t HttpCipherSuites[] =
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
  HTTP_STATE_CREATE,
  HTTP_STATE_INIT,
  HTTP_STATE_WAIT_NET_EVENT,
  HTTP_STATE_SEND_REQUEST,
  HTTP_STATE_LOOP_REQUEST_FIRMWARE,
  HTTP_STATE_GET_RESPONSE,
  HTTP_STATE_END
} HttpState_t;

typedef struct {
  configuration_t *configuration;
  system_status_t *system_status;
  cpp_freertos::BinarySemaphore *configurationLock;
  cpp_freertos::BinarySemaphore *systemStatusLock;
  cpp_freertos::Queue *systemMessageQueue;
  cpp_freertos::Queue *dataLogPutQueue;
  cpp_freertos::Queue *connectionRequestQueue;
  cpp_freertos::Queue *connectionResponseQueue;
  cpp_freertos::Queue *dataFilePutRequestQueue;
  cpp_freertos::Queue *dataFilePutResponseQueue;
  cpp_freertos::BinarySemaphore *rpcLock;
  YarrowContext *yarrowContext;
  JsonRPC *streamRpc;
} HttpParam_t;

class HttpTask : public cpp_freertos::Thread {

public:
  HttpTask(const char *taskName, uint16_t stackSize, uint8_t priority, HttpParam_t httpParam);

protected:
  char_t http_buffer[HTTP_BUFFER_SIZE];
  size_t http_buffer_length;
  virtual void Run();

private:

  #if (ENABLE_STACK_USAGE)
  void TaskMonitorStack();
  #endif
  void TaskWatchDog(uint32_t millis_standby);
  void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

  bool do_firmware_set_name(Module_Type module_type, uint8_t version, uint8_t revision);
  bool do_firmware_add_block(uint8_t *block_addr, uint16_t block_len);
  bool do_firmware_end_data(bool saveFile);

  static error_t httpClientTlsInitCallback(HttpClientContext *context, TlsContext *tlsContext);

  HttpState_t state;
  HttpParam_t param;
  HttpClientContext httpClientContext;

  inline static YarrowContext *HttpYarrowContext;

  // Client's PSK key
  inline static uint8_t *HttpClientPSKKey;

  // Client's PSK identity
  inline static char_t HttpClientPSKIdentity[CLIENT_PSK_IDENTITY_LENGTH];

  inline static char_t *HttpServer;

  bool is_event_rpc;

  // queue to Put Firmware to SD
  file_put_request_t firmwareDownloadChunck;
  file_put_response_t sdcard_task_response;

};

#endif
#endif
