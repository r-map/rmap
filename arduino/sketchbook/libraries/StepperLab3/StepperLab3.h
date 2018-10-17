/*
  StepperLab3.h - Stepper Library to use with L293 chip.
  Created by Martin Nawrath and Andreas Muxel, 2008.2009,2010
  http://interface.khm.de
  Released into the public domain.
*/

#ifndef StepperLab3_h
#define StepperLab3_h



class StepperLab3
{
  public:
	// constructor
    StepperLab3();
	// attach
	 void attach(int pin1, int pin2, int pin3, int pin4);
	// SA_Shield Motor Connection
	
	
	// set mode (default half-step)
	void setFullStep();
	void setHalfStep();
	// set power range 0..100
	 void setPower(int power);  
	// set speed
	void setSpeed(int speed);
      // accelerate or brake within given number of steps to full speed or to 0
	void setRampSteps(int step);
	// set step , let motor run to step +-
	void relativeSteps(int step);
	void absoluteSteps(int step);
	void setRefPosition ( int apos);

	// get step return actual position
	int getSteps(void);
	// rotate infinetly, 1 right 0 left 
	void rotate(int direction);
	void singleStep(int direction);
	// get status step ready
	int stepReady(void);
	int stepDone(void);

	
	// debugger
	int stop(void);

	int debugger(void);


	

volatile unsigned int _tcnt2;


	
  private:
   void _setupTimer();
  
	// motor pins

	// number of steps
	int _numberOfSteps;
	// mode

	// pwm

	// speed
	unsigned long _delaySpeed;
	unsigned long _lastTimeSpeed;
	// drive parameters
	

};

#endif

