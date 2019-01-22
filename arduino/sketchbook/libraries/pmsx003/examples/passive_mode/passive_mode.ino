#include <Arduino.h>
#include <pms.h>

Pmsx003 pms(D3, D4);


void setup(void) {
	Serial.begin(115200);
	Serial.println("Start");

	pms.begin();

	pms.waitForData(Pmsx003::wakeupTime);
	pms.write(Pmsx003::cmdModePassive);
}

////////////////////////////////////////

void loop(void) {

	const Pmsx003::pmsIdx n = Pmsx003::nValues_PmsDataNames;
	Pmsx003::pmsData data[n];

	pms.write(Pmsx003::cmdReadData);
	delay(5000);
	Pmsx003::PmsStatus status = pms.read(data, n);

	switch (status) {
		case Pmsx003::OK:
		{
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
