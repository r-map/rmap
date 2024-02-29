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

uint32_t DATE_TIME_PTR_TEST = 1679875200ul;   // Set create file from 27/03/2023... To Now()
bool starting_test = false;
uint32_t data_field_create = 0;

void setup() {
 
  // Semaphore, Queue && Param Config for TASK
#if (ENABLE_I2C1)
  static BinarySemaphore *wireLock;       // Access I2C external interface UPIN_27
#endif

#if (ENABLE_I2C2)
  static BinarySemaphore *wire2Lock;      // Access I2C internal EEprom, Display
#endif

  static BinarySemaphore *rtcLock;        // RTC (Access lock)

  static BinarySemaphore *rpcLock;        // RPC (Access lock)

  // System semaphore
  static BinarySemaphore *configurationLock;  // Access Configuration
  static BinarySemaphore *systemStatusLock;   // Access System status
  static BinarySemaphore *registerAccessLock; // Access Register Cyphal Specifications

  // System Queue (Generic Message from/to Task)
  static Queue *systemMessageQueue;
  //Data MMC/SD WR/RD
  static Queue *dataRmapPutQueue;
  static Queue *dataRmapGetRequestQueue;
  static Queue *dataRmapGetResponseQueue;

  // System and status configuration struct
  static configuration_t configuration = {0};
  static system_status_t system_status = {0};

  // Initializing basic hardware's configuration, variables and function
  SetupSystemPeripheral();
  init_debug(SERIAL_DEBUG_BAUD_RATE);
  init_wire();
  init_rtc(true);

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
  rtcLock = new BinarySemaphore(true);
  rpcLock = new BinarySemaphore(true);

  // Software Semaphore
  configurationLock = new BinarySemaphore(true);
  systemStatusLock = new BinarySemaphore(true);
  registerAccessLock = new BinarySemaphore(true);

  // Creating queue
  systemMessageQueue = new Queue(SYSTEM_MESSAGE_QUEUE_LENGTH, sizeof(system_message_t));
  dataRmapPutQueue = new Queue(RMAP_PUT_DATA_QUEUE_LENGTH, sizeof(rmap_archive_data_t));
  dataRmapGetRequestQueue = new Queue(RMAP_GET_DATA_QUEUE_LENGTH, sizeof(rmap_get_request_t));
  dataRmapGetResponseQueue = new Queue(RMAP_GET_DATA_QUEUE_LENGTH, sizeof(rmap_get_response_t));

  TRACE_INFO_F(F("Initialization HW Base done\r\n\r\n"));

  // Get Serial Number and Print Fixed to Serial logger default
  configuration.board_master.serial_number = StimaV4GetSerialNumber();

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

  // ***************** SET PARAMETER TO TASK *********************

  // TASK WDT, INFO STACK PARAM CONFIG AND CHECK BOOTLOADER STATUS
  static WdtParam_t wdtParam = {0};
  wdtParam.system_status = &system_status;
  wdtParam.systemStatusLock = systemStatusLock;
  wdtParam.rtcLock = rtcLock;
  wdtParam.eeprom = &memEprom;

  // TASK SUPERVISOR PARAM CONFIG
  static SdParam_t sdParam = {0};
  sdParam.configuration = &configuration;
  sdParam.system_status = &system_status;
  sdParam.systemMessageQueue = systemMessageQueue;
  sdParam.dataRmapPutQueue = dataRmapPutQueue;
  sdParam.dataRmapGetRequestQueue = dataRmapGetRequestQueue;
  sdParam.dataRmapGetResponseQueue = dataRmapGetResponseQueue;
  sdParam.eeprom = &memEprom;
  sdParam.rtcLock = rtcLock;
  sdParam.configurationLock = configurationLock;
  sdParam.systemStatusLock = systemStatusLock;

  // TASK SUPERVISOR PARAM CONFIG
  static SupervisorParam_t supervisorParam = {0};
  supervisorParam.configuration = &configuration;
  supervisorParam.system_status = &system_status;
  supervisorParam.registerAccessLock = registerAccessLock;
  supervisorParam.configurationLock = configurationLock;
  supervisorParam.systemStatusLock = systemStatusLock;
  supervisorParam.systemMessageQueue = systemMessageQueue;
  supervisorParam.dataRmapPutQueue = dataRmapPutQueue;
  supervisorParam.dataRmapGetRequestQueue = dataRmapGetRequestQueue;
  supervisorParam.dataRmapGetResponseQueue = dataRmapGetResponseQueue;
  supervisorParam.eeprom = &memEprom;

  // *****************************************************************************
  // Startup Task, Supervisor as first for Loading parameter generic configuration
  // *****************************************************************************

  static SupervisorTask supervisor_task("SupervisorTask", 600, OS_TASK_PRIORITY_02, supervisorParam);

  static SdTask sd_task("SdTask", 1400, OS_TASK_PRIORITY_01, sdParam);

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
    rtc.setHours(10);rtc.setMinutes(55);rtc.setSeconds(30);
    rtc.setWeekDay(3);rtc.setDay(5);rtc.setMonth(4);rtc.setYear(23);
  }
  // Start LowPower configuration
  LowPower.begin();
}
