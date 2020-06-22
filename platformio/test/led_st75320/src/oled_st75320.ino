/*

  "Hello World" version for u8g2 API
   U8g2 Project: STM nucleo Test Board
   with st75320 based display and in particular http://www.jlxlcd.cn/html/zh-detail-867.html
   https://it.aliexpress.com/item/2055196707.html (3.3V I2C version)

   Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)


    D0-> SCL
    D1, D2, D3, D4, D5 must be tied together ->  SDA
    D6, D7 are the two low bits of the device address bits -> GND/VDD
    VDD -> 3.3V
    VSS -> GND

    Additionally you should connect A0 and CS to high (as adviced in
    the controller datasheet).  Finally you must connect the reset
    input to the display somehow: Connecting RES to high might work,
    but maybe better let u8g2 control the RES (reset) signal: This
    means, connect the RES input of the display to any empty GPIO of
    your controller and pass the GPIO number to the u8g2 constructor.


*/


// une u8g2 as alternative to more simple u8x8
#define USE_U8G2


#include <Arduino.h>
#include <Wire.h>

#if defined(ARDUINO_ARCH_STM32)
#define WIREX Wire1
TwoWire WIREX(PB4, PA7);   // D12 A6
// Define the dimension of the U8*log window
#define U8LOG_WIDTH 15
#define U8LOG_HEIGHT 6

#else
#define WIREX Wire

// Define the dimension of the U8*log window
#define U8LOG_WIDTH 20
#define U8LOG_HEIGHT 16

#endif


//display definition
#define OLEDI2CADDRESS 63

// display definitions
#define fontNameS u8g2_font_logisoso32_tf
#define fontNameB u8g2_font_logisoso92_tn
#define fontName u8g2_font_logisoso20_tf

#ifdef USE_U8G2
#include <U8g2lib.h>

U8G2_ST75320_JLX320240_F_2ND_HW_I2C  u8g2(U8G2_R0);

// Create a U8g2log object
U8G2LOG u8g2log;

#else

#include <U8x8lib.h>

U8X8_ST75320_JLX320240_2ND_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
// Create a U8x8log object
U8X8LOG u8x8log;

#endif


// Allocate static memory for the U8x8log window
uint8_t u8log_buffer[U8LOG_WIDTH*U8LOG_HEIGHT];


void setup(void)
{
  WIREX.begin();
  //WIREX.setClock(200000);

  //The Wire library enables the internal pullup resistors for SDA and SCL.
  //You can turn them off after Wire.begin()

  //if you want to set the internal pullup
  //digitalWrite( SDA, HIGH);
  //digitalWrite( SCL, HIGH);

#ifdef USE_U8G2

  // start up display
  u8g2.setI2CAddress(OLEDI2CADDRESS*2);
  u8g2.begin();
  u8g2.setDrawColor(1);
  u8g2.setContrast(80);
  u8g2.setFont(fontNameS);
  u8g2.setFontMode(0); // enable transparent mode, which is faster
  u8g2.clearBuffer();
  u8g2.setCursor(0, 200); 
  u8g2.print(F("Hello World!"));
  u8g2.sendBuffer();

  // Start U8x8log, connect to U8x8, set the dimension and assign the static memory
  u8g2log.begin(u8g2, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
  u8g2log.setLineHeightOffset(0);	// set extra space between lines in pixel, this can be negative
  u8g2log.setRedrawMode(0);		// 0: Update screen with newline, 1: Update screen for every char
  
#else  

  u8x8.setI2CAddress(OLEDI2CADDRESS*2);
  u8x8.begin();
  u8x8.setPowerSave(0);

  u8x8.setFont(fontName);
  u8x8.drawString(0,0,"Hello");
  u8x8.drawString(0,1,"World!");

  // Start U8x8log, connect to U8x8, set the dimension and assign the static memory
  u8x8log.begin(u8x8, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
  
  // Set the U8x8log redraw mode
  u8x8log.setRedrawMode(1);		// 0: Update screen with newline, 1: Update screen for every char  

#endif
}

void loop(void)
{
  delay(1000);

  #ifdef USE_U8G2
  u8g2log.println(millis());
  #else
  u8x8log.println(millis());
  #endif
  
}
