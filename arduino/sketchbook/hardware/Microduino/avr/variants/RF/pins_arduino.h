
/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: wiring.h 249 2007-02-03 16:52:51Z mellis $
*/

/*
	This version of pins_arduino.h is for the Zigduino r1
	Pierce Nichols 2011 Oct 11
*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <avr/pgmspace.h>

#define NUM_DIGITAL_PINS            22
#define NUM_ANALOG_INPUTS           6
#define analogInputToDigitalPin(p) (((p) == 7) ? 14 : \
                                 ((p) == 6) ? 15 : \
                                 ((p) == 5) ? 16 : \
                                 ((p) == 4) ? 17 : \
                                 ((p) == 3) ? 20 : \
                                 ((p) == 2) ? 21 : \
                                 -1)
                                 
                                 
#define analogPinToChannel(p)     (((p) == 14) ? 7 : \
                                 ((p) == 15) ? 6 : \
                                 ((p) == 16) ? 5 : \
                                 ((p) == 17) ? 4 : \
                                 ((p) == 20) ? 3 : \
                                 ((p) == 21) ? 2 : \
                                 -1)                            
                                
#define digitalPinHasPWM(p)         ((p) == 4 ||(p) == 5 ||(p) == 6 ||(p) == 7 ||(p) == 8 ||(p) == 9 ||(p) == 10)

const static uint8_t SCK  = 13;
const static uint8_t MISO = 12;
const static uint8_t MOSI = 11;
const static uint8_t SS = 10;

const static uint8_t SDA = 18;
const static uint8_t SCL = 19;
const static uint8_t LED = 13;

const static uint8_t A0 = 14;
const static uint8_t A1 = 15;
const static uint8_t A2 = 16;
const static uint8_t A3 = 17;

const static uint8_t A6 = 20;
const static uint8_t A7 = 21;

//const static uint8_t RFTX = 23;
//const static uint8_t RFRX = 24;

//const static uint8_t BATMON = 35;

// A majority of the pins are NOT PCINTs, SO BE WARNED (i.e. you cannot use them as receive pins)
// Only pins available for RECEIVE (TRANSMIT can be on any pin):
// Pins: 0, 7, 8, 9, 10, 11, 12, 13

#define digitalPinToPCICR(p)    ( ((((p) >= 7) && ((p) <= 13)) || \
                                  ((p) == 0) ) ? \
                                  (&PCICR) : ((uint8_t *)0) )

#define digitalPinToPCICRbit(p) ( ((p) == 0) ? 1 : 0 ) 

#define digitalPinToPCMSK(p)    ( ((((p) >= 7) && ((p) <= 13))) ? (&PCMSK0) : \
                                ( ((p) == 0) ? (&PCMSK1) : \
                                ((uint8_t *)0) ) )

#define digitalPinToPCMSKbit(p) ( ((p) == 13) ? 1 : \
                                ( ((p) == 11) ? 2 : \
                                ( ((p) == 12) ? 3 : \
                                ( ((p) == 10) ? 4 : \
                                ( ((p) == 9) ? 5 : \
                                ( ((p) == 8) ? 6 : \
                                ( ((p) == 7) ? 7 : \
                                ( ((p) == 0) ? 0 : \
                                0 ) ) ) ) ) ) )

#ifdef ARDUINO_MAIN

const uint16_t PROGMEM port_to_mode_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t)&DDRB,
	NOT_A_PORT,
	(uint16_t)&DDRD,
	(uint16_t)&DDRE,
	(uint16_t)&DDRF,
	(uint16_t)&DDRG,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
};

const uint16_t PROGMEM port_to_output_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t)&PORTB,
	NOT_A_PORT,
	(uint16_t)&PORTD,
	(uint16_t)&PORTE,
	(uint16_t)&PORTF,
	(uint16_t)&PORTG,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
};

const uint16_t PROGMEM port_to_input_PGM[] = {
	NOT_A_PIN,
	NOT_A_PIN,
	(uint16_t)&PINB,
	NOT_A_PIN,
	(uint16_t)&PIND,
	(uint16_t)&PINE,
	(uint16_t)&PINF,
	(uint16_t)&PING,
	NOT_A_PIN,
	NOT_A_PIN,
	NOT_A_PIN,
	NOT_A_PIN,
	NOT_A_PIN,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
	// PORTLIST		
	// -------------------------------------------		
	PE	, // PE 0 ** 0 ** USART0_RX	
	PE	, // PE 1 ** 1 ** USART0_TX	
	PD	, // PD 2 ** 2 ** USART1_RX	
	PD	, // PD 3 ** 3** USART1_TX	

	PE	, // PE 3 ** 4 ** PWM
	PE	, // PE 4 ** 5 ** PWM
	PE	, // PE 5 ** 6 ** PWM
	PB	, // PB 7 ** 7 **PWM

	PB	, // PB 6 ** 8 ** PWM
	PB	, // PB 5 ** 9 ** PWM
	PB	, // PB 4 ** 10 ** PWM

	PB	, // PB 2 ** 11 ** SPI_MOSI
	PB	, // PB 3 ** 12 ** SPI_MISO
	PB	, // PB 1 ** 13 ** SPI_SCK


	PF	, // PF 7 ** 14 ** A0
	PF	, // PF 6 ** 15 ** A1	
	PF	, // PF 5 ** 16 ** A2
	PF	, // PF 4 ** 17 ** A3	

	PD	, // PD 1 ** 18 ** I2C_SDA
	PD	, // PD 0 ** 19 ** I2C_SCL

	PF	, // PF 3 ** 20 ** A6
	PF	, // PF 2 ** 21 ** A7	
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
	// PIN IN PORT		
	// -------------------------------------------		
	_BV( 0 )	, 
	_BV( 1 )	, 
	_BV( 2 )	, 
	_BV( 3 )	, 

	_BV( 3 )	, 
	_BV( 4 )	, 
	_BV( 5 )	, 
	_BV( 7 )	, 
	
	_BV( 6 )	, 
	_BV( 5 )	, 
	_BV( 4 )	,
	 
	_BV( 2 )	, 
	_BV( 3 )	, 
	_BV( 1 )	, 
	
	_BV( 7 )	, 
	_BV( 6 )	, 
	_BV( 5 )	, 
	_BV( 4 )	, 
	
	_BV( 1 )	, 
	_BV( 0 )	, 
	
	_BV( 3 )	, 
	_BV( 2 )	, 
	
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
	// TIMERS		
	// -------------------------------------------	
	NOT_ON_TIMER	, // PE 0 ** 0 ** USART0_RX	
	NOT_ON_TIMER	, // PE 1 ** 1 ** USART0_TX	
	NOT_ON_TIMER	, // PD 2 ** 2 ** USART1_RX	
	NOT_ON_TIMER	, // PD 3 ** 3** USART1_TX	

	TIMER3A	, // PE 2 ** 4 ** PWM
	TIMER3B	, // PE 3 ** 5 ** PWM
	TIMER3C	, // PE 4 ** 6 ** PWM
	TIMER1C	, // PE 7 ** 7 ** PWM

	TIMER1B	, // PB 6 ** 8 ** PWM
	TIMER1A	, // PB 5 ** 9 ** PWM
	TIMER2A	, // PB 4 ** 10 ** PWM

	NOT_ON_TIMER	, // PB 2 ** 11 ** SPI_MOSI
	NOT_ON_TIMER	, // PB 3 ** 12 ** SPI_MISO
	NOT_ON_TIMER	, // PB 1 ** 13 ** SPI_SCK


	NOT_ON_TIMER	, // PF 7 ** 14 ** A0
	NOT_ON_TIMER	, // PF 6 ** 15 ** A1	
	NOT_ON_TIMER	, // PF 5 ** 16 ** A2
	NOT_ON_TIMER	, // PF 4 ** 17 ** A3	

	NOT_ON_TIMER	, // PD 1 ** 18 ** I2C_SDA
	NOT_ON_TIMER	, // PD 0** 19 ** I2C_SCL

	NOT_ON_TIMER	, // PF 3 ** 20 ** A6
	NOT_ON_TIMER	, // PF 2 ** 21 ** A7	
};	

#endif
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
#define SERIAL_PORT_MONITOR         Serial
#define SERIAL_PORT_HARDWARE        Serial
#define SERIAL_PORT_HARDWARE1       Serial1
#define SERIAL_PORT_HARDWARE_OPEN   Serial1

#endif