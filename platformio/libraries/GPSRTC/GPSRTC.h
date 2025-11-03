
#define I2C_ADDRESS        0x20                      //7 bit address 0x40 write, 0x41 read

/*
 * GPSRTC.h - library for I2C GPS RTC
 * This library is intended to be uses with Arduino Time.h library functions
 */

#ifndef GPSRTC_h
#define GPSRTC_h

#include <TimeLib.h>
#include "registers.h"

// library interface description
class GPSRTC
{
  // user-accessible "public" interface
  public:
    GPSRTC();
    static time_t get();
    static uint8_t read(tmElements_t &tm);

};

extern GPSRTC GpsRtc;

#endif
 


