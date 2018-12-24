/*
    Rotary Encoder - Polling Example
    
    The circuit:
    * encoder pin A to Arduino pin 2
    * encoder pin B to Arduino pin 3
    * encoder ground pin to ground (GND)
    
*/

#include <Rotary.h>

Rotary r = Rotary(D6, D7);

void setup() {
  Serial.begin(115200);
  Serial.println("Started");
  r.begin();

}

void loop() {

  unsigned char result = r.process();
  if (result) {
    Serial.println(result == DIR_CW ? "Right" : "Left");
  }
}
