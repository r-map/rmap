#include <Arduino.h> ////
#include "sensor.h" ////
#include "display.h"
#include <string.h>
#include <cocoos.h>
#include <stdio.h>

static const char *tempdata;
static uint8_t _x = 1;
static uint8_t _y = 2;
static uint8_t _z = 3;

DisplayMsg_t displayMessages[10];

static void update() {
  // print temp data
  Serial.println(tempdata); ////

  // print gyro data
  Serial.print("Gyro: ");
  Serial.print(_x);
  Serial.print(", ");  
  Serial.print(_y);
  Serial.print(", ");  
  Serial.println(_z);
}

static void updateData(uint8_t id, const char *data) {
  if (id == TEMP_DATA) {
    tempdata = data;
  }
  else if (id == GYRO_DATA) {
    _x = data[0];
    _y = data[1];
    _z = data[2];
  }
}
static Display_t display = {
    .update = &update,
    .updateData = &updateData
};

Display_t *display_get(void) {
  return &display;
}

void display_init(void) {
  debug("display init");
}


