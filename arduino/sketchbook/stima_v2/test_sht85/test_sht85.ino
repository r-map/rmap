#include <Wire.h>
#include "SHTSensor.h"

SHTI2cSensor sht;

void setup() {

  Serial.begin(115200);
  
  Wire.begin();
  //digitalWrite( SDA, HIGH);
  //digitalWrite( SCL, HIGH);
  
  //sht.setAccuracy(SHTI2cSensor::SHT_ACCURACY_MEDIUM);
  sht.softreset();
  delay(10);
  sht.clearstatusregister();
}

void loop() {
  
  if (sht.readSample()) {
    Serial.print("SHT:\n");
    Serial.print("  RH: ");
    Serial.print(sht.getHumidity(), 2);
    Serial.print("\n");
    Serial.print("  T:  ");
    Serial.print(sht.getTemperature(), 2);
    Serial.print("\n");
  } else {
    Serial.print("Error in readSample()\n");
  }
  
  delay(1000);
}
