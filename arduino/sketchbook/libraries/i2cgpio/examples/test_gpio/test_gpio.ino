/*
example to use one i2c-pwm with i2cgpio library
*/

#include <i2cgpio.h>

i2cgpio gpio;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.notice(F("Started" CR));  

  //Start I2C communication routines
  Wire.begin();

  //The Wire library enables the internal pullup resistors for SDA and SCL.
  //You can turn them off after Wire.begin()
  // do not need this with patched Wire library
  //digitalWrite( SDA, LOW);
  //digitalWrite( SCL, LOW);
  digitalWrite( SDA, HIGH);
  digitalWrite( SCL, HIGH);

}


void loop() {

  Log.notice(F("digitalWrite" CR));  
  gpio.digitalWrite(1,HIGH);
  gpio.digitalWrite(2,HIGH);
  delay(3000);

  Log.notice(F("analogWrite" CR));  
  gpio.analogWrite(1,0);
  gpio.analogWrite(2,254);
  delay(3000);

  Log.notice(F("analogWrite" CR));  
  gpio.analogWrite(2,127);
  delay(3000);

  Log.notice(F("analogWrite" CR));  
  gpio.analogWrite(2,0);
  gpio.analogWrite(1,254);
  delay(3000);

  Log.notice(F("analogWrite" CR));  
  gpio.analogWrite(1,127);


  Log.notice(F("digitalWrite" CR));  
  gpio.digitalWrite(1,LOW);
  gpio.digitalWrite(2,LOW);


  Log.notice(F("PIN 1 value: %d" CR),gpio.analogRead(1));  
  Log.notice(F("PIN 2 value: %d" CR),gpio.analogRead(2));  

  delay(6000);
  
}
