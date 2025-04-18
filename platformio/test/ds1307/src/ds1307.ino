/****************************************************************************
 *
 *  Copyright (c) 2020, Paolo Patruno (p.patruno@iperbole.bologna.it)
 *
 ***************************************************************************/

#include <ArduinoLog.h>
#include <DS1307RTC.h>
#include <Wire.h>

// Length of datetime string %04u-%02u-%02uT%02u:%02u:%02u
#define DATE_TIME_STRING_LENGTH                       (25+30)

void printTimestamp(Print* _logOutput) {
  char c[12];
  sprintf(c, "%10lu ", millis());
  _logOutput->print(c);
}

void printNewline(Print* _logOutput) {
  _logOutput->print('\n');
}

void setup (void)
{
  
  // start up the serial interface
  Serial.begin(115200);
  //Start logging
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
  Log.setSuffix(printNewline); // Uncomment to get newline as suffix
  Log.notice(F("STARTED"));

  setTime(12,0,0,1,4,2025);

  Wire.begin();

  if (!Wire.setClock(25000)){
    Log.error("Setting i2c clock");
  }
  Log.notice("i2c clock %l",Wire.getClock());
  //Wire.setTimeOut(1000);
  
  delay(5000);

   if (!RTC.set(now())) Log.error("Setting RTC time from system time");
   Log.notice("Is running %T",RTC.isRunning());  
}

void loop()
{
  delay(1000);

 
  char dt[DATE_TIME_STRING_LENGTH];
  time_t time=0;

  /*
  time=now();
  snprintf(dt, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u",
	   year(time), month(time), day(time), hour(time), minute(time), second(time));
  Log.notice("Get system time %s",dt);
  */

  time=RTC.get();
  snprintf(dt, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u",
	   year(time), month(time), day(time), hour(time), minute(time), second(time));
  Log.notice("Get RTC time    %s",dt);

}
