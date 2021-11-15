/*!
* This example sketch shows most of the features of the ArduinoLog library with SD card
*

USE_SDCARD
RAM:   [=         ]  10.4% (used 1700 bytes from 16384 bytes)
Flash: [==        ]  16.7% (used 21668 bytes from 130048 bytes)

 serial only logger
RAM:   [=         ]   6.1% (used 996 bytes from 16384 bytes)
Flash: [=         ]  12.7% (used 16500 bytes from 130048 bytes)

DISABLE_LOGGING
RAM:   [=         ]   5.8% (used 958 bytes from 16384 bytes)
Flash: [=         ]   8.8% (used 11426 bytes from 130048 bytes)
  
*/

//#define DISABLE_LOGGING
#define USE_SDCARD

#include <ArduinoLog.h>
#include <SPI.h>
#include <SdFat.h>
#include <StreamUtils.h>

#define SDCARD_CHIP_SELECT_PIN    7
#define SPI_SPEED                 SD_SCK_MHZ(4)
#define SDCARD_LOGGING_FILE_NAME  "test.log"
  // Pass log level, whether to show log level, and print interface.
  // Available levels are:
  // LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_TRACE, LOG_LEVEL_VERBOSE
  // Note: if you want to fully remove all logging code, uncomment #define DISABLE_LOGGING in Logging.h
  //       this will significantly reduce your project size

#define LOG_LEVEL LOG_LEVEL_VERBOSE

#ifdef USE_SDCARD  
/*!
\var SD
\brief SD-Card structure.
*/
SdFat SD;

/*!
\var logFile
\brief File for logging on SD-Card.
*/
File logFile;

/*!
\var loggingStream
\brief stream for logging on Serial and  SD-Card together.
*/
WriteLoggingStream loggingStream(logFile,Serial);
#endif

int          intValue1  , intValue2;
long         longValue1, longValue2;
bool         boolValue1, boolValue2;
const char * charArray    = "this is a string";
String       stringValue1 = "this is a string";
float        floatValue;
double       doubleValue;

// if you want use a second logger
// Logging SDLog = Logging();


void logPrefix(Print* _logOutput) {
  char m[12];
  sprintf(m, "%10lu ", millis());
  _logOutput->print("#");
  _logOutput->print(m);
  _logOutput->print(": ");
}

void logSuffix(Print* _logOutput) {
  _logOutput->print('\n');
  //_logOutput->flush();  // we use this to flush every log message
}


void setup() {
  // Set up serial port and wait until connected
  Serial.begin(115200);
  SPI.begin();
  pinMode(SDCARD_CHIP_SELECT_PIN, OUTPUT);
  digitalWrite(SDCARD_CHIP_SELECT_PIN, HIGH);

  while(!Serial && !Serial.available()){}
  randomSeed(analogRead(0));

#ifdef USE_SDCARD  
  Serial.println("\nInitializing SD card..." );
  if (!SD.begin(SDCARD_CHIP_SELECT_PIN,SPI_SPEED)){
    Serial.println   (F("initialization failed. Things to check:"));
    Serial.println   (F("* is a card inserted?"));
    Serial.println   (F("* is your wiring correct?"));
    Serial.println   (F("* did you change the chipSelect pin to match your shield or module?"));
  } else {
    Serial.println   (F("Wiring is correct and a card is present."));
    Serial.print   (F("The FAT type of the volume: "));
    Serial.println   (SD.vol()->fatType());
  }
  
  logFile= SD.open(SDCARD_LOGGING_FILE_NAME, O_RDWR | O_CREAT | O_APPEND);
  if (logFile) {
    logFile.seekEnd(0);
    Log.begin(LOG_LEVEL, &loggingStream);
  } else {
    Log.begin(LOG_LEVEL, &Serial);
  }
#else
  Log.begin(LOG_LEVEL, &Serial);
#endif
  
  Log.setPrefix(logPrefix);
  Log.setSuffix(logSuffix);


  //Start logging

  Log.notice(F("******************************************" ));                     // Info string
  Log.notice(  "***          Logging example" );                                    // Info string in flash memory
  Log.notice(F("******************************************" ));                     // Info string
  
}

void loop() {

    // set up some random variables
    intValue1  = random(100);
    intValue2  = random(10000);
    longValue1 = random(1000000);
    longValue2 = random(100000000);
    boolValue1 = random(2)==0;
    boolValue2 = random(2)==1;
    floatValue = 12.34;
    doubleValue= 1234.56789;

    Log.notice   (  "Log as Info with integer values           : %d, %d"                   , intValue1,  intValue2);
    Log.notice   (F("Log as Info with hex values               : %x, %X"                  ), intValue1,  intValue1);
    Log.notice   (  "Log as Info with hex values               : %x, %X"                   , intValue2,  intValue2);
    Log.notice   (F("Log as Info with binary values            : %b, %B"                  ), intValue1,  intValue1);
    Log.notice   (  "Log as Info with binary values            : %b, %B"                   , intValue2,  intValue2);
    Log.notice   (F("Log as Info with long values              : %l, %l"                  ), longValue1, longValue2);
    Log.notice   (  "Log as Info with bool values              : %t, %T"                   , boolValue1, boolValue2);
    Log.notice   (F("Log as Info with string value             : %s"                      ), charArray);
    Log.notice   (  "Log as Info with string value             : %s"                       , stringValue1.c_str());
    Log.notice   (F("Log as Info with float value              : %F"                       ), floatValue);
    Log.notice   (  "Log as Info with float value              : %F"                        , floatValue);
    Log.notice   (  "Log as Info with float value 1 decimal    : %1"                        , floatValue);
    Log.notice   (F("Log as Info with double value             : %D"                      ), doubleValue);
    Log.notice   (  "Log as Info with double value             : %D"                       , doubleValue);
    Log.notice   (  "Log as Info with double value 4 decimal   : %4"                       , doubleValue);
    Log.notice   (F("Log as Debug with mixed values            : %d, %d, %l, %l, %t, %T"  ), intValue1 , intValue2,
                longValue1, longValue2, boolValue1, boolValue2);
    Log.trace    (  "Log as Trace with bool value              : %T"                       , boolValue1);
    Log.warning  (  "Log as Warning with bool value            : %T"                       , boolValue1);
    Log.error    (  "Log as Error with bool value              : %T"                       , boolValue1);
    Log.fatal    (  "Log as Fatal with bool value              : %T"                       , boolValue1);
    Log.verbose  (F("Log as Verbose with bool value            : %T"   CR                 ), boolValue2);

    /*
    // if you use a second logger
    SDLog.notice   (  "Log as Info with integer values           : %d, %d"                   , intValue1,  intValue2);
    SDLog.notice   (F("Log as Info with hex values               : %x, %X"                  ), intValue1,  intValue1);
    SDLog.notice   (  "Log as Info with hex values               : %x, %X"                   , intValue2,  intValue2);
    SDLog.notice   (F("Log as Info with binary values            : %b, %B"                  ), intValue1,  intValue1);
    SDLog.notice   (  "Log as Info with binary values            : %b, %B"                   , intValue2,  intValue2);
    SDLog.notice   (F("Log as Info with long values              : %l, %l"                  ), longValue1, longValue2);
    SDLog.notice   (  "Log as Info with bool values              : %t, %T"                   , boolValue1, boolValue2);
    SDLog.notice   (F("Log as Info with string value             : %s"                      ), charArray);
    SDLog.notice   (  "Log as Info with string value             : %s"                       , stringValue1.c_str());
    SDLog.notice   (F("Log as Info with float value              : %F"                       ), floatValue);
    SDLog.notice   (  "Log as Info with float value              : %F"                        , floatValue);
    SDLog.notice   (  "Log as Info with float value 1 decimal    : %1"                        , floatValue);
    SDLog.notice   (F("Log as Info with double value             : %D"                      ), doubleValue);
    SDLog.notice   (  "Log as Info with double value             : %D"                       , doubleValue);
    SDLog.notice   (  "Log as Info with double value 4 decimal   : %4"                       , doubleValue);
    SDLog.notice   (F("Log as Debug with mixed values            : %d, %d, %l, %l, %t, %T"  ), intValue1 , intValue2,
                longValue1, longValue2, boolValue1, boolValue2);
    SDLog.trace    (  "Log as Trace with bool value              : %T"                       , boolValue1);
    SDLog.warning  (  "Log as Warning with bool value            : %T"                       , boolValue1);
    SDLog.error    (  "Log as Error with bool value              : %T"                       , boolValue1);
    SDLog.fatal    (  "Log as Fatal with bool value              : %T"                       , boolValue1);
    SDLog.verbose  (F("Log as Verbose with bool value            : %T"                      ), boolValue2);
    */
    
#ifdef USE_SDCARD  
    logFile.flush();
#endif
    
    delay(5000);

}
