#include <frt.h>

namespace
{

	// This is the data shared between producer and consumer task
	struct Data {
		int value;
		unsigned long timestamp;
	};

	// This is our queue, which can store 10 elements of Data
	frt::Queue<Data, 10> queue;

	// This is the mutex to protect our output from getting mixed up
	frt::Mutex serial_mutex;

	// The producer task
	class ProducerTask final :
		public frt::Task<ProducerTask, 100>
	{
	public:
		bool run()
		{
			// We push the analog value and the current timestamp
			queue.push({analogRead(0), millis()});

			// Note that there is no msleep() here, so this task runs
			// "full throttle" and will only yield if either preempted
			// by a higher priority task on the next scheduler tick or
			// the queue being full.

			return true;
		}
	};

	// The consumer task
	class ConsumerTask final :
		public frt::Task<ConsumerTask, 150>
	{
	public:
		bool run()
		{
			Data data;

			// Get the (possibly old) data from the queue
			queue.pop(data);

			// This variant of pop() will wait forever for data to
			// arrive. If you ever plan to stop this task, use a
			// variant with timeout, so run() is left once in a while.

			const unsigned long now = millis();

			// If you dare, try to remove the mutex and see what happens
			serial_mutex.lock();
			Serial.print(F("Got value "));
			Serial.print(data.value);
			Serial.print(F(" at "));
			Serial.print(now);
			Serial.print(F(" which was queued at "));
			Serial.println(data.timestamp);
			serial_mutex.unlock();

			// Again, no msleep() here

			return true;
		}

	};

	// Instances of the above tasks
	ProducerTask producer_task;
	ConsumerTask consumer_task;

	// The high priority monitoring task
	class MonitoringTask final :
		public frt::Task<MonitoringTask>
	{
	public:
		bool run()
		{
			msleep(1000, remainder);

			serial_mutex.lock();
			Serial.print(F("Queue fill level: "));
			Serial.println(queue.getFillLevel());
			Serial.print(F("Producer stack used: "));
			Serial.println(producer_task.getUsedStackSize());
			Serial.print(F("Consumer stack used: "));
			Serial.println(consumer_task.getUsedStackSize());
			serial_mutex.unlock();

			return true;
		}

	private:
		unsigned int remainder = 0;
	};

	// Instance of the above task
	MonitoringTask monitoring_task;

}

void setup()
{
	Serial.begin(9600);

	while (!Serial);

	monitoring_task.start(3);

	// As the consumer task is started with lower priority first, the
	// moment the producer starts, it will fill the queue completely
	// and stall. The consumer will wake and pop one element, then the
	// producer will kick in again filling the queue (only one item).
	// Only then the consumer will print out its line. It continues into
	// run() and pops one item, and so on.
	// You should play a bit with the start sequence and priorities here
	// to get a feeling on what's going on.
	consumer_task.start(1);
	producer_task.start(2);
}

void loop()
{
	// As both producer and consumer are always runnable, you will
	// never see the next line.
	Serial.println(F("IDLE"));
}
