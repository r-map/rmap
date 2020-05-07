/*

  "Hello World" version for u8g2 API
   U8g2 Project: SSD1306 Test Board
   use Wemos oled board 2.1.0

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

*/

#include <SPI.h>

// une u8g2 as alternative to more simple u8x8
#define USE_U8G2


#include <Arduino.h>
#include <Wire.h>

#ifdef USE_U8G2
// Define the dimension of the U8*log window
#define U8LOG_WIDTH 20
#define U8LOG_HEIGHT 8
#else
// Define the dimension of the U8*log window
#define U8LOG_WIDTH 10
#define U8LOG_HEIGHT 6
#endif


//display definition
#define OLEDI2CADDRESS 0X3C

// display definitions
#define fontNameS u8g2_font_tom_thumb_4x6_tf
#define fontNameB u8g2_font_t0_11_tf
#define fontName u8x8_font_5x7_f

#ifdef USE_U8G2
#include <U8g2lib.h>

U8G2_IL3820_V2_296X128_F_4W_HW_SPI  u8g2(U8G2_R0,8,9,10);
// Create a U8g2log object
U8G2LOG u8g2log;

#else

#include <U8x8lib.h>

U8X8_IL3820_V2_296X128_4W_HW_SPI  u8x8(8,9,10);

// Create a U8x8log object
U8X8LOG u8x8log;

#endif


// Allocate static memory for the U8x8log window
uint8_t u8log_buffer[U8LOG_WIDTH*U8LOG_HEIGHT];


void setup(void)
{

  // start the SPI library:
  SPI.begin();
  
#ifdef USE_U8G2

  // start up display
  u8g2.begin();
  u8g2.setFont(fontNameS);
  u8g2.setFontMode(0); // enable transparent mode, which is faster
  u8g2.clearBuffer();
  u8g2.setCursor(0, 10); 
  u8g2.print(F("Hello World!"));
  u8g2.sendBuffer();

  // Start U8x8log, connect to U8x8, set the dimension and assign the static memory
  u8g2log.begin(u8g2, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
  u8g2log.setLineHeightOffset(0);	// set extra space between lines in pixel, this can be negative
  u8g2log.setRedrawMode(0);		// 0: Update screen with newline, 1: Update screen for every char
  
#else  

  u8x8.begin();
  u8x8.setPowerSave(0);

  u8x8.setFont(fontName);
  u8x8.drawString(0,0,"Hello");
  u8x8.drawString(0,1,"World!");
  u8x8.refreshDisplay();
  
  // Start U8x8log, connect to U8x8, set the dimension and assign the static memory
  u8x8log.begin(u8x8, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
  
  // Set the U8x8log redraw mode
  u8x8log.setRedrawMode(0);		// 0: Update screen with newline, 1: Update screen for every char  

#endif
}

void loop(void)
{
  delay(3000);

  #ifdef USE_U8G2
  u8g2log.println(millis());
  #else
  u8x8log.println(millis());
  u8x8.refreshDisplay();
  #endif
  
}
