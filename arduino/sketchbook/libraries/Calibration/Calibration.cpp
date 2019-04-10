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
	bool inRange = false; 
	int i = 0;

	if (numPoints == 0) {
	  *concentration = input;
	  return false;
	}
	
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

	float slope;
	float intersection;

	// If the input is in a range, we calculate in the slope 
	// and the intersection of the function
	if (inRange) 
	{
	        IF_CASDEBUG(CADBGSERIAL.println(F("Value is in range")));

		switch (func_type)
		  {
		  case 0: 
		    IF_CASDEBUG(CADBGSERIAL.println(F("LOG function")));
		    // Slope of the logarithmic function 
		    slope = (values[i] - values[i+1]) / (log10(concentrations[i]) - log10(concentrations[i+1]));
		    // Intersection of the logarithmic function
		    intersection = values[i] - slope * log10(concentrations[i]);
		    break;
		  case 1:
		    IF_CASDEBUG(CADBGSERIAL.println(F("LIN function")));
		    // Slope of the linear function 
		    slope = (values[i+1] - values[i]) / (concentrations[i+1] - concentrations[i]);
		    // Intersection of the linear function
		    intersection = values[i] + slope * concentrations[i];
		    break;
		  }		  
	}
	// Else, we calculate the function with the nearest point
	else
	{
	  IF_CASDEBUG(CADBGSERIAL.println(F("Value is not in range ")));
	  if (fabs(input - values[0]) < fabs(input - values[numPoints-1])) {
	    switch (func_type)
	      {
	      case 0:
		// Slope of the logarithmic function
		slope = (values[1] - values[0]) / (log10(concentrations[1]) - log10(concentrations[0]));
		// Intersection of the logarithmic function
		intersection = values[0] - slope * log10(concentrations[0]);
		break;
	      case 1:
		// Slope of the linear function
		slope = (values[1] - values[0]) / (concentrations[1] - concentrations[0]);
		// Intersection of the linear function
		intersection = values[0] - slope * concentrations[0];
		break;
	      }
	  } else {
	    switch (func_type)
	      {
	      case 0:
		// Slope of the logarithmic function
		slope = (values[numPoints-1] - values[numPoints-2]) / (log10(concentrations[numPoints-1]) - log10(concentrations[numPoints-2]));
		// Intersection of the logarithmic function
		intersection = values[numPoints-1] - slope * log10(concentrations[numPoints-1]);
		break;
	      case 1:
		// Slope of the linear function
		slope = (values[numPoints-1] - values[numPoints-2]) / (concentrations[numPoints-1] - concentrations[numPoints-2]);
		// Intersection of the linear function
		intersection = values[numPoints-1] - slope * concentrations[numPoints-1];
		break;
	      }
	  }
	}
	
	// Return the value of the concetration
	IF_CASDEBUG(CADBGSERIAL.print(F("input: ")));
	IF_CASDEBUG(CADBGSERIAL.println(input));
	IF_CASDEBUG(CADBGSERIAL.print(F("intersection: ")));
	IF_CASDEBUG(CADBGSERIAL.println(intersection));
	IF_CASDEBUG(CADBGSERIAL.print(F("slope: ")));
	IF_CASDEBUG(CADBGSERIAL.println(slope));
	
	switch (func_type)
	  {
	  case 0: 
	    *concentration = pow(10, ((input - intersection) / slope));
	    break;
	  case 1: 
	    *concentration = (input - intersection) / slope;
	    break;
	  }
	return true;

}
