/*
 * GPSRTC.h - library for GPS I2C RTC
  
Copyright (C) 2014  Paolo Paruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Paruno <p.patruno@iperbole.bologna.it>

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

#include "Arduino.h"
#include <Wire.h>
#include "GPSRTC.h"


GPSRTC::GPSRTC()
{
  //  Wire.begin();
}
 
// PUBLIC FUNCTIONS
time_t GPSRTC::get()   // Aquire data from buffer and convert to time_t
{
  tmElements_t tm;
  
  if (read(tm) != 0){
    return 0UL;
  }
  return(makeTime(tm));
}

// Aquire data from the RTC chip
uint8_t GPSRTC::read( tmElements_t &tm)
{

  unsigned char msb, lsb;

  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(I2C_GPS_REG_DATETIME);
  if (Wire.endTransmission() != 0) return 1;             // End Write Transmission 
  
  Wire.requestFrom(I2C_ADDRESS, 7);
  delay(100);
  if (Wire.available()>=7){

    msb = Wire.read();
    lsb = Wire.read();
  
    tm.Year = y2kYearToTm(((int) lsb<<8 | msb) - 2000);
    tm.Month = Wire.read();
    tm.Day = Wire.read();
    tm.Hour = Wire.read();
    tm.Minute = Wire.read();
    tm.Second = Wire.read();

    Wire.endTransmission();

    return 0;
  }
  else{
    return 1;
  }
}


GPSRTC GpsRtc = GPSRTC(); // create an instance for the user

