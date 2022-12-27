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

  // System Queue
  static Queue *systemMessageQueue;
  // Data queue
  static Queue *elaborataDataQueue;
  static Queue *requestDataQueue;
  static Queue *reportDataQueue;

  // System semaphore
  static BinarySemaphore *configurationLock;
  static BinarySemaphore *systemStatusLock;

  // System configuration
  static configuration_t configuration = {0};
  static system_status_t system_status = {0};
  #if (ENABLE_ACCELEROMETER)
  static accelerometer_t config_accelerometer = {0};
  #endif

  // Hardware Semaphore
#if (ENABLE_I2C1)
  wireLock = new BinarySemaphore(true);
#endif
#if (ENABLE_I2C2)
  wire2Lock = new BinarySemaphore(true);
#endif
  configurationLock = new BinarySemaphore(true);
  systemStatusLock = new BinarySemaphore(true);

  // Creating queue
  systemMessageQueue = new Queue(SYSTEM_MESSAGE_QUEUE_LENGTH, sizeof(system_message_t));
  elaborataDataQueue = new Queue(ELABORATE_DATA_QUEUE_LENGTH, sizeof(elaborate_data_t));
  requestDataQueue = new Queue(REQUEST_DATA_QUEUE_LENGTH, sizeof(request_data_t));
  reportDataQueue = new Queue(REPORT_DATA_QUEUE_LENGTH, sizeof(report_t));

  // Initializing basic hardware's configuration
  SetupSystemPeripheral();
  init_debug(SERIAL_DEBUG_BAUD_RATE);
  init_wire();
  init_pins();
  init_rtc(false);

  // init_registers();

  // Alim Sens x I2C Test ( Force ON )
  // digitalWrite(PIN_EN_5VS, 1);  // Enable + 5VS / +3V3S External Connector Power Sens
  // digitalWrite(PIN_EN_SPLY, 1); // Enable Supply + 3V3_I2C / + 5V_I2C
  // digitalWrite(PIN_I2C2_EN, 1); // I2C External Enable PIN (LevelShitf PCA9517D)
  
  // Analog Read examples
  // digitalWrite(PIN_EN_5VA, 1); Enable Alim. Analog Comparator (xIAN)
  // analogReadResolution(12);
  // uint32_t data1;
  // uint32_t data2;
  // uint32_t data3;
  // uint32_t data4;
  // uint8_t count=0;
  // while(1) {
  //   data1 = analogRead(PIN_ANALOG_01);
  //   data2 = analogRead(PIN_ANALOG_02);
  //   data3 = analogRead(PIN_ANALOG_03);
  //   data4 = analogRead(PIN_ANALOG_04);
  //   delay(500);
  //   Serial.print(data1);
  //   Serial.print(", ");
  //   Serial.print(data2);
  //   Serial.print(", ");
  //   Serial.print(data3);
  //   Serial.print(", ");
  //   Serial.println(data4);
  // }

  TRACE_INFO_F(F("Initialization HW Base done\r\n"));

  // Setup paramete for Task

  ProvaParam_t provaParam = {};

#if (ENABLE_CAN)
  CanParam_t canParam;
  canParam.configuration = &configuration;
  canParam.system_status = &system_status;
  canParam.configurationLock = configurationLock;
  canParam.systemStatusLock = systemStatusLock;
  canParam.systemMessageQueue = systemMessageQueue;
  canParam.requestDataQueue = requestDataQueue;
  canParam.reportDataQueue = reportDataQueue;
#if (ENABLE_I2C1)
  canParam.wire = &Wire;
  canParam.wireLock = wireLock;
#endif
#endif

#if (ENABLE_ACCELEROMETER)
  AccelerometerParam_t accelerometerParam;
  accelerometerParam.configuration = &configuration;
  accelerometerParam.system_status = &system_status;
  accelerometerParam.accelerometer_configuration = &config_accelerometer;
#if (ENABLE_I2C1)
  accelerometerParam.wire = &Wire;
  accelerometerParam.wireLock = wireLock;
#endif
  accelerometerParam.configurationLock = configurationLock;
  accelerometerParam.systemStatusLock = systemStatusLock;
  accelerometerParam.systemMessageQueue = systemMessageQueue;
#endif

#if ((MODULE_TYPE == STIMA_MODULE_TYPE_THR) || (MODULE_TYPE == STIMA_MODULE_TYPE_TH))
  TemperatureHumidtySensorParam_t thSensorParam;
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

  ElaboradeDataParam_t elaborateDataParam;
  elaborateDataParam.configuration = &configuration;
  elaborateDataParam.system_status = &system_status;
  elaborateDataParam.configurationLock = configurationLock;
  elaborateDataParam.systemStatusLock = systemStatusLock;
  elaborateDataParam.systemMessageQueue = systemMessageQueue;
  elaborateDataParam.elaborataDataQueue = elaborataDataQueue;
  elaborateDataParam.requestDataQueue = requestDataQueue;
  elaborateDataParam.reportDataQueue = reportDataQueue;

  SupervisorParam_t supervisorParam;
  supervisorParam.configuration = &configuration;
  supervisorParam.system_status = &system_status;
#if (ENABLE_I2C1)
  supervisorParam.wire = &Wire;
  supervisorParam.wireLock = wireLock;
#endif
  supervisorParam.configurationLock = configurationLock;
  supervisorParam.systemStatusLock = systemStatusLock;
  supervisorParam.systemMessageQueue = systemMessageQueue;

// uint32_t pippo = 0;
//   while(1) {
//     Serial.flush();
//     Serial.print("A....");
//     Serial.println(pippo);
//     // Print date...
//     TRACE_INFO_F(F("%d/%d/%d "), rtc.getDay(), rtc.getMonth(), rtc.getYear());
//     // ...and time
//     TRACE_INFO_F(F("%d:%d:%d.%d\n"), rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getSubSeconds());
// //    delay(1000);
//     delay(20);
//     Serial.flush();
//     LowPower.idle(3000);

//     delay(200);
//     pippo++;
//     //delay(10);
//   }

  // ********************************************************
  //                     Startup Task
  // ********************************************************
  static ProvaTask prova_task("ProvaTask", 100, OS_TASK_PRIORITY_01, provaParam);
//  static SupervisorTask supervisor_task("SupervisorTask", 100, OS_TASK_PRIORITY_04, supervisorParam);

#if ((MODULE_TYPE == STIMA_MODULE_TYPE_THR) || (MODULE_TYPE == STIMA_MODULE_TYPE_TH))
//  static TemperatureHumidtySensorTask th_sensor_task("THTask", 400, OS_TASK_PRIORITY_03, thSensorParam);
#endif
//  static ElaborateDataTask elaborate_data_task("ElaborateDataTask", 400, OS_TASK_PRIORITY_02, elaborateDataParam);

#if (ENABLE_ACCELEROMETER)
//  static AccelerometerTask accelerometer_task("AccelerometerTask", 400, OS_TASK_PRIORITY_01, accelerometerParam);
#endif

#if (ENABLE_CAN)
//  static CanTask can_task("CanTask", 7200, OS_TASK_PRIORITY_02, canParam);
#endif

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

void init_rtc(bool init)
{
  // Create istance/init RTC object
  STM32RTC& rtc = STM32RTC::getInstance();

  // Select RTC clock source: LSE_CLOCK
  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin(); // initialize RTC 24H format

  // Set the time if requireq to Reset value
  if(init) {
    rtc.setHours(10);
    rtc.setMinutes(0);
    rtc.setSeconds(0);
    // Set the date
    rtc.setWeekDay(0);
    rtc.setDay(27);
    rtc.setMonth(12);
    rtc.setYear(22);
  }
  // Start LowPower configuration
  LowPower.begin();
}
