#include "ozgps.h"
#include <cmath>


#ifndef GPS_I2C_THREAD_H_
#define GPS_I2C_THREAD_H_

#define DATABUFFERLEN 120

#define DEVICE_ADDRESS 0x50
#define DEVICE_ADDRESS_R 0x54

struct gps_i2c_data_t {
  int id;
  frtosLogging* logger;
  gpsStatus_t* status;
  georef_t* georef;
  frtosRtc* frtosRTC;
  MutexStandard* i2cmutex;  
};

using namespace cpp_freertos;

class gpsI2cThread : public Thread {
  
 public:
  /**
   *  Constructor to create a GPS thread.
   *
   *  @param gps_i2c_data data used by thread.
   */
  
  gpsI2cThread(gps_i2c_data_t* gps_data);
  ~gpsI2cThread();
  virtual void Cleanup();
  
 protected:
  virtual void Run();

 private:
  void doI2cNmea();
  int i2c_write(uint8_t device_addr, const uint8_t* writeData, size_t data_len);
  int i2c_read(uint8_t device_addr, char* readData, size_t data_len);  
  void reset();
  void stepOne();
  size_t stepTwo(char* dataBuffer, size_t dataBufferLen);
  gps_i2c_data_t* data;
  OZGPS gps;
  MGPS mgps;
  time_t timestamp;
  uint32_t dataLength;
  uint8_t nomessagecounter;
};

#endif
