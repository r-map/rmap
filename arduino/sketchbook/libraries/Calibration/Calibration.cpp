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


bool Calibration::setCalibrationPoints(float calValues[], float calConcentrations[], uint8_t numPoints)
{ 
	
  if (numPoints > MAX_POINTS) 
    {
      // "ERROR: MAX_POINTS allowed = 10 ");
      return false;
    }
	
  float Ro = calValues[0];
  // Store the calibration values
  for (int i = 0; i < numPoints; i++)
    {
      values[i] = calValues[i] / Ro; 
      concentrations[i] = calConcentrations[i];
    }
 
  return true;

}

bool Calibration::getConcentration(float input,float *concentration)
{
	bool inRange = false; 
	int i = 0;
	
	// This loop is to find the range where the input is located
	while ((!inRange) && (i < (numPoints-1))) {
		
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
	// and the intersection of the logaritmic function
	if (inRange) 
	{
		// Slope of the logarithmic function 
		slope = (values[i] - values[i+1]) / (log10(concentrations[i]) - log10(concentrations[i+1]));
		// Intersection of the logarithmic function
		intersection = values[i] - slope * log10(concentrations[i]);	
	}
	// Else, we calculate the logarithmic function with the nearest point
	else
	{
		if (fabs(input - values[0]) < fabs(input - values[numPoints-1])) {
			// Slope of the logarithmic function
			slope = (values[1] - values[0]) / (log10(concentrations[1]) - log10(concentrations[0]));
			// Intersection of the logarithmic function
			intersection = values[0] - slope * log10(concentrations[0]);
		} else {
			// Slope of the logarithmic function
			slope = (values[numPoints-1] - values[numPoints-2]) / (log10(concentrations[numPoints-1]) - log10(concentrations[numPoints-2]));
			// Intersection of the logarithmic function
			intersection = values[numPoints-1] - slope * log10(concentrations[numPoints-1]);
		}
	}
	
	// Return the value of the concetration
	*concentration = pow(10, ((input - intersection) / slope));
	
	return true;

}
