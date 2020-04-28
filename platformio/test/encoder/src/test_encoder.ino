/*
    Rotary Encoder - Polling Example
    
    The circuit:
    * encoder pin A to Arduino pin 5
    * encoder pin B to Arduino pin 6
    * encoder ground pin to ground (GND)
    
*/

#define PINA 5
#define PINB 6

#include <Rotary.h>

Rotary r = Rotary(PINA, PINB);

void setup() {
  Serial.begin(115200);
  Serial.println("Started");
  pinMode(PINA, INPUT_PULLUP);
  pinMode(PINB, INPUT_PULLUP);  

  r.begin();

}

void loop() {

  unsigned char result = r.process();
  if (result) {
    Serial.println(result == DIR_CW ? "Right" : "Left");
  }
}
