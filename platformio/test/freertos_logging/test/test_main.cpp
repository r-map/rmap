#include <unity.h>
#include <Arduino.h> 
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
#include "ByteBuffer.h"
#include "BufferStream.h"

/*
This test check various output using logging
Logging have a own semaphore protecting stream for output
There are 4 thread concurrent logging on output on
different delay iside a window of ~10 sec
The tests are protected inside "testsemaphore" to
secure the rigth sequence for write and read in the stream
A string stream is used here to check logging in and out 
*/

#ifndef ARDUINO_ARCH_AVR    
int          intValue1  , intValue2;
long         longValue1, longValue2;
bool         boolValue1, boolValue2;
const char * charArray    = "this is a string";
String       stringValue1 = "this is a string";
float        floatValue;
double       doubleValue;
#endif

ByteBuffer bytebuffer;
BufferStream  bufferstream(bytebuffer);

MutexStandard loggingsemaphore;
#ifndef ARDUINO_ARCH_AVR    
MutexStandard testsemaphore;
#endif

#ifdef ARDUINO_ARCH_AVR
// this compute the the difference fron heap and stack
// ehwn heap and stack overload an crash happen
int freeRam ()
{
  //DBGSERIAL.println(__malloc_margin);

  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
#endif

void test0(){
#ifndef ARDUINO_ARCH_AVR    
  LockGuard guard(testsemaphore);
#endif
#ifdef ARDUINO_ARCH_AVR
  Serial.print(F("#free ram on test0: "));
  Serial.println(freeRam());
#endif
  frtosLog.notice(F("Logging example"));                       // Info string in flash memory
  TEST_ASSERT_EQUAL_STRING("#N: Logging example\n" ,bufferstream.readString().c_str());
}

#ifndef ARDUINO_ARCH_AVR
void test1(){
  LockGuard guard(testsemaphore);
#ifdef ARDUINO_ARCH_AVR
  Serial.print(F("#free ram on test1: "));
  Serial.println(freeRam());
#endif
  frtosLog.notice   (F("Log as Info with integer values : %d, %d")  , intValue1,  intValue2);
  TEST_ASSERT_EQUAL_STRING(F("#N: Log as Info with integer values : 100, 10000\n"),bufferstream.readString().c_str());
}
void test2(){
  LockGuard guard(testsemaphore);
  frtosLog.notice   (F("Log as Info with hex values     : %x, %X"                  ), intValue1,  intValue1);
  TEST_ASSERT_EQUAL_STRING(F("#N: Log as Info with hex values     : 64, 0x64\n"),bufferstream.readString().c_str());
}
void test3(){
  LockGuard guard(testsemaphore);
  frtosLog.notice   (F("Log as Info with hex values     : %x, %X")                   , intValue2,  intValue2);
  TEST_ASSERT_EQUAL_STRING(F("#N: Log as Info with hex values     : 2710, 0x2710\n"),bufferstream.readString().c_str());
}
void test4(){
  LockGuard guard(testsemaphore);
  frtosLog.notice   (F("Log as Info with binary values  : %b, %B"                  ), intValue1,  intValue1);
  TEST_ASSERT_EQUAL_STRING(F("#N: Log as Info with binary values  : 1100100, 0b1100100\n"),bufferstream.readString().c_str());
}
void test5(){
  LockGuard guard(testsemaphore);
  frtosLog.notice   (F("Log as Info with binary values  : %b, %B")                   , intValue2,  intValue2);
  TEST_ASSERT_EQUAL_STRING(F("#N: Log as Info with binary values  : 10011100010000, 0b10011100010000\n"),bufferstream.readString().c_str());
}
void test6(){
  LockGuard guard(testsemaphore);
  frtosLog.notice   (F("Log as Info with long values    : %l, %l"                  ), longValue1, longValue2);
  TEST_ASSERT_EQUAL_STRING(F("#N: Log as Info with long values    : 1000000, 100000000\n"),bufferstream.readString().c_str());
}
void test7(){
  LockGuard guard(testsemaphore);
  frtosLog.notice   (F("Log as Info with bool values    : %t, %T")                   , boolValue1, boolValue2);
  TEST_ASSERT_EQUAL_STRING(F("#N: Log as Info with bool values    : T, false\n"),bufferstream.readString().c_str());
}
void test8(){
  LockGuard guard(testsemaphore);
  frtosLog.notice   (F("Log as Info with string value   : %s"                      ), charArray);
  TEST_ASSERT_EQUAL_STRING(F("#N: Log as Info with string value   : this is a string\n"),bufferstream.readString().c_str());
}
void test9(){
  LockGuard guard(testsemaphore);
  frtosLog.notice   (  "Log as Info with string value   : %s"                       , stringValue1.c_str());
  TEST_ASSERT_EQUAL_STRING("#N: Log as Info with string value   : this is a string\n",bufferstream.readString().c_str());
}
void test10(){
  LockGuard guard(testsemaphore);
  frtosLog.notice   (F("Log as Info with float value   : %F"                       ), floatValue);
  TEST_ASSERT_EQUAL_STRING(F("#N: Log as Info with float value   : 12.34\n"),bufferstream.readString().c_str());
}
void test11(){
  LockGuard guard(testsemaphore);
  frtosLog.notice   (F("Log as Info with float value   : %F")                        , floatValue);
  TEST_ASSERT_EQUAL_STRING(F("#N: Log as Info with float value   : 12.34\n"),bufferstream.readString().c_str());
}
void test12(){
  LockGuard guard(testsemaphore);
  frtosLog.notice   (F("Log as Info with double value   : %D"                      ), doubleValue);
  TEST_ASSERT_EQUAL_STRING(F("#N: Log as Info with double value   : 1234.57\n"),bufferstream.readString().c_str());
}
void test13(){
  LockGuard guard(testsemaphore);
  frtosLog.notice   (F("Log as Info with double value   : %D")                       , doubleValue);
  TEST_ASSERT_EQUAL_STRING("#N: Log as Info with double value   : 1234.57\n",bufferstream.readString().c_str());
}
void test14(){
  LockGuard guard(testsemaphore);
  frtosLog.notice   (F("Log as Debug with mixed values  : %d, %d, %l, %l, %t, %T"  ), intValue1 , intValue2,
				longValue1, longValue2, boolValue1, boolValue2);
  TEST_ASSERT_EQUAL_STRING("#N: Log as Debug with mixed values  : 100, 10000, 1000000, 100000000, T, false\n",bufferstream.readString().c_str());
}
void test15(){
  LockGuard guard(testsemaphore);
  frtosLog.trace    (F("Log as Trace with bool value    : %T")                       , boolValue1);
  TEST_ASSERT_EQUAL_STRING("#T: Log as Trace with bool value    : true\n",bufferstream.readString().c_str());
}
void test16(){
  LockGuard guard(testsemaphore);
  frtosLog.warning  (F("Log as Warning with bool value  : %T")                       , boolValue1);
  TEST_ASSERT_EQUAL_STRING(F("#W: Log as Warning with bool value  : true\n"),bufferstream.readString().c_str());
}
void test17(){
  LockGuard guard(testsemaphore);
  frtosLog.error    (F("Log as Error with bool value    : %T")                       , boolValue1);
  TEST_ASSERT_EQUAL_STRING(F("#E: Log as Error with bool value    : true\n"),bufferstream.readString().c_str());
}
void test18(){
  LockGuard guard(testsemaphore);
  frtosLog.fatal    (F("Log as Fatal with bool value    : %T")                       , boolValue1);
  TEST_ASSERT_EQUAL_STRING(F("#F: Log as Fatal with bool value    : true\n"),bufferstream.readString().c_str());
}
void test19(){
  LockGuard guard(testsemaphore);
  frtosLog.verbose  (F("Log as Verbose with bool value   : %T"  CR CR               ), boolValue2);
  TEST_ASSERT_EQUAL_STRING(F("#V: Log as Verbose with bool value   : false\n\n\n"),bufferstream.readString().c_str());
}
#endif

  //Serial.print("ind: ");
  //Serial.println(ind);
using namespace cpp_freertos;

void printTimestamp(Print* _logOutput) {
  char c[12];
  sprintf(c, "%10lu ", millis());
  _logOutput->print(c);
}
 
void printNewline(Print* _logOutput) {
  _logOutput->print('\n');
}


class sampleThread : public Thread {
  
public:
  
  sampleThread(int i, int delayInSeconds)
    : Thread("Thread One", 100, 1), 
      Id (i), 
      DelayInSeconds(delayInSeconds)
  {
            Start();
  };
  
protected:

  virtual void Run() {

#ifdef ARDUINO_ARCH_AVR
    Serial.print(F("#free ram on task run: "));
    Serial.println(freeRam());
#endif

    //frtosLog.notice("Starting Thread %d", Id);

#ifndef ARDUINO_ARCH_AVR    
    // set up some random variables
    intValue1  = 100;
    intValue2  = 10000;
    longValue1 = 1000000;
    longValue2 = 100000000;
    boolValue1 = true;
    boolValue2 = false;
    floatValue = 12.34;
    doubleValue= 1234.56789;
#endif    
    while (true) {

    Delay(Ticks::SecondsToTicks(DelayInSeconds));

    RUN_TEST(test0);
#ifndef ARDUINO_ARCH_AVR    
    RUN_TEST(test1);
    RUN_TEST(test2);
    RUN_TEST(test3);
    RUN_TEST(test4);
    RUN_TEST(test5);
    RUN_TEST(test6);
    RUN_TEST(test7);
    RUN_TEST(test8);
    RUN_TEST(test9);
    RUN_TEST(test10);
    RUN_TEST(test11);
    RUN_TEST(test12);
    RUN_TEST(test13);
    RUN_TEST(test14);
    RUN_TEST(test15);
    RUN_TEST(test16);
    RUN_TEST(test17);
    RUN_TEST(test18);
    RUN_TEST(test19);
#endif
    
    Delay(Ticks::SecondsToTicks(10-DelayInSeconds));

    UNITY_END(); // stop unit testing

    }
  };

private:
  int Id;
  int DelayInSeconds;
};

void setup (void)
{

  // Pass log level, whether to show log level, and print interface.
  // Available levels are:
  // LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_TRACE, LOG_LEVEL_VERBOSE
  // Note: if you want to fully remove all logging code, uncomment #define DISABLE_LOGGING in Logging.h
  //       this will significantly reduce your project size
  
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);
  
  UNITY_BEGIN();    // IMPORTANT LINE!

#ifdef ARDUINO_ARCH_AVR
  Serial.print(F("#free ram on setup: "));
  Serial.println(freeRam());
#endif

  //Start logging
#ifdef ARDUINO_ARCH_AVR    
  bytebuffer.init(60);
#else
  bytebuffer.init(120);
#endif
  frtosLog.begin(LOG_LEVEL_VERBOSE, &bufferstream,loggingsemaphore);
  //frtosLog.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
  frtosLog.setSuffix(printNewline); // Uncomment to get newline as suffix

  static sampleThread p1(1, 4);
#ifdef ARDUINO_ARCH_AVR
  Serial.print(F("#free ram on post thread 1: "));
  Serial.println(freeRam());
#endif

#ifndef ARDUINO_ARCH_AVR  
  static sampleThread p2(2, 4);
#ifdef ARDUINO_ARCH_AVR
  Serial.print(F("#free ram on post thread 2: "));
  Serial.println(freeRam());
#endif
  
  static sampleThread p3(3, 6);
#ifdef ARDUINO_ARCH_AVR
  Serial.print(F("#free ram on post thread 3: "));
  Serial.println(freeRam());
#endif
  
  static sampleThread p4(4, 6);
#ifdef ARDUINO_ARCH_AVR
  Serial.print(F("#free ram on post thread 4: "));
  Serial.println(freeRam());
#endif
#endif
  
  Thread::StartScheduler();
  
}

void loop()
{
  // Empty. Things are done in Tasks.
}

