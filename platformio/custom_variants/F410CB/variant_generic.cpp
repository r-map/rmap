/*
 *******************************************************************************
 * Copyright (c) 2021, STMicroelectronics
 * All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */

#if defined(ARDUINO_GENERIC_F410CBTX)

#include "pins_arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

// Digital PinName array
const PinName digitalPin[] = {
  PA_10,  // Digital pin 0
  PA_9,   // Digital pin 1
  PB_11,  // Digital pin 2
  PB_10,  // Digital pin 3
  PA_8,   // Digital pin 4
  PA_13,  // Digital pin 5
  PA_14,  // Digital pin 6
  PA_15,  // Digital pin 7
  PB_3,   // Digital pin 8
  PB_4,   // Digital pin 9
  PA_4,   // Digital pin 10
  PA_7,   // Digital pin 11
  PA_6,   // Digital pin 12
  PA_5,   // Digital pin 13
  PA_0,   // Digital pin 14
  PA_1,   // Digital pin 15
  PA_2,   // Digital pin 16
  PA_3,   // Digital pin 17
  PB_7,   // Digital pin 18
  PB_6,   // Digital pin 19
  PB_0,   // Digital pin 20
  PB_1    // Digital pin 21
};

// Analog (Ax) pin number array
const uint32_t analogInputPin[] = {
  14,  // A0,  PA0
  15,  // A1,  PA1
  16,  // A2,  PA2
  17,  // A3,  PA3
  18,  // A4,  PB7
  19,  // A5,  PB6
  20,  // A6,  PB0
  21,  // A7,  PB1
};

#ifdef __cplusplus
}
#endif
#endif

