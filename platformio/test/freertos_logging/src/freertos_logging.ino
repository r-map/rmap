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


void printTimestamp(Print* _logOutput) {
  char c[12];
  sprintf(c, "%10lu ", millis());
  _logOutput->print(c);
}

void printNewline(Print* _logOutput) {
  _logOutput->print('\n');
}


using namespace cpp_freertos;

class sampleThread : public Thread {
  
public:
  
  sampleThread(int i, int delayInSeconds)
    : Thread("Thread One", 200, 1), 
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
      frtosLog.notice("message from thread %d", Id);
    }
  };

private:
  int Id;
  int DelayInSeconds;
};


void setup (void)
{

  MutexStandard loggingmutex;
  
  // start up the serial interface
  Serial.begin(115200);
  frtosLog.begin(LOG_LEVEL_VERBOSE, &Serial,loggingmutex);
  frtosLog.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
  frtosLog.setSuffix(printNewline); // Uncomment to get newline as suffix

  //Start logging

  frtosLog.notice(F("Testing FreeRTOS C++ wrappers with logger"));                     // Info string with Newline
  
  static sampleThread p1(1, 2);
  static sampleThread p2(2, 5);
  
  Thread::StartScheduler();
  
}

void loop()
{
  // Empty. Things are done in Tasks.
}

