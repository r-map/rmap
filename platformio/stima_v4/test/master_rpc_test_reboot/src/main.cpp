/**@file main.cpp */

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

#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"

void setup() {
 
  // Semaphore, Queue && Param Config for TASK
#if (ENABLE_I2C1)
  static BinarySemaphore *wireLock;       // Access I2C external interface UPIN_27
#endif

#if (ENABLE_I2C2)
  static BinarySemaphore *wire2Lock;      // Access I2C internal EEprom, Display
#endif

#if (ENABLE_CAN)
  static BinarySemaphore *canLock;        // Can BUS
#endif

#if (ENABLE_QSPI)
  static BinarySemaphore *qspiLock;       // Qspi (Flash Memory)
#endif

  static BinarySemaphore *rtcLock;        // RTC (Access lock)

  static BinarySemaphore *rpcLock;        // RPC (Access lock)

  // System semaphore
  static BinarySemaphore *configurationLock;  // Access Configuration
  static BinarySemaphore *systemStatusLock;   // Access System status
  static BinarySemaphore *registerAccessLock; // Access Register Cyphal Specifications

  // System Queue (Generic Message from/to Task)
  static Queue *systemMessageQueue;
  // Data queue (Request / exchange data from Data Task)
  static Queue *connectionRequestQueue;
  static Queue *connectionResponseQueue;
  //Data MMC/SD WR/RD
  static Queue *dataRmapPutQueue;
  static Queue *dataRmapGetRequestQueue;
  static Queue *dataRmapGetResponseQueue;
  static Queue *dataFilePutRequestQueue;
  static Queue *dataFilePutResponseQueue;
  static Queue *dataFileGetRequestQueue;
  static Queue *dataFileGetResponseQueue;
  static Queue *dataLogPutQueue;

  // System and status configuration struct
  static configuration_t configuration = {0};
  static system_status_t system_status = {0};

  // Net Interface
  static YarrowContext yarrowContext;
  static uint8_t seed[SEED_LENGTH];

  // Initializing basic hardware's configuration, variables and function
  SetupSystemPeripheral();
  init_debug(SERIAL_DEBUG_BAUD_RATE);
  init_wire();
  init_rtc(INIT_PARAMETER);
  init_net(&yarrowContext, seed, sizeof(seed));

  // Init SystemStatus Parameter !=0 ... For Check control Value
  // Task check init data (Wdt = True, TaskStack Max, TaskReady = False)
  // TOTAL_INFO_TASK Number of Task checked
#if (ENABLE_STACK_USAGE)
  for(uint8_t id = 0; id < TOTAL_INFO_TASK; id++)
  {
    system_status.tasks[id].stack = 0xFFFFu;
  }
#endif
  // Disable all Task before INIT
  for(uint8_t id = 0; id < TOTAL_INFO_TASK; id++)
  {
    system_status.tasks[id].state = task_flag::suspended;
  }

#if (ENABLE_WDT)
  // Init the watchdog timer WDT_TIMEOUT_BASE_US mseconds timeout (only control system)
  // Wdt Task Reset the value after All Task reset property single Flag
  if(IWatchdog.isReset()) {
    delay(50);
    TRACE_INFO_F(F("\r\n\r\nALERT: Verified an WDT Reset !!!\r\n\r\n"));
    IWatchdog.clearReset();
  }
  IWatchdog.begin(WDT_TIMEOUT_BASE_US);
#endif

  // Hardware Semaphore
#if (ENABLE_I2C1)
  wireLock = new BinarySemaphore(true);
#endif
#if (ENABLE_I2C2)
  wire2Lock = new BinarySemaphore(true);
#endif
#if (ENABLE_CAN)
  canLock = new BinarySemaphore(true);
#endif
#if (ENABLE_QSPI)
  qspiLock = new BinarySemaphore(true);
#endif
  rtcLock = new BinarySemaphore(true);
  rpcLock = new BinarySemaphore(true);

  // Software Semaphore
  configurationLock = new BinarySemaphore(true);
  systemStatusLock = new BinarySemaphore(true);
  registerAccessLock = new BinarySemaphore(true);

  // Creating queue
  systemMessageQueue = new Queue(SYSTEM_MESSAGE_QUEUE_LENGTH, sizeof(system_message_t));
  connectionRequestQueue = new Queue(REQUEST_DATA_QUEUE_LENGTH, sizeof(connection_request_t));
  connectionResponseQueue = new Queue(RESPONSE_DATA_QUEUE_LENGTH, sizeof(connection_response_t));
  dataFilePutRequestQueue = new Queue(FILE_PUT_DATA_QUEUE_LENGTH, sizeof(file_put_request_t));
  dataFilePutResponseQueue = new Queue(FILE_PUT_DATA_QUEUE_LENGTH, sizeof(file_put_response_t));
  dataFileGetRequestQueue = new Queue(FILE_GET_DATA_QUEUE_LENGTH, sizeof(file_get_request_t));
  dataFileGetResponseQueue = new Queue(FILE_GET_DATA_QUEUE_LENGTH, sizeof(file_get_response_t));
  dataRmapPutQueue = new Queue(RMAP_PUT_DATA_QUEUE_LENGTH, sizeof(rmap_archive_data_t));
  dataRmapGetRequestQueue = new Queue(RMAP_GET_DATA_QUEUE_LENGTH, sizeof(rmap_get_request_t));
  dataRmapGetResponseQueue = new Queue(RMAP_GET_DATA_QUEUE_LENGTH, sizeof(rmap_get_response_t));
  dataLogPutQueue = new Queue(LOG_PUT_DATA_QUEUE_LENGTH, LOG_PUT_DATA_ELEMENT_SIZE);

  TRACE_INFO_F(F("Initialization HW Base done\r\n\r\n"));

  // ***************************************************************
  //           Setup parameter for Task and local Class
  // ***************************************************************

#if (ENABLE_I2C2)
  // Load Info from E2 boot_check flag and send to Config
  static EEprom  memEprom(&Wire2, wire2Lock);
  bootloader_t boot_check = {0};
  #if INIT_PARAMETER
  boot_check.app_executed_ok = true;
  memEprom.Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_check, sizeof(boot_check));
  #else
  memEprom.Read(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_check, sizeof(boot_check));
  #endif
  // Optional send other InfoParm Boot (Uploaded, rollback, error fail ecc.. to config) ...
#endif
  // Reset Factory register value
  static EERegister clRegister(&Wire2, wire2Lock);
  #if INIT_PARAMETER
  clRegister.doFactoryReset();
  #endif
  // Init access Flash istance object
  static Flash memFlash(&hqspi);

  // ***************** SET PARAMETER TO CLASS ********************

  // RPC Remote Procedure Call object and Stream Object.
  static RpcParam_t rpcParam = {0};
  rpcParam.configuration = &configuration;
  rpcParam.system_status = &system_status;
  rpcParam.systemMessageQueue = systemMessageQueue;
  rpcParam.eeprom = &memEprom;
  rpcParam.rtcLock = rtcLock;
  rpcParam.configurationLock = configurationLock;
  rpcParam.systemStatusLock = systemStatusLock;
  rpcParam.rpcLock = rpcLock;
  // Init class procedure and linked callBack with parameter
  static JsonRPC streamRpc;
  static RegisterRPC localRpc(rpcParam);
  localRpc.init(&streamRpc);

  // ***************** SET PARAMETER TO TASK *********************

  // TASK WDT, INFO STACK PARAM CONFIG AND CHECK BOOTLOADER STATUS
  static WdtParam_t wdtParam = {0};
  wdtParam.system_status = &system_status;
  wdtParam.systemStatusLock = systemStatusLock;
  wdtParam.rtcLock = rtcLock;
  wdtParam.dataLogPutQueue = dataLogPutQueue;
  wdtParam.eeprom = &memEprom;

#if (ENABLE_USBSERIAL)
 // TASK SUPERVISOR PARAM CONFIG
  static UsbSerialParam_t usbSerialParam = {0};
  usbSerialParam.configuration = &configuration;
  usbSerialParam.system_status = &system_status;
  usbSerialParam.systemMessageQueue = systemMessageQueue;
  usbSerialParam.flash = &memFlash;
  usbSerialParam.eeprom = &memEprom;
  usbSerialParam.qspiLock = qspiLock;  
  usbSerialParam.rtcLock = rtcLock;
  usbSerialParam.configurationLock = configurationLock;
  usbSerialParam.systemStatusLock = systemStatusLock;
  usbSerialParam.streamRpc = &streamRpc;
  usbSerialParam.rpcLock = rpcLock;
#endif

  // *****************************************************************************
  // Startup Task, Supervisor as first for Loading parameter generic configuration
  // *****************************************************************************

  // BEEP on Start ( Test Reboot )
  digitalWrite(HFLT_PIN, 1);
  delay(300);
  digitalWrite(HFLT_PIN, 0);
  delay(300);

#if (ENABLE_USBSERIAL)
  static UsbSerialTask usbSerial_task("UsbSerialTask", 1100, OS_TASK_PRIORITY_01, usbSerialParam);
#endif

  static WdtTask wdt_task("WdtTask", 400, OS_TASK_PRIORITY_04, wdtParam);

  // Startup Schedulher
  Thread::StartScheduler();
}

// FreeRTOS idleHook callBack to loop
void loop() {
  // Enable LowPower idleHock reduce power consumption without disable sysTick
  #if (USE_LOWPOWER_IDLE_LOOP)
  LowPower.idleHook();
  #endif
}

// Setup Wire I2C SPI Interface
void init_wire()
{
  // Setup I2C
#if (ENABLE_I2C1)
  Wire.begin();
  Wire.setClock(I2C1_BUS_CLOCK_HZ);
#endif

#if (ENABLE_I2C2)
  Wire2.begin();
  Wire2.setClock(I2C2_BUS_CLOCK_HZ);
#endif

  // Setup SPI
 #if (ENABLE_SPI1)
  SPI.begin();        
 #endif
  
  // Start EN Pow GSM ready to SET
  digitalWrite(PIN_GSM_EN_POW, HIGH);
}

// Setup RTC HW && LowPower Class STM32
void init_rtc(bool init)
{
  // Init istance to STM RTC object
  STM32RTC& rtc = STM32RTC::getInstance();
  // Select RTC clock source: LSE_CLOCK (First Istance)
  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin(); // initialize RTC 24H format
  // Set the time if requireq to Reset value
  if(init) {
    // Set the date && Time Init Value
    rtc.setHours(0);rtc.setMinutes(0);rtc.setSeconds(0);
    rtc.setWeekDay(0);rtc.setDay(1);rtc.setMonth(1);rtc.setYear(23);
  }
  // Start LowPower configuration
  LowPower.begin();
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

  // TCP/IP stack initialization
  error = netInit();
  if (error)
  {
    TRACE_ERROR_F(F("Failed to initialize TCP/IP stack %s\r\n"), ERROR_STRING);
    return error;
  }

  return error;
}