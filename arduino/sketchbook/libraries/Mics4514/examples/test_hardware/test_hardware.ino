#include "Arduino.h"
//#include <SoftwareSerial.h>
#include "Mics4514.h"


#define SCALE1PIN 8
#define SCALE2PIN 9

#define COPIN 1
#define NO2PIN 0
#define HEATERPIN 7

static const int SAMPLES=3;

mics4514::Mics4514 sensor(COPIN,NO2PIN,HEATERPIN,SCALE1PIN,SCALE2PIN);


void setup()
{

  Serial.begin(9600);
  Serial.println("Start");

}

void loop()
{

  Serial.println("normal heat");  
  sensor.normal_heat();
  delay(5000);

  Serial.println("fast heat");  
  sensor.blocking_fast_heat();
  delay(5000);

  Serial.println("scale 1");  
  digitalWrite(SCALE1PIN, HIGH);  
  delay(5000);
  digitalWrite(SCALE1PIN, LOW);  

  Serial.println("scale 2");  
  digitalWrite(SCALE2PIN, HIGH);  
  delay(5000);
  digitalWrite(SCALE2PIN, LOW);  
  
  int co, no2;
  bool ok;
  ok = sensor.query_data_auto(&co, &no2, SAMPLES);
  
  if (ok) {
    Serial.print("CO resistance :");
    Serial.println(co); 
    Serial.print("NO2 resistance:");
    Serial.println(no2); 
  } else {
    Serial.println(F("Error getting data from SENSOR!"));
  }
  
  delay(5000);
}
