#include <Arduino_FreeRTOS.h>
#include <semphr.h>  // add the FreeRTOS functions for Semaphores (or Flags).
#include <Wire.h>
#include "SHTSensor.h"
// Include the AVR LibC functions for power reduction.
// https://create.arduino.cc/projecthub/feilipu/battery-powered-arduino-applications-through-freertos-3b7401
#include <avr/power.h>

// Declare a mutex Semaphore Handle which we will use to manage the Serial Port.
// It will be used to ensure only only one Task is accessing this resource at any time.
SemaphoreHandle_t xSensorSemaphore;

// initialize sht with default values
SHTI2cSensor sht;


void setup() {

  // Digital Input Disable on Analogue Pins
  // When this bit is written logic one, the digital input buffer on the corresponding ADC pin is disabled.
  // The corresponding PIN Register bit will always read as zero when this bit is set. When an
  // analogue signal is applied to the ADC7..0 pin and the digital input from this pin is not needed, this
  // bit should be written logic one to reduce power consumption in the digital input buffer.
  
#if defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) // Mega with 2560
  DIDR0 = 0xFF;
  DIDR2 = 0xFF;
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284PA__) // Goldilocks with 1284p
  DIDR0 = 0xFF;
#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega8__) // assume we're using an Arduino with 328p
  DIDR0 = 0x3F;
#elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__) // assume we're using an Arduino Leonardo with 32u4
  DIDR0 = 0xF3;
  DIDR2 = 0x3F;
#endif
  

  // Analogue Comparator Disable
  // When the ACD bit is written logic one, the power to the Analogue Comparator is switched off.
  // This bit can be set at any time to turn off the Analogue Comparator.
  // This will reduce power consumption in Active and Idle mode.
  // When changing the ACD bit, the Analogue Comparator Interrupt must be disabled by clearing the ACIE bit in ACSR.
  // Otherwise an interrupt can occur when the ACD bit is changed.
  ACSR &= ~_BV(ACIE);
  ACSR |= _BV(ACD);
  
  
  // CHOOSE ANY OF THESE <avr/power.h> MACROS THAT YOU NEED.
  // Any *_disable() macro can be reversed by the corresponding *_enable() macro.

  // Disable the Analog to Digital Converter module.
  power_adc_disable();
  
  // Disable the Serial Peripheral Interface module.
  power_spi_disable();
  
  // Disable the Two Wire Interface or I2C module.
  //power_twi_disable();
  
  // Disable the Timer 0 module. millis() will stop working.
  //power_timer0_disable();
  
  // Disable the Timer 1 module.
  //power_timer1_disable();

  // Disable the Timer 2 module. Used for RTC in Goldilocks 1284p devices.
  power_timer2_disable();


  // Now continue to initialise Tasks, and configure the Interfaces (that are not disabled).

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
  // Remember that loop() is simply the FreeRTOS idle task. Something to do, when there's nothing else to do.
  
  // There are several macros provided in the  header file to actually put
  // the device into sleep mode.
  // See ATmega328p Datasheet for more detailed descriptions.
  
  // SLEEP_MODE_IDLE
  // SLEEP_MODE_ADC
  // SLEEP_MODE_PWR_DOWN
  // SLEEP_MODE_PWR_SAVE
  // SLEEP_MODE_STANDBY
  // SLEEP_MODE_EXT_STANDBY
  
  set_sleep_mode( SLEEP_MODE_PWR_DOWN );
 
  portENTER_CRITICAL();

  sleep_enable();
 
  // Only if there is support to disable the brown-out detection.
  // If the brown-out is not set, it doesn't cost much to check.
#if defined(BODS) && defined(BODSE)
  sleep_bod_disable();
#endif
  
  portEXIT_CRITICAL();

  sleep_cpu(); // Good night.
 
  // Ugh. Yawn... I've been woken up. Better disable sleep mode.
  // Reset the sleep_mode() faster than sleep_disable();
  sleep_reset();
  
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

