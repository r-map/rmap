#include <hpm.h>
#include <SoftwareSerial.h>
SoftwareSerial hpm_ser(D5, D6);

hpm HPM;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  hpm_ser.begin(9600);

  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.notice(F("Started" CR));  
  // set uart for hpm, and uart for debug
  // set listen function if one or more SoftwareSerials are used
  HPM.init(&hpm_ser);
  HPM.startParticleMeasurement();
}

void loop() {
  // put your main code here, to run repeatedly:

  // update values at every 5s interval

  HPM.readParticleMeasuringResults();
  uint16_t val = HPM.get(PM25_TYPE)
  Serial.print(F("PM2.5 val: ")); 
  Serial.print(val);
  Serial.println(F("  ug/m3"));
  val = HPM.get(PM10_TYPE)
  Serial.print(F("PM10  val: ")); 
  Serial.print(val);
  Serial.println(F("  ug/m3"));

  delay(5000);
}
