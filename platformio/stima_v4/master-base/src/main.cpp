#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"

static Queue *requestDataQueue;
static Queue *reportDataQueue;

void setup() {

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

  LCDParam_t lcdParam;
  lcdParam.configuration = &configuration;
  #if (ENABLE_I2C2)
  lcdParam.wire = &Wire2;
  lcdParam.wireLock = wire2Lock;
  #endif

  CanParam_t can_param;
  can_param.requestDataQueue = requestDataQueue;
  can_param.reportDataQueue = reportDataQueue;
  #if (ENABLE_I2C2)
  can_param.wire = &Wire2;
  can_param.wireLock = wire2Lock;
  #endif

  SupervisorParam_t supervisorParam;
  supervisorParam.configuration = &configuration;
  #if (ENABLE_I2C2)
  supervisorParam.wire = &Wire2;
  supervisorParam.wireLock = wire2Lock;
  #endif
  supervisorParam.configurationLock = configurationLock;

  static ProvaTask prova_task("PROVA TASK", 600, OS_TASK_PRIORITY_01, provaParam);
  static LCDTask lcd_task("LCD TASK", 600, OS_TASK_PRIORITY_02, lcdParam);
  static CanTask can_task("CAN TASK", 12000, OS_TASK_PRIORITY_03, can_param);
  static SupervisorTask supervisor_task("SUPERVISOR TASK", 800, OS_TASK_PRIORITY_04, supervisorParam);

  // Startup Schedulher
  Thread::StartScheduler();
}

void loop() {
}

void input_pin_encoder_A() {
  Serial.print("ENC_A Event: ");
  Serial.print(digitalRead(PIN_ENCODER_A));
  Serial.print(digitalRead(PIN_ENCODER_B));
  Serial.print(digitalRead(PIN_ENCODER_INT));
}

void input_pin_encoder_B() {
  Serial.print("ENC_B Event: ");
  Serial.print(digitalRead(PIN_ENCODER_A));
  Serial.print(digitalRead(PIN_ENCODER_B));
  Serial.print(digitalRead(PIN_ENCODER_INT));
}

void input_pin_encoder_C() {
  Serial.print("ENC_ENT Event: ");
  Serial.print(digitalRead(PIN_ENCODER_A));
  Serial.print(digitalRead(PIN_ENCODER_B));
  Serial.print(digitalRead(PIN_ENCODER_INT));
}

void init_pins()
{
  // Attach interrrupt
  attachInterrupt(PIN_ENCODER_A, input_pin_encoder_A, CHANGE);
  attachInterrupt(PIN_ENCODER_B, input_pin_encoder_B, CHANGE);
  attachInterrupt(PIN_ENCODER_INT, input_pin_encoder_C, CHANGE);
  // Enable Encoder && Display
  digitalWrite(PIN_ENCODER_EN5, 1);
  digitalWrite(PIN_DSP_POWER, 1);
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