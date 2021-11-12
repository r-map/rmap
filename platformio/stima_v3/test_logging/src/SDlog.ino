#include <ArduinoLog.h>
#include <SPI.h>
#include <SdFat.h>

#define SDCARD_CHIP_SELECT_PIN 7
#define SPI_SPEED SD_SCK_MHZ(4)

/*!
* This example sketch shows most of the features of the ArduinoLog library with SD card
*
*/

SdFat SD;
File logfile;

int          intValue1  , intValue2;
long         longValue1, longValue2;
bool         boolValue1, boolValue2;
const char * charArray    = "this is a string";
String       stringValue1 = "this is a string";
float        floatValue;
double       doubleValue;

Logging SDLog = Logging();

void setup() {
  // Set up serial port and wait until connected
  Serial.begin(9600);
  while(!Serial && !Serial.available()){}
  randomSeed(analogRead(0));
  // Pass log level, whether to show log level, and print interface.
  // Available levels are:
  // LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_TRACE, LOG_LEVEL_VERBOSE
  // Note: if you want to fully remove all logging code, uncomment #define DISABLE_LOGGING in Logging.h
  //       this will significantly reduce your project size
  
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  //Log.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
  Log.setSuffix(printNewline); // Uncomment to get newline as suffix
  SDLog.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
  SDLog.setSuffix(printNewline); // Uncomment to get newline as suffix
  
  //Start logging

  Log.notice(F("******************************************" ));                     // Info string
  Log.notice(  "***          Logging example                " );                       // Info string in flash memory
  Log.notice(F("******************************************" ));                     // Info string
  
  SPI.begin();
  pinMode(SDCARD_CHIP_SELECT_PIN, OUTPUT);
  digitalWrite(SDCARD_CHIP_SELECT_PIN, HIGH);
  Log.notice   ("\nInitializing SD card..." );
  
  if (!SD.begin(SDCARD_CHIP_SELECT_PIN,SPI_SPEED)){
    Log.notice   ("initialization failed. Things to check:" );
    Log.notice   ("* is a card inserted?" );
    Log.notice   ("* is your wiring correct?" );
    Log.notice   ("* did you change the chipSelect pin to match your shield or module?" );
    while (1);
    
  } else {
    Log.notice   ("Wiring is correct and a card is present." );
    Log.notice   ("The FAT type of the volume: %s" ,SD.vol()->fatType());
  }
  
  logfile=SD.open("mylog.log", O_RDWR | O_CREAT | O_APPEND);
    if (logfile) {
      logfile.seekEnd(0);
      SDLog.begin(LOG_LEVEL_VERBOSE, &logfile);
    }
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

    Log.notice   (  "Log as Info with integer values : %d, %d"                   , intValue1,  intValue2);
    Log.notice   (F("Log as Info with hex values     : %x, %X"                  ), intValue1,  intValue1);
    Log.notice   (  "Log as Info with hex values     : %x, %X"                   , intValue2,  intValue2);
    Log.notice   (F("Log as Info with binary values  : %b, %B"                  ), intValue1,  intValue1);
    Log.notice   (  "Log as Info with binary values  : %b, %B"                   , intValue2,  intValue2);
    Log.notice   (F("Log as Info with long values    : %l, %l"                  ), longValue1, longValue2);
    Log.notice   (  "Log as Info with bool values    : %t, %T"                   , boolValue1, boolValue2);
    Log.notice   (F("Log as Info with string value   : %s"                      ), charArray);
    Log.notice   (  "Log as Info with string value   : %s"                       , stringValue1.c_str());
    Log.notice   (F("Log as Info with float value   : %F"                       ), floatValue);
    Log.notice   (  "Log as Info with float value   : %F"                        , floatValue);
    Log.notice   (F("Log as Info with double value   : %D"                      ), doubleValue);
    Log.notice   (  "Log as Info with double value   : %D"                       , doubleValue);
    Log.notice   (F("Log as Debug with mixed values  : %d, %d, %l, %l, %t, %T"  ), intValue1 , intValue2,
                longValue1, longValue2, boolValue1, boolValue2);
    Log.trace    (  "Log as Trace with bool value    : %T"                       , boolValue1);
    Log.warning  (  "Log as Warning with bool value  : %T"                       , boolValue1);
    Log.error    (  "Log as Error with bool value    : %T"                       , boolValue1);
    Log.fatal    (  "Log as Fatal with bool value    : %T"                       , boolValue1);
    Log.verbose  (F("Log as Verbose with bool value   : %T"   CR               ), boolValue2);
    delay(5000);


    SDLog.notice   (  "Log as Info with integer values : %d, %d"                   , intValue1,  intValue2);
    SDLog.notice   (F("Log as Info with hex values     : %x, %X"                  ), intValue1,  intValue1);
    SDLog.notice   (  "Log as Info with hex values     : %x, %X"                   , intValue2,  intValue2);
    SDLog.notice   (F("Log as Info with binary values  : %b, %B"                  ), intValue1,  intValue1);
    SDLog.notice   (  "Log as Info with binary values  : %b, %B"                   , intValue2,  intValue2);
    SDLog.notice   (F("Log as Info with long values    : %l, %l"                  ), longValue1, longValue2);
    SDLog.notice   (  "Log as Info with bool values    : %t, %T"                   , boolValue1, boolValue2);
    SDLog.notice   (F("Log as Info with string value   : %s"                      ), charArray);
    SDLog.notice   (  "Log as Info with string value   : %s"                       , stringValue1.c_str());
    SDLog.notice   (F("Log as Info with float value   : %F"                       ), floatValue);
    SDLog.notice   (  "Log as Info with float value   : %F"                        , floatValue);
    SDLog.notice   (F("Log as Info with double value   : %D"                      ), doubleValue);
    SDLog.notice   (  "Log as Info with double value   : %D"                       , doubleValue);
    SDLog.notice   (F("Log as Debug with mixed values  : %d, %d, %l, %l, %t, %T"  ), intValue1 , intValue2,
                longValue1, longValue2, boolValue1, boolValue2);
    SDLog.trace    (  "Log as Trace with bool value    : %T"                       , boolValue1);
    SDLog.warning  (  "Log as Warning with bool value  : %T"                       , boolValue1);
    SDLog.error    (  "Log as Error with bool value    : %T"                       , boolValue1);
    SDLog.fatal    (  "Log as Fatal with bool value    : %T"                       , boolValue1);
    SDLog.verbose  (F("Log as Verbose with bool value   : %T"                     ), boolValue2);

    logfile.flush();
}

void printTimestamp(Print* _logOutput) {
  char c[12];
  int m = sprintf(c, "%10lu ", millis());
  _logOutput->print(c);
}

void printNewline(Print* _logOutput) {
  _logOutput->print('\n');
}

