//#include "arduino_thread.h"
#include "stimawifi.h"


void loop() {
  webserver.handleClient();
  //MDNS.update();
  // sometimes ESP32 do not reconnect and we need a restart
  uint16_t counter=0;
  while (WiFi.status() != WL_CONNECTED) { //lost connection
    frtosLog.error(F("WIFI disconnected!"));
    if (oledpresent){
      u8g2.clearBuffer();
      u8g2.setCursor(0, 20); 
      u8g2.print(F("WIFI KO"));
      u8g2.sendBuffer();
    }
    if(counter++>=300) reboot(); //300 seconds timeout - reset board
    delay(1000);
  }
  Alarm.delay(0);
}
