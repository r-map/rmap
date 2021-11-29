// ----------------------------------------------------------------------------
// LCDiSpeed - LCD Interface Speed test for LiquidCrystal_PCF8574 library
// ----------------------------------------------------------------------------
// This sketch is a wrapper sketch for the hd44780 library example LCDiSpeed.
// Note:
// This is not a normal sketch and should not be used as model or example
// of hd44780 library sketches.
// This sketch is simple wrapper that declares the needed lcd object for the
// hd44780 library sketch.
// It is provided as a convenient way to run a pre-configured sketch for
// the i/o class.
// The source code for this sketch lives in hd44780 examples:
// hd44780/examples/hd44780examples/LCDiSpeed/LCDiSpeed.ino
// From IDE:
// [File]->Examples-> hd44780/hd44780examples/LCDiSpeed
//

#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

// ugliness to allow this sketch to locate the hd44780 example sketch
// note: hd44780.h is not needed, it is only included to get its directory on the include path
#include <hd44780.h>
#undef hd44780_h // undefine this so the example sketch does not think hd44780 is being used.

// cols and rows don't have to be exact to still get accurate transfer numbers
#define LCD_COLS 16
#define LCD_ROWS 2

// declare the lcd object
// Note: The i2c address must match the backpack address
// and the library only works with certain backpacks
const uint8_t i2cAddr = 0x27;
LiquidCrystal_PCF8574 lcd(i2cAddr);

// tell the hd44780 sketch the lcd object has been declared
#define HD44780_LCDOBJECT

// tell lcd hd44780 sketch to call custom customLCDinit() for initialization
#define LCDISPEED_CALL_CUSTOM_LCDINIT

// have to use custom initialzation for this library
void custom_LCDinit(void)
{
	lcd.begin(LCD_COLS, LCD_ROWS);
	lcd.setBacklight(0xff);	
}

// include the hd44780 library LCDiSpeed sketch source code
#include <examples/hd44780examples/LCDiSpeed/LCDiSpeed.ino>
