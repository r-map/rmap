#include <ArduinoLog.h>
#include <TimeLib.h>
#include <pcf8563.h>

time_t  current_time;

void resetrtc(){
  // set defaul date and time
  // 4/7/2021 18:30:45
  
  tmElements_t tm;
  tm.Hour=18;
  tm.Minute=30;
  tm.Second=45;
  tm.Day=4;
  tm.Month=7;
  tm.Year = y2kYearToTm(21);
  current_time = makeTime(tm);
  setTime(current_time);
  LOGN(F("set default date and time: %d:%d:%d  %d:%d:%d "),year(),month(),day(),hour(),minute(),second());

  if (!setrtc(current_time))
    LOGE("error setting default date and time");
}

void initrtc(){

  Pcf8563::enable();

  if ( getrtc(current_time)){
    setTime(current_time);
    LOGN(F("Set current date and time from RTC: %d:%d:%d  %d:%d:%d "),year(),month(),day(),hour(),minute(),second());
  } else {
    LOGE(F("error getting rtc date time"));
    resetrtc();
  }
}


bool getrtc(time_t &time){

  tmElements_t tm;

  if (Pcf8563::getDateTime(&tm.Hour,
			   &tm.Minute,
			   &tm.Second,
			   &tm.Day,
			   &tm.Month,
			   &tm.Year)){

    tm.Year = y2kYearToTm(tm.Year);
    time = makeTime(tm);
    return true;
  }
  time=0;
  return false;
}

bool setrtc(const time_t time){

  tmElements_t tm;
  breakTime(time,tm);
  tm.Year = tmYearToY2k(tm.Year);
  return Pcf8563::setDateTime(tm.Hour,
			      tm.Minute,
			      tm.Second,
			      tm.Day,
			      tm.Month,
			      tm.Year,0,0);
}

void logPrefix(Print* _logOutput) {
  #define DATE_TIME_STRING_LENGTH (20)
  char dt[DATE_TIME_STRING_LENGTH];
  snprintf(dt, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u", year(), month(), day(), hour(), minute(), second());  
  _logOutput->print("#");
  _logOutput->print(dt);
  _logOutput->print(" ");
}

void logSuffix(Print* _logOutput) {
  _logOutput->print('\n');
  //_logOutput->flush();  // we use this to flush every log message
}


void setup()
{
  Serial.begin(115200);
  Wire.begin();
  Log.begin(LOG_LEVEL,&Serial);
  Log.setPrefix(logPrefix); // Uncomment to get timestamps as prefix
  Log.setSuffix(logSuffix); // Uncomment to get newline as suffix

  LOGN("Start rtc test");

  //resetrtc();             // uncomment to reset the date and time to default
  initrtc();
}

void loop()
{ 
  LOGN("getrtc: %T",getrtc(current_time));
  if (current_time > 0) setTime(current_time);
  delay(5000);

  LOGN(F("Current date and time: %d:%d:%d  %d:%d:%d "),year(),month(),day(),hour(),minute(),second());
  
  LOGN("setrtc: %T",setrtc(now()));
  delay(5000);
}
