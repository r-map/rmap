/*

  U8x8lib.cpp
  
  Arduino specific low level functions


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

#include "U8x8lib.h"

#include "Wire.h"


/*=============================================*/

size_t U8X8::write(uint8_t v) 
{
  if ( v == '\n' )
  {
    uint8_t dy = u8x8_pgm_read(u8x8.font+3);		/* new 2019 format */
    ty+=dy;
    tx=0;
  }
  else
  {
    uint8_t dx = u8x8_pgm_read(u8x8.font+2);		/* new 2019 format */
    u8x8_DrawGlyph(&u8x8, tx, ty, v);

    tx+=dx;
  }
  return 1;
}

#define UNUSED_PIN_ARDUINO_CALLBACK

/*=============================================*/
/*=== ARDUINO GPIO & DELAY ===*/

#ifdef ARDUINO
#ifdef U8X8_USE_PINS
extern "C" uint8_t u8x8_gpio_and_delay_arduino(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, U8X8_UNUSED void *arg_ptr)
{
  #ifndef UNUSED_PIN_ARDUINO_CALLBACK
  uint8_t i;
  switch(msg)
  {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
    
      for( i = 0; i < U8X8_PIN_CNT; i++ )
	if ( u8x8->pins[i] != U8X8_PIN_NONE )
	{
	  if ( i < U8X8_PIN_OUTPUT_CNT )
	  {
	    pinMode(u8x8->pins[i], OUTPUT);
	  }
	  else
	  {
#ifdef INPUT_PULLUP
	    pinMode(u8x8->pins[i], INPUT_PULLUP);
#else
	    pinMode(u8x8->pins[i], OUTPUT);
	    digitalWrite(u8x8->pins[i], 1);
#endif 
	  }
	}
	  
      break;

#ifndef __AVR__	
    /* this case is not compiled for any AVR, because AVR uC are so slow */
    /* that this delay does not matter */
    case U8X8_MSG_DELAY_NANO:
      delayMicroseconds(arg_int==0?0:1);
      break;
#endif
    
    case U8X8_MSG_DELAY_10MICRO:
      /* not used at the moment */
      break;
    
    case U8X8_MSG_DELAY_100NANO:
      /* not used at the moment */
      break;
   
    case U8X8_MSG_DELAY_MILLI:
      delay(arg_int);
      break;
    case U8X8_MSG_DELAY_I2C:
      /* arg_int is 1 or 4: 100KHz (5us) or 400KHz (1.25us) */
      delayMicroseconds(arg_int<=2?5:2);
      break;
    case U8X8_MSG_GPIO_I2C_CLOCK:
    case U8X8_MSG_GPIO_I2C_DATA:
      if ( arg_int == 0 )
      {
	pinMode(u8x8_GetPinValue(u8x8, msg), OUTPUT);
	digitalWrite(u8x8_GetPinValue(u8x8, msg), 0);
      }
      else
      {
#ifdef INPUT_PULLUP
	pinMode(u8x8_GetPinValue(u8x8, msg), INPUT_PULLUP);
#else
	pinMode(u8x8_GetPinValue(u8x8, msg), OUTPUT);
	digitalWrite(u8x8_GetPinValue(u8x8, msg), 1);
#endif 
      }
      break;
    default:
      if ( msg >= U8X8_MSG_GPIO(0) )
      {
	i = u8x8_GetPinValue(u8x8, msg);
	if ( i != U8X8_PIN_NONE )
	{
	  if ( u8x8_GetPinIndex(u8x8, msg) < U8X8_PIN_OUTPUT_CNT )
	  {
	    digitalWrite(i, arg_int);
	  }
	  else
	  {
	    if ( u8x8_GetPinIndex(u8x8, msg) == U8X8_PIN_OUTPUT_CNT )
	    {
	      // call yield() for the first pin only, u8x8 will always request all the pins, so this should be ok
	      yield();
	    }
	    u8x8_SetGPIOResult(u8x8, digitalRead(i) == 0 ? 0 : 1);
	  }
	}
	break;
      }
      
      return 0;
  }
  #endif // UNUSED_PIN_ARDUINO_CALLBACK
  return 1;
}
#endif // U8X8_USE_PINS


#ifndef __AVR_ARCH__
#define __AVR_ARCH__ 0
#endif


/*=============================================*/
/*=== 4 WIRE HARDWARE SPI ===*/

#ifdef U8X8_USE_PINS


/*=============================================*/
/*=== HARDWARE I2C ===*/

extern "C" uint8_t u8x8_byte_arduino_freertos_hw_i2c(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr)
{
  // Extern C to static cast C++ local pointer
  TwoWire *localPtr = static_cast<TwoWire*>(u8x8->wirePtr);

  switch(msg)
  {
    case U8X8_MSG_BYTE_SEND:
      localPtr->write((uint8_t *)arg_ptr, (int)arg_int);
      break;
    case U8X8_MSG_BYTE_INIT:
      // Not needed
      break;
    case U8X8_MSG_BYTE_SET_DC:
      // Not needed
      break;
    case U8X8_MSG_BYTE_START_TRANSFER:
      // Not needed reset I2C Bus
      localPtr->beginTransmission(u8x8_GetI2CAddress(u8x8)>>1);
      break;
    case U8X8_MSG_BYTE_END_TRANSFER:
      localPtr->endTransmission();
      break;
    default:
      return 0;
  }
  return 1;
}

#endif // U8X8_USE_PINS


void u8x8_SetPin_HW_I2C(u8x8_t *u8x8, uint8_t reset, uint8_t clock, uint8_t data)
{
  #ifndef UNUSED_PIN_ARDUINO_CALLBACK
  u8x8_SetPin(u8x8, U8X8_PIN_RESET, reset);
  u8x8_SetPin(u8x8, U8X8_PIN_I2C_CLOCK, clock);
  u8x8_SetPin(u8x8, U8X8_PIN_I2C_DATA, data);
  #endif
}

#endif // U8X8_USE_PINS
