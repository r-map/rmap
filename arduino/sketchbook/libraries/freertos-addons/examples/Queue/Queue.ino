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

#include <STM32FreeRTOS.h>
#include "task.h"
#include "thread.hpp"
#include "ticks.hpp"
#include "queue.hpp"

//#include "freertos-addons.h"

using namespace cpp_freertos;

class ProducerThread : public Thread {
  
public:
  
  ProducerThread(int i, int delayInSeconds, int burstAmount, Queue &q, Mutex &lock)
    : Thread("ProducerThread", 100, 1), 
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
    int Message = 0;
    
    while (true) {
      
      Serial.print("DelayInSeconds: ");
      Serial.println(DelayInSeconds);
      Delay(Ticks::SecondsToTicks(DelayInSeconds));
      for (int i = 0; i < BurstAmount; i++) {
	LockGuard guard(Lock);
	Serial.print("[P ");
	Serial.print(Id);
	Serial.print("] Sending Message: ");
	Serial.println(Message);
	MessageQueue.Enqueue(&Message);
	Message++;
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
    : Thread("ConsumerThread", 100, 2), 
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
    int Message;
    
    while (true) {
      
      MessageQueue.Dequeue(&Message);
      LockGuard guard(Lock);
      Serial.print("[C");
      Serial.print(Id);
      Serial.print("] Received Message: ");
      Serial.println( Message);
      Delay(Ticks::SecondsToTicks(DelayInSeconds));
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
  MessageQueue = new Queue(1, sizeof(int));

  MutexStandard *SharedLock;
  SharedLock = new MutexStandard();
   
  ProducerThread p1(10, 1, 10, *MessageQueue,*SharedLock);
  ConsumerThread c1(20, 1, *MessageQueue,*SharedLock);
  
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

