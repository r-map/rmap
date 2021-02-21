/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _VARIANT_ARDUINO_STM32_
#define _VARIANT_ARDUINO_STM32_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

  
// * = F103C8-CB    | DIGITAL |
//                  |---------|
#define PA0  A0  // | 0       |
#define PA1  A1  // | 1       |
#define PA2  A2  // | 2       |
#define PA3  A3  // | 2       |
#define PB7  A4  // | 4       |
#define PB6  A5  // | 5       |
#define PB0  A6  // | 6       |
#define PB1  A7  // | 7       |


#define  PA10 0  // Digital pin 0
#define  PA9  1  // Digital pin 1
#define  PB11 2  // Digital pin 2
#define  PB10 3  // Digital pin 3
#define  PA8  4  // Digital pin 4
#define  PA13 5  // Digital pin 5
#define  PA14 6  // Digital pin 6
#define  PA15 7  // Digital pin 7

  
#define PB3  8   // | 8       |
#define PB4  9   // | 9       |
#define PA4 10   // | 10      |
#define PA7 11   // | 11      |
#define PA6 12   // | 12      |
#define PA5 13   // | 13      |
  //#define PA0 14   // | 14      |
  //#define PA1 15   // | 15      |

  //#define PA2 16   // | 16      |
  //#define PA3 17   // | 17      |
  //#define PB7 18   // | 18      |
  //#define PB6 19   // | 19      |
  //#define PB0 20   // | 20      |
  //#define PB1 21   // | 21      |
  //                  |---------|

// This must be a literal
#define NUM_DIGITAL_PINS        22
#define NUM_ANALOG_INPUTS       8

// On-board LED pin number
#ifndef LED_BUILTIN
#define LED_BUILTIN             PA5
#endif
#define LED_GREEN               LED_BUILTIN

// SPI Definitions
#define PIN_SPI_SS              PA4
#define PIN_SPI_MOSI            PA7
#define PIN_SPI_MISO            PA6
#define PIN_SPI_SCK             PA5

// I2C Definitions
#define PIN_WIRE_SDA            PB7
#define PIN_WIRE_SCL            PB6

// Timer Definitions
// Use TIM6/TIM7 when possible as servo and tone don't need GPIO output pin
#define TIMER_TONE              TIM3
#define TIMER_SERVO             TIM2

// UART Definitions
#define SERIAL_UART_INSTANCE    1
// Default pin used for 'Serial' instance
// Mandatory for Firmata
#define PIN_SERIAL_RX           PA_10
#define PIN_SERIAL_TX           PA_9

#ifdef __cplusplus
} // extern "C"
#endif
/*----------------------------------------------------------------------------
 *        Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

#ifdef __cplusplus
  // These serial port names are intended to allow libraries and architecture-neutral
  // sketches to automatically default to the correct port name for a particular type
  // of use.  For example, a GPS module would normally connect to SERIAL_PORT_HARDWARE_OPEN,
  // the first hardware serial port whose RX/TX pins are not dedicated to another use.
  //
  // SERIAL_PORT_MONITOR        Port which normally prints to the Arduino Serial Monitor
  //
  // SERIAL_PORT_USBVIRTUAL     Port which is USB virtual serial
  //
  // SERIAL_PORT_LINUXBRIDGE    Port which connects to a Linux system via Bridge library
  //
  // SERIAL_PORT_HARDWARE       Hardware serial port, physical RX & TX pins.
  //
  // SERIAL_PORT_HARDWARE_OPEN  Hardware serial ports which are open for use.  Their RX & TX
  //                            pins are NOT connected to anything by default.
  #define SERIAL_PORT_MONITOR     Serial
  #define SERIAL_PORT_HARDWARE    Serial1
#endif

#endif /* _VARIANT_ARDUINO_STM32_ */
