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

#define I2C_MY_ADDRESS 10
#define I2C_OTHER_ADDRESS 11

// set the I2C clock frequency 
#define I2C_CLOCK 30418

//#define  I2C_HOW_MANY_BUSY_CHECKS_AFTER_STOP  10

#define MAX_SENT_BYTES     BUFFER_LENGTH   //maximum amount of data that I could receive from a master device 

/*
extern "C" {
    #include "utility/twi.h"
}
*/
// logging level at compile time
// Available levels are:
// LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
#define LOG_LEVEL   LOG_LEVEL_ERROR
#include <ArduinoLog.h>
#include "Wire.h"


volatile static uint8_t         receivedData[MAX_SENT_BYTES];
volatile static uint8_t         new_data=0;                        //new data received 

uint8_t errorcount=0;

/**
 * This routine turns off the I2C bus and clears it
 * on return SCA and SCL pins are tri-state inputs.
 * You need to call Wire.begin() after this to re-enable I2C
 * This routine does NOT use the Wire library at all.
 *
 * returns 0 if bus cleared
 *         1 if SCL held low.
 *         2 if SDA held low by slave clock stretch for > 2sec
 *         3 if SDA held low after 20 clocks.
 */
int I2C_ClearBus() {
#if defined(TWCR) && defined(TWEN)
  TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif

  pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
  pinMode(SCL, INPUT_PULLUP);

  boolean SCL_LOW = (digitalRead(SCL) == LOW); // Check is SCL is Low.
  if (SCL_LOW) { //If it is held low Arduno cannot become the I2C master. 
    return 1; //I2C bus error. Could not clear SCL clock line held low
  }

  boolean SDA_LOW = (digitalRead(SDA) == LOW);  // vi. Check SDA input.
  int clockCount = 20; // > 2x9 clock

  while (SDA_LOW && (clockCount > 0)) { //  vii. If SDA is Low,
    clockCount--;
  // Note: I2C bus is open collector so do NOT drive SCL or SDA high.
    pinMode(SCL, INPUT); // release SCL pullup so that when made output it will be LOW
    pinMode(SCL, OUTPUT); // then clock SCL Low
    delayMicroseconds(10); //  for >5uS
    pinMode(SCL, INPUT); // release SCL LOW
    pinMode(SCL, INPUT_PULLUP); // turn on pullup resistors again
    // do not force high as slave may be holding it low for clock stretching.
    delayMicroseconds(10); //  for >5uS
    // The >5uS is so that even the slowest I2C devices are handled.
    SCL_LOW = (digitalRead(SCL) == LOW); // Check if SCL is Low.
    int counter = 20;
    while (SCL_LOW && (counter > 0)) {  //  loop waiting for SCL to become High only wait 2sec.
      counter--;
      delay(100);
      SCL_LOW = (digitalRead(SCL) == LOW);
    }
    if (SCL_LOW) { // still low after 2 sec error
      return 2; // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
    }
    SDA_LOW = (digitalRead(SDA) == LOW); //   and check SDA input again and loop
  }
  if (SDA_LOW) { // still low
    return 3; // I2C bus error. Could not clear. SDA data line held low
  }

  // else pull SDA line low for Start or Repeated Start
  pinMode(SDA, INPUT); // remove pullup.
  pinMode(SDA, OUTPUT);  // and then make it LOW i.e. send an I2C Start or Repeated start control.
  // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
  /// A Repeat Start is a Start occurring after a Start with no intervening Stop.
  delayMicroseconds(10); // wait >5uS
  pinMode(SDA, INPUT); // remove output low
  pinMode(SDA, INPUT_PULLUP); // and make SDA high i.e. send I2C STOP control.
  delayMicroseconds(10); // x. wait >5uS
  pinMode(SDA, INPUT); // and reset pins as tri-state inputs which is the default state on reset
  pinMode(SCL, INPUT);
  return 0; // all ok
}


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
  Wire.write(61);
  Wire.write(62);
  Wire.write(63);
  Wire.write(64);
  Wire.write(65);
  Wire.write(66);
  Wire.write(67);
  Wire.write(68);
  Wire.write(69);
  Wire.write(70);
  //Write up to 32 byte, since master is responsible for reading and sending NACK
  //32 byte limit is in the Wire library, we have to live with it unless writing our own wire library
}

//Handler for receiving data
void receiveEvent( int bytesReceived)
{
  //LOGN("receive event, bytes: %d" CR,bytesReceived);
  new_data=bytesReceived;
  for (int a = 0; a < bytesReceived; a++) {
    if (a < MAX_SENT_BYTES) {
      receivedData[a] = Wire.read();
      //LOGN("received: %X" CR, receivedCommands[a]);
      //    } else {
      //Wire.read();  // if we receive more data then allowed just throw it away
    }
  }
  if ( new_data > MAX_SENT_BYTES) new_data = MAX_SENT_BYTES;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

//Check for new incoming command on I2C
void received_data(){
  unsigned long startt=millis();
  unsigned long delms=random(5000);
  LOGT("wait: %d ms" CR, delms);
  while (new_data == 0 && (millis()-startt) < delms) delay(1);
  LOGN("Received data: %d" CR, new_data);
  noInterrupts();
  for (int a = 0; a < new_data; a++) {
    LOGN("received: %d" CR, receivedData[a]);
  }
  new_data=0;
  interrupts();
}

void send_data(){
  LOGN(F("Send Data" CR));

  digitalWrite(3,HIGH);

  Wire.beginTransmission(I2C_OTHER_ADDRESS);
  Wire.write(1);
  Wire.write(2);
  Wire.write(3);
  Wire.write(4);
  Wire.write(5);
  Wire.write(6);
  Wire.write(7);
  Wire.write(8);
  Wire.write(9);
  Wire.write(10);
  Wire.write(11);
  Wire.write(12);
  Wire.write(13);
  Wire.write(14);
  Wire.write(15);
  Wire.write(16);
  Wire.write(17);
  Wire.write(18);
  Wire.write(19);
  Wire.write(20);
  Wire.write(21);
  Wire.write(22);
  Wire.write(23);
  Wire.write(24);
  Wire.write(25);
  Wire.write(26);
  Wire.write(27);
  Wire.write(28);
  Wire.write(29);
  Wire.write(30);
  Wire.write(31);
  Wire.write(32);

  uint8_t status=Wire.endTransmission(); // End Write Transmission 
  if (status != 0){
    LOGE(F("Wire error %d" CR),status);
    errorcount++;
    /*
      twi_stop();
      delay(10);
      pinMode(SDA, INPUT_PULLUP);
      for (int i = 0; i < 8; i++) {
      pinMode(SCL, OUTPUT);
      digitalWrite(SCL, LOW);
      delayMicroseconds(6);
      pinMode(SCL, INPUT_PULLUP);
      delayMicroseconds(6);
      }
      twi_init();
      twi_setAddress(I2C_MY_ADDRESS);
      twi_setFrequency(I2C_CLOCK);
    */

    if (errorcount >10 ){
      errorcount=0;
      //http://www.forward.com.au/pfod/ArduinoProgramming/I2C_ClearBus/index.html
      int rtn = I2C_ClearBus(); // clear the I2C bus first before calling Wire.begin()
      if (rtn != 0) {
	LOGE(F("I2C bus error. Could not clear" CR));
	if (rtn == 1) {
	  LOGE(F("SCL clock line held low" CR));
	} else if (rtn == 2) {
	  LOGE(F("SCL clock line held low by slave clock stretch" CR));
	} else if (rtn == 3) {
	  LOGE(F("SDA data line held low" CR));
	}
      } else { // bus clear
	// re-enable Wire
	// now can start Wire Arduino master
	
	Wire.begin(I2C_MY_ADDRESS);
	//set the i2c clock 
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
	
      }
    }
    digitalWrite(3,LOW);
  }
}

void request_data(){
  LOGN(F("Request Data" CR));
  Wire.requestFrom(I2C_OTHER_ADDRESS,20);
  uint8_t a=1;
  while  (Wire.available()>0) {   // slave may send less than requested
    LOGN(F("requested %d: %d" CR),a++,Wire.read());
  }
}

void setup() {
  
  randomSeed(analogRead(0));
  Serial.begin(115200);        // connect to the serial port

  // Pass log level, whether to show log level, and print interface.
  // Available levels are:
  // LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
  // Note: if you want to fully remove all logging code, change #define LOG_LEVEL ....
  //       this will significantly reduce your project size

  // set runtime log level to the same of compile time
  Log.begin(LOG_LEVEL, &Serial);

  LOGF(F("Start i2c-multimaster firmware version: %d" CR),VERSION);

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

  received_data();
  send_data();
  request_data();
  
}
