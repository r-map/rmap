/********************************************************
 * PID Basic Example
 * Reading analog input 0 to control analog PWM output 3
 ********************************************************/

#include <PID_v1.h>

#define PIN_INPUT 0
#define PIN_OUTPUT 3

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
double Kp=2, Ki=5, Kd=1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

void setup()
{
  Serial.begin(115200);
  //initialize the variables we're linked to
  //Input = analogRead(PIN_INPUT);
  Input = 120;
  Setpoint = 100;

  //turn the PID on
  myPID.SetMode(AUTOMATIC);
}

void loop()
{
  //Input = analogRead(PIN_INPUT);
  myPID.Compute();
  Serial.println(Output);
  //analogWrite(PIN_OUTPUT, Output);
  delay(1000);
}


