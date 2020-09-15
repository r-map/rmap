#include "Arduino.h"
#include "canard.h"
#include "uavcan.h"

unsigned long previousMillis = 0;     // stores last time output was updated
const long interval = 1000;           // interval at which to print output (milliseconds)

void setup() {

  Serial.begin(115200);
  Serial.print("Initializing...");
  Serial.println(HAL_RCC_GetHCLKFreq());

  // initialize digital pins
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  CAN_HW_Init();
  uavcanInit();

}

void loop() {

  unsigned long currentMillis;

  currentMillis = millis();

  sendCanard();
  receiveCanard();
  spinCanard();
  publishCanard();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    showRcpwmonUart();
  }

}
