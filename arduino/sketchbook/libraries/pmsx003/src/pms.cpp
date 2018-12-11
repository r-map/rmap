
#include <pms.h>

////////////////////////////////////////

#if defined NOMINMAX

#if defined min
#undef min
#endif
template <class T>
inline const T& __attribute__((always_inline)) min(const T& a, const T& b) {
	return !(b < a) ? a : b;
}

#endif

////////////////////////////////////////

inline void __attribute__((always_inline)) swapEndianBig16(uint16_t *x) {
	constexpr union {
		// endian.test16 == 0x0001 for low endian
		// endian.test16 == 0x0100 for big endian
		// should be properly optimized by compiler
		uint16_t test16;
		uint8_t test8[2];
	} endian = { .test8 = { 1,0 } };

	if (endian.test16 != 0x0100) {
		uint8_t hi = (*x & 0xff00) >> 8;
		uint8_t lo = (*x & 0xff);
		*x = lo << 8 | hi;
	}
}

////////////////////////////////////////

void sumBuffer(uint16_t *sum, const uint8_t *buffer, uint16_t cnt) {
	for (; cnt > 0; --cnt, ++buffer) {
		*sum += *buffer;
	}
}

inline void sumBuffer(uint16_t *sum, const uint16_t data) {
	*sum += (data & 0xFF) + (data >> 8);
}

////////////////////////////////////////

void Pmsx003::setTimeout(const decltype(timeout) timeout) {
	this->_pmsSerial->setTimeout(timeout);
	this->timeout = timeout;
};

decltype(Pmsx003::timeout) Pmsx003::getTimeout(void) const {
	return timeout;
};

Pmsx003::Pmsx003(int8_t swsRX, int8_t swsTX) : passive(tribool(unknown)), sleep(tribool(unknown)) {
#if defined PMS_DYNAMIC
	begin();
#endif
	this->_pmsSerial = new SoftwareSerial(swsRX, swsTX);
};

Pmsx003::~Pmsx003() {
#if defined PMS_DYNAMIC
	end();
#endif
}

bool Pmsx003::begin(void) {
	this->_pmsSerial->setTimeout(Pmsx003::timeoutPassive);
	this->_pmsSerial->begin(9600);
	return true;
};

void Pmsx003::end(void) {
	this->_pmsSerial->end();
};

size_t Pmsx003::available(void) {
	while (this->_pmsSerial->available()) {
		if (this->_pmsSerial->peek() != sig[0]) {
			this->_pmsSerial->read();
		} else {
			break;
		}
	}
	return static_cast<size_t>(this->_pmsSerial->available());
}

Pmsx003::PmsStatus Pmsx003::read(pmsData *data, const size_t nData, const uint8_t dataSize) {

	if (available() < (dataSize + 2) * sizeof(pmsData) + sizeof(sig)) {
		return noData;
	}

	this->_pmsSerial->read(); // Value is equal to sig[0]. There is no need to check the value, it was checked by prior peek()

	if (this->_pmsSerial->read() != sig[1]) // The rest of the buffer will be invalidated during the next read attempt
		return readError;

	uint16_t sum{ 0 };
	sumBuffer(&sum, (uint8_t *)&sig, sizeof(sig));

	pmsData thisFrameLen{ 0x1c };
	if (this->_pmsSerial->readBytes((uint8_t*)&thisFrameLen, sizeof(thisFrameLen)) != sizeof(thisFrameLen)) {
		return readError;
	};

	if (thisFrameLen % 2 != 0) {
		return frameLenMismatch;
	}
	sumBuffer(&sum, thisFrameLen);

	const decltype(thisFrameLen) maxFrameLen{ 2 * 0x1c };    // arbitrary

	swapEndianBig16(&thisFrameLen);
	if (thisFrameLen > maxFrameLen) {
		return frameLenMismatch;
	}

	size_t toRead{ min<unsigned int>(thisFrameLen - 2, nData * sizeof(pmsData)) };
	if (data == nullptr) {
		toRead = 0;
	}

	if (toRead) {
		if (this->_pmsSerial->readBytes((uint8_t*)data, toRead) != toRead) {
			return readError;
		}
		sumBuffer(&sum, (uint8_t*)data, toRead);

		for (size_t i = 0; i < nData; ++i) {
			swapEndianBig16(&data[i]);
		}
	}

	pmsData crc;
	for (; toRead < thisFrameLen; toRead += 2) {
		if (this->_pmsSerial->readBytes((uint8_t*)&crc, sizeof(crc)) != sizeof(crc)) {
			return readError;
		};

		if (toRead < thisFrameLen - 2)
			sumBuffer(&sum, crc);
	}

	swapEndianBig16(&crc);

	if (sum != crc) {
		return sumError;
	}

	return OK;
}

void Pmsx003::flushInput(void) {
	this->_pmsSerial->flush();
}

bool Pmsx003::waitForData(const unsigned int maxTime, const size_t nData) {
	const auto t0 = millis();
	if (nData == 0) {
		for (; (millis() - t0) < maxTime; delay(1)) {
			if (this->_pmsSerial->available()) {
				return true;
			}
		}
		return this->_pmsSerial->available();
	}

	for (; (millis() - t0) < maxTime; delay(1)) {
		if (available() >= nData) {
			return true;
		}
	}
	return available() >= nData;
}

bool Pmsx003::write(const PmsCmd cmd) {
	static_assert(sizeof(cmd) >= 3, "Wrong definition of PmsCmd (too short)");

	if ((cmd != cmdReadData) && (cmd != cmdWakeup)) {
		flushInput();
	}

	if (this->_pmsSerial->write(sig, sizeof(sig)) != sizeof(sig)) {
		return false;
	}
	const size_t cmdSize = 3;
	if (this->_pmsSerial->write((uint8_t*)&cmd, cmdSize) != cmdSize) {
		return false;
	}

	uint16_t sum{ 0 };
	sumBuffer(&sum, sig, sizeof(sig));
	sumBuffer(&sum, (uint8_t*)&cmd, cmdSize);
	swapEndianBig16(&sum);
	if (this->_pmsSerial->write((uint8_t*)&sum, sizeof(sum)) != sizeof(sum)) {
		return false;
	}

	switch (cmd) {
		case cmdModePassive:
			passive = tribool(true);
			break;
		case cmdModeActive:
			passive = tribool(false);
			break;
		case cmdSleep:
			sleep = tribool(true);
			break;
		case cmdWakeup:
			sleep = tribool(false);
			passive = tribool(false);
			// waitForData(wakeupTime);
			break;
		default:
			break;
	}
	if ((cmd != cmdReadData) && (cmd != cmdWakeup)) {
		const auto responseFrameSize = 8;
		if (!waitForData(ackTimeout, responseFrameSize)) {
			this->_pmsSerial->flush();
			return true;
		}
		Pmsx003::pmsData response = 0xCCCC;
		read(&response, 1, 1);
	}

	/*
		if ((cmd != cmdReadData) && (cmd != cmdWakeup)) {
			const auto responseFrameSize = 8;
			if (!waitForData(ackTimeout, responseFrameSize)) {
				this->_pmsSerial->flushInput();
				return false;
			}
			Pmsx003::pmsData response = 0xCCCC;
			if (read(&response, 1, 1) != OK) {
				return false;
			}
			if ((response >> 8) != (cmd & 0xFF)) {
				return false;
			}
		}
	*/

	return true;
}

const char *Pmsx003::getMetrics(const pmsIdx idx) {
	return idx < nValues_PmsDataNames ? Pmsx003::metrics[idx] : "???";
}

const char *Pmsx003::getDataNames(const pmsIdx idx) {
	return idx < nValues_PmsDataNames ? Pmsx003::dataNames[idx] : "???";
}

const char * Pmsx003::errorMsg[nValues_PmsStatus]{
	"OK",
	"noData",
	"readError",
	"frameLenMismatch",
	"sumError"
};

const char *Pmsx003::metrics[]{
	"mcg/m3",
	"mcg/m3",
	"mcg/m3",

	"mcg/m3",
	"mcg/m3",
	"mcg/m3",

	"/0.1L",
	"/0.1L",
	"/0.1L",
	"/0.1L",
	"/0.1L",
	"/0.1L",

	"???"
};

const char *Pmsx003::dataNames[]{
	"PM1.0, CF=1",
	"PM2.5, CF=1",
	"PM10.  CF=1",
	"PM1.0",
	"PM2.5",
	"PM10.",

	"Particles < 0.3 micron",
	"Particles < 0.5 micron",
	"Particles < 1.0 micron",
	"Particles < 2.5 micron",
	"Particles < 5.0 micron",
	"Particles < 10. micron",

	"Reserved_0"
};
