/**@file main.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <m.baldinetti@digiteco.it>
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

  // RPC
  /*!
  \var streamRpc()
  \brief Remote Procedure Call object.
  */
  static JsonRPC streamRpc;
  streamRpc.init();

  // Initializing basic hardware's configuration
  SetupSystemPeripheral();
  init_debug(SERIAL_DEBUG_BAUD_RATE);
  init_wire();
  init_rtc(INIT_PARAMETER);
  init_net(&yarrowContext, seed, sizeof(seed));
  init_rpc(&streamRpc);

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

  // Get Serial Number and Print Fixed to Serial logger default
  configuration.board_master.serial_number = StimaV4GetSerialNumber();
  Serial.println();
  Serial.println(F("*****************************"));
  Serial.println(F("* Stima V4 MASTER - SER.NUM *"));
  Serial.println(F("*****************************"));
  Serial.print(F("COD: "));
  for(uint8_t uid=0; uid<8; uid++) {
    if(uid) Serial.print("-");
    if((uint8_t)((configuration.board_master.serial_number >> (8*uid)) & 0xFF) < 16) Serial.print("0");
    Serial.print((uint8_t)((configuration.board_master.serial_number >> (8*uid)) & 0xFF), 16);
  }
  Serial.println("\r\n");

  // ***************************************************************
  //                  Setup parameter for Task
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

  // ***************** SET PARAMETER TO TASK *********************

  // TASK WDT, INFO STACK PARAM CONFIG AND CHECK BOOTLOADER STATUS
  static WdtParam_t wdtParam = {0};
  wdtParam.system_status = &system_status;
  wdtParam.systemStatusLock = systemStatusLock;
  wdtParam.rtcLock = rtcLock;
  wdtParam.dataLogPutQueue = dataLogPutQueue;
  wdtParam.eeprom = &memEprom;

#if (ENABLE_MMC)
 // TASK SUPERVISOR PARAM CONFIG
  static MmcParam_t mmcParam = {0};
  mmcParam.configuration = &configuration;
  mmcParam.system_status = &system_status;
  mmcParam.systemMessageQueue = systemMessageQueue;
  mmcParam.dataRmapPutQueue = dataRmapPutQueue;
  mmcParam.dataRmapGetRequestQueue = dataRmapGetRequestQueue;
  mmcParam.dataRmapGetResponseQueue = dataRmapGetResponseQueue;
  mmcParam.dataLogPutQueue = dataLogPutQueue;
  mmcParam.dataFilePutRequestQueue = dataFilePutRequestQueue;
  mmcParam.dataFilePutResponseQueue = dataFilePutResponseQueue;
  mmcParam.dataFileGetRequestQueue = dataFileGetRequestQueue;
  mmcParam.dataFileGetResponseQueue = dataFileGetResponseQueue;
  mmcParam.flash = &memFlash;
  mmcParam.eeprom = &memEprom;
  mmcParam.qspiLock = qspiLock;  
  mmcParam.rtcLock = rtcLock;
  mmcParam.configurationLock = configurationLock;
  mmcParam.systemStatusLock = systemStatusLock;
#endif

#if (ENABLE_SD)
 // TASK SUPERVISOR PARAM CONFIG
  static SdParam_t sdParam = {0};
  sdParam.configuration = &configuration;
  sdParam.system_status = &system_status;
  sdParam.systemMessageQueue = systemMessageQueue;
  sdParam.dataRmapPutQueue = dataRmapPutQueue;
  sdParam.dataRmapGetRequestQueue = dataRmapGetRequestQueue;
  sdParam.dataRmapGetResponseQueue = dataRmapGetResponseQueue;
  sdParam.dataLogPutQueue = dataLogPutQueue;
  sdParam.dataFilePutRequestQueue = dataFilePutRequestQueue;
  sdParam.dataFilePutResponseQueue = dataFilePutResponseQueue;
  sdParam.dataFileGetRequestQueue = dataFileGetRequestQueue;
  sdParam.dataFileGetResponseQueue = dataFileGetResponseQueue;
  sdParam.flash = &memFlash;
  sdParam.eeprom = &memEprom;
  sdParam.qspiLock = qspiLock;  
  sdParam.rtcLock = rtcLock;
  sdParam.configurationLock = configurationLock;
  sdParam.systemStatusLock = systemStatusLock;
#endif

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

#if (ENABLE_LCD)
  // TASK LCD DISPLAY PARAM CONFIG
  static LCDParam_t lcdParam = {0};
  lcdParam.configuration = &configuration;
  lcdParam.system_status = &system_status;  
  lcdParam.configurationLock = configurationLock;
  lcdParam.systemStatusLock = systemStatusLock;
  lcdParam.systemMessageQueue = systemMessageQueue;
  lcdParam.dataLogPutQueue = dataLogPutQueue;
  lcdParam.rtcLock = rtcLock;
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
  canParam.dataRmapPutQueue = dataRmapPutQueue;
  canParam.dataLogPutQueue = dataLogPutQueue;
  canParam.dataFileGetRequestQueue = dataFileGetRequestQueue;
  canParam.dataFileGetResponseQueue = dataFileGetResponseQueue;
  canParam.eeprom = &memEprom;
  canParam.clRegister = &clRegister;
  canParam.flash = &memFlash;
  canParam.canLock = canLock;
  canParam.qspiLock = qspiLock;  
  canParam.rtcLock = rtcLock;
#endif

  // TASK SUPERVISOR PARAM CONFIG
  static SupervisorParam_t supervisorParam = {0};
  supervisorParam.configuration = &configuration;
  supervisorParam.system_status = &system_status;
  supervisorParam.registerAccessLock = registerAccessLock;
  supervisorParam.configurationLock = configurationLock;
  supervisorParam.systemStatusLock = systemStatusLock;
  supervisorParam.systemMessageQueue = systemMessageQueue;
  supervisorParam.connectionRequestQueue = connectionRequestQueue;
  supervisorParam.connectionResponseQueue = connectionResponseQueue;
  supervisorParam.dataRmapPutQueue = dataRmapPutQueue;
  supervisorParam.dataRmapGetRequestQueue = dataRmapGetRequestQueue;
  supervisorParam.dataRmapGetResponseQueue = dataRmapGetResponseQueue;
  supervisorParam.dataFilePutRequestQueue = dataFilePutRequestQueue;
  supervisorParam.dataFilePutResponseQueue = dataFilePutResponseQueue;
  supervisorParam.eeprom = &memEprom;
  supervisorParam.clRegister = &clRegister;

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
  // TASK MODEM 2G/4G
  static ModemParam_t modemParam = {0};
  modemParam.configuration = &configuration;
  modemParam.system_status = &system_status;
  modemParam.configurationLock = configurationLock;
  modemParam.systemStatusLock = systemStatusLock;
  modemParam.systemMessageQueue = systemMessageQueue;
  modemParam.dataLogPutQueue = dataLogPutQueue;
  modemParam.connectionRequestQueue = connectionRequestQueue;
  modemParam.connectionResponseQueue = connectionResponseQueue;
#endif

#if (USE_NTP)
  // TASK NET NTP
  static NtpParam_t ntpParam = {0};
  ntpParam.configuration = &configuration;
  ntpParam.system_status = &system_status;
  ntpParam.rtcLock = rtcLock;
  ntpParam.configurationLock = configurationLock;
  ntpParam.systemStatusLock = systemStatusLock;
  ntpParam.systemMessageQueue = systemMessageQueue;
  ntpParam.dataLogPutQueue = dataLogPutQueue;
  ntpParam.connectionRequestQueue = connectionRequestQueue;
  ntpParam.connectionResponseQueue = connectionResponseQueue;
#endif

#if (USE_HTTP)
  // TASK HTTP
  static HttpParam_t httpParam = {0};
  httpParam.configuration = &configuration;
  httpParam.system_status = &system_status;
  httpParam.configurationLock = configurationLock;
  httpParam.systemStatusLock = systemStatusLock;
  httpParam.systemMessageQueue = systemMessageQueue;
  httpParam.dataLogPutQueue = dataLogPutQueue;
  httpParam.connectionRequestQueue = connectionRequestQueue;
  httpParam.connectionResponseQueue = connectionResponseQueue;
  httpParam.yarrowContext = &yarrowContext;
  httpParam.rpcLock = rpcLock;
#endif

#if (USE_MQTT)
  // TASK MQTT
  static MqttParam_t mqttParam = {0};
  mqttParam.configuration = &configuration;
  mqttParam.system_status = &system_status;
  mqttParam.configurationLock = configurationLock;
  mqttParam.systemStatusLock = systemStatusLock;
  mqttParam.systemMessageQueue = systemMessageQueue;
  mqttParam.dataRmapGetRequestQueue = dataRmapGetRequestQueue;
  mqttParam.dataRmapGetResponseQueue = dataRmapGetResponseQueue;
  mqttParam.dataLogPutQueue = dataLogPutQueue;
  mqttParam.connectionRequestQueue = connectionRequestQueue;
  mqttParam.connectionResponseQueue = connectionResponseQueue;
  mqttParam.yarrowContext = &yarrowContext;
  mqttParam.rpcLock = rpcLock;
#endif

  // *****************************************************************************
  // Startup Task, Supervisor as first for Loading parameter generic configuration
  // *****************************************************************************

  static SupervisorTask supervisor_task("SupervisorTask", 500, OS_TASK_PRIORITY_02, supervisorParam);

#if (ENABLE_MMC)
  static MmcTask mmc_task("MmcTask", 1400, OS_TASK_PRIORITY_01, mmcParam);
#endif
#if (ENABLE_SD)
  static SdTask sd_task("SdTask", 1400, OS_TASK_PRIORITY_01, sdParam);
#endif

#if (ENABLE_USBSERIAL)
  static UsbSerialTask usbSerial_task("UsbSerialTask", 200, OS_TASK_PRIORITY_01, usbSerialParam);
#endif

#if (ENABLE_LCD)
  static LCDTask lcd_task("LcdTask", 300, OS_TASK_PRIORITY_03, lcdParam);
#endif

#if (ENABLE_CAN)
  static CanTask can_task("CanTask", 11800, OS_TASK_PRIORITY_02, canParam);
#endif

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
  static ModemTask modem_task("ModemTask", 650, OS_TASK_PRIORITY_02, modemParam);
#endif

#if (USE_NTP)
  static NtpTask ntp_task("NtpTask", 500, OS_TASK_PRIORITY_02, ntpParam);
#endif

#if (USE_HTTP)
  static HttpTask http_task("HttpTask", 500, OS_TASK_PRIORITY_02, httpParam);
#endif

#if (USE_MQTT)
  static MqttTask mqtt_task("MqttTask", 500, OS_TASK_PRIORITY_02, mqttParam);
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

void init_rpc(JsonRPC *streamRpc)
{
#if (USE_RPC_METHOD_CONFIGURE)
  streamRpc->registerMethod("configure", &configure);
#endif

#if (USE_RPC_METHOD_PREPARE)
  streamRpc->registerMethod("prepare", &prepare);
#endif

#if (USE_RPC_METHOD_GETJSON)
  streamRpc->registerMethod("getjson", &getjson);
#endif

#if (USE_RPC_METHOD_PREPANDGET)
  streamRpc->registerMethod("prepandget", &prepandget);
#endif

#if (USE_RPC_METHOD_REBOOT)
  streamRpc->registerMethod("reboot", &reboot);
#endif

#if (USE_RPC_METHOD_RECOVERY && USE_MQTT)
  streamRpc->registerMethod("recovery", &recovery);
#endif
}

#if (USE_RPC_METHOD_CONFIGURE)
int configure(JsonObject params, JsonObject result)
{
  bool is_error = false;
  bool is_sensor_config = false;

  for (JsonPair it : params)
  {
    // loop in params
    if (strcmp(it.key().c_str(), "reset") == 0)
    {
      if (it.value().as<bool>() == true)
      {
        TRACE_INFO_F(F("DO RESET CONFIGURATION\r\n"));
        // set_default_configuration();
        // lcd_error |= lcd.clear();
        // lcd_error |= lcd.print(F("Reset configuration")) == 0;
      }
      else if (strcmp(it.key().c_str(), "save") == 0)
      {
        if (it.value().as<bool>() == true)
        {
          TRACE_INFO_F(F("DO SAVE CONFIGURATION\r\n"));
          // save_configuration(CONFIGURATION_CURRENT);
          // lcd_error |= lcd.clear();
          // lcd_error |= lcd.print(F("Save configuration")) == 0;
        }
      }
      else if (strcmp(it.key().c_str(), "board") == 0)
      {
        // it.value().as<const char *>()
      }
      else if (strcmp(it.key().c_str(), "boardtype") == 0)
      {
        // it.value().as<unsigned int>()
      }
      else if (strcmp(it.key().c_str(), "sn") == 0)
      {
        // it.value().as<const char *>()
      }
      else if (strcmp(it.key().c_str(), "cansampletime") == 0)
      {
        // it.value().as<unsigned int>()
      }
      else if (strcmp(it.key().c_str(), "node_id") == 0)
      {
        // it.value().as<unsigned int>()
      }
      else if (strcmp(it.key().c_str(), "subject") == 0)
      {
        // it.value().as<const char *>()
      }
      else if (strcmp(it.key().c_str(), "subject_id") == 0)
      {
        // it.value().as<unsigned int>()
      }
      else if (strcmp(it.key().c_str(), "driver") == 0)
      {
        // it.value().as<const char *>()
        is_sensor_config = true;
      }
      else if (strcmp(it.key().c_str(), "type") == 0)
      {
        // it.value().as<const char *>()
        is_sensor_config = true;
      }
      else if (strcmp(it.key().c_str(), "timerange") == 0)
      {
        // Pindicator
        // it.value().as<JsonArray>()[0].as<unsigned int>()

        // P1
        // it.value().as<JsonArray>()[1].as<unsigned int>()

        // P2
        // it.value().as<JsonArray>()[2].as<unsigned int>()

        is_sensor_config = true;
      }
      else if (strcmp(it.key().c_str(), "level") == 0)
      {
        // LevelType1
        // it.value().as<JsonArray>()[0].as<unsigned int>()

        // L1
        // it.value().as<JsonArray>()[1].as<unsigned int>()

        // LevelType2
        // it.value().as<JsonArray>()[2].as<unsigned int>()

        // L2
        // it.value().as<JsonArray>()[3].as<unsigned int>()

        is_sensor_config = true;
      }
      else if (strcmp(it.key().c_str(), "sd") == 0)
      {
        for (JsonPair sd : it.value().as<JsonObject>())
        {
          // constantdata btable
          // sd.key().c_str()

          // constantdata value
          // sd.value().as<const char *>()

          // constantdata index must be incremented in order to configure the next value
          // if (constantdata_count < USE_CONSTANTDATA_COUNT)
          // {
          //   constantdata_count++;
          // }
          // else
          // {
          //   is_error = true;
          // }
        }
      }
      else if (strcmp(it.key().c_str(), "stationslug") == 0)
      {
        // it.value().as<const char *>()
      }
      else if (strcmp(it.key().c_str(), "boardslug") == 0)
      {
        // it.value().as<const char *>()
      }
      else if (strcmp(it.key().c_str(), "lon") == 0)
      {
        // it.value().as<long int>()
      }
      else if (strcmp(it.key().c_str(), "lat") == 0)
      {
        // it.value().as<long int>()
      }
      else if (strcmp(it.key().c_str(), "network") == 0)
      {
        // it.value().as<const char *>()
      }
    }
    else if (strcmp(it.key().c_str(), "date") == 0)
    {
      // tmElements_t tm;
      // tm.Year = y2kYearToTm(it.value().as<JsonArray>()[0].as<int>() - 2000);
      // tm.Month = it.value().as<JsonArray>()[1].as<int>();
      // tm.Day = it.value().as<JsonArray>()[2].as<int>();
      // tm.Hour = it.value().as<JsonArray>()[3].as<int>();
      // tm.Minute = it.value().as<JsonArray>()[4].as<int>();
      // tm.Second = it.value().as<JsonArray>()[5].as<int>();
      // system_time = makeTime(tm);
    }
#if (USE_MQTT)
    else if (strcmp(it.key().c_str(), "mqttrootpath") == 0)
    {
      // mqtt_root_topic
      // it.value().as<const char *>()
    }
    else if (strcmp(it.key().c_str(), "mqttrpcpath") == 0)
    {
      // mqtt_rpc_topic
      // it.value().as<const char *>()
    }
    else if (strcmp(it.key().c_str(), "mqttmaintpath") == 0)
    {
      // mqtt_maint_topic
      // it.value().as<const char *>()
    }
    else if (strcmp(it.key().c_str(), "mqttserver") == 0)
    {
      // mqtt_server
      // it.value().as<const char *>()
    }
    else if (strcmp(it.key().c_str(), "mqttsampletime") == 0)
    {
      // report_s
      // it.value().as<unsigned int>()
    }
    else if (strcmp(it.key().c_str(), "mqttuser") == 0)
    {
      // mqtt_username
      // it.value().as<const char *>()
    }
    else if (strcmp(it.key().c_str(), "mqttpassword") == 0)
    {
      // mqtt_password
      // it.value().as<const char *>()
    }
    else if (strcmp(it.key().c_str(), "mqttpskkey") == 0)
    {
      // client_psk_key
      // trasformare da stringa come it.value().as<const char *>() array di uint8_t per salvataggio in client_psk_key
    }
#endif
#if (USE_NTP)
    else if (strcmp(it.key().c_str(), "ntpserver") == 0)
    {
      // ntp_server
      // it.value().as<const char *>()
    }
#endif
#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
    else if (strcmp(it.key().c_str(), "gsmapn") == 0)
    {
      // gsm_apn
      // it.value().as<const char *>()
    }
    else if (strcmp(it.key().c_str(), "pppnumber") == 0)
    {
      // gsm_number
      // it.value().as<const char *>()
    }
#endif
    else
    {
      is_error = true;
    }
  }

  // when is_sensor_config = true a sensor was configured, then the index sensors_count must be incremented
  // in order to configure the next sensor
  if (is_sensor_config)
  {
    // if (writable_configuration.sensors_count < SENSORS_MAX)
    // {
    //   // writable_configuration.sensors_count++;
    // }
    // else
    // {
    //   is_error = true;
    // }
  }

  if (is_error)
  {
    result[F("state")] = F("error");
    return E_INVALID_PARAMS;
  }
  else
  {
    result[F("state")] = F("done");
    return E_SUCCESS;
  }
  }
#endif

#if (USE_RPC_METHOD_RECOVERY && USE_MQTT)
int recovery(JsonObject params, JsonObject result)
{
  static int state;
  static int tmpstate;
  static time_t ptr_time;
  static File mqtt_ptr_rpc_file;

  switch (rpc_state)
  {
  case RPC_INIT:

    state = E_BUSY;
    {
      bool found = false;

      for (JsonPair it : params)
      {
        if (strcmp(it.key().c_str(), "dts") == 0)
        {
          found = true;
          if (!sdcard_open_file(&SD, &mqtt_ptr_rpc_file, SDCARD_MQTT_PTR_RPC_FILE_NAME, O_RDWR | O_CREAT))
          {
            tmpstate = E_INTERNAL_ERROR;
            is_sdcard_error = true;
            result[F("state")] = F("error");
            LOGE(F("SD Card opening ptr data on file %s... [ %s ]"), SDCARD_MQTT_PTR_RPC_FILE_NAME, FAIL_STRING);
            rpc_state = RPC_END;
            break;
          }

          tmElements_t datetime;
          datetime.Year = CalendarYrToTm(it.value().as<JsonArray>()[0].as<int>());
          datetime.Month = it.value().as<JsonArray>()[1].as<int>();
          datetime.Day = it.value().as<JsonArray>()[2].as<int>();
          datetime.Hour = it.value().as<JsonArray>()[3].as<int>();
          datetime.Minute = it.value().as<JsonArray>()[4].as<int>();
          datetime.Second = it.value().as<JsonArray>()[5].as<int>();
          ptr_time = makeTime(datetime);
          LOGN(F("RPC Data pointer... [ %d/%d/%d %d:%d:%d ]"), datetime.Day, datetime.Month, tmYearToCalendar(datetime.Year), datetime.Hour, datetime.Minute, datetime.Second);

          rpc_state = RPC_EXECUTE;

          break;
        }
      }

      if (!found)
      {
        tmpstate = E_INVALID_PARAMS;
        result[F("state")] = F("error");
        LOGE(F("Invalid params [ %s ]"), FAIL_STRING);

        rpc_state = RPC_END;
      }
    }
    break;

  case RPC_EXECUTE:

    if (mqtt_ptr_rpc_file.seekSet(0) && mqtt_ptr_rpc_file.write(&ptr_time, sizeof(time_t)) == sizeof(time_t))
    {
      mqtt_ptr_rpc_file.flush();
      mqtt_ptr_rpc_file.close();

      LOGN(F("SD Card writing ptr data on file %s... [ %s ]"), SDCARD_MQTT_PTR_RPC_FILE_NAME, OK_STRING);
      tmpstate = E_SUCCESS;
      result[F("state")] = F("done");
    }
    else
    {
      tmpstate = E_INTERNAL_ERROR;
      result[F("state")] = F("error");
      LOGE(F("SD Card writing ptr data on file %s... [ %s ]"), SDCARD_MQTT_PTR_RPC_FILE_NAME, FAIL_STRING);
    }

    rpc_state = RPC_END;

  case RPC_END:

    rpc_state = RPC_INIT;
    state = tmpstate;
    break;
  }

  return state;
}
#endif

#if (USE_RPC_METHOD_REBOOT)
int reboot(JsonObject params, JsonObject result)
{
  // print lcd message before reboot

  for (JsonPair it : params)
  {
    // loop in params
    if (strcmp(it.key().c_str(), "update") == 0)
    {
      if (it.value().as<bool>() == true)
      {
        TRACE_INFO_F(F("UPDATE FIRMWARE\r\n"));
        // set_default_configuration();
        // lcd_error |= lcd.clear();
        // lcd_error |= lcd.print(F("Reset configuration")) == 0;
      }
    }
  }

  TRACE_INFO_F(F("DO RESET CONFIGURATION\r\n"));

  TRACE_INFO_F(F("Reboot\r\n"));
  result[F("state")] = "done";
  NVIC_SystemReset(); // Do reboot!
  return E_SUCCESS;
}
#endif
