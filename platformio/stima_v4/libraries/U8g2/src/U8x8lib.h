/*

  U8x8lib.h
  
  C++ Arduino wrapper for the u8x8 struct and c functions.
  
  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  


*/

#ifndef U8X8LIB_HH
#define U8X8LIB_HH

#ifdef ARDUINO
#include <Arduino.h>
#include <Print.h>
#endif

#include "clib/u8x8.h"

/*
  Uncomment this to switch off Wire.setClock() invocations.
  This is useful if you connect multiple devices to the same I2C bus that 
  is used for the monochrome display.
  For example the Arduino Nano RP2040 connect uses the only I2C bus 
  already for the internal communication with the integrated on-board components
  wifi, crypto and accelerometer and does not work correctly if the U8g2 library
  modifies the I2c clock speed.
  Instead of uncommenting the line below (which needs a library modification)
  you can also just add the following define before including the U8x8lib header:
      #define U8X8_DO_NOT_SET_WIRE_CLOCK
      #include "U8x8lib.h" 
*/
// #define U8X8_DO_NOT_SET_WIRE_CLOCK

/* 
  Uncomment this to enable AVR optimization for I2C 
  This is disabled by default, because it will not correctly set the pullups.
  Instead the SW will always drive the I2C bus.
*/
//#define U8X8_USE_ARDUINO_AVR_SW_I2C_OPTIMIZATION

/*
  Uncomment this to enable Teensy 3 I2C-Library i2c_t3
  This can/should be used for Teensy >= 3 and Teensy LC.
*/
// #define U8X8_HAVE_HW_I2C_TEENSY3

/* Assumption: All Arduino Boards have "SPI.h" */
#ifndef U8X8_NO_HW_SPI
#define U8X8_HAVE_HW_SPI
#endif

/* Assumption: All Arduino Boards have "Wire.h" */
#ifndef U8X8_NO_HW_I2C
#define U8X8_HAVE_HW_I2C
#endif

/* Undefine U8X8_HAVE_HW_SPI for those Boards without SPI.h */

#ifdef ARDUINO_AVR_DIGISPARK

#ifdef KENDRYTE_K210

#ifdef U8X8_HAVE_HW_SPI
#undef U8X8_HAVE_HW_SPI
#endif 

#endif

#ifdef U8X8_HAVE_HW_SPI
#undef U8X8_HAVE_HW_SPI
#endif 

#ifdef U8X8_HAVE_HW_I2C
#undef U8X8_HAVE_HW_I2C
#endif 

#endif

#ifdef __AVR_ATtiny85__
#ifdef U8X8_HAVE_HW_SPI
#undef U8X8_HAVE_HW_SPI
#endif 

#ifdef U8X8_HAVE_HW_I2C
#undef U8X8_HAVE_HW_I2C
#endif 
#endif

/* ATmegaXXM1 do not have I2C */
#if defined(__AVR_ATmega16M1__) || defined(__AVR_ATmega32M1__) || defined(__AVR_ATmega64M1__)
#ifdef U8X8_HAVE_HW_I2C
#undef U8X8_HAVE_HW_I2C
#endif 
#endif

/* ATmegaXXC1 do not have I2C */
#if defined(__AVR_ATmega16C1__) || defined(__AVR_ATmega32C1__) || defined(__AVR_ATmega64C1__)
#ifdef U8X8_HAVE_HW_I2C
#undef U8X8_HAVE_HW_I2C
#endif 
#endif


/* define U8X8_HAVE_2ND_HW_I2C if the board has a second wire interface*/
#ifdef U8X8_HAVE_HW_I2C
#ifdef WIRE_INTERFACES_COUNT
#if WIRE_INTERFACES_COUNT > 1
#define U8X8_HAVE_2ND_HW_I2C
#endif
#endif
#endif /* U8X8_HAVE_HW_I2C */

/* define U8X8_HAVE_2ND_HW_SPI if the board has a second wire interface*/
/* As of writing this, I did not found any official board which supports this */
/* so this is not tested (May 2017), issue #224 */
/* fixed ifdef, #410, #377 */
/* meanwhile it is defined e.g. here: https://github.com/arduino/ArduinoCore-samd/blob/master/variants/mkrzero/variant.h#L91 */
/* so it should be available for mkrzero */

#ifdef SPI_INTERFACES_COUNT
#if SPI_INTERFACES_COUNT > 1
#define U8X8_HAVE_2ND_HW_SPI
#endif
#endif


extern "C" uint8_t u8x8_gpio_and_delay_arduino(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
extern "C" uint8_t u8x8_byte_arduino_freertos_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

#ifdef U8X8_USE_PINS
void u8x8_SetPin_HW_I2C(u8x8_t *u8x8, uint8_t reset, uint8_t clock = U8X8_PIN_NONE, uint8_t data = U8X8_PIN_NONE);
#endif

class U8X8
#ifdef ARDUINO
: public Print
#endif
{
  protected:
    u8x8_t u8x8;
  public:
    uint8_t tx, ty;
  
    U8X8(void) { home();  }
    u8x8_t *getU8x8(void) { return &u8x8; }

    void sendF(const char *fmt, ...) 
      { va_list va; va_start(va, fmt); u8x8_cad_vsendf(&u8x8, fmt, va); va_end(va); }
    
    uint32_t getBusClock(void) { return u8x8.bus_clock; }
    void setBusClock(uint32_t clock_speed) { u8x8.bus_clock = clock_speed; }
    
    void setI2CAddress(uint8_t adr) { u8x8_SetI2CAddress(&u8x8, adr); }

    uint8_t getCols(void) { return u8x8_GetCols(&u8x8); }
    uint8_t getRows(void) { return u8x8_GetRows(&u8x8); }
    
    void drawTile(uint8_t x, uint8_t y, uint8_t cnt, uint8_t *tile_ptr) {
      u8x8_DrawTile(&u8x8, x, y, cnt, tile_ptr); }

#ifdef U8X8_WITH_USER_PTR
      void *getUserPtr() { return u8x8_GetUserPtr(&u8x8); }
      void setUserPtr(void *p) { u8x8_SetUserPtr(&u8x8, p); }
#endif

      
#ifdef U8X8_USE_PINS 
    /* set the menu pins before calling begin() or initDisplay() */
    void setMenuSelectPin(uint8_t val) {
      u8x8_SetMenuSelectPin(&u8x8, val); }
    void setMenuPrevPin(uint8_t val) {
      u8x8_SetMenuPrevPin(&u8x8, val); }
    void setMenuNextPin(uint8_t val) {
      u8x8_SetMenuNextPin(&u8x8, val); }
    void setMenuUpPin(uint8_t val) {
      u8x8_SetMenuUpPin(&u8x8, val); }
    void setMenuDownPin(uint8_t val) {
      u8x8_SetMenuDownPin(&u8x8, val); }
    void setMenuHomePin(uint8_t val) {
      u8x8_SetMenuHomePin(&u8x8, val); }
#endif
      
    void initDisplay(void) {
      u8x8_InitDisplay(&u8x8); }

    /* call initInterface if the uC comes out of deep sleep mode and display is already running */
    /* initInterface is part if initDisplay, do not call both use either initDisplay OR initInterface */       
    void initInterface(void) {          
      u8x8_InitInterface(&u8x8); }
      
    void clearDisplay(void) {
      u8x8_ClearDisplay(&u8x8); }

    void fillDisplay(void) {
      u8x8_FillDisplay(&u8x8); }
      
    void setPowerSave(uint8_t is_enable) {
      u8x8_SetPowerSave(&u8x8, is_enable); }

    bool begin(void) {
      initDisplay(); clearDisplay(); setPowerSave(0); return 1; }

#ifdef U8X8_USE_PINS 
    /* use U8X8_PIN_NONE if a pin is not required */
    bool begin(uint8_t menu_select_pin, uint8_t menu_next_pin, uint8_t menu_prev_pin, uint8_t menu_up_pin = U8X8_PIN_NONE, uint8_t menu_down_pin = U8X8_PIN_NONE, uint8_t menu_home_pin = U8X8_PIN_NONE) {
      setMenuSelectPin(menu_select_pin);
      setMenuNextPin(menu_next_pin);
      setMenuPrevPin(menu_prev_pin);
      setMenuUpPin(menu_up_pin);
      setMenuDownPin(menu_down_pin);
      setMenuHomePin(menu_home_pin);
      return begin(); }
#endif
      
    void setFlipMode(uint8_t mode) {
      u8x8_SetFlipMode(&u8x8, mode); }

    void refreshDisplay(void) {			// Dec 16: Only required for SSD1606
      u8x8_RefreshDisplay(&u8x8); }
      
    void clearLine(uint8_t line) {
      u8x8_ClearLine(&u8x8, line); }

    void setContrast(uint8_t value) {
      u8x8_SetContrast(&u8x8, value); }

    void setInverseFont(uint8_t value) {
      u8x8_SetInverseFont(&u8x8, value); }

    void setFont(const uint8_t *font_8x8) {
      u8x8_SetFont(&u8x8, font_8x8); }

    void drawGlyph(uint8_t x, uint8_t y, uint8_t encoding) {
      u8x8_DrawGlyph(&u8x8, x, y, encoding); }

    void draw2x2Glyph(uint8_t x, uint8_t y, uint8_t encoding) {
      u8x8_Draw2x2Glyph(&u8x8, x, y, encoding); }

    void draw1x2Glyph(uint8_t x, uint8_t y, uint8_t encoding) {
      u8x8_Draw1x2Glyph(&u8x8, x, y, encoding); }

    void drawString(uint8_t x, uint8_t y, const char *s) {
      u8x8_DrawString(&u8x8, x, y, s); }
      
    void drawUTF8(uint8_t x, uint8_t y, const char *s) {
      u8x8_DrawUTF8(&u8x8, x, y, s); }

    void draw2x2String(uint8_t x, uint8_t y, const char *s) {
      u8x8_Draw2x2String(&u8x8, x, y, s); }

    void draw1x2String(uint8_t x, uint8_t y, const char *s) {
      u8x8_Draw1x2String(&u8x8, x, y, s); }
      
    void draw2x2UTF8(uint8_t x, uint8_t y, const char *s) {
      u8x8_Draw2x2UTF8(&u8x8, x, y, s); }

    void draw1x2UTF8(uint8_t x, uint8_t y, const char *s) {
      u8x8_Draw1x2UTF8(&u8x8, x, y, s); }
      
    uint8_t getUTF8Len(const char *s) {
      return u8x8_GetUTF8Len(&u8x8, s); }
    
    size_t write(uint8_t v);
    /* code extended and moved to .cpp file, issue 74
    size_t write(uint8_t v) {
      u8x8_DrawGlyph(&u8x8, tx, ty, v);
      tx++;
      return 1;
     }
      */
     
    size_t write(const uint8_t *buffer, size_t size) {
      size_t cnt = 0;
      while( size > 0 ) {
	cnt += write(*buffer++); 
	size--;
      }
      return cnt;
    }
     
     void inverse(void) { setInverseFont(1); }
     void noInverse(void) { setInverseFont(0); }
     
    /* return 0 for no event or U8X8_MSG_GPIO_MENU_SELECT, */
    /* U8X8_MSG_GPIO_MENU_NEXT, U8X8_MSG_GPIO_MENU_PREV, */
    /* U8X8_MSG_GPIO_MENU_HOME */
    uint8_t getMenuEvent(void) { return u8x8_GetMenuEvent(&u8x8); }

    uint8_t userInterfaceSelectionList(const char *title, uint8_t start_pos, const char *sl) {
      return u8x8_UserInterfaceSelectionList(&u8x8, title, start_pos, sl); }
    uint8_t userInterfaceMessage(const char *title1, const char *title2, const char *title3, const char *buttons) {
      return u8x8_UserInterfaceMessage(&u8x8, title1, title2, title3, buttons); }
    uint8_t userInterfaceInputValue(const char *title, const char *pre, uint8_t *value, uint8_t lo, uint8_t hi, uint8_t digits, const char *post) {
      return u8x8_UserInterfaceInputValue(&u8x8, title, pre, value, lo, hi, digits, post); }
         
     /* LiquidCrystal compatible functions */
    void home(void) { tx = 0; ty = 0; }
    void clear(void) { clearDisplay(); home(); }
    void noDisplay(void) { u8x8_SetPowerSave(&u8x8, 1); }
    void display(void) { u8x8_SetPowerSave(&u8x8, 0); }
    void setCursor(uint8_t x, uint8_t y) { tx = x; ty = y; }

    void drawLog(uint8_t x, uint8_t y, class U8X8LOG &u8x8log);
    
};

class U8X8LOG
#ifdef ARDUINO
: public Print
#endif
{
  
  public:
    u8log_t u8log;
  
    /* the constructor does nothing, use begin() instead */
    U8X8LOG(void) { }
  
    /* connect to u8g2, draw to u8g2 whenever required */
    bool begin(class U8X8 &u8x8, uint8_t width, uint8_t height, uint8_t *buf)  { 
      u8log_Init(&u8log, width, height, buf);      
      u8log_SetCallback(&u8log, u8log_u8x8_cb, u8x8.getU8x8());
      return true;
    }
    
    /* disconnected version, manual redraw required */
    bool begin(uint8_t width, uint8_t height, uint8_t *buf) { 
      u8log_Init(&u8log, width, height, buf);  
      return true;
    }
    
    void setLineHeightOffset(int8_t line_height_offset) {
      u8log_SetLineHeightOffset(&u8log, line_height_offset); }

    void setRedrawMode(uint8_t is_redraw_line_for_each_char) {
      u8log_SetRedrawMode(&u8log, is_redraw_line_for_each_char); }
    
    /* virtual function for print base class */    
    size_t write(uint8_t v) {
      u8log_WriteChar(&u8log, v);
      return 1;
     }

    size_t write(const uint8_t *buffer, size_t size) {
      size_t cnt = 0;
      while( size > 0 ) {
	cnt += write(*buffer++); 
	size--;
      }
      return cnt;
    }  

    void writeString(const char *s) { u8log_WriteString(&u8log, s); }
    void writeChar(uint8_t c) { u8log_WriteChar(&u8log, c); }
    void writeHex8(uint8_t b) { u8log_WriteHex8(&u8log, b); }
    void writeHex16(uint16_t v) { u8log_WriteHex16(&u8log, v); }
    void writeHex32(uint32_t v) { u8log_WriteHex32(&u8log, v); }
    void writeDec8(uint8_t v, uint8_t d) { u8log_WriteDec8(&u8log, v, d); }
    void writeDec16(uint8_t v, uint8_t d) { u8log_WriteDec16(&u8log, v, d); }    
};


/* u8log_u8x8.c */
inline void U8X8::drawLog(uint8_t x, uint8_t y, class U8X8LOG &u8x8log)
{
  u8x8_DrawLog(&u8x8, x, y, &(u8x8log.u8log)); 
}



#ifdef U8X8_USE_PINS

class U8X8_NULL : public U8X8 {
  public: U8X8_NULL(void) : U8X8() {
    u8x8_Setup(getU8x8(), u8x8_d_null_cb, u8x8_cad_empty, u8x8_byte_empty, u8x8_dummy_cb);
  }
};


// constructor list start
/* generated code (codebuild), u8g2 project */
class U8X8_SH1108_128X160_FREERTOS_HW_I2C : public U8X8 {
  public: U8X8_SH1108_128X160_FREERTOS_HW_I2C(uint8_t reset = U8X8_PIN_NONE) : U8X8() {
    u8x8_Setup(getU8x8(), u8x8_d_sh1108_128x160, u8x8_cad_ssd13xx_i2c, u8x8_byte_arduino_freertos_hw_i2c, u8x8_gpio_and_delay_arduino);
    u8x8_SetPin_HW_I2C(getU8x8(), reset);
  }
};
// constructor list end
  

#endif // U8X8_USE_PINS

#endif /* _U8X8LIB_HH */


