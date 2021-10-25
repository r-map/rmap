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


#include "Calibration.h"

using namespace calibration;


/*!
 * @brief	Description: Point to point calculation function 
 * @param 	float: input: input value for getting concentration 
 * @param 	float: output: output concentration value 
 * @return	the status of elaboration
 */


bool Calibration::setCalibrationPoints(float calValues[], float calConcentrations[], uint8_t numberPoints,uint8_t function_type)
{ 
	
  if (numPoints > MAX_POINTS) 
    {
      IF_CASDEBUG(CADBGSERIAL.print(F("ERROR: MAX_POINTS allowed = ")));
      IF_CASDEBUG(CADBGSERIAL.println(MAX_POINTS));
      return false;
    }

  numPoints= numberPoints;
  func_type=function_type;
  
  // Store the calibration values
  for (int i = 0; i < numPoints; i++)
    {
      values[i] = calValues[i];
      concentrations[i] = calConcentrations[i];
    }
 
  return true;

}

bool Calibration::getConcentration(float input,float *concentration)
{
  
  if (numPoints == 0) {
    *concentration = input;
    return false;
  }

  bool inRange = false; 
  int i = 0;
  float intersection;
  uint8_t primo,secondo;
  
  
  // This loop is to find the range where the input is located
  while ((!inRange) && (i < (numPoints-1))) {
    
    IF_CASDEBUG(CADBGSERIAL.print(F("interval: ")));
    IF_CASDEBUG(CADBGSERIAL.print(values[i]));
    IF_CASDEBUG(CADBGSERIAL.print(F(" / ")));
    IF_CASDEBUG(CADBGSERIAL.println(values[i+1]));
    
    if      ((input >  values[i]) && (input <= values[i + 1]))
      inRange = true;
    else if ((input <= values[i]) && (input >  values[i + 1]))
      inRange = true;
    else
			i++;
  }
  
  // If the input is in a range
  if (inRange) {
    IF_CASDEBUG(CADBGSERIAL.println(F("Value is in range")));
    primo=i;
    secondo=i+1;
  } else {
    // Else, we calculate the function with the nearest point
    IF_CASDEBUG(CADBGSERIAL.println(F("Value is not in range ")));
    if (fabs(input - values[0]) < fabs(input - values[numPoints-1])) {
      primo=0;
      secondo=1;
    } else {
      primo=numPoints-2;
      secondo=numPoints-1;	    
    }
  }
  
  IF_CASDEBUG(CADBGSERIAL.print(F("input: ")));
  IF_CASDEBUG(CADBGSERIAL.println(input));
  IF_CASDEBUG(CADBGSERIAL.print(F("intersection: ")));
  IF_CASDEBUG(CADBGSERIAL.println(intersection));
  
  // Return the value of the concetration
  switch (func_type)
    {
    case 0: 
      IF_CASDEBUG(CADBGSERIAL.println(F("LOG function")));
      intersection = (log10(input) - log10(values[primo])) / (log10(values[secondo]) - log10(values[primo]));
      *concentration = concentrations[primo] + pow(10,(log10(concentrations[secondo]) - log10(concentrations[primo])) * intersection);
      break;

    case 1:
      IF_CASDEBUG(CADBGSERIAL.println(F("LIN function")));
      intersection = (input - values[primo]) / (values[secondo] - values[primo]);
      *concentration = concentrations[primo] + (concentrations[secondo] - concentrations[primo]) * intersection;      
      break;
    }		  
  
  return true;
  
}
