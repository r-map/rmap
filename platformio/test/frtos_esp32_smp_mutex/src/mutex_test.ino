/*
  Rui Santos & Sara Santos - Random Nerd Tutorials Complete project
  details at
  https://RandomNerdTutorials.com/esp32-freertos-mutex-arduino/
  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files.  The above copyright notice and this permission notice shall
  be included in all copies or substantial portions of the Software.
*/

#include "mutex.hpp"
#include <frtosLog.h>


using namespace cpp_freertos;


#define MUTEX_TIMEOUT 5000  // 5s timeout

SemaphoreHandle_t serialMutex = NULL;
MutexStandard mutex;

void Task1_frtoslog(void *parameter) {
  for (;;) {
    frtosLog.notice("Task1: Logging from Task    1\n");
    delay(10);
  }
}

void Task2_frtoslog(void *parameter) {
  for (;;) {
    frtosLog.notice("Task2: Logging from Task       2\n");
    delay(1);
  }
}

void Task1_frtos(void *parameter) {
  for (;;) {
    {
      LockGuard guard(mutex);
      Serial.print("Task1: Logging from Task    1\n");
    }
    delay(10);
  }
}

void Task2_frtos(void *parameter) {
  for (;;) {
    {
      LockGuard guard(mutex);
      Serial.print("Task2: Logging from Task       2\n");
    }
    delay(1);
  }
}

void Task1(void *parameter) {
  for (;;) {
    if (xSemaphoreTake(serialMutex, MUTEX_TIMEOUT)) {
      Serial.println("Task1: Logging from Task    1");
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
    Serial.println("Task1: End of log from Task  1");
    xSemaphoreGive(serialMutex);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void Task2(void *parameter) {
  for (;;) {
    if (xSemaphoreTake(serialMutex, MUTEX_TIMEOUT)) {
      Serial.println("Task2: Logging from Task        2");
    }
    vTaskDelay(80 / portTICK_PERIOD_MS);
    LockGuard guard(mutex);
    Serial.println("Task2: End of log from Task      2");
    xSemaphoreGive(serialMutex);
    vTaskDelay(300 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting FreeRTOS");
  frtosLog.begin(LOG_LEVEL_NOTICE, &Serial,mutex);

  frtosLog.notice("ciao ciao");

  /*
  serialMutex = xSemaphoreCreateMutex();
  if (serialMutex == NULL) {
    Serial.println("Failed to create mutex!");
    while (1);
  }
  */
  
  xTaskCreatePinnedToCore(
    Task1_frtoslog,         // Task function
    //Task1_frtos,         // Task function
    //Task1,                  // Task function
    "Task1",                // Task name
    3000,                   // Stack size
    NULL,                   // Task parameters
    1,                      // Priority
    NULL,                   // Task handle
    0                       // Core ID
  );

  xTaskCreatePinnedToCore(
    Task2_frtoslog,         // Task function
    //Task2_frtos,         // Task function
    //Task2,                  // Task function
    "Task2",                // Task name
    3000,                   // Stack size
    NULL,                   // Task parameters
    2,                      // Higher priority
    NULL,                   // Task handle
    1                       // Core ID
  );
}

void loop() {
  delay(1000);
}
