#include "common.h"

void gpsThread::GPS_SerialInit() {
  data->logger->notice(F("gps start GPS serial init"));  
  Serial0.begin(GPS_SERIAL_SPEED);
  Serial0.setTimeout(100);
  data->logger->notice(F("gps end GPS serial init"));
}

// get data from GPS in NMEA format
void gpsThread::doSerialNmea(){

  memset(&mgps,0,sizeof(mgps));
  
  # define MESSAGELEN (100)
  char message[MESSAGELEN];
  uint8_t len=Serial0.readBytesUntil(10, message, MESSAGELEN-1);
  message[len]=0;      // add terminator for printing
  
  if (len > 0) data->logger->trace(F("gps message: %s"), message);
  
  for (uint8_t i = 0; i < len; i++) {
    uint8_t gpsflag;
    //Serial.print(message[i],DEC);
    //Serial.println(" ");
    gpsflag = gps.encode(message[i]);
    if(gps.valid){
      break;
      //}else{
      //data->logger->notice(F("gps gps_error: %d"), gpsflag);
    }
  }
  
  if (gps.valid){                      // we have a valid GPS NMEA record
    
    // GGA and GLL record do not have date but only time
    if (mgps.rmc.time.year != 0){
      // we have a valid date and time
      data->logger->notice(F("gps RMC datetime: %d %d %d %d %d %d"), mgps.rmc.time.year, mgps.rmc.time.mon, mgps.rmc.time.day,
			   mgps.rmc.time.hours, mgps.rmc.time.min, mgps.rmc.time.sec);  
      // set system time and RTC time
      setTime(mgps.rmc.time.hours, mgps.rmc.time.min, mgps.rmc.time.sec
	      ,mgps.rmc.time.day,mgps.rmc.time.mon,mgps.rmc.time.year);
      if (!data->frtosRTC->set(now())){
	frtosLog.error("gps Setting RTC time from GPS!");
      }
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
    // set coordinate and timestamp in georef structure
    data->georef->mutex->Lock();    

    if (mgps.gga.dms.latitude != 0.0 and mgps.gga.dms.longitude != 0.0){
      data->logger->notice(F("gps GGA coordinate : %5 %5"), mgps.gga.dms.latitude, mgps.gga.dms.longitude);
      itoa(int(std::round(mgps.gga.dms.latitude*100000)),data->georef->lat,10);
      itoa(int(std::round(mgps.gga.dms.longitude*100000)),data->georef->lon,10);
      data->georef->timestamp=now();
      timestamp=now();
      data->status->receive=ok;
    }
	
    if (mgps.gll.dms.latitude != 0.0 and mgps.gll.dms.longitude != 0.0){
      data->logger->notice(F("gps GLL latitude : %5 %5"), mgps.gll.dms.latitude, mgps.gll.dms.longitude);
      itoa(int(std::round(mgps.gll.dms.latitude*100000)),data->georef->lat,10);
      itoa(int(std::round(mgps.gll.dms.longitude*100000)),data->georef->lon,10);
      data->georef->timestamp=now();
      timestamp=now();
      data->status->receive=ok;
    }
    
    if (mgps.rmc.dms.latitude != 0.0 and mgps.rmc.dms.longitude != 0.0){
      data->logger->notice(F("gps RMC latitude : %5 %5"), mgps.rmc.dms.latitude, mgps.rmc.dms.longitude);
      itoa(int(std::round(mgps.rmc.dms.latitude*100000)),data->georef->lat,10);
      itoa(int(std::round(mgps.rmc.dms.longitude*100000)),data->georef->lon,10);
      data->georef->timestamp=now();           // TODO create datetime from RMC datetime
    }

    data->georef->mutex->Unlock();
  }
}

using namespace cpp_freertos;

gpsThread::gpsThread(gps_data_t* gps_data)
  : Thread{"GPS", TASK_GPS_STACK_SIZE, TASK_GPS_PRIORITY
           # if portNUM_PROCESSORS > 1
	   ,1  // if multicore 1 indicate the index number of the CPU which the task should be pinned to
           #endif
          }
    ,data{gps_data}
{
  //data->logger->notice(F("gps Create Thread %s %d"), GetName().c_str(), data->id);
  data->status->receive=unknown;
  data->status->memory_collision=ok;
  data->status->no_heap_memory=ok;
  timestamp=0;
  //Start();
};

gpsThread::~gpsThread()
{
}
  
void gpsThread::Cleanup()
{
  data->logger->notice(F("Delete Thread %s %d"), GetName().c_str(), data->id);
  data->status->receive=unknown;
  data->status->memory_collision=unknown;
  data->status->no_heap_memory=unknown;
  delete this;
}

void gpsThread::Run() {
  data->logger->notice(F("Starting Thread %s %d"), GetName().c_str(), data->id);

  gps.init(&mgps);
  //gps.set_filter(0xE); // filter "RMC","GGA","GLL" records
  gps.set_filter(0x2);   // filter "RMC" record only

  //Init GPS serial port
  GPS_SerialInit();
  data->logger->notice(F("gps Listening on GPS serial port"));
  
  for(;;){
    while (Serial0.available()){
      doSerialNmea();         // read data from GPS
    }
    if ((now()-timestamp) > 30){    // if the last coordinates are more than 30 sec old 
      data->status->receive=error;
    }else{
      data->status->receive=ok;
    }
    Delay(500);          // GPS provide data every 1 sec
    //Delay(Ticks::SecondsToTicks(1));
    //if( esp_get_minimum_free_heap_size() < HEAP_MIN_WARNING){
    //  data->logger->error(F("HEAP: %l"),esp_get_minimum_free_heap_size());
    //  data->status->no_heap_memory=error;
    //}
    
    //data->logger->notice(F("stack gps: %d"),uxTaskGetStackHighWaterMark(NULL));
    if(uxTaskGetStackHighWaterMark(NULL) < STACK_MIN_WARNING){
      data->logger->error(F("gps stack"));
      data->status->memory_collision=error;
    }
  }
};  

