/* stepper library StepperLab3 
/*
 * ramp example
 *
 * KHM Lab3 2010
 * Kunsthochschule fuer Medien Koeln
 * Academy of Media Arts Cologne
 * http://interface.khm.de
 
 */

// import stepper library
#include <StepperLab3.h>
// create instance for stepper
StepperLab3 myStepper;

int state=1;
int statea=0;

int cnt10ms;
int cnt2;

int ledGreen= 5;
int motor1_1=12;
int motor1_2= 10;
int motor2_1= 11;
int motor2_2= 9;


void setup(){
  Serial.begin(115200);
  Serial.println(" stepperLab3 library test");

  pinMode(ledGreen,OUTPUT);  // LED pin
  myStepper.attach(motor1_1,motor1_2,motor2_1,motor2_2);

  myStepper.setPower(900);
  myStepper.setSpeed(500);
  myStepper.setFullStep();

}


// this block is executed in a loop after setup is called one time
void loop(){
  delay(10);
  cnt10ms++;
  cnt2++;

  if ( state != statea) {  // print state
    Serial.print("state:");
    Serial.print(state);
    Serial.println("");
  }
  statea=state;

  //  myStepper.debugger();

  switch (state) {

  case 1:
    state = 300;
    myStepper.setRampSteps(15);
    break;

  case 300:  //  start new position 
    myStepper.setPower(900);   // power = 900 run
    myStepper.setSpeed(1000);  // speed = 1000* 100ms = 100 steps/second
    myStepper.absoluteSteps(100);
    state=301;
    break;

  case 301:  //  wait unil position is reached
    if (myStepper.stepReady() == 1) { 
      myStepper.setPower(500);   // power = 500  idle
      state=302;
    }
    break;

  case 302: //  start new position 
    myStepper.setPower(900);   // power = 900 run
    myStepper.absoluteSteps(0);
    state=303;
    break;

  case 303: //  wait unil position is reached
    if (myStepper.stepReady() == 1) { 
      myStepper.setPower(500);   // power = 500  idle
      state=400;
      cnt2=0;
    }
    break;   

  case 400: // 2 sec pause
    if (cnt2 > 200) {
      cnt2=0;
      state=300;
    }
    break;
  }

  if (cnt10ms % 50==0)  { // blink led 500 ms
    digitalWrite(ledGreen,!digitalRead(ledGreen));
  }
}
//*********************************************************************

















