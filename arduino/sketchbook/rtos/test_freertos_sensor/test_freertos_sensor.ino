#include <Arduino_FreeRTOS.h>
#include <semphr.h>  // add the FreeRTOS functions for Semaphores (or Flags).
#include <Wire.h>
#include "SHTSensor.h"

// Declare a mutex Semaphore Handle which we will use to manage the Serial Port.
// It will be used to ensure only only one Task is accessing this resource at any time.
SemaphoreHandle_t xSensorSemaphore;

// initialize sht with default values
SHTI2cSensor sht;


void setup() {
 
  Serial.begin(112500);
  delay(1000);
  Serial.println("Started");

  // Semaphores are useful to stop a Task proceeding, where it should be paused to wait,
  // because it is sharing a resource, such as the Serial port.
  // Semaphores should only be used whilst the scheduler is running, but we can set it up here.
  if ( xSensorSemaphore == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
  {
    xSensorSemaphore = xSemaphoreCreateMutex();  // Create a mutex semaphore we will use to manage the Serial Port
    if ( ( xSensorSemaphore ) != NULL )
      xSemaphoreGive( ( xSensorSemaphore ) );  // Make the Serial Port available for use, by "Giving" the Semaphore.
  }
  
  Wire.begin();
  //reset and clear status
  sht.softReset();
  delay(10);
  sht.clearStatusRegister();

  
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
 
}
 
void loop() {
  // Empty. Things are done in Tasks.
}
 
void taskOne( void * parameter )
{

  for (;;) {
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second

    // See if we can obtain or "Take" the Serial Semaphore.
    // If the semaphore is not available, wait 5 ticks of the Scheduler to see if it becomes free.
    if ( xSemaphoreTake( xSensorSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      // We were able to obtain or "Take" the semaphore and can now access the shared resource.

      // do and read one measure in blocking mode
      if (sht.readSample()) {
	Serial.println("1 SHT single shot:");
	Serial.print("1   RH: ");
	Serial.println(sht.getHumidity(), 2);
	Serial.print("1   T:  ");
	Serial.println(sht.getTemperature(), 2);
      } else {
	Serial.print("1 Error in readSample()\n");
	break;
      }
      xSemaphoreGive( xSensorSemaphore ); // Now free or "Give" the Serial Port for others.
    }
  }
  
  Serial.println("Ending task 1");
  vTaskDelete( NULL );
}
 
void taskTwo( void * parameter)
{
 
  for (;;) {

    // See if we can obtain or "Take" the Serial Semaphore.
    // If the semaphore is not available, wait 5 ticks of the Scheduler to see if it becomes free.
    if ( xSemaphoreTake( xSensorSemaphore, ( TickType_t ) 5 ) == pdTRUE )
      {
	// We were able to obtain or "Take" the semaphore and can now access the shared resource.
	// do and read one measure in blocking mode
	if (sht.readSample()) {
	  Serial.println("2 SHT single shot:");
	  Serial.print("2   RH: ");
	  Serial.println(sht.getHumidity(), 2);
	  Serial.print("2   T:  ");
	  Serial.println(sht.getTemperature(), 2);
	} else {
	  Serial.print("2 Error in readSample()\n");
	  break;
	}
	xSemaphoreGive( xSensorSemaphore ); // Now free or "Give" the Serial Port for others.
      }      
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  }
  
  Serial.println("Ending task 2");
  vTaskDelete( NULL );
}

