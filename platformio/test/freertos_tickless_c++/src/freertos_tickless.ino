/****************************************************************************
 *
 *  Copyright (c) 2021, Paolo Patruno (p.patruno@iperbole.bologna.it)
 *
 ***************************************************************************/

#define LED LED_BUILTIN

#include "STM32FreeRTOS.h"
#include "task.h"
#include "thread.hpp"
#include "ticks.hpp"
#include "frtosLog.h"

MutexStandard loggingmutex;

using namespace cpp_freertos;

class sensorThread : public Thread {  
public:
  sensorThread(int i, int delayInSeconds)
    : Thread("Thread", 500, 1), 
      Id (i), 
      DelayInSeconds(delayInSeconds)
  {
    Start();
  };
  
protected:
  virtual void Run() {
    frtosLog.notice("Starting Thread %d",Id);
    while (true) {
      Delay(Ticks::SecondsToTicks(DelayInSeconds));
      frtosLog.notice(F("Free stack bytes : %d" ),uxTaskGetStackHighWaterMark( NULL ));
      digitalToggle(LED);
      }
  };

private:
  int Id;
  int DelayInSeconds;
};

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
  pinMode(LED, OUTPUT);   
  // start up the serial interface
  Serial.begin(115200);
  frtosLog.begin(LOG_LEVEL_VERBOSE, &Serial,loggingmutex);
  frtosLog.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
  frtosLog.setSuffix(printNewline); // Uncomment to get newline as suffix

  //Start logging
  Log.notice(F("Testing FreeRTOS tickless C++ wrappers"));                     // Info string with Newline
  static sensorThread p1(1, 3);

  Thread::StartScheduler();
  
}

void loop()
{
  // Empty. Things are done in Tasks.
}

