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

#include "Wire.h"

//////////////////////////////////////////////////////////////////////////////////////
// I2C handlers
// Handler for requesting data
//
void requestEvent()
{
  Wire.write(((uint8_t *)i2c_dataset2)+receivedCommands[0],32);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void mgr_command(){

  static uint8_t _command;
  
  //Check for new incoming command on I2C
  if (new_command != 0) {
    _command = new_command;                                                   //save command byte for processing
    new_command = 0;                                                          //clear it
    //_command = _command & 0x0F;                                               //empty 4MSB bits   
    switch (_command) {

    case I2C_MANAGER_COMMAND_BUTTON1_SHORTPRESSED:
      {
	LOGN(F("COMMAND: button 1 short pressed" CR));	
	break;
      }

    case I2C_MANAGER_COMMAND_BUTTON1_LONGPRESSED:
      {
	LOGN(F("COMMAND: BUTTON1_LONGPRESSED" CR));
	break;
      }

    case I2C_MANAGER_COMMAND_ENCODER_RIGHT:
      {
	LOGN(F("COMMAND: ENCODER_RIGHT" CR));
	break;
      }

    case I2C_MANAGER_COMMAND_ENCODER_LEFT:
      {
	LOGN(F("COMMAND: ENCODER_LEFT" CR));
	break;
      }

    default:
      {
	LOGN(F("WRONG command" CR));
	break;
      }	
    } //switch  
  }
}


void begin() {
  Wire.onRequest(requestEvent);          // Set up event handlers
}


//Start I2C communication routines
//Wire.pins(SDA, SCL);
Wire.begin(i2c_address);


void loop() {

  mgr_command();

}
