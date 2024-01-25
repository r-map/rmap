#include "stimawifi.h"

void doUdp(void){
  
  // If UDP packet received...
  int packetSize = UDP.parsePacket();
  if (packetSize) {
    //frtosLog.notice(F("Received packet! Size: %d"),packetSize);
    
    uint8_t gpsflag;
    while(UDP.available()) {
      char c=UDP.read();
      gpsflag = gps.encode(c);
      if(gps.valid){
	frtosLog.notice("RMC latitude : %D", mgps.rmc.dms.latitude);
	frtosLog.notice("RMC longitude: %D", mgps.rmc.dms.longitude);
	frtosLog.notice("GGA latitude : %D", mgps.gga.dms.latitude);
	frtosLog.notice("GGA longitude: %D", mgps.gga.dms.longitude);
	frtosLog.notice("GLL latitude : %D", mgps.gll.dms.latitude);
	frtosLog.notice("GLL longitude: %D", mgps.gll.dms.longitude);
	frtosLog.notice("RMC datetime: %d %d %d %d %d %d", mgps.rmc.time.year, mgps.rmc.time.mon, mgps.rmc.time.day,
			mgps.rmc.time.hours, mgps.rmc.time.min, mgps.rmc.time.sec);  
	//UDP.flush();
      }else{
	frtosLog.notice("gps_error: %d", gpsflag);
      }
    }
  }else{
    frtosLog.notice(F("No Received packet!"));
  }
}

using namespace cpp_freertos;

udpThread::udpThread(udp_data_t &udp_data)
  : Thread("UDP", 50000, 1),
    data(udp_data)
{
  //data.logger.notice("Create Thread %s %d", GetName().c_str(), data.id);
  data.status.receive=unknown;
  //Start();
};

udpThread::~udpThread()
{
  data.logger.notice("Delete Thread %s %d", GetName().c_str(), data.id);
  data.status.receive=unknown;
}
  
void udpThread::Cleanup()
{
  delete this;
}

void udpThread::Run() {
  data.logger.notice("Starting Thread %s %d", GetName().c_str(), data.id);
  for(;;){
    doUdp();
    Delay(Ticks::SecondsToTicks(1));
  }
};  

