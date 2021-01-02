/****************************************************************************
 *
 *  Copyright (c) 2020, Paolo Patruno (p.patruno@iperbole.bologna.it)
 *
 ***************************************************************************/


#include "STM32FreeRTOS.h"
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
  
  sensorThread(int i, int delayInSeconds,sensor_t mysensor,MutexStandard& sdmutex)
    : Thread("Thread One", 200, 1), 
      Id (i), 
      DelayInSeconds(delayInSeconds),
      sensor(mysensor),
      sd(nullptr)
  {
	Start();
        sd=frtosSensorDriver::create(sensors[i].driver,sensors[i].type,sdmutex);
  };
  
protected:

  virtual void Run() {
    
    frtosLog.notice("Starting Thread %d %s %s",Id,sd->driver,sd->type);
    if (sd == nullptr){
      frtosLog.notice(F("%d:%s %s : driver not created !"),Id,sensor.driver,sd->type);
    }else{
      frtosLog.notice(F("%d:%s %s : driver created !"),Id,sensor.driver,sd->type);
    }
    
    sd->setup(sensor.driver,sensor.address);

    while (true) {
      Delay(Ticks::SecondsToTicks(DelayInSeconds));

      if (sd != nullptr){
	unsigned long waittime=0;
	if (sd->prepare(waittime) != SD_SUCCESS){
	  frtosLog.notice("%d:%s %s prepare failed !", Id,sd->driver,sd->type);
	}

	//wait sensors to go ready
	frtosLog.notice("%d:%s %s wait sensors for ms: %d",Id,sensor.driver,sd->type,waittime);
	TickType_t ticks=Ticks::MsToTicks(waittime);
	Delay( ticks ? ticks : 1 );            /* Minimum delay = 1 tick */
	
	// get integers values 
#define LENVALUES 3
	uint32_t values[LENVALUES];
	size_t lenvalues=LENVALUES;

	for (uint8_t ii = 0; ii < lenvalues; ii++) {
	  values[ii]=0xFFFFFFFF;
	}

	if (sd->get(values,lenvalues) == SD_SUCCESS){
	  for (uint8_t ii = 0; ii < lenvalues; ii++) {
	    frtosLog.notice("%d:%s %s value: %d",Id,sd->driver,sd->type,values[ii]);
	  }
	}else{
	  frtosLog.notice("%d:%s %s Error",Id,sd->driver,sd->type);
	}
	frtosLog.notice(F("Free stack bytes : %d" ),uxTaskGetStackHighWaterMark( NULL ));
	
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
  strcpy(sensors[0].type,"ADT");
  sensors[0].address=73;

  strcpy(sensors[1].driver,"I2C");
  strcpy(sensors[1].type,"HIH");
  sensors[1].address=39;
  
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
    ST[i]=new sensorThread(i, i+5,sensors[i],sdmutex);
  }
  
  Thread::StartScheduler();
  
}

void loop()
{
  // Empty. Things are done in Tasks.
}

