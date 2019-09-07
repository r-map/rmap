#include <Arduino_FreeRTOS.h>
#include <semphr.h>  // add the FreeRTOS functions for Semaphores (or Flags).
#include <Wire.h>
#include "SHTSensor.h"
#include "sps30.h"
// Include the AVR LibC functions for power reduction.
// https://create.arduino.cc/projecthub/feilipu/battery-powered-arduino-applications-through-freertos-3b7401
#include <avr/power.h>
#include <avr/sleep.h>

// Declare a mutex Semaphore Handle which we will use to manage the Serial Port.
// It will be used to ensure only only one Task is accessing this resource at any time.
SemaphoreHandle_t xSensorSemaphore;
SemaphoreHandle_t xSerialSemaphore;

// initialize sht with default values
SHTI2cSensor sht;

#define SP30_COMMS I2C_COMMS
SPS30 sps30;

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

  if ( xSerialSemaphore == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
  {
    xSerialSemaphore = xSemaphoreCreateMutex();  // Create a mutex semaphore we will use to manage the Serial Port
    if ( ( xSerialSemaphore ) != NULL )
      xSemaphoreGive( ( xSerialSemaphore ) );  // Make the Serial Port available for use, by "Giving" the Semaphore.
  }

  
  Wire.begin();

  //SHT
  //reset and clear status
  sht.softReset();
  delay(10);
  sht.clearStatusRegister();

  //SPS
  // Begin communication channel;
  if (sps30.begin(SP30_COMMS) == false) {
    Serial.println(F("could not initialize communication channel."));
  }

  // check for SPS30 connection
  if (sps30.probe() == false) {
    Serial.println(F("could not probe / connect with SPS30."));
  }
  else
    Serial.println(F("Detected SPS30."));

  // reset SPS30 connection
  if (sps30.reset() == false) {
    Serial.println(F("could not reset."));
  }

  if (sps30.start() == true)
    Serial.println(F("Measurement started\n"));
  else
    Serial.println(F("Could NOT start measurement"));
  

  xTaskCreate(
	      taskOne,          // Task function.
	      "Task SHT",        // String with name of task.
	      300,              // Stack size in bytes.
	      NULL,             // Parameter passed as input of the task
	      1,                // Priority of the task.
	      NULL);            // Task handle.
  
  xTaskCreate(
	      taskTwo,          // Task function.
	      "Task SPS",        // String with name of task.
	      300,              // Stack size in bytes.
	      NULL,             // Parameter passed as input of the task
	      1,                // Priority of the task.
	      NULL);            // Task handle.
  
}
/*
// no sleep loop
void loop() {
  // Empty. Things are done in Tasks.
  // Remember that loop() is simply the FreeRTOS idle task. Something to do, when there's nothing else to do.
}
*/

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
  
  set_sleep_mode( SLEEP_MODE_IDLE );
 
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

  Serial.println("task one started");
  for (;;) {

    // See if we can obtain or "Take" the Sensor Semaphore.
    // If the semaphore is not available, wait 5 ticks of the Scheduler to see if it becomes free.
    if ( xSemaphoreTake( xSensorSemaphore, ( TickType_t ) 5 ) == pdTRUE )
      {
	// We were able to obtain or "Take" the semaphore and can now access the shared resource.
	
	// do and read one measure in blocking mode
	bool status=sht.readSample();
	
	xSemaphoreGive( xSensorSemaphore ); // Now free or "Give" the Sensor Port for others.
	
	
	// See if we can obtain or "Take" the Serial Semaphore.
	// If the semaphore is not available, wait 5 ticks of the Scheduler to see if it becomes free.
	if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
	  {
	      if (status) {
	      Serial.println("SHT single shot:");
	      Serial.print("  RH: ");
	      Serial.println(sht.getHumidity(), 2);
	      Serial.print("  T:  ");
	      Serial.println(sht.getTemperature(), 2);
	      } else {
	      Serial.print("Error in readSample()\n");
	      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.	    
	      break;
	      }
	    xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.	    
	  }
	vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
      }
  }
  
  Serial.println("Ending task 1");
  vTaskDelete( NULL );
}
 
void taskTwo( void * parameter)
{
 
  for (;;) {

    // See if we can obtain or "Take" the Sensor Semaphore.
    // If the semaphore is not available, wait 5 ticks of the Scheduler to see if it becomes free.
    if ( xSemaphoreTake( xSensorSemaphore, ( TickType_t ) 5 ) == pdTRUE )
      {
	// We were able to obtain or "Take" the semaphore and can now access the shared resource.
	// do and read one measure in blocking mode

	// loop to get data
	static bool header = true;
	struct sps_values val;
	uint8_t ret = sps30.GetValues(&val);
	xSemaphoreGive( xSensorSemaphore ); // Now free or "Give" the Sensor Port for others.

	// See if we can obtain or "Take" the Serial Semaphore.
	// If the semaphore is not available, wait 5 ticks of the Scheduler to see if it becomes free.
	if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
	  {

	    // data might not have been ready
	    if (ret != ERR_OK){
	      Serial.println(F("Error during reading values"));
	      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.	    
	      break;
	    }
	    
	    // only print header first time
	    if (header) {
	      Serial.println(F("-------------Mass -----------    ------------- Number --------------   -Average-"));
	      Serial.println(F("     Concentration [μg/m3]             Concentration [#/cm3]             [μm]"));
	      Serial.println(F("P1.0\tP2.5\tP4.0\tP10\tP0.5\tP1.0\tP2.5\tP4.0\tP10\tPartSize\n"));
	      header = false;
	    }
	    
	    Serial.print(val.MassPM1);
	    Serial.print(F("\t"));
	    Serial.print(val.MassPM2);
	    Serial.print(F("\t"));
	    Serial.print(val.MassPM4);
	    Serial.print(F("\t"));
	    Serial.print(val.MassPM10);
	    Serial.print(F("\t"));
	    Serial.print(val.NumPM0);
	    Serial.print(F("\t"));
	    Serial.print(val.NumPM1);
	    Serial.print(F("\t"));
	    Serial.print(val.NumPM2);
	    Serial.print(F("\t"));
	    Serial.print(val.NumPM4);
	    Serial.print(F("\t"));
	    Serial.print(val.NumPM10);
	    Serial.print(F("\t"));
	    Serial.println(val.PartSize);

	    xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
	    
	  }
	vTaskDelay( 2000 / portTICK_PERIOD_MS ); // wait for two second
      }
  }
  
  Serial.println("Ending task 2");
  vTaskDelete( NULL );
}

