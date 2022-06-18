/**
* @file extIntDriverArduino.c
* @brief External interrupt line driver
*
* @section License
*
* Copyright (C) 2010-2015 Oryx Embedded SARL. All rights reserved.
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
* @author Oryx Embedded SARL (www.oryx-embedded.com)
* @version 1.6.5
**/

#include "mydebug.h"
#include "arduino_interrupt_driver.h"

/**
* @brief External interrupt line driver
**/
const ExtIntDriver extIntDriver = {
  extIntInit,
  extIntEnableIrq,
  extIntDisableIrq
};

/**
* @brief EXTI configuration
* @return Error code
**/
error_t extIntInit(void) {
  // Serial.println("init int");
  pinMode(ENC28J60_INT_PIN, INPUT);
  return NO_ERROR;
}

/**
* @brief Enable external interrupts
**/
void extIntEnableIrq(void) {
  // Serial.println("init enable");
  // attachInterrupt(digitalPinToInterrupt(ENC28J60_INT_PIN), intHandler, LOW);
}

/**
* @brief Disable external interrupts
**/
void extIntDisableIrq(void) {
  // Serial.println("init disable");
  // detachInterrupt(digitalPinToInterrupt(ENC28J60_INT_PIN));
}

/**
* @brief External interrupt handler
**/
void intHandler(void) {
  // if (digitalRead(ENC28J60_INT_PIN) == LOW) {
    // NetInterface *interface;
    // interface = &netInterface[0];
    // enc28j60IrqHandler(interface);
  // }
}
