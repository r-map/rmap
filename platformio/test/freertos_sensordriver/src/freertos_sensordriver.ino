/****************************************************************************
 *
 *  Copyright (c) 2020, Paolo Patruno (p.patruno@iperbole.bologna.it)
 *
 ***************************************************************************/

#define SHT_ADDRESS 33

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
#include "task.h"
#include "thread.hpp"
#include "ticks.hpp"
#include <frtosLog.h>
#include <Wire.h>
#include <frtosSensorDriverb.h>

void printTimestamp(Print* _logOutput) {
  char c[12];
  sprintf(c, "%10lu ", millis());
  _logOutput->print(c);
}

void printNewline(Print* _logOutput) {
  _logOutput->print('\n');
}

#define SENSORS_LEN 2

struct sensor_t
{
  char driver[5];         // driver name
  char type[5];         // driver name
  uint8_t address;            // i2c address
} sensors[SENSORS_LEN];
frtosSensorDriver* sd[SENSORS_LEN];

using namespace cpp_freertos;

class sensorThread : public Thread {
  
public:
  
  sensorThread(int i, int delayInSeconds,sensor_t mysensor,MutexStandard sdmutex)
    : Thread("Thread One", 200, 1), 
      Id (i), 
      DelayInSeconds(delayInSeconds),
      sensor(mysensor)
  {
        sd=frtosSensorDriver::create(sensors[i].driver,sensors[i].type,sdmutex);
	Start();
  };
  
protected:

  virtual void Run() {
    
    frtosLog.notice("Starting Thread %d",Id,sd->driver);
    if (sd == NULL){
      frtosLog.notice(F("%s : driver not created !"),sensor.driver);
    }else{
      frtosLog.notice(F("%s : driver created !"),sensor.driver);
    }
    
    sd->setup(sensor.driver,sensor.address);

    while (true) {
      Delay(Ticks::SecondsToTicks(DelayInSeconds));

      unsigned long waittime;
      if (!sd == NULL){
	if (sd->prepare(waittime) == SD_SUCCESS){
	}else{
	  frtosLog.notice("%d:%s prepare failed !", Id,sd->driver);
	}

	//wait sensors to go ready
	frtosLog.notice("%d:%s wait sensors for ms: %d",Id,sensor.driver,waittime);
	delay(waittime);  // 500 for tmp and 250 for adt and 2500 for davis
	//Delay(Ticks::SecondsToTicks(waittime/1000));
	
	// get integers values 
#define LENVALUES 3
	long values[LENVALUES];
	size_t lenvalues=LENVALUES;

	for (int ii = 0; ii < lenvalues; ii++) {
	  values[ii]=0xFFFFFFFF;
	}

	if (sd->get(values,lenvalues) == SD_SUCCESS){
	  for (int ii = 0; ii < lenvalues; ii++) {
	    frtosLog.notice("%d:%s value: %d",Id,sd->driver,values[ii]);
	  }
	}else{
	  frtosLog.notice("%d:%s Error",Id,sd->driver);
	}
      }
    }
  };

private:
  int Id;
  int DelayInSeconds;
  frtosSensorDriver* sd;
  sensor_t sensor;
};

sensorThread* ST[SENSORS_LEN];

void setup (void)
{

  MutexStandard loggingmutex;
  MutexStandard sdmutex;

  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"SCD");
  sensors[0].address=SCD30_ADDRESS;

  strcpy(sensors[1].driver,"I2C");
  strcpy(sensors[1].type,"SHT");
  sensors[1].address=SHT_ADDRESS;
  
  // start up the i2c interface
  Wire.begin();
  
  // start up the serial interface
  Serial.begin(115200);
  frtosLog.begin(LOG_LEVEL_VERBOSE, &Serial,loggingmutex);
  frtosLog.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
  frtosLog.setSuffix(printNewline); // Uncomment to get newline as suffix

  //Start logging
  frtosLog.notice(F("Testing FreeRTOS C++ wrappers to SensorDriver"));                     // Info string with Newline

  for (int i = 0; i < SENSORS_LEN; i++) {
    ST[i]=new sensorThread(i, 2,sensors[i],sdmutex);
  }
  
  Thread::StartScheduler();
  
}

void loop()
{
  // Empty. Things are done in Tasks.
}

