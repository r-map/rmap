/**
 * Example application demonstrating task procedure sharing, message passing and more.
 *
 * The application consists of four tasks:
 *
 *  - temp sensor task: waits for an event to signaled from the temp sensor driver. New temp
 */
#include <Arduino.h> ////
#include <cocoos.h>

#include "sensor.h"

static Evt_t sensorEvt;

static uint8_t sensorTaskId;
static uint8_t sensorTaskData;

/********************************************/
/*            System threads                */
/********************************************/

static void arduino_start_timer(void) { ////
  //  Start the AVR Timer 1 to generate interrupt ticks for cocoOS to perform
  //  background processing.  AVR Timer 0 is reserved for Arduino timekeeping.
  
  // Set PORTB pins as output, but off
	DDRB = 0xFF;
	PORTB = 0x00;

	// Turn on timer 
	// TCCR1B |= _BV(CS10);  // no prescaler
	TCCR1B = (1<<CS10) | (1<<CS12); //set the prescaler as 1024
	TIMSK1 |= _BV(TOIE1);

	// Turn interrupts on.
	sei();
	
} ////

ISR(TIMER1_OVF_vect) { ////
  //  Handle the AVR Timer 1 interrupt ticks for cocoOS to perform
  //  background processing.  Copied from ticker().
  static int ticks = 0;

  // debug("os_tick"); ////
  os_tick();
  
  if (++ticks == 100) {
      // service the sensor drivers each 100 ms
      // This is not a polling of the sensors, it is just simulating
      // something is stimulating the sensor.
      //tempSensor_service();
      //gyroSensor_service();      
      ticks = 0;
  }

  //read keyboard
  //if (input_available()) event_ISR_signal(serialinputEvt);

} ////


/********************************************/
/*            Application tasks             */
/********************************************/

static void sensorTask() {

  task_open();

  sensor_setup();
  for (;;) {
    task_wait(200);
    sensor_prepare();
    sensor_get();
  }
  task_close();
}

/********************************************/
/*            Setup and main                */
/********************************************/

static void arduino_setup(void) { ////
  //  Run initialisation for Arduino, since we are using main() instead of setup()+loop().
  init();  // initialize Arduino timers  
  //mysetup();
} ////

static void system_setup(void) {
  arduino_setup(); ////
}

int main(void) {
  system_setup();
  os_init();

  // create events
  //debug("event_create"); ////
  sensorEvt   = event_create();

  // sensor tasks using same task procedure, but having unique task data.
  task_create( sensorTask, &sensorTaskData, 10, 0, 0, 0 );
  
  //// Start the AVR timer to generate ticks for background processing.
  //debug("arduino_start_timer"); ////
  arduino_start_timer(); ////

  //debug("os_start"); ////
  os_start();  //  Never returns.
  
	return EXIT_SUCCESS;
}

