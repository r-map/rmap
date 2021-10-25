#include <Wire.h>
#include <HIH61XX.h>


//  Create an HIH61XX with I2C address 0x27, powered by pin 8
HIH61XX hih(0x28, 4);


void setup()
{
  Serial.begin(9600);
  Serial.println("Started");
  Wire.begin();
}


void loop()
{
  //  start the sensor
  hih.start();

  //  request an update of the humidity and temperature
  hih.update();

  Serial.print("Humidity: ");
  Serial.print(hih.humidity(), 5);
  Serial.print(" RH (");
  Serial.print(hih.humidity_Raw());
  Serial.println(")");

  Serial.print("Temperature: ");
  Serial.print(hih.temperature(), 5);
  Serial.print(" C (");
  Serial.print(hih.temperature_Raw());
  Serial.println(")");

  while(1) {
    while(Serial.available()) {
      hih.commandRequest(Serial);
    }
    delay(100);
  }
}
