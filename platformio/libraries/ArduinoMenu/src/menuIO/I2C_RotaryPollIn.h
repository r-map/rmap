/**********************************************************************
Copyright (C) 2019  Paolo Paruno <p.patruno@iperbole.bologna.it>
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

/*********************************************************************
 *
 * This program implements management of remote input encoder over I2C
 * 

 Do not require multimaster I2C and poll the i2c remote encoder

use: https://github.com/r-map/rmap/tree/master/arduino/sketchbook/domotica/i2c-gpio-server

register include file are:
https://github.com/r-map/rmap/blob/master/arduino/sketchbook/libraries/Registers/registers-gpio.h
https://github.com/r-map/rmap/blob/master/arduino/sketchbook/libraries/Registers/registers-manager.h

**********************************************************************/

#ifndef RSITE_ARDUINO_MENU_I2C_ROTARYPOLL_ENCODER
  #define RSITE_ARDUINO_MENU_I2C_ROTARYPOLL_ENCODER

#include "Wire.h"
#include "registers-gpio.h"      //Register definitions
#include "registers-manager.h"      //Register definitions



/**
 * This routine turns off the I2C bus and clears it
 * on return SCA and SCL pins are tri-state inputs.
 * You need to call Wire.begin() after this to re-enable I2C
 * This routine does NOT use the Wire library at all.
 *
 * returns 0 if bus cleared
 *         1 if SCL held low.
 *         2 if SDA held low by slave clock stretch for > 2sec
 *         3 if SDA held low after 20 clocks.
 */

namespace Menu {

  template<uint8_t i2caddress=I2C_GPIO_DEFAULTADDRESS>
  class i2cpollencoderIn {
    
  public:
    void begin() {
    }
  };

  //emulate a stream based on i2cpollencoderIn movement returning +/- for every steps
  template<uint8_t i2caddress>
    class i2cpollencoderInStream:public menuIn {
  public:
    i2cpollencoderIn<i2caddress> &enc;//associated hardware i2cpollencoderIn
    i2cpollencoderInStream(i2cpollencoderIn<i2caddress> &enc):enc(enc) {}
    uint8_t      lastcommand;                 //command received
    uint8_t      navail=0;                    //number of command in queue

    int available(void) {

      if (navail > 0) return navail;
      //Serial.println(F("Read command"));             
      Wire.beginTransmission(i2caddress);
      Wire.write(I2C_GPIO_LAST_COMMAND);
      if (Wire.endTransmission() != 0) {
	Serial.println(F("Wire Error"));
      } else {
	Wire.requestFrom(I2C_GPIO_DEFAULTADDRESS,1);
	if (Wire.available() < 1)    // slave may send less than requested
	  { 
	    //Serial.println(F("no data available"));
	  }
	else
	  {
	    lastcommand = Wire.read();
	    //while (Wire.available()) Wire.read();
	    //Serial.print(F("lastcommand: "));
	    //Serial.println(lastcommand);
	    navail=1;
	  }
      }
      return navail;
    }

    int peek(void) override {

      //Check for new incoming command on I2C
      switch (lastcommand) {

      case I2C_MANAGER_NOCOMMAND:
	//Serial.println(F("COMMAND: none"));	
	return -1;
	break;
	
      case I2C_MANAGER_COMMAND_BUTTON1_SHORTPRESSED:
	{
	  //Serial.println(F("COMMAND: button 1 short pressed"));	
	  return options->navCodes[enterCmd].ch;
	  break;
	}
	  
      case I2C_MANAGER_COMMAND_BUTTON1_LONGPRESSED:
	{
	  //Serial.println(F("COMMAND: BUTTON1_LONGPRESSED"));
	  return options->navCodes[escCmd].ch;
	  break;
	}

      case  I2C_MANAGER_COMMAND_ENCODER_RIGHT:
	{
	  //Serial.println(F("COMMAND: ENCODER_RIGHT"));
	  return options->navCodes[upCmd].ch;
	  break;
	}
	
      case I2C_MANAGER_COMMAND_ENCODER_LEFT:
	{
	  //Serial.println(F("COMMAND: ENCODER_LEFT"));
	  return options->navCodes[downCmd].ch;
	  break;
	}
	  
      default:
	{
	  //Serial.println(F("WRONG command"));
	  return -1;
	  break;
	}	
      } //switch  
    }

    int read() override {
      
      uint8_t command=lastcommand;
      lastcommand=I2C_MANAGER_NOCOMMAND;
      navail=0;
      
      switch (command) {
	
      case I2C_MANAGER_NOCOMMAND:
	//Serial.println(F("COMMAND: none"));	
	return -1;
	break;
	
      case I2C_MANAGER_COMMAND_BUTTON1_SHORTPRESSED:
	{
	  //Serial.println(F("COMMAND: button 1 short pressed"));	
	  return options->navCodes[enterCmd].ch;
	  break;
	}
	
      case I2C_MANAGER_COMMAND_BUTTON1_LONGPRESSED:
	{
	  //Serial.println(F("COMMAND: BUTTON1_LONGPRESSED"));
	  return options->navCodes[escCmd].ch;
	  break;
	}
	
      case I2C_MANAGER_COMMAND_ENCODER_RIGHT:
	{
	  //Serial.println(F("COMMAND: ENCODER_RIGHT"));
	  return options->navCodes[upCmd].ch;	    
	  break;
	}
	
      case I2C_MANAGER_COMMAND_ENCODER_LEFT:
	{
	  //Serial.println(F("COMMAND: ENCODER_LEFT"));
	  return options->navCodes[downCmd].ch;
	  break;
	}
	
      default:
	{
	  //Serial.println(F("WRONG command"));
	  return -1;
	  break;
	}	
      } //switch
    }

    void flush() {lastcommand=I2C_MANAGER_NOCOMMAND; navail=0;}
    
    size_t write(uint8_t v) {return 1;}
    
  };

}//namespace Menu

#endif

