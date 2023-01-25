/**@file main.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@digiteco.it>
authors:
Marco Baldinetti <marco.baldinetti@digiteco.it>

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

  // System semaphore
  static BinarySemaphore *configurationLock;  // Access Configuration
  static BinarySemaphore *systemStatusLock;   // Access System status
  static BinarySemaphore *registerAccessLock; // Access Register Cyphal Specifications

  // System Queue (Generic Message from/to Task)
  static Queue *systemMessageQueue;
  // Data queue (Request / exchange data from Data Task)
  static Queue *systemRequestQueue;
  static Queue *systemResponseQueue;
  //TODO: Data SD WR/RMAP
  //static Queue *reportDataQueue;

  // System and status configuration struct
  static configuration_t configuration = {0};
  static system_status_t system_status = {0};

  // Net Interface
  static YarrowContext yarrowContext;
  static uint8_t seed[SEED_LENGTH];

  // Initializing basic hardware's configuration
  SetupSystemPeripheral();
  init_debug(SERIAL_DEBUG_BAUD_RATE);
  init_wire();
  init_rtc(INIT_PARAMETER);
  init_net(&yarrowContext, seed, sizeof(seed));
  // init_sdcard();

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
    TRACE_INFO_F(F("Verified an WDT Reset...\r\n"));
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
  
  // Software Semaphore
  configurationLock = new BinarySemaphore(true);
  systemStatusLock = new BinarySemaphore(true);
  registerAccessLock = new BinarySemaphore(true);

  // Creating queue
  systemMessageQueue = new Queue(SYSTEM_MESSAGE_QUEUE_LENGTH, sizeof(system_message_t));
  systemRequestQueue = new Queue(REQUEST_DATA_QUEUE_LENGTH, sizeof(system_request_t));
  systemResponseQueue = new Queue(RESPONSE_DATA_QUEUE_LENGTH, sizeof(system_response_t));
  //TODO: Data SD WR/RMAP
  //reportDataQueue = new Queue(REPORT_DATA_QUEUE_LENGTH, sizeof(report_t));

  TRACE_INFO_F(F("Initialization HW Base done\r\n"));

  // ***************************************************************
  //                  Setup parameter for Task
  // ***************************************************************

#if (ENABLE_I2C2)
  // Load Info from E2 boot_check flag and send to Config
  static EEprom  memEprom(&Wire2, wire2Lock);
  bootloader_t boot_check = {0};
  #if INIT_PARAMETER
  boot_check.app_executed_ok = true;
  boot_check.version = MODULE_MAIN_VERSION;
  boot_check.revision = MODULE_MINOR_VERSION;
  #ifdef NODE_SERIAL_NUMBER
  configuration.board_master.serial_number = NODE_SERIAL_NUMBER;
  memEprom.Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_check, sizeof(boot_check));
  #endif
  #else
  memEprom.Read(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_check, sizeof(boot_check));
  configuration.board_master.serial_number = boot_check.serial_number;
  #endif
  // Optional send other InfoParm Boot (Uploaded, rollback, error fail ecc.. to config)
#endif

  // TASK WDT, INFO STACK PARAM CONFIG AND CHECK BOOTLOADER STATUS
  static WdtParam_t wdtParam = {0};
  wdtParam.system_status = &system_status;
  wdtParam.systemStatusLock = systemStatusLock;
  wdtParam.rtcLock = rtcLock;
#if (ENABLE_I2C2)
  wdtParam.wire = &Wire2;
  wdtParam.wireLock = wire2Lock;
#endif

#if (ENABLE_LCD)
  // TASK LCD DISPLAY PARAM CONFIG
  static LCDParam_t lcdParam = {0};
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
#endif

#if (ENABLE_CAN)
  // TASK CAN PARAM CONFIG
  static CanParam_t canParam = {0};
  canParam.configuration = &configuration;
  canParam.system_status = &system_status;
  canParam.configurationLock = configurationLock;
  canParam.systemStatusLock = systemStatusLock;
  canParam.registerAccessLock = registerAccessLock;
  canParam.systemMessageQueue = systemMessageQueue;
  // canParam.requestDataQueue = requestDataQueue;
  // canParam.reportDataQueue = reportDataQueue;
  canParam.canLock = canLock;  
  canParam.qspiLock = qspiLock;  
  canParam.rtcLock = rtcLock;
#if (ENABLE_I2C2)
  canParam.wire = &Wire2;
  canParam.wireLock = wire2Lock;
#endif
#endif

 // TASK SUPERVISOR PARAM CONFIG
  static SupervisorParam_t supervisorParam = {0};
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
  static ModemParam_t modemParam = {0};
  modemParam.configuration = &configuration;
  modemParam.system_status = &system_status;
  modemParam.configurationLock = configurationLock;
  modemParam.systemStatusLock = systemStatusLock;
  modemParam.systemRequestQueue = systemRequestQueue;
  modemParam.systemResponseQueue = systemResponseQueue;
#endif

#if (USE_NTP)
  static NtpParam_t ntpParam = {0};
  ntpParam.configuration = &configuration;
  ntpParam.system_status = &system_status;
  ntpParam.rtcLock = rtcLock;
  ntpParam.configurationLock = configurationLock;
  ntpParam.systemStatusLock = systemStatusLock;
  ntpParam.systemRequestQueue = systemRequestQueue;
  ntpParam.systemResponseQueue = systemResponseQueue;
#endif

#if (USE_HTTP)
  
  static HttpParam_t httpParam = {0};
  httpParam.configuration = &configuration;
  httpParam.system_status = &system_status;
  httpParam.configurationLock = configurationLock;
  httpParam.systemStatusLock = systemStatusLock;
  httpParam.systemRequestQueue = systemRequestQueue;
  httpParam.systemResponseQueue = systemResponseQueue;
#endif

#if (USE_MQTT)
  static MqttParam_t mqttParam = {0};
  mqttParam.configuration = &configuration;
  mqttParam.system_status = &system_status;
  mqttParam.configurationLock = configurationLock;
  mqttParam.systemStatusLock = systemStatusLock;
  mqttParam.systemRequestQueue = systemRequestQueue;
  mqttParam.systemResponseQueue = systemResponseQueue;
  mqttParam.yarrowContext = &yarrowContext;
#endif

  #if INIT_PARAMETER
  // Reset Factory register value
  EERegister initRegister(&Wire2, wire2Lock);
  initRegister.doFactoryReset();
  #endif

  // *****************************************************************************
  // Startup Task, Supervisor as first for Loading parameter generic configuration
  // *****************************************************************************

  static SupervisorTask supervisor_task("SupervisorTask", 450, OS_TASK_PRIORITY_02, supervisorParam);

#if (ENABLE_LCD)
  static LCDTask lcd_task("LcdTask", 300, OS_TASK_PRIORITY_01, lcdParam);
#endif

#if (ENABLE_CAN)
  static CanTask can_task("CanTask", 11400, OS_TASK_PRIORITY_02, canParam);
#endif

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
  static ModemTask modem_task("ModemTask", 650, OS_TASK_PRIORITY_02, modemParam);
#endif

#if (USE_NTP)
  static NtpTask ntp_task("NtpTask", 450, OS_TASK_PRIORITY_02, ntpParam);
#endif

#if (USE_HTTP)
  static HttpTask http_task("HttpTask", 400, OS_TASK_PRIORITY_02, httpParam);
#endif

#if (USE_MQTT)
  static MqttTask mqtt_task("MqttTask", 500, OS_TASK_PRIORITY_02, mqttParam);
#endif

  static WdtTask wdt_task("WdtTask", 350, OS_TASK_PRIORITY_01, wdtParam);

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

// Setup Wire I2C Interface
void init_wire()
{
#if (ENABLE_I2C1)
  Wire.begin();
  Wire.setClock(I2C1_BUS_CLOCK_HZ);
#endif

#if (ENABLE_I2C2)
  Wire2.begin();
  Wire2.setClock(I2C2_BUS_CLOCK_HZ);
#endif
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

  // // TCP/IP stack initialization
  error = netInit();
  if (error)
  {
    TRACE_ERROR_F(F("Failed to initialize TCP/IP stack %s\r\n"), ERROR_STRING);
    return error;
  }

  return error;
}