#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"

void setup() {

  // Semaphore, Queue && Param Config for TASK
#if (ENABLE_I2C1)
  static BinarySemaphore *wireLock;       // Access I2C internal EEprom, Accelerometer
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

  // Hardware Semaphore
#if (ENABLE_I2C1)
  wireLock = new BinarySemaphore(true);
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

  TRACE_INFO_F(F("Initialization HW Base done\r\n"));

  // ***************************************************************
  //                  Setup parameter for Task
  // ***************************************************************

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

  // *****************************************************************************
  // Startup Task, Supervisor as first for Loading parameter generic configuration
  // *****************************************************************************

  static ElaborateDataTask elaborate_data_task("ElaborateDataTask", 350, OS_TASK_PRIORITY_02, elaborateDataParam);

  // Run Schedulher
  Thread::StartScheduler();

}

// FreeRTOS idleHook callBack to loop
void loop() {
}

/// @brief Init Pin (Diag and configuration)
void init_pins() {
}

// Setup Wire I2C Interface
void init_wire()
{
#if (ENABLE_I2C1)
  Wire.begin();
  Wire.setClock(I2C1_BUS_CLOCK_HZ);
#endif
}
