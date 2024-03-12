#include "stimawifi.h"
#include <cmath>

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
OZGPS gps;
MGPS mgps;

void doUdp(udp_data_t& data){

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
      data.logger->notice(F("RMC latitude : %5"), mgps.rmc.dms.latitude);
      data.logger->notice(F("RMC longitude: %5"), mgps.rmc.dms.longitude);
      //data.logger->notice(F("GGA latitude : %5"), mgps.gga.dms.latitude);
      //data.logger->notice(F("GGA longitude: %5"), mgps.gga.dms.longitude);
      //data.logger->notice(F("GLL latitude : %5"), mgps.gll.dms.latitude);
      //data.logger->notice(F("GLL longitude: %5"), mgps.gll.dms.longitude);
      data.logger->notice(F("RMC datetime: %d %d %d %d %d %d"), mgps.rmc.time.year, mgps.rmc.time.mon, mgps.rmc.time.day,
			  mgps.rmc.time.hours, mgps.rmc.time.min, mgps.rmc.time.sec);  
      
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
      itoa(int(std::round(mgps.rmc.dms.latitude*100000)),data.georef->lat,10);
      itoa(int(std::round(mgps.rmc.dms.longitude*100000)),data.georef->lon,10);
      data.georef->timestamp=now();           // TODO create datetime from RMC datetime
      data.georef->mutex->Unlock();
      data.status->receive=ok;
      
    }
  }
  if ((now()-data.georef->timestamp) > 30) data.status->receive=error;
}

using namespace cpp_freertos;

udpThread::udpThread(udp_data_t& udp_data)
  : Thread{"UDP", 20000, 3}
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
  frtosLog.notice(F("Stop listening on UDP port %d"),UDP_PORT);
  data.logger->notice("Delete Thread %s %d", GetName().c_str(), data.id);
  data.status->receive=unknown;
  delete this;
}

void udpThread::Run() {
  data.logger->notice("Starting Thread %s %d", GetName().c_str(), data.id);

  gps.init(&mgps);
  //gps.set_filter(0xE); // "RMC","GGA","GLL"
  gps.set_filter(0x2); // "RMC"

  // Begin listening to UDP port
  UDP.begin(UDP_PORT);
  data.logger->notice(F("Listening on UDP port %d"),UDP_PORT);
  
  for(;;){
    doUdp(data);
    const TickType_t xDelay = 10;
    Delay(xDelay);
    //Delay(Ticks::SecondsToTicks(1));
  }
};  

