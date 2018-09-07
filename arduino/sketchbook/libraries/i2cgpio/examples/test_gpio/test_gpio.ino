/*
example to use one i2c-pwm with i2cgpio library
*/

#include <i2cgpio.h>

i2cgpio gpio(I2C_PWM_DEFAULTADDRESS);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.notice(F("Started" CR));  
}


void loop() {

  Log.notice(F("digitalWrite" CR));  
  gpio.digitalWrite(1,HIGH);
  gpio.digitalWrite(2,LOW);
  delay(3000);
  Log.notice(F("digitalWrite" CR));  
  gpio.digitalWrite(1,LOW);
  gpio.digitalWrite(2,HIGH);

  Log.notice(F("analogWrite" CR));  
  gpio.analogWrite(1,0);
  gpio.analogWrite(2,254);
  delay(3000);
  Log.notice(F("analogWrite" CR));  
  gpio.analogWrite(1,127);
  gpio.analogWrite(2,127);
  delay(3000);
  Log.notice(F("analogWrite" CR));  
  gpio.analogWrite(1,254);
  gpio.analogWrite(2,0);

  Log.notice(F("PIN 1 value: %d" CR),gpio.analogRead(1));  
  Log.notice(F("PIN 2 value: %d" CR),gpio.analogRead(2));  
  
}
