/*
  StepperLab3.cpp - Stepper Library to use with L293 or other H-Bridge chip.
  Created by Martin Nawrath, KHM 2010.
  http://interface.khm.de
  Released into the public domain.
  
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  
    History:
  	Nov/2010 - V1.0 
  	Jan/2§12 - V1.1    Arduino 1.0
  
*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "StepperLab3.h"


static volatile int _motorPin1;
static volatile	int _motorPin2;
static volatile	int _motorPin3;
static volatile	int _motorPin4;

static volatile   byte _f_int;
static volatile   byte _f_step;
static volatile  byte _f_ramp;

static volatile  unsigned long int _ddsreg;
static volatile  unsigned int _directionStepper ;
static volatile  unsigned int _localStep;

static volatile  int _absoluteStep;
static volatile  int _currentStep;
static volatile  int _differenceSteps ;

static volatile  int _stepReadyStatus; 
static volatile  int _continues ;

static volatile  int _dspeed=1000;
static volatile  int _setSpeed;
static volatile  int _stepperMode;

unsigned int _rampSteps;
unsigned int _rampSpeed;
unsigned int _rampThres;
unsigned int _tmp;
unsigned int _stepsMade;

static volatile	int _pwmValue;
static volatile	int _pwm23Value;
static volatile	int _pwm1A;
static volatile	int _pwm1B;

// constructor
StepperLab3::StepperLab3(){};
// attach
void StepperLab3::attach(int pin1, int pin2, int pin3, int pin4)
{

	StepperLab3::_setupTimer();
       
	_motorPin1 = pin1;
	_motorPin2 = pin2;
	_motorPin3 = pin3;
	_motorPin4 = pin4;
	pinMode(_motorPin1, OUTPUT);
	pinMode(_motorPin2, OUTPUT);
	pinMode(_motorPin3, OUTPUT);
	pinMode(_motorPin4, OUTPUT);
	// PWM pins
	pinMode(9, OUTPUT);
	pinMode(10, OUTPUT);
	// init variables
	_stepperMode = 0;
	_lastTimeSpeed = 0;
	_directionStepper = 0;
	_localStep = 0;
	_currentStep = 0;
	_absoluteStep = 0;
	_delaySpeed = 0;
	_continues =0;
	
	_stepReadyStatus = 0;
}
//****************************************************************************
void StepperLab3::_setupTimer() {
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || (__AVR_ATmega1280__)
    
	// set Timer 1 prescaler to 1
	TCCR1B &=  ~((1<<CS11) | (1<<CS12));  // Reset Bits
	TCCR1B |=  (1<<CS10);    // set Bits
		
	   
	// Fast PWM 10 bit , WGM13=0,WGM12=1 WGM11=1, WGM10=1
	// 64uS / 15.625 KHz	
	
	TCCR1A |= (1<<WGM11) ;  //  WGM10=1
	TCCR1B |= (1<<WGM12) ;  //  WGM10=1
	TCCR1A |= (1<<WGM10);  //  WGM11=1
	TCCR1B &= ~(1<<WGM13);  //  WGM13=0
//	TCCR1B |= (1<<WGM13);  //  WGM13=1
	
	//OCR1A=500;
	
	 
	
	// set pins as pwm output
	TCCR1A |= ((1<<COM1A1) | (1<<COM1B1));
	
	// enable Timer1 Interrupt 
	TIMSK1 |= (1<<OCIE1A);
	
	
#endif	
}
//****************************************************************************
// set mode (0 = half step mode, 1 = full step mode)
void StepperLab3::setFullStep(){
	// mode
	_stepperMode = 0;
}
//****************************************************************************
void StepperLab3::setHalfStep(){
	// mode
	_stepperMode = 1;
}
//****************************************************************************
// set power
void StepperLab3::setPower(int power){

	_f_int=0;
	while(_f_int==0); // critical section, wait untlt interrupt is done

	// set pwm values
	_pwmValue = power;
	_pwm23Value = _pwmValue*9;
	_pwm23Value = _pwm23Value/10;
	_pwm1A=power;
	_pwm1B=power;
	TCCR1A |= (1 << COM1A1);
	TCCR1B |= (1 << COM1B1);
	// set pwm duty
	OCR1A = _pwm1A;
	OCR1B = _pwm1B;
}
//****************************************************************************
// set speed
void StepperLab3::setSpeed(int speed){

	_f_int=0;
	while(_f_int==0); // critical section, wait untlt interrupt is done
	// remap value
	if (speed==0) speed++;
	_setSpeed =  speed;
	_dspeed =  speed;
}
//****************************************************************************
// set ramp steps
void StepperLab3::setRampSteps(int step){

	_f_int=0;
	while(_f_int==0); // critical section, wait untlt interrupt is done
	_rampSteps= step;

}
//****************************************************************************
// set relative steps
void StepperLab3::relativeSteps(int step){

	_f_int=0;
	while(_f_int==0); // critical section, wait untlt interrupt is done
	_f_step=0;
	_absoluteStep = _absoluteStep + step;
	_stepReadyStatus = 0;
	_continues = 0;
	_stepsMade=0;
	_f_ramp=0;

	if (_rampSteps == 0) _rampSteps= 0;
	_dspeed =  _setSpeed;
	
	_rampSpeed= _dspeed / _rampSteps;
	_stepsMade=0;
	_f_ramp=0;
			
	if (_rampSteps !=0) { 
		_f_ramp=1; 
		_differenceSteps = abs(_absoluteStep - _currentStep);
		_rampThres=0;
		if (_differenceSteps <= _rampSteps * 2){
			_rampThres= _differenceSteps / 2;
		}
	}

}
//****************************************************************************
// set absolute steps
void StepperLab3::absoluteSteps(int step){
	// critical section, wait untlt interrupt is done

	_f_int=0;
	 while(_f_int==0);
	 _f_step=0;
	
	_absoluteStep = step;
	_stepReadyStatus = 0; 
	_continues = 0;
	
	if (_rampSteps == 0) _rampSteps= 0;
	_dspeed =  _setSpeed;
	
	_rampSpeed= _dspeed / _rampSteps;
	_stepsMade=0;
	_f_ramp=0;
	if (_rampSteps !=0) { 
		_f_ramp=1; 
		_differenceSteps = abs(_absoluteStep - _currentStep);
		_rampThres=0;
		if (_differenceSteps <= _rampSteps * 2){
			_rampThres= _differenceSteps / 2;
		}
	}



}
//****************************************************************************
// rotate continuesly, right or left
void  StepperLab3::rotate(int direction){
	// critical section, wait untlt interrupt is done
	_f_int=0;
	while(_f_int==0);
	_rampSteps= 0;
	_continues = direction;
}
//****************************************************************************
// set reference position
void  StepperLab3::setRefPosition( int apos){
	_f_int=0;
	 while(_f_int==0);
	
	_absoluteStep = apos;
	_currentStep = apos;
	
	_stepReadyStatus = 0; 
	_stepsMade=0;
}
//****************************************************************************
// get steps
int StepperLab3::getSteps(void){
    _f_int=0;
    while(_f_int==0); // critical section, wait untlt interrupt is done
	return _currentStep;
}
//****************************************************************************
// get status step ready
int StepperLab3::stepReady(void){

	_f_int=0;
	while(_f_int==0); // critical section, wait untlt interrupt is done
	return _stepReadyStatus;
}

//****************************************************************************
// debugger
int StepperLab3::debugger(void){

	if(_f_step==1) {
		Serial.print("a:" );	
		Serial.print(_absoluteStep );
		Serial.print(" c:" );	
		Serial.print(_currentStep );
		Serial.print(" d:" );
		Serial.print(_differenceSteps );
		Serial.print(" sm:" );
		Serial.print(_stepsMade );
		Serial.print(" v:" );
		Serial.print(_dspeed );
		if (_f_ramp==1) Serial.print(" + " );
		if (_f_ramp==2) Serial.print(" . " );
		if (_f_ramp==3) Serial.print(" - " );
		
		Serial.println("  " );
		
	}
	_f_step=0;
}

//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************

SIGNAL (TIMER1_COMPA_vect) { 
	
	//	bitWrite(PORTD,6,1);  // Test Output  PD7
	
_ddsreg= _ddsreg+_dspeed;

if (_ddsreg >= 156250)  { //  speed calulation
	_ddsreg=0;
 	
	// bitWrite(PORTD,7,!bitRead(PORTD,7));  // toggle Test Output  PD7
		if (_continues != 0 )_absoluteStep=_absoluteStep+_continues ; // motor free run
		_differenceSteps = _absoluteStep - _currentStep;

	// just if there is a difference / steps to go
	if(_differenceSteps != 0){
		_stepsMade++;
		// ramp function for acceleration	
		if (_f_ramp!= 0){
			switch (_f_ramp){
				case 1:
					_dspeed =  (_rampSpeed  * _stepsMade) ;
					 if (_stepsMade > _rampThres && _rampThres !=0 )_f_ramp=3;
					 if (_stepsMade == _rampSteps) _f_ramp=2;
					break;
				case 2:
					_dspeed=_setSpeed;
					if (abs(_differenceSteps)  <= _rampSteps) _f_ramp=3;
					break;
					
				case 3:
					_dspeed = (_rampSpeed  * abs(_differenceSteps)  ) ;
					break;
			}
	
		}
		// reset status step ready
		_stepReadyStatus = 0; 

		if(_differenceSteps > 0) _directionStepper = 1; // set direction
		if(_differenceSteps < 0) _directionStepper = 0; // set direction
		

		// check direction
		if (_directionStepper == 0){
			_localStep--;
			_currentStep--;
		}
		else {
		_localStep++;
		_currentStep++;
		}
		
		// check if half step or full step
		if(_stepperMode == 0){

			if (_localStep > 4)_localStep = 1;
			if (_localStep < 1)_localStep = 4;
	

	    	// check which phase
			switch (_localStep) {
			case 1:
			  digitalWrite(_motorPin1, LOW);
			  digitalWrite(_motorPin2, LOW);
			  digitalWrite(_motorPin3, LOW);
			  digitalWrite(_motorPin4, HIGH);
			  break;
			case 2:
			  digitalWrite(_motorPin1, LOW);
			  digitalWrite(_motorPin2, HIGH);
			  digitalWrite(_motorPin3, LOW);
			  digitalWrite(_motorPin4, LOW);
			  break;
			case 3:
			  digitalWrite(_motorPin1, LOW);
			  digitalWrite(_motorPin2, LOW);
			  digitalWrite(_motorPin3, HIGH);
			  digitalWrite(_motorPin4, LOW);
			  break;
			case 4:
			  digitalWrite(_motorPin1, HIGH);
			  digitalWrite(_motorPin2, LOW);
			  digitalWrite(_motorPin3, LOW);
			  digitalWrite(_motorPin4, LOW);
			  break;
			}
		} // Stepper Mode 0
		
		 if(_stepperMode != 0){
			// limes
			if (_localStep > 8) _localStep = 1;
			if (_localStep < 1) _localStep = 8;

			// check which phase
			switch (_localStep) {
			case 1:

			digitalWrite(_motorPin1, LOW);
			digitalWrite(_motorPin2, LOW);
			digitalWrite(_motorPin3, LOW);
			digitalWrite(_motorPin4, HIGH);
			_pwm1A = _pwmValue;
			_pwm1B = _pwmValue;
			break;
			case 2:

			digitalWrite(_motorPin1, LOW);
			digitalWrite(_motorPin2, HIGH);
			digitalWrite(_motorPin3, LOW);
			digitalWrite(_motorPin4, HIGH);
			_pwm1A = _pwm23Value;
			_pwm1B = _pwm23Value;
			break;
			case 3:

			digitalWrite(_motorPin1, LOW);
			digitalWrite(_motorPin2, HIGH);
			digitalWrite(_motorPin3, LOW);
			digitalWrite(_motorPin4, LOW);
			_pwm1A = _pwmValue;
			_pwm1B = _pwmValue;
			break;
			case 4:

			digitalWrite(_motorPin1, LOW);
			digitalWrite(_motorPin2, HIGH);
			digitalWrite(_motorPin3, HIGH);
			digitalWrite(_motorPin4, LOW);
			_pwm1A = _pwm23Value;
			_pwm1B = _pwm23Value;
			break;
			case 5:

			digitalWrite(_motorPin1, LOW);
			digitalWrite(_motorPin2, LOW);
			digitalWrite(_motorPin3, HIGH);
			digitalWrite(_motorPin4, LOW);
			_pwm1A = _pwmValue;
			_pwm1B = _pwmValue;
			break;
			case 6:

			digitalWrite(_motorPin1, HIGH);
			digitalWrite(_motorPin2, LOW);
			digitalWrite(_motorPin3, HIGH);
			digitalWrite(_motorPin4, LOW);
			_pwm1A = _pwm23Value;
			_pwm1B = _pwm23Value;
			break;
			case 7:

			digitalWrite(_motorPin1, HIGH);
			digitalWrite(_motorPin2, LOW);
			digitalWrite(_motorPin3, LOW);
			digitalWrite(_motorPin4, LOW);
			_pwm1A = _pwmValue;
			_pwm1B = _pwmValue;
			break;
			case 8:

			digitalWrite(_motorPin1, HIGH);
			digitalWrite(_motorPin2, LOW);
			digitalWrite(_motorPin3, LOW);
			digitalWrite(_motorPin4, HIGH);
			_pwm1A = _pwm23Value;
			_pwm1B = _pwm23Value;
			break;
			}
			OCR1A = _pwm1A;
			OCR1B = _pwm1B;
		 }	
		_f_step=1;
	}else{  // if difference steps
	_stepReadyStatus = 1; // set status step ready
	} // end if difference steps
	
} // end if step timingr
_f_int=1;  // flag interrupt service done

// bitWrite(PORTD,6,0);  // Test Output  PD7

}



