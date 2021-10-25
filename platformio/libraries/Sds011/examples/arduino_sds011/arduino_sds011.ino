#include <SoftwareSerial.h>
#include "Sds011.h"


static const int PM25_NORM=25;
static const int PM10_NORM=40;
static const int SAMPLES=3;

// RX, TX
SoftwareSerial mySerial(D5,D6);
sds011::Sds011 sensor(mySerial);
//sds011::Sds011 sensor(Serial1);

String val_to_str(uint16_t v)
{
    String r;

    r = String(v/10);
    if (v < 1000 && v%10) {
        r += String(".") + String(v%10);
    }

    for (int i = 4 - r.length(); i > 0; i--) {
        r = String(" ") + r;
    }

    return r;
}


void display_data(uint16_t pm25, uint16_t pm10)
{

  Serial.println(F("\n\npm\t 2.5\t 10"));

  Serial.print("ug\t");
  Serial.print(val_to_str(pm25).c_str());
  Serial.print("\t");
  Serial.println(val_to_str(pm10).c_str());

  Serial.print("%\t");
  Serial.print(val_to_str((10*pm25/PM25_NORM)*10).c_str());
  Serial.print("\t");
  Serial.println(val_to_str((10*pm10/PM10_NORM)*10).c_str());
}


void setup()
{

    Serial.begin(115200);
    mySerial.begin(9600);

    Serial.println("Hello");
    
    delay(1000);
    
    Serial.print("Sds011 firmware version: ");
    Serial.println(sensor.firmware_version());

    sensor.set_mode(sds011::QUERY);
    sensor.set_sleep(sds011::SLEEP);
}

void loop()
{
    int pm25, pm10;

    sensor.set_sleep(sds011::WORK);

    if (sensor.query_data_auto(&pm25, &pm10, SAMPLES)) {
      display_data(pm25, pm10);
    } else {
      Serial.println(F("Sensor Error!"));
    }
    sensor.set_sleep(sds011::SLEEP);
    delay(10000);
}
