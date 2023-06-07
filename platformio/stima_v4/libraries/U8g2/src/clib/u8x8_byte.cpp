/*

  u8x8_byte.c 
  
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

#include "u8x8.h"

#include <STM32FreeRTOS.h>
#include "ticks.hpp"
#include "thread.hpp"
#include "semaphore.hpp"
#include "Wire.h"

using namespace cpp_freertos;


uint8_t u8x8_byte_SetDC(u8x8_t *u8x8, uint8_t dc)
{
  return u8x8->byte_cb(u8x8, U8X8_MSG_BYTE_SET_DC, dc, NULL);
}

uint8_t u8x8_byte_SendBytes(u8x8_t *u8x8, uint8_t cnt, uint8_t *data)
{
  return u8x8->byte_cb(u8x8, U8X8_MSG_BYTE_SEND, cnt, (void *)data);
}

uint8_t u8x8_byte_SendByte(u8x8_t *u8x8, uint8_t byte)
{
  return u8x8_byte_SendBytes(u8x8, 1, &byte);
}

// Using 
uint8_t u8x8_byte_StartTransfer(u8x8_t *u8x8)
{
  BinarySemaphore *localLock = static_cast<BinarySemaphore *>(u8x8->wireLockPtr);
	if (localLock->Take(Ticks::MsToTicks(1000))) {
    return u8x8->byte_cb(u8x8, U8X8_MSG_BYTE_START_TRANSFER, 0, NULL);
  }
  else
    return 1;
}

uint8_t u8x8_byte_EndTransfer(u8x8_t *u8x8)
{
  BinarySemaphore *localLock = static_cast<BinarySemaphore *>(u8x8->wireLockPtr);
	localLock->Give();
  return u8x8->byte_cb(u8x8, U8X8_MSG_BYTE_END_TRANSFER, 0, NULL);
}

