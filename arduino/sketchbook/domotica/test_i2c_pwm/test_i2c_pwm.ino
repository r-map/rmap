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

#include "Wire.h"
#include <Arduino.h>
#include <registers-pwm.h>         //Register definitions

byte start_address = 1;
byte end_address = 127;


const char version[] = "1.0";


extern "C" { 
#include "utility/twi.h"  // from Wire library, so we can do bus scanning
}

// Scan the I2C bus between addresses from_addr and to_addr.
// On each address, call the callback function with the address and result.
// If result==0, address was found, otherwise, address wasn't found
// (can use result to potentially get other status on the I2C bus, see twi.c)
// Assumes Wire.begin() has already been called
void scanI2CBus(byte from_addr, byte to_addr, 
                void(*callback)(byte address, byte result) ) 
{
  byte rc;
  byte data = 0; // not used, just an address to feed to twi_writeTo()
  for( byte addr = from_addr; addr <= to_addr; addr++ ) {
    rc = twi_writeTo(addr, &data, 0, 1, 0);
    callback( addr, rc );
  }
}

// Called when address is found in scanI2CBus()
// Feel free to change this as needed
// (like adding I2C comm code to figure out what kind of I2C device is there)
void scanFunc( byte addr, byte result ) {
  Serial.print("addr: ");
  Serial.print(addr,HEX);
  Serial.print(" ");
  Serial.print(addr);
  Serial.print( (result==0) ? " Found!":"       ");
  Serial.print( (addr%4) ? "\t":"\n\r");
}


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
  Serial.print(F("\nManage I2C - "));
  Serial.println(version);
  Serial.println();
  Serial.println(F("scan I2C bus:"));
  Serial.println(F("\ti = scan one time"));
  Serial.println();
  Serial.println(F("\td = change device address"));
  Serial.println();
  Serial.println(F("device to manage:"));
  Serial.println(F("\ta = pwm1"));
  Serial.println(F("\tb = pwm2"));
  Serial.println(F("\n\? = help - this page"));
  Serial.println();
}


void setup() {

  Serial.begin(115200);        // connect to the serial port
  Serial.setTimeout(10000);
  Serial.print(F("Start device manager"));
  
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


  char command = getCommand();
  switch (command)
    {

    case 'd':
      {      

	int new_address;

	new_address= -1;
	while (new_address < 1 || new_address > 127){
	  Serial.println(F("digit new i2c address for i2c-pwm (1-127)"));
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	delay(1000);
      
	Wire.beginTransmission(I2C_PWM_DEFAULTADDRESS);
	Wire.write(I2C_PWM_ADDRESS);
	Wire.write(new_address);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
	
	delay(1000);
	
	
	delay(1000);
	Wire.beginTransmission(I2C_PWM_DEFAULTADDRESS);
	Wire.write(I2C_PWM_COMMAND);
	Wire.write(I2C_PWM_COMMAND_SAVE);
	if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission 
	
	Serial.println(F("Done; switch off"));
	delay(10000);
	
	
	displayHelp();
	break;
      }
	
    case 'a':
    case 'b':
      {      

	int new_value;

	new_value= -1;
	while (new_value < 1 || new_value > 255){
	  Serial.println(F("digit new value (1-255)"));
	  new_value=Serial.parseInt();
	  Serial.println(new_value);
	}
	delay(1000);
      
	Wire.beginTransmission(I2C_PWM_DEFAULTADDRESS);
	switch (command)
	  {
	    
	  case 'a':
	    Wire.write(I2C_PWM_PWM1);
	    break;
	  case 'b':  
	    Wire.write(I2C_PWM_PWM2);
	    break;
	  }	      
	Wire.write(new_value);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
	
	delay(1000);
	Wire.beginTransmission(I2C_PWM_DEFAULTADDRESS);
	Wire.write(I2C_PWM_COMMAND);
	Wire.write(I2C_PWM_COMMAND_TAKE);
	if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission 
	
	Serial.println(F("Done"));
	delay(10000);
	
	
	displayHelp();
	break;
      }

    case 'i':
      {

	Serial.println("\nI2CScanner ready!");
	
	Serial.print("starting scanning of I2C bus from ");
	Serial.print(start_address,HEX);
	Serial.print(" to ");
	Serial.print(end_address,HEX);
	Serial.println("...Hex");
	
	// start the scan, will call "scanFunc()" on result from each address
	scanI2CBus( start_address, end_address, scanFunc );
      
	Serial.println("\ndone");
	
	displayHelp();
	break;
      }
	
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
