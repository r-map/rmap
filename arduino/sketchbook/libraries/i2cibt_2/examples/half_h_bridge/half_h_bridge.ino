/*
example to use two half bridge for pwm output or solid state power switch
*/

#include <i2cibt_2.h>

i2cgpio gpio(I2C_PWM_DEFAULTADDRESS);
i2cibt_2 hbridge(IBT_2_2HALF,gpio);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.notice(F("Started" CR));    
}

void loop() {

  // activate the two half bridge
  hbridge.start();
  if (hbridge.protect()) Log.notice(F("ALARM!!" CR)); 
  delay(5000);
  if (hbridge.protect()) Log.notice(F("ALARM!!" CR)); 



  // dimmer from min to max to min half bridge right
  int new_value= 0;
  int inc =1;
  while (new_value >= 0){
    if (new_value == 255) inc =-1;
    Serial.println(new_value);
    hbridge.setpwm(new_value,IBT_2_R_HALF);
    if (hbridge.protectdelay()) Log.notice(F("ALARM!!" CR)); 
    Log.notice(F("Current: %d A" CR),hbridge.get(bridge_r_half)*0.0415);
    new_value+= inc;
  }

  // switch off the right half bridge
  hbridge.stop(IBT_2_R_HALF);


  // dimmer from min to max to min half bridge left
  new_value= 0;
  inc =1;
  while (new_value >= 0){
    if (new_value == 255) inc =-1;
    Serial.println(new_value);
    hbridge.setpwm(new_value,IBT_2_L_HALF);
    if (hbridge.protectdelay()) Log.notice(F("ALARM!!" CR)); 
    Log.notice(F("Current: %d A" CR),hbridge.get(bridge_r_half)*0.0415);
    new_value+= inc;
  }

  // switch off the left half bridge
  hbridge.stop(IBT_2_L_HALF);

  delay(10000);

}


