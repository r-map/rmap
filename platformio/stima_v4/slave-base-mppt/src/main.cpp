#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"

void setup() {
  // Static for Freertos local visibility
  static configuration_t config_module;

  // Initializing basic hardware's configuration
  SetupSystemPeripheral();
  init_debug(115200);
  init_wire();
  // init_pins();
  init_tasks();
  init_sensors();
  // init_registers();
  // init_can();

  delay(500);

  TRACE_INFO_F(F("Initialization HW Base done\r\n"));

  ProvaParam_t provaParam = {};

  SupervisorParam_t supervisorParam;
  supervisorParam.configuration = &config_module;
#if (ENABLE_I2C1)
  supervisorParam.wire = &Wire;
  supervisorParam.wireLock = wireLock;
#endif
  supervisorParam.configurationLock = configurationLock;

  static ProvaTask prova_task("PROVA TASK", 100, OS_TASK_PRIORITY_01, provaParam);
  static SupervisorTask supervisor_task("SUPERVISOR TASK", 200, OS_TASK_PRIORITY_01, supervisorParam);

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
#if (ENABLE_I2C1)
  wireLock = new BinarySemaphore(true);
#endif
#if (ENABLE_I2C2)
  wire2Lock = new BinarySemaphore(true);
#endif
  configurationLock = new BinarySemaphore(true);
}

void init_sensors()
{
}

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