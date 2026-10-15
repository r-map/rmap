/*
  Rui Santos & Sara Santos - Random Nerd Tutorials Complete project
  details at
  https://RandomNerdTutorials.com/esp32-freertos-mutex-arduino/
  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files.  The above copyright notice and this permission notice shall
  be included in all copies or substantial portions of the Software.
*/

/*
  COREID_TASK1 1 COREID_TASK2 1   Work
  COREID_TASK1 0 COREID_TASK2 1   Do not work
  COREID_TASK1 0 COREID_TASK2 0   Do not work

  check with:
  pio device monitor |grep -v 123456789012345678901234567890
*/

#define COREID_TASK1 0
#define COREID_TASK2 1

#include "mutex.hpp"
#include <frtosLog.h>

using namespace cpp_freertos;

SemaphoreHandle_t serialMutex = NULL;
MutexStandard mutex;

void Task1_frtoslog(void *parameter) {
  for (;;) {
    frtosLog.notice("123456789012345678901234567890\n");
    delay(random(0,5));
  }
}

void Task2_frtoslog(void *parameter) {
  for (;;) {
    frtosLog.notice("123456789012345678901234567890\n");
    delay(random(0,5));
  }
}

void Task1_frtos(void *parameter) {
  for (;;) {
    {
      LockGuard guard(mutex);
      Serial.print("123456789012345678901234567890\n");
    }
    delay(random(0,5));
  }
}

void Task2_frtos(void *parameter) {
  for (;;) {
    {
      LockGuard guard(mutex);
      Serial.print("123456789012345678901234567890\n");
    }
    delay(random(0,5));
  }
}

void Task1(void *parameter) {
  for (;;) {
    if (xSemaphoreTake(serialMutex,portMAX_DELAY)) {
      Serial.print("123456789012345678901234567890\n");
    }else{
      Serial.println("error");
    }
    xSemaphoreGive(serialMutex);
    vTaskDelay(random(0,5) / portTICK_PERIOD_MS);
  }
}

void Task2(void *parameter) {
  for (;;) {
    if (xSemaphoreTake(serialMutex,portMAX_DELAY)) {
      Serial.print("123456789012345678901234567890\n");
    }else{
      Serial.println("error");
    }
    xSemaphoreGive(serialMutex);
    vTaskDelay(random(0,5) / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("Starting FreeRTOS");
  frtosLog.begin(LOG_LEVEL_NOTICE, &Serial,mutex);

  frtosLog.notice("ciao ciao\n");

  serialMutex = xSemaphoreCreateMutex();
  if (serialMutex == NULL) {
    Serial.println("Failed to create mutex!");
    while (1);
  }
  
  xTaskCreatePinnedToCore(
			  //Task1_frtoslog,         // Task function
			  //Task1_frtos,            // Task function
			  Task1,                  // Task function
			  "Task1",                // Task name
			  10000,                  // Stack size
			  NULL,                   // Task parameters
			  1,                      // Priority
			  NULL,                   // Task handle
			  COREID_TASK1            // Core ID
			  );
  
  xTaskCreatePinnedToCore(
			  //Task2_frtoslog,         // Task function
			  //Task2_frtos,            // Task function
			  Task2,                  // Task function
			  "Task2",                // Task name
			  10000,                  // Stack size
			  NULL,                   // Task parameters
			  1,                      // Priority
			  NULL,                   // Task handle
			  COREID_TASK2            // Core ID
			  );
}

void loop() {
  delay(1000);
}
