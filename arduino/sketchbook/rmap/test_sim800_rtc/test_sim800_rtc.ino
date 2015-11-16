/*
Copyright (C) 2015  Paolo Paruno <p.patruno@iperbole.bologna.it>
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
*/

#define GSMAPN "ibox.tim.it"
#define GSMUSER ""
#define GSMPASSWORD ""

#include "sim800.h"
#include <Time.h>


//GSM Shield for Arduino
//Simple sketch to start a connection as client.

SIM800 s800;
char buf[200];

// utility function to debug
void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

// utility function to debug
void digitalClockDisplay(time_t t ){
  // digital clock display of the time

  Serial.print("#");

  if (t == 0UL){
    Serial.println("NULL");
  }
  else{	  
    printDigits(hour(t));
    Serial.print(":");
    printDigits(minute(t));
    Serial.print(":");
    printDigits(second(t));
    Serial.print(" ");
    printDigits(day(t));
    Serial.print(" ");
    printDigits(month(t));
    Serial.print(" ");
    Serial.print(year(t)); 
    Serial.println(); 
  }
}


void setup()
{

  //Serial connection.
  Serial.begin(9600);
  Serial.println("SIM800 Shield testing.");

  for (int i=0; i<10; i++){
    delay(5000);
    Serial.println("try to init sim800");

#ifdef HARDWARESERIAL
    if (s800.init( 7, 6)) break;
#else
    FOR THIS EXAMPLE YOU HAVE TO SET HARDWARESERIAL STATIC IN SIM800 LIBRARY
      //if (s800.init(&Serial1 , 7, 6)) break;
#endif

  }

  Serial.println("try to setup sim800");
  s800.setup();

  tmElements_t tm;

  tm.Year = y2kYearToTm(15);
  tm.Month = 4;
  tm.Day = 30;
  tm.Hour = 12;
  tm.Minute = 30;
  tm.Second = 59;

  s800.RTCset(makeTime(tm));

  setSyncProvider(s800.RTCget);   // the function to get the time from the RTC

  if(timeStatus()== timeNotSet) {
    Serial.println("#The time has never been set");
  }else{
    digitalClockDisplay(now());
  }


  // this depend on firmware version !!!
  //s800.stopNTP();
  //s800.startNTP(GSMAPN, GSMUSER, GSMPASSWORD);
  //s800.ATcommand("+CNTPCID=2",buf); 
  //s800.ATcommand("+CNTP=\"pool.ntp.org\"",buf); 

  //s800.ATcommand("+CNTP=\"pool.ntp.org\",8,1,2",buf); 
  //s800.ATcommand("+CNTP=\"193.204.114.232\",8,1,2",buf); 

};

void loop()
{  
  Serial.println("null loop");

  Serial.println("sim800 RTC time");
  digitalClockDisplay(s800.RTCget());

  Serial.println("system time");

  if (timeStatus() == timeNeedsSync) {
    Serial.print("#The time had been set but a sync attempt did not succeed");
    Serial.println(timeStatus());
  }

  if(timeStatus()== timeNotSet) {
    Serial.println("#The time has never been set");
  }else{
    digitalClockDisplay(now()); 
  }

  delay(10000);

};
