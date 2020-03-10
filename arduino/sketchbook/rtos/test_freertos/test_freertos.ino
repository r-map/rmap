#include <Arduino_FreeRTOS.h>


void setup() {
 
  Serial.begin(112500);
  delay(1000);
  Serial.println("Started");
 
  xTaskCreate(
                    taskOne,          /* Task function. */
                    "TaskOne",        /* String with name of task. */
                    128,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
 
  xTaskCreate(
                    taskTwo,          /* Task function. */
                    "TaskTwo",        /* String with name of task. */
                    128,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */


  // The scheduler was started in initVariant() found in variantHooks.c but in RMAP was moved here
  vTaskStartScheduler(); // initialise and run the freeRTOS scheduler. Execution should never return here.
  
}
 
void loop() {
  // Empty. Things are done in Tasks.
}
 
void taskOne( void * parameter )
{
 
    for( int i = 0;i<10;i++ ){
 
        Serial.println("Hello from task 1");
        delay(1000);
    }
 
    Serial.println("Ending task 1");
    vTaskDelete( NULL );
 
}
 
void taskTwo( void * parameter)
{
 
    for( int i = 0;i<10;i++ ){
 
        Serial.println("Hello from task 2");
        delay(1000);
    }
    Serial.println("Ending task 2");
    vTaskDelete( NULL );
 
}

