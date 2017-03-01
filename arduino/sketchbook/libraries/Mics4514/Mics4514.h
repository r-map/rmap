/* Arduino mics 4514 Library
 * Copyright (C) 2017 by Paolo Patruno
 *
 * This file is part of the RMAP project https://github.com/r-map/rmap
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with the Arduino SdFat Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */


#ifndef _MICS4514_H
#define _MICS4514_H

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

namespace mics4514 {

    class Mics4514
    {
        public:

      Mics4514(uint8_t copin,uint8_t no2pin,uint8_t heaterpin,uint8_t scale1pin,uint8_t scale2pin);
      void sleep();
      void blocking_fast_heat();
      void fast_heat();
      void normal_heat();
      bool query_data(int *co, int *no2);
      bool query_data_auto(int *co, int *no2, int n);
	      
        private:

      uint8_t _copin;
      uint8_t _no2pin;
      uint8_t _heaterpin;
      uint8_t _scale1pin;
      uint8_t _scale2pin;

      enum Micsstatus { cold, hot, fasthot, heating };
      Micsstatus _state;
      uint8_t _lastfastheatertime;
      uint8_t _lastheatertime=0;

      void _filter_data(int n, int *co_table, int *no2_table, int *co, int *no2);
      
    };
}
#endif
