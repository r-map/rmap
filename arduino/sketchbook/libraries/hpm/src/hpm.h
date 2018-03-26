#ifndef _HPM_H
#define _HPM_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoLog.h>

#define HPM_RESP_TIME 1500

enum hpm_sensor_type {
	PM25_TYPE,
	PM10_TYPE
};

class hpm {

public:

	hpm();

	bool init(Stream *serial);
	uint16_t get(hpm_sensor_type type=PM25_TYPE);
	uint16_t readParticleMeasuringResults(hpm_sensor_type type=PM25_TYPE);
	bool startParticleMeasurement();
	bool stopParticleMeasurement();
	bool stopAutoSend();
	bool enableAutoSend();
	bool setCustomerAdjustmentCoefficient(uint8_t value);
	uint8_t readCustomerAdjustmentCoefficient();
	bool loop();

private:

	Stream* _serial;
	Stream* _debugSerial;
	
	uint16_t pm25_val;
	uint16_t pm10_val;
	uint8_t coefficient;
	
	bool sendCmd(uint8_t len, uint8_t cmd, uint8_t value=0);
	bool readResponse();
	uint8_t getCheckSum8(uint8_t *buf, uint8_t len);
	uint16_t getCheckSum(uint8_t *buf, uint8_t len);
	void flush();

};

#endif
