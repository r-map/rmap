
/*
  FreqCounter.h - Library for a Frequency Counter c.
  Created by Martin Nawrath, KHM Lab3, Dec. 2008
  Released into the public domain.
*/



#ifndef FreqCounter_h
#define FreqCounter_h


#include <avr/interrupt.h>
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

namespace FreqCounter {

	extern unsigned long f_freq;
	extern volatile unsigned char f_ready;
	extern volatile unsigned char f_mlt;
	extern volatile unsigned int f_tics;
	extern volatile unsigned int f_period;
	extern volatile unsigned int f_comp;
	
	void start(int ms);
	
	
}

#endif





