#include <frt.h>

namespace
{

	// This is our queue holding at most five unsigned 16 bit integers
	frt::Queue<uint16_t, 5> queue;

	// This is the mutex to protect our output from getting mixed up
	frt::Mutex serial_mutex;

	// The print task
	class PrintTask final :
		public frt::Task<PrintTask>
	{
	public:
		PrintTask() :
			conversion_count(0)
		{
		}

		bool run()
		{
			startConversion();

			uint16_t adc_value;

			// This task could be stopped, as we wait with timeout here
			if (queue.pop(adc_value, 15)) {
				// conversion_count is concurrently accessed
				mutex.lock();
				++conversion_count;
				mutex.unlock();

				// Serial needs to be locked against MonitoringTask
				serial_mutex.lock();
				Serial.println(adc_value);
				serial_mutex.unlock();

				// If you comment out the Serial.println() above
				// you get way more conversions per second.
			}

			return true;
		}

		uint32_t getAndResetConversionCount()
		{
			// Called from MonitoringTask, thus the mutex
			mutex.lock();
			const uint32_t res = conversion_count;
			conversion_count = 0;
			mutex.unlock();

			return res;
		}

	private:
		void startConversion()
		{
			ADCSRA |= bit(ADSC) | bit(ADIE);
		}

		uint32_t conversion_count;
		frt::Mutex mutex;
	};

	// Our PrintTask instance
	PrintTask print_task;

	// The monitoring task
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
			Serial.print(F("Print stack used: "));
			Serial.println(print_task.getUsedStackSize());
			Serial.print(F("Conversions per second: "));
			Serial.println(print_task.getAndResetConversionCount());
			serial_mutex.unlock();

			return true;
		}

	private:
		unsigned int remainder = 0;
	};

	// The MonitoringTask instance
	MonitoringTask monitoring_task;

}

void setup()
{
	Serial.begin(9600);

	while (!Serial);

	// This is ATMega328 specific
	ADMUX = bit(REFS0); // AVcc as reference

	// Start monitoring task with low priority
	monitoring_task.start(1);

	// Start print task with high priority
	print_task.start(2);
}

void loop()
{
	// Nothing to do here
}

// This ISR is called when the ADC is finished
ISR(ADC_vect)
{
	queue.preparePushFromInterrupt();

	const uint16_t adc_value = ADCL | ADCH << 8;
	queue.pushFromInterrupt(adc_value);

	queue.finalizePushFromInterrupt();
}
