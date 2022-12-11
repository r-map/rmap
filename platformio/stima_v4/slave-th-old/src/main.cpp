/**@file main.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
<http://www.gnu.org/licenses/>.
**********************************************************************/

#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"

void setup() {
  osInitKernel();
  init_debug(115200);
  init_wire();

  init_pins();
  init_tasks();
  load_configuration();
  init_sensors();

  // LedParam_t ledParam = {LED2_PIN, 100, 900};

  TemperatureHumidtySensorParam_t th_sensor_param;
  th_sensor_param.sensors_count = &configuration.sensors_count;
  th_sensor_param.sensors = configuration.sensors;
  th_sensor_param.acquisition_delay_ms = &configuration.sensor_acquisition_delay_ms;
  th_sensor_param.wireLock = wireLock;
  th_sensor_param.elaborataDataQueue = elaborataDataQueue;

  ElaboradeDataParam_t elaborate_data_param;
  elaborate_data_param.elaborataDataQueue = elaborataDataQueue;
  elaborate_data_param.requestDataQueue = requestDataQueue;
  elaborate_data_param.reportDataQueue = reportDataQueue;
  elaborate_data_param.acquisition_delay_ms = &configuration.sensor_acquisition_delay_ms;
  elaborate_data_param.observation_time_s = &configuration.observation_time_s;

  init_sdcard();
  init_registers();
  init_can();

  // Led Low Init Setup OK
  digitalWrite(LED2_PIN, LOW);

  CanParam_t can_param;
  can_param.requestDataQueue = requestDataQueue;
  can_param.reportDataQueue = reportDataQueue;

  // static LedTask led_task("LED 2 TASK", 100, OS_TASK_PRIORITY_01, ledParam);
  static TemperatureHumidtySensorTask th_sensor_task("TH SENSOR TASK", 800, OS_TASK_PRIORITY_04, th_sensor_param);
  static ElaborateDataSensorTask elaborate_data_task("ELAB DATA TASK", 1100, OS_TASK_PRIORITY_03, elaborate_data_param);
  static CanTask can_task("CAN TASK", 8192, OS_TASK_PRIORITY_02, can_param);

  Thread::StartScheduler();
}

void loop() {}

void print_configuration() {
  char stima_name[20];
  getStimaNameByType(stima_name, configuration.module_type);
  TRACE_INFO(F("--> type: %s\r\n"), stima_name);
  TRACE_INFO(F("--> main version: %u\r\n"), configuration.module_main_version);
  TRACE_INFO(F("--> minor version: %u\r\n"), configuration.module_minor_version);
  TRACE_INFO(F("--> acquisition delay: %u [ms]\r\n"), configuration.sensor_acquisition_delay_ms);

  TRACE_INFO(F("--> %u configured sensors\r\n"), configuration.sensors_count);
  for (uint8_t i=0; i<configuration.sensors_count; i++) {
    TRACE_INFO(F("--> %u: %s-%s 0x%02X [ %s ]\r\n"), i+1, SENSOR_DRIVER_I2C, configuration.sensors[i].type, configuration.sensors[i].i2c_address, configuration.sensors[i].is_redundant ? REDUNDANT_STRING : MAIN_STRING);
  }
}

void save_configuration(bool is_default) {
  if (is_default) {
    TRACE_INFO(F("Save default configuration... [ %s ]\r\n"), OK_STRING);
    configuration.module_main_version = MODULE_MAIN_VERSION;
    configuration.module_minor_version = MODULE_MINOR_VERSION;
    configuration.module_type = MODULE_TYPE;
    configuration.sensor_acquisition_delay_ms = SENSORS_ACQUISITION_DELAY_MS;
    configuration.sensors_count = 0;

    #if (USE_SENSOR_ADT)
    strcpy(configuration.sensors[configuration.sensors_count].type, SENSOR_TYPE_ADT);
    configuration.sensors[configuration.sensors_count].i2c_address = 0x28;
    configuration.sensors[configuration.sensors_count].is_redundant = false;
    configuration.sensors_count++;
    #endif

    #if (USE_SENSOR_HIH)
    strcpy(configuration.sensors[configuration.sensors_count].type, SENSOR_TYPE_HIH);
    configuration.sensors[configuration.sensors_count].i2c_address = 0x28;
    configuration.sensors[configuration.sensors_count].is_redundant = false;
    configuration.sensors_count++;
    #endif

    #if (USE_SENSOR_HYT)
    strcpy(configuration.sensors[configuration.sensors_count].type, SENSOR_TYPE_HYT);
    configuration.sensors[configuration.sensors_count].i2c_address = HYT_DEFAULT_ADDRESS;
    configuration.sensors[configuration.sensors_count].is_redundant = false;
    configuration.sensors_count++;
    #endif

    #if (USE_SENSOR_SHT)
    strcpy(configuration.sensors[configuration.sensors_count].type, SENSOR_TYPE_SHT);
    configuration.sensors[configuration.sensors_count].i2c_address = SHT_DEFAULT_ADDRESS;
    configuration.sensors[configuration.sensors_count].is_redundant = true;
    configuration.sensors_count++;
    #endif
  }
  else {
    TRACE_INFO(F("Save configuration... [ %s ]\r\n"), OK_STRING);
  }

  //! write configuration to eeprom
  // ee_write(&configuration, CONFIGURATION_EEPROM_ADDRESS, sizeof(configuration));

  print_configuration();
}

void load_configuration() {
  //! read configuration from eeprom
  // ee_read(&configuration, CONFIGURATION_EEPROM_ADDRESS, sizeof(configuration));

  // if (configuration.module_type != MODULE_TYPE || configuration.module_version != MODULE_VERSION || digitalRead(CONFIGURATION_RESET_PIN) == LOW) {
  if (true) {
    save_configuration(CONFIGURATION_DEFAULT);
  }
  else {
    TRACE_INFO(F("Load configuration... [ %s ]\r\n"), OK_STRING);
    print_configuration();
  }
}

void init_tasks() {
  #if (HARDWARE_I2C == ENABLE)
  wireLock = new BinarySemaphore(true);
  #endif

  elaborataDataQueue = new Queue(ELABORATE_DATA_QUEUE_LENGTH, sizeof(elaborate_data_t));
  requestDataQueue = new Queue(REQUEST_DATA_QUEUE_LENGTH, sizeof(request_data_t));
  reportDataQueue = new Queue(REPORT_DATA_QUEUE_LENGTH, sizeof(report_t));
}

void init_sensors () {
}

void init_wire() {
  #if (HARDWARE_I2C == ENABLE)
  Wire.setSCL(I2C1_SCL);
  Wire.setSDA(I2C1_SDA);
  Wire.begin();
  Wire.setClock(I2C1_BUS_CLOCK_HZ);
  #endif
}

void init_sdcard() {
  if (!setupSd(SPI2_MOSI, SPI2_MISO, SPI2_CLK, SDCARD_CS, SPI2_BUS_CLOCK_MHZ)) {
    TRACE_ERROR(F("Initialization SD card error"));
    LOCAL_ASSERT(false);
  }
  TRACE_INFO(F("Initialization SD card done"));
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

void init_pins() {
  // *****************************************************
  //            STARTUP LED E PIN DIAGNOSTICI
  // *****************************************************
  // Output mode for LED BLINK SW LOOP (High per Setup)
  // Input mode for test button
  pinMode(LED2_PIN, OUTPUT);
  pinMode(USER_BTN, INPUT);
  digitalWrite(LED2_PIN, HIGH);
}

void init_can() {
  // *****************************************************
  //            STARTUP CANBUS E CANARD SPEED
  // *****************************************************
  TRACE_INFO(F("Initializing CANBUS..., PCLK1 Clock Freq: %d\r\n"), HAL_RCC_GetPCLK1Freq());
  if (!CAN_HW_Init()) {
      TRACE_ERROR(F("Initialization CAN BUS error\r\n"));
      LOCAL_ASSERT(false);
  }
  TRACE_INFO(F("Initialization CAN BUS done\r\n"));
}

// *********************************************************************************************
//          Inizializzazione generale HW, canard, CAN_BUS, e dispositivi collegati
// *********************************************************************************************

// Setup HW (PIN, interface, filter, baud)
bool CAN_HW_Init(void) {
  TRACE_INFO(F("Initializing CANBUS..., PCLK1 Clock Freq: %d\r\n"), HAL_RCC_GetPCLK1Freq());

  // Definition CAN structure variable
  CAN_HandleTypeDef CAN_Handle;

  // Definition GPIO and CAN filter structure variables
  GPIO_InitTypeDef GPIO_InitStruct;
  CAN_FilterTypeDef CAN_FilterInitStruct;

  // GPIO Ports clock enable
  __HAL_RCC_GPIOA_CLK_ENABLE();

  // CAN1 clock enable
  __HAL_RCC_CAN1_CLK_ENABLE();

  #if defined(STM32L452xx)
  // Mapping GPIO for CAN
  /* Configure CAN pin: RX */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  /* Configure CAN pin: TX */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  #else
  #error "Warning untested processor variant"
  #endif

  // Setup CAN Istance Basic
  CAN_Handle.Instance = CAN1;
  CAN_Handle.Init.Mode = CAN_MODE_NORMAL;
  CAN_Handle.Init.TimeTriggeredMode = DISABLE;
  CAN_Handle.Init.AutoBusOff = DISABLE;
  CAN_Handle.Init.AutoWakeUp = DISABLE;
  CAN_Handle.Init.AutoRetransmission = DISABLE;
  CAN_Handle.Init.ReceiveFifoLocked = DISABLE;
  CAN_Handle.Init.TransmitFifoPriority = DISABLE;
  // Check error initialization CAN
  if (HAL_CAN_Init(&CAN_Handle) != HAL_OK) {
    TRACE_ERROR(F("Error initialization HW CAN base\r\n"));
    LOCAL_ASSERT(false);
    return false;
  }

  // CAN filter basic initialization
  CAN_FilterInitStruct.FilterIdHigh = 0x0000;
  CAN_FilterInitStruct.FilterIdLow = 0x0000;
  CAN_FilterInitStruct.FilterMaskIdHigh = 0x0000;
  CAN_FilterInitStruct.FilterMaskIdLow = 0x0000;
  CAN_FilterInitStruct.FilterFIFOAssignment = CAN_RX_FIFO0;
  CAN_FilterInitStruct.FilterBank = 0;
  CAN_FilterInitStruct.FilterMode = CAN_FILTERMODE_IDMASK;
  CAN_FilterInitStruct.FilterScale = CAN_FILTERSCALE_32BIT;
  CAN_FilterInitStruct.FilterActivation = ENABLE;

  // Check error initalization CAN filter
  if (HAL_CAN_ConfigFilter(&CAN_Handle, &CAN_FilterInitStruct) != HAL_OK) {
    TRACE_ERROR(F("Error initialization filter CAN base\r\n"));
    LOCAL_ASSERT(false);
    return false;
  }

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
    TRACE_ERROR(F("Error redefinition bxCANComputeTimings, try loading default...\r\n"));
    val.natural32.value.count       = 2;
    val.natural32.value.elements[0] = CAN_BIT_RATE;
    val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
    registerWrite("uavcan.can.bitrate", &val);
    result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
    if (!result) {
      TRACE_ERROR(F("Error initialization bxCANComputeTimings\r\n"));
      LOCAL_ASSERT(false);
      return false;
    }
  }
  // Attivazione bxCAN sulle interfacce richieste, velocità e modalità
  result = bxCANConfigure(0, timings, false);
  if (!result) {
    TRACE_ERROR(F("Error initialization bxCANConfigure\r\n"));
    LOCAL_ASSERT(false);
    return false;
  }
  // *******************     CANARD SETUP TIMINGS AND SPEED COMPLETE   *******************

  // Check error starting CAN
  if (HAL_CAN_Start(&CAN_Handle) != HAL_OK) {
    TRACE_ERROR(F("CAN startup ERROR!!!\r\n"));
    LOCAL_ASSERT(false);
    return false;
  }

  // Enable Interrupt RX Standard CallBack -> CAN1_RX0_IRQHandler
  if (HAL_CAN_ActivateNotification(&CAN_Handle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
    TRACE_ERROR(F("Error initialization interrupt CAN base\r\n"));
    LOCAL_ASSERT(false);
    return false;
  }
  // Setup Priority e CB CAN_IRQ_RX Enable
  HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);

  // Setup Completato
  return true;
}
