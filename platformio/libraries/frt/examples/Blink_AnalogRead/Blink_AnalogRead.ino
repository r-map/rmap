#include <frt.h>

namespace
{
	// An anonymous namespace tells the linker that this code is only
	// referenced in this very compilation unit. That's good for larger
	// projects but not necessary for this sketch.

	// Define the blink task and implement run().
	// - final is optional but good practice
	// - run() must be public
	// - sleeping with remainder is optional
	// - inlining run() is optional
	class BlinkTask final :
		public frt::Task<BlinkTask>
	{
	public:
		bool run()
		{
			digitalWrite(LED_BUILTIN, true);
			msleep(400, remainder);
			digitalWrite(LED_BUILTIN, false);
			msleep(400, remainder);

			return true;
		}

	private:
		unsigned int remainder = 0;
	};

	// Define the analog read task and implement run().
	// - 84 is the stack size in bytes
	class AnalogReadTask final :
		public frt::Task<AnalogReadTask, 84>
	{
	public:
		bool run()
		{
			Serial.println(analogRead(A0));
			msleep(100, remainder);

			return true;
		}

	private:
		unsigned int remainder = 0;
	};

	// Create the static BlinkTask instance
	BlinkTask blink_task;
	// Create the static AnalogReadTask instance
	AnalogReadTask analog_read_task;

}

void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);

	Serial.begin(9600);

	while (!Serial);

	// Start the blink task with priority 1
	blink_task.start(1);
	// Start the analog read task with higher priority 2
	analog_read_task.start(2);
}

// This is called by the idle task (at the lowest priority 0)
void loop()
{
	static bool stopped = false;

	// Stop analog read task after 5s
	if (!stopped && millis() > 5000) {
		// Get the stack size used this time. This varies from run to
		// run due to interrupts being served while the analog read task
		// was running. Don't be too conservative!
		Serial.print(F("AnalogReadTask used "));
		Serial.print(analog_read_task.getUsedStackSize());
		Serial.println(F(" bytes of stack this time."));

		Serial.println(F("Stopping"));
		analog_read_task.stopFromIdleTask();
		Serial.println(F("Stopped"));
		stopped = true;
	}
}
