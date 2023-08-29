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

#define WIND_POWER_ON_DELAY_MS                          (5000)
#define WIND_POWER_PIN                                  (4)
#define GWS_SERIAL_BAUD                                 (9600)
#define GWS_SERIAL_TIMEOUT_MS                           (500)
#define UART_RX_BUFFER_LENGTH                           (120)
uint16_t uart_rx_buffer_length;
char uart_rx_buffer[UART_RX_BUFFER_LENGTH];
#ifndef ARDUINO_ARCH_AVR
//HardwareSerial Serial1(PB11, PB10);
HardwareSerial Serial1(D0, D1);
#endif

const char version[] = "2.0";

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

void windsonicSerialReset() {
  // reset serial settings
  Serial1.setTimeout(GWS_SERIAL_TIMEOUT_MS);
  Serial1.begin(GWS_SERIAL_BAUD);
  windsonicFlush();
}

void windsonicReceiveTerminatedMessage(const char terminator){
  // receive message on serial until terminator is found and terminate message
  uart_rx_buffer_length=Serial1.readBytesUntil(terminator, uart_rx_buffer, UART_RX_BUFFER_LENGTH);
  uart_rx_buffer[uart_rx_buffer_length] = '\0';
  uart_rx_buffer_length++;
  Serial.print(F("receive:"));
  Serial.println(uart_rx_buffer);
}

void windsonicReceiveMessage(const char terminator){
  uart_rx_buffer_length=Serial1.readBytesUntil(terminator, uart_rx_buffer, UART_RX_BUFFER_LENGTH);
}

void windsonicPowerOff () {
  digitalWrite(WIND_POWER_PIN, LOW);
}

void windsonicPowerOn () {
  digitalWrite(WIND_POWER_PIN, HIGH);
}

void windsonicFlush(void){
  while (Serial1.available() > 0){
    //Serial1.read();
    Serial.write((uint8_t)Serial1.read());
    Serial.flush();
  }
  memset(uart_rx_buffer, 0, UART_RX_BUFFER_LENGTH);
  uart_rx_buffer_length = 0;
}

bool windsonicEnterConfigMode(void){
  // try to enter in config mode
  // windsonic can be in polled or continuous mode
  // return true on success
  
  uint8_t count=0;
  bool config_mode=false;
  Serial1.setTimeout(1000);  
  Serial.println(F("try to enter configure mode"));
  windsonicFlush();
  do {
    /*
    if ((count % 2) == 0) {
      Serial.println("send:*Q");
      Serial1.print("*Q");
      Serial1.flush();
    }else{
      Serial.println("send:*");
      Serial1.print("*");
      Serial1.flush();
    }
    */

    Serial.println("send:**Q");
    Serial1.print("**Q");
    delay(100);
    Serial.println("send:**Q");
    Serial1.print("**Q");
    delay(100);
    Serial.println("send:**Q");
    Serial1.print("**Q");
    Serial1.flush();
    
    #define CONFMSG "CONFIGURATION MODE"
    uint8_t countr=0;
    do{
      uart_rx_buffer_length=Serial1.readBytesUntil('\r', uart_rx_buffer, UART_RX_BUFFER_LENGTH-1);
      countr++;
      if (uart_rx_buffer_length >= strlen(CONFMSG)){
	uart_rx_buffer[uart_rx_buffer_length] = '\0';
	uart_rx_buffer_length++;
	Serial.print(F("receive:"));
	Serial.println(uart_rx_buffer);
	if (strcmp(&uart_rx_buffer[uart_rx_buffer_length-strlen(CONFMSG)-1],CONFMSG)==0){
	  Serial.println(F("entered configure mode"));
	  config_mode=true;
	  break;
	}
      }
      //} while ((uart_rx_buffer_length > 0) || (countr < 10));
    } while (countr < 3);
    if (!config_mode) {
      count++;
      delay(1000);
      Serial.println("send:Q");
      Serial1.print("Q\r\n");
      Serial1.flush();
      delay(1000);
      windsonicFlush();
    }
  } while ((!config_mode) && (count < 2));
  
  windsonicFlush();
  return config_mode;
}


// this is required to reset windsonic to default configuration and baud
bool windsonicEnterConfigModeAllBaudrate() {
  // try to enter in config mode
  // windsonic can be in polled or continuous mode
  // no matter about witch baudrate windsonic want to comunicate
  // return true on success

  // change baudrate on winsonic is a very strange procedure!
  // it need confirmation and we need to set other things before
  // quitting to force windsonic to save parameters
  
  /* Initialize serial for wind sensor comunication
     WindSonic default settings are :
     Bits per second            option 1: 9600 ; option 2 : 19200
     Data bits                  8
     Parity                     None
     Stop bits                  1
     Flow Control(Handshaking)  None
  */

  // try different fixed baud rate
  long int baudrate []={9600,2400,4800,19200,38400};

  for (byte i=0; (i<(sizeof(baudrate) / sizeof(long int))); i++) {

    //ATTENTION here all is blocking!

    Serial.print(F("TRY BAUDRATE: "));
    Serial.println(baudrate[i]);
    Serial1.begin(baudrate[i]);
    
    if (windsonicEnterConfigMode()){   
      Serial.println(F("send: D3"));
      Serial1.print("D3\r\n");
      windsonicReceiveTerminatedMessage('\r');
      delay(500);
      windsonicFlush();

      Serial.println(F("send: M2"));
      Serial1.print("M2\r\n");
      windsonicReceiveTerminatedMessage('\r');
      delay(500);
      windsonicFlush();

      Serial.println(F("send: M4"));
      Serial1.print("M4\r\n");
      windsonicReceiveTerminatedMessage('\r');
      delay(500);
      windsonicFlush();
      
      Serial.println(F("send: D3"));
      Serial1.print("D3\r\n");
      windsonicReceiveTerminatedMessage('\r');
      delay(500);
      windsonicFlush();
      
      Serial.println(F("baudrate found"));
          
      //// set Communications protocol RS232
      //Serial.println(F("send: E3"));
      //Serial1.print("E3\r\n");
      //windsonicReceiveTerminatedMessage('\r');
      //delay(500);
      //windsonicFlush();

      // set Baud rate 9600
      Serial.println(F("send: B3"));
      Serial1.print("B3\r\n");
      delay(1000);
      windsonicFlush();
      Serial1.end();
      Serial1.begin(GWS_SERIAL_BAUD);
      delay(1000);
      Serial.println(F("send: B at 9600"));
      Serial1.print("B\r\n");
      delay(1000);
      windsonicFlush();

      Serial.println(F("send: M2"));
      Serial1.print("M2\r\n");
      windsonicReceiveTerminatedMessage('\r');
      delay(500);
      windsonicFlush();
      
      Serial.println("send:Q");
      Serial1.print("Q\r\n");
      Serial1.flush();
      delay(1000);
      windsonicFlush();

      if (!windsonicEnterConfigMode()){
	Serial.println("failed");
      } else { 
	return true;
      }
    }

    windsonicPowerOff();
    delay(1000);
    windsonicPowerOn();
    delay(WIND_POWER_ON_DELAY_MS);
    windsonicSerialReset();
       
  }
  Serial.println(F("inizialize failed"));
  return false;
}

// this is required to reset windsonic to default configuration and baud
bool windsonicInitSafeMode() {
  //use safe mode to reset windsonic to comunicate on RS232
  // safe mode in windsonic is very strange
  // only some paramters are taken in account for save
  // for example baudrate can be changed only in configuration mode ...
  
  Serial.println(F("TRY SAFE MODE"));
  Serial1.end();
  Serial1.begin(19200);
  Serial1.setTimeout(10);  
  windsonicFlush();

  uint8_t count=0;
  bool config_mode=false;
  #define SAFEMSG "SAFE MODE (RS232 ONLY)"
  do {
    Serial.println(F("try to enter safe mode"));
    windsonicPowerOff();
    delay(2000);
    windsonicPowerOn();
    uint16_t countr=0;
    while (countr < 400){
      Serial1.print("*****************");      // enter in setup
      Serial1.flush();
      countr++;
      uart_rx_buffer_length=Serial1.readBytesUntil('\r', uart_rx_buffer, UART_RX_BUFFER_LENGTH-1);
      if (uart_rx_buffer_length >= strlen(SAFEMSG)){
	uart_rx_buffer[uart_rx_buffer_length] = '\0';
	uart_rx_buffer_length++;
	Serial.print(F("receive:"));
	Serial.println(uart_rx_buffer);
	if (strcmp(&uart_rx_buffer[uart_rx_buffer_length-strlen(SAFEMSG)-1],SAFEMSG)==0){
	  Serial.println(F("entered safe mode"));
	  config_mode=true;
	  break;
	}
      }
    }
    count++;
  } while ((!config_mode) && (count < 3));
  
  Serial1.print("\r\n");
  delay(500);
  windsonicFlush();
  
  if (config_mode){


    Serial.println(F("send: M2"));
    Serial1.print("M2\r\n");
    windsonicReceiveTerminatedMessage('\r');
    delay(500);
    windsonicFlush();
    
    Serial.println(F("send: M4"));
    Serial1.print("M4\r\n");
    windsonicReceiveTerminatedMessage('\r');
    delay(500);
    windsonicFlush();
      
    Serial.println(F("send: D3"));
    Serial1.print("D3\r\n");
    windsonicReceiveTerminatedMessage('\r');
    delay(500);
    windsonicFlush();

    // set Communications protocol RS232
    Serial.println(F("send: E3"));
    Serial1.print("E3\r\n");
    windsonicReceiveTerminatedMessage('\r');
    delay(2000);
    windsonicFlush();

    // set Baud rate 9600
    Serial.println(F("send: B3"));
    Serial1.print("B3\r\n");
    delay(1000);
    windsonicFlush();
    /*
    Serial1.end();
    Serial1.begin(GWS_SERIAL_BAUD);
    delay(1000);
    Serial.println(F("send: B at 9600"));
    Serial1.print("B\r\n");
    delay(1000);
    windsonicFlush();
    */
    
    Serial.println(F("send: M2"));
    Serial1.print("M2\r\n");
    windsonicReceiveTerminatedMessage('\r');
    delay(500);
    windsonicFlush();
    
    Serial.println("send:Q");
    Serial1.print("Q\r\n");
    Serial1.flush();
    delay(1000);
    windsonicFlush();

    Serial1.end();
    Serial1.begin(GWS_SERIAL_BAUD);

    windsonicPowerOff();
    delay(1000);
    windsonicPowerOn();
    delay(WIND_POWER_ON_DELAY_MS);
    windsonicSerialReset();

    
    if (!windsonicEnterConfigMode()){
      Serial.println("failed");
    } else { 
      return true;
    }
  }
    
  windsonicPowerOff();
  delay(1000);
  windsonicPowerOn();
  delay(WIND_POWER_ON_DELAY_MS);
  windsonicSerialReset();
  Serial.println("failed enter safe mode");
  return false;

}

void windsonicConfigure(void){
  // configure windsonic starting by any unknow settings
  // enter in safe mode to change comunication port
  // change RS232 baudrate
  // enter in config mode and set all parameters
  
  Serial.println(F("go to configure Windsonic"));

  while (!windsonicInitSafeMode()) {
    if (windsonicEnterConfigModeAllBaudrate()) break;
  }
  
  Serial.println(F("send: L1"));
  Serial1.print("L1\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: C2"));
  Serial1.print("C2\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: H2"));
  Serial1.print("H2\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: K50"));
  Serial1.print("K50\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: M4"));
  Serial1.print("M4\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
    
  Serial.println(F("send: NQ"));
  Serial1.print("NQ\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: O1"));
  Serial1.print("O1\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: U1"));
  Serial1.print("U1\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
    
  Serial.println(F("send: Y1"));
  Serial1.print("Y1\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: P2"));
  Serial1.print("P2\r\n");   // 2 per second;  required by i2c-wind setted for 1 measure by sec.
  windsonicReceiveMessage('\r');
  delay(500);
  windsonicFlush();

  Serial.println(F("exit configure mode"));
  Serial1.print("Q\r\n");
  delay(10);
  Serial1.print("Q\r\n");
  windsonicFlush();
  delay(1000);
  windsonicFlush();
}

void windsonicSconfigure(void){
  // unset any usefull settings on windsonic
  
  Serial.println(F("go to sconfigure Windsonic"));

  while (!windsonicEnterConfigModeAllBaudrate()) {
    windsonicInitSafeMode();
  }
  Serial.println(F("send: M4"));
  Serial1.print("M4\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: E2"));
  Serial1.print("E2\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  
  Serial.println(F("send: B2"));
  Serial1.print("B2\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();
  Serial1.end();
  Serial1.begin(4800);
  Serial1.print("B\r\n");
  delay(500);
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();

  Serial.println(F("send: M2"));
  Serial1.print("M2\r\n");
  windsonicReceiveTerminatedMessage('\r');
  delay(500);
  windsonicFlush();

  Serial.println(F("exit configure mode"));
  Serial1.print("Q\r\n");
  delay(10);
  Serial1.print("Q\r\n");
  windsonicFlush();
  delay(1000);
  windsonicFlush();
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
  Serial.println(F("\tu = windsonic sconfigurator ! (use to broke your sensor!)"));
  Serial.println(F("\tv = windsonic setup"));
  Serial.println(F("\tz = windsonic transparent mode"));
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

	float new_value= -1;
	while (new_value < 0. || new_value > 32767){
	  Serial.print(F("digit new value for ADC calibration offset(0/32767) (default 0): "));
	  new_value=Serial.parseFloat();
	  Serial.println(new_value);
	}
	delay(1000);

	Wire.beginTransmission(I2C_SOLAR_RADIATION_DEFAULT_ADDRESS);
	buffer[0]=I2C_SOLAR_RADIATION_ADC_CALIBRATION_OFFSET_ADDRESS;
	memcpy(&new_value, &buffer[1], sizeof(new_value));
	buffer[I2C_SOLAR_RADIATION_ADC_CALIBRATION_OFFSET_LENGTH+1]=crc8(buffer, I2C_SOLAR_RADIATION_ADC_CALIBRATION_OFFSET_LENGTH+1);
	Wire.write(buffer,I2C_SOLAR_RADIATION_ADC_CALIBRATION_OFFSET_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);



	new_value= -32768;
	while (new_value < -32767. || new_value > 32767.){
	  Serial.print(F("digit new value for ADC calibration gain(-32767./32767.) (default 1.0): "));
	  new_value=Serial.parseFloat();
	  Serial.println(new_value);
	}
	delay(1000);

	Wire.beginTransmission(I2C_SOLAR_RADIATION_DEFAULT_ADDRESS);
	buffer[0]=I2C_SOLAR_RADIATION_ADC_CALIBRATION_GAIN_ADDRESS;
	memcpy(&new_value, &buffer[1], sizeof(new_value));
	buffer[I2C_SOLAR_RADIATION_ADC_CALIBRATION_GAIN_LENGTH+1]=crc8(buffer, I2C_SOLAR_RADIATION_ADC_CALIBRATION_GAIN_LENGTH+1);
	Wire.write(buffer,I2C_SOLAR_RADIATION_ADC_CALIBRATION_GAIN_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);


	new_value= -1;
	while (new_value < 1. || new_value > 5000.){
	  Serial.print(F("digit new value for sensor voltage max(1./5000.) (default 2000.): "));
	  new_value=Serial.parseFloat();
	  Serial.println(new_value);
	}
	delay(1000);

	Wire.beginTransmission(I2C_SOLAR_RADIATION_DEFAULT_ADDRESS);
	buffer[0]=I2C_SOLAR_RADIATION_SENSOR_VOLTAGE_MAX_ADDRESS;
	memcpy(&new_value, &buffer[1], sizeof(new_value));
	buffer[I2C_SOLAR_RADIATION_SENSOR_VOLTAGE_MAX_LENGTH+1]=crc8(buffer, I2C_SOLAR_RADIATION_SENSOR_VOLTAGE_MAX_LENGTH+1);
	Wire.write(buffer,I2C_SOLAR_RADIATION_SENSOR_VOLTAGE_MAX_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);


	new_value= -1;
	while (new_value < 1. || new_value > 3000.){
	  Serial.print(F("digit new value for sensor radiation max(1./3000.) (default 2000.): "));
	  new_value=Serial.parseFloat();
	  Serial.println(new_value);
	}
	delay(1000);

	Wire.beginTransmission(I2C_SOLAR_RADIATION_DEFAULT_ADDRESS);
	buffer[0]=I2C_SOLAR_RADIATION_SENSOR_RADIATION_MAX_ADDRESS;
	memcpy(&new_value, &buffer[1], sizeof(new_value));
	buffer[I2C_SOLAR_RADIATION_SENSOR_RADIATION_MAX_LENGTH+1]=crc8(buffer, I2C_SOLAR_RADIATION_SENSOR_RADIATION_MAX_LENGTH+1);
	Wire.write(buffer,I2C_SOLAR_RADIATION_SENSOR_RADIATION_MAX_LENGTH+2);
	if (Wire.endTransmission() != 0) Serial.println(F("Wire Error"));             // End Write Transmission

	delay(1000);

	
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
	while (new_type.length() != 4) {
	  new_type = Serial.readStringUntil('\n');
	}

	Serial.println(new_type);
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
	new_type="";
	while (new_type.length() != 4) {
	  new_type = Serial.readStringUntil('\n');
	}

	Serial.println(new_type);
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

    case 'u':
      {
	Serial1.begin(GWS_SERIAL_BAUD);
	Serial1.setTimeout(500);
	Serial.setTimeout(500);
	Serial.println("Windsonic sconfiguration");
	pinMode(WIND_POWER_PIN, OUTPUT);
	windsonicPowerOff();
	delay(1000);
	windsonicPowerOn();
	delay(WIND_POWER_ON_DELAY_MS);
	Serial.println("windsonic ON");

	windsonicSerialReset();
	windsonicSconfigure();

	while(true){
	  delay(1000);
	  windsonicFlush();
	  Serial1.print("?Q!\r\n");
	  windsonicReceiveMessage('\r');
	  Serial.println(uart_rx_buffer);
	}
	  
	break;
      }

    case 'v':
      {


	Serial1.begin(GWS_SERIAL_BAUD);
	Serial1.setTimeout(500);
	Serial.setTimeout(500);
	Serial.println("Windsonic configuration");
	pinMode(WIND_POWER_PIN, OUTPUT);
	windsonicPowerOff();
	delay(1000);
	windsonicPowerOn();
	delay(WIND_POWER_ON_DELAY_MS);
	Serial.println("windsonic ON");
	windsonicSerialReset();
	
	windsonicConfigure();

	while(true){
	  delay(1000);
	  windsonicFlush();
	  Serial1.print("?Q!\n");
	  windsonicReceiveMessage('\n');
	  Serial.println(uart_rx_buffer);
	}
	  
	break;
      }
      
    case 'z':
      {

	long int baudrate = -1;
	while (baudrate < 2400 || baudrate > 38400){
	  Serial.print(F("digit new i2c address for i2c-rain (244-38400) default: "));
	  Serial.println(GWS_SERIAL_BAUD);
	  baudrate=Serial.parseInt();
	  Serial.println(baudrate);
	}
	
	Serial1.begin(baudrate);
	Serial1.setTimeout(500);
	Serial.setTimeout(500);
	Serial.println("transparent mode with windsonic at:");
	Serial.println(baudrate);
	
	pinMode(WIND_POWER_PIN, OUTPUT);
	windsonicPowerOff();
	delay(1000);
	windsonicPowerOn();
	//delay(WIND_POWER_ON_DELAY_MS);
	Serial.println("windsonic ON");
 	
	while (true){
	  while (Serial1.available() > 0){
	    Serial.write((uint8_t)Serial1.read());
	  }
	  Serial.flush();
	  while (Serial.available() > 0){
	    Serial1.write((uint8_t)Serial.read());
	  }
	  Serial1.flush();
	}

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
