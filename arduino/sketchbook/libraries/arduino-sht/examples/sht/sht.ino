#include <Wire.h>
#include "SHTSensor.h"

// initialize sht with default values
SHTI2cSensor sht;

void setup() {

  Serial.begin(115200);
  Serial.println("Started");
  
  Wire.begin();
  //digitalWrite( SDA, HIGH);
  //digitalWrite( SCL, HIGH);

  // change if required default REPEATABILITY and MPS
  //sht.setAccuracy(SHTI2cSensor::SHT_REPEATABILITY_MEDIUM);

  //reset and clear status
  sht.softReset();
  delay(10);
  sht.clearStatusRegister();
  
  delay(3000);

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

  delay(3000);

  // start periodic mode
  sht.periodicDataAcquisition();

  // use this to check the sensor warming it
  // but this activate alert in checkStatus
  // activate heater
  //sht.heaterEnable();
  
}

void loop() {

  
  if (sht.fetchData()){                     // query data
    if (sht.getValues()) {                  // if data available get it
      Serial.print("\nSHT periodic:\n");
      Serial.print("  RH: ");
      Serial.println(sht.getHumidity(), 2);
      Serial.print("  T:  ");
      Serial.println(sht.getTemperature(), 2);

      if(!sht.checkStatus()){
	Serial.println("there are some problems on the sensor");
      }
      
    } else {
      Serial.println("wait");               // no data available
    }
  }else{
    Serial.println("Error");    
  }
  
  delay(100); // do not go tired

  //sht.stopPeriodicDataAcquisition();

}
