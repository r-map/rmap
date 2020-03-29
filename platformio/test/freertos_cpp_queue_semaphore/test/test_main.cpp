#include <unity.h>
#include <Arduino.h> 
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
#include <critical.hpp>

using namespace cpp_freertos;

int nmessages=0;

struct message_t
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
    
    message_t Message;
    Message.Id=Id;
    Message.Value= Id;
    
    while (true) {
      
      Lock.Lock();
      TEST_ASSERT_EQUAL(0,0);
      Lock.Unlock();
      
      Delay(Ticks::SecondsToTicks(DelayInSeconds));

      for (int i = 0; i < BurstAmount; i++) {

	Lock.Lock();
	TEST_ASSERT_EQUAL(0,0);
	Lock.Unlock();

	MessageQueue.Enqueue(&Message);
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
    
    message_t Message;
    
    while (true) {
      
      {
	LockGuard guard(Lock);
	TEST_ASSERT_EQUAL(0,0);
	//guard.~LockGuard();   // automatic unlock, not needed
      }

      Delay(Ticks::SecondsToTicks(DelayInSeconds));
      MessageQueue.Dequeue(&Message);

      {
	LockGuard guard(Lock);

	TEST_ASSERT_EQUAL(Id, 50);

	TEST_ASSERT_EQUAL( Message.Id,  Message.Value);
	//guard.~LockGuard();   // automatic unlock, not needed

	//Serial.println("message");
	nmessages++;
	if (nmessages > 3) {
	  UNITY_END(); // stop unit testing
	  //CriticalSection::SuspendScheduler();
	  //Thread::EndScheduler();
	  //CriticalSection::ResumeScheduler();
	}
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
  Queue *MessageQueue;
  MessageQueue = new Queue(3, sizeof(message_t));

  MutexStandard *SharedLock;
  SharedLock = new MutexStandard();
   
  static ProducerThread p1(20,  1, 7, *MessageQueue,*SharedLock);
  static ProducerThread p2(30,  2, 3, *MessageQueue,*SharedLock);
  static ConsumerThread p3(50,  1,    *MessageQueue,*SharedLock);

  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);

 
  UNITY_BEGIN();    // IMPORTANT LINE!
  //Serial.println("started");
  
  RUN_TEST(Thread::StartScheduler);

  // do not go here
  //Serial.println("ended");  
  //UNITY_END(); // stop unit testing

}

void loop(void) {};
