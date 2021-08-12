/* Simple example for FreeRTOS

 with  preemptive scheduler
 a mutex for serial port is missed
 so serial port output should be corrupted

 use cooperative scheduler !

*/

#ifdef ARDUINO_ARCH_AVR
  #include <Arduino_FreeRTOS.h>
#else 
  #ifdef ARDUINO_ARCH_STM32
    #include "STM32FreeRTOS.h"
  #else
    #include "FreeRTOS.h"
  #endif
#endif

// Dimensions the buffer that the task being created will use as its stack.
//   NOTE:  This is the number of words the stack will hold, not the number of
//   bytes.  For example, if each stack item is 32-bits, and this is set to 100,
//   then 400 bytes (100 * 32-bits) will be allocated.
#define STACK_SIZE_ONE 120

// Structure that will hold the TCB of the task being created.
StaticTask_t xTaskBufferOne;

// Buffer that the task being created will use as its stack.  Note this is
//   an array of StackType_t variables.  The size of StackType_t is dependent on
//   the RTOS port.
StackType_t xStackOne[ STACK_SIZE_ONE ];

#define STACK_SIZE_TWO 120
StaticTask_t xTaskBufferTwo;
StackType_t xStackTwo[ STACK_SIZE_TWO ];



UBaseType_t freeStack ()
{
  return  uxTaskGetStackHighWaterMark( NULL );
  //return xPortGetFreeHeapSize();
}

void taskOne( void * parameter )
{
 
    for( int i = 0;i<10;i++ ){
 
        Serial.println("Hello from task 1");
	Serial.print(F("#free stack on task 1: "));
	Serial.println(freeStack());
	vTaskDelay(1000/portTICK_PERIOD_MS);
	//taskYIELD();
	//delay(1000);
    }
 
    Serial.println("Ending task 1");
    vTaskDelete( NULL );
 
}

void taskTwo( void * parameter)
{
 
    for( int i = 0;i<10;i++ ){
 
        Serial.println("Hello from task 2");
	Serial.print(F("#free stack on task 2: "));
	Serial.println(freeStack());
	vTaskDelay(1500/portTICK_PERIOD_MS);
	//taskYIELD();
	//delay(1500);
    }
    Serial.println("Ending task 2");
    vTaskDelete( NULL );
 
}


void setup() {
 
  Serial.begin(112500);
  delay(1000);
  Serial.println("Started");
  Serial.print(F("#free stack on start: "));
  Serial.println(freeStack());

  /*
  xTaskCreate(
	      taskOne,          // Task function.
	      "TaskOne",        // String with name of task.
	      120,              // Stack size in bytes.
	      NULL,             // Parameter passed as input of the task
	      1,                // Priority of the task.
	      NULL);            // Task handle.

  xTaskCreate(
	      taskTwo,          // Task function.
	      "TaskTwo",        // String with name of task.
	      120,              // Stack size in bytes.
	      NULL,             // Parameter passed as input of the task
	      1,                // Priority of the task.
	      NULL);            // Task handle.
  */

  xTaskCreateStatic(taskOne, "taskOne", STACK_SIZE_ONE, NULL, 1, xStackOne, &xTaskBufferOne);
  xTaskCreateStatic(taskTwo, "taskTwo", STACK_SIZE_TWO, NULL, 1, xStackTwo, &xTaskBufferTwo);
  
  Serial.print(F("#free stack before start scheduler: "));
  Serial.println(freeStack());
  
  // The scheduler was started in initVariant() found in variantHooks.c but in RMAP was moved here
  vTaskStartScheduler(); // initialise and run the freeRTOS scheduler. Execution should never return here.
  
}
 
void loop() {
  //Serial.println(F("#loop"));
  // Empty. Things are done in Tasks.
}
 
