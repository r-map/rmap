/*
  SensorDriver.h - Library for read sensor.
  Created by Paolo Patruno , November 30, 2013.
  Released into the GPL licenze.
*/

#ifndef FRTOSSENSORDRIVERB_H
#define FRTOSSENSORDRIVERB_H

#include "SensorDriverb.h"
#include <ArduinoLog.h>
#ifdef ARDUINO_ARCH_AVR
  #include <ArduinoSTL.h>
  #include <Arduino_FreeRTOS.h>
#else 
  #ifdef ARDUINO_ARCH_STM32
    #include "STM32FreeRTOS.h"
  #else
    #include "FreeRTOS.h"
  #endif
#endif

#include <mutex.hpp>

using namespace cpp_freertos;

class frtosSensorDriver : public SensorDriver
{
public:
  static frtosSensorDriver* create(const char* driver,const char* type,  MutexStandard& mutex);
  frtosSensorDriver(const char* mydriver,const char* mytype, MutexStandard& mutex);
  ~frtosSensorDriver();
  int setup(const char* driver, const int address, const int node=0, const char* type=NULL);
  int prepare(uint32_t& waittime) ;
  int get(uint32_t values[],size_t lenvalues) ;
#if defined (USEGETDATA)
  int getdata(uint32_t& data,uint16_t& width);
#endif
#if defined(USEAJSON)
  aJsonObject* getJson() ;
#endif
#if defined(USEARDUINOJSON)
  int getJson(char *json_buffer, size_t json_buffer_length) ;
#endif  
  
  const char* driver;
  const char* type;

private:
  MutexStandard _mutex;
  SensorDriver *_sd;
};

#endif

