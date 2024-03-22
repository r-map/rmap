#include "ozgps.h"
#include <cmath>

#ifndef GPS_THREAD_H_
#define GPS_THREAD_H_

/*
Android-GPSd-Forwarder send a burst of GPS packets > 6
so we have trouble to do not lost some packets
in lwip the buffer size for GPS is defined by
CONFIG_LWIP_GPS_RECVMBOX_SIZE
with default to 6
In arduino we cannot change this value because the librari is precompiled.
We can only speed up the task and hope.
The priority is set to 3 and delay is very short.
*/

struct gps_data_t {
  int id;
  frtosLogging* logger;
  gpsStatus_t* status;
  georef_t* georef;
};

void doGps(gps_data_t& data);

using namespace cpp_freertos;

class gpsThread : public Thread {
  
 public:
  /**
   *  Constructor to create a GPS thread.
   *
   *  @param gps_data data used by thread.
   */
  
  gpsThread(gps_data_t& gps_data);
  ~gpsThread();
  virtual void Cleanup();
  
 protected:
  virtual void Run();

 private:
  gps_data_t data;

};

#endif
