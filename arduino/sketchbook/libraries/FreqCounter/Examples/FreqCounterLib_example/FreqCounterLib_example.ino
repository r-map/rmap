// Frequency Counter Lib example

/*
  Martin Nawrath KHM LAB3
  Kunsthochschule f¸r Medien Kˆln
  Academy of Media Arts
  http://www.khm.de
  http://interface.khm.de/index.php/labor/experimente/	
 */

#define SAMPLETIME 3
#define ANALOGPIN 0

#include <FreqCounter.h>

unsigned long frq;
int cnt;
int pinLed=13;

void setup() {
  pinMode(pinLed, OUTPUT);
  Serial.begin(9600);        // connect to the serial port
  Serial.println("Frequency Counter");
}

void loop() {
  // wait if any serial is going on
  //FreqCounter::f_comp=10;   // Cal Value / Calibrate with professional Freq Counter
  FreqCounter::start(round(SAMPLETIME * float(1000)));  // 10000 ms Gate Time

  Serial.println (analogRead(ANALOGPIN));    // read the input pin

  while (FreqCounter::f_ready == 0) {
    //    Serial.print("*");
  }

  frq=FreqCounter::f_freq/SAMPLETIME;

  Serial.println("");
  Serial.print(cnt++);
  Serial.print("  Freq: ");
  Serial.println(frq);
  digitalWrite(pinLed,!digitalRead(pinLed));  // blink Led
}  
