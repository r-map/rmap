/*
 * TimeGPS.pde
 * example code illustrating time synced from a I2C GPS
 * 
 */
#include <Wire.h>
#include <Time.h>
#include <GPSRTC.h>       //http://arduiniana.org/libraries/TinyGPS/

uint32_t  prevDisplay;


void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  
  Serial.println("started");

  delay(1000);

  setSyncProvider(GpsRtc.get);
  digitalClockDisplay();  

}

void loop()
{
  if(timeStatus()!= timeNotSet) 
  {
     if(now() != prevDisplay) //update the display only if the time has changed
     {
       prevDisplay = now();
       digitalClockDisplay();  
     }
  }	 
}

