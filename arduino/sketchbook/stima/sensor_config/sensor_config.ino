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
//#include "registers-wind.h"         //Register definitions
//#include "registers-windsonic.h"         //Register definitions
#include "registers-th.h"         //Register definitions
#include "registers-rain.h"         //Register definitions
//#include "registers-sdsmics.h"         //Register definitions
//#include <HIH61XXCommander.h>

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
  Serial.print(F("\nSensor configuration - "));
  Serial.println(version);
  Serial.println();
  Serial.println(F("scan I2C bus:"));
  Serial.println(F("\ti = scan one time"));
  Serial.println();
  Serial.println(F("Sensor to config:"));
  Serial.println(F("\tw = i2c-wind"));
  Serial.println(F("\ts = i2c-windsonic"));
  Serial.println(F("\tt = i2c-th"));
  Serial.println(F("\tr = i2c-rain"));
  Serial.println(F("\td = i2c-sds011"));
  Serial.println(F("\th = hih humidity sensor"));
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


  char command = getCommand();
  switch (command)
    {

    case 'w':
      {

	int new_address;
	int oneshot;
	int sensortype;

	new_address= -1;
	while (new_address < 1 || new_address > 127){
	  Serial.println(F("digit new i2c address for i2c-wind (1-127)"));
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	delay(1000);

	//Wire.beginTransmission(I2C_WIND_DEFAULTADDRESS);
	//Wire.write(I2C_WIND_ADDRESS);
	//Wire.write(new_address);
	//if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);

	sensortype=-1;
	while (sensortype < 1 || sensortype > 2){
	  Serial.println(F("digit sensortype code for i2c-wind (1 Davis, 2 Inspeed)"));
	  sensortype=Serial.parseInt();
	  Serial.println(sensortype);
	}
	delay(1000);

	//Wire.beginTransmission(I2C_WIND_DEFAULTADDRESS);
	//Wire.write(I2C_WIND_SENSORTYPE);
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

	//Wire.beginTransmission(I2C_WIND_DEFAULTADDRESS);
	//Wire.write(I2C_WIND_ONESHOT);
	Wire.write((bool)oneshot);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);
	//Wire.beginTransmission(I2C_WIND_DEFAULTADDRESS);
	//Wire.write(I2C_WIND_COMMAND);
	//Wire.write(I2C_WIND_COMMAND_SAVE);
	if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission

	Serial.println(F("Done; switch off"));
	delay(10000);


	displayHelp();
	break;
      }

    case 's':
      {
	int new_address;
	int oneshot;

	new_address= -1;
	while (new_address < 1 || new_address > 127){
	  Serial.println(F("digit new i2c address for i2c-windsonic (1-127)"));
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	delay(1000);

	//Wire.beginTransmission(I2C_WINDSONIC_DEFAULTADDRESS);
	//Wire.write(I2C_WINDSONIC_ADDRESS);
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

	//Wire.beginTransmission(I2C_WINDSONIC_DEFAULTADDRESS);
	//Wire.write(I2C_WINDSONIC_ONESHOT);
	Wire.write((bool)oneshot);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);
	//Wire.beginTransmission(I2C_WINDSONIC_DEFAULTADDRESS);
	//Wire.write(I2C_WINDSONIC_COMMAND);
	//Wire.write(I2C_WINDSONIC_COMMAND_SAVE);
	if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission

	Serial.println(F("Done; switch off"));
	delay(10000);


	displayHelp();
	break;
      }

    case 't':
      {
	int new_address;
  int old_address;
  int oneshot;

  old_address=-1;
  new_address=-1;

  while (old_address < 1 || old_address > 127){
    Serial.println(F("digit old i2c address for i2c-th (1-127)"));
    old_address=Serial.parseInt();
  }

	new_address=-1;
	while (new_address < 1 || new_address > 127){
	  Serial.println(F("digit new i2c address for i2c-th (1-127)"));
	  new_address=Serial.parseInt();
	}

	delay(1000);

	Wire.beginTransmission(old_address);
  Wire.write(I2C_TH_ADDRESS_ADDRESS);
  Wire.write(I2C_TH_ADDRESS_LENGTH);
  Wire.write(new_address);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);

	new_address=-1;
	while (new_address < 1 || new_address > 127){
	  Serial.println(F("digit new i2c temperature address for i2c-th (1-127)"));
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	delay(1000);

	Wire.beginTransmission(old_address);
	Wire.write(I2C_TH_TEMPERATURE_ADDRESS_ADDRESS);
  Wire.write(I2C_TH_TEMPERATURE_ADDRESS_LENGTH);
	Wire.write(new_address);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);

	new_address=-1;
	while (new_address < 1 || new_address > 127){
	  Serial.println(F("digit new i2c humidity address for i2c-th (1-127)"));
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	delay(1000);

	Wire.beginTransmission(old_address);
  Wire.write(I2C_TH_HUMIDITY_ADDRESS_ADDRESS);
  Wire.write(I2C_TH_HUMIDITY_ADDRESS_LENGTH);
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

	Wire.beginTransmission(old_address);
  Wire.write(I2C_TH_ONESHOT_ADDRESS);
  Wire.write(I2C_TH_ONESHOT_LENGTH);
	Wire.write((bool)oneshot);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

  Wire.beginTransmission(old_address);
  Wire.write(I2C_TH_CONTINUOUS_ADDRESS);
  Wire.write(I2C_TH_CONTINUOUS_LENGTH);
  Wire.write(!(bool)oneshot);
  if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);

  Wire.beginTransmission(old_address);
	Wire.write(I2C_COMMAND_ID);
  Wire.write(I2C_TH_COMMAND_SAVE);
	if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission

	Serial.println(F("Done; switch off"));
	delay(10000);

	displayHelp();
	break;
      }
    case 'r':
      {
	int new_address;
  int old_address;
	int oneshot;

  old_address=-1;
	new_address=-1;

  while (old_address < 1 || old_address > 127){
    Serial.println(F("digit old i2c address for i2c-rain (1-127)"));
    old_address=Serial.parseInt();
  }
  
	while (new_address < 1 || new_address > 127){
	  Serial.println(F("digit new i2c address for i2c-rain (1-127)"));
	  new_address=Serial.parseInt();
	}

	delay(1000);

	Wire.beginTransmission(old_address);
	Wire.write(I2C_RAIN_ADDRESS_ADDRESS);
  Wire.write(I2C_RAIN_ADDRESS_LENGTH);
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

	Wire.beginTransmission(old_address);
	Wire.write(I2C_RAIN_ONESHOT_ADDRESS);
  Wire.write(I2C_RAIN_ONESHOT_LENGTH);
	Wire.write((bool)oneshot);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

  Wire.beginTransmission(old_address);
  Wire.write(I2C_RAIN_CONTINUOUS_ADDRESS);
  Wire.write(I2C_RAIN_CONTINUOUS_LENGTH);
  Wire.write(!(bool)oneshot);
  if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);
	Wire.beginTransmission(old_address);
	Wire.write(I2C_COMMAND_ID);
	Wire.write(I2C_RAIN_COMMAND_SAVE);
	if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission

	Serial.println(F("Done; switch off"));
	delay(10000);

	displayHelp();
	break;
      }


    case 'd':
      {
	int new_address;
	int oneshot;

	new_address=-1;
	while (new_address < 1 || new_address > 127){
	  Serial.println(F("digit new i2c address for i2c-sds011 (1-127)"));
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}

	delay(1000);

	//Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);
	//Wire.write(I2C_SDSMICS_ADDRESS);
	Wire.write(new_address);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);

	oneshot=-1;
	while (oneshot < 0 || oneshot > 1){
	  Serial.println(F("digit 1 for oneshotmode; 0 for continous mode for i2c-sds011 (0/1)"));
	  oneshot=Serial.parseInt();
	  Serial.println(oneshot);
	}
	delay(1000);

	//Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);
	//Wire.write(I2C_SDSMICS_ONESHOT);
	Wire.write((bool)oneshot);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);
	//Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);
	//Wire.write(I2C_SDSMICS_COMMAND);
	//Wire.write(I2C_SDSMICS_COMMAND_SAVE);
	if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission

	Serial.println(F("Done; switch off"));
	delay(10000);

	displayHelp();
	break;
      }

    case 'h':
      {
	int address;
	int powerpin;


	Serial.println(F("If you want to use Command Mode to setup HIH61xx sensor you MUST use one pin to power the HIH!"));
	Serial.println(F("If not this will not work!\n\n"));

	address=-1;
	while (address < 1 || address > 127){
	  Serial.println(F("digit old i2c address for HIH sensor (1-127)"));
	  address=Serial.parseInt();
	  Serial.println(address);
	}


	powerpin=-1;
	while (powerpin < 1 || powerpin > 50){
	  Serial.println(F("digit the pin number for power HIH sensor (1-127)"));
	  powerpin=Serial.parseInt();
	  Serial.println(powerpin);
	}

	delay(1000);
	//  Create an HIH61XXCommander with I2C address, powered by one digital pin
	//  If you want to use Command Mode you MUST use the powerPin!
	//HIH61XXCommander hih(address, powerpin);

	//  start the sensor, eeprom data is automatically read.
	//hih.start();

	Serial.println("started HIH fo command mode");

	address=-1;
	while (address < 1 || address > 127){
	  Serial.println(F("digit new i2c address for HIH sensor (1-127)"));
	  address=Serial.parseInt();
	  Serial.println(address);
	}

	//  This is how you change the I2C address to 0x28:
	//hih.setAddress(address);
	//hih.restart();

	//  Make sure we're not in Command Mode any more:
	//hih.leaveCommandMode();


	Serial.println(F("Done; switch off"));
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
