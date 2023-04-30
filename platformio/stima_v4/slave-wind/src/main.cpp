#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"

void setup() {

  // Semaphore, Queue && Param Config for TASK
#if (ENABLE_I2C1)
  static BinarySemaphore *wireLock;       // Access I2C internal EEprom, Accelerometer
#endif

#if (ENABLE_I2C2)
  static BinarySemaphore *wire2Lock;      // Access I2C External (Sensor)
#endif

#if (ENABLE_CAN)
  static BinarySemaphore *canLock;        // Can BUS
#endif

#if (ENABLE_QSPI)
  static BinarySemaphore *qspiLock;       // Qspi (Flash Memory)
#endif

  static BinarySemaphore *rtcLock;        // RTC (Access lock)

  // System Queue (Generic Message from/to Task)
  static Queue *systemMessageQueue;
  // Data queue (Request / exchange data from Can to Sensor and Elaborate Task)
  static Queue *elaborataDataQueue;
  static Queue *requestDataQueue;
  static Queue *reportDataQueue;

  // System semaphore
  static BinarySemaphore *configurationLock;  // Access Configuration
  static BinarySemaphore *systemStatusLock;   // Access System status
  static BinarySemaphore *registerAccessLock; // Access Register Cyphal Specifications

  // System and status configuration struct
  static configuration_t configuration = {0};
  static system_status_t system_status = {0};

  // Initializing basic hardware's configuration
  SetupSystemPeripheral();
  init_debug(SERIAL_DEBUG_BAUD_RATE);
  init_wire();
  init_pins();
  init_rtc(INIT_PARAMETER);

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
  elaborataDataQueue = new Queue(ELABORATE_DATA_QUEUE_LENGTH, sizeof(elaborate_data_t));
  requestDataQueue = new Queue(REQUEST_DATA_QUEUE_LENGTH, sizeof(request_data_t));
  reportDataQueue = new Queue(REPORT_DATA_QUEUE_LENGTH, sizeof(report_t));

  TRACE_INFO_F(F("Initialization HW Base done\r\n"));

  // Get Serial Number
  configuration.serial_number = StimaV4GetSerialNumber();

  // Serial Print Fixed for Serial Number
  Serial.println();
  Serial.println(F("*****************************"));
  Serial.println(F("* Stima V4 SLAVE - SER.NUM. *"));
  Serial.println(F("*****************************"));
  Serial.print(F("COD: "));
  for(int8_t id=7; id>=0; id--) {
    if((uint8_t)((configuration.serial_number >> (8*id)) & 0xFF) < 16) Serial.print(F("0"));
    Serial.print((uint8_t)((configuration.serial_number >> (8*id)) & 0xFF), 16);
    if(id) Serial.print(F("-"));
  }
  Serial.println("\r\n");

  // ***************************************************************
  //                  Setup parameter for Task
  // ***************************************************************

#if (ENABLE_I2C1)
  // Load Info from E2 boot_check flag and send to Config
  static EEprom  memEprom(&Wire, wireLock);
  bootloader_t boot_check = {0};
  #if INIT_PARAMETER
  boot_check.app_executed_ok = true;
  memEprom.Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_check, sizeof(boot_check));
  #else
  memEprom.Read(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_check, sizeof(boot_check));
  #endif
  // Optional send other InfoParm Boot (Uploaded, rollback, error fail ecc.. to config)
#endif
  // Reset Factory register value
  static EERegister clRegister(&Wire, wireLock);
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
  wdtParam.eeprom = &memEprom;

#if (ENABLE_CAN)
  // TASK CAN PARAM CONFIG
  static CanParam_t canParam = {0};
  canParam.configuration = &configuration;
  canParam.system_status = &system_status;
  canParam.boot_request = &boot_check;
  canParam.configurationLock = configurationLock;
  canParam.systemStatusLock = systemStatusLock;
  canParam.registerAccessLock = registerAccessLock;
  canParam.systemMessageQueue = systemMessageQueue;
  canParam.requestDataQueue = requestDataQueue;
  canParam.reportDataQueue = reportDataQueue;
  canParam.eeprom = &memEprom;
  canParam.clRegister = &clRegister;
  canParam.flash = &memFlash;
  canParam.canLock = canLock;
  canParam.qspiLock = qspiLock;  
  canParam.rtcLock = rtcLock;
#endif

#if (ENABLE_ACCELEROMETER)
  // TASK ACCELEROMETER PARAM CONFIG
  static AccelerometerParam_t accelerometerParam = {0};
  accelerometerParam.configuration = &configuration;
  accelerometerParam.system_status = &system_status;
  accelerometerParam.clRegister = &clRegister;
#if (ENABLE_I2C1)
  accelerometerParam.wire = &Wire;
  accelerometerParam.wireLock = wireLock;
#endif
  accelerometerParam.systemStatusLock = systemStatusLock;
  accelerometerParam.registerAccessLock = registerAccessLock;
  accelerometerParam.systemMessageQueue = systemMessageQueue;
#endif

#if (MODULE_TYPE == STIMA_MODULE_TYPE_WIND)
  static WindSensorParam_t windSensorParam = {0};
  windSensorParam.configuration = &configuration;
  windSensorParam.system_status = &system_status;
  windSensorParam.configurationLock = configurationLock;
  windSensorParam.systemStatusLock = systemStatusLock;
  windSensorParam.systemMessageQueue = systemMessageQueue;
  windSensorParam.elaborataDataQueue = elaborataDataQueue;
#endif

  // TASK ELABORATE DATA PARAM CONFIG
  static ElaborateDataParam_t elaborateDataParam = {0};
  elaborateDataParam.configuration = &configuration;
  elaborateDataParam.system_status = &system_status;
  elaborateDataParam.configurationLock = configurationLock;
  elaborateDataParam.systemStatusLock = systemStatusLock;
  elaborateDataParam.systemMessageQueue = systemMessageQueue;
  elaborateDataParam.elaborataDataQueue = elaborataDataQueue;
  elaborateDataParam.requestDataQueue = requestDataQueue;
  elaborateDataParam.reportDataQueue = reportDataQueue;

  // TASK SUPERVISOR PARAM CONFIG
  static SupervisorParam_t supervisorParam = {0};
  supervisorParam.configuration = &configuration;
  supervisorParam.system_status = &system_status;
  supervisorParam.clRegister = &clRegister;
  supervisorParam.configurationLock = configurationLock;
  supervisorParam.systemStatusLock = systemStatusLock;
  supervisorParam.registerAccessLock = registerAccessLock;
  supervisorParam.systemMessageQueue = systemMessageQueue;

  // *****************************************************************************
  // Startup Task, Supervisor as first for Loading parameter generic configuration
  // *****************************************************************************
  static SupervisorTask supervisor_task("SupervisorTask", 300, OS_TASK_PRIORITY_04, supervisorParam);

#if (MODULE_TYPE == STIMA_MODULE_TYPE_WIND)
  static WindSensorTask wind_sensor_task("WindTask", 350, OS_TASK_PRIORITY_03, windSensorParam);
#endif
  static ElaborateDataTask elaborate_data_task("ElaborateDataTask", 350, OS_TASK_PRIORITY_02, elaborateDataParam);

#if (ENABLE_ACCELEROMETER)
  static AccelerometerTask accelerometer_task("AccelerometerTask", 350, OS_TASK_PRIORITY_01, accelerometerParam);
#endif

#if (ENABLE_CAN)
  static CanTask can_task("CanTask", 7200, OS_TASK_PRIORITY_02, canParam);
#endif

#if (ENABLE_WDT)
  static WdtTask wdt_task("WdtTask", 350, OS_TASK_PRIORITY_01, wdtParam);
#endif

  // Run Schedulher
  Thread::StartScheduler();

}

// FreeRTOS idleHook callBack to loop
void loop() {
  // Enable LowPower idleHock reduce power consumption without disable sysTick
  LowPower.idleHook();
}

/// @brief Init Pin (Diag and configuration)
void init_pins() {
  #if (ENABLE_DIAG_PIN)
  // *****************************************************
  //     STARTUP LED E PIN DIAGNOSTICI (SE UTILIZZATI)
  // *****************************************************
  // Output mode for LED(1/2) BLINK SW LOOP (High per Setup)
  // Input mode for test button
  #ifdef LED1_PIN
  pinMode(LED1_PIN, OUTPUT);
  digitalWrite(LED1_PIN, HIGH);
  #endif
  #ifdef LED2_PIN
  pinMode(LED2_PIN, OUTPUT);
  digitalWrite(LED1_PIN, LOW);
  #endif
  #ifdef USER_INP
  pinMode(USER_INP, INPUT);
  #endif
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
