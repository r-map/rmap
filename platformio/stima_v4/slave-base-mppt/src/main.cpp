#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"

void setup() {

  // Semaphore, Queue && Param Config for TASK
  #if (ENABLE_I2C1)
  static BinarySemaphore *wireLock;
  #endif
  #if (ENABLE_I2C2)
  static BinarySemaphore *wire2Lock;
  #endif
  static BinarySemaphore *configurationLock;
  static configuration_t config_module;
  static Queue *elaborataDataQueue;
  static Queue *requestDataQueue;
  static Queue *reportDataQueue;

  // Initializing basic hardware's configuration
  SetupSystemPeripheral();

  // Start and Setup local Module
  // Except local HW config for module TASK
  init_debug(115200);
  init_wire();
  init_pins();

  // Init Semaphore
#if (ENABLE_I2C1)
  wireLock = new BinarySemaphore(true);
#endif
#if (ENABLE_I2C2)
  wire2Lock = new BinarySemaphore(true);
#endif
  configurationLock = new BinarySemaphore(true);

  // Init required sensor && driver param
  init_sensors();

  TRACE_INFO_F(F("Initialization HW Base done\r\n"));

  // ********************************************************
  //                     Param Config Task
  // ********************************************************
  ProvaParam_t provaParam = {};

  CanParam_t can_param;
  can_param.requestDataQueue = requestDataQueue;
  can_param.reportDataQueue = reportDataQueue;
#if (ENABLE_I2C1)
  can_param.wire = &Wire;
  can_param.wireLock = wireLock;
#endif

  SupervisorParam_t supervisorParam;
  supervisorParam.configuration = &config_module;
#if (ENABLE_I2C1)
  supervisorParam.wire = &Wire;
  supervisorParam.wireLock = wireLock;
#endif
  supervisorParam.configurationLock = configurationLock;

  static ProvaTask prova_task("PROVA TASK", 100, OS_TASK_PRIORITY_01, provaParam);
  static CanTask can_task("CAN TASK", 8192, OS_TASK_PRIORITY_02, can_param);
  static SupervisorTask supervisor_task("SUPERVISOR TASK", 200, OS_TASK_PRIORITY_01, supervisorParam);

  // Startup Schedulher
  Thread::StartScheduler();
}

void loop() {
}

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

void init_sensors()
{
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
