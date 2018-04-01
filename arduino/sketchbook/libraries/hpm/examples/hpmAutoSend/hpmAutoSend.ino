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
  //HPM.init(&hpm_ser, &Serial, HPM_listen);
  Serial.println(HPM.init(&hpm_ser));
  // enable auto send, allows auto sampling at 1s interval
  Serial.println(HPM.startParticleMeasurement());
  Serial.println(HPM.enableAutoSend());
  Serial.println(F("End setup"));  
  
}

void loop() {
  // loop() return true if values for HPM is successfully updated
  if(HPM.loop()){
    Serial.print("PM2.5 val: ");
    Serial.print(HPM.get());
    Serial.println("  ug/m3");
  }
}
