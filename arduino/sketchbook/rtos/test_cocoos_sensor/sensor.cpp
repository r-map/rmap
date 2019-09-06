#include <Arduino.h>
#include <Wire.h>
#include "SHTSensor.h"

// initialize sht with default values
SHTI2cSensor sht;

#ifdef __cplusplus ////
extern "C" {
#endif ////


void sensor_setup() {

  Serial.begin(115200);
  Serial.println("Started");

  Serial.println("Started");
  
  Wire.begin();
  //reset and clear status
  sht.softReset();
  //delay(10);
  sht.clearStatusRegister();

  Serial.println("en setup");

}  

void sensor_prepare() {
  Serial.println("prepare");
}

void sensor_get() {
  Serial.println("get");  
  // do and read one measure in blocking mode
  if (sht.readSample()) {
    Serial.println("SHT single shot:");
    Serial.print("  RH: ");
    Serial.println(sht.getHumidity(), 2);
    Serial.print("  T:  ");
    Serial.println(sht.getTemperature(), 2);
  } else {
    Serial.print("Error in readSample()\n");
  }
}  

#ifdef __cplusplus ////
}
#endif ////
