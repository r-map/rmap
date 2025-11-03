/*
  RTC library for FreeRTOS
  version 1.0.0

Copyright (C) 2020  Paolo Patruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef FRTOSRTC_H
#define FRTOSRTC_H
#include <DS1307RTC.h>
#ifdef ARDUINO_ARCH_AVR
  #include <Arduino_FreeRTOS.h>
#else 
  #ifdef ARDUINO_ARCH_STM32
    #include "STM32FreeRTOS.h"
  #else
    #include "FreeRTOS.h"
  #endif
#endif

#include <mutex.hpp>
#include <TimeLib.h>

using namespace cpp_freertos;

/*!
 frtosRtc is a FreeRTOS wrapper to DS1307RTC
*/

class frtosRtc {
private:
  DS1307RTC _ds1307rtc;
  MutexStandard _semaphore;

public:
  /*!
   * default Constructor
   */
  frtosRtc()
    : _ds1307rtc(),
      _semaphore() {}

    /**
    * Initializing, must be called as first.
    * \param ds1307rtc - ds1307 standard driver.
    * \param semaphore - lock i2c bus.
    * \return void
    *
    */
  
  void begin(DS1307RTC &ds1307rtc, MutexStandard &semaphore);

  time_t get();
  uint8_t set(time_t t);
  uint8_t read(tmElements_t &tm);
  uint8_t write(tmElements_t &tm);
  unsigned char isRunning();
};

//extern frtosRtc frtosRTC;

#endif
