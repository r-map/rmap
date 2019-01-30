
#define SENSORS_LEN 1
#define LENVALUES 2
#define MR_PWM   D5
#define ML_PWM   D6
#define MR_EN    D3
#define ML_EN    D4
#define MR_IS    A0
#define ML_IS    A0
#define SCL D1
#define SDA D2

#include <Wire.h>
#include <SensorDriverb.h>
#include <PID_v1.h>
#include <ibt_2.h>

struct sensor_t
{
  char driver[5];         // driver name
  char type[5];         // driver name
  int address;            // i2c address
} sensors[SENSORS_LEN];
SensorDriver* sd[SENSORS_LEN];

char* json;
//aJsonObject* aj;

//Define Variables we'll be connecting to
double Setpoint, Input, Output;
//Specify the links and initial tuning parameters

double gain=10./127.;
double ct=60.;
double tau=30;


double Kp=1.2*ct/(gain*tau);
double Ki=Kp/(2.*tau);
double Kd=(0.5*tau)/Kp;

PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

ibt_2 hbridge(IBT_2_2HALF,MR_PWM,ML_PWM,MR_EN ,ML_EN ,MR_IS ,ML_IS);


void setup()
{
  hbridge.start(IBT_2_R_HALF);
  
  //turn the PID on
  Setpoint = 40.0;
  myPID.SetMode(AUTOMATIC);

  strcpy(sensors[0].driver,"I2C");
  strcpy(sensors[0].type,"HIH");
  sensors[0].address=39;
  // 39	0x27
  // 73	0x49
  
  // start up the serial interface
  Serial.begin(115200);
  Serial.println("started");
  Serial.println(Kp);
  Serial.println(Ki);
  Serial.println(Kd);
 
  // start up the i2c interface
  Wire.begin(SDA,SCL);

  for (int i = 0; i < SENSORS_LEN; i++) {

    sd[i]=SensorDriver::create(sensors[i].driver,sensors[i].type);
    if (sd[i] == 0){
      Serial.print(sensors[i].driver);
      Serial.println(": driver not created !");
    }else{
      sd[i]->setup(sensors[i].driver,sensors[i].address);
    }
  }
}

void loop()
{
  long unsigned int waittime,maxwaittime=0;

  // prepare sensors to measure
  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == 0){
      if (sd[i]->prepare(waittime) == SD_SUCCESS){
        Serial.print(sensors[i].driver);
        Serial.print(" : ");
        Serial.print(sensors[i].type);
        Serial.println(" : Prepare OK");
	maxwaittime=max(maxwaittime,waittime);
      }else{
	Serial.print(sensors[i].driver);
        Serial.print(" : ");
        Serial.print(sensors[i].type);
	Serial.println(" : Prepare failed !");
      }
    }
  }

  //wait sensors to go ready
  Serial.print("# wait sensors for ms:");  Serial.println(maxwaittime);
  delay(maxwaittime);  // 500 for tmp and 250 for adt and 2500 for davis

  for (int i = 0; i < SENSORS_LEN; i++) {
    if (!sd[i] == 0){
      // get integers values 
      long values[LENVALUES];
      size_t lenvalues=LENVALUES;

      if (sd[i]->get(values,lenvalues) == SD_SUCCESS){
	for (size_t ii = 0; ii < lenvalues; ii++) {
	  Serial.println(values[ii]);
	}

	Input = (float(values[1])/100.)-273.15;
	Serial.print("Temperatura: ");
	Serial.print(Input);
	Serial.print(" ");
      
	myPID.Compute();
	Serial.print("PID command: ");
	Serial.println(Output);
	hbridge.setpwm(int(Output),IBT_2_R_HALF);

      }else{
	Serial.println("Error");
      }
    }
  }
  
  // sleep some time to do not go tired ;)
  //Serial.println("#sleep for 1s");
  delay(1000);

}
