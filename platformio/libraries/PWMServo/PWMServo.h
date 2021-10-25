#ifndef PWMServo_h
#define PWMServo_h

/*
  PWMServo.h - Hardware Servo Timer Library
  http://arduiniana.org/libraries/pwmservo/
  Author: Jim Studt, jim@federated.com
  Copyright (c) 2007 David A. Mellis.  All right reserved.
  renamed to PWMServo by Mikal Hart
  ported to other chips by Paul Stoffregen

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
*/

#include <inttypes.h>

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) // Arduino
  #define SERVO_PIN_A 9
  #define SERVO_PIN_B 10
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) // Arduino Mega
  #define SERVO_PIN_A 11
  #define SERVO_PIN_B 12
  #define SERVO_PIN_C 13
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__) // Microduino core+ 644
  #define SERVO_PIN_A 9
  #define SERVO_PIN_B 10
#elif defined(__AVR_AT90USB162__) // Teensy 1.0
  #define SERVO_PIN_A 17
  #define SERVO_PIN_B 18
  #define SERVO_PIN_C 15
#elif defined(__AVR_ATmega32U4__) // Teensy 2.0
  #define SERVO_PIN_A 14
  #define SERVO_PIN_B 15
  #define SERVO_PIN_C 4
#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__) // Teensy++
  #define SERVO_PIN_A 25
  #define SERVO_PIN_B 26
  #define SERVO_PIN_C 27
#elif defined(__arm__) && defined(TEENSYDUINO)
  #define SERVO_PIN_A 9
  #define SERVO_PIN_B 10
  // all PWM pins supported, but names not defined here...
  // just use the actual PWM pin number with attach()
#else
  #define SERVO_PIN_A 9
  #define SERVO_PIN_B 10
#endif

class PWMServo
{
  private:
    uint8_t pin;
    uint8_t angle;       // in degrees
    uint8_t min16;       // minimum pulse, 16uS units  (default is 34)
    uint8_t max16;       // maximum pulse, 16uS units, 0-4ms range (default is 150)
#if defined(__AVR__)
    static void seizeTimer1();
    static void releaseTimer1();
    static uint8_t attachedA;
    static uint8_t attachedB;
    #ifdef SERVO_PIN_C
    static uint8_t attachedC;
    #endif
#elif defined(__arm__) && defined(TEENSYDUINO)
    static uint32_t attachedpins[]; // 1 bit per digital pin
#endif
  public:
    PWMServo();
    uint8_t attach(int pinArg) { return attach(pinArg, 544, 2400); }
                             // pulse length for 0 degrees in microseconds, 544uS default
                             // pulse length for 180 degrees in microseconds, 2400uS default
    uint8_t attach(int pinArg, int min, int max);
                             // attach to a pin, sets pinMode, returns 0 on failure, won't
                             // position the servo until a subsequent write() happens
                             // Only works for 9 and 10.
    void detach();
    void write(int angleArg); // specify the angle in degrees, 0 to 180
    uint8_t read() { return angle; }
    uint8_t attached();
};

#endif
