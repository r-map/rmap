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
  // init_can();

  TRACE_INFO_F(F("Initialization HW Base done\r\n"));

  ProvaParam_t provaParam = {};

  SupervisorParam_t supervisorParam;
  supervisorParam.configuration = &configuration;
  supervisorParam.system_status = &system_status;
#if (ENABLE_I2C2)
  supervisorParam.wire = &Wire2;
  supervisorParam.wireLock = wire2Lock;
#endif
  supervisorParam.configurationLock = configurationLock;
  supervisorParam.systemStatusLock = systemStatusLock;
  supervisorParam.systemStatusQueue = systemStatusQueue;
  supervisorParam.systemRequestQueue = systemRequestQueue;
  supervisorParam.systemResponseQueue = systemResponseQueue;

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
  ModemParam_t modemParam;
  modemParam.configuration = &configuration;
  modemParam.systemStatusQueue = systemStatusQueue;
  modemParam.systemRequestQueue = systemRequestQueue;
  modemParam.systemResponseQueue = systemResponseQueue;
#endif

  static ProvaTask prova_task("ProvaTask", 100, OS_TASK_PRIORITY_01, provaParam);
  static SupervisorTask supervisor_task("SupervisorTask", 100, OS_TASK_PRIORITY_01, supervisorParam);
#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
  static ModemTask modem_task("ModemTask", 100, OS_TASK_PRIORITY_01, modemParam);
#endif

  // Startup Schedulher
  Thread::StartScheduler();
}

void loop() {
}

void init_pins()
{
}

void init_tasks()
{
  memset(&configuration, 0, sizeof(configuration_t));
  memset(&system_status, 0, sizeof(system_status_t));

  configurationLock = new BinarySemaphore(true);
  systemStatusLock = new BinarySemaphore(true);

  systemStatusQueue = new Queue(SYSTEM_STATUS_QUEUE_LENGTH, sizeof(system_status_t));

#if ((MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM) || (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_ETH))
  systemRequestQueue = new Queue(SYSTEM_REQUEST_QUEUE_LENGTH, sizeof(system_request_t));
  systemResponseQueue = new Queue(SYSTEM_RESPONSE_QUEUE_LENGTH, sizeof(system_response_t));
#endif
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

bool init_net(YarrowContext *yarrowContext, uint8_t *seed, size_t seed_length)
{
  error_t error = NO_ERROR;

  // Initialize hardware cryptographic accelerator
  error = stm32l4xxCryptoInit();
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR_F(F("Failed to initialize hardware crypto accelerator!\r\n"));
    return error;
  }

  // Generate a random seed
  error = trngGetRandomData(seed, seed_length);
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR_F(F("Failed to generate random data!\r\n"));
    return error;
  }

  // PRNG initialization
  error = yarrowInit(yarrowContext);
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR_F(F("Failed to initialize PRNG!\r\n"));
    return error;
  }

  // Properly seed the PRNG
  error = yarrowSeed(yarrowContext, seed, seed_length);
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR_F(F("Failed to seed PRNG!\r\n"));
    return error;
  }

  // // TCP/IP stack initialization
  error = netInit();
  if (error)
  {
    TRACE_ERROR_F(F("Failed to initialize TCP/IP stack!\r\n"));
    return error;
  }

  return error;
}