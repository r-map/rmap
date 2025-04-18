/****************************************************************************
 *
 *  Copyright (c) 2020, Paolo Patruno (p.patruno@iperbole.bologna.it)
 *
 ***************************************************************************/

#ifdef ARDUINO_ARCH_AVR
#include <ArduinoSTL.h>
#include <Arduino_FreeRTOS.h>
#else 
#ifdef ARDUINO_ARCH_STM32
#include "STM32FreeRTOS.h"
#else
#include "FreeRTOS.h"
#endif
#endif
#include "task.h"
#include "thread.hpp"
#include "ticks.hpp"
#include <frtosLog.h>
#include <frtosRtc.h>
#include <Wire.h>

// Length of datetime string %04u-%02u-%02uT%02u:%02u:%02u
#define DATE_TIME_STRING_LENGTH                       (25)

void printTimestamp(Print* _logOutput) {
  char c[12];
  sprintf(c, "%10lu ", millis());
  _logOutput->print(c);
}

void printNewline(Print* _logOutput) {
  _logOutput->print('\n');
}

frtosRtc frtosRTC = frtosRtc();
MutexStandard loggingmutex;
MutexStandard i2cmutex;

using namespace cpp_freertos;

class sampleThread : public Thread {
  
public:
  
  sampleThread(int i, int delayInSeconds)
    : Thread("Thread Sample", 10000, 1), 
      Id (i),
      DelayInSeconds(delayInSeconds)
  {
            Start();
  };
  
protected:

  virtual void Run() {
    
    frtosLog.notice("Starting Thread %d", Id);
    
    while (true) {
      Delay(Ticks::SecondsToTicks(DelayInSeconds));

      char dt[DATE_TIME_STRING_LENGTH];
      time_t time=0;

      time=frtosRTC.get();
      snprintf(dt, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u",
	       year(time), month(time), day(time), hour(time), minute(time), second(time));
      frtosLog.notice("message from thread %d Get frtosRTC time %s",Id,dt);

    }
  };

private:
  int Id;
  int DelayInSeconds;
};


void setup (void)
{
  
  // start up the serial interface
  Serial.begin(115200);
  frtosLog.begin(LOG_LEVEL_VERBOSE, &Serial,loggingmutex);
  frtosLog.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
  frtosLog.setSuffix(printNewline); // Uncomment to get newline as suffix

  setTime(12,0,0,1,4,2025);

  delay(5000);
  Wire.begin();
  if(!Wire.setClock(25000)){
    frtosLog.error("Setting i2c clock");
  }
  //Wire.setTimeOut(300);
  frtosLog.notice("i2c clock %l",Wire.getClock());
  
  frtosRTC.begin(RTC,i2cmutex);
  
  //Start logging
  frtosLog.notice(F("Testing FreeRTOS C++ wrappers with logger"));
  frtosLog.notice(F("VERSION 1"));

  if (!frtosRTC.set(now())){
    frtosLog.error("Setting frtosRTC time from system time");
  }
  
  static sampleThread p1(1, 2);
  static sampleThread p2(2, 5);
  
  //Thread::StartScheduler();
  
}

void loop()
{
  delay(1000);

  char dt[DATE_TIME_STRING_LENGTH];
  time_t time=0;

  time=now();
  snprintf(dt, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u",
	   year(time), month(time), day(time), hour(time), minute(time), second(time));
  frtosLog.notice("Get system time %s",dt);
  
  //frtosLog.notice(F("Testing FreeRTOS C++ wrappers with logger"));   
  // Empty. Things are done in Tasks.
}

