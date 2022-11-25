#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include <Arduino.h>
#include "main.h"
#include <U8g2lib.h>

U8G2_SH1108_128X160_F_2ND_HW_I2C u8g2(U8G2_R1);

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

  u8g2.begin();
  // Test Display Output
  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.drawStr(0,10,"Hello World!");	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display


  TRACE_INFO_F(F("Initialization HW Base done\r\n"));

  ProvaParam_t provaParam = {};

  // SupervisorParam_t supervisorParam;
  // supervisorParam.configuration = &configuration;
  // #if (ENABLE_I2C2)
  // supervisorParam.wire = &Wire2;
  // supervisorParam.wireLock = wire2Lock;
  // #endif
  // supervisorParam.configurationLock = configurationLock;

  static ProvaTask prova_task("PROVA TASK", 100, OS_TASK_PRIORITY_01, provaParam);
  // static SupervisorTask supervisor_task("SUPERVISOR TASK", 100, OS_TASK_PRIORITY_01, supervisorParam);

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