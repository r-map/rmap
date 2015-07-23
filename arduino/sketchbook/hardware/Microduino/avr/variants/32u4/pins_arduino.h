//=======Microduino

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <avr/pgmspace.h>

// Workaround for wrong definitions in "iom32u4.h".
// This should be fixed in the AVR toolchain.
#undef UHCON
#undef UHINT
#undef UHIEN
#undef UHADDR
#undef UHFNUM
#undef UHFNUML
#undef UHFNUMH
#undef UHFLEN
#undef UPINRQX
#undef UPINTX
#undef UPNUM
#undef UPRST
#undef UPCONX
#undef UPCFG0X
#undef UPCFG1X
#undef UPSTAX
#undef UPCFG2X
#undef UPIENX
#undef UPDATX
#undef TCCR2A
#undef WGM20
#undef WGM21
#undef COM2B0
#undef COM2B1
#undef COM2A0
#undef COM2A1
#undef TCCR2B
#undef CS20
#undef CS21
#undef CS22
#undef WGM22
#undef FOC2B
#undef FOC2A
#undef TCNT2
#undef TCNT2_0
#undef TCNT2_1
#undef TCNT2_2
#undef TCNT2_3
#undef TCNT2_4
#undef TCNT2_5
#undef TCNT2_6
#undef TCNT2_7
#undef OCR2A
#undef OCR2_0
#undef OCR2_1
#undef OCR2_2
#undef OCR2_3
#undef OCR2_4
#undef OCR2_5
#undef OCR2_6
#undef OCR2_7
#undef OCR2B
#undef OCR2_0
#undef OCR2_1
#undef OCR2_2
#undef OCR2_3
#undef OCR2_4
#undef OCR2_5
#undef OCR2_6
#undef OCR2_7

#define NUM_DIGITAL_PINS  22
#define NUM_ANALOG_INPUTS 10

#define TX_RX_LED_INIT	DDRD |= (1<<5), DDRB |= (1<<0)
#define TXLED0			PORTD |= (1<<5)
#define TXLED1			PORTD &= ~(1<<5)
#define RXLED0			PORTB |= (1<<0)
#define RXLED1			PORTB &= ~(1<<0)

static const uint8_t SDA = 18;
static const uint8_t SCL = 19;

static const uint8_t SS   = 10;
static const uint8_t MOSI = 11;
static const uint8_t MISO = 12;
static const uint8_t SCK  = 13;

// Mapping of analog pins as digital I/O
static const uint8_t A0 = 14;
static const uint8_t A1 = 15;
static const uint8_t A2 = 16;
static const uint8_t A3 = 17;
static const uint8_t A6 = 20;
static const uint8_t A7 = 21;

static const uint8_t A8 = 8;
static const uint8_t A9 = 9;
static const uint8_t A10 = 3;
static const uint8_t A11 = 4;


#define digitalPinToPCICR(p)    ((((p) >= 8 && (p) <= 13)) ? (&PCICR) : ((uint8_t *)0))
#define digitalPinToPCICRbit(p) 0
#define digitalPinToPCMSK(p)    ((((p) >= 8 && (p) <= 13)) ? (&PCMSK0) : ((uint8_t *)0))

#define digitalPinToPCMSKbit(p) ( \
    ((p)== 0) ? 2 : \
    ((p)== 1) ? 3 : \
    ((p)== 2) ? 6 : \
    ((p)== 3) ? 6 : \
    ((p)== 4) ? 7 : \
    ((p)== 5) ? 6 : \
    ((p)== 6) ? 7 : \
    ((p)== 7) ? 7 : \
    ((p)== 8) ? 6 : \
    ((p)== 9) ? 5 : \
    ((p)==10) ? 0 : \
    ((p)==11) ? 2 : \
    ((p)==12) ? 3 : \
    ((p)==13) ? 1 : \
    ((p)==14) ? 7 : \
    ((p)==15) ? 6 : \
    ((p)==16) ? 5 : \
    ((p)==17) ? 4 : \
    ((p)==18) ? 1 : \
    ((p)==19) ? 0 : \
    ((p)==20) ? 1 : \
    ((p)==21) ? 0 : \
    0 \
)


//	__AVR_ATmega32U4__ has an unusual mapping of pins to channels
extern const uint8_t PROGMEM analog_pin_to_channel_PGM[];
#define analogPinToChannel(p)     (((p) == 14) ? 7 : \
                                 ((p) == 15) ? 6 : \
                                 ((p) == 16) ? 5 : \
                                 ((p) == 17) ? 4 : \
                                 ((p) == 20) ? 1 : \
                                 ((p) == 21) ? 0 : \
                                 ((p) == 8) ? 13 : \
                                 ((p) == 9) ? 12 : \
                                 ((p) == 3) ? 9 : \
                                 ((p) == 4) ? 10 : \
                                 -1)                         
                                 
#ifdef ARDUINO_MAIN


const uint16_t PROGMEM port_to_mode_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &DDRB,
	(uint16_t) &DDRC,
	(uint16_t) &DDRD,
	(uint16_t) &DDRE,
	(uint16_t) &DDRF,
};

const uint16_t PROGMEM port_to_output_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &PORTB,
	(uint16_t) &PORTC,
	(uint16_t) &PORTD,
	(uint16_t) &PORTE,
	(uint16_t) &PORTF,
};

const uint16_t PROGMEM port_to_input_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &PINB,
	(uint16_t) &PINC,
	(uint16_t) &PIND,
	(uint16_t) &PINE,
	(uint16_t) &PINF,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
	PD, // D0 - PD2
	PD,	// D1 - PD3
	PE, // D2 - PE6
	PD,	// D3 - PD6
	PD,	// D4 - PD7
	PC, // D5 - PC6
	PC, // D6 - PC7
	PB, // D7 - PB7
	
	PB, // D8 - PB6
	PB,	// D9 - PB5
	PB, // D10 - PB0
	PB,	// D11 - MOSI - PB2
	PB, // D12 -MISO -  PB3
	PB, // D13 -SCK -  PB1

	PF,	// D14 - A0 - PF7
	PF, // D15 - A1 - PF6
	PF, // D16 - A2 - PF5
	PF, // D17 - A3 - PF4

	PD, // D18 - PD1
	PD, // D19 - PD0

	PF, // D20 - A6 - PF1
	PF, // D21 - A7 - PF0
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
	_BV(2), // D0 - PD2
	_BV(3),	// D1 - PD3
	_BV(6), // D2 - PE6
	_BV(6),	// D3 - PD6
	_BV(7),	// D4 - PD7
	_BV(6), // D5 - PC6
	_BV(7), // D6 - PC7
	_BV(7), // D7 - PE7

	_BV(6), // D8 - PB6
	_BV(5),	// D9 - PB5
	_BV(0), // D10 - PB0
	_BV(2),	// D11 - MOSI - PB2
	_BV(3), // D12 -MISO -  PB3
	_BV(1), // D13 -SCK -  PB1
			
	_BV(7),	// D14 - A0 - PF7
	_BV(6), // D15 - A1 - PF6
	_BV(5), // D16 - A2 - PF5
	_BV(4), // D17 - A3 - PF4
	
	_BV(1), // D18 - PD1
	_BV(0), // D19 - PD0
	
	_BV(1), // D20 - A6 - PF1
	_BV(0), // D21 - A7 - PF0
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
	NOT_ON_TIMER,	
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	TIMER4D,		/* 4 */
	TIMER3A,		/* 5 */
	TIMER4A,		/* 6 */
	TIMER0A,		/* 7 */
	
	TIMER1B,		/* 8 */
	TIMER1A,		/* 9 */
	NOT_ON_TIMER,
	NOT_ON_TIMER,	
	NOT_ON_TIMER,	
	NOT_ON_TIMER,

	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,

	NOT_ON_TIMER,
	TIMER0B,		/* 19 */

	NOT_ON_TIMER,
	NOT_ON_TIMER,
};


#endif /* ARDUINO_MAIN */

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
#define SERIAL_PORT_MONITOR        Serial
#define SERIAL_PORT_USBVIRTUAL     Serial
#define SERIAL_PORT_HARDWARE       Serial1
#define SERIAL_PORT_HARDWARE_OPEN  Serial1

#endif /* Pins_Arduino_h */
