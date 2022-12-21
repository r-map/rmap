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

  static Queue *systemStatusQueue;
  static Queue *systemRequestQueue;
  static Queue *systemResponseQueue;
  
  static Queue *elaborataDataQueue;
  static Queue *requestDataQueue;
  static Queue *reportDataQueue;

  static BinarySemaphore *configurationLock;
  static BinarySemaphore *systemStatusLock;

  static configuration_t configuration = {0};
  static system_status_t system_status = {0};
  #if (ENABLE_ACCELEROMETER)
  static accelerometer_t config_accelerometer = {0};
  #endif

#if (ENABLE_I2C1)
  wireLock = new BinarySemaphore(true);
#endif
#if (ENABLE_I2C2)
  wire2Lock = new BinarySemaphore(true);
#endif
  configurationLock = new BinarySemaphore(true);
  systemStatusLock = new BinarySemaphore(true);

  systemRequestQueue = new Queue(SYSTEM_REQUEST_QUEUE_LENGTH, sizeof(system_request_t));
  systemResponseQueue = new Queue(SYSTEM_RESPONSE_QUEUE_LENGTH, sizeof(system_response_t));
  elaborataDataQueue = new Queue(ELABORATE_DATA_QUEUE_LENGTH, sizeof(elaborate_data_t));
  requestDataQueue = new Queue(REQUEST_DATA_QUEUE_LENGTH, sizeof(request_data_t));
  reportDataQueue = new Queue(REPORT_DATA_QUEUE_LENGTH, sizeof(report_t));

  // Initializing basic hardware's configuration
  SetupSystemPeripheral();
  init_debug(SERIAL_DEBUG_BAUD_RATE);
  init_wire();
  init_pins();
  init_sensors();
  // init_registers();

  // Alim Sens x I2C Test ( Force ON )
  digitalWrite(PIN_EN_5VS, 1);  // Enable + 5VS / +3V3S External Connector Power Sens
  digitalWrite(PIN_EN_SPLY, 1); // Enable Supply + 3V3_I2C / + 5V_I2C
  digitalWrite(PIN_I2C2_EN, 1); // I2C External Enable PIN (LevelShitf PCA9517D)
  
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

  ProvaParam_t provaParam = {};

#if (ENABLE_CAN)
  CanParam_t canParam;
  canParam.configuration = &configuration;
  canParam.system_status = &system_status;
  canParam.configurationLock = configurationLock;
  canParam.systemStatusLock = systemStatusLock;
  canParam.systemRequestQueue = systemRequestQueue;
  canParam.systemResponseQueue = systemResponseQueue;
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
  accelerometerParam.systemRequestQueue = systemRequestQueue;
  accelerometerParam.systemResponseQueue = systemResponseQueue;
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
  thSensorParam.systemRequestQueue = systemRequestQueue;
  thSensorParam.systemResponseQueue = systemResponseQueue;
  thSensorParam.elaborataDataQueue = elaborataDataQueue;
#endif

  ElaboradeDataParam_t elaborateDataParam;
  elaborateDataParam.configuration = &configuration;
  elaborateDataParam.system_status = &system_status;
  elaborateDataParam.configurationLock = configurationLock;
  elaborateDataParam.systemStatusLock = systemStatusLock;
  elaborateDataParam.systemRequestQueue = systemRequestQueue;
  elaborateDataParam.systemResponseQueue = systemResponseQueue;
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
  supervisorParam.systemRequestQueue = systemRequestQueue;
  supervisorParam.systemResponseQueue = systemResponseQueue;

  // ********************************************************
  //                     Startup Task
  // ********************************************************
  static ProvaTask prova_task("ProvaTask", 100, OS_TASK_PRIORITY_01, provaParam);
  static SupervisorTask supervisor_task("SupervisorTask", 100, OS_TASK_PRIORITY_04, supervisorParam);

#if ((MODULE_TYPE == STIMA_MODULE_TYPE_THR) || (MODULE_TYPE == STIMA_MODULE_TYPE_TH))
  static TemperatureHumidtySensorTask th_sensor_task("THTask", 400, OS_TASK_PRIORITY_03, thSensorParam);
#endif
  static ElaborateDataTask elaborate_data_task("ElaborateDataTask", 400, OS_TASK_PRIORITY_02, elaborateDataParam);

#if (ENABLE_ACCELEROMETER)
  static AccelerometerTask accelerometer_task("AccelerometerTask", 400, OS_TASK_PRIORITY_01, accelerometerParam);
#endif

#if (ENABLE_CAN)
  static CanTask can_task("CanTask", 7200, OS_TASK_PRIORITY_02, canParam);
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
