/****************************************************************************
 *
 *  Copyright (c) 2020, Paolo Patruno (p.patruno@iperbole.bologna.it)
 *
 ***************************************************************************/

#include <ArduinoLog.h>
#include <SensorDriverb.h>

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

SensorDriver* sd[SENSORS_LEN];


void setup (void)
{


  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"ADT");
  sensors[0].address=73;

  strcpy(sensors[1].driver,"I2C");
  strcpy(sensors[1].type,"HIH");
  sensors[1].address=39;
  
  // start up the serial interface
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
  Log.setSuffix(printNewline); // Uncomment to get newline as suffix

  //Start logging
  Log.notice(F("Starting Testing SensorDriver"));

  // start up the i2c interface
  Wire.begin();

  for (int i = 0; i < SENSORS_LEN; i++) {

    sd[i]=SensorDriver::create(sensors[i].driver,sensors[i].type);

    if (sd[i] == nullptr){
      Log.notice(F("%s %s : driver not created !"),sensors[i].driver,sensors[i].type);
    }else{
      Log.notice(F("%s %s : driver created !"),sensors[i].driver,sensors[i].type);
      if (sd[i]->setup(sensors[i].driver,sensors[i].address) == SD_SUCCESS) {
	Log.notice(F("%s %s : setup OK"),sensors[i].driver,sensors[i].type);
      }else{
	Log.error(F("%s %s : error setup"),sensors[i].driver,sensors[i].type);
      }
    }
  }  
}

void loop()
{

  long unsigned int waittime,maxwaittime=0;

  // prepare sensors to measure
  for (int i = 0; i < SENSORS_LEN; i++) {
    if (sd[i] == nullptr){
      Log.error("%s %s : skipped", sensors[i].driver,sensors[i].type);      
    }else{
      if (sd[i]->prepare(waittime) == SD_SUCCESS){
	maxwaittime=max(maxwaittime,waittime);
      }else{
	Log.error(F("%s %s : error prepare"),sensors[i].driver,sensors[i].type);	
      }
    }
  }

  //wait sensors to go ready
  Log.notice("wait sensors for ms: %d",waittime);
  delay(maxwaittime);  // 500 for tmp and 250 for adt and 2500 for davis
  
  for (int i = 0; i < SENSORS_LEN; i++) {
    if (sd[i] == nullptr){
      Log.error("%s %s : skipped", sensors[i].driver,sensors[i].type);      
    }else{      
      size_t lenvalues=2;
      long int values[lenvalues];
      for (uint8_t ii = 0; ii < lenvalues; ii++) {
	values[ii]=0XFFFFFFFF;
      }
      if (sd[i]->get(values,lenvalues) == SD_SUCCESS) {
	for (uint8_t ii = 0; ii < lenvalues; ii++) {
	  Log.notice("%s %s : value: %d",sensors[i].driver,sensors[i].type,values[ii]);
	}
      }else{
	Log.error("%s %s : Error getting data",sensors[i].driver,sensors[i].type);
      }
    }
  }

  // sleep some time to do not go tired ;)
  delay(5000);

}

