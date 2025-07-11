#include "common.h"

OZGPS gps_gps;
MGPS gps_mgps;


#if defined(ARDUINO_LOLIN_C3_MINI)
HardwareSerial Serial2(0);  //if using UART1
#endif


void gpsThread::GPS_SerialInit() {
  data->logger->notice(F("gps start GPS serial init"));  
  Serial2.begin(GPS_SERIAL_SPEED);
  Serial.setTimeout(100);
  data->logger->notice(F("gps end GPS serial init"));
}

void gpsThread::doSerialNmea(){

  memset(&gps_mgps,0,sizeof(gps_mgps));

  # define MESSAGELEN (100)
  char message[MESSAGELEN];
  uint8_t len=Serial2.readBytesUntil(10, message, MESSAGELEN-1);
  message[len]=0;
  
  if (len > 0) data->logger->notice("gps message: %s", message);

  for (uint8_t i = 0; i < len; i++) {
    uint8_t gpsflag;
    //Serial.print(message[i],DEC);
    //Serial.println(" ");
    gpsflag = gps_gps.encode(message[i]);
    if(gps_gps.valid){
      break;
      //}else{
      //data->logger->notice("gps gps_error: %d", gpsflag);
    }
  }
  
  if (gps_gps.valid){
    data->logger->notice(F("gps RMC latitude : %5"), gps_mgps.rmc.dms.latitude);
    data->logger->notice(F("gps RMC longitude: %5"), gps_mgps.rmc.dms.longitude);
    //data->logger->notice(F("gps GGA latitude : %5"), gps_mgps.gga.dms.latitude);
    //data->logger->notice(F("gps GGA longitude: %5"), gps_mgps.gga.dms.longitude);
    //data->logger->notice(F("gps GLL latitude : %5"), gps_mgps.gll.dms.latitude);
    //data->logger->notice(F("gps GLL longitude: %5"), gps_mgps.gll.dms.longitude);
    data->logger->notice(F("gps RMC datetime: %d %d %d %d %d %d"), gps_mgps.rmc.time.year, gps_mgps.rmc.time.mon, gps_mgps.rmc.time.day,
			gps_mgps.rmc.time.hours, gps_mgps.rmc.time.min, gps_mgps.rmc.time.sec);  

    setTime(gps_mgps.rmc.time.hours, gps_mgps.rmc.time.min, gps_mgps.rmc.time.sec
	    ,gps_mgps.rmc.time.day,gps_mgps.rmc.time.mon,gps_mgps.rmc.time.year);
    if (!data->frtosRTC->set(now())){
      frtosLog.error("gps Setting RTC time from GPS!");
    }
    
    /*
      If you just need “GPS” coordinates, any of the GGA, RMC, or GLL
      sentences will do the job. However, if you need specific
      information, like an object’s altitude, you will need to use GGA
      sentences. Similar to this, if you need information about the
      object’s speed, you will have to use RMC sentences. That said,
      NMEA sentences should be chosen according to the additional
      information that you need.
    */
    
    data->georef->mutex->Lock();    
    itoa(int(std::round(gps_mgps.rmc.dms.latitude*100000)),data->georef->lat,10);
    itoa(int(std::round(gps_mgps.rmc.dms.longitude*100000)),data->georef->lon,10);
    data->georef->timestamp=now();           // TODO create datetime from RMC datetime
    data->georef->mutex->Unlock();
    data->status->receive=ok;
  }
}

using namespace cpp_freertos;

gpsThread::gpsThread(gps_data_t* gps_data)
  : Thread{"GPS", TASK_GPS_STACK_SIZE, TASK_GPS_PRIORITY}
    ,data{gps_data}
{
  //data->logger->notice("gps Create Thread %s %d", GetName().c_str(), data->id);
  data->status->receive=unknown;
  //Start();
};

gpsThread::~gpsThread()
{
}
  
void gpsThread::Cleanup()
{
  data->logger->notice("Delete Thread %s %d", GetName().c_str(), data->id);
  data->status->receive=unknown;
  delete this;
}

void gpsThread::Run() {
  data->logger->notice("Starting Thread %s %d", GetName().c_str(), data->id);

  gps_gps.init(&gps_mgps);
  //gps.set_filter(0xE); // "RMC","GGA","GLL"
  gps_gps.set_filter(0x2); // "RMC"

  //Init GPS serial port
  GPS_SerialInit();
  data->logger->notice(F("gps Listening on GPS serial port"));
  
  for(;;){
    while (Serial2.available()){
      doSerialNmea();
    }
    if ((now()-data->georef->timestamp) > 30){
      data->status->receive=error;
    }else{
      data->status->receive=ok;
    }
    Delay(500);
    //Delay(Ticks::SecondsToTicks(1));
    //if( esp_get_minimum_free_heap_size() < HEAP_MIN_WARNING){
    //  data->logger->error(F("HEAP: %l"),esp_get_minimum_free_heap_size());
    //  data->status->no_heap_memory=error;
    //}
    
    //data->logger->notice("stack gps: %d",uxTaskGetStackHighWaterMark(NULL));
    if(uxTaskGetStackHighWaterMark(NULL) < STACK_MIN_WARNING){
      data->logger->error("gps stack");
      data->status->memory_collision=error;
    }
  }
};  

