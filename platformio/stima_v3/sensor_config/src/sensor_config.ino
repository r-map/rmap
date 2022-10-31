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
#include "registers-radiation.h"           //Register definitions
#include "registers-wind.h"                //Register definitions
#include "registers-th.h"                  //Register definitions
#include "registers-rain.h"                //Register definitions
#include <i2c_utility.h>

byte start_address = 1;
byte end_address = 127;


const char version[] = "1.1";


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
  Serial.println(F("\ts = i2c-radiation"));
  Serial.println(F("\tw = i2c-wind"));
  Serial.println(F("\tt = i2c-th"));
  Serial.println(F("\tr = i2c-rain"));
  //Serial.println(F("Output:"));
  //Serial.println(F("\tp = toggle printAll - printFound."));
  //Serial.println(F("\th = toggle header - noHeader."));
  Serial.println(F("\n\? = help - this page"));
  Serial.println();
}


void setup() {

  Serial.begin(115200);        // connect to the serial port
  Serial.setTimeout(60000);
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

  uint8_t buffer[32];
  char charbuffer[32];

  char command = getCommand();
  switch (command)
    {

    case 's':
      {
	
	int new_address;
	int oneshot;
	
	new_address= -1;
	while (new_address < 1 || new_address > 127){
	  Serial.print(F("digit new i2c address for i2c-radiation (1-127) default: "));
	  Serial.println(I2C_SOLAR_RADIATION_DEFAULT_ADDRESS);
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	delay(1000);

	Wire.beginTransmission(I2C_SOLAR_RADIATION_DEFAULT_ADDRESS);
	buffer[0]=I2C_SOLAR_RADIATION_ADDRESS_ADDRESS;
	buffer[1]=new_address;
	buffer[I2C_SOLAR_RADIATION_ADDRESS_LENGTH+1]=crc8(buffer, I2C_SOLAR_RADIATION_ADDRESS_LENGTH+1);
	Wire.write(buffer,I2C_SOLAR_RADIATION_ADDRESS_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);
	
	oneshot=-1;
	while (oneshot < 0 || oneshot > 1){
	  Serial.println(F("digit 2 for oneshotmode; 1 for continous mode for i2c-radiation (1/2)"));
	  oneshot=Serial.parseInt();
	  Serial.println(oneshot);
	}
	delay(1000);

	Wire.beginTransmission(I2C_SOLAR_RADIATION_DEFAULT_ADDRESS);
	buffer[0]=I2C_SOLAR_RADIATION_ONESHOT_ADDRESS;
	buffer[1]=(bool)(oneshot-1);
	buffer[I2C_SOLAR_RADIATION_ONESHOT_LENGTH+1]=crc8(buffer, I2C_SOLAR_RADIATION_ONESHOT_LENGTH+1);
	Wire.write(buffer,I2C_SOLAR_RADIATION_ONESHOT_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission
	
	delay(1000);
	Serial.println("save configuration");
	Wire.beginTransmission(I2C_SOLAR_RADIATION_DEFAULT_ADDRESS);
	buffer[0]=I2C_COMMAND_ID;
	buffer[1]=I2C_SOLAR_RADIATION_COMMAND_SAVE;
	buffer[2]=crc8(buffer, 2);
	Wire.write(buffer,3);
	if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission
	
	Serial.println(F("Done; switch off"));
	delay(10000);

	displayHelp();
	break;
      }
      
    case 'w':
      {
	
	int new_address;
	int oneshot;
	int sensortype;
	
	new_address= -1;
	while (new_address < 1 || new_address > 127){
	  Serial.print(F("digit new i2c address for i2c-wind (1-127) default: "));
	  Serial.println(I2C_WIND_DEFAULT_ADDRESS);
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	delay(1000);

	Wire.beginTransmission(I2C_WIND_DEFAULT_ADDRESS);
	buffer[0]=I2C_WIND_ADDRESS_ADDRESS;
	buffer[1]=new_address;
	buffer[I2C_WIND_ADDRESS_LENGTH+1]=crc8(buffer, I2C_WIND_ADDRESS_LENGTH+1);
	Wire.write(buffer,I2C_WIND_ADDRESS_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);
	
	sensortype=-1;
	while (sensortype < 1 || sensortype > 2){
	  Serial.println(F("digit sensortype code for i2c-wind (1 Davis, 2 Inspeed)"));
	  sensortype=Serial.parseInt();
	  Serial.println(sensortype);
	}
	delay(1000);

	Wire.beginTransmission(I2C_WIND_DEFAULT_ADDRESS);
	buffer[0]=I2C_WIND_TYPE_ADDRESS;
	buffer[1]=(uint8_t)sensortype;
	buffer[I2C_WIND_TYPE_LENGTH+1]=crc8(buffer, I2C_WIND_TYPE_LENGTH+1);
	Wire.write(buffer,I2C_WIND_TYPE_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);
	
	oneshot=-1;
	while (oneshot < 0 || oneshot > 1){
	  Serial.println(F("digit 2 for oneshotmode; 1 for continous mode for i2c-wind (1/2)"));
	  oneshot=Serial.parseInt();
	  Serial.println(oneshot);
	}
	delay(1000);

	Wire.beginTransmission(I2C_WIND_DEFAULT_ADDRESS);
	buffer[0]=I2C_WIND_ONESHOT_ADDRESS;
	buffer[1]=(bool)(oneshot-1);
	buffer[I2C_WIND_ONESHOT_LENGTH+1]=crc8(buffer, I2C_WIND_ONESHOT_LENGTH+1);
	Wire.write(buffer,I2C_WIND_ONESHOT_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission
	
	delay(1000);
	Serial.println("save configuration");
	Wire.beginTransmission(I2C_WIND_DEFAULT_ADDRESS);
	buffer[0]=I2C_COMMAND_ID;
	buffer[1]=I2C_WIND_COMMAND_SAVE;
	buffer[2]=crc8(buffer, 2);
	Wire.write(buffer,3);
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
	  Serial.print(F("digit new i2c address for i2c-th (1-127) default: "));
	  Serial.println(I2C_TH_DEFAULT_ADDRESS);
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	
	delay(1000);
	
	Wire.beginTransmission(I2C_TH_DEFAULT_ADDRESS);
	buffer[0]=I2C_TH_ADDRESS_ADDRESS;
	buffer[1]=new_address;
	buffer[I2C_TH_ADDRESS_LENGTH+1]=crc8(buffer, I2C_TH_ADDRESS_LENGTH+1);
	Wire.write(buffer,I2C_TH_ADDRESS_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);

	String new_type;	
	Serial.println(F("digit new i2c sensor1 TYPE i2c-th (3 char uppercase)"));
	if (Serial.available() > 0) {
	  new_type = Serial.readStringUntil('\n');
	}

	delay(1000);

	Wire.beginTransmission(I2C_TH_DEFAULT_ADDRESS);
	charbuffer[0]=I2C_TH_SENSOR1_TYPE_ADDRESS;
	new_type.toCharArray(&charbuffer[1], 4);
	charbuffer[I2C_TH_SENSOR1_TYPE_LENGTH+1]=crc8((uint8_t*)charbuffer, I2C_TH_SENSOR1_TYPE_LENGTH+1);
	Wire.write(charbuffer,I2C_TH_SENSOR1_TYPE_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);

	
	new_address=-1;
	while (new_address < 1 || new_address > 127){
	  Serial.println(F("digit new i2c sensor1 address for i2c-th (1-127)"));
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	delay(1000);

	Wire.beginTransmission(I2C_TH_DEFAULT_ADDRESS);
	buffer[0]=I2C_TH_SENSOR1_I2C_ADDRESS_ADDRESS;
	buffer[1]=new_address;
	buffer[I2C_TH_SENSOR1_I2C_ADDRESS_LENGTH+1]=crc8(buffer, I2C_TH_SENSOR1_I2C_ADDRESS_LENGTH+1);
	Wire.write(buffer,I2C_TH_SENSOR1_I2C_ADDRESS_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);



	Serial.println(F("digit new i2c sensor2 TYPE i2c-th (3 char uppercase)"));
	if (Serial.available() > 0) {
	  new_type = Serial.readStringUntil('\n');
	}

	delay(1000);

	Wire.beginTransmission(I2C_TH_DEFAULT_ADDRESS);
	charbuffer[0]=I2C_TH_SENSOR2_TYPE_ADDRESS;
	new_type.toCharArray(&charbuffer[1], 4);
	charbuffer[I2C_TH_SENSOR2_TYPE_LENGTH+1]=crc8((uint8_t*)charbuffer, I2C_TH_SENSOR2_TYPE_LENGTH+1);
	Wire.write(charbuffer,I2C_TH_SENSOR2_TYPE_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);


	new_address=-1;
	while (new_address < 1 || new_address > 127){
	  Serial.println(F("digit new i2c sensor2 address for i2c-th (1-127)"));
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	delay(1000);

	Wire.beginTransmission(I2C_TH_DEFAULT_ADDRESS);
	buffer[0]=I2C_TH_SENSOR2_I2C_ADDRESS_ADDRESS;
	buffer[1]=new_address;
	buffer[I2C_TH_SENSOR2_I2C_ADDRESS_LENGTH+1]=crc8(buffer, I2C_TH_SENSOR2_I2C_ADDRESS_LENGTH+1);
	Wire.write(buffer,I2C_TH_SENSOR2_I2C_ADDRESS_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);


	oneshot=-1;
	while (oneshot < 0 || oneshot > 1){
	  Serial.println(F("digit 2 for oneshotmode; 1 for continous mode for i2c-th (1/2)"));
	  oneshot=Serial.parseInt();
	  Serial.println(oneshot);
	}
	delay(1000);


	Wire.beginTransmission(I2C_TH_DEFAULT_ADDRESS);
	buffer[0]=I2C_TH_ONESHOT_ADDRESS;
	buffer[1]=(bool)(oneshot-1);
	buffer[I2C_TH_ONESHOT_LENGTH+1]=crc8(buffer, I2C_TH_ONESHOT_LENGTH+1);
	Wire.write(buffer,I2C_TH_ONESHOT_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);

	Serial.println("save configuration");
	Wire.beginTransmission(I2C_TH_DEFAULT_ADDRESS);
	buffer[0]=I2C_COMMAND_ID;
	buffer[1]=I2C_TH_COMMAND_SAVE;
	buffer[2]=crc8(buffer, 2);
	Wire.write(buffer,3);
	if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission
	
	Serial.println(F("Done; switch off"));
	delay(10000);

	displayHelp();
	break;
      }
    case 'r':
      {
	int new_address=-1;
	int oneshot;
	uint16_t tipping_bucket_time_ms=0;
	uint8_t rain_for_tip=0;
	
	while (new_address < 1 || new_address > 127){
	  Serial.print(F("digit new i2c address for i2c-rain (1-127) default: "));
	  Serial.println(I2C_RAIN_DEFAULT_ADDRESS);
	  new_address=Serial.parseInt();
	  Serial.println(new_address);
	}
	
	delay(1000);
	
	Wire.beginTransmission(I2C_RAIN_DEFAULT_ADDRESS);
	buffer[0]=I2C_RAIN_ADDRESS_ADDRESS;
	buffer[1]=new_address;
	buffer[I2C_RAIN_ADDRESS_LENGTH+1]=crc8(buffer, I2C_RAIN_ADDRESS_LENGTH+1);
	Wire.write(buffer,I2C_RAIN_ADDRESS_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission
	
	delay(1000);
	
	oneshot=-1;
	while (oneshot < 1 || oneshot > 2){
	  Serial.println(F("digit 2 for oneshotmode; 1 for continous mode for i2c-rain (1/2) (1 is not supported for now)"));
	  oneshot=Serial.parseInt();
	  Serial.println(oneshot);
	}
	delay(1000);


	Wire.beginTransmission(I2C_RAIN_DEFAULT_ADDRESS);
	buffer[0]=I2C_RAIN_ONESHOT_ADDRESS;
	buffer[1]=(bool)(oneshot-1);
	buffer[I2C_RAIN_ONESHOT_LENGTH+1]=crc8(buffer, I2C_RAIN_ONESHOT_LENGTH+1);
	Wire.write(buffer,I2C_RAIN_ONESHOT_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	while (tipping_bucket_time_ms < 2 || tipping_bucket_time_ms > 1000){
	  Serial.println(F("Tipping bucket time in milliseconds for i2c-rain (2-1000)"));
	  tipping_bucket_time_ms=Serial.parseInt();
	  Serial.println(tipping_bucket_time_ms);
	}
	
	delay(1000);

	Wire.beginTransmission(I2C_RAIN_DEFAULT_ADDRESS);
	buffer[0]=I2C_RAIN_TIPTIME_ADDRESS;
	buffer[1]=(uint8_t)tipping_bucket_time_ms;
	buffer[2]=(uint8_t)(tipping_bucket_time_ms >> 8); // Get upper byte of 16-bit var
	buffer[I2C_RAIN_TIPTIME_LENGTH+1]=crc8(buffer, I2C_RAIN_TIPTIME_LENGTH+1);
	Wire.write(buffer,I2C_RAIN_TIPTIME_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	while (rain_for_tip < 1 || rain_for_tip > 20){
	  Serial.println(F("Rain for tip for i2c-rain (1-20)"));
	  rain_for_tip=Serial.parseInt();
	  Serial.println(rain_for_tip);
	}
	
	delay(1000);

	Wire.beginTransmission(I2C_RAIN_DEFAULT_ADDRESS);
	buffer[0]=I2C_RAIN_RAINFORTIP_ADDRESS;
	buffer[1]=rain_for_tip;
	buffer[I2C_RAIN_RAINFORTIP_LENGTH+1]=crc8(buffer, I2C_RAIN_RAINFORTIP_LENGTH+1);
	Wire.write(buffer,I2C_RAIN_RAINFORTIP_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);
	Serial.println("save configuration");
	Wire.beginTransmission(I2C_RAIN_DEFAULT_ADDRESS);
	buffer[0]=I2C_COMMAND_ID;
	buffer[1]=I2C_RAIN_COMMAND_SAVE;
	buffer[2]=crc8(buffer, 2);
	Wire.write(buffer,3);
	if (Wire.endTransmission() != 0)  Serial.println(F("Wire Error"));             // End Write Transmission
	
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
