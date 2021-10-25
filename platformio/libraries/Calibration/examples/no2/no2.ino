
/*  
 *  
 *  Explanation: Translate sensor resistance or ony other sensor output to
 *  a value getted by interpolate calibration points,
 *  printing the result through the USB
 *  
 *  Copyright (C) 2017 Paolo Patruno
 *  http://rmap.cc 
 *  
 *  This program is free software: you can redistribute it and/or modify  
 *  it under the terms of the GNU General Public License as published by  
 *  the Free Software Foundation, either version 3 of the License, or  
 *  (at your option) any later version.  
 *   
 *  This program is distributed in the hope that it will be useful,  
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of  
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
 *  GNU General Public License for more details.  
 *   
 *  You should have received a copy of the GNU General Public License  
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  
 *  
 */

#include "Calibration.h"


// Concentrations used in calibration process
#define POINT1_PPM_NO2 10.0   // <-- Normal concentration in air
#define POINT2_PPM_NO2 50.0   
#define POINT3_PPM_NO2 100.0 

// Calibration resistences obtained during calibration process (in KOHMs)
#define POINT1_RES_NO2 45.25  // <-- Rs at normal concentration in air
#define POINT2_RES_NO2 25.50
#define POINT3_RES_NO2 3.55

// Define the number of calibration points
#define no2numPoints 3

// Concentratios used in calibration process
#define POINT1_PPM_CO 100.0   // <--- Ro value at this concentration
#define POINT2_PPM_CO 300.0   // 
#define POINT3_PPM_CO 1000.0  // 

// Calibration resistances obtained during calibration process
#define POINT1_RES_CO 230.30 // <-- Ro Resistance at 100 ppm. Necessary value.
#define POINT2_RES_CO 40.665 //
#define POINT3_RES_CO 20.300 //

// Define the number of calibration points
#define conumPoints 3

float coconcentrations[]  = { POINT1_PPM_CO, POINT2_PPM_CO, POINT3_PPM_CO };
float coresistences[]     = { POINT1_RES_CO, POINT2_RES_CO, POINT3_RES_CO };

float no2concentrations[] = {POINT1_PPM_NO2, POINT2_PPM_NO2, POINT3_PPM_NO2};
float no2resistences[]    = {POINT1_RES_NO2, POINT2_RES_NO2, POINT3_RES_NO2};

// NO2 Sensor calibration
calibration::Calibration NO2Cal;

// CO Sensor calibration
calibration::Calibration COCal;


void setup ()

{
  Serial.begin(9600);
  Serial.println("Hello");

  NO2Cal.setCalibrationPoints(no2resistences, no2concentrations, no2numPoints);
  COCal.setCalibrationPoints(coresistences, coconcentrations, conumPoints);
}


void loop ()
{
  //float rs=3.55; // Kohm
  float ppm;

  for (int i=0; i< no2numPoints;i++) {
    if (NO2Cal.getConcentration(no2resistences[i],&ppm))
      {
	Serial.print("NO2 ");
	Serial.println(ppm);
      }else
      {
	Serial.println("calibration ERROR");
      }
  }
  
    //rs=230.3; // Kohm
  for (int i=0; i< no2numPoints;i++) {
    if (COCal.getConcentration(coresistences[i],&ppm))
      {
	Serial.print("CO  ");
	Serial.println(ppm);
      }else
      {
	Serial.println("calibration ERROR");
      }
  }
  
  delay(10000);
}
