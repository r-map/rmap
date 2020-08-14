/*
Copyright (C) 2020  Paolo Paruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Freeg Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * Pin mapping table
 *
 * Platform     IR input    IR output
 * ----------------------------------
 * BluePill     PA6         PA7      
 */


#define IRMP_PROTOCOL_NAMES 1 // Enable protocol number mapping to protocol strings - requires some FLASH.
#define IRMP_SUPPORT_NEC_PROTOCOL     1
#define IRSND_SUPPORT_NEC_PROTOCOL    1
#define IRMP_USE_COMPLETE_CALLBACK       1 // Enable callback functionality
#define IRMP_ENABLE_PIN_CHANGE_INTERRUPT 1 // Enable interrupt functionality
#define USE_ONE_TIMER_FOR_IRMP_AND_IRSND // otherwise we get an error: redefinition of 'void __vector_8()
#include <irmp.c.h>
#include <irsnd.c.h>

#ifdef ARDUINO_ARCH_AVR
#include <ArduinoSTL.h>
#include <Arduino_FreeRTOS.h>
#else 
#ifdef ARDUINO_ARCH_STM32
#include "STM32FreeRTOS.h"
#else
#include "FreeRTOS.h"
#endif
#endif
#include "task.h"
#include "thread.hpp"
#include "ticks.hpp"
#include "queue.hpp"
#include <frtosLog.h>


void printTimestamp(Print* _logOutput) {
  char c[12];
  sprintf(c, "%10lu ", millis());
  _logOutput->print(c);
}

void printNewline(Print* _logOutput) {
  _logOutput->print('\n');
}

using namespace cpp_freertos;

Queue *MessageQueue;

void handleReceivedIRData(){
  IRMP_DATA irmp_data;
  irmp_get_data(&irmp_data);
  // enable interrupts
  interrupts();
  //irmp_result_print(&irmp_data);
  if (! (irmp_data.flags & IRMP_FLAG_REPETITION)){
      // Its a new key
    
    MessageQueue->EnqueueFromISR (&irmp_data,NULL);
  
  }
}

class irThread : public Thread {
  
public:
  
  irThread(int i, Queue &q)
    : Thread("Thread One", 200,1), 
      Id (i),
      MessageQueue(q)
  {
            Start();
  };
  
protected:

  virtual void Run() {
    
    frtosLog.notice("Starting Thread %d", Id);

    irmp_init();
    irsnd_init();
    IRMP_DATA irmp_data;
    IRMP_DATA irsnd_data;
 
    frtosLog.notice("message from thread %d", Id);
    frtosLog.notice(F("Ready to receive IR signals at pin %d"), IRMP_INPUT_PIN);
    frtosLog.notice(F("Ready to send IR signals at pin %d"), IRSND_OUTPUT_PIN);

    irmp_register_complete_callback_function(&handleReceivedIRData);
 
    while (true){
      MessageQueue.Dequeue(&irmp_data);
      //if (irmp_get_data(&irmp_data)) {
	/*
	  irmp_data.protocol (8 Bit)
	  irmp_data.address (16 Bit)
	  irmp_data.command (16 Bit)
	  irmp_data.flags (8 Bit)
	*/

      frtosLog.notice("Received %d %d %d %d",irmp_data.protocol,irmp_data.address,irmp_data.command,irmp_data.flags);
 
      irsnd_data.protocol = IRMP_NEC_PROTOCOL;
      irsnd_data.address = irmp_data.address;
      irsnd_data.command = irmp_data.command;
      irsnd_data.flags = 1; // repeat frame 1 time
      frtosLog.notice("Send %d %d %d %d",irsnd_data.protocol,irsnd_data.address,irsnd_data.command,irsnd_data.flags);
      irsnd_send_data(&irsnd_data, true); // true = wait for frame to end. This stores timer state and restores it after sending
      
    }
  };

private:
  int Id;
  Queue &MessageQueue;
};


void setup (void)
{

  MutexStandard loggingmutex;
  
  // start up the serial interface
  Serial.begin(115200);
  frtosLog.begin(LOG_LEVEL_VERBOSE, &Serial,loggingmutex);
  frtosLog.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
  frtosLog.setSuffix(printNewline); // Uncomment to get newline as suffix

  //Start logging

  frtosLog.notice(F("Testing FreeRTOS C++ wrappers with logger"));                     // Info string with Newline

  MessageQueue = new Queue(10, sizeof(IRMP_DATA));
  
  static irThread p1(1,*MessageQueue);
  
  Thread::StartScheduler();
  
}

void loop()
{
  // Empty. Things are done in Tasks.
}

