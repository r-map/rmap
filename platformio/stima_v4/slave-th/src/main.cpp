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
  // Software Semaphore
  configurationLock = new BinarySemaphore(true);
  systemStatusLock = new BinarySemaphore(true);
  registerAccessLock = new BinarySemaphore(true);

  // Creating queue
  systemMessageQueue = new Queue(SYSTEM_MESSAGE_QUEUE_LENGTH, sizeof(system_message_t));
  elaborataDataQueue = new Queue(ELABORATE_DATA_QUEUE_LENGTH, sizeof(elaborate_data_t));
  requestDataQueue = new Queue(REQUEST_DATA_QUEUE_LENGTH, sizeof(request_data_t));
  reportDataQueue = new Queue(REPORT_DATA_QUEUE_LENGTH, sizeof(report_t));

  TRACE_INFO_F(F("MAIN: Initialization HW Base done\r\n"));

  // ***************************************************************
  //                  Setup parameter for Task
  // ***************************************************************
#if (ENABLE_INFO)
  // TASK INFO PARAM CONFIG
  static InfoParam_t infoParam = {0};
  infoParam.system_status = &system_status;
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
  canParam.requestDataQueue = requestDataQueue;
  canParam.reportDataQueue = reportDataQueue;
  canParam.canLock = canLock;  
  canParam.qspiLock = qspiLock;  
#if (ENABLE_I2C1)
  canParam.wire = &Wire;
  canParam.wireLock = wireLock;
#endif
#endif

#if (ENABLE_ACCELEROMETER)
  // TASK ACCELEROMETER PARAM CONFIG
  static AccelerometerParam_t accelerometerParam = {0};
  accelerometerParam.configuration = &configuration;
  accelerometerParam.system_status = &system_status;
#if (ENABLE_I2C1)
  accelerometerParam.wire = &Wire;
  accelerometerParam.wireLock = wireLock;
#endif
  accelerometerParam.systemStatusLock = systemStatusLock;
  accelerometerParam.registerAccessLock = registerAccessLock;
  accelerometerParam.systemMessageQueue = systemMessageQueue;
#endif

#if ((MODULE_TYPE == STIMA_MODULE_TYPE_THR) || (MODULE_TYPE == STIMA_MODULE_TYPE_TH))
  // TASK SENSOR PARAM CONFIG
  static TemperatureHumidtySensorParam_t thSensorParam = {0};
  thSensorParam.configuration = &configuration;
  thSensorParam.system_status = &system_status;
#if (ENABLE_I2C2)
  thSensorParam.wire = &Wire2;
  thSensorParam.wireLock = wire2Lock;
#endif
  thSensorParam.configurationLock = configurationLock;
  thSensorParam.systemStatusLock = systemStatusLock;
  thSensorParam.systemMessageQueue = systemMessageQueue;
  thSensorParam.elaborataDataQueue = elaborataDataQueue;
#endif

  // TASK ELABORATE DATA PARAM CONFIG
  static ElaboradeDataParam_t elaborateDataParam = {0};
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
#if (ENABLE_I2C1)
  supervisorParam.wire = &Wire;
  supervisorParam.wireLock = wireLock;
#endif
  supervisorParam.configurationLock = configurationLock;
  supervisorParam.systemStatusLock = systemStatusLock;
  supervisorParam.registerAccessLock = registerAccessLock;
  supervisorParam.systemMessageQueue = systemMessageQueue;

  #if INIT_PARAMETER
  // Reset Factory register value
  EERegister initRegister(&Wire, wireLock);
  initRegister.doFactoryReset();
  #endif

  // *****************************************************************************
  // Startup Task, Supervisor as first for Loading parameter generic configuration
  // *****************************************************************************
  static SupervisorTask supervisor_task("SupervisorTask", 350, OS_TASK_PRIORITY_04, supervisorParam);

#if ((MODULE_TYPE == STIMA_MODULE_TYPE_THR) || (MODULE_TYPE == STIMA_MODULE_TYPE_TH))
  static TemperatureHumidtySensorTask th_sensor_task("THTask", 400, OS_TASK_PRIORITY_03, thSensorParam);
#endif
  static ElaborateDataTask elaborate_data_task("ElaborateDataTask", 250, OS_TASK_PRIORITY_02, elaborateDataParam);

#if (ENABLE_ACCELEROMETER)
  static AccelerometerTask accelerometer_task("AccelerometerTask", 400, OS_TASK_PRIORITY_01, accelerometerParam);
#endif

#if (ENABLE_CAN)
  static CanTask can_task("CanTask", 7100, OS_TASK_PRIORITY_02, canParam);
#endif

#if (ENABLE_INFO)
  static InfoTask info_task("InfoTask", 100, OS_TASK_PRIORITY_01, infoParam);
#endif

  // Run Schedulher
  Thread::StartScheduler();

}

// FreeRTOS idleHook callBack to loop
void loop() {
  // Enable LowPower idleHock power consumption
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
