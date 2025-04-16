/*
  RTC library for FreeRTOS
  version 1.0.0

Copyright (C) 2025  Paolo Patruno <p.patruno@iperbole.bologna.it>
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

#include "frtosRtc.h"

void frtosRtc::begin(DS1307RTC &ds1307rtc, MutexStandard &semaphore){
  _semaphore = semaphore;
  _ds1307rtc = ds1307rtc;
}

time_t frtosRtc::get(){
  LockGuard guard(_semaphore);
  return _ds1307rtc.get();
}

uint8_t frtosRtc::set(time_t t){
  LockGuard guard(_semaphore);
  return _ds1307rtc.set(t);
}

uint8_t frtosRtc::read(tmElements_t &tm){
  LockGuard guard(_semaphore);
  return _ds1307rtc.read(tm);
}

uint8_t frtosRtc::write(tmElements_t &tm){
  LockGuard guard(_semaphore);
  return _ds1307rtc.write(tm);
}


//frtosRtc frtosRTC = frtosRtc();

