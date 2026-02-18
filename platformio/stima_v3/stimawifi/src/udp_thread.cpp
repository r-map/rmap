#include "common.h"

/*
Android-GPSd-Forwarder send a burst of UDP packets > 6
so we have trouble to do not lost some packets
in lwip the buffer size for UDP is defined by
CONFIG_LWIP_UDP_RECVMBOX_SIZE
with default to 6
In arduino we cannot change this value because the librari is precompiled.
We can only speed up the task and hope.
The priority is set to 3 and delay is very short.
*/

void udpThread::doUdp(){

  while (UDP.parsePacket()){
    // If UDP packet received...
    //data.logger->notice(F("Received packet!"));
    memset(&mgps,0,sizeof(mgps));

    while(UDP.available()) {
      uint8_t gpsflag;
      gpsflag = gps.encode(UDP.read());
      if(gps.valid) break;
	//}else{
	//data.logger->notice("gps_error: %d", gpsflag);
    }

    while(UDP.available()) {
      UDP.read();
    }
    
    if (gps.valid){
      data->logger->notice(F("udp RMC latitude : %5"), mgps.rmc.dms.latitude);
      data->logger->notice(F("udp RMC longitude: %5"), mgps.rmc.dms.longitude);
      //data.logger->notice(F("GGA latitude : %5"), mgps.gga.dms.latitude);
      //data.logger->notice(F("GGA longitude: %5"), mgps.gga.dms.longitude);
      //data.logger->notice(F("GLL latitude : %5"), mgps.gll.dms.latitude);
      //data.logger->notice(F("GLL longitude: %5"), mgps.gll.dms.longitude);
      data->logger->notice(F("udp RMC datetime: %d %d %d %d %d %d"), mgps.rmc.time.year, mgps.rmc.time.mon, mgps.rmc.time.day,
			  mgps.rmc.time.hours, mgps.rmc.time.min, mgps.rmc.time.sec);  

      setTime(mgps.rmc.time.hours, mgps.rmc.time.min, mgps.rmc.time.sec
	      ,mgps.rmc.time.day,mgps.rmc.time.mon,mgps.rmc.time.year);
      if (!data->frtosRTC->set(now())){
	frtosLog.error("udp Setting RTC time from UDP!");
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
      itoa(int(std::round(mgps.rmc.dms.latitude*100000)),data->georef->lat,10);
      itoa(int(std::round(mgps.rmc.dms.longitude*100000)),data->georef->lon,10);
      data->georef->timestamp=now();           // TODO create datetime from RMC datetime
      data->georef->mutex->Unlock();
      timestamp=now();
      data->status->receive=ok;
      
    }
  }
  if ((now()-timestamp) > 30){
    data->status->receive=error;
  }else{
    data->status->receive=ok;
  }
}

using namespace cpp_freertos;

udpThread::udpThread(udp_data_t* udp_data)
  : Thread{"UDP", TASK_UDP_STACK_SIZE, TASK_UDP_PRIORITY}  // 1152 free
    ,data{udp_data}
{
  //data->logger->notice("Create Thread %s %d", GetName().c_str(), data->id);
  data->status->receive=unknown;
  data->status->memory_collision=ok;
  data->status->no_heap_memory=ok;
  timestamp=0;
  //Start();
};

udpThread::~udpThread()
{
}
  
void udpThread::Cleanup()
{
  UDP.stop();
  data->logger->notice(F("udp Stop listening on UDP port %d"),UDP_PORT);
  data->logger->notice("udp Delete Thread %s %d", GetName().c_str(), data->id);
  data->status->receive=unknown;
  data->status->memory_collision=unknown;
  data->status->no_heap_memory=unknown;
  delete this;
}

void udpThread::Run() {
  data->logger->notice("udp Starting Thread %s %d", GetName().c_str(), data->id);

  gps.init(&mgps);
  //gps.set_filter(0xE); // "RMC","GGA","GLL"
  gps.set_filter(0x2); // "RMC"

  // Begin listening to UDP port
  UDP.begin(UDP_PORT);
  data->logger->notice(F("udp Listening on UDP port %d"),UDP_PORT);
  
  for(;;){
    doUdp();
    Delay(10);
    //Delay(Ticks::SecondsToTicks(1));

    //if( esp_get_minimum_free_heap_size() < HEAP_MIN_WARNING){
    //  data->logger->error(F("HEAP: %l"),esp_get_minimum_free_heap_size());
    //  data->status->no_heap_memory=error;
    //}
    //data.logger->notice("stack udp: %d",uxTaskGetStackHighWaterMark(NULL));
    if(uxTaskGetStackHighWaterMark(NULL) < STACK_MIN_WARNING){
      data->logger->error("udp stack");
      data->status->memory_collision=error;
    }
  }
};  

