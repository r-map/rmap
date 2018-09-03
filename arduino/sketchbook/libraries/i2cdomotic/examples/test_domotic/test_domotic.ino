/*
example to use one i2c-pwm with i2cdomotic library
*/

#include <i2cdomotic.h>

i2cdomotic domotic(I2C_PWM_DEFAULTADDRESS);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.notice(F("Started" CR));  
}


void loop() {

  Log.notice(F("digitalWrite" CR));  
  domotic.digitalWrite(1,HIGH);
  domotic.digitalWrite(2,LOW);
  delay(3000);
  Log.notice(F("digitalWrite" CR));  
  domotic.digitalWrite(1,LOW);
  domotic.digitalWrite(2,HIGH);

  Log.notice(F("analogWrite" CR));  
  domotic.analogWrite(1,0);
  domotic.analogWrite(2,254);
  delay(3000);
  Log.notice(F("analogWrite" CR));  
  domotic.analogWrite(1,127);
  domotic.analogWrite(2,127);
  delay(3000);
  Log.notice(F("analogWrite" CR));  
  domotic.analogWrite(1,254);
  domotic.analogWrite(2,0);

  Log.notice(F("PIN 1 value: %d" CR),domotic.analogRead(1));  
  Log.notice(F("PIN 2 value: %d" CR),domotic.analogRead(2));  
  
}
