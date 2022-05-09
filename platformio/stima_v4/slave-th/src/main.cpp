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

#include "os_port_config.h"
#include <STM32FreeRTOS.h>
#include <Arduino.h>
#include "SdFat.h"
#include <os_port.h>

OsTaskId taskProvaId;
void TaskProva( void *pvParameters );

void TaskProva(void *pvParameters) {
   // (void) pvParameters;

   for (;;) {
      osDelayTask(1000);
      Serial.println("Task di prova");
   }
}

void setup() {
   Serial.begin(115200);
   osInitKernel();

   taskProvaId = osCreateTask("PROVA", TaskProva, NULL, 200, OS_TASK_PRIORITY_NORMAL);

   osStartKernel();
}

void loop() {
}
