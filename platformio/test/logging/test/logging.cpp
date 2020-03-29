#include <unity.h>
#include <ArduinoLog.h>
#include "StringStream.h"

/*!
* This example sketch shows most of the features of the ArduinoLog library
*
*/


int          intValue1  , intValue2;
long         longValue1, longValue2;
bool         boolValue1, boolValue2;
const char * charArray    = "this is a string";
String       stringValue1 = "this is a string";
float        floatValue;
double       doubleValue;

String s;
StringStream sstream(s);


void setup() {

  // Pass log level, whether to show log level, and print interface.
  // Available levels are:
  // LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_TRACE, LOG_LEVEL_VERBOSE
  // Note: if you want to fully remove all logging code, uncomment #define DISABLE_LOGGING in Logging.h
  //       this will significantly reduce your project size
  
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);
  
  UNITY_BEGIN();    // IMPORTANT LINE!
  
  //Start logging
  Log.begin(LOG_LEVEL_VERBOSE, &sstream);
  
}

void test0(){
  Log.notice(  "Logging example" CR);                       // Info string in flash memory
  TEST_ASSERT_EQUAL_STRING("#N: Logging example\n" ,sstream.readString().c_str());
}
 
void test1(){
  Log.notice   (  "Log as Info with integer values : %d, %d" CR , intValue1,  intValue2);
  TEST_ASSERT_EQUAL_STRING("#N: Log as Info with integer values : 100, 10000\n",sstream.readString().c_str());
}
void test2(){
  Log.notice   (F("Log as Info with hex values     : %x, %X" CR                 ), intValue1,  intValue1);
  TEST_ASSERT_EQUAL_STRING("#N: Log as Info with hex values     : 64, 0x64\n",sstream.readString().c_str());
}
void test3(){
  Log.notice   (  "Log as Info with hex values     : %x, %X" CR                  , intValue2,  intValue2);
  TEST_ASSERT_EQUAL_STRING("#N: Log as Info with hex values     : 2710, 0x2710\n",sstream.readString().c_str());
}
void test4(){
  Log.notice   (F("Log as Info with binary values  : %b, %B" CR                 ), intValue1,  intValue1);
  TEST_ASSERT_EQUAL_STRING("#N: Log as Info with binary values  : 1100100, 0b1100100\n",sstream.readString().c_str());
}
void test5(){
  Log.notice   (  "Log as Info with binary values  : %b, %B" CR                  , intValue2,  intValue2);
  TEST_ASSERT_EQUAL_STRING("#N: Log as Info with binary values  : 10011100010000, 0b10011100010000\n",sstream.readString().c_str());
}
void test6(){
  Log.notice   (F("Log as Info with long values    : %l, %l" CR                 ), longValue1, longValue2);
  TEST_ASSERT_EQUAL_STRING("#N: Log as Info with long values    : 1000000, 100000000\n",sstream.readString().c_str());
}
void test7(){
  Log.notice   (  "Log as Info with bool values    : %t, %T" CR                  , boolValue1, boolValue2);
  TEST_ASSERT_EQUAL_STRING("#N: Log as Info with bool values    : T, false\n",sstream.readString().c_str());
}
void test8(){
  Log.notice   (F("Log as Info with string value   : %s" CR                     ), charArray);
  TEST_ASSERT_EQUAL_STRING("#N: Log as Info with string value   : this is a string\n",sstream.readString().c_str());
}
void test9(){
  Log.notice   (  "Log as Info with string value   : %s" CR                      , stringValue1.c_str());
  TEST_ASSERT_EQUAL_STRING("#N: Log as Info with string value   : this is a string\n",sstream.readString().c_str());
}
void test10(){
  Log.notice   (F("Log as Info with float value   : %F" CR                      ), floatValue);
  TEST_ASSERT_EQUAL_STRING("#N: Log as Info with float value   : 12.34\n",sstream.readString().c_str());
}
void test11(){
  Log.notice   (  "Log as Info with float value   : %F" CR                       , floatValue);
  TEST_ASSERT_EQUAL_STRING("#N: Log as Info with float value   : 12.34\n",sstream.readString().c_str());
}
void test12(){
  Log.notice   (F("Log as Info with double value   : %D" CR                     ), doubleValue);
  TEST_ASSERT_EQUAL_STRING("#N: Log as Info with double value   : 1234.57\n",sstream.readString().c_str());
}
void test13(){
  Log.notice   (  "Log as Info with double value   : %D" CR                      , doubleValue);
  TEST_ASSERT_EQUAL_STRING("#N: Log as Info with double value   : 1234.57\n",sstream.readString().c_str());
}
void test14(){
  Log.notice   (F("Log as Debug with mixed values  : %d, %d, %l, %l, %t, %T" CR ), intValue1 , intValue2,
				longValue1, longValue2, boolValue1, boolValue2);
  TEST_ASSERT_EQUAL_STRING("#N: Log as Debug with mixed values  : 100, 10000, 1000000, 100000000, T, false\n",sstream.readString().c_str());
}
void test15(){
  Log.trace    (  "Log as Trace with bool value    : %T" CR                      , boolValue1);
  TEST_ASSERT_EQUAL_STRING("#T: Log as Trace with bool value    : true\n",sstream.readString().c_str());
}
void test16(){
  Log.warning  (  "Log as Warning with bool value  : %T" CR                      , boolValue1);
  TEST_ASSERT_EQUAL_STRING("#W: Log as Warning with bool value  : true\n",sstream.readString().c_str());
}
void test17(){
  Log.error    (  "Log as Error with bool value    : %T" CR                      , boolValue1);
  TEST_ASSERT_EQUAL_STRING("#E: Log as Error with bool value    : true\n",sstream.readString().c_str());
}
void test18(){
  Log.fatal    (  "Log as Fatal with bool value    : %T" CR                      , boolValue1);
  TEST_ASSERT_EQUAL_STRING("#F: Log as Fatal with bool value    : true\n",sstream.readString().c_str());
}
void test19(){
  Log.verbose  (F("Log as Verbose with bool value   : %T" CR CR CR               ), boolValue2);
  TEST_ASSERT_EQUAL_STRING("#V: Log as Verbose with bool value   : false\n\n\n",sstream.readString().c_str());
}


void loop() {

    // set up some random variables
    intValue1  = 100;
    intValue2  = 10000;
    longValue1 = 1000000;
    longValue2 = 100000000;
    boolValue1 = true;
    boolValue2 = false;
    floatValue = 12.34;
    doubleValue= 1234.56789;

    RUN_TEST(test0);
    sstream.flush();    
    RUN_TEST(test1);
    sstream.flush();    
    RUN_TEST(test2);
    sstream.flush();    
    RUN_TEST(test3);
    sstream.flush();    
    RUN_TEST(test4);
    sstream.flush();    
    RUN_TEST(test5);
    sstream.flush();    
    RUN_TEST(test6);
    sstream.flush();    
    RUN_TEST(test7);
    sstream.flush();    
    RUN_TEST(test8);
    sstream.flush();    
    RUN_TEST(test9);
    sstream.flush();    
    RUN_TEST(test10);
    sstream.flush();    
    RUN_TEST(test11);
    sstream.flush();    
    RUN_TEST(test12);
    sstream.flush();    
    RUN_TEST(test13);
    sstream.flush();    
    RUN_TEST(test14);
    sstream.flush();    
    RUN_TEST(test15);
    sstream.flush();    
    RUN_TEST(test16);
    sstream.flush();    
    RUN_TEST(test17);
    sstream.flush();    
    RUN_TEST(test18);
    sstream.flush();    
    RUN_TEST(test19);

    UNITY_END(); // stop unit testing

    delay(5000);

}

