#include "ozgps.h"
#include <cmath>

#ifndef GPS_THREAD_H_
#define GPS_THREAD_H_

/*
NEO-6 - Data Sheet:
Serial Port 1 Output 9600 Baud, 8 bits, no parity bit, 1 stop bit
Configured to transmit both NMEA and UBX protocols, but only following NMEA and no
UBX messages have been activated at start-up:
GGA, GLL, GSA, GSV, RMC, VTG, TXT
*/

struct gps_data_t {
  int id;
  frtosLogging* logger;
  gpsStatus_t* status;
  georef_t* georef;
  frtosRtc* frtosRTC;
};

using namespace cpp_freertos;

class gpsThread : public Thread {
  
 public:
  /**
   *  Constructor to create a GPS thread.
   *
   *  @param gps_data data used by thread.
   */
  
  gpsThread(gps_data_t* gps_data);
  ~gpsThread();
  virtual void Cleanup();
  
 protected:
  virtual void Run();

 private:
  void GPS_SerialInit();
  void doSerialNmea();
  gps_data_t* data;
  OZGPS gps;
  MGPS mgps;
  time_t timestamp;
};

#endif
