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
 * This program test multimaster I2C
 * 
**********************************************************************/

#define VERSION 01             //Software version for cross checking

#define PRIMO

#ifdef PRIMO
#define I2C_MY_ADDRESS 8
#define I2C_OTHER_ADDRESS 7
#else
#define I2C_MY_ADDRESS 7
#define I2C_OTHER_ADDRESS 8
#endif

// set the I2C clock frequency 
#define I2C_CLOCK 30418

#include "Wire.h"
#include <ArduinoLog.h>
#include <avr/sleep.h>

// logging level at compile time
// Available levels are:
// LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
#define LOG_LEVEL   LOG_LEVEL_VERBOSE

#define MAX_SENT_BYTES     0x0F   //maximum amount of data that I could receive from a master device 

volatile static uint8_t         receivedData[MAX_SENT_BYTES];
volatile static uint8_t         new_data=0;                        //new data received 

//////////////////////////////////////////////////////////////////////////////////////
// I2C handlers
// Handler for requesting data
//
void requestEvent()
{
  Wire.write(51);
  Wire.write(52);
  Wire.write(53);
  Wire.write(54);
  Wire.write(55);
  Wire.write(56);
  Wire.write(57);
  Wire.write(58);
  Wire.write(59);
  Wire.write(60);
  //Write up to 32 byte, since master is responsible for reading and sending NACK
  //32 byte limit is in the Wire library, we have to live with it unless writing our own wire library
}

//Handler for receiving data
void receiveEvent( int bytesReceived)
{
  //LOGN("receive event, bytes: %d" CR,bytesReceived);
  new_data=bytesReceived;
  if (new_data > bytesReceived) new_data=MAX_SENT_BYTES;
  for (int a = 0; a < bytesReceived; a++) {
    if (a < MAX_SENT_BYTES) {
      receivedData[a] = Wire.read();
      //LOGN("received: %X" CR, receivedCommands[a]);
    } else {
      Wire.read();  // if we receive more data then allowed just throw it away
    }
  }  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

//Check for new incoming command on I2C
void received_data(){
  if (new_data > 0) {
    LOGN("Received data" CR);
    for (int a = 0; a < new_data; a++) {
      LOGN("received: %X" CR, receivedData[a]);
    }
    new_data=0;
  }
}

void send_data(){
  LOGN(F("Send Data" CR));

  Wire.beginTransmission(I2C_OTHER_ADDRESS);
  Wire.write(0X01);
  Wire.write(0X02);
  Wire.write(0X03);
  Wire.write(0X04);
  Wire.write(0X05);
  Wire.write(0X04);
  Wire.write(0X03);
  Wire.write(0X02);
  Wire.write(0X01);
  uint8_t status=Wire.endTransmission(); // End Write Transmission 
  if (status != 0)   LOGE(F("Wire error %d" CR),status);
}

void request_data(){
  LOGN(F("Request Data" CR));
  Wire.requestFrom(I2C_OTHER_ADDRESS,4);
  uint8_t available=Wire.available();
  for  (int a=0; a < available; a++) {   // slave may send less than requested
    LOGN(F("requested %d %d" CR),a,Wire.read());
  }
}

void setup() {

  Serial.begin(115200);        // connect to the serial port

  // Pass log level, whether to show log level, and print interface.
  // Available levels are:
  // LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
  // Note: if you want to fully remove all logging code, change #define LOG_LEVEL ....
  //       this will significantly reduce your project size

  // set runtime log level to the same of compile time
  Log.begin(LOG_LEVEL, &Serial);

  LOGN(F("Start i2c-manager-firmware version: %d" CR),VERSION);

  //Start I2C communication routines
  Wire.begin(I2C_MY_ADDRESS);
  
  //set the i2c clock 
  //TWBR = ((F_CPU / I2C_CLOCK) - 16) / 2;
  //TWBR =255    //  30418,25 Hz  : minimum freq with prescaler set to 1 and CPU clock to 16MHz
  Wire.setClock(I2C_CLOCK);

  //The Wire library enables the internal pullup resistors for SDA and SCL.
  //You can turn them off after Wire.begin()
  // do not need this with patched Wire library
  //digitalWrite( SDA, LOW);
  //digitalWrite( SCL, LOW);

  digitalWrite( SDA, HIGH);
  digitalWrite( SCL, HIGH);

  Wire.onRequest(requestEvent);          // Set up event handlers
  Wire.onReceive(receiveEvent);

  LOGN(F("end setup" CR));

}

void loop() {

  //#ifdef PRIMO
  received_data();
  //#else
  send_data();
  request_data();
  //#endif
  
}
