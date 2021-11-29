// vi:ts=4
// ----------------------------------------------------------------------------
// LineWrap - simple demonstration of automatic linewrap functionality
// Created by Bill Perry 2020-06-26
// bperrybap@opensource.billsworld.billandterrie.com
//
// This example code is unlicensed and is released into the public domain
// ----------------------------------------------------------------------------
//
// This sketch is for LCD devices like the Tsingtek Display HC1627 modules
// in i2c mode. Devices such as HC1627-B-LWH-I2C-30 or HC1627-SYH-I2C-30
// These devices have a native I2C interface rather than use a simple I2C
// i/o expander chip such as a PCF8574 or MCP23008.
//
// NOTE:
//	These devices usually need external pullups as they typically are not on
//	the module.
// WARNING:
//	Use caution when using 3v only processors like arm and ESP8266 processors
//	when interfacing with 5v modules as not doing proper level shifting or
//	incorrectly hooking things up can damage the processor.
// 
// Sketch demonstrates hd44780 library automatic line wrapping functionality.
//
// Background:
// hd44780 LCDs do not use linear continuous memory for the characters
// on the lines on the display.
// This means that simply sending continuous characters to the
// display will not fill lines and wrap appropriately as might be expected.
// The hd44780 library solves this issue by adding a line wrapping capability
// in s/w that can be enabled & disabled.
// This allows the host to send characters to the display continuously and they
// will wrap to the next lower line when the end of the visible line has been
// reached. When on the bottom line it will wrap back to the top line.
// 
// (Configure LCD_COLS & LCD_ROWS if desired/needed)
// Expected behavior of the sketch:
// - display a banner announcing the test.
// - print the configured LCD geometry 
// - print a long text string to demostrate automatic line wrapping 
// - print lots of characters (slowly) to show how the full wrapping works.
// (loop)
//
// If initialization of the LCD fails and the arduino supports a built in LED,
// the sketch will simply blink the built in LED with the initalization error
// code.
//
// Special note for certain 16x1 displays:
// Some 16x1 displays are actually a 8x2 display that have both lines on
// a single line on the display.
// If you have one of these displays, simply set the geometry to 8x2 instead
// of 16x1. 
// In normal sketches, lineWrap() mode will allow this type of display to
// properly function as a 16x1 display in that it will allow printing up to
// 16 characters on the display without having to manually set the cursor
// position to print the right characters on the half of the display.
// However, when using this 8x2 display as a 16x1 display, 
// scrollDisplayLeft() and scrollDisplayRight() will not work as intended.
// They will shift the two halves of the display rather than the entire display.
// This is because the hd44780 chip is doing the shift and chip is hard coded
// internally for two lines.
// ----------------------------------------------------------------------------
// While not all modules use the same pinout,
// Be VERY careful and check your datasheet.
//
// This pin table is for the Tsingtek Display HC1627-SYH-I2C-30 or
// HC1627-SYH-I2C-30 module when it is strapped for i2c mode operation
//
// pin 14 is the pin closest to the edge of the PCB
// 14 - connect to gnd
// 13 - connect to vcc
// 12 - ID0 --- controls bit 1 of 7 bit i2c address, strap accordingly
// 11 - ID1 --- controls bit 2 of 7 bit i2c address, strap accordingly
// 10 - not connected
//  9 - not connected
//  8 - SCL
//  7 - SDA
//  6 - connect to gnd
//  5 - connect to gnd
//  4 - connect to gnd
//  3 - Vo Contrast Voltage input
//  2 - VCC (5v)
//  1 - LCD gnd
// 15 - Backlight Anode (+5v)
// 16 - Backlight Cathode (Gnd)
//
// I2C IDx address table
// ID1 ID0 ADDR
//  L   L   0x38/0x39
//  L   H   0x3A/0x3B
//  H   L   0x3C/0x3D
//  H   H   0x3E/0x3F
//
// library only needs to know the base address (the lower address)
//
// ----------------------------------------------------------------------------


// include the needed headers.
#include <Wire.h>
#include <hd44780.h>						// main hd44780 header
#include <hd44780ioClass/hd44780_HC1627_I2C.h>	// i2c LCD i/o class header

// Note, i2c address can be specified or automatically located
// If you wish to use a specific address comment out this constructor
// and use the constructor below that specifies the address

// declare the lcd object for auto i2c address location
hd44780_HC1627_I2C lcd;

//
// manually enter base address of LCD.
// Base Addresses 
// - 0x38, 0x3A, 0x3C, or 0x3E

// declare i2c address and constructor for specified i2c address
//const int i2c_addr = 0x38;
//hd44780_HC1627_I2C lcd(i2c_addr); // use device at this address


// LCD geometry
// while 16x2 will work on most displays even if the geometry is different,
// for actual wrap testing of a particular LCD it is best to use the correct
// geometry.
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

void setup()
{
int status;

	// initialize LCD with number of columns and rows: 
	// hd44780 returns a status from begin() that can be used
	// to determine if initalization failed.
	// the actual status codes are defined in <hd44780.h>
	status = lcd.begin(LCD_COLS, LCD_ROWS);
	if(status) // non zero status means it was unsuccesful
	{
		// begin() failed so blink error code using the onboard LED if possible
		hd44780::fatalError(status); // does not return
	}

	// turn on automatic line wrapping
	// which automatically wraps lines to the next lower line and wraps back
	// to the top when at the bottom line
	// NOTE: 
	// noLineWrap() can be used to disable automatic line wrapping.
	// _write() can be called instead of write() to send data bytes
	// to the display bypassing any special character or line wrap processing.
	lcd.lineWrap();
}

void loop()
{
	lcd.clear();
	lcd.print("WrapTest"); 
	delay(2000);
	lcd.clear();

	//print the configured LCD geometry
	lcd.print(LCD_COLS);
	lcd.print("x");
	lcd.print(LCD_ROWS);
	delay(3000);
	lcd.clear();

	// print a long text string
	// without line wrapping enabled, the text would not wrap properly
	// to the next line.

	if(LCD_COLS == 8)
		lcd.print("A long text line");
	else
		lcd.print("This is a very long line of text");
	delay(3000);

	lcd.clear();

	// now print 2 full displays worth of characters to show
	// the full wrapping.

	lcd.cursor(); // turn on cursor so you can see where it is

	char c = '0'; // start at the character for the number zero
	for(int i = 2*LCD_COLS*LCD_ROWS; i; i--)
	{
		lcd.print(c++);
		delay(200); // slow things down to watch the printing & wrapping

		if(c > 0x7e) // wrap back to beginning of printable ASCII chars
			c = '!'; 
	}
	delay(3000);
	lcd.noCursor(); // turn off cursor
}
