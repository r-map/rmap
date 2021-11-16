#define SIM800_ON_OFF_PIN     (5)
#define GSM_APN        ("ibox.tim.it")
#define GSM_USERNAME   ("")
#define GSM_PASSWORD   ("")
#define NTP_SERVER    ("pool.ntp.org")

#define DATE_TIME_STRING_LENGTH (20)

#ifndef ARDUINO_ARCH_AVR
HardwareSerial Serial1(PB11, PB10);
#endif


#include <debug_config.h>
#include <sim800Client.h>
#include <Sim800IPStack.h>
#include <Countdown.h>
#include <ArduinoLog.h>
#include <ntp.h>

sim800Client s800;

bool initmodem(void)
{
  LOGN("try to init sim800");
  if(s800.init(SIM800_ON_OFF_PIN)){
    s800.setTimeout(6000);
    LOGN("initialized sim800");
  }else{
    LOGN("error initializing sim800");
    return false;
  }
    
  while(true){
    int rc = s800.switchOn();
    if (rc == SIM800_OK) {
      delay(SIM800_WAIT_FOR_AUTOBAUD_DELAY_MS);
      LOGN("switch on OK");
      break;
    }else if (rc == SIM800_ERROR){
      LOGN("switch on failed");
      return false;
    }
  }

  while(true){
    int rc =s800.initAutobaud();
    if (rc == SIM800_OK) {
      delay(SIM800_WAIT_FOR_AUTOBAUD_DELAY_MS);
      LOGN("autobaud OK");
      break;
    }else if (rc == SIM800_ERROR){
      LOGN("autobaud failed");
      return false;
    }
  }

  while(true){
    int rc =s800.setup();
    if (rc == SIM800_OK) {
      LOGN("setup OK");
      break;
    }else if (rc == SIM800_ERROR){
      LOGN("setup failed");
      return false;
    }
  }
  

  while(true){
    int rc =s800.startConnection(GSM_APN,GSM_USERNAME,GSM_PASSWORD);
    if (rc == SIM800_OK) {
      LOGN("start connection OK");
      break;
    }else if (rc == SIM800_ERROR){
      LOGN("start connection failed");
      return false;
    }
  }
  return true;
}

void getNtp(){

  while(true){
    int rc = s800.connection(SIM800_CONNECTION_UDP, NTP_SERVER, NTP_SERVER_PORT);
    if (rc == SIM800_OK) {
      LOGN(F("ntp connected"));
      break;
    }else if (rc == SIM800_ERROR){
      LOGE(F("error ntp connect"));
      return;
    }    
  }
      
  if(Ntp::sendRequest(&s800)){
    time_t  current_ntp_time = Ntp::getResponse(&s800);
    if( current_ntp_time > 0){
      setTime(current_ntp_time);
      
      LOGN(F("Current NTP date and time: %d:%d:%d  %d:%d:%d "),day(),month(),year(),hour(),minute(),second());
    } else {
      LOGE(F("error ntp getReponse"));
    }
  } else {
    LOGE(F("error ntp sendReponse"));
  }

  while(true){
    int rc = s800.stopConnection();
    if (rc == SIM800_OK) {
      LOGN(F("ntp disconnected"));
      break;
    }else if (rc == SIM800_ERROR){
      LOGE(F("error ntp disconnect"));
      return;
    }    
  }
  
}

void logPrefix(Print* _logOutput) {
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
  Log.begin(LOG_LEVEL,&Serial);
  Log.setPrefix(logPrefix); // Uncomment to get timestamps as prefix
  Log.setSuffix(logSuffix); // Uncomment to get newline as suffix
  delay(2000);
  LOGN("NTP test");

  while(!initmodem()){
    delay(5000);
  }
}

void loop()
{ 
  getNtp();
  delay(5000);
}
