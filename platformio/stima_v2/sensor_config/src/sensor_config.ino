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
#include <registers-wind_v2.h>         //Register definitions
#include <registers-windsonic.h>         //Register definitions
#include <registers-th_v2.h>         //Register definitions
#include <registers-rain_v2.h>         //Register definitions
#include <registers-sdsmics.h>         //Register definitions
#include <HIH61XXCommander.h>
#include "Calibration.h"

byte start_address = 1;
byte end_address = 127;


const char version[] = "1.0";


//extern "C" { 
  //#include "utility/twi.h"  // from Wire library, so we can do bus scanning
//}

// Scan the I2C bus between addresses from_addr and to_addr.
// On each address, call the callback function with the address and result.
// If result==0, address was found, otherwise, address wasn't found
// (can use result to potentially get other status on the I2C bus, see twi.c)
// Assumes Wire.begin() has already been called
void scanI2CBus(byte from_addr, byte to_addr, 
                void(*callback)(byte address, byte result) ) 
{
  byte rc;
  //byte data = 0; // not used, just an address to feed to twi_writeTo()
  for( byte addr = from_addr; addr <= to_addr; addr++ ) {

    Wire.beginTransmission (addr);
    rc = (Wire.endTransmission () != 0);
    //rc = twi_writeTo(addr, &data, 0, 1, 0);
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
  Serial.println(F("\td = i2c-sdsmics"));
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
      }

    case 't':
      {
	int new_address;
	int oneshot;

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
      }
    case 'r':
      {
	int new_address;
	int oneshot;

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
      }


    case 'd':
      {
	int new_address;
	int oneshot;

	new_address=-1;
	while (new_address < 1 || new_address > 127){
	  Serial.println(F("digit new i2c address for i2c-sdsmics (1-127)"));
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	
	delay(1000);      
	
	Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);
	Wire.write(I2C_SDSMICS_ADDRESS);
	Wire.write(new_address);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
	
	delay(1000);

 	oneshot=-1;
	while (oneshot < 0 || oneshot > 1){
	  Serial.println(F("digit 1 for oneshotmode; 0 for continous mode for i2c-sdsmics (0/1)"));
	  oneshot=Serial.parseInt();
	  Serial.println(oneshot);
	}
	delay(1000);
	
	Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);
	Wire.write(I2C_SDSMICS_ONESHOT);
	Wire.write((bool)oneshot);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 

 	int8_t calibration=-1;

	//NO2
	while (true){
	  while (calibration < 0 || calibration >= MAX_POINTS){
	    Serial.println(F("Select calibration point (0/MAX_POINT-1) for NO2"));
	    Serial.println(F("digit 0 - MAX_POINTS-1 for calibration; MAX_POINTS to continue without calibration or terminate calibration for i2c-sdsmics"));
	    calibration=Serial.parseInt();
	    Serial.println(calibration);
	  }
	  if (calibration == MAX_POINTS) break;

	  delay(1000);
	  
	  Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);   // Open I2C line in write mode
	  Wire.write(I2C_MICS4514_NO2RESISTANCE);             // Set the register pointer to I2C_MICS4514_NO2RESISTANCE
	  if (Wire.endTransmission() != 0){
	    Serial.println(F("ERROR writing on I2C BUS"));
	    return;
	  }
	  
	  Wire.requestFrom(I2C_SDSMICS_DEFAULTADDRESS,2);
	  if (Wire.available() < 2){    // slave may send less than requested
	    Serial.println(F("ERROR reading on I2C BUS"));
	    return;
	  }
	  
	  byte MSB = Wire.read();
	  byte LSB = Wire.read();
	  
	  if ((MSB == 255) & (LSB ==255)){ 
	    Serial.println(F("Missing value reading resistance"));
	    return;
	  }
	  
	  float resistance = float((MSB << 8) | LSB); 
	  
	  float concentration=0.;
	  while (concentration == 0.){
	    Serial.println(F("digit concentration for calibration"));
	    concentration=Serial.parseFloat();
	    Serial.println(concentration);
	  }
	    

	  Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);
	  Wire.write(NO2CONCENTRATIONS+sizeof(concentration)*calibration);
	  Wire.write((uint8_t*)&resistance,4);
	  if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 

	  Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);
	  Wire.write(NO2RESISTENCES+sizeof(concentration)*calibration);
	  Wire.write((uint8_t*)&concentration,4);
	  if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
	  	  
	}

	while (calibration < 0 || calibration >= MAX_POINTS){
	  Serial.println(F("digit number of calibration points for i2c-sdsmics"));
	  calibration=Serial.parseInt();
	  Serial.println(calibration);
	}

	Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);
	Wire.write(NO2NUMPOINTS);
	Wire.write(calibration);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 


	// PM25
	while (true){
	  while (calibration < 0 || calibration >= MAX_POINTS){
	    Serial.println(F("Select calibration point (0/MAX_POINT-1) for PM 2.5"));
	    Serial.println(F("digit 0 - MAX_POINTS-1 for calibration; MAX_POINTS to continue without calibration or terminate calibration for i2c-sdsmics"));
	    calibration=Serial.parseInt();
	    Serial.println(calibration);
	  }
	  if (calibration == MAX_POINTS) break;

	  delay(1000);
	  
	  Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);   // Open I2C line in write mode
	  Wire.write(I2C_MICS4514_PM25SAMPLE);             // Set the register pointer to I2C_MICS4514_NO2RESISTANCE
	  if (Wire.endTransmission() != 0){
	    Serial.println(F("ERROR writing on I2C BUS"));
	    return;
	  }
	  
	  Wire.requestFrom(I2C_SDSMICS_DEFAULTADDRESS,2);
	  if (Wire.available() < 2){    // slave may send less than requested
	    Serial.println(F("ERROR reading on I2C BUS"));
	    return;
	  }
	  
	  byte MSB = Wire.read();
	  byte LSB = Wire.read();
	  
	  if ((MSB == 255) & (LSB ==255)){ 
	    Serial.println(F("Missing value reading resistance"));
	    return;
	  }
	  
	  float sample = float((MSB << 8) | LSB); 
	  
	  float concentration=0.;
	  while (concentration == 0.){
	    Serial.println(F("digit concentration for calibration"));
	    concentration=Serial.parseFloat();
	    Serial.println(concentration);
	  }
	    

	  Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);
	  Wire.write(PM25CONCENTRATIONS+sizeof(concentration)*calibration);
	  Wire.write((uint8_t*)&sample,4);
	  if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 

	  Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);
	  Wire.write(PM25SAMPLES+sizeof(concentration)*calibration);
	  Wire.write((uint8_t*)&concentration,4);
	  if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
	  	  
	}

	while (calibration < 0 || calibration >= MAX_POINTS){
	  Serial.println(F("digit number of calibration points for i2c-sdsmics"));
	  calibration=Serial.parseInt();
	  Serial.println(calibration);
	}

	Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);
	Wire.write(PM25NUMPOINTS);
	Wire.write(calibration);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 


	// PM10
	while (true){
	  while (calibration < 0 || calibration >= MAX_POINTS){
	    Serial.println(F("Select calibration point (0/MAX_POINT-1) for PM 10"));
	    Serial.println(F("digit 0 - MAX_POINTS-1 for calibration; MAX_POINTS to continue without calibration or terminate calibration for i2c-sdsmics"));
	    calibration=Serial.parseInt();
	    Serial.println(calibration);
	  }
	  if (calibration == MAX_POINTS) break;

	  delay(1000);
	  
	  Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);   // Open I2C line in write mode
	  Wire.write(I2C_MICS4514_PM10SAMPLE);             // Set the register pointer to I2C_MICS4514_NO2RESISTANCE
	  if (Wire.endTransmission() != 0){
	    Serial.println(F("ERROR writing on I2C BUS"));
	    return;
	  }
	  
	  Wire.requestFrom(I2C_SDSMICS_DEFAULTADDRESS,2);
	  if (Wire.available() < 2){    // slave may send less than requested
	    Serial.println(F("ERROR reading on I2C BUS"));
	    return;
	  }
	  
	  byte MSB = Wire.read();
	  byte LSB = Wire.read();
	  
	  if ((MSB == 255) & (LSB ==255)){ 
	    Serial.println(F("Missing value reading resistance"));
	    return;
	  }
	  
	  float sample = float((MSB << 8) | LSB); 
	  
	  float concentration=0.;
	  while (concentration == 0.){
	    Serial.println(F("digit concentration for calibration"));
	    concentration=Serial.parseFloat();
	    Serial.println(concentration);
	  }
	    

	  Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);
	  Wire.write(PM10CONCENTRATIONS+sizeof(concentration)*calibration);
	  Wire.write((uint8_t*)&sample,4);
	  if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 

	  Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);
	  Wire.write(PM10SAMPLES+sizeof(concentration)*calibration);
	  Wire.write((uint8_t*)&concentration,4);
	  if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 
	  	  
	}

	while (calibration < 0 || calibration >= MAX_POINTS){
	  Serial.println(F("digit number of calibration points for i2c-sdsmics"));
	  calibration=Serial.parseInt();
	  Serial.println(calibration);
	}

	Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);
	Wire.write(PM25NUMPOINTS);
	Wire.write(calibration);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission 

	
	// TODO the same for I2C_MICS4514_CORESISTANCE


	// Save !
	delay(1000);
	Wire.beginTransmission(I2C_SDSMICS_DEFAULTADDRESS);
	Wire.write(I2C_SDSMICS_COMMAND);
	Wire.write(I2C_SDSMICS_COMMAND_SAVE);
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
	HIH61XXCommander hih(address, powerpin);
	
	//  start the sensor, eeprom data is automatically read.
	hih.start();
	
	Serial.println("started HIH fo command mode");
 
	address=-1;
	while (address < 1 || address > 127){
	  Serial.println(F("digit new i2c address for HIH sensor (1-127)"));
	  address=Serial.parseInt();
	  Serial.println(address);
	}

	//  This is how you change the I2C address to 0x28:
	hih.setAddress(address);
	hih.restart();
	
	//  Make sure we're not in Command Mode any more:
	hih.leaveCommandMode();
	  

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
