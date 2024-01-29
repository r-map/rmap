#include "stimawifi.h"

WiFiUDP UDP;
OZGPS gps;
MGPS mgps;

void doUdp(udp_data_t& data){
  
  // If UDP packet received...
  int packetSize = UDP.parsePacket();
  if (packetSize) {
    data.logger->notice(F("Received packet! Size: %d"),packetSize);
    
    uint8_t gpsflag;
    while(UDP.available()) {
      char c=UDP.read();
      gpsflag = gps.encode(c);
      if(gps.valid){
	data.logger->notice("RMC latitude : %D", mgps.rmc.dms.latitude);
	data.logger->notice("RMC longitude: %D", mgps.rmc.dms.longitude);
	data.logger->notice("GGA latitude : %D", mgps.gga.dms.latitude);
	data.logger->notice("GGA longitude: %D", mgps.gga.dms.longitude);
	data.logger->notice("GLL latitude : %D", mgps.gll.dms.latitude);
	data.logger->notice("GLL longitude: %D", mgps.gll.dms.longitude);
	data.logger->notice("RMC datetime: %d %d %d %d %d %d", mgps.rmc.time.year, mgps.rmc.time.mon, mgps.rmc.time.day,
			mgps.rmc.time.hours, mgps.rmc.time.min, mgps.rmc.time.sec);  
	//UDP.flush();
      }else{
	data.logger->notice("gps_error: %d", gpsflag);
      }
    }
  }else{
    data.logger->notice(F("No Received packet!"));
  }
}

using namespace cpp_freertos;

udpThread::udpThread(udp_data_t& udp_data)
  : Thread{"UDP", 10000, 1}
    ,data{udp_data}
{
  //data->logger->notice("Create Thread %s %d", GetName().c_str(), data->id);
  data.status->receive=unknown;
  //Start();
};

udpThread::~udpThread()
{
  data.logger->notice("Delete Thread %s %d", GetName().c_str(), data.id);
  data.status->receive=unknown;
}
  
void udpThread::Cleanup()
{
  UDP.stop();
  frtosLog.notice(F("Stop listening on UDP port %d"),UDP_PORT);
  delete this;
}

void udpThread::Run() {
  data.logger->notice("Starting Thread %s %d", GetName().c_str(), data.id);

  gps.init(&mgps);
  gps.set_filter(0xE); // "RMC","GGA","GLL"

  // Begin listening to UDP port
  UDP.begin(UDP_PORT);
  data.logger->notice(F("Listening on UDP port %d"),UDP_PORT);
  
  for(;;){
    doUdp(data);
    Delay(Ticks::SecondsToTicks(1));
  }
};  

