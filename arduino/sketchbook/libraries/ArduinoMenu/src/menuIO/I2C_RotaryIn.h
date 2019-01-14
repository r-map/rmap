/**********************************************************************
Copyright (C) 2018  Paolo Paruno <p.patruno@iperbole.bologna.it>
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
#include "registers-manager.h"      //Register definitions

volatile static uint8_t         new_command;                        //new command received (!=0)


//Handler for receiving data
void receiveEvent( int bytesReceived)
{
  //LOGN("receive event, bytes: %d" CR,bytesReceived);

  if (bytesReceived == 2){
    // check for a command
    if (Wire.read() == I2C_MANAGER_COMMAND) {
      //LOGN("       received command: %X" CR,receivedCommands[1]);
      new_command = Wire.read(); return; }
  }  
}

namespace Menu {

  template<uint8_t pinA,uint8_t pinB>
  class encoderIn {
  public:
    volatile int pos=0;

    void begin() {
      Wire.onReceive(receiveEvent);          // Set up event handlers
    }

  };

  //emulate a stream based on encoderIn movement returning +/- for every steps
  //buffer not needer because we have an accumulator
  template<uint8_t pinA,uint8_t pinB>
    class encoderInStream:public menuIn {
  public:
    encoderIn<pinA,pinB> &enc;//associated hardware encoderIn
    int oldPos=0;
  encoderInStream(encoderIn<pinA,pinB> &enc):enc(enc) {}

    int available(void) {
      if (new_command == 0) return 0;
      return 1;
    }

    int peek(void) override {

      //Check for new incoming command on I2C
      if (new_command != 0) {
	switch (new_command) {

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
      return -1;
    }

    int read() override {

      if (new_command != 0) {
	uint8_t _command=new_command;
	new_command=0;
	
	switch (_command) {

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
      return -1;
    }

    void flush() {new_command=0;}
    
    size_t write(uint8_t v) {return 1;}
    
  };

}//namespace Menu

#endif

