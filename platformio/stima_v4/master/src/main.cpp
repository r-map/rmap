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
  JsonRPC streamRpc();

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

  if (params.isNull())
    is_mqtt_rpc_delay = true; // configure without params is used
                              // to set a long delay before disconnect
                              // after the data are sended
  for (JsonPair it : params)
  {
    if (strcmp(it.key().c_str(), "reset") == 0)
    {
      if (it.value().as<bool>() == true)
      {
        set_default_configuration();
#if (USE_LCD)
        lcd_error |= lcd.clear();
        lcd_error |= lcd.print(F("Reset configuration")) == 0;
#endif
      }
    }
    else if (strcmp(it.key().c_str(), "save") == 0)
    {
      if (it.value().as<bool>() == true)
      {
        save_configuration(CONFIGURATION_CURRENT);
#if (USE_LCD)
        lcd_error |= lcd.clear();
        lcd_error |= lcd.print(F("Save configuration")) == 0;
#endif
      }
    }
#if (USE_MQTT)
    else if (strcmp(it.key().c_str(), "mqttserver") == 0)
    {
      strncpy(writable_configuration.mqtt_server, it.value().as<const char *>(), MQTT_SERVER_LENGTH);
    }
    else if (strcmp(it.key().c_str(), "mqttrootpath") == 0)
    {
      strncpy(writable_configuration.mqtt_root_topic, it.value().as<const char *>(), MQTT_ROOT_TOPIC_LENGTH);
    }
    else if (strcmp(it.key().c_str(), "mqttrpcpath") == 0)
    {
      strncpy(writable_configuration.mqtt_rpc_topic, it.value().as<const char *>(), MQTT_RPC_TOPIC_LENGTH);
    }
    else if (strcmp(it.key().c_str(), "mqttmaintpath") == 0)
    {
      strncpy(writable_configuration.mqtt_maint_topic, it.value().as<const char *>(), MQTT_MAINT_TOPIC_LENGTH);
    }
    else if (strcmp(it.key().c_str(), "mqttsampletime") == 0)
    {
      writable_configuration.report_seconds = it.value().as<unsigned int>();
    }
    else if (strcmp(it.key().c_str(), "mqttuser") == 0)
    {
      strncpy(writable_configuration.mqtt_username, it.value().as<const char *>(), MQTT_USERNAME_LENGTH);
    }
    else if (strcmp(it.key().c_str(), "mqttpassword") == 0)
    {
      strncpy(writable_configuration.mqtt_password, it.value().as<const char *>(), MQTT_PASSWORD_LENGTH);
    }
    else if (strcmp(it.key().c_str(), "stationslug") == 0)
    {
      strncpy(writable_configuration.stationslug, it.value().as<const char *>(), STATIONSLUG_LENGTH);
    }
    else if (strcmp(it.key().c_str(), "boardslug") == 0)
    {
      strncpy(writable_configuration.boardslug, it.value().as<const char *>(), BOARDSLUG_LENGTH);
    }
    else if (strcmp(it.key().c_str(), "mqttpskkey") == 0)
    {
      // skip it
    }
    else if (strcmp(it.key().c_str(), "sd") == 0)
    {
      for (JsonPair sd : it.value().as<JsonObject>())
      {
        strncpy(writable_configuration.constantdata[writable_configuration.constantdata_count].btable, sd.key().c_str(), CONSTANTDATA_BTABLE_LENGTH);
        strncpy(writable_configuration.constantdata[writable_configuration.constantdata_count].value, sd.value().as<const char *>(), CONSTANTDATA_VALUE_LENGTH);

        if (writable_configuration.sensors_count < USE_CONSTANTDATA_COUNT)
          writable_configuration.constantdata_count++;
        else
        {
          is_error = true;
        }
      }
    }
#endif
#if (USE_NTP)
    else if (strcmp(it.key().c_str(), "ntpserver") == 0)
    {
      strncpy(writable_configuration.ntp_server, it.value().as<const char *>(), NTP_SERVER_LENGTH);
    }
#endif
    else if (strcmp(it.key().c_str(), "date") == 0)
    {
#if (USE_RTC)

      tmElements_t tm;
      tm.Year = y2kYearToTm(it.value().as<JsonArray>()[0].as<int>() - 2000);
      tm.Month = it.value().as<JsonArray>()[1].as<int>();
      tm.Day = it.value().as<JsonArray>()[2].as<int>();
      tm.Hour = it.value().as<JsonArray>()[3].as<int>();
      tm.Minute = it.value().as<JsonArray>()[4].as<int>();
      tm.Second = it.value().as<JsonArray>()[5].as<int>();

      system_time = makeTime(tm);
      setTime(system_time);
      /*
            Pcf8563::disable();
            Pcf8563::setDate(it.value().as<JsonArray>()[2].as<int>(), it.value().as<JsonArray>()[1].as<int>(), it.value().as<JsonArray>()[0].as<int>() - 2000, weekday()-1, 0);
            Pcf8563::setTime(it.value().as<JsonArray>()[3].as<int>(), it.value().as<JsonArray>()[4].as<int>(), it.value().as<JsonArray>()[5].as<int>());
            Pcf8563::enable();
      */
      setSyncInterval(NTP_TIME_FOR_RESYNC_S);
      setSyncProvider(getSystemTime);
#elif (USE_TIMER_1)
      setTime(it.value().as<JsonArray>()[3].as<int>(), it.value().as<JsonArray>()[4].as<int>(), it.value().as<JsonArray>()[5].as<int>(), it.value().as<JsonArray>()[2].as<int>(), it.value().as<JsonArray>()[1].as<int>(), it.value().as<JsonArray>()[0].as<int>() - 2000);
#endif
    }
    else if (strcmp(it.key().c_str(), "mac") == 0)
    {
#if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH)
      for (uint8_t i = 0; i < ETHERNET_MAC_LENGTH; i++)
      {
        writable_configuration.ethernet_mac[i] = it.value().as<JsonArray>()[i];
      }
#else
      LOGV(F("Configuration mac parameter ignored"));
#endif
    }
#if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM)
    else if (strcmp(it.key().c_str(), "gsmapn") == 0)
    {
      strncpy(writable_configuration.gsm_apn, it.value().as<const char *>(), GSM_APN_LENGTH);
    }
#endif
    else if (strcmp(it.key().c_str(), "driver") == 0)
    {
      strncpy(writable_configuration.sensors[writable_configuration.sensors_count].driver, it.value().as<const char *>(), DRIVER_LENGTH);
      is_sensor_config = true;
    }
    else if (strcmp(it.key().c_str(), "type") == 0)
    {
      strncpy(writable_configuration.sensors[writable_configuration.sensors_count].type, it.value().as<const char *>(), TYPE_LENGTH);
      is_sensor_config = true;
    }
    else if (strcmp(it.key().c_str(), "address") == 0)
    {
      writable_configuration.sensors[writable_configuration.sensors_count].address = it.value().as<unsigned char>();
      is_sensor_config = true;
    }
    else if (strcmp(it.key().c_str(), "node") == 0)
    {
      writable_configuration.sensors[writable_configuration.sensors_count].node = it.value().as<unsigned char>();
      is_sensor_config = true;
    }
    else if (strcmp(it.key().c_str(), "mqttpath") == 0)
    {
      strncpy(writable_configuration.sensors[writable_configuration.sensors_count].mqtt_topic, it.value().as<const char *>(), MQTT_SENSOR_TOPIC_LENGTH);
      is_sensor_config = true;
    }
    else
    {
      is_error = true;
    }
  }

  if (is_sensor_config)
  {
    if (writable_configuration.sensors_count < SENSORS_MAX)
      writable_configuration.sensors_count++;
    else
    {
      is_error = true;
    }
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
#if (USE_LCD)
  // print lcd message before reboot
#endif
  TRACE_INFO_F(F("Reboot\r\n"));
  result[F("state")] = "done";
  NVIC_SystemReset(); // Do reboot!
  return E_SUCCESS;
}
#endif
