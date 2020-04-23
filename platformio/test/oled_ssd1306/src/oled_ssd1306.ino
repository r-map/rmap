/*

  "Hello World" version for u8g2 API
   U8g2 Project: SSD1306 Test Board
   use Wemos oled board 2.1.0

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

*/

// une u8g2 as alternative to more simple u8x8
#define USE_U8G2

//display definition
#define OLEDI2CADDRESS 0X3C
// display definitions
#define fontNameS u8g2_font_tom_thumb_4x6_tf
#define fontNameB u8g2_font_t0_11_tf
#define fontName u8x8_font_5x7_f

#include <Arduino.h>
#include <Wire.h>

#ifdef USE_U8G2
#include <U8g2lib.h>

U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0);

#else

#include <U8x8lib.h>

U8X8_SSD1306_64X48_ER_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

// Create a U8x8log object
U8X8LOG u8x8log;

// Define the dimension of the U8x8log window
#define U8LOG_WIDTH 10
#define U8LOG_HEIGHT 6

// Allocate static memory for the U8x8log window
uint8_t u8log_buffer[U8LOG_WIDTH*U8LOG_HEIGHT];

#endif
  
void setup(void)
{
  Wire.begin();

  //The Wire library enables the internal pullup resistors for SDA and SCL.
  //You can turn them off after Wire.begin()

  //if you want to set the internal pullup
  //digitalWrite( SDA, HIGH);
  //digitalWrite( SCL, HIGH);

#ifdef USE_U8G2

  // start up display
  u8g2.setI2CAddress(OLEDI2CADDRESS*2);
  u8g2.begin();
  u8g2.setFont(fontNameS);
  u8g2.setFontMode(0); // enable transparent mode, which is faster
  u8g2.clearBuffer();
  u8g2.setCursor(0, 10); 
  u8g2.print(F("Hello World!"));
  u8g2.sendBuffer();

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
#ifndef USE_U8G2
  u8x8log.println(millis());
#endif
}
