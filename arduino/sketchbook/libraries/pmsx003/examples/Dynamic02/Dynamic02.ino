#include <Arduino.h>
#include <pms.h>

////////////////////////////////////////

//
// Please uncomment #define PMS_DYNAMIC in pmsConfig.h file
//

#if defined PMS_DYNAMIC
Pmsx003 *pms_ = nullptr;
#define pms (*pms_)
#else
Pmsx003 pms(D3, D4);
#endif

////////////////////////////////////////

void setup(void) {
	Serial.begin(115200);
	while (!Serial) {};
	Serial.println("Pmsx003");

#if defined PMS_DYNAMIC
	pms_ = new Pmsx003(D3, D4);
#else
	pms.begin();
#endif 

	pms.waitForData(Pmsx003::wakeupTime);
	pms.write(Pmsx003::cmdModeActive);
}

////////////////////////////////////////

auto lastRead = millis();

void loop(void) {

	const Pmsx003::pmsIdx n = Pmsx003::nValues_PmsDataNames;
	Pmsx003::pmsData data[n];

	auto t0Read = millis();
	Pmsx003::PmsStatus status = pms.read(data, n);
	auto t1Read = millis();

	switch (status) {
		case Pmsx003::OK:
		{
			Serial.print("_________________ time of read(): ");
			Serial.print(t1Read - t0Read);
			Serial.println(" msec");
			auto newRead = millis();
			Serial.print("Wait time ");
			Serial.println(newRead - lastRead);
			lastRead = newRead;

			for (Pmsx003::pmsIdx i = 0; i < n; ++i) {
				Serial.print(data[i]);
				Serial.print("\t");
				Serial.print(Pmsx003::getDataNames(i));
				Serial.print(" [");
				Serial.print(Pmsx003::getMetrics(i));
				Serial.print("]");
				Serial.println();
			}
			break;
		}
		case Pmsx003::noData:
			break;
		default:
			Serial.println("_________________");
			Serial.println(Pmsx003::errorMsg[status]);
	};
}
