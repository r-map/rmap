#include <Wire.h>
#include <SensorDriver.h>

#define SENSORS_LEN 1

struct sensor_t
{
  char driver[5];         // driver name
  char type[5];         // driver name
  int address;            // i2c address
} sensors[SENSORS_LEN];
SensorDriver* sd[SENSORS_LEN];

char* json;
aJsonObject* aj;

void setup()
{

  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"SMI");
  sensors[0].address=I2C_SDSMICS_DEFAULTADDRESS;

  // start up the serial interface
  Serial.begin(9600);
  Serial.println("started");

  // start up the i2c interface
  Wire.begin();

  // set the frequency 
  #define I2C_CLOCK 50000

  //set the i2c clock 
  TWBR = ((F_CPU / I2C_CLOCK) - 16) / 2;

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
  delay(maxwaittime);

  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == NULL){
      // get values in json format
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
  Serial.println("sleep for 10s");
  delay(10000);

}
