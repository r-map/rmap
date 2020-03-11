/****************************************************************************
 *
 *  Copyright (c) 2017, Michael Becker (michael.f.becker@gmail.com)
 *
 *  This file is part of the FreeRTOS Add-ons project.
 *
 *  Source Code:
 *  https://github.com/michaelbecker/freertos-addons
 *
 *  Project Page:
 *  http://michaelbecker.github.io/freertos-addons/
 *
 *  On-line Documentation:
 *  http://michaelbecker.github.io/freertos-addons/docs/html/index.html
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files
 *  (the "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so,subject to the
 *  following conditions:
 *
 *  + The above copyright notice and this permission notice shall be included
 *    in all copies or substantial portions of the Software.
 *  + Credit is appreciated, but not required, if you find this project
 *    useful enough to include in your application, product, device, etc.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ***************************************************************************/

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

//#include "freertos-addons.h"

using namespace cpp_freertos;

typedef struct message_t
{
  int Id;
  int Value;
};


class ProducerThread : public Thread {
  
public:
  
  ProducerThread(int i, int delayInSeconds, int burstAmount, Queue &q, Mutex &lock)
    : Thread("ProducerThread", 200, 1), 
      Id (i), 
      DelayInSeconds(delayInSeconds),
      BurstAmount(burstAmount),
      MessageQueue(q),
      Lock(lock)
  {
            Start();
  };
  
protected:

  virtual void Run() {
    
    Serial.print("Starting ProducerThread ");
    Serial.println(Id);
    message_t Message;
    Message.Id=Id;
    Message.Value= 0;
    
    while (true) {
      
      Lock.Lock();
      Serial.print("producer DelayInSeconds: ");
      Serial.println(DelayInSeconds);
      Lock.Unlock();
      
      Delay(Ticks::SecondsToTicks(DelayInSeconds));

      for (int i = 0; i < BurstAmount; i++) {

	Lock.Lock();
	Serial.print("[P ");
	Serial.print(Id);
	Serial.print("] Sending  Message: ");
	Serial.println(Message.Value);
	Lock.Unlock();

	MessageQueue.Enqueue(&Message);
	Message.Value++;
      }
    }
  };

private:
  int Id;
  int DelayInSeconds;
  int BurstAmount;
  Queue &MessageQueue;
  Mutex& Lock;
};


class ConsumerThread : public Thread {
  
public:

  ConsumerThread(int i, int delayInSeconds, Queue &q, Mutex &lock)
    : Thread("ConsumerThread", 200, 2), 
      Id (i), 
      DelayInSeconds(delayInSeconds),
      MessageQueue(q),
      Lock(lock)
  {
    Start();
  };
  
protected:
  
  virtual void Run() {
    
    Serial.print("Starting ConsumerThread ");
    Serial.println(Id);
    message_t Message;
    
    while (true) {
      
      {
	LockGuard guard(Lock);
	Serial.print("consumer DelayInSeconds: ");
	Serial.println(DelayInSeconds);
	//guard.~LockGuard();   // automatic unlock, not needed
      }

      Delay(Ticks::SecondsToTicks(DelayInSeconds));
      MessageQueue.Dequeue(&Message);

      {
	LockGuard guard(Lock);
	Serial.print("[C ");
	Serial.print(Id);
	Serial.print("] Received Message: Id ");
	Serial.print( Message.Id);
	Serial.print(" value: ");
	Serial.println( Message.Value);
	//guard.~LockGuard();   // automatic unlock, not needed
      }
    }
  };
  
private:
  int Id;
  int DelayInSeconds;
  Queue &MessageQueue;
  Mutex& Lock;
};


void setup (void)
{
  
  // start up the serial interface
  Serial.begin(115200);
  Serial.println("started");

  Serial.println("Testing FreeRTOS C++ wrappers");
  Serial.println("Queues Simple Producer / Consumer");

  delay(1000);
  
  Queue *MessageQueue;
  //
  //  These parameters may be adjusted to explore queue 
  //  behaviors.
  //
  MessageQueue = new Queue(3, sizeof(message_t));

  MutexStandard *SharedLock;
  SharedLock = new MutexStandard();
   
  static ProducerThread p1(1,  5, 7, *MessageQueue,*SharedLock);
  static ProducerThread p2(2, 10, 3, *MessageQueue,*SharedLock);
  static ConsumerThread p3(50, 1,    *MessageQueue,*SharedLock);
  
  Thread::StartScheduler();
  
  //
  //  We shouldn't ever get here unless someone calls 
  //  Thread::EndScheduler()
  //
  
  Serial.println("Scheduler ended!");

}

void loop()
{
  // Empty. Things are done in Tasks.
}

