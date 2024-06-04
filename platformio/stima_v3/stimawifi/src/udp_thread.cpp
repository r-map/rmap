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

WiFiUDP UDP;
OZGPS udp_gps;
MGPS udp_mgps;

void doUdp(udp_data_t& data){

  while (UDP.parsePacket()){
    // If UDP packet received...
    //data.logger->notice(F("Received packet!"));
    memset(&udp_mgps,0,sizeof(udp_mgps));

    while(UDP.available()) {
      uint8_t gpsflag;
      gpsflag = udp_gps.encode(UDP.read());
      if(udp_gps.valid) break;
	//}else{
	//data.logger->notice("gps_error: %d", gpsflag);
    }

    while(UDP.available()) {
      UDP.read();
    }
    
    if (udp_gps.valid){
      data.logger->notice(F("udp RMC latitude : %5"), udp_mgps.rmc.dms.latitude);
      data.logger->notice(F("udp RMC longitude: %5"), udp_mgps.rmc.dms.longitude);
      //data.logger->notice(F("GGA latitude : %5"), udp_mgps.gga.dms.latitude);
      //data.logger->notice(F("GGA longitude: %5"), udp_mgps.gga.dms.longitude);
      //data.logger->notice(F("GLL latitude : %5"), udp_mgps.gll.dms.latitude);
      //data.logger->notice(F("GLL longitude: %5"), udp_mgps.gll.dms.longitude);
      data.logger->notice(F("udp RMC datetime: %d %d %d %d %d %d"), udp_mgps.rmc.time.year, udp_mgps.rmc.time.mon, udp_mgps.rmc.time.day,
			  udp_mgps.rmc.time.hours, udp_mgps.rmc.time.min, udp_mgps.rmc.time.sec);  

      setTime(udp_mgps.rmc.time.hours, udp_mgps.rmc.time.min, udp_mgps.rmc.time.sec
	      ,udp_mgps.rmc.time.day,udp_mgps.rmc.time.mon,udp_mgps.rmc.time.year);
      if (RTC.set(now()) != 0){
	frtosLog.error("Setting RTC time from UDP!");
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
      
      data.georef->mutex->Lock();    
      itoa(int(std::round(udp_mgps.rmc.dms.latitude*100000)),data.georef->lat,10);
      itoa(int(std::round(udp_mgps.rmc.dms.longitude*100000)),data.georef->lon,10);
      data.georef->timestamp=now();           // TODO create datetime from RMC datetime
      data.georef->mutex->Unlock();
      data.status->receive=ok;
      
    }
  }
  if ((now()-data.georef->timestamp) > 30){
    data.status->receive=error;
  }else{
    data.status->receive=ok;
  }
}

using namespace cpp_freertos;

udpThread::udpThread(udp_data_t& udp_data)
  : Thread{"UDP", 2000, 3}  // 1152 free
    ,data{udp_data}
{
  //data->logger->notice("Create Thread %s %d", GetName().c_str(), data->id);
  data.status->receive=unknown;
  //Start();
};

udpThread::~udpThread()
{
}
  
void udpThread::Cleanup()
{
  UDP.stop();
  data.logger->notice(F("udp Stop listening on UDP port %d"),UDP_PORT);
  data.logger->notice("udp Delete Thread %s %d", GetName().c_str(), data.id);
  data.status->receive=unknown;
  delete this;
}

void udpThread::Run() {
  data.logger->notice("udp Starting Thread %s %d", GetName().c_str(), data.id);

  udp_gps.init(&udp_mgps);
  //udp_gps.set_filter(0xE); // "RMC","GGA","GLL"
  udp_gps.set_filter(0x2); // "RMC"

  // Begin listening to UDP port
  UDP.begin(UDP_PORT);
  data.logger->notice(F("udp Listening on UDP port %d"),UDP_PORT);
  
  for(;;){
    doUdp(data);
    Delay(10);
    //Delay(Ticks::SecondsToTicks(1));

    //data.logger->notice("stack udp: %d",uxTaskGetStackHighWaterMark(NULL));
    if(uxTaskGetStackHighWaterMark(NULL) < STACK_MIN_WARNING) data.logger->error("udp stack");
  }
};  

