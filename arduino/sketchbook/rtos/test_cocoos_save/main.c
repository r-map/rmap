
/**
 * Example application demonstrating task procedure sharing, message passing and more.
 *
 * The application consists of four tasks:
 *
 *  - temp sensor task: waits for an event to signaled from the temp sensor driver. New temp
 *    data is fetched from the sensor and sent in a message to the display task.
 *
 *  - gyro sensor task: waits for a timeout and then polls the sensor for new data. New data is
 *    fetched from the sensor and sent in a message to the display task.
 *
 *  - control task: waits for up/down arrow events to be signaled and then changes channel of
 *    the temp sensor.
 *
 *  - display task: writes sensor values to the terminal
 *
 *  Two linux threads are created at startup:
 *   - One that simulates a timer tick and calls os_tick(). It also periodically services the
 *     sensors. This makes the sensors signal it has new data by signaling an event or setting
 *     a polled flag.
 *
 *   - One thread that reads characters from stdin and signals cocoOS events when up/down arrow are pressed.
 *
 *  Main flow of execution:
 *  When the sensors are serviced they eventually signals new data available by signaling an event (temp
 *  sensor) or sets a polled flag (gyro sensor). The sensor tasks waiting for event/timeout, checks the
 *  sensor for new data and posts a message with the new data to the display task.
 *
 *  Simultaneously, the thread reading characters from stdin, signals events when up/down
 *  arrow keys are pressed on the keyboard. The control task then changes channel on the temp sensor.
 *
 *  Task procedure sharing
 *  The two sensor tasks use the same task procedure. All work and data handling is done through the task
 *  data pointer assigned to each task. This points to a structure holding sensor configuration/functions
 *  and an array holding sensor data.
 *
 */
#include <Arduino.h> ////
#include <Time.h> ////
#include <TimeLib.h> ////
typedef unsigned long time_t; ////  TODO: Fix the declaration

#include <stdio.h>
#include <stdlib.h>
//// #include <pthread.h>
//// #include <sys/types.h>
//// #include <unistd.h>
#include <time.h>
#include <string.h>
#include <cocoos.h>

#include "sensor.h"
#include "temp_sensor.h"
#include "gyro_sensor.h"
#include "display.h"

static Evt_t tempEvt;
static Evt_t prevChEvt;
static Evt_t nextChEvt;

static uint8_t displayTaskId;

typedef struct {
  Sensor_t *sensor;
  uint8_t data[64];
} TaskData_t;

static TaskData_t tempTaskData;
static TaskData_t gyroTaskData;

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
      tempSensor_service();
      gyroSensor_service();
      ticks = 0;
  }
} ////

/* //// Arduino: Moved to timer handler above. We use a real timer tick instead of simulation.
// simulation of a timer tick
static void *ticker(void* arg) {
  // setup a 1 ms tick
  //// struct timespec req = {.tv_sec = 0,.tv_nsec = 1000000};

  static int cnt = 0;

  while(1) {
    debug("ticker"); ////

    delay(1); ////
    //// nanosleep(&req, NULL);

    os_tick();

    if (++cnt == 100) {
      // service the sensor drivers each 100 ms
      // This is not a polling of the sensors, it is just simulating
      // something is stimulating the sensor.
      tempSensor_service();
      gyroSensor_service();
      cnt = 0;
    }
  }
  return (void*)0;
}
*/ ////

// thread reading stdin and reacting on arrow up/down
static void *input(void* arg) {

  while (1) {
    char c1;
    while ( (c1 = getchar()) != ESC);

    char c2 = getchar();
    if ( c2 == '[') {
      char c3 = getchar();
      if (c3 == 'A') {
        // arrow up pressed
        event_ISR_signal(prevChEvt);
      }
      else if (c3 == 'B') {
        // arrow down pressed
        event_ISR_signal(nextChEvt);
      }
    }
  }
  return (void*)0;
}



/********************************************/
/*            Application tasks             */
/********************************************/

static void sensorTask() {
  TaskData_t *taskData = NULL; ////
  task_open();

  for (;;) {
     // We should not make this static, because it should be
     // unique for each task.
     //// TaskData_t *taskData = (TaskData_t*)task_get_data();
     taskData = (TaskData_t*)task_get_data(); ////

    // The task either waits for the event to be signaled
    // or a poll timeout.
    if (taskData->sensor->info.event == 0) {
      task_wait(taskData->sensor->info.period_ms);
    }
    else {
      event_wait(*(taskData->sensor->info.event));
    }

    // we have to fetch the data pointer again after the wait
    taskData = (TaskData_t*)task_get_data();

    // Do we have new data?
    if (taskData->sensor->info.poll()) {

      // Copy sensor data to task data
      uint8_t nread = taskData->sensor->info.data(taskData->data, 64);
      taskData->data[nread] = '\0';

      // And put it into a display message. Use the sensor id as message signal.
      // Note: When posting a message, its contents is copied into the message queue,
      // so it is safe to use a non-static message variable created on the stack here.
      // The display message does not contain the data buffer, only a pointer to it.
      // This is also ok, as it points to a static data buffer in the task data structure.
      DisplayMsg_t msg;
      msg.super.signal = taskData->sensor->info.id;
      msg.data = (const char*)taskData->data;
      
      ////  Note for Arduino: msg_post() must be called in a C source file, not C++.
      ////  The macro expansion fails in C++ with a cross-initialisation error.
      msg_post(displayTaskId, msg);
    }
  }

  task_close();
}

// Task that changes channel on its associated sensor
static void controlTask() {
  Evt_t event; ////
  task_open();

  for (;;) {
    event_wait_multiple(0, prevChEvt, nextChEvt);
    //// Evt_t event = event_last_signaled_get();
    event = event_last_signaled_get(); ////

    Sensor_t *sensor = ((TaskData_t*)task_get_data())->sensor;

    if(event == nextChEvt) {
      sensor->control.next_channel();
    }
    else if (event == prevChEvt) {
      sensor->control.prev_channel();
    }
  }

  task_close();
}


static void displayTask() {
  static DisplayMsg_t msg;
  task_open();
  msg.super.signal = DISPLAY_MSG;
  msg_post_every(displayTaskId, msg, 20);

  for (;;) {
    msg_receive( os_get_running_tid(), &msg );
    Display_t *display = (Display_t*)task_get_data();

    if (msg.super.signal == DISPLAY_MSG) {
      display->update();
    }
    else {
      display->updateData(msg.super.signal, msg.data);
    }

  }
  task_close();
}



/********************************************/
/*            Setup and main                */
/********************************************/

static void arduino_setup(void) { ////
  //  Run initialisation for Arduino, since we are using main() instead of setup()+loop().
  init();  // initialize Arduino timers  
  debug("------------------arduino_setup");
} ////

static void system_setup(void) {
  arduino_setup(); ////
  debug("display_init"); ////
  display_init();

  /* ////  Threads not applicable for Arduino.
  pthread_t tid;

  // create a time tick thread which drives cocoOS by calling the os_tick() method.
  int err = pthread_create(&tid, NULL, &ticker, NULL);

  if (err != 0) {
    printf("\ncan't create thread :[%s]", strerror(err));
    abort();
  }

  // create thread that reads stdin and delivers characters as events to
  // the control task
  err = pthread_create(&tid, NULL, &input, NULL);

  if (err != 0) {
    printf("\ncan't create thread :[%s]", strerror(err));
    abort();
  }
  */ ////

}

int main(void) {
  system_setup();
  debug("os_init"); ////
  os_init();

  // create events
  debug("event_create"); ////
  tempEvt   = event_create();
  prevChEvt = event_create();
  nextChEvt = event_create();

  // Initialize the sensors
  debug("tempSensor_get"); ////
  tempTaskData.sensor = tempSensor_get();
  debug("tempSensor.init"); ////
  tempTaskData.sensor->control.init(TEMP_DATA, &tempEvt, 0);

  debug("gyroSensor_get"); ////
  gyroTaskData.sensor = gyroSensor_get();
  debug("gyroSensor.init"); ////
  gyroTaskData.sensor->control.init(GYRO_DATA, 0, 500);

  // Two sensor tasks using same task procedure, but having unique task data.
  debug("task_create"); ////
  task_create( sensorTask, &tempTaskData, 10, 0, 0, 0 );
  task_create( sensorTask, &gyroTaskData, 20, 0, 0, 0 );

  // control task that changes temp sensor channel
  task_create( controlTask, &tempTaskData, 30, 0, 0, 0 );

  // display task that displays sensor readings
  displayTaskId = task_create( displayTask, display_get(), 50, (Msg_t*)displayMessages, 10, sizeof(DisplayMsg_t) );
  
  //// Start the AVR timer to generate ticks for background processing.
  debug("arduino_start_timer"); ////
  arduino_start_timer(); ////

  debug("os_start"); ////
  os_start();  //  Never returns.
  
	return EXIT_SUCCESS;
}

