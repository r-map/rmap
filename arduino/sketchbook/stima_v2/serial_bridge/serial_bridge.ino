/*
AUTHOR: Hazim Bitar (techbitar)
DATE: Aug 29, 2013
LICENSE: Public domain (use at your own risk)
CONTACT: techbitar at gmail dot com (techbitar.com)

http://www.instructables.com/id/Modify-The-HC-05-Bluetooth-Module-Defaults-Using-A/?ALLSTEPS


How to connect:

with arduino mega you can use Serial1
with other board you have to try SoftwareSerial

with arduino mega:
TXD -> RX1
RXD -> TX1 

ON HOST SIDE USE COMMAND:
ino serial  -- --omap crcrlf

*/

//#include <SoftwareSerial.h>
//SoftwareSerial BTSerial(10, 11); // RX | TX
//SoftwareSerial SSerial(10, 11); // RX | TX

//#define MYSERIAL SSerial
#define MYSERIAL Serial1

void setup()
{
  Serial.begin(9600);
  delay(1000);
  Serial.println("Started bridge mode from HSerial and MYSERIAL");
  MYSERIAL.begin(9600);
  delay(1000);
}

void loop()
{
  // Keep reading from HC-05 and send to Arduino Serial Monitor
  while (MYSERIAL.available()>0)
    Serial.write(MYSERIAL.read());

  // Keep reading from Arduino Serial Monitor and send to HC-05
  while (Serial.available()>0)
    MYSERIAL.write(Serial.read());
}
