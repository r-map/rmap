#include <Wire.h>
#include <frtosSensorDriverb.h>

#define SENSORS_LEN 1

struct sensor_t
{
  char driver[5];         // driver name
  char type[5];         // driver name
  int address;            // i2c address
} sensors[SENSORS_LEN];
frtosSensorDriver* sd[SENSORS_LEN];

void setup()
{

  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"SCD");
  sensors[0].address=SCD30_ADDRESS;
  
  // start up the serial interface
  Serial.begin(115200);
  Serial.println("started");

  // start up the i2c interface
  Wire.begin();

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
#define LENVALUES 3
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
      
  // sleep some time to do not go tired ;)
  Serial.println("sleep for 10s");
  delay(10000);

}
