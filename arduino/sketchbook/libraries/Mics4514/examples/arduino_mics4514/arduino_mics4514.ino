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
  Serial.println("Hello");

  sensor.blocking_fast_heat();

}

void loop()
{
    int co, no2;
    bool ok;

    ok = sensor.query_data_auto(&co, &no2, SAMPLES);

    if (ok) {
      Serial.print("CO  :");
      Serial.println(co); 
      Serial.print("NO2 :");
      Serial.println(no2); 
    } else {
      Serial.println(F("Error getting data from SENSOR!"));
    }

    delay(10000);
}
