#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"
#include "bxcan.h"
#include "register.hpp"

void setup() {
  // Static for Freertos local visibility
  static accelerometer_t config_accelerometer;
  static configuration_t config_module;
  static Queue *elaborataDataQueue;
  static Queue *requestDataQueue;
  static Queue *reportDataQueue;

  // Initializing basic hardware's configuration
  SetupSystemPeripheral();

  // Start and Setup local Module
  init_debug(115200);
  init_wire();
  init_pins();
  init_tasks();
  init_sensors();
  init_registers();
  init_can();

  TRACE_INFO_F(F("Initialization HW Base done\r\n"));

  // ********************************************************
  //                     Param Config Task
  // ********************************************************
  ProvaParam_t provaParam = {};

  AccelerometerParam_t accelerometerParam;
  accelerometerParam.configuration = &config_accelerometer;
#if (ENABLE_I2C1)
  accelerometerParam.wire = &Wire;
  accelerometerParam.wireLock = wireLock;
#endif
  accelerometerParam.configurationLock = configurationLock;

  CanParam_t can_param;
  can_param.requestDataQueue = requestDataQueue;
  can_param.reportDataQueue = reportDataQueue;

  SupervisorParam_t supervisorParam;
  supervisorParam.configuration = &config_module;
#if (ENABLE_I2C1)
  supervisorParam.wire = &Wire;
  supervisorParam.wireLock = wireLock;
#endif
  supervisorParam.configurationLock = configurationLock;

  // ********************************************************
  //                     Startup Task
  // ********************************************************
  static ProvaTask prova_task("PROVA TASK", 100, OS_TASK_PRIORITY_01, provaParam);
  static AccelerometerTask accelerometer_task("ACCELEROMETER TASK", 400, OS_TASK_PRIORITY_01, accelerometerParam);
  static CanTask can_task("CAN TASK", 8192, OS_TASK_PRIORITY_02, can_param);
  static SupervisorTask supervisor_task("SUPERVISOR TASK", 200, OS_TASK_PRIORITY_01, supervisorParam);

  // ********************************************************
  //                    Run RTOS Scheduler
  // ********************************************************
  // Startup Schedulher
  Thread::StartScheduler();
  TRACE_INFO_F(F("RUN Scheduler\r\n"));
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

  #if (ENABLE_CAN)
  // Resume from LowPower or reset the controller TJA1443ATK
  // N.B.!!! Setup bxCAN before resume Power cause HAL_Error()
  digitalWrite(PIN_CAN_EN, 1);
  digitalWrite(PIN_CAN_STB, 1);
  #endif

}

void init_registers() {
  // ********************************************************************************
  //    FIXED REGISTER_INIT, FARE INIT OPZIONALE x REGISTRI FISSI ECC. E/O INVAR.
  // ********************************************************************************
  #ifdef INIT_REGISTER
  // Inizializzazione fissa dei registri nella modalità utilizzata (prepare SD/FLASH/ROM con default Value)
  registerSetup(true);
  #else
  // Default dei registri nella modalità utilizzata (prepare SD/FLASH/ROM con default Value)
  // Creazione dei registri standard base se non esistono
  registerSetup(false);
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

// *********************************************************************************************
//          Inizializzazione generale HW, canard, CAN_BUS, e dispositivi collegati
// *********************************************************************************************
void init_can(void)
{
  #if (!ENABLE_CAN)
  TRACE_INFO_F(F("CAN missing module, configuration abort..."));
  return;
  #endif

  TRACE_INFO_F(F("Starting CAN Configuration"));
  // *******************         CANARD SETUP TIMINGS AND SPEED        *******************
  // CAN BITRATE Dinamico su LoadRegister (CAN_FD 2xREG natural32 0=Speed, 1=0 (Not Used))
  uavcan_register_Value_1_0 val = {0};
  uavcan_register_Value_1_0_select_natural32_(&val);
  val.natural32.value.count       = 2;
  val.natural32.value.elements[0] = CAN_BIT_RATE;
  val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
  registerRead("uavcan.can.bitrate", &val);
  LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural32_(&val) && (val.natural32.value.count == 2));

  // Dynamic BIT RATE Change CAN Speed to CAN_BIT_RATE (register default/defined)
  BxCANTimings timings;
  bool result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
  if (!result) {
      TRACE_INFO_F(F("Error redefinition bxCANComputeTimings, try loading default..."));
      val.natural32.value.count       = 2;
      val.natural32.value.elements[0] = CAN_BIT_RATE;
      val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
      registerWrite("uavcan.can.bitrate", &val);
      result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
      if (!result) {
          TRACE_INFO_F(F("Error initialization bxCANComputeTimings"));
          LOCAL_ASSERT(false);
          return;
      }
  }

  // Attivazione bxCAN sulle interfacce richieste, velocità e modalità
  result = bxCANConfigure(0, timings, false);
  if (!result) {
      TRACE_INFO_F(F("Error initialization bxCANConfigure"));
      LOCAL_ASSERT(false);
      return;
  }
  // *******************     CANARD SETUP TIMINGS AND SPEED COMPLETE   *******************

  // Check error starting CAN
  if (HAL_CAN_Start(&hcan1) != HAL_OK) {
      TRACE_INFO_F(F("CAN startup ERROR!!!"));
      LOCAL_ASSERT(false);
      return;
  }

  // Enable Interrupt RX Standard CallBack -> CAN1_RX0_IRQHandler
  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
      TRACE_INFO_F(F("Error initialization interrupt CAN base"));
      LOCAL_ASSERT(false);
      return;
  }
  // Setup Priority e CB CAN_IRQ_RX Enable
  HAL_NVIC_SetPriority(CAN1_RX0_IRQn, CAN_NVIC_INT_PREMPT_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);

  // Setup Completato
  TRACE_INFO_F(F("CAN Configuration complete..."));
}
