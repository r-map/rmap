/**@file main.cpp */

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

#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"

void setup() {
  // osInitKernel();

  // Initializing basic hardware's configuration
  SetupSystemPeripheral();
  init_debug(SERIAL_DEBUG_BAUD_RATE);
  init_wire();
  init_pins();
  init_tasks();
  init_sensors();
  init_net(&yarrowContext, seed, sizeof(seed));
  // init_sdcard();
  // init_registers();

  TRACE_INFO_F(F("Initialization HW Base done\r\n"));

  ProvaParam_t provaParam = {};

  LCDParam_t lcdParam;
  lcdParam.configuration = &configuration;
  lcdParam.system_status = &system_status;
  lcdParam.configurationLock = configurationLock;
  lcdParam.systemStatusLock = systemStatusLock;
  lcdParam.systemRequestQueue = systemRequestQueue;
  lcdParam.systemResponseQueue = systemResponseQueue;
#if (ENABLE_I2C2)
  lcdParam.wire = &Wire2;
  lcdParam.wireLock = wire2Lock;
#endif

#if (ENABLE_CAN)
  CanParam_t canParam;
  canParam.configuration = &configuration;
  canParam.system_status = &system_status;
  canParam.configurationLock = configurationLock;
  canParam.systemStatusLock = systemStatusLock;
  canParam.systemRequestQueue = systemRequestQueue;
  canParam.systemResponseQueue = systemResponseQueue;
  // canParam.requestDataQueue = requestDataQueue;
  // canParam.reportDataQueue = reportDataQueue;
#if (ENABLE_I2C2)
  canParam.wire = &Wire2;
  canParam.wireLock = wire2Lock;
#endif
#endif

  SupervisorParam_t supervisorParam;
  supervisorParam.configuration = &configuration;
  supervisorParam.system_status = &system_status;
#if (ENABLE_I2C2)
  supervisorParam.wire = &Wire2;
  supervisorParam.wireLock = wire2Lock;
#endif
  supervisorParam.configurationLock = configurationLock;
  supervisorParam.systemStatusLock = systemStatusLock;
  supervisorParam.systemRequestQueue = systemRequestQueue;
  supervisorParam.systemResponseQueue = systemResponseQueue;

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
  ModemParam_t modemParam;
  modemParam.configuration = &configuration;
  modemParam.system_status = &system_status;
  modemParam.configurationLock = configurationLock;
  modemParam.systemStatusLock = systemStatusLock;
  modemParam.systemRequestQueue = systemRequestQueue;
  modemParam.systemResponseQueue = systemResponseQueue;
#endif

#if (USE_NTP)
  NtpParam_t ntpParam;
  ntpParam.configuration = &configuration;
  ntpParam.system_status = &system_status;
  ntpParam.configurationLock = configurationLock;
  ntpParam.systemStatusLock = systemStatusLock;
  ntpParam.systemRequestQueue = systemRequestQueue;
  ntpParam.systemResponseQueue = systemResponseQueue;
#endif

#if (USE_HTTP)
  HttpParam_t httpParam;
  httpParam.configuration = &configuration;
  httpParam.system_status = &system_status;
  httpParam.configurationLock = configurationLock;
  httpParam.systemStatusLock = systemStatusLock;
  httpParam.systemRequestQueue = systemRequestQueue;
  httpParam.systemResponseQueue = systemResponseQueue;
#endif

#if (USE_MQTT)
  MqttParam_t mqttParam;
  mqttParam.configuration = &configuration;
  mqttParam.system_status = &system_status;
  mqttParam.configurationLock = configurationLock;
  mqttParam.systemStatusLock = systemStatusLock;
  mqttParam.systemRequestQueue = systemRequestQueue;
  mqttParam.systemResponseQueue = systemResponseQueue;
  mqttParam.yarrowContext = &yarrowContext;
#endif

  static ProvaTask prova_task("ProvaTask", 100, OS_TASK_PRIORITY_01, provaParam);
  static SupervisorTask supervisor_task("SupervisorTask", 100, OS_TASK_PRIORITY_02, supervisorParam);

  static LCDTask lcd_task("LcdTask", 100, OS_TASK_PRIORITY_01, lcdParam);

#if (ENABLE_CAN)
  static CanTask can_task("CanTask", 12000, OS_TASK_PRIORITY_02, canParam);
#endif

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
  static ModemTask modem_task("ModemTask", 800, OS_TASK_PRIORITY_02, modemParam);
#endif

#if (USE_NTP)
  static NtpTask ntp_task("NtpTask", 400, OS_TASK_PRIORITY_02, ntpParam);
#endif

#if (USE_HTTP)
  static HttpTask http_task("HttpTask", 600, OS_TASK_PRIORITY_02, httpParam);
#endif

#if (USE_MQTT)
  static MqttTask mqtt_task("MqttTask", 600, OS_TASK_PRIORITY_02, mqttParam);
#endif

  // Startup Schedulher
  Thread::StartScheduler();
}

void loop() {
}

void init_pins()
{
  // Attach interrrupt
  attachInterrupt(PIN_ENCODER_A, input_pin_encoder_A, CHANGE);
  attachInterrupt(PIN_ENCODER_B, input_pin_encoder_B, CHANGE);
  attachInterrupt(PIN_ENCODER_INT, input_pin_encoder_C, CHANGE);
  // Enable Encoder && Display
  digitalWrite(PIN_ENCODER_EN5, HIGH);
  digitalWrite(PIN_DSP_POWER, HIGH);
}

void init_tasks()
{
  memset(&configuration, 0, sizeof(configuration_t));
  memset(&system_status, 0, sizeof(system_status_t));

  configurationLock = new BinarySemaphore(true);
  systemStatusLock = new BinarySemaphore(true);

  systemRequestQueue = new Queue(SYSTEM_REQUEST_QUEUE_LENGTH, sizeof(system_request_t));
  systemResponseQueue = new Queue(SYSTEM_RESPONSE_QUEUE_LENGTH, sizeof(system_response_t));
}

void init_sensors()
{
}

void init_wire()
{
#if (ENABLE_I2C1)
  Wire.begin();
  Wire.setClock(I2C1_BUS_CLOCK_HZ);
  wireLock = new BinarySemaphore(true);
#endif

#if (ENABLE_I2C2)
  Wire2.begin();
  Wire2.setClock(I2C2_BUS_CLOCK_HZ);
  wire2Lock = new BinarySemaphore(true);
#endif
}

void input_pin_encoder_A()
{
  TRACE_DEBUG_F(F("ENC_A Event: %d %d %d\r\n"), digitalRead(PIN_ENCODER_A), digitalRead(PIN_ENCODER_B), digitalRead(PIN_ENCODER_INT));
}

void input_pin_encoder_B()
{
  TRACE_DEBUG_F(F("ENC_B Event: %d %d %d\r\n"), digitalRead(PIN_ENCODER_A), digitalRead(PIN_ENCODER_B), digitalRead(PIN_ENCODER_INT));
}

void input_pin_encoder_C()
{
  TRACE_DEBUG_F(F("ENC_ENT Event: %d %d %d\r\n"), digitalRead(PIN_ENCODER_A), digitalRead(PIN_ENCODER_B), digitalRead(PIN_ENCODER_INT));
}

bool init_net(YarrowContext *yarrowContext, uint8_t *seed, size_t seed_length)
{
  error_t error = NO_ERROR;

  // Initialize hardware cryptographic accelerator
  error = stm32l4xxCryptoInit();
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR_F(F("Failed to initialize hardware crypto accelerator %s\r\n"), ERROR_STRING);
    return error;
  }

  // Generate a random seed
  error = trngGetRandomData(seed, seed_length);
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR_F(F("Failed to generate random data %s\r\n"), ERROR_STRING);
    return error;
  }

  // PRNG initialization
  error = yarrowInit(yarrowContext);
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR_F(F("Failed to initialize PRNG %s\r\n"), ERROR_STRING);
    return error;
  }

  // Properly seed the PRNG
  error = yarrowSeed(yarrowContext, seed, seed_length);
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR_F(F("Failed to seed PRNG %s\r\n"), ERROR_STRING);
    return error;
  }

  // // TCP/IP stack initialization
  error = netInit();
  if (error)
  {
    TRACE_ERROR_F(F("Failed to initialize TCP/IP stack %s\r\n"), ERROR_STRING);
    return error;
  }

  return error;
}