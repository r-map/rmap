/*
example to use one h bridge with pwm with brake, clock wise and reverse
*/


#include <i2cibt_2.h>
#include <i2cgpio.h>

i2cgpio gpio(I2C_PWM_DEFAULTADDRESS);
i2cibt_2 mybridge(IBT_2_FULL,gpio);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.notice(F("Started" CR));  
}

void loop() {

  // activate the bridge
  mybridge.start();
  if (mybridge.protect()) Log.notice(F("ALARM!!" CR)); 
  delay(5000);
  if (mybridge.protect()) Log.notice(F("ALARM!!" CR)); 
  
  // dimmer from min to max to min clock wise
  int new_value= 0;
  int inc =1;
  while (new_value >= 0){
    if (new_value == 255) inc =-1;
    Serial.println(new_value);
    mybridge.setrotation(new_value,CW);
    if (mybridge.protectdelay()) Log.notice(F("ALARM!!" CR)); 
    Log.notice(F("Current: %d A" CR),mybridge.get(bridge_r_half)*0.0415);
    new_value+= inc;
  }

  // dimmer from min to max to min reverse
  new_value= 0;
  inc =1;
  while (new_value >= 0){
    if (new_value == 255) inc =-1;
    Serial.println(new_value);
    mybridge.setrotation(new_value,CCW);
    if (mybridge.protectdelay()) Log.notice(F("ALARM!!" CR)); 
    Log.notice(F("Current: %d A" CR),mybridge.get(bridge_r_half)*0.0415);
    new_value+= inc;
  }

  // brake to gnd
  mybridge.brake();
  delay(5000);
  mybridge.stop();
  
}


