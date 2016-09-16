/**********************************************************************
Copyright (C) 2016  Paolo Paruno <p.patruno@iperbole.bologna.it>
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
**********************************************************************/

#include "Wire.h"
#include <Arduino.h>
#include "registers-wind.h"         //Register definitions
#include "registers-windsonic.h"         //Register definitions
#include "registers-th.h"         //Register definitions
#include "registers-rain.h"         //Register definitions

const char version[] = "1.0";


char getCommand()
{
  char c = '\0';
  if (Serial.available())
  {
    c = Serial.read();
  }
  return c;
}

void displayHelp()
{
  Serial.print(F("\nSensor configuration - "));
  Serial.println(version);
  Serial.println();
  Serial.println(F("Sensor to config:"));
  Serial.println(F("\tw = i2c-wind"));
  Serial.println(F("\ts = i2c-windsonic"));
  Serial.println(F("\tt = i2c-th"));
  Serial.println(F("\tr = i2c-rain"));
  Serial.println(F("\th = hih humidity sensorr"));
  //Serial.println(F("Output:"));
  //Serial.println(F("\tp = toggle printAll - printFound."));
  //Serial.println(F("\th = toggle header - noHeader."));
  Serial.println(F("\n\? = help - this page"));
  Serial.println();
}


void setup() {

  Serial.begin(115200);        // connect to the serial port
  Serial.setTimeout(10000);
  Serial.print(F("Start sensor config"));
  
  //Start I2C communication routines
  Wire.begin();

  //The Wire library enables the internal pullup resistors for SDA and SCL.
  //You can turn them off after Wire.begin()
  // do not need this with patched Wire library
  //digitalWrite( SDA, LOW);
  //digitalWrite( SCL, LOW);
  digitalWrite( SDA, HIGH);
  digitalWrite( SCL, HIGH);
  
  displayHelp();
  
}


void loop() {

  int new_address;
  int oneshot;
  int sensortype;

  char command = getCommand();
  switch (command)
    {
    case 'w':
      
      new_address= -1;
      while (new_address < 1 || new_address > 127){
	Serial.println(F("digit new i2c address for i2c-wind (1-127)"));
	new_address=Serial.parseInt();
	Serial.println(new_address);
      }
      delay(1000);
      
      Wire.beginTransmission(I2C_WIND_DEFAULTADDRESS);
      Wire.write(I2C_WIND_ADDRESS);
      Wire.write(new_address);
      if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
      
      delay(1000);

      sensortype=-1;
      while (sensortype < 1 || sensortype > 2){
	Serial.println(F("digit sensortype code for i2c-wind (1 Davis, 2 Inspeed)"));
	sensortype=Serial.parseInt();
	Serial.println(sensortype);
      }
      delay(1000);
      
      Wire.beginTransmission(I2C_WIND_DEFAULTADDRESS);
      Wire.write(I2C_WIND_SENSORTYPE);
      Wire.write((uint8_t)sensortype);
      if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
      
      delay(1000);

      oneshot=-1;
      while (oneshot < 0 || oneshot > 1){
	Serial.println(F("digit 1 for oneshotmode; 0 for continous mode for i2c-wind (0/1)"));
	oneshot=Serial.parseInt();
	Serial.println(oneshot);
      }
      delay(1000);
      
      Wire.beginTransmission(I2C_WIND_DEFAULTADDRESS);
      Wire.write(I2C_WIND_ONESHOT);
      Wire.write((bool)oneshot);
      if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
      
      delay(1000);
      Wire.beginTransmission(I2C_WIND_DEFAULTADDRESS);
      Wire.write(I2C_WIND_COMMAND);
      Wire.write(I2C_WIND_COMMAND_SAVE);
      if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission 
      
      Serial.println(F("Done; switch off"));
      delay(10000);


      displayHelp();
      break;


    case 's':
      
      new_address= -1;
      while (new_address < 1 || new_address > 127){
	Serial.println(F("digit new i2c address for i2c-windsonic (1-127)"));
	new_address=Serial.parseInt();
	Serial.println(new_address);
      }
      delay(1000);
      
      Wire.beginTransmission(I2C_WINDSONIC_DEFAULTADDRESS);
      Wire.write(I2C_WINDSONIC_ADDRESS);
      Wire.write(new_address);
      if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
      
      delay(1000);

      oneshot=-1;
      while (oneshot < 0 || oneshot > 1){
	Serial.println(F("digit 1 for oneshotmode; 0 for continous mode for i2c-windsonic (0/1)"));
	oneshot=Serial.parseInt();
	Serial.println(oneshot);
      }
      delay(1000);
      
      Wire.beginTransmission(I2C_WINDSONIC_DEFAULTADDRESS);
      Wire.write(I2C_WINDSONIC_ONESHOT);
      Wire.write((bool)oneshot);
      if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
      
      delay(1000);
      Wire.beginTransmission(I2C_WINDSONIC_DEFAULTADDRESS);
      Wire.write(I2C_WINDSONIC_COMMAND);
      Wire.write(I2C_WINDSONIC_COMMAND_SAVE);
      if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission 
      
      Serial.println(F("Done; switch off"));
      delay(10000);


      displayHelp();
      break;


    case 't':

      new_address=-1;
      while (new_address < 1 || new_address > 127){
	Serial.println(F("digit new i2c address for i2c-th (1-127)"));
	new_address=Serial.parseInt();
	Serial.println(new_address);
      }

      delay(1000);      

      Wire.beginTransmission(I2C_TH_DEFAULTADDRESS);
      Wire.write(I2C_TH_ADDRESS);
      Wire.write(new_address);
      if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
      
      delay(1000);

      new_address=-1;
      while (new_address < 1 || new_address > 127){
	Serial.println(F("digit new i2c_temperature address for i2c-th (1-127)"));
	new_address=Serial.parseInt();
	Serial.println(new_address);
      }
      delay(1000);
      
      Wire.beginTransmission(I2C_TH_DEFAULTADDRESS);
      Wire.write(I2C_TH_TEMPERATUREADDRESS);
      Wire.write(new_address);
      if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
      
      delay(1000);

      new_address=-1;
      while (new_address < 1 || new_address > 127){
	Serial.println(F("digit new i2c_humidity address for i2c-th (1-127)"));
	new_address=Serial.parseInt();
	Serial.println(new_address);
      }
      delay(1000);
      
      Wire.beginTransmission(I2C_TH_DEFAULTADDRESS);
      Wire.write(I2C_TH_HUMIDITYADDRESS);
      Wire.write(new_address);
      if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
      
      delay(1000);

      oneshot=-1;
      while (oneshot < 0 || oneshot > 1){
	Serial.println(F("digit 1 for oneshotmode; 0 for continous mode for i2c-th (0/1)"));
	oneshot=Serial.parseInt();
	Serial.println(oneshot);
      }
      delay(1000);
      
      Wire.beginTransmission(I2C_TH_DEFAULTADDRESS);
      Wire.write(I2C_TH_ONESHOT);
      Wire.write((bool)oneshot);
      if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
      
      delay(1000);
      Wire.beginTransmission(I2C_TH_DEFAULTADDRESS);
      Wire.write(I2C_TH_COMMAND);
      Wire.write(I2C_TH_COMMAND_SAVE);
      if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission 
      
      Serial.println(F("Done; switch off"));
      delay(10000);

      displayHelp();
      break;

    case 'r':

      new_address=-1;
      while (new_address < 1 || new_address > 127){
	Serial.println(F("digit new i2c address for i2c-rain (1-127)"));
	new_address=Serial.parseInt();
	Serial.println(new_address);
      }

      delay(1000);      

      Wire.beginTransmission(I2C_RAIN_DEFAULTADDRESS);
      Wire.write(I2C_RAIN_ADDRESS);
      Wire.write(new_address);
      if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
      
      delay(1000);

      oneshot=-1;
      while (oneshot < 0 || oneshot > 1){
	Serial.println(F("digit 1 for oneshotmode; 0 for continous mode for i2c-rain (0/1)"));
	oneshot=Serial.parseInt();
	Serial.println(oneshot);
      }
      delay(1000);
      
      Wire.beginTransmission(I2C_RAIN_DEFAULTADDRESS);
      Wire.write(I2C_RAIN_ONESHOT);
      Wire.write((bool)oneshot);
      if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
      
      delay(1000);
      Wire.beginTransmission(I2C_RAIN_DEFAULTADDRESS);
      Wire.write(I2C_RAIN_COMMAND);
      Wire.write(I2C_RAIN_COMMAND_SAVE);
      if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission 
      
      Serial.println(F("Done; switch off"));
      delay(10000);

      displayHelp();
      break;

    case 'h':
      Serial.println(F("\tto be done"));
      displayHelp();
      break;
      
    case '?':
      displayHelp();
      break;

    case '\0':
      break;
      
    default:
      Serial.println(F("\tinvalid"));
      displayHelp();
      break;
    }
}
