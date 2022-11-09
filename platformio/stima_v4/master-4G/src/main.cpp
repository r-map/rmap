#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"

void setup() {
  // osInitKernel();

  // Initializing basic hardware's configuration
  SetupSystemPeripheral();
  init_debug(115200);
  init_wire();
  init_pins();
  init_tasks();
  init_sensors();
  // init_sdcard();
  // init_registers();
  // init_can();

  // uint8_t write[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  // uint8_t read[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  // BSP_QSPI_Init();
  // read[0] = 1;
  // hqspi.State = HAL_QSPI_STATE_READY;
  // while (BSP_QSPI_GetStatus() != QSPI_OK);
  // BSP_QSPI_Erase_Block(0);
  // BSP_QSPI_Write(write, 0, sizeof(uint8_t) * 10);
  // BSP_QSPI_Read(read, 0, sizeof(uint8_t) * 10);

  // error_t error = NO_ERROR;

  // Initialize hardware cryptographic accelerator
  // error = stm32l4xxCryptoInit();
  // // Any error to report?
  // if (error)
  // {
  //   // Debug message
  //   TRACE_ERROR("Failed to initialize hardware crypto accelerator!\r\n");
  // }

  // // TCP/IP stack initialization
  // error = netInit();
  // if (error)
  // {
  //   TRACE_ERROR("Failed to initialize TCP/IP stack!\r\n");
  // }

  TRACE_INFO_F(F("Initialization HW Base done\r\n"));

  ProvaParam_t provaParam = {};

  SupervisorParam_t supervisorParam;
  supervisorParam.configuration = &configuration;
#if (ENABLE_I2C2)
  supervisorParam.wire = &Wire2;
  supervisorParam.wireLock = wire2Lock;
#endif
  supervisorParam.configurationLock = configurationLock;

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
  ModemParam_t modemParam;
#endif

  static ProvaTask prova_task("PROVA TASK", 100, OS_TASK_PRIORITY_01, provaParam);
  static SupervisorTask supervisor_task("SUPERVISOR TASK", 100, OS_TASK_PRIORITY_01, supervisorParam);
#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
  static ModemTask modem_task("MODEM TASK", 100, OS_TASK_PRIORITY_01, modemParam);
#endif

  // Startup Schedulher
  Thread::StartScheduler();
}

void loop() {
}

void init_pins()
{
#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
  pinMode(PIN_GSM_EN_POW, OUTPUT);
  pinMode(PIN_GSM_PW_KEY, OUTPUT);
  pinMode(PIN_7600E_RI, INPUT);

  digitalWrite(PIN_GSM_EN_POW, LOW);
  digitalWrite(PIN_GSM_PW_KEY, LOW);
#endif
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