// This software is distributed under the terms of the MIT License.
// Progetto RMAP - STIMA V4
// Hardware Config, STIMAV4 MASTER Board - Rev.1.00
// Copyright (C) 2022 Digiteco s.r.l.
// Author: Gasperini Moreno <m.gasperini@digiteco.it>

#define TRACE_LEVEL STIMA_TRACE_LEVEL

// Arduino
#include "module_master_hal.hpp"
#include "task_config.h"
#include <STM32FreeRTOS.h>
#include "thread.hpp"
// #include "semaphore.hpp"
// #include "queue.hpp"
// #include "stm32l4xx_hal.h"
// Private HW Configuration
// #include <Arduino.h>
// #include "eeprom.h
#include "tasks/prova_task.h"


// *********************************************************************************************
//                                       SETUP AMBIENTE
// *********************************************************************************************
void setup(void) {

  // STARTUP PRIVATE BASIC HARDWARE CONFIG AND ISTANCE
  SetupSystemPeripheral();

  // *****************************************************
  //  STARTUP SERIAL MONITOR DIAGNOSTICI E BOARD PINOUT
  // *****************************************************
  Serial.begin(115200);
  // Wait for serial port to connect
  while (!Serial) {
  }
  Serial.println(F("Start RS232 Monitor"));

  // End Config
  Serial.println(F("Initialization HW Base done"));

  // Startup Schedulher
  ProvaParam_t provaParam = {};
  static ProvaTask prova_task("PROVA TASK", 100, OS_TASK_PRIORITY_01, provaParam);
  cpp_freertos::Thread::StartScheduler();
}


// *************************************************************************************************
//                                          MAIN LOOP
// *************************************************************************************************

/* Get the rtc object */
//STM32RTC& rtc = STM32RTC::getInstance();

void loop(void) {
  // BinarySemaphore *wireLock;
  // wireLock = new BinarySemaphore(true);

  // uint8_t write_buffer[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  // uint8_t read_buffer[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  // EEprom eeprom = EEprom();

  // // eeprom.Write(0, write_buffer, 10);
  // delay(100);
  // eeprom.Read(0, read_buffer, 10);

  // Serial.println("Read");
  // for (uint8_t i=0; i<10; i++) {
  //   Serial.println(read_buffer[i]);
  // }
  // delay(5000);

  // RTC_TimeTypeDef sTime = {0};
  // RTC_DateTypeDef sDate = {0};
  // byte last_ss=100;
  //
  // LowPower.enableWakeupFrom(&rtc, cbf, &atime);
  //
  // rtc.setClockSource(STM32RTC::LSE_CLOCK);
  // rtc.begin(); // initialize RTC 24H format
  // rtc.setTime(10, 0, 0);
  // rtc.setDate(1, 17, 10, 22);
  //
  // Configure low power
  // LowPower.begin();
  // LowPower.enableWakeupFrom(&rtc, alarmMatch, &atime);
  //
  // // Configure first alarm in 2 second then it will be done in the rtc callback
  // rtc.setAlarmEpoch( rtc.getEpoch() + 2);
  //
  // do {
  //   if(HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD)!= HAL_OK) {
  //     delay(300);
  //     Serial.println(F("Error!!!,"));
  //   }
  //   if(sTime.Seconds!=last_ss) {
  //     last_ss=sTime.Seconds;
  //     Serial.print(F("Event second... BCD -> "));
  //     Serial.println(sTime.Seconds);
  //     //LowPower.deepSleep();
  //   }
  // } while (1);
}
