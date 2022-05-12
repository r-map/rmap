/**
* @file main.cpp
* @brief Main
*
* @section License
*
* SPDX-License-Identifier: GPL-2.0-or-later
*
* Copyright (C) 2022 Marco Baldinetti. All rights reserved.
*
* This file is part of CycloneTCP Open.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
* @author Marco Baldinetti <marco.baldinetti@alling.it>
* @version 0.0.1
**/

// #include "os_port_config.h"
#include <STM32FreeRTOS.h>
#include <Arduino.h>
#include "SdFat.h"
// #include <os_port.h>
#include "thread.hpp"
#include "ticks.hpp"

using namespace cpp_freertos;

#define LED1   PC7
#define LED2   PB7
#define LED3   PB14

class MyLed : public Thread {

public:
   MyLed(uint8_t i, uint8_t led, uint16_t onDelayMs, uint16_t offDelayMs) : Thread( 100, 1),
   Id (i),
   Led(led),
   OnDelayMs(onDelayMs),
   OffDelayMs(offDelayMs)
   {
      Start();
   };

protected:
   virtual void Run() {
      pinMode(Led, OUTPUT);
      while (true) {
         digitalWrite(Led, HIGH);
         Delay(Ticks::MsToTicks(OnDelayMs));
         digitalWrite(Led, LOW);
         Delay(Ticks::MsToTicks(OffDelayMs));
      }
   };

private:
   uint8_t Id;
   uint8_t Led;
   uint16_t OnDelayMs;
   uint16_t OffDelayMs;
};

void setup() {
   Serial.begin(115200);

   static MyLed led_1(1, LED1, 100, 900);
   static MyLed led_2(2, LED2, 200, 800);
   static MyLed led_3(3, LED3, 300, 700);

   Thread::StartScheduler();
}

void loop() {
}
