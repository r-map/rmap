#include <WiFiUdp.h>
#include "ozgps.h"
#include <cmath>

#ifndef UDP_THREAD_H_
#define UDP_THREAD_H_

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

struct udp_data_t {
  int id;
  frtosLogging* logger;
  udpStatus_t* status;
  georef_t* georef;
};

using namespace cpp_freertos;

class udpThread : public Thread {
  
 public:
  /**
   *  Constructor to create a UDP thread.
   *
   *  @param udp_data data used by thread.
   */
  
  udpThread(udp_data_t* udp_data);
  ~udpThread();
  virtual void Cleanup();
  
 protected:
  virtual void Run();

 private:
  void doUdp();
  udp_data_t* data;

};

#endif
