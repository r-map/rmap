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

  // STARTUP PRIVATE BASIC HARDWARE CONFIG AND ISTANCE
  SetupSystemPeripheral();

  init_debug(115200);

  // init_wire();

  init_pins();
  init_tasks();
  // init_sensors();
  // init_sdcard();
  // init_registers();
  // init_can();

  error_t error = NO_ERROR;

  // Initialize hardware cryptographic accelerator
  error = stm32l4xxCryptoInit();
  // Any error to report?
  if (error) {
    // Debug message
    TRACE_ERROR("Failed to initialize hardware crypto accelerator!\r\n");
  }

  // TCP/IP stack initialization
  error = netInit();
  if (error) {
    TRACE_ERROR("Failed to initialize TCP/IP stack!\r\n");
  }

  TRACE_INFO(F("Initialization HW Base done\r\n"));

  SupervisorParam_t supervisorParam;
  supervisorParam.configuration = &configuration;
  supervisorParam.wireLock = wireLock;
  supervisorParam.configurationLock = configurationLock;
  
  // LedParam_t ledParam1 = {LED1_PIN, 100, 900};
  // LedParam_t ledParam2 = {LED2_PIN, 200, 800};
  // LedParam_t ledParam3 = {LED3_PIN, 300, 700};
  // EthernetParam_t ethernetParam;
  // MqttParam_t mqttParam;

  // ethernetParam.interface = &netInterface[0];
  // ethernetParam.tickHandlerMs = APP_ETHERNET_TICK_EVENT_HANDLER_MS;
  //
  // mqttParam.timeoutMs = APP_MQTT_TIMEOUT_MS;
  // mqttParam.attemptDelayMs = APP_MQTT_ATTEMPT_DELAY_MS;
  // strncpy(mqttParam.server, APP_MQTT_SERVER_NAME, APP_MQTT_SERVER_NAME_LENGTH);
  // mqttParam.port = APP_MQTT_SERVER_PORT;
  // mqttParam.version = MQTT_VERSION_3_1_1;
  // mqttParam.transportProtocol = MQTT_TRANSPORT_PROTOCOL_TLS;
  // mqttParam.qos = MQTT_QOS_LEVEL_1;
  // mqttParam.keepAliveS = APP_MQTT_KEEP_ALIVE_S;
  // memset(mqttParam.clientIdentifier, 0, APP_MQTT_CLIENT_IDENTIFIER_LENGTH);
  // memset(mqttParam.username, 0, APP_MQTT_USERNAME_LENGTH);
  // memset(mqttParam.password, 0, APP_MQTT_USERNAME_LENGTH);
  // // strncpy(mqttParam.username, "username", APP_MQTT_USERNAME_LENGTH);
  // // strncpy(mqttParam.password, "password", APP_MQTT_PASSWORD_LENGTH);
  // memset(mqttParam.willTopic, 0, APP_MQTT_WILL_TOPIC_LENGTH);
  // // strncpy(mqttParam.willTopic, "board/status", APP_MQTT_WILL_TOPIC_LENGTH);
  // memset(mqttParam.willMsg, 0, APP_MQTT_WILL_MSG_LENGTH);
  // // strncpy(mqttParam.willMsg, "offline", APP_MQTT_WILL_MSG_LENGTH);
  // mqttParam.isWillMsgRetain = false;
  // mqttParam.isCleanSession = true;
  // mqttParam.isPublishRetain = false;
  // // mqttParam.clientPsk = (uint8_t *)clientPsk;
  // // mqttParam.cipherSuites = (uint16_t *)cipherSuites;

  // static LedTask led_1_task("LED 1 TASK", 100, OS_TASK_PRIORITY_01, ledParam1);
  // static LedTask led_2_task("LED 2 TASK", 100, OS_TASK_PRIORITY_01, ledParam2);
  // static LedTask led_3_task("LED 3 TASK", 100, OS_TASK_PRIORITY_01, ledParam3);
  // static EthernetTask eth_task("ETH TASK", 100, OS_TASK_PRIORITY_03, ethernetParam);
  // static MqttTask mqtt_task("MQTT TASK", 1024, OS_TASK_PRIORITY_02, mqttParam);

  static SupervisorTask supervisor_task("SUPERVISOR TASK", 100, OS_TASK_PRIORITY_01, supervisorParam);
  Thread::StartScheduler();
}


/// @brief Idle Task
void loop() {
}

void init_tasks() {
  #if (HARDWARE_I2C == ENABLE)
  wireLock = new BinarySemaphore(true);
  #endif

  configurationLock = new BinarySemaphore(true);
}

void init_sensors () {
}

void init_wire() {
  #if (HARDWARE_I2C == ENABLE)
  // Wire.setSCL(I2C1_SCL);
  // Wire.setSDA(I2C1_SDA);
  // Wire.begin();
  // Wire.setClock(I2C1_BUS_CLOCK_HZ);
  #endif
}

void init_sdcard() {
  // if (!setupSd(SPI2_MOSI, SPI2_MISO, SPI2_CLK, SDCARD_CS, SPI2_BUS_CLOCK_MHZ)) {
  //   TRACE_ERROR(F("Initialization SD card error"));
  //   LOCAL_ASSERT(false);
  // }
  // TRACE_INFO(F("Initialization SD card done"));
}

void init_registers() {
  // // ********************************************************************************
  // //    FIXED REGISTER_INIT, FARE INIT OPZIONALE x REGISTRI FISSI ECC. E/O INVAR.
  // // ********************************************************************************
  // #ifdef INIT_REGISTER
  // // Inizializzazione fissa dei registri nella modalità utilizzata (prepare SD/FLASH/ROM con default Value)
  // registerSetup(true);
  // #else
  // // Default dei registri nella modalità utilizzata (prepare SD/FLASH/ROM con default Value)
  // // Creazione dei registri standard base se non esistono
  // registerSetup(false);
  // #endif
}

void init_pins() {
  // *****************************************************
  //            STARTUP LED E PIN DIAGNOSTICI
  // *****************************************************
  // Output mode for LED BLINK SW LOOP (High per Setup)
  // Input mode for test button
  pinMode(LED2_PIN, OUTPUT);
  pinMode(USER_BTN, INPUT);

  // Led Low Init Setup OK
  digitalWrite(LED2_PIN, LOW);
}

void init_can() {
  // // *****************************************************
  // //            STARTUP CANBUS E CANARD SPEED
  // // *****************************************************
  // TRACE_INFO(F("Initializing CANBUS..., PCLK1 Clock Freq: %d\r\n"), HAL_RCC_GetPCLK1Freq());
  // if (!CAN_HW_Init()) {
  //     TRACE_ERROR(F("Initialization CAN BUS error\r\n"));
  //     LOCAL_ASSERT(false);
  // }
  // TRACE_INFO(F("Initialization CAN BUS done\r\n"));
}

// *********************************************************************************************
//          Inizializzazione generale HW, canard, CAN_BUS, e dispositivi collegati
// *********************************************************************************************

// Setup HW (PIN, interface, filter, baud)
bool CAN_HW_Init(void) {
  // TRACE_INFO(F("Initializing CANBUS..., PCLK1 Clock Freq: %d\r\n"), HAL_RCC_GetPCLK1Freq());
  //
  // // Definition CAN structure variable
  // CAN_HandleTypeDef CAN_Handle;
  //
  // // Definition GPIO and CAN filter structure variables
  // GPIO_InitTypeDef GPIO_InitStruct;
  // CAN_FilterTypeDef CAN_FilterInitStruct;
  //
  // // GPIO Ports clock enable
  // __HAL_RCC_GPIOA_CLK_ENABLE();
  //
  // // CAN1 clock enable
  // __HAL_RCC_CAN1_CLK_ENABLE();
  //
  // #if defined(STM32L452xx)
  // // Mapping GPIO for CAN
  // /* Configure CAN pin: RX */
  // GPIO_InitStruct.Pin = GPIO_PIN_11;
  // GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  // GPIO_InitStruct.Pull = GPIO_NOPULL;
  // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  // GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
  // HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  // /* Configure CAN pin: TX */
  // GPIO_InitStruct.Pin = GPIO_PIN_12;
  // GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  // GPIO_InitStruct.Pull = GPIO_PULLUP;
  // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  // GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
  // HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  // #else
  // #error "Warning untested processor variant"
  // #endif
  //
  // // Setup CAN Istance Basic
  // CAN_Handle.Instance = CAN1;
  // CAN_Handle.Init.Mode = CAN_MODE_NORMAL;
  // CAN_Handle.Init.TimeTriggeredMode = DISABLE;
  // CAN_Handle.Init.AutoBusOff = DISABLE;
  // CAN_Handle.Init.AutoWakeUp = DISABLE;
  // CAN_Handle.Init.AutoRetransmission = DISABLE;
  // CAN_Handle.Init.ReceiveFifoLocked = DISABLE;
  // CAN_Handle.Init.TransmitFifoPriority = DISABLE;
  // // Check error initialization CAN
  // if (HAL_CAN_Init(&CAN_Handle) != HAL_OK) {
  //   TRACE_ERROR(F("Error initialization HW CAN base\r\n"));
  //   LOCAL_ASSERT(false);
  //   return false;
  // }
  //
  // // CAN filter basic initialization
  // CAN_FilterInitStruct.FilterIdHigh = 0x0000;
  // CAN_FilterInitStruct.FilterIdLow = 0x0000;
  // CAN_FilterInitStruct.FilterMaskIdHigh = 0x0000;
  // CAN_FilterInitStruct.FilterMaskIdLow = 0x0000;
  // CAN_FilterInitStruct.FilterFIFOAssignment = CAN_RX_FIFO0;
  // CAN_FilterInitStruct.FilterBank = 0;
  // CAN_FilterInitStruct.FilterMode = CAN_FILTERMODE_IDMASK;
  // CAN_FilterInitStruct.FilterScale = CAN_FILTERSCALE_32BIT;
  // CAN_FilterInitStruct.FilterActivation = ENABLE;
  //
  // // Check error initalization CAN filter
  // if (HAL_CAN_ConfigFilter(&CAN_Handle, &CAN_FilterInitStruct) != HAL_OK) {
  //   TRACE_ERROR(F("Error initialization filter CAN base\r\n"));
  //   LOCAL_ASSERT(false);
  //   return false;
  // }
  //
  // // *******************         CANARD SETUP TIMINGS AND SPEED        *******************
  // // CAN BITRATE Dinamico su LoadRegister (CAN_FD 2xREG natural32 0=Speed, 1=0 (Not Used))
  // uavcan_register_Value_1_0 val = {0};
  // uavcan_register_Value_1_0_select_natural32_(&val);
  // val.natural32.value.count       = 2;
  // val.natural32.value.elements[0] = CAN_BIT_RATE;
  // val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
  // registerRead("uavcan.can.bitrate", &val);
  // LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural32_(&val) && (val.natural32.value.count == 2));
  //
  // // Dynamic BIT RATE Change CAN Speed to CAN_BIT_RATE (register default/defined)
  // BxCANTimings timings;
  // bool result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
  // if (!result) {
  //   TRACE_ERROR(F("Error redefinition bxCANComputeTimings, try loading default...\r\n"));
  //   val.natural32.value.count       = 2;
  //   val.natural32.value.elements[0] = CAN_BIT_RATE;
  //   val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
  //   registerWrite("uavcan.can.bitrate", &val);
  //   result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
  //   if (!result) {
  //     TRACE_ERROR(F("Error initialization bxCANComputeTimings\r\n"));
  //     LOCAL_ASSERT(false);
  //     return false;
  //   }
  // }
  // // Attivazione bxCAN sulle interfacce richieste, velocità e modalità
  // result = bxCANConfigure(0, timings, false);
  // if (!result) {
  //   TRACE_ERROR(F("Error initialization bxCANConfigure\r\n"));
  //   LOCAL_ASSERT(false);
  //   return false;
  // }
  // // *******************     CANARD SETUP TIMINGS AND SPEED COMPLETE   *******************
  //
  // // Check error starting CAN
  // if (HAL_CAN_Start(&CAN_Handle) != HAL_OK) {
  //   TRACE_ERROR(F("CAN startup ERROR!!!\r\n"));
  //   LOCAL_ASSERT(false);
  //   return false;
  // }
  //
  // // Enable Interrupt RX Standard CallBack -> CAN1_RX0_IRQHandler
  // if (HAL_CAN_ActivateNotification(&CAN_Handle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
  //   TRACE_ERROR(F("Error initialization interrupt CAN base\r\n"));
  //   LOCAL_ASSERT(false);
  //   return false;
  // }
  // // Setup Priority e CB CAN_IRQ_RX Enable
  // HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
  // HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
  //
  // // Setup Completato
  // return true;
}
