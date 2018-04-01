/*
TODO

* missing data as -1
* writeble registers (oneshot) do not work

 */


#include <Wire.h>
#include <SensorDriver.h>
//#include "registers-wind.h"
//#include "registers-rain.h"
//#include "registers-th.h"

#define SENSORS_LEN 1

struct sensor_t
{
  char driver[5];         // driver name
  char type[5];         // driver name
  int address;            // i2c address
} sensors[SENSORS_LEN];
SensorDriver* sd[SENSORS_LEN];

char* json;
//aJsonObject* aj;

//#define LENBUF 120
//const int node=0; 
//char mainbuf[LENBUF];
//size_t lenbuf=LENBUF;

//#include <RF24Network.h>
//#include <RF24.h>
//#include <SPI.h>

// nRF24L01(+) radio attached using Getting Started board 
//RF24 radio(9,10);

// Network uses that radio
//RF24Network network(radio);


void setup()
{

 /*
  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"DW1");
  sensors[0].address=I2C_WIND_ADDRESS;

  strcpy(sensors[1].driver,"I2C");
  strcpy(sensors[1].type,"TBR");
  sensors[1].address=I2C_RAIN_ADDRESS;

  strcpy(sensors[2].driver,"I2C");
  strcpy(sensors[2].type,"BMP");
  sensors[2].address=119;
  strcpy(sensors[3].driver,"I2C");
  strcpy(sensors[3].type,"ADT");
  sensors[3].address=0x33;
  strcpy(sensors[4].driver,"I2C");
  strcpy(sensors[4].type,"TMP");
  sensors[4].address=72;
  strcpy(sensors[5].driver,"I2C");
  strcpy(sensors[5].type,"HIH");
  sensors[5].address=39;

  //

  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"TMP");
  sensors[0].address=72;

  strcpy(sensors[1].driver,"I2C");
  strcpy(sensors[1].type,"TMP");
  sensors[1].address=73;
  */

  /*
  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"ITH");
  sensors[0].address=I2C_TH_ADDRESS;

  strcpy(sensors[1].driver,"I2C");
  strcpy(sensors[1].type,"NTH");
  sensors[1].address=I2C_TH_ADDRESS;

  strcpy(sensors[2].driver,"I2C");
  strcpy(sensors[2].type,"MTH");
  sensors[2].address=I2C_TH_ADDRESS;

  strcpy(sensors[3].driver,"I2C");
  strcpy(sensors[3].type,"XTH");
  sensors[3].address=I2C_TH_ADDRESS;
  */

  /*
  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"XTH");
  sensors[0].address=I2C_TH_ADDRESS;
  */


  strcpy(sensors[0].driver,"SERI");
  strcpy(sensors[0].type,"HPM");
  sensors[0].address=73;

  // start up the serial interface
  Serial.begin(19200);
  Serial.println("started");

  // start up the i2c interface
  Wire.begin();

  // set the frequency 
#define I2C_CLOCK 50000

  //set the i2c clock 
  //TWBR = ((F_CPU / I2C_CLOCK) - 16) / 2;


  delay(1000);


  for (int i = 0; i < SENSORS_LEN; i++) {

    sd[i]=SensorDriver::create(sensors[i].driver,sensors[i].type);
    if (sd[i] == NULL){
      Serial.print(sensors[i].driver);
      Serial.println(": driver not created !");
    }else{
      Serial.print(sensors[i].driver);
      Serial.println(": driver created");
      sd[i]->setup(sensors[i].driver,sensors[i].address);
    }
  }

  Serial.println("end setup");

}

void loop()
{
  long unsigned int waittime,maxwaittime=0;

  // prepare sensors to measure
  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == NULL){
      if (sd[i]->prepare(waittime) == SD_SUCCESS){
	maxwaittime=max(maxwaittime,waittime);
      }else{
	Serial.print(sensors[i].driver);
	Serial.println(": prepare failed !");
      }
    }
  }

  //wait sensors to go ready
  //Serial.print("# wait sensors for ms:");  Serial.println(maxwaittime);
  delay(maxwaittime);  // 500 for tmp and 250 for adt and 2500 for davis

  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == NULL){
      
      
      // get integers values 
#define LENVALUES 2
      long values[LENVALUES];
      size_t lenvalues=LENVALUES;

      for (int ii = 0; ii < lenvalues; ii++) {
	values[ii]=0xFFFFFFFF;
      }

      if (sd[i]->get(values,lenvalues) == SD_SUCCESS){
	for (int ii = 0; ii < lenvalues; ii++) {
	  Serial.print(F("value: "));
	  Serial.println(values[ii]);
	}
      }else{
	Serial.println("Error");
      }
      
      /*
      unsigned long data;
      unsigned short width;
      if (sd[i]->getdata(data,width) == SD_SUCCESS){
	Serial.print(F("value: "));
	Serial.println(data);
	Serial.print(F("  width: "));
	Serial.println(width);
      }else{
	Serial.println("Error");
      }
      */

      
      /*
      // get values in json format
      aj=sd[i]->getJson();
      json=aJson.print(aj,50);
      Serial.print(sensors[i].type);
	Serial.print(" : ");
      Serial.println(json);
      free(json);
      aJson.deleteItem(aj);
      */
    }
  }

  // sleep some time to do not go tired ;)
  Serial.println("sleep for 10s");
  delay(10000);

  //Serial.println("Reboot");
  //delay(1000);
  //asm volatile ("  jmp 0");

}
