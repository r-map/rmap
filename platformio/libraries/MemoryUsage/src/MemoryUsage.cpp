/*
MemoryUsage.c - MemoryUsage library
Copyright (c) 2015 Thierry Paris.  All right reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*! \file MemoryUsage.cpp

Main library code file.
*/

#include "Arduino.h"
#include "MemoryUsage.h"

/// Thanks to adafruit : https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
int mu_freeRam()
{
	//extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

/// Copy / adaptation of the library StackPaint available here : https://github.com/WickedDevice/StackPaint

#define STACK_CANARY	0xa5

// post paint derived by this post:
// https://www.avrfreaks.net/comment/1314296#comment-1314296

void post_StackPaint(void)
{
    uint8_t *p = &_end;

    while(p < (uint8_t*)&p)
    {
        *p = STACK_CANARY;
        p++;
    }
} 

uint16_t post_StackCount(void)
{
  const uint8_t *p =  (const uint8_t*) (__brkval == 0 ? (uint8_t *) &__heap_start : __brkval);
  //const uint8_t *p = &_end;
    uint16_t       c = 0;

    while(*p == STACK_CANARY && p < (uint8_t*)&p)
    {
        p++;
        c++;
    }

    return c;
}

void mu_StackPaint(void) __attribute__((naked)) __attribute__((section(".init1")));


/// Function called before any other function.
// see this issue https://github.com/Locoduino/MemoryUsage/issues/2
void mu_StackPaint(void)
{
#if 1
	uint8_t *p = &_end;

	while (p <= &__stack)
	{
		*p = STACK_CANARY;
		p++;
	}
#else
	__asm volatile (
		"    ldi r30,lo8(_end)\n"
		"    ldi r31,hi8(_end)\n"
		"    ldi r24,lo8(STACK_CANARY)\n" // STACK_CANARY = 0xc5
		"    ldi r25,hi8(__stack)\n"
		"    rjmp .cmp\n"
		".loop:\n"
		"    st Z+,r24\n"
		".cmp:\n"
		"    cpi r30,lo8(__stack)\n"
		"    cpc r31,r25\n"
		"    brlo .loop\n"
		"    breq .loop"::);
#endif
}

/// Checks the first undecorated byte.
uint16_t mu_StackCount(void)
{
	uint8_t *p = (__brkval == 0 ? (uint8_t *) &__heap_start : __brkval);

	while (*p == STACK_CANARY && (int) p <= SP)
		p++;

	return (uint16_t)RAMEND - (uint16_t)p;
}

/// Modified function from http://www.avr-developers.com/mm/memoryusage.html
void SRamDisplay(void)
{
	int	data_size	=	(int)&__data_end - (int)&__data_start;
	int	bss_size	=	(int)&__bss_end - (int)&__data_end;
	int heap_end 	= 	(int) (__brkval == 0 ? (uint8_t *) &__heap_start : __brkval);
	//int	heap_end	=	(int)SP - (int)&__malloc_margin;
	int	heap_size	=	heap_end - (int)&__bss_end;
	int	stack_size	=	RAMEND - (int)SP + 1;
	int	available	=	(RAMEND - (int)&__data_start + 1);
	
	available	-=	data_size + bss_size + heap_size + stack_size;

	Serial.print(F("+----------------+ "));			Serial.print((int)&__data_start);	Serial.println(" (__data_start)");
	Serial.print(F("+      data      +"));			Serial.println();
	Serial.print(F("+    variables   + size = "));	Serial.println(data_size);
	Serial.print(F("+----------------+ "));			Serial.print((int)&__data_end);		Serial.println(" (__data_end / __bss_start)");
	Serial.print(F("+      bss       +"));			Serial.println();
	Serial.print(F("+    variables   + size = "));	Serial.println(bss_size);
	Serial.print(F("+----------------+ "));			Serial.print((int)&__bss_end);		Serial.println(" (__bss_end / __heap_start)");
	Serial.print(F("+      heap      + size = "));	Serial.println(heap_size);
	Serial.print(F("+----------------+ "));			Serial.print((int)heap_end);		Serial.println(" (__brkval if not 0, or __heap_start)");
	Serial.print(F("+                +"));			Serial.println();
	Serial.print(F("+                +"));			Serial.println();
	Serial.print(F("+   FREE RAM     + size = "));	Serial.println(available);
	Serial.print(F("+                +"));			Serial.println();
	Serial.print(F("+                +"));			Serial.println();
	Serial.print(F("+----------------+ "));			Serial.print((int)SP);		Serial.println(" (SP)");
	Serial.print(F("+     stack      + size = "));	Serial.println(stack_size);
	Serial.print(F("+----------------+ "));			Serial.print((int)RAMEND);		Serial.println(" (RAMEND / __stack)");

	Serial.println();
	Serial.println();
}
