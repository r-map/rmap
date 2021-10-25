// Sweep
// by BARRAGAN <http://barraganstudio.com>

// http://arduiniana.org/libraries/pwmservo/

//   Board                     SERVO_PIN_A   SERVO_PIN_B   SERVO_PIN_C
//   -----                     -----------   -----------   -----------
//   Arduino Uno, Duemilanove       9            10          (none)
//   Arduino Mega                  11            12            13
//   Sanguino                      13            12          (none)
//   Teensy 1.0                    17            18            15
//   Teensy 2.0                    14            15             4
//   Teensy++ 1.0 or 2.0           25            26            27
//   Teensy LC & 3.x                 (all PWM pins are usable)

#include <PWMServo.h>

PWMServo myservo;  // create servo object to control a servo

int pos = 0;    // variable to store the servo position

void setup() {
  myservo.attach(SERVO_PIN_A);  // attaches the servo on pin 9 to the servo object
  //myservo.attach(SERVO_PIN_A, 1000, 2000); // some motors need min/max setting
}


void loop() {
  for(pos = 0; pos < 180; pos += 1) { // goes from 0 degrees to 180 degrees, 1 degree steps
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for(pos = 180; pos>=1; pos-=1) {   // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}
