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
  Serial.println(F("\tx = onoff1"));
  Serial.println(F("\ty = onoff2"));
  Serial.println();
  Serial.println(F("\tm = pwm1 dimmer loop"));
  Serial.println(F("\tn = pwm2 dimmer loop"));
  Serial.println();
  Serial.println(F("\tk = analog read 1"));
  Serial.println(F("\tl = analog read 2"));
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
      
	Wire.beginTransmission(I2C_PWM_DEFAULTADDRESS);
	Wire.write(I2C_PWM_ADDRESS);
	Wire.write(new_address);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
	
	delay(1);
	
	Wire.beginTransmission(I2C_PWM_DEFAULTADDRESS);
	Wire.write(I2C_PWM_COMMAND);
	Wire.write(I2C_PWM_COMMAND_SAVE);
	if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission 
	
	Serial.println(F("Done; switch off"));
	
	displayHelp();
	break;
      }
	
    case 'a':
    case 'b':
      {      

	int new_value;

	new_value= -1;
	while (new_value < 0 || new_value > 255){
	  Serial.println(F("digit new value (0-255)"));
	  new_value=Serial.parseInt();
	  Serial.println(new_value);
	}
      
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
	
	delay(1);
	Wire.beginTransmission(I2C_PWM_DEFAULTADDRESS);
	Wire.write(I2C_PWM_COMMAND);
	Wire.write(I2C_PWM_COMMAND_TAKE);
	if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission 
	
	Serial.println(F("Done"));
	
	displayHelp();
	break;
      }

    case 'k':
    case 'l':
      {      
	Wire.beginTransmission(I2C_PWM_DEFAULTADDRESS);
	Wire.write(I2C_PWM_COMMAND);
	Wire.write(I2C_PWM_COMMAND_ONESHOT_START);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));
       	delay(1000);
	Wire.beginTransmission(I2C_PWM_DEFAULTADDRESS);
	Wire.write(I2C_PWM_COMMAND);
	Wire.write(I2C_PWM_COMMAND_ONESHOT_STOP);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));
       	delay(10);

	Wire.beginTransmission(I2C_PWM_DEFAULTADDRESS);
	switch (command)
	  {
	    
	  case 'k':
	    Wire.write(I2C_PWM_ANALOG1);
	    break;
	  case 'l':  
	    Wire.write(I2C_PWM_ANALOG2);
	    break;
	  }
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));       
	
	Wire.requestFrom(I2C_PWM_DEFAULTADDRESS,2);
	if (Wire.available() < 2)    // slave may send less than requested
	  { 
	    Serial.println(F("no data available"));
	  }
	byte LSB = Wire.read();
	byte MSB = Wire.read();
	
	if ((MSB == 255) & (LSB ==255))
	  { 
	    Serial.println(F("missing value"));
	  }
	
	//it's a 10bit int, from 0 to 1023
	int value = ((MSB << 8) | LSB); // & 0xFFF; 
	
	Serial.print(F("value: "));
	Serial.println(value);
	
	Serial.println(F("Done"));
	
	displayHelp();
	break;
      }


    case 'm':
    case 'n':
      {      

	//for( int new_value = -1; new_value < 255; new_value++ ) {
	
	int new_value= 0;
	int inc =1;
	while (new_value >= 0){
	  if (new_value == 255) inc =-1;

	  Serial.println(new_value);
      
	  Wire.beginTransmission(I2C_PWM_DEFAULTADDRESS);
	  switch (command)
	    {
	      
	    case 'm':
	      Wire.write(I2C_PWM_PWM1);
	      break;
	    case 'n':  
	      Wire.write(I2C_PWM_PWM2);
	      break;
	    }	      
	  Wire.write(new_value);
	  if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));               // End Write Transmission 
	
	  delay(1);
	  Wire.beginTransmission(I2C_PWM_DEFAULTADDRESS);
	  Wire.write(I2C_PWM_COMMAND);
	  Wire.write(I2C_PWM_COMMAND_TAKE);
	  if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));              // End Write Transmission 
	  delay(1);	  

	  new_value+= inc;
	}
	
	displayHelp();
	break;
      }
      
    case 'x':
    case 'y':
      {      

	int new_value;

	new_value= -1;
	while (new_value < 0 || new_value > 1){
	  Serial.println(F("digit new value (0/1)"));
	  new_value=Serial.parseInt();
	  Serial.println(new_value);
	}
      
	Wire.beginTransmission(I2C_PWM_DEFAULTADDRESS);
	switch (command)
	  {
	    
	  case 'x':
	    Wire.write(I2C_PWM_ONOFF1);
	    break;
	  case 'y':  
	    Wire.write(I2C_PWM_ONOFF2);
	    break;
	  }
	Wire.write(new_value);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
	
	delay(1);
	Wire.beginTransmission(I2C_PWM_DEFAULTADDRESS);
	Wire.write(I2C_PWM_COMMAND);
	Wire.write(I2C_PWM_COMMAND_TAKE);
	if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission 
	
	Serial.println(F("Done"));
	
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
