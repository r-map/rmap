/* Calibration Library
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


#ifndef Calibration_h
#define Calibration_h

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include "config.h"

namespace calibration {

    class Calibration
    {
        public:

      //Calibration();
      bool setCalibrationPoints(float calValues[], float calConcentrations[], uint8_t numberPoints);
      bool getConcentration(float input,float *concentration);
	      
        private:

      // Arrays for point to point calibration
      float values[MAX_POINTS]; 
      float concentrations[MAX_POINTS];
      uint8_t numPoints;

    };
}
#endif




