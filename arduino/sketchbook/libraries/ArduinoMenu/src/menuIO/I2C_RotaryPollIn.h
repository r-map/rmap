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
 * This program implements management of remote input over I2C
 * 
**********************************************************************/

#ifndef RSITE_ARDUINO_MENU_I2C_ROTARY_ENCODER
  #define RSITE_ARDUINO_MENU_I2C_ROTARY_ENCODER

#include "Wire.h"
#include "registers-gpio.h"      //Register definitions
#include "registers-manager.h"      //Register definitions

void stepClock(void){
  for (int i = 0; i < 8; i++) {
    pinMode(SCL, OUTPUT);
    digitalWrite(SCL, LOW);
    delayMicroseconds(6);
    pinMode(SCL, INPUT);
    delayMicroseconds(6);
  }
}


namespace Menu {

  template<uint8_t pinA,uint8_t pinB>
  class encoderIn {
  public:

    void begin() {
      //Wire.onReceive(receiveEvent);          // Set up event handlers
    }

  };

  //emulate a stream based on encoderIn movement returning +/- for every steps
  //buffer not needer because we have an accumulator
  template<uint8_t pinA,uint8_t pinB>
    class encoderInStream:public menuIn {
  public:
    encoderIn<pinA,pinB> &enc;//associated hardware encoderIn
  encoderInStream(encoderIn<pinA,pinB> &enc):enc(enc) {}
    uint8_t      lastcommand;                 //command received
    uint8_t      navail=0;                    //number of command in queue

    int available(void) {

      if (navail > 0) return navail;
      //Serial.println(F("Read command"));             
      Wire.beginTransmission(I2C_GPIO_DEFAULTADDRESS);
      Wire.write(I2C_GPIO_LAST_COMMAND);
      if (Wire.endTransmission() != 0) {
	Serial.println(F("Wire Error"));
	stepClock();
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

