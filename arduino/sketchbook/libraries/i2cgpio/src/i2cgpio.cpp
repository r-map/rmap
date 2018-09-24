/*
Copyright (C) 2018  Paolo Paruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Freeg Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "i2cgpio.h"


i2cgpio::i2cgpio(unsigned short int address):
  _address(address)
{
  /*
  //Start I2C communication routines
  Wire.begin();

  //The Wire library enables the internal pullup resistors for SDA and SCL.
  //You can turn them off after Wire.begin()
  // do not need this with patched Wire library
  //digitalWrite( SDA, LOW);
  //digitalWrite( SCL, LOW);
  digitalWrite( SDA, HIGH);
  digitalWrite( SCL, HIGH);
  */  
}


uint8_t i2cgpio::digitalWrite(uint8_t pin, uint8_t value){

  // wake up
  Wire.beginTransmission(_address);
  if (Wire.endTransmission() != 0) return 1;
  delay(10);
  
  Wire.beginTransmission(_address);
  switch (pin)
    {
    case 1:
      Wire.write(I2C_PWM_ONOFF1);
      break;
    case 2:  
      Wire.write(I2C_PWM_ONOFF2);
      break;
    }
  Wire.write(value);
  if (Wire.endTransmission() != 0) return 1;
  
  delay(10);
  Wire.beginTransmission(_address);
  Wire.write(I2C_PWM_COMMAND);
  Wire.write(I2C_PWM_COMMAND_TAKE);
  if (Wire.endTransmission() != 0) return 1;
  return 0;
}


uint8_t i2cgpio::analogWrite(uint8_t pin, uint8_t value){

  // wake up
  Wire.beginTransmission(_address);
  if (Wire.endTransmission() != 0) return 1;
  delay(10);

  Wire.beginTransmission(_address);
  switch (pin)
    {	    
    case 1:
      Wire.write(I2C_PWM_PWM1);
      break;
    case 2:  
      Wire.write(I2C_PWM_PWM2);
      break;
    }	      
  Wire.write(value);
  if (Wire.endTransmission() != 0) return 1;
  delay(10);
  Wire.beginTransmission(_address);
  Wire.write(I2C_PWM_COMMAND);
  Wire.write(I2C_PWM_COMMAND_TAKE);
  if (Wire.endTransmission() != 0) return 1;
  return 0;
}

uint16_t i2cgpio::analogRead(uint8_t pin){

  // wake up
  Wire.beginTransmission(_address);
  if (Wire.endTransmission() != 0) return 1;
  delay(10);

  Wire.beginTransmission(_address);
  Wire.write(I2C_PWM_COMMAND);
  Wire.write(I2C_PWM_COMMAND_ONESHOT_START);
  if (Wire.endTransmission() != 0) return 0xFFFFFFFF;
  delay(1000);
  Wire.beginTransmission(_address);
  Wire.write(I2C_PWM_COMMAND);
  Wire.write(I2C_PWM_COMMAND_ONESHOT_STOP);
  if (Wire.endTransmission() != 0) return 0xFFFFFFFF;
  delay(10);
  Wire.beginTransmission(_address);
  switch (pin)
    {
    case 1:
      Wire.write(I2C_PWM_ANALOG1);
      break;
    case 2:  
      Wire.write(I2C_PWM_ANALOG2);
      break;
    }
  if (Wire.endTransmission() != 0) return 0xFFFFFFFF;       
  Wire.requestFrom(_address,2);
  if (Wire.available() < 2)    // slave may send less than requested
    { 
      //Serial.println(F("no data available"));
      return 0xFFFFFFFF;
    }
  byte LSB = Wire.read();
  byte MSB = Wire.read();
  
  if ((MSB == 255) & (LSB ==255))
    { 
      //Serial.println(F("missing value"));
      return 0xFFFFFFFF;
    }
  
  //it's a 10bit int, from 0 to 1023
  int value = ((MSB << 8) | LSB); // & 0xFFF; 
	
  return value;
}

