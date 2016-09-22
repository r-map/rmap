#include <SoftwareSerial.h>
#include "Sds011.h"
//#include "Pcd8544.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

static const int PM25_NORM=25;
static const int PM10_NORM=40;
static const int SAMPLES=10;

#ifdef ESP8266
sds011::Sds011 sensor(Serial);
pcd8544::Pcd8544 display(13, 12, 14);
#else
// RX, TX
//SoftwareSerial mySerial(8,9);
//sds011::Sds011 sensor(mySerial);
sds011::Sds011 sensor(Serial1);
//pcd8544::Pcd8544 display(A3, A2, A1, A0, 13);
#endif

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

/*
void display_data(uint16_t pm25, uint16_t pm10)
{
    display.clear();
    display.setCursor(0, 0);

    display.println("    2.5   10");

    display.print("ug ");
    display.print(val_to_str(pm25).c_str());

    display.setCursor(8*7, 1);
    display.println(val_to_str(pm10).c_str());

    display.print("%  ");
    display.print(val_to_str((10*pm25/PM25_NORM)*10).c_str());
    display.setCursor(8*7, 2);
    display.print(val_to_str((10*pm10/PM10_NORM)*10).c_str());
}
*/

void setup()
{
    bool clear = true;

#ifndef ESP8266
    //mySerial.begin(9600);
    Serial1.begin(9600);
#endif
    Serial.begin(9600);

#ifdef ESP8266
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    WiFi.forceSleepBegin(); // Use WiFi.forceSleepWake() to enable wifi

    String r = ESP.getResetReason();

    if (r == "Deep-Sleep Wake") {
        clear = false;
    }
#endif

    /*
    display.begin();
    if (clear) {
        display.clear();
        display.setCursor(0,0);
        display.println("Hello");
    }
    */
    sensor.set_sleep(false);
    sensor.set_mode(sds011::QUERY);
}

void loop()
{
    int pm25, pm10;
    bool ok;

    sensor.set_sleep(false);
    delay(1000);
    ok = sensor.query_data_auto(&pm25, &pm10, SAMPLES);
    sensor.set_sleep(true);

    if (ok) {
      //display_data(pm25, pm10);
      Serial.print(F("pm25: "));
      Serial.println(pm25);

      Serial.print(F("pm10: "));
      Serial.println(pm10);

    } else {
      /*
        display.clear();
        display.setCursor(0, 0);
        display.println("NO SENSOR!");
      */
      Serial.println(F("NO SENSOR!"));
    }

#ifdef ESP8266
    ESP.deepSleep(1000*1000*10, WAKE_RF_DEFAULT);
#else
    delay(60000);
#endif
}
