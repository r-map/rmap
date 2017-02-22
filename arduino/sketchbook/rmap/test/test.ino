#include <Wire.h>
#include <SensorDriver.h>
#include <registers-rain.h>

#define SENSORS_LEN 2

struct sensor_t {
  char driver[5];         // driver name
  char type[5];         // driver name
  int address;            // i2c address
} sensors[SENSORS_LEN];
SensorDriver* sd[SENSORS_LEN];

char* json;
aJsonObject* aj;

void setup() {
  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"TBR");
  sensors[0].address = 0x21;

  strcpy(sensors[1].driver,"I2C");
  strcpy(sensors[1].type,"HYT");
  sensors[1].address = 0x28;

  // start up the serial interface
  Serial.begin(115200);
  Serial.println("started");

  // start up the i2c interface
  Wire.begin();
  Wire.setClock(50000);
  delay(1000);

  for (int i = 0; i < SENSORS_LEN; i++) {
    sd[i]=SensorDriver::create(sensors[i].driver,sensors[i].type);
    if (sd[i] == NULL){
      Serial.print(sensors[i].driver);
      Serial.println(": driver not created !");
    }
    else {
      Serial.print(sensors[i].driver);
      Serial.println(": driver created");
      sd[i]->setup(sensors[i].driver,sensors[i].address);
    }
  }

  Serial.println("end setup");
}

void loop() {
  long unsigned int waittime, maxwaittime = 0;

  // prepare sensors to measure
  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == NULL){
      if (sd[i]->prepare(waittime) == SD_SUCCESS) {
	      maxwaittime = max(maxwaittime,waittime);
      }
      else{
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
      aj=sd[i]->getJson();
      json=aJson.print(aj,50);
      Serial.print(sensors[i].type);
	    Serial.print(" : ");
      Serial.println(json);
      free(json);
      aJson.deleteItem(aj);
    }
  }

  // sleep some time to do not go tired ;)
  delay(2000);
}
